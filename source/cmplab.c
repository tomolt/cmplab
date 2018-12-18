#include <stdlib.h>
#include <stdio.h>

typedef struct {
	int prefix; // index into dict
	int suffix; // a symbol
} lzw_word;

void encode_lzw(FILE *in, FILE *out)
{
	lzw_word dict[65536];
	for (int i = 0; i < 256; ++i)
		dict[i] = (lzw_word){-1, i};
	int top = 256;
	int sym = fgetc(in);
	if (sym == EOF) return;
	int index = sym;
	for (;;) {
		sym = fgetc(in);
		if (sym == EOF) break;
		lzw_word word = {index, sym};
		int succ = -1;
		for (int i = 0; i < top; ++i) {
			if (dict[i].prefix != word.prefix) continue;
			if (dict[i].suffix != word.suffix) continue;
			succ = i;
			break;
		}
		if (succ >= 0) {
			index = succ;
		} else {
			dict[top++] = word;
			fputc(index / 256, out);
			fputc(index % 256, out);
			index = sym;
		}
	}
	fputc(index / 256, out);
	fputc(index % 256, out);
}

int main(int argc, char *argv[])
{
	(void) argc, (void) argv;
	encode_lzw(stdin, stdout);
	return EXIT_SUCCESS;
}
