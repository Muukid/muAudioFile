/*
============================================================
                        DEMO INFO

DEMO NAME:          audio.c
DEMO WRITTEN BY:    Muukid
CREATION DATE:      2024-12-29
LAST UPDATED:       2024-12-29

============================================================
                        DEMO PURPOSE

This demo tests reading and information from an audio file
to another. This is tested by creating a copy of another
audio file, but with its audio in reverse.

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

	// Audio profile
	muafUnspecificProfile profile;

	// Data read from the audio file
	muByte* data;
	// Size of data
	size_m data_size;

int main(void)
{
	// Get profile
	printf("Getting profile of 'resources/test_audio'...\n");
	res = mu_get_audio_file_profile("resources/test_audio", &profile);
	if (res != MUAF_SUCCESS) {
		printf("Non-successful result - %s", muaf_result_get_name(res));
		if (muaf_result_is_fatal(res)) {
			printf(" (fatal; exiting)\n");
			return -1;
		}
		printf("\n");
	}
	printf("Retrieved\n");

	// Print profile information
	printf("\n== Profile info ==\n");
	printf("Frame count \t- %" PRIu64 "\n", profile.num_frames);
	printf("Sample rate \t- %" PRIu32 "\n", profile.sample_rate);
	printf("Channels \t- %" PRIu16 "\n", profile.num_channels);
	printf("Audio format \t- %s\n", muaf_audio_format_get_nice_name(profile.audio_format));
	printf("File format \t- %s\n", muaf_audio_file_format_get_nice_name(profile.file_format));

	// Do not continue if audio format is unrecognized
	if (profile.audio_format == MUAF_FORMAT_UNKNOWN) {
		printf("Unrecognized/Unsupported audio format; exiting\n");
		mu_free_audio_file_profile(&profile);
		return -1;
	}

	printf("\n== Write info ==\n");

	// Get wrapper based on this file
	printf("Getting wrapper...\n");
	muafUnspecificWrapper wrapper;
	res = mu_get_audio_file_wrapper(&profile, &wrapper);
	if (res != MUAF_SUCCESS) {
		printf("Non-successful result - %s", muaf_result_get_name(res));
		if (muaf_result_is_fatal(res)) {
			printf(" (fatal; exiting)\n");
			mu_free_audio_file_profile(&profile);
			return -1;
		}
		printf("\n");
	}

	// Write wrapper
	printf("Writing wrapper to 'output/test_audio_out'...\n");
	res = mu_create_audio_file_wrapper("output/test_audio_out", &wrapper);
	if (res != MUAF_SUCCESS) {
		printf("Non-successful result - %s", muaf_result_get_name(res));
		if (muaf_result_is_fatal(res)) {
			printf(" (fatal; exiting)\n");
			mu_free_audio_file_profile(&profile);
			return -1;
		}
		printf("\n");
	}

	// Extract audio data
	printf("Extracting audio data...\n");
	muByte* audio_data; // (Holder for decompressed data)
	size_m frame_size; // (Holder for frame size)
	// - Uncompressed
	if (!muaf_audio_format_compressed(profile.audio_format)) {
		// Calculate frame size
		frame_size = muaf_audio_format_sample_size(profile.audio_format) * ((size_m)profile.num_channels);

		// Allocate data
		audio_data = (muByte*)malloc(frame_size * ((size_m)wrapper.num_frames));
		if (!audio_data) {
			printf("Failed to allocate data; exiting\n");
			mu_free_audio_file_wrapper(&wrapper);
			mu_free_audio_file_profile(&profile);
			return -1;
		}

		// Read frames
		res = mu_read_uncompressed_audio_file("resources/test_audio", &profile, 0, wrapper.num_frames, audio_data);
		if (res != MUAF_SUCCESS) {
			printf("Non-succesful result - %s", muaf_result_get_name(res));
			if (muaf_result_is_fatal(res)) {
				printf(" (fatal; exiting)\n");
				free(audio_data);
				mu_free_audio_file_wrapper(&wrapper);
				mu_free_audio_file_profile(&profile);
			}
			printf("\n");
		}
	}
	// - Compressed
	else {
		printf("Compressed audio formats unsupported in this demo; exiting\n");
		mu_free_audio_file_wrapper(&wrapper);
		mu_free_audio_file_profile(&profile);
		return -1;
	}

	// Reverse each frame
	printf("Reversing frames...\n");

	// Allocate frame container
	muByte* frame = (muByte*)malloc(frame_size);
	if (!frame) {
		printf("Failed to allocate frame container; exiting\n");
		free(audio_data);
		mu_free_audio_file_wrapper(&wrapper);
		mu_free_audio_file_profile(&profile);
		return -1;
	}

	// Loop through half of frames
	muafFrames num_frames = wrapper.num_frames / 2;
	for (muafFrames f = 0; f < num_frames; ++f) {
		// Reverse equivalent frame index:
		muafFrames fi = wrapper.num_frames - f - 1;

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
	printf("Copying audio data to 'output/test_audio_out'...\n");
	// - Uncompressed
	if (!muaf_audio_format_compressed(profile.audio_format)) {
		// Write uncompressed audio data
		mu_write_uncompressed_audio_file("output/test_audio_out", &wrapper, 0, wrapper.num_frames, audio_data);
		if (res != MUAF_SUCCESS) {
			printf("Non-successful result - %s", muaf_result_get_name(res));
			if (muaf_result_is_fatal(res)) {
				printf(" (fatal; exiting)\n");
				free(audio_data);
				mu_free_audio_file_wrapper(&wrapper);
				mu_free_audio_file_profile(&profile);
			}
			printf("\n");
		}
	}
	// - Compressed
	else {
		printf("Compressed audio format unsupported in this demo; exiting\n");
		free(audio_data);
		mu_free_audio_file_wrapper(&wrapper);
		mu_free_audio_file_profile(&profile);
	}

	// Free audio data
	free(audio_data);
	// Free wrapper
	mu_free_audio_file_wrapper(&wrapper);
	// Free profile
	mu_free_audio_file_profile(&profile);

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

