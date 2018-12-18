/****
 * MIT License
 *
 * Copyright (c) 2018 Thomas Oltmann
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 ****/

#include <stdlib.h>
#include <stdio.h>

typedef struct {
	int prefix; // index into dict
	int suffix; // a symbol
} lzw_word;

static int initdict(lzw_word dict[65536])
{
	for (int i = 0; i < 256; ++i)
		dict[i] = (lzw_word){-1, i};
	return 256;
}

static int fgetidx(FILE *in)
{
	int hi = fgetc(in);
	if (hi == EOF) return EOF;
	int lo = fgetc(in);
	if (lo == EOF) return EOF;
	return hi * 256 + lo;
}

static void fputidx(int idx, FILE *out)
{
	fputc(idx / 256, out);
	fputc(idx % 256, out);
}

static int findword(lzw_word dict[65536], int top, int index, int sym)
{
	for (int i = 0; i < top; ++i) {
		if (dict[i].prefix != index) continue;
		if (dict[i].suffix != sym) continue;
		return i;
	}
	return -1;
}

void encode_lzw(FILE *in, FILE *out)
{
	lzw_word dict[65536];
	int top = initdict(dict);

	int sym = fgetc(in);
	if (sym == EOF) return;
	int index = sym;

	for (;;) {
		sym = fgetc(in);
		if (sym == EOF) break;

		int succ = findword(dict, top, index, sym);

		if (succ >= 0) {
			index = succ;
		} else {
			dict[top++] = (lzw_word) {index, sym};
			fputidx(index, out);
			index = sym;
		}
	}

	fputidx(index, out);
}

static int firstsym(lzw_word dict[65536], int idx)
{
	if (dict[idx].prefix < 0)
		return dict[idx].suffix;
	else
		return firstsym(dict, dict[idx].prefix);
}

static void fputword(lzw_word dict[65536], int idx, FILE *out)
{
	if (idx < 0) return;
	fputword(dict, dict[idx].prefix, out);
	fputc(dict[idx].suffix, out);
}

void decode_lzw(FILE *in, FILE *out)
{
	lzw_word dict[65536];
	int top = initdict(dict);

	int index = fgetidx(in);
	if (index == EOF) return;
	fputword(dict, index, out);

	for (;;) {
		int succ = fgetidx(in);
		if (succ == EOF) break;

		int sym = firstsym(dict, succ < top ? succ : index);
		dict[top++] = (lzw_word) {index, sym};

		fputword(dict, succ, out);

		index = succ;
	}
}

int main(int argc, char *argv[])
{
	(void) argc, (void) argv;

	FILE *buf = tmpfile();
	encode_lzw(stdin, buf);
	rewind(buf);
	decode_lzw(buf, stdout);
	fclose(buf);

	return EXIT_SUCCESS;
}
