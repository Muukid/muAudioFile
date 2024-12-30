/*
============================================================
                        DEMO INFO

DEMO NAME:          FLAC.c
DEMO WRITTEN BY:    Muukid
CREATION DATE:      2024-12-30
LAST UPDATED:       2024-12-31

============================================================
                        DEMO PURPOSE

This demo tests reading and information from a FLAC file to
another. This is tested by creating a copy of another FLAC
file, but with its audio in reverse.

This demo is dependent on the 'resources' folder within the
demos folder. It is also dependent on the folder 'output'
existing within the executing directory of the program.

============================================================
                        LICENSE INFO

All code is licensed under MIT License or public domain, 
whichever you prefer.
More explicit license information at the end of file.

============================================================
*/

/* Including */
	
	// Include muAudioFile
	#define MUAF_NAMES
	#define MUAF_IMPLEMENTATION
	#include "muAudioFile.h"

	// For printing:
	#include <stdio.h>
	// For print types:
	#include <inttypes.h>
	// For allocation:
	#include <stdlib.h>
	// For memory functions:
	#include <string.h>

/* Resources */

	// Result value holder
	muafResult res;

	// FLAC profile
	muFLACProfile profile;

int main(void)
{
	// Get FLAC profile
	printf("Getting FLAC profile of 'resources/test.flac'...\n");
	res = mu_get_FLAC_profile("resources/test.flac", &profile);
	if (res != MUAF_SUCCESS) {
		printf("Non-successful result - %s", muaf_result_get_name(res));
		if (muaf_result_is_fatal(res)) {
			printf(" (fatal; exiting)\n");
			return -1;
		}
		printf("\n");
	}
	printf("Retrieved\n");

	// Print chunk info
	printf("\n== Chunk info ==\n");
	printf("min_block_size  - %" PRIu16 "\n", profile.min_block_size);
	printf("max_block_size  - %" PRIu16 "\n", profile.max_block_size);
	printf("min_frame_size  - %" PRIu32 "\n", profile.min_frame_size);
	printf("max_frame_size  - %" PRIu32 "\n", profile.max_frame_size);
	printf("sample_rate     - %" PRIu32 "\n", profile.sample_rate);
	printf("num_channels    - %" PRIu8  "\n", profile.num_channels);
	printf("bits_per_sample - %" PRIu8  "\n", profile.bits_per_sample);
	printf("num_samples     - %" PRIu64 "\n", profile.num_samples);
	printf("low_checksum    - %" PRIu64 "\n", profile.low_checksum);
	printf("high_checksum   - %" PRIu64 "\n", profile.high_checksum);

	// Print metadata block info
	printf("\n== Metadata block info ==\n");
	printf("Metadata block count - %zu\n", profile.num_metadata_blocks);
	for (size_m b = 0; b < profile.num_metadata_blocks; ++b) {
		printf("#%zu\t - type %" PRIu8 ",\t length %" PRIu32 ",\t index %zu\n", b,
			profile.metadata_blocks[b].block_type,
			profile.metadata_blocks[b].length,
			profile.metadata_blocks[b].index
		);
	}

	// Free FLAC profile
	mu_free_FLAC_profile(&profile);
	printf("\nSuccessful\n");
	return 0;
}

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2024 Hum
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/

