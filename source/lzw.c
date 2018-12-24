/****
 * This file is part of cmplab, the rapid compression experimentation project.
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

#include <stdio.h>
#include <stdint.h>

#include "bitstream.h"
#include "base.h"

#define LZW_DICT_SIZE 65536

typedef int LzwIdx;

typedef struct {
	LzwIdx prefix;
	Symbol suffix;
} lzw_word;

static LzwIdx initdict(lzw_word dict[LZW_DICT_SIZE])
{
	for (Symbol sym = 0; sym < ALPHABET_SIZE; ++sym)
		dict[sym] = (lzw_word){-1, sym};
	return ALPHABET_SIZE;
}

static LzwIdx findword(lzw_word dict[LZW_DICT_SIZE], LzwIdx top, LzwIdx index, Symbol sym)
{
	for (LzwIdx i = 0; i < top; ++i) {
		if (dict[i].prefix != index) continue;
		if (dict[i].suffix != sym) continue;
		return i;
	}
	return -1;
}

void encode_lzw(FILE *in, Bitstream *out)
{
	lzw_word dict[LZW_DICT_SIZE];
	LzwIdx top = initdict(dict);

	int bitsize = 1;
	while (ALPHABET_SIZE >> bitsize > 0) ++bitsize;

	LzwIdx index = fgetc(in);
	if (feof(in)) return;

	for (;;) {
		Symbol sym = fgetc(in);
		if (feof(in)) break;

		LzwIdx succ = findword(dict, top, index, sym);

		if (succ >= 0) {
			index = succ;
		} else {
			dict[top++] = (lzw_word) {index, sym};
			bitstreamWriteBits(out, bitsize, index);
			index = sym;

			if (top >= (1 << bitsize) - 1) {
				++bitsize;
			}
		}
	}

	bitstreamWriteBits(out, bitsize, index);
}

static Symbol firstsym(lzw_word dict[LZW_DICT_SIZE], LzwIdx idx)
{
	if (dict[idx].prefix < 0)
		return dict[idx].suffix;
	else
		return firstsym(dict, dict[idx].prefix);
}

static void fputword(lzw_word dict[LZW_DICT_SIZE], LzwIdx idx, FILE *out)
{
	if (idx < 0) return;
	fputword(dict, dict[idx].prefix, out);
	fputc(dict[idx].suffix, out);
}

void decode_lzw(Bitstream *in, FILE *out)
{
	lzw_word dict[LZW_DICT_SIZE];
	LzwIdx top = initdict(dict);

	int bitsize = 1;
	while (ALPHABET_SIZE >> bitsize > 0) ++bitsize;

	LzwIdx index = bitstreamReadBits(in, bitsize);
	if (feof(in->file)) return;
	fputword(dict, index, out);

	for (;;) {
		LzwIdx succ = bitstreamReadBits(in, bitsize);
		if (feof(in->file)) break;

		Symbol sym = firstsym(dict, succ < top ? succ : index);
		dict[top++] = (lzw_word) {index, sym};

		if (top >= (1 << bitsize) - 2) {
			++bitsize;
		}

		fputword(dict, succ, out);

		index = succ;
	}
}
