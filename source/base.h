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

// depends on stdint.h
// depends on bitstream.h

#ifdef CMPLAB_BASE_H
#error multiple inclusion
#endif
#define CMPLAB_BASE_H

#define STATIC_LENGTH(a) (sizeof(a) / sizeof(*(a)))

#define KB(x) ((x) * 1024)
#define MB(x) KB(KB(x))
#define GB(x) KB(MB(x))

typedef int32_t Symbol;
typedef Symbol  Synsym; // synthetic symbol
typedef int64_t Count; // roughly proportional to the length of the input

#define ALPHABET_SIZE 256

typedef struct {
	char const *identifier;
	void (*encode)(FILE *, Bitstream *);
	void (*decode)(Bitstream *, FILE *);
} Algorithm;
