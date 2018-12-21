#include <stdio.h>

#define SD_IMPLEMENT_HERE
#include "sd_cuts.h"

#include "band.h"

static void bitstream_emptyband(void)
{
	sd_push("empty band");
	FILE *file = tmpfile();
	Band wband = {file, 0, 0};
	bflushwrite(&wband);
	rewind(file);
	Band rband = {file, 0, 0};
	bflushread(&rband);
	sd_assert(!feof(file));
	fclose(file);
	sd_pop();
}

static void bitstream_nooverread(void)
{
	sd_push("no overread");
	FILE *file = tmpfile();
	fwrite("ABCD", 1, 4, file);
	rewind(file);
	Band band = {file, 0, 0};
	bflushread(&band);
	breadbits(&band, 32);
	sd_assert(!feof(file));
	fclose(file);
	sd_pop();
}

#define BITSTREAM_DATA_SIZE 1024

static void bitstream_roundtrip(void)
{
	/* setup */
	sd_push("roundtrip");
	FILE *file = tmpfile();
	/* generate test data */
	struct {
		unsigned long bits;
		int length;
	} data[BITSTREAM_DATA_SIZE];
	for (int i = 0; i < BITSTREAM_DATA_SIZE; ++i) {
		data[i].length = rand() % 33;
		data[i].bits = rand() & ((1 << data[i].length) - 1);
	}
	/* write */
	Band wband = {file, 0, 0};
	for (int i = 0; i < BITSTREAM_DATA_SIZE; ++i) {
		bwritebits(&wband, data[i].length, data[i].bits);
	}
	bflushwrite(&wband);
	rewind(file);
	/* read */
	Band rband = {file, 0, 0};
	bflushread(&rband);
	for (int i = 0; i < BITSTREAM_DATA_SIZE; ++i) {
		sd_push("i = %d", i);
		unsigned long read_back = breadbits(&rband, data[i].length);
		sd_assertiq(data[i].bits, read_back);
		sd_pop();
	}
	/* cleanup */
	fclose(file);
	sd_pop();
}

static void test_bitstream(void)
{
	sd_push("bitstream");
	bitstream_emptyband();
	bitstream_nooverread();
	bitstream_roundtrip();
	sd_pop();
}

int main()
{
	sd_execmodel = sd_resilient;
	sd_init();
	sd_branch( test_bitstream(); );
	sd_summarize();
	return 0;
}
