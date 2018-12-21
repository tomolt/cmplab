#include <stdio.h>

#define SD_IMPLEMENT_HERE
#include "sd_cuts.h"

#include "band.h"

#define BITSTREAM_DATA_SIZE 16

static void test_bitstream(void)
{
	/* setup */
	sd_push("bitstream");
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
	Band rband = {file, 0, 32};
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

int main()
{
	sd_execmodel = sd_resilient;
	sd_init();
	sd_branch( test_bitstream(); );
	sd_summarize();
	return 0;
}
