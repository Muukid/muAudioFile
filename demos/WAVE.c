/*
============================================================
                        DEMO INFO

DEMO NAME:          WAVE.c
DEMO WRITTEN BY:    Muukid
CREATION DATE:      2024-12-25
LAST UPDATED:       2024-12-31

============================================================
                        DEMO PURPOSE

This demo tests reading and information from a WAVE file to
another. This is tested by creating a copy of another WAVE
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

	// WAVE profile
	muWAVEProfile profile;

int main(void)
{
	// Get WAVE profile
	printf("Getting WAVE profile of 'resources/test.wav'...\n");
	res = mu_get_WAVE_profile("resources/test.wav", &profile);
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
	printf("fmt \t- at byte %zu,\t length %" PRIu32 "\n", profile.chunks.fmt, profile.chunks.fmt_len);
	printf("wave \t- at byte %zu,\t length %" PRIu32 "\n", profile.chunks.wave, profile.chunks.wave_len);
	if (profile.chunks.fact == 0) {
		printf("fact \t- NA\n");
	} else {
		printf("fact \t- at byte %zu,\t length %" PRIu32 "\n", profile.chunks.fact, profile.chunks.fact_len);
	}
	if (profile.chunks.cue == 0) {
		printf("cue \t- NA\n");
	} else {
		printf("cue \t- at byte %zu,\t length %" PRIu32 "\n", profile.chunks.cue, profile.chunks.cue_len);
	}
	if (profile.chunks.playlist == 0) {
		printf("playlist \t- NA\n");
	} else {
		printf("playlist \t- at byte %zu,\t length %" PRIu32 "\n", profile.chunks.playlist, profile.chunks.playlist_len);
	}
	if (profile.chunks.assoc_data == 0) {
		printf("assoc-data \t- NA\n");
	} else {
		printf("assoc-data \t- at byte %zu,\t length %" PRIu32 "\n", profile.chunks.assoc_data, profile.chunks.assoc_data_len);
	}

	// Print WAVE profile basic fmt info
	printf("\n== Basic fmt info ==\n");
	printf("Format tag         - %" PRIu16 "\n",   profile.format_tag);
	printf("Channels           - %" PRIu16 "\n",   profile.channels);
	printf("Samples per sec    - %" PRIu32 "Hz\n", profile.samples_per_sec);
	printf("Avg. bytes per sec - %" PRIu32 "\n",   profile.avg_bytes_per_sec);
	printf("Block align        - %" PRIu16 "\n",   profile.block_align);

	// Print WAVE format specific fields
	printf("\n== WAVE format specific fields ==\n");
	switch (profile.format_tag) {
		default: printf("NA\n"); break;

		// WAVE PCM
		case MU_WAVE_FORMAT_PCM: {
			printf("Bits per sample \t- %" PRIu16 "\n", profile.specific_fields.wave_pcm->bits_per_sample);
		} break;
	}

	printf("\n== Write info ==\n");

	// Get unspecific audio format
	muafAudioFormat audio_format = mu_get_WAVE_audio_format(&profile);
	printf("muaf format - %s\n", muaf_audio_format_get_name(audio_format));
	// Return if unsupported
	if (audio_format == MUAF_FORMAT_UNKNOWN) {
		printf("Unknown, exiting...\n");
		mu_free_WAVE_profile(&profile);
		return -1;
	}

	// Get WAVE wrapper based on this file
	printf("Getting wrapper...\n");
	muWAVEWrapper wrapper;
	res = mu_get_WAVE_wrapper_from_WAVE(&profile, &wrapper);
	if (res != MUAF_SUCCESS) {
		printf("Non-successful result - %s", muaf_result_get_name(res));
		if (muaf_result_is_fatal(res)) {
			printf(" (fatal; exiting)\n");
			mu_free_WAVE_profile(&profile);
			return -1;
		}
		printf("\n");
	}

	// Write WAVE wrapper
	printf("Writing wave wrapper to 'output/test_out.wav'...\n");
	res = mu_create_WAVE_wrapper("output/test_out.wav", &wrapper);
	if (res != MUAF_SUCCESS) {
		printf("Non-successful result - %s", muaf_result_get_name(res));
		if (muaf_result_is_fatal(res)) {
			printf(" (fatal; exiting)\n");
			mu_free_WAVE_profile(&profile);
			return -1;
		}
		printf("\n");
	}

	// Extract audio data from file
	printf("Extracting audio data...\n");
	// Allocate audio data
	size_m frame_size = muaf_audio_format_sample_size(audio_format) * ((size_m)profile.channels);
	muByte* audio_data = (muByte*)malloc(frame_size * ((size_m)wrapper.num_frames));
	if (!audio_data) {
		printf("Failed to allocate data; exiting\n");
		mu_free_WAVE_wrapper(&wrapper);
		mu_free_WAVE_profile(&profile);
		return -1;
	}

	// PCM handling
	if (MUAF_FORMAT_IS_PCM(audio_format)) {
		// Read frames
		res = mu_read_WAVE_PCM("resources/test.wav", &profile, 0, wrapper.num_frames, audio_data);
		if (res != MUAF_SUCCESS) {
			printf("Non-successful result - %s", muaf_result_get_name(res));
			if (muaf_result_is_fatal(res)) {
				printf(" (fatal; exiting)\n");
				free(audio_data);
				mu_free_WAVE_wrapper(&wrapper);
				mu_free_WAVE_profile(&profile);
				return -1;
			}
			printf("\n");
		}
	}
	// Non-handled
	else {
		printf("Audio format unsupported in this demo; exiting\n");
		free(audio_data);
		mu_free_WAVE_wrapper(&wrapper);
		mu_free_WAVE_profile(&profile);
		return -1;
	}

	// Reverse each frame
	printf("Reversing frames...\n");

	// Allocate frame container
	muByte* frame = (muByte*)malloc(frame_size);
	if (!frame) {
		printf("Failed to allocate frame container; exiting\n");
		free(audio_data);
		mu_free_WAVE_wrapper(&wrapper);
		mu_free_WAVE_profile(&profile);
		return -1;
	}

	// Loop through half of frames
	uint32_m num_frames = wrapper.num_frames / 2;
	for (uint32_m f = 0; f < num_frames; ++f) {
		// Reverse equivalent frame index:
		uint32_m fi = wrapper.num_frames - f - 1;

		// Get current frame
		memcpy(frame, &audio_data[f*frame_size], frame_size);
		// Copy reverse equivalent frame
		memcpy(&audio_data[f*frame_size], &audio_data[fi*frame_size], frame_size);
		// Copy current sample to reverse equivalent
		memcpy(&audio_data[fi*frame_size], frame, frame_size);
	}

	// Free frame container
	free(frame);

	// Write audio data to new file
	printf("Copying audio data to 'output/test_out.wav'...\n");
	// PCM handling
	if (MUAF_FORMAT_IS_PCM(audio_format)) {
		// Write frames
		res = mu_write_WAVE_PCM("output/test_out.wav", &wrapper, 0, wrapper.num_frames, audio_data);
		if (res != MUAF_SUCCESS) {
			printf("Non-successful result - %s", muaf_result_get_name(res));
			if (muaf_result_is_fatal(res)) {
				printf(" (fatal; exiting)\n");
				free(audio_data);
				mu_free_WAVE_wrapper(&wrapper);
				mu_free_WAVE_profile(&profile);
				return -1;
			}
			printf("\n");
		}
	}
	// - Compressed
	else {
		printf("Audio format unsupported in this demo; exiting\n");
		free(audio_data);
		mu_free_WAVE_wrapper(&wrapper);
		mu_free_WAVE_profile(&profile);
		return -1;
	}

	// Free extracted audio data
	free(audio_data);
	// Free WAVE wrapper
	mu_free_WAVE_wrapper(&wrapper);
	// Free WAVE profile
	mu_free_WAVE_profile(&profile);

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

