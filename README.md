

# muAudioFile v1.0.0

muAudioFile (acronymized to "muaf") is a public domain single-file C library for reading and writing audio file data. Its header is automatically defined upon inclusion if not already included (`MUAF_H`), and the source code is defined if `MUAF_IMPLEMENTATION` is defined, following the internal structure of:

```c
#ifndef MUAF_H
#define MUAF_H
// (Header code)
#endif

#ifdef MUAF_IMPLEMENTATION
// (Source code)
#endif
```

Therefore, a standard inclusion of the file to get all automatic functionality looks like:

```c
#define MUAF_IMPLEMENTATION
#include "muAudioFile.h"
```

More information about the general structure of a mu library is provided at [the mu library information GitHub repository](https://github.com/Muukid/mu-library-information).

# Demos

Demos are designed for muaf to both test its functionality and to allow users to get the basic idea of the structure of the library quickly without having to read the documentation in full. These demos are available in the `demos` folder.

## Demo resources

The demos use other files to operate correctly when running as a compiled executable. These other files can be found in the `resources` folder within `demos`, and this folder is expected to be in the same location that the program is executing from. For example, if a user compiles a demo into `main.exe`, and decides to run it, the `resources` folder from `demos` should be in the same directory as `main.exe`.

# Licensing

muaf is licensed under public domain or MIT, whichever you prefer. More information is provided in the accompanying file `license.md` and at the bottom of `muAudioFile.h`.

# General known bugs and limitations

This section covers all of the general known bugs and limitations of muaf. Known bugs and limitations for each supported file format are individually provided in their respective sections.

## File access

muaf is designed so that audio data can be retrieved from an audio file without ever needing to have the whole file loaded in memory at once. However, this means that the file needs to be opened and closed across function calls, meaning that muaf assumes that the file remains unchanged across these function calls, which may not be the case. muaf does not check for this nor attempt to prevent files from being unchanged across these function calls, and it is up to the user to ensure this. Failure to do so can result in undefined behavior, and likely unexpected crashing.

## Unvalidated information

When reading from a file, muaf only checks if the values it depends on are correct. Since muaf does not check every value within an audio file, there can be values that go unvalidated, without muaf throwing any warnings or non-fatal result values to indicate that they may be incorrect. In other words, even if muaf has successfully read from a file, that does not mean that the file has been fully validated.

Overall, muaf is okay with files not strictly complying with the specification when reading from them (such as things like end-of-file padding not being performed). muaf still follows the specification as closely as possible when writing to audio files.


# Other library dependencies

muaf has a dependency on:

* [muUtility v2.0.1](https://github.com/Muukid/muUtility/releases/tag/v2.0.1).

> Note that mu libraries store their dependencies within their files, so you don't need to import these dependencies yourself; this section's purpose is purely to provide more information about the contents that this file defines. The libraries listed may also have other dependencies that they also include that aren't explicitly listed here.

# Format-inspecific API

muaf's API is split into two sections: the format-specific API, and the format-inspecific API. The inspecific API wraps around the specific API, encapsulating the specific API's functionality, and allowing you to retrieve general information about an audio file without having to directly consider what specific audio file format it is.

## Audio profile

For each audio format, general information about the audio stored in the file can be retrieved. This general information is called the "profile". This profile is defined for several reasons:

* It gives the user a way to load general information about the audio file first, before taking more processing time and memory to load more significant portions of the data stored within the audio file.

* Having this information gives the user a way to load select portions of the audio data encoded in the file without ever having to load the whole audio file at once in memory, which can be useful or even necessary for larger audio files.

* It allows the API to internally map how the file is formatted and hold onto that information for later, which can make subsequent calls to load information from the audio file quicker, since information about the file's general structure is held onto across function calls.

In the format-specific API, the profile stores general information that very directly corresponds to the general information stored in the audio file itself. For example, [the WAVE profile](#wave-profile) stores information about what supported chunks are provided in the file and where they are. It also stores the information within certain chunks that provide general information about the audio, such as the fmt chunk, which provides information such as the number of channels.

In the format-inspecific API, the profile primarily stores general information that all of the supported audio file formats within muaf have in common. This allows the user to retrieve information about the audio file without having to know how to retrieve it from the exact audio file format that it's in. The inspecific profile also loads and stores the specific profile information.

An audio file's inspecific profile can be retrieved using the function `mu_get_audio_file_profile`, defined below: 

```c
MUDEF muafResult mu_get_audio_file_profile(const char* filename, muafInspecificProfile* profile);
```


Once an inspecific profile has been successfully or non-fatally retrieved (indicated by [the result return value](#result)), the profile is filled with data, some of which may be manually allocated automatically. To free this data, use the function `mu_free_audio_file_profile`, defined below: 

```c
MUDEF void mu_free_audio_file_profile(muafInspecificProfile* profile);
```


The inspecific profile is represented by the struct `muafInspecificProfile`, which has the following members:

* `muafSampleCount num_samples` - the number of samples.

* `muafSampleRate sample_rate` - the rate of samples per second.

* `muafChannels num_channels` - the number of channels.

* `muafFormat file_format` - the [file format](#audio-file-formats).

* `muafSpecificProfile specific` - the [format-specific profile](#format-specific-profile).

### Audio file formats

The type `muafFormat` represents a file format supported in muaf, and has the following defined values:

* `MUAF_UNKNOWN` - an unknown/unrecognized file format.

* `MUAF_WAVE` - the [WAVE file format](#wave-api).

### Format specific profile

The union `muafSpecificProfile` acts as a container for the format-specific profile of some audio file, and has the following members:

* `muWAVEProfile* WAVE` - the [WAVE profile](#wave-profile).

## Get audio file format

The function `mu_audio_file_format` retrieves the [audio file format](#audio-file-formats) of a given file, defined below: 

```c
MUDEF muafFormat mu_audio_file_format(const char* filename);
```


This function does not use the filename extension to identify the audio file format, but instead, the actual contents of the file itself. This function returns `MUAF_UNKNOWN` if rather the audio file format could not be identified to be anything supported in muaf, or muaf failed to retrieve the file's data.

# WAVE API

This section describes muaf's API for the [Waveform Audio File Format](https://en.wikipedia.org/wiki/WAV), or WAVE. The code for this API is built based off of the original August 1991 specification for WAVE (specifically [this archive](https://www.mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/Docs/riffmci.pdf)), and this section of muaf's documentation will reference values that are defined in this specification.

## WAVE profile

A WAVE file's profile can be retrieved with the function `mu_get_WAVE_profile`, defined below: 

```c
MUDEF muafResult mu_get_WAVE_profile(const char* filename, muWAVEProfile* profile);
```


Once retrieved, the profile must be deallocated at some point using the function `mu_free_WAVE_profile`, defined below: 

```c
MUDEF void mu_free_WAVE_profile(muWAVEProfile* profile);
```


The struct `muWAVEProfile` represents the audio file profile of a WAVE file, and has the following members:

* `uint16_m format_tag` - the value of wFormatTag in fmt's common-fields; the format category.

* `uint16_m channels` - the value of wChannels in fmt's common-fields; the number of channels.

* `uint32_m samples_per_sec` - the value of dwSamplesPerSec in fmt's common-fields; the sampling rate.

* `uint32_m avg_bytes_per_sec` - the value of dwAvgBytesPerSec in fmt's common-fields; used for buffer estimation.

* `uint16_m block_align` - the value of wBlockAlign in fmt's common-fields; the data block size.

* `muWAVEFormatSpecificFields specific_fields` - any [WAVE format-specific information in the format-specific-fields portion](#wave-format-specific-fields) of fmt, corresponding to wFormatTag.

* `muWAVEChunks chunks` - the [index locations of each chunk](#wave-chunks) within the WAVE file.

### WAVE format specific fields

The union `muWAVEFormatSpecificFields` represents any data specific to a value for wFormatTag in the format-specific-fields portion of the fmt chunk. It has the following members:

* `muWAVEPCM* wave_pcm` - the format-specific-fields data for the [WAVE PCM format](#wave-pcm-format).

### WAVE formats

This section lists every supported WAVE format, including a macro for its supported wFormatTag value in WAVE's specification, and its [format-specific-fields data](#wave-format-specific-fields) (if any).

#### WAVE PCM format

The WAVE PCM format represents the wFormatTag value WAVE_FORMAT_PCM, Microsoft's pulse code modulation format. Its macro is `MU_WAVE_FORMAT_PCM`, which is defined as the value `0x0001`.

This format does store data in the [format-specific-fields portion of the fmt chunk](#wave-format-specific-fields). This data is represented by the struct `muWAVEPCM`, which has the following member:

* `uint16_m bits_per_sample` - the value of wBitsPerSample in format-specific-fields's PCM-format-specific form; the sample size.

### WAVE format names

The name function `mu_WAVE_format_get_name` returns a `const char*` representation of a given WAVE audio format (for example, `MU_WAVE_FORMAT_PCM` returns "MU_WAVE_FORMAT_PCM"), defined below: 

```c
MUDEF const char* mu_WAVE_format_get_name(uint16_m format_tag);
```


This function returns "MU_UNKNOWN" in the case that `format_tag` is an unrecognized value.

> This function is a "name" function, and therefore is only defined if `MUAF_NAMES` is also defined.

### WAVE chunks

The struct `muWAVEChunks` stores the index location of known chunks within the WAVE file. It has the following members:

* `size_m fmt` - the fmt-ck chunk.

* `uint32_m fmt_len` - the recorded length of the fmt-ck chunk.

* `size_m wave` - the wave-data chunk.

* `uint32_m wave_len` - the recorded length of the wave-data chunk.

* `size_m fact` - the fact-ck chunk.

* `uint32_m fact_len` - the recorded length of the fact-ck chunk.

* `size_m cue` - the cue-ck chunk.

* `uint32_m cue_len` - the recorded length of the cue-ck chunk.

* `size_m playlist` - the playlist-ck chunk.

* `uint32_m playlist_len` - the recorded length of the playlist-ck chunk.

* `size_m assoc_data` - the assoc-data-list chunk.

* `uint32_m assoc_data_len` - the recorded length of the assoc-data-list chunk.

If one of the index values defined above is set to 0, that indicates that the chunk doesn't appear within the WAVE file. If it is not set to 0, the chunk does appear within the WAVE file, and the index value indicates where in the file, starting at 0 for the first byte within the file and on.

The index values do *not* reference the index of the chunk (which would start at its ckID), but instead reference where the actual data for the chunk starts (which starts at ckData); the index values are an index to the chunk's ckData, not the chunk's ckID. This includes chunks that are wrapped in a LIST; chunks wrapped in a list (such as the assoc-data-list chunk) have their indexes and lengths referencing the LIST chunk itself.

A specification-compliant WAVE file will always have the chunks fmt-ck and wave-data. Although the specification does state that fmt-ck has to be defined before wave-data, muaf ignores this, and can successfully load files that don't follow this. The ignoring of this rule only applies when muaf is *reading* from a WAVE file; muaf follows this (and all other rules laid out in the specification) when *writing* a WAVE file.

Additionally, WAVE's specifications list chunks in a specific order (fmt-ck, fact-ck, cue-ck, playlist-ck, assoc-data-list, wave-data), but I am unaware if they necessarily *need* to be in this order (besides the previously mentioned rule that fmt-ck occurs before wave-data), so muaf permits WAVE files to have any chunks in any order, as long as fmt-ck and wave-data appear at some point. muaf writes WAVE files in the specified order, though.

## WAVE known bugs and limitations

This section covers the known bugs and limitations for the implementation of WAVE in muaf.

### Support for LIST wave-data

muaf does not currently support wave-data that's provided in the form of a LIST. It only supports wave-data in the form of a data-ck chunk.

### Support for wBitsPerSample

muaf does not currently support values for wBitsPerSample that are not divisible by 8.

# Result

The type `muafResult` (typedef for `uint32_m`) is defined to represent how a task went. Result values can be "fatal" (meaning that the task completely failed to execute, and the program will continue as if the task had never been attempted), "non-fatal" (meaning that the task partially failed, but was still able to complete the task), and "successful" (meaning that the task fully succeeded as intended).

## Result values

The following values are defined for `muafResult` (all values not explicitly stated as being fatal, non-fatal, or successful are assumed to be fatal).

### General result values

* `MUAF_SUCCESS` - the task was successfully completed; real value 0.

* `MUAF_FAILED_MALLOC` - a vital call to malloc failed.

* `MUAF_FAILED_OPEN_FILE` - an attempt to open the file failed.

### WAVE result values

* `MUAF_INVALID_WAVE_CHUNK_LENGTH_FOR_FILE` - a WAVE chunk has a recorded length that is out of range for the file's actual length.

* `MUAF_INVALID_WAVE_MISSING_FMT` - the required WAVE chunk fmt-ck was not found.

* `MUAF_INVALID_WAVE_MISSING_WAVE_DATA` - the required WAVE chunk wave-data was not found.

* `MUAF_INVALID_WAVE_FMT_LENGTH` - the WAVE chunk fmt-ck has an invalid recorded length.

* `MUAF_INVALID_WAVE_FMT_CHANNELS` - the WAVE chunk fmt-ck's value 'wChannels' has an invalid value of 0.

* `MUAF_INVALID_WAVE_FMT_SAMPLES_PER_SEC` - the WAVE chunk fmt-ck's value 'dwSamplesPerSec' has an invalid value of 0.

* `MUAF_INVALID_WAVE_FMT_PCM_BITS_PER_SAMPLE` - the WAVE chunk fmt-ck's PCM-format-specific value 'wBitsPerSample' has an invalid value; it's rather equal to 0, non-divisible by 8 without a remainder, or doesn't align with the value for wBlockAlign.

* `MUAF_FAILED_WAVE_IDENTIFICATION` - an attempt to retrieve WAVE information from a file that doesn't appear to be a WAVE file has occurred.

## Check if result is fatal

The function `muaf_result_is_fatal` returns whether or not a given `muafResult` value is fatal, defined below: 

```c
MUDEF muBool muaf_result_is_fatal(muafResult result);
```


This function returns `MU_TRUE` if the value of `result` is invalid/unrecognized.

## Result name

The function `muaf_result_get_name` returns a `const char*` representation of a given result value (for example, `MUAF_SUCCESS` returns "MUAF_SUCCESS"), defined below: 

```c
MUDEF const char* muaf_result_get_name(muafResult result);
```


The function returns "MU_UNKNOWN" in the case that `result` is an invalid result value.

> This function is a "name" function, and therefore is only defined if `MUAF_NAMES` is also defined.

# C standard library dependencies

muaf has several C standard library dependencies, all of which are overridable by defining them before the inclusion of its header. The following is a list of those dependencies.

## `stdio.h` dependencies

* `FILE_M` - equivalent to `FILE`.

* `mu_fopen` - equivalent to `fopen`.

* `mu_fclose` - equivalent to `fclose`.

* `mu_fread` - equivalent to `fread`.

* `mu_fseek` - equivalent to `fseek`.

* `MU_SEEK_END` - equivalent to `SEEK_END`.

* `MU_SEEK_SET` - equivalent to `SEEK_SET`.

* `mu_ftell` - equivalent to `ftell`.

## `stdlib.h` dependencies

* `mu_malloc` - equivalent to `malloc`.

* `mu_free` - equivalent to `free`.

## `string.h` dependencies

* `mu_memset` - equivalent to `memset`.
