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

#include <stdlib.h>
#include <stdio.h>

#include "sd_cuts.h"

#include "base.h"
#include "bitstream.h"

static void emptyBitstream(void)
{
	sd_push("empty bitstream");
	FILE *file = tmpfile();
	Bitstream w = {file, 0, 0};
	bitstreamFlushWrite(&w);
	rewind(file);
	Bitstream r = {file, 0, 0};
	bitstreamFlushRead(&r);
	sd_assert(!feof(file));
	fclose(file);
	sd_pop();
}

static void noOverread(void)
{
	sd_push("no overread");
	FILE *file = tmpfile();
	fwrite("ABCD", 1, 4, file);
	rewind(file);
	Bitstream b = {file, 0, 0};
	bitstreamFlushRead(&b);
	bitstreamReadBits(&b, 32);
	sd_assert(!feof(file));
	fclose(file);
	sd_pop();
}

#define ROUNDTRIP_DATA_SIZE MB(1)

static void roundtrip(void)
{
	/* setup */
	sd_push("roundtrip");
	FILE *file = tmpfile();
	/* generate test data */
	struct rtdata {
		unsigned long bits;
		int length;
	} *data = malloc(ROUNDTRIP_DATA_SIZE * sizeof(struct rtdata));
	for (int i = 0; i < ROUNDTRIP_DATA_SIZE; ++i) {
		data[i].length = rand() % 33;
		data[i].bits = rand() & ((1 << data[i].length) - 1);
	}
	/* write */
	Bitstream w = {file, 0, 0};
	for (int i = 0; i < ROUNDTRIP_DATA_SIZE; ++i) {
		bitstreamWriteBits(&w, data[i].length, data[i].bits);
	}
	bitstreamFlushWrite(&w);
	rewind(file);
	/* read */
	Bitstream r = {file, 0, 0};
	bitstreamFlushRead(&r);
	for (int i = 0; i < ROUNDTRIP_DATA_SIZE; ++i) {
		sd_push("i = %d", i);
		unsigned long read_back = bitstreamReadBits(&r, data[i].length);
		sd_assertiq(data[i].bits, read_back);
		sd_pop();
	}
	/* cleanup */
	free(data);
	fclose(file);
	sd_pop();
}

void bitstreamTest(void)
{
	sd_push("bitstream");
	emptyBitstream();
	noOverread();
	roundtrip();
	sd_pop();
}
