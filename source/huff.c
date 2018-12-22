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

#define _GNU_SOURCE // for qsort_r
#include <stdlib.h>
#include <stdio.h>

#include "base.h"
#include "band.h"

static void countfreqs(FILE *in, Count freqs[ALPHABET_SIZE])
{
	for (Symbol sym = 0; sym < ALPHABET_SIZE; ++sym)
		freqs[sym] = 0;
	for (;;) {
		Symbol sym = fgetc(in);
		if (feof(in)) break;
		++freqs[sym];
	}
}

typedef struct {
	Count *freqs;
	Synsym *heap;
	Count count;
} hf_queue;

static void percup(hf_queue q, Count at, Synsym sym)
{
	Count const p = (at - 1) / 2;
	if (at > 0 && q.freqs[sym] < q.freqs[q.heap[p]]) {
		q.heap[at] = q.heap[p];
		percup(q, p, sym);
	} else {
		q.heap[at] = sym;
	}
}

static void percdown(hf_queue q, Count at, Synsym sym)
{
	Count const left  = 2 * at + 1;
	Count const right = 2 * at + 2;
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

static Count freq2hier(Count sfreqs[ALPHABET_SIZE], Symbol hier[ALPHABET_SIZE * 2 - 1])
{
	Synsym heap[ALPHABET_SIZE];
	Count freqs[ALPHABET_SIZE * 2 - 1];
	for (Symbol i = 0; i < ALPHABET_SIZE; ++i)
		freqs[i] = sfreqs[i];
	hf_queue q = {freqs, heap, 0};
	for (Symbol sym = 0; sym < ALPHABET_SIZE; ++sym) {
		if (freqs[sym] == 0) continue;
		q.heap[q.count++] = sym;
	}
	for (Count i = q.count / 2 - 1; i >= 0; --i)
		percdown(q, i, q.heap[i]);

	for (Synsym i = 0; i < ALPHABET_SIZE * 2 - 1; ++i)
		hier[i] = -1;
	Count ncount = ALPHABET_SIZE; // node count
	while (q.count > 1) {
		Synsym first  = q.heap[0];
		percdown(q, 0, q.heap[--q.count]);
		Synsym second = q.heap[0];
		percdown(q, 0, q.heap[--q.count]);
		Synsym new = ncount++;
		q.freqs[new] = q.freqs[first] + q.freqs[second];
		hier[first] = new;
		hier[second] = new;
		percup(q, q.count++, new);
	}
	return ncount;
}

static void hier2len(Synsym hier[ALPHABET_SIZE * 2 - 1], Count ncount, int len[ALPHABET_SIZE])
{
	int depth[ALPHABET_SIZE * 2 - 1];
	depth[ncount - 1] = 1;
	for (int i = ncount - 2; i >= 0; --i) {
		if (hier[i] < 0) {
			depth[i] = -1;
		} else {
			depth[i] = depth[hier[i]] + 1;
		}
	}
	for (int i = ncount - 1; i < ALPHABET_SIZE; ++i)
		depth[i] = -1;
	for (int i = 0; i < ALPHABET_SIZE; ++i)
		len[i] = depth[i];
}

static int symlen_compare(void const *ap, void const *bp, void *ud)
{
	int *len = ud;
	Symbol as = * (Symbol *) ap;
	Symbol bs = * (Symbol *) bp;
	int ld = len[as] - len[bs];
	if (ld != 0) {
		return ld;
	} else {
		return as - bs;
	}
}

static void symsbylen(int len[ALPHABET_SIZE], Symbol syms[ALPHABET_SIZE])
{
	// TODO filter out unused symbols before sorting
	for (Symbol sym = 0; sym < ALPHABET_SIZE; ++sym)
		syms[sym] = sym;
	qsort_r(syms, ALPHABET_SIZE, sizeof(*syms), symlen_compare, len);
}

static void len2code(Symbol syms[ALPHABET_SIZE], int len[ALPHABET_SIZE], unsigned long code[ALPHABET_SIZE])
{
	Symbol sym_start = 0;
	while (len[syms[sym_start]] < 0) ++sym_start;

	unsigned long next = 0;
	int prev_len = len[syms[sym_start]];
	for (Symbol i = sym_start; i < ALPHABET_SIZE; ++i) {
		Symbol sym = syms[i];
		next <<= len[sym] - prev_len;
		code[sym] = next++;
		prev_len = len[sym];
	}
}

void encode_huff(FILE *in, Band *out, Band *table)
{
	Count freqs[ALPHABET_SIZE];
	Synsym hier[ALPHABET_SIZE * 2 - 1];
	int len[ALPHABET_SIZE];
	Symbol syms[ALPHABET_SIZE];
	countfreqs(stdin, freqs);
	int ncount = freq2hier(freqs, hier);
	hier2len(hier, ncount, len);
	// TODO output table
	(void) table;

	rewind(in);
	symsbylen(len, syms);
	unsigned long code[ALPHABET_SIZE];
	len2code(syms, len, code);
	for (;;) {
		Symbol sym = fgetc(in);
		if (feof(in)) break;

		bwritebits(out, len[sym], code[sym]);
	}
}
