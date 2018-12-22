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

#include "base.h"
#include "band.h"

extern void encode_lzw(FILE *in, Band *out);
extern void decode_lzw(Band *in, FILE *out);

extern void encode_huff(FILE *in, Band *out, Band *table);

int main(int argc, char *argv[])
{
	(void) argc, (void) argv;

#if 1
	Band ew = {stdout, 0, 0};
	encode_huff(stdin, &ew, NULL);
	bflushwrite(&ew);
#endif

#if 0
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
