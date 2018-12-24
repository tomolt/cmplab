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
#include <string.h>

#include "base.h"
#include "bitstream.h"

extern void encode_lzw(FILE *in, Bitstream *out);
extern void decode_lzw(Bitstream *in, FILE *out);

extern void encode_zle(FILE *in, Bitstream *out);
extern void decode_zle(Bitstream *in, FILE *out);

extern void encode_huff(FILE *in, Bitstream *out, Bitstream *table);

static void usage(char const *name, char const *arg)
{
	fprintf(stderr, "incorrect %s.\n", arg);
	fprintf(stderr, "%s: <usage goes here at some point>\n", name);
}

int main(int argc, char *argv[])
{
	if (argc != 3) {
		usage(argv[0], "argument count");
		return EXIT_FAILURE;
	}

	if (strcmp(argv[1], "lzw") == 0) {
	} else {
		usage(argv[0], "algorithm");
		return EXIT_FAILURE;
	}

	enum { ENCODE, DECODE, ROUNDTRIP } mode;
	if (strcmp(argv[2], "encode") == 0) {
		mode = ENCODE;
	} else if (strcmp(argv[2], "decode") == 0) {
		mode = DECODE;
	} else if (strcmp(argv[2], "roundtrip") == 0) {
		mode = ROUNDTRIP;
	} else {
		usage(argv[0], "mode");
		return EXIT_FAILURE;
	}

	FILE *buf;
	Bitstream outb, inb;
	switch (mode) {
	case ENCODE:
		outb = (Bitstream) {stdout, 0, 0};
		encode_lzw(stdin, &outb);
		bitstreamFlushWrite(&outb);
		break;
	case DECODE:
		inb = (Bitstream) {stdin, 0, 0};
		bitstreamFlushRead(&inb);
		decode_lzw(&inb, stdout);
		break;
	case ROUNDTRIP:
		buf = tmpfile();
		outb = (Bitstream) {buf, 0, 0};
		encode_lzw(stdin, &outb);
		bitstreamFlushWrite(&outb);
		rewind(buf);
		inb = (Bitstream) {buf, 0, 0};
		bitstreamFlushRead(&inb);
		decode_lzw(&inb, stdout);
		fclose(buf);
		break;
	}

	return EXIT_SUCCESS;
}
