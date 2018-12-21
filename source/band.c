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

#include <stdio.h>

#include "band.h"

static void flushreadbuf(Band *band)
{
	band->buf_bits = (fgetc(band->file) << 24)
		| (fgetc(band->file) << 16)
		| (fgetc(band->file) << 8)
		|  fgetc(band->file);
	band->buf_cur = 0;
}

static void readbits_r(Band *band, int count, int shift, unsigned long *bits)
{
	int left = 32 - band->buf_cur;
	if (left < count) {
		*bits |= (band->buf_bits >> band->buf_cur) << shift;
		flushreadbuf(band);
		readbits_r(band, count - left, shift + left, bits);
	} else {
		unsigned long mask = (1 << count) - 1;
		*bits |= ((band->buf_bits >> band->buf_cur) & mask) << shift;
		band->buf_cur += count;
	}
}

unsigned long breadbits(Band *band, int count)
{
	unsigned long bits = 0;
	readbits_r(band, count, 0, &bits);
	return bits;
}

static void flushwritebuf(Band *band)
{
	fputc(band->buf_bits >> 24, band->file);
	fputc((band->buf_bits >> 16) & 0xFF, band->file);
	fputc((band->buf_bits >> 8) & 0xFF, band->file);
	fputc(band->buf_bits & 0xFF, band->file);
	band->buf_bits = 0;
	band->buf_cur = 0;
}

void bwritebits(Band *band, int count, unsigned long bits)
{
	int left = 32 - band->buf_cur;
	if (left < count) {
		band->buf_bits |= bits << band->buf_cur;
		flushwritebuf(band);
		bwritebits(band, count - left, bits >> count);
	} else {
		unsigned long mask = (1 << count) - 1;
		band->buf_bits |= (bits & mask) << band->buf_cur;
		band->buf_cur += count;
	}
}

void bclosewrite(Band *band)
{
	if (band->buf_cur > 0)
		flushwritebuf(band);
	fclose(band->file);
}
