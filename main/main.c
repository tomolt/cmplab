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
#include <string.h>
#include <assert.h>

#include "band.h"

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

static int findword(lzw_word dict[65536], int top, int index, int sym)
{
	for (int i = 0; i < top; ++i) {
		if (dict[i].prefix != index) continue;
		if (dict[i].suffix != sym) continue;
		return i;
	}
	return -1;
}

void encode_lzw(FILE *in, Band *out)
{
	lzw_word dict[65536];
	int top = initdict(dict);
	int bitsize = 9;

	int sym = fgetc(in);
	if (feof(in)) return;
	int index = sym;

	for (;;) {
		sym = fgetc(in);
		if (feof(in)) break;

		int succ = findword(dict, top, index, sym);

		if (succ >= 0) {
			index = succ;
		} else {
			dict[top++] = (lzw_word) {index, sym};
			bwritebits(out, bitsize, index);
			index = sym;

			if (top >= (1 << bitsize) - 1) {
				++bitsize;
			}
		}
	}

	bwritebits(out, bitsize, index);
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

void decode_lzw(Band *in, FILE *out)
{
	lzw_word dict[65536];
	int top = initdict(dict);
	int bitsize = 9;

	int index = breadbits(in, bitsize);
	if (feof(in->file)) return;
	fputword(dict, index, out);

	for (;;) {
		int succ = breadbits(in, bitsize);
		if (feof(in->file)) break;

		int sym = firstsym(dict, succ < top ? succ : index);
		dict[top++] = (lzw_word) {index, sym};

		if (top >= (1 << bitsize) - 2) {
			++bitsize;
		}

		fputword(dict, succ, out);

		index = succ;
	}
}

static void countfreqs(FILE *in, int freqs[256])
{
	for (int i = 0; i < 256; ++i)
		freqs[i] = 0;
	for (;;) {
		int sym = fgetc(in);
		if (sym == EOF) break;
		++freqs[sym];
	}
}

typedef struct {
	int *freqs;
	int *heap;
	int count;
} hf_queue;

static void percup(hf_queue q, int at, int sym)
{
	int const p = (at - 1) / 2;
	if (at > 0 && q.freqs[sym] < q.freqs[q.heap[p]]) {
		q.heap[at] = q.heap[p];
		percup(q, p, sym);
	} else {
		q.heap[at] = sym;
	}
}

static void percdown(hf_queue q, int at, int sym)
{
	int const left  = 2 * at + 1;
	int const right = 2 * at + 2;
	if (left < q.count && q.freqs[sym] > q.freqs[q.heap[left]]) {
		q.heap[at] = q.heap[left];
		percdown(q, left, sym);
	} else if (right < q.count && q.freqs[sym] > q.freqs[q.heap[right]]) {
		q.heap[at] = q.heap[right];
		percdown(q, right, sym);
	} else {
		q.heap[at] = sym;
	}
}

static int freq2hier(int freqs[256], int hier[511])
{
	for (int i = 0; i < 511; ++i)
		hier[i] = -1;
	int heap[256];
	hf_queue q = {freqs, heap, 0};
	for (int sym = 0; sym < 256; ++sym) {
		if (freqs[sym] == 0) continue;
		q.heap[q.count++] = sym;
	}
	for (int i = 127; i >= 0; --i)
		percdown(q, i, q.heap[i]);
	int ncount = 256; // node count
	while (q.count > 1) {
		int first  = q.heap[0];
		percdown(q, 0, q.heap[--q.count]);
		int second = q.heap[0];
		percdown(q, 0, q.heap[--q.count]);
		int new = ncount++;
		hier[first] = new;
		hier[second] = new;
		percup(q, q.count++, new);
	}
	return ncount;
}

static void hier2len(int hier[511], int ncount, int len[256])
{
	int depth[511];
	depth[ncount - 1] = 1;
	for (int i = ncount - 2; i >= 0; --i) {
		if (hier[i] < 0) {
			depth[i] = -1;
		} else {
			depth[i] = depth[hier[i]] + 1;
		}
	}
	for (int i = 0; i < 256; ++i)
		len[i] = depth[i];
}

int main(int argc, char *argv[])
{
	(void) argc, (void) argv;

#if 0
	int freqs[256];
	countfreqs(stdin, freqs);

	int hier[511];
	int ncount = freq2hier(freqs, hier);

	int len[256];
	hier2len(hier, ncount, len);

	for (int i = 0; i < 256; ++i) {
		if (len[i] < 0) continue;
		fprintf(stdout, "#%d : %d\n", i, len[i]);
	}
#endif

#if 1
	FILE *buf = tmpfile();
	Band ew = {buf, 0, 0};
	encode_lzw(stdin, &ew);
	bflushwrite(&ew);
	rewind(buf);
	Band dr = {buf, 0, 0};
	bflushread(&dr);
	decode_lzw(&dr, stdout);
	fclose(buf);
#endif

	return EXIT_SUCCESS;
}
