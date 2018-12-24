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

#include "bitstream.h"

static void flushReadBuffer(Bitstream *bs)
{
	unsigned long d1 = fgetc(bs->file);
	unsigned long d2 = fgetc(bs->file);
	unsigned long d3 = fgetc(bs->file);
	unsigned long d4 = fgetc(bs->file);
	bs->buf_bits = (d1 << 24) | (d2 << 16) | (d3 << 8) | d4;
	bs->buf_cur = 0;
}

void bitstreamFlushRead(Bitstream *bs)
{
	flushReadBuffer(bs);
}

static void readBitsRecursive(Bitstream *bs,
	int count, int shift, unsigned long *bits)
{
	int left = 32 - bs->buf_cur;
	if (left < count) {
		*bits |= (bs->buf_bits >> bs->buf_cur) << shift;
		flushReadBuffer(bs);
		readBitsRecursive(bs, count - left, shift + left, bits);
	} else {
		unsigned long mask = (1 << count) - 1;
		*bits |= ((bs->buf_bits >> bs->buf_cur) & mask) << shift;
		bs->buf_cur += count;
	}
}

unsigned long bitstreamReadBits(Bitstream *bs, int count)
{
	unsigned long bits = 0;
	readBitsRecursive(bs, count, 0, &bits);
	return bits;
}

static void flushWriteBuffer(Bitstream *bs)
{
	fputc(bs->buf_bits >> 24, bs->file);
	fputc((bs->buf_bits >> 16) & 0xFF, bs->file);
	fputc((bs->buf_bits >> 8) & 0xFF, bs->file);
	fputc(bs->buf_bits & 0xFF, bs->file);
	bs->buf_bits = 0;
	bs->buf_cur = 0;
}

void bitstreamWriteBits(Bitstream *bs, int count, unsigned long bits)
{
	int left = 32 - bs->buf_cur;
	if (left < count) {
		bs->buf_bits |= bits << bs->buf_cur;
		flushWriteBuffer(bs);
		bitstreamWriteBits(bs, count - left, bits >> left);
	} else {
		unsigned long mask = (1 << count) - 1;
		bs->buf_bits |= (bits & mask) << bs->buf_cur;
		bs->buf_cur += count;
	}
}

void bitstreamFlushWrite(Bitstream *bs)
{
	flushWriteBuffer(bs);
}
