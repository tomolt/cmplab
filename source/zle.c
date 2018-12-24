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

void encode_zle(FILE *in, Bitstream *out)
{
	int bitsize = 1;
	while ((ALPHABET_SIZE - 1) >> bitsize > 0) ++bitsize;

	Symbol sym;
	for (;;) {
		do {
			sym = fgetc(in);
			if (feof(in)) return;
			bitstreamWriteBits(out, bitsize, sym);
		} while (sym != 0);

		unsigned int run = 0;
		do {
			sym = fgetc(in);
			++run; // also counts the first non-zero, but that's okay because we start run from 0 instead of 1.
		} while (sym == 0);
		bitstreamWriteBits(out, 16, run - 1); // we know that run is always at least 1, so we start subtract that.
	}
}

void decode_zle(Bitstream *in, FILE *out)
{
	int bitsize = 1;
	while ((ALPHABET_SIZE - 1) >> bitsize > 0) ++bitsize;

	for (;;) {
		for (;;) {
			Symbol sym = bitstreamReadBits(in, bitsize);
			if (feof(in->file)) return;
			fputc(sym, out);
			if (sym == 0) break;
		}

		unsigned int run = bitstreamReadBits(in, 16) + 1; // we know that run is always at least 1, so we start subtract that.
		do {
			fputc(0, out);
			--run;
		} while (run > 0); // works because we already output the first zero in the previous loop.
	}
}
