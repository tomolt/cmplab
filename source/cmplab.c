#include <stdlib.h>
#include <stdio.h>

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

static int fgetidx(FILE *in)
{
	int hi = fgetc(in);
	if (hi == EOF) return EOF;
	int lo = fgetc(in);
	if (lo == EOF) return EOF;
	return hi * 256 + lo;
}

static void fputidx(int idx, FILE *out)
{
	fputc(idx / 256, out);
	fputc(idx % 256, out);
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

void encode_lzw(FILE *in, FILE *out)
{
	lzw_word dict[65536];
	int top = initdict(dict);

	int sym = fgetc(in);
	if (sym == EOF) return;
	int index = sym;

	for (;;) {
		sym = fgetc(in);
		if (sym == EOF) break;

		int succ = findword(dict, top, index, sym);

		if (succ >= 0) {
			index = succ;
		} else {
			dict[top++] = (lzw_word) {index, sym};
			fputidx(index, out);
			index = sym;
		}
	}

	fputidx(index, out);
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

void decode_lzw(FILE *in, FILE *out)
{
	lzw_word dict[65536];
	int top = initdict(dict);

	int index = fgetidx(in);
	if (index == EOF) return;
	fputword(dict, index, out);

	for (;;) {
		int succ = fgetidx(in);
		if (succ == EOF) break;

		int sym = firstsym(dict, succ < top ? succ : index);
		dict[top++] = (lzw_word) {index, sym};

		fputword(dict, succ, out);

		index = succ;
	}
}

int main(int argc, char *argv[])
{
	(void) argc, (void) argv;

	FILE *buf = tmpfile();
	encode_lzw(stdin, buf);
	rewind(buf);
	decode_lzw(buf, stdout);
	fclose(buf);

	return EXIT_SUCCESS;
}
