

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

The demos use other files to operate correctly when running as a compiled executable. These other files can be found in the `resources` folder within `demos`, and this folder is expected to be in the same location that the program is executing from. For exmaple, if a user compiles a demo into `main.exe`, and decides to run it, the `resources` folder from `demos` should be in the same directory as `main.exe`.

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

# Version

The macros `MUAF_VERSION_MAJOR`, `MUAF_VERSION_MINOR`, and `MUAF_VERSION_PATCH` are defined to match its respective release version, following the format of `MAJOR.MINOR.PATCH`.

# Audio file formats

The type `muafFileFormat` (typedef for `uint8_m`) represents a file format supported in muaf, and has the following defined values:

* `MUAF_UNKNOWN` - an unknown and/or unrecognized file format.

* `MUAF_WAVE` - the [WAVE file format](#wave-api).

* `MUAF_FLAC` - the [FLAC file format](#flac-api).

## Audio file format names

The name function `muaf_audio_file_format_get_name` returns a `const char*` representation of a given audio file format (for example, `MUAF_WAVE` returns "MUAF_WAVE"), defined below: 

```c
MUDEF const char* muaf_audio_file_format_get_name(muafFileFormat format);
```


This function returns "MUAF_UNKNOWN" in the case that `format` is an unrecognized value.

> This function is a "name" function, and therefore is only defined if `MUAF_NAMES` is also defined.

## Audio file format nice names

The name function `muaf_audio_file_format_get_nice_name` returns a presentable `const char*` representation of a given audio file format (for example, `MUAF_WAVE` returns "WAVE (.wav, .wave)"), defined below: 

```c
MUDEF const char* muaf_audio_file_format_get_nice_name(muafFileFormat format);
```


This function returns "Unknown" in the case that `format` is an unrecognized value.

> This function is a "name" function, and therefore is only defined if `MUAF_NAMES` is also defined.

## Get audio file format

The function `mu_audio_file_format` retrieves the [audio file format](#audio-file-formats) of a given file, defined below: 

```c
MUDEF muafFileFormat mu_audio_file_format(const char* filename);
```


This function does not use the filename extension to identify the audio file format, but instead, the actual contents of the file itself. This function returns `MUAF_UNKNOWN` if rather the audio file format could not be identified to be anything supported in muaf, or muaf failed to retrieve the file's data.

# Audio formats

The type `muafAudioFormat` (typedef for `uint32_m`) represents a decompressed audio format supported for reading and writing in muaf. Each audio format has a corresponding type that represents how each decompressed sample is stored.

The type `muafAudioFormat` has the following defined values:

* `MUAF_FORMAT_UNKNOWN` - unknown or unsupported audio format.

* `MUAF_FORMAT_PCM_U8` - unsigned 8-bit PCM (range 0 to 255, 0x00 to 0xFF). Corresponding type is `uint8_m`. Supported by:

    * WAVE.

* `MUAF_FORMAT_PCM_S8` - signed 8-bit PCM (range -128 to 127, -0x80 to 0x7F). Corresponding type is `int8_m`. Supported by:

    * FLAC.

* `MUAF_FORMAT_PCM_S16` - signed 16-bit PCM (range -32768 to 32767, -0x8000 to 0x7FFF). Corresponding type is `int16_m`. Supported by:

    * WAVE.

    * FLAC.

* `MUAF_FORMAT_PCM_S24` - signed 24-bit PCM (range -8388608 to 8388607, -0x800000 to 0x7FFFFF). Corresponding type is `int32_m`. Supported by:

    * WAVE.

    * FLAC.

* `MUAF_FORMAT_PCM_S32` - signed 32-bit PCM (range -2147483648 to 2147483647, -0x80000000 to 0x7FFFFFFF). Corresponding type is `int32_m`. Supported by:

    * WAVE.

    * FLAC.

* `MUAF_FORMAT_PCM_S64` - signed 64-bit PCM (range -9223372036854775808 to 9223372036854775807, -0x8000000000000000 to 0x7FFFFFFFFFFFFFFF). Corresponding type is `int64_m`. Supported by:

    * WAVE.

## Audio format names

The name function `muaf_audio_format_get_name` returns a `const char*` representation of a given audio format (for example, `MUAF_FORMAT_PCM_U8` returns "MUAF_FORMAT_PCM_U8"), defined below: 

```c
MUDEF const char* muaf_audio_format_get_name(muafAudioFormat format);
```


This function returns "MUAF_FORMAT_UNKNOWN" in the case that `format` is an unrecognized value.

> This function is a "name" function, and therefore is only defined if `MUAF_NAMES` is also defined.

## Audio format nice names

The name function `muaf_audio_format_get_nice_name` returns a presentable `const char*` representation of a given audio format (for example, `MUAF_FORMAT_PCM_U8` returns "8-bit unsigned PCM"), defined below: 

```c
MUDEF const char* muaf_audio_format_get_nice_name(muafAudioFormat format);
```


This function returns "Unknown" in the case that `format` is an unrecognized value.

> This function is a "name" function, and therefore is only defined if `MUAF_NAMES` is also defined.

## Get audio format support

The function `muaf_audio_format_supported` returns whether or not a given audio format is compatible for reading and writing with a given audio file format, defined below: 

```c
MUDEF muBool muaf_audio_format_supported(muafFileFormat file_format, muafAudioFormat audio_format);
```


This function returns `MU_FALSE` if the given file format or audio format is unrecognized.

## Get audio format sample size

The function `muaf_audio_format_sample_size` returns the size of an audio format's corresponding type, defined below: 

```c
MUDEF size_m muaf_audio_format_sample_size(muafAudioFormat format);
```


If the given format is unrecognized, this function returns 0.

## Is audio format PCM

The macro function `MUAF_FORMAT_IS_PCM` takes in a format, and forms an expression that represents whether or not the given format is a PCM format, defined below: 

```c
#define MUAF_FORMAT_IS_PCM(format) (format >= MUAF_FORMAT_PCM_U8 && format <= MUAF_FORMAT_PCM_S64)
```


# WAVE API

This section describes muaf's API for the [Waveform Audio File Format](https://en.wikipedia.org/wiki/WAV), or WAVE. The code for this API is built based off of the original August 1991 specification for WAVE (specifically [this archive](https://www.mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/Docs/riffmci.pdf)), and this section of muaf's documentation will reference concepts that are defined in this specification.

## Reading WAVE audio data

This section covers the functionality for reading WAVE audio data.

### Read PCM WAVE data

The function `mu_read_WAVE_PCM` reads frames from a WAVE file encoded in PCM, defined below: 

```c
MUDEF muafResult mu_read_WAVE_PCM(const char* filename, muWAVEProfile* profile, uint32_m beg_frame, uint32_m frame_len, void* data);
```


The given and already loaded [WAVE profile](#wave-profile) must have a [supported audio format](#audio-formats), and said audio format [must be PCM](#is-audio-format-pcm). The given frame range must be valid for the given WAVE file, and the given data must be large enough to hold the requested amount of frames in the audio format's corresponding type.

### Get WAVE audio format

The function `mu_get_WAVE_audio_format` returns the [audio format](#audio-formats) that a WAVE file is encoded in, defined below: 

```c
MUDEF muafAudioFormat mu_get_WAVE_audio_format(muWAVEProfile* profile);
```


If the given and already loaded [WAVE profile](#wave-profile) does not have a [supported audio format](#audio-formats) equivalent, this function returns `MUAF_FORMAT_UNKNOWN`.

## Writing WAVE audio data

This section covers the functionality for writing WAVE audio data.

### Write PCM WAVE data

The function `mu_write_WAVE_PCM` writes frames to a WAVE file encoded in PCM, defined below: 

```c
MUDEF muafResult mu_write_WAVE_PCM(const char* filename, muWAVEWrapper* wrapper, uint32_m beg_frame, uint32_m frame_len, void* data);
```


The given and already created [WAVE wrapper](#wave-wrapper) must have [a PCM audio format](#is-audio-format-pcm). The given frame range must be valid for the given WAVE wrapper, and the given data must hold the amount of frames specified in the audio format's corresponding type.

The data passed into this function may be changed by this function.

This function can be called on any valid frame range any number of times in any order.

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

* `uint32_m samples_per_sec` - the value of dwSamplesPerSec in fmt's common-fields; the sample rate.

* `uint32_m avg_bytes_per_sec` - the value of dwAvgBytesPerSec in fmt's common-fields; used for buffer estimation.

* `uint16_m block_align` - the value of wBlockAlign in fmt's common-fields; the data block size.

* `muWAVEFormatSpecificFields specific_fields` - any [WAVE format-specific information in the format-specific-fields portion](#wave-format-specific-fields) of fmt, corresponding to wFormatTag.

* `muWAVEChunks chunks` - the [index locations of each chunk](#wave-chunks) within the WAVE file.

## WAVE formats

This section lists every supported WAVE format, including a macro for its supported wFormatTag value in WAVE's specification, and its [format-specific-fields data](#wave-format-specific-fields) (if any).

### WAVE PCM format

The WAVE PCM format represents the wFormatTag value WAVE_FORMAT_PCM, Microsoft's pulse code modulation format. Its macro is `MU_WAVE_FORMAT_PCM`, which is defined as the value `0x0001`.

This format does store data in the [format-specific-fields portion of the fmt chunk](#wave-format-specific-fields). This data is represented by the struct `muWAVEPCM`, which has the following member:

* `uint16_m bits_per_sample` - the value of wBitsPerSample in format-specific-fields's PCM-format-specific form; the size of each sample, in bits.

## WAVE format specific fields

The union `muWAVEFormatSpecificFields` represents any data specific to a value for wFormatTag in the format-specific-fields portion of the fmt chunk. It has the following members:

* `muWAVEPCM* wave_pcm` - the format-specific-fields data for the [WAVE PCM format](#wave-pcm-format).

## WAVE chunks

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

## WAVE wrapper

The function `mu_create_WAVE_wrapper` creates a WAVE file based on the given WAVE wrapper information, defined below: 

```c
MUDEF muafResult mu_create_WAVE_wrapper(const char* filename, muWAVEWrapper* wrapper);
```


The function `mu_free_WAVE_wrapper` frees any manually allocated data that might have been generated from its call to `mu_create_WAVE_wrapper`, defined below: 

```c
MUDEF void mu_free_WAVE_wrapper(muWAVEWrapper* wrapper);
```


Upon a successful/non-fatal call to `mu_create_WAVE_wrapper`, the WAVE file is properly encoded, although with all of the audio data initialized to 0.

The struct `muWAVEWrapper` represents a WAVE file wrapper, and has the following members:

* `muafAudioFormat audio_format` - the [audio format](#audio-formats) of the WAVE file.

* `uint32_m num_frames` - the amount of frames in the WAVE file.

* `uint16_m num_channels` - the number of channels.

* `uint32_m sample_rate` - the amount of samples that should be played every second per channel.

* `muWAVEChunks chunks` - the location of the chunks in the audio file. This is used internally, and should not be filled in by the user.

### Get WAVE wrapper from WAVE file

The function `mu_get_WAVE_wrapper_from_WAVE` fills in information for a WAVE wrapper based on the contents of another WAVE file, defined below: 

```c
MUDEF muafResult mu_get_WAVE_wrapper_from_WAVE(muWAVEProfile* profile, muWAVEWrapper* wrapper);
```


The given profile must already be loaded, and have a [supported audio format](#audio-formats).

## WAVE known bugs and limitations

This section covers the known bugs and limitations for the implementation of WAVE in muaf.

### Support for LIST wave-data

muaf does not currently support wave-data that's provided in the form of a LIST. It only supports wave-data in the form of a data-ck chunk.

# FLAC API

## FLAC profile

A FLAC file's profile can be retrieved with the function `mu_get_FLAC_profile`, defined below: 

```c
MUDEF muafResult mu_get_FLAC_profile(const char* filename, muFLACProfile* profile);
```


Once retrieved, the profile must be deallocated at some point using the functino `mu_free_FLAC_profile`, defined below: 

```c
MUDEF void mu_free_FLAC_profile(muFLACProfile* profile);
```


The struct `muFLACProfile` represents the audio file profile of a FLAC file, and has the following members:

* `muBool contains_audio` - whether or not the given FLAC file has any audio data stored in it.

* `uint16_m min_block_size` - the first value in the streaminfo metadata block; "The minimum block size (in samples) used in the stream, excluding the last block."

* `uint16_m max_block_size` - the second value in the streaminfo metadata block; "The maximum block size (in samples) used in the stream."

* `uint32_m min_frame_size` - the third value in the streaminfo metadata block; "The minimum frame size (in bytes) used in the stream."

* `uint32_m max_frame_size` - the fourth value in the streaminfo metadata block; "The maximum frame size (in bytes) used in the stream."

* `uint32_m sample_rate` - the fifth value in the streaminfo metadata block; "Sample rate in Hz."

* `uint8_m num_channels` - the sixth value in the streaminfo metadata block; "(number of channels)-1."

* `uint8_m bits_per_sample` - the seventh value in the streaminfo metadata block; "	(bits per sample)-1."

* `uint64_m num_samples` - the eight value in the streaminfo metadata block; "Total number of interchannel samples in the stream."

* `uint64_m low_checksum` - the low bytes of the ninth value in the streaminfo metadata block; low bytes of "MD5 checksum of the unencoded audio data."

* `uint64_m high_checksum` - the high bytes of the ninth value in the streaminfo metadata block; high bytes of "MD5 checksum of the unencoded audio data."

* `size_m num_metadata_blocks` - the number of metadata blocks in the FLAC file (excluding streaminfo).

* `muFLACMetadataBlock* metadata_blocks` - the [metadata blocks](#flac-metadata-blocks) in the FLAC file (excluding streaminfo).

## FLAC metadata blocks

The struct `muFLACMetadataBlock` represents a metadata block in a FLAC file. It has the following members:

* `uint8_m block_type` - the [metadata block type](#flac-metadata-block-types).

* `uint32_m length` - the recorded length of the metadata block.

* `size_m index` - the index location of the metadata block data, starting at 0 for the first byte in the file.

### FLAC metadata block types

The following recognized metadata block type values for FLAC in muaf are defined:

* `MU_FLAC_METADATA_PADDING` - "Padding" (real value 1).

* `MU_FLAC_METADATA_APPLICATION` - "Application" (real value 2).

* `MU_FLAC_METADATA_SEEK_TABLE` - "Seek table" (real value 3).

* `MU_FLAC_METADATA_VORBIS_COMMENT` - "Vorbis comment" (real value 4).

* `MU_FLAC_METADATA_CUESHEET` - "Cuesheet" (real value 5).

* `MU_FLAC_METADATA_PICTURE` - "Picture" (real value 6).

Metadata blocks that don't have any of these block types are still loaded by muaf (besides any forbidden values), and muaf does not provide a built-in way to read the values from all of these block types; these defined values are provided purely for convenience, and are copied directly from the specification.

# Result

The type `muafResult` (typedef for `uint32_m`) is defined to represent how a task went. Result values can be "fatal" (meaning that the task completely failed to execute, and the program will continue as if the task had never been attempted), "non-fatal" (meaning that the task partially failed, but was still able to complete the task), and "successful" (meaning that the task fully succeeded as intended).

## Result values

The following values are defined for `muafResult` (all values not explicitly stated as being fatal, non-fatal, or successful are assumed to be fatal).

### General result values

* `MUAF_SUCCESS` - the task was successfully completed; real value 0.

* `MUAF_FAILED_MALLOC` - a vital call to `malloc` failed.

* `MUAF_FAILED_OPEN_FILE` - an attempt to open the file failed.

* `MUAF_FAILED_AUDIO_FILE_FORMAT_IDENTIFICATION` - an attempt to retrieve/write information from/to an audio file failed, as the assumed file format was not the case (such as trying to get the profile of a WAVE file using `mu_get_WAVE_profile` when the file in question doesn't appear to actually be a WAVE file).

* `MUAF_FAILED_UNSUPPORTED_AUDIO_FORMAT` - the file is in an audio format that muaf does not support for the given task.

* `MUAF_FAILED_CREATE_FILE` - an attempt to create the file failed.

* `MUAF_FAILED_REALLOC` - a vital call to `realloc` failed.

### WAVE result values

* `MUAF_INVALID_WAVE_CHUNK_LENGTH_FOR_FILE` - a WAVE chunk has a recorded length that is out of range for the file's actual length.

* `MUAF_INVALID_WAVE_MISSING_FMT` - the required WAVE chunk fmt-ck was not found.

* `MUAF_INVALID_WAVE_MISSING_WAVE_DATA` - the required WAVE chunk wave-data was not found.

* `MUAF_INVALID_WAVE_FMT_LENGTH` - the WAVE chunk fmt-ck has an invalid recorded length.

* `MUAF_INVALID_WAVE_FMT_PCM_BITS_PER_SAMPLE` - the WAVE chunk fmt-ck's PCM-format-specific value 'wBitsPerSample' has an invalid value; it's rather equal to 0, non-divisible by 8 without a remainder, doesn't evenly divide the length of the wave data, or doesn't align with the value for wBlockAlign.

* `MUAF_INVALID_WAVE_FMT_CHANNELS` - the WAVE chunk fmt-ck's value 'wChannels' has an invalid value of 0.

* `MUAF_INVALID_WAVE_FMT_SAMPLES_PER_SEC` - the WAVE chunk fmt-ck's value 'dwSamplesPerSec' has an invalid value of 0.

* `MUAF_INVALID_WAVE_FILE_WRITE_SIZE` - the WAVE file could not be created, as the size of the WAVE file would be over the maximum file size of a WAVE file due to any of the limitations of how big certain values can be encoded in WAVE (such as the ckSize for the RIFF chunk).

### FLAC result values

* `MUAF_INVALID_FLAC_STREAMINFO_LENGTH` - the streaminfo metadata block has an invalid recorded length (not 34 bytes).

* `MUAF_INVALID_FLAC_STREAMINFO_BLOCK_SIZE_RANGE` - the listed minimum or maximum block size within streaminfo was not within the required value range of 16 to 65535.

* `MUAF_INVALID_FLAC_STREAMINFO_BLOCK_SIZE_MIN_MAX` - the listed minimum and maximum block size within streaminfo do not make sense, as the maximum is smaller than the minimum.

* `MUAF_INVALID_FLAC_STREAMINFO_FRAME_SIZE_MIN_MAX` - the listed minimum and maximum frame size within streaminfo do not make sense, as the maximum is smaller than the minimum.

* `MUAF_INVALID_FLAC_STREAMINFO_BITS_PER_SAMPLE` - the listed bits per sample within streaminfo (after accounting for subtraction) is not within the permitted range of 4 to 32.

* `MUAF_INVALID_FLAC_STREAMINFO_SAMPLE_COUNT` - the listed amount of samples doesn't make sense, rather because the FLAC file doesn't contain audio and the sample count is over 0, or because the FLAC file does contain audio and the sample count is 0.

* `MUAF_INVALID_FLAC_METADATA_BLOCK_LENGTH` - rather the recorded length of a metadata block goes past the end of the file, or a metadata block's header was indicated despite there not being enough space left in the file to store another metadata block header.

* `MUAF_INVALID_FLAC_METADATA_BLOCK_TYPE` - the type of a metadata block type within the FLAC file was the forbidden value 127.

* `MUAF_INVALID_FLAC_DUPLICATE_STREAMINFO` - more than one streaminfo metadata block was identified.

* `MUAF_INVALID_FLAC_DUPLICATE_SEEK_TABLE` - more than one seek table metadata block was identified.

* `MUAF_INVALID_FLAC_DUPLICATE_VORBIS_COMMENT` - more than one vorbis comment metadata block was identified.

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

* `mu_fputc` - equivalent to `fputc`.

* `mu_fwrite` - equivalent to `fwrite`.

## `stdlib.h` dependencies

* `mu_malloc` - equivalent to `malloc`.

* `mu_free` - equivalent to `free`.

* `mu_realloc` - equivalent to `realloc`.

## `string.h` dependencies

* `mu_memset` - equivalent to `memset`.

* `mu_memcpy` - equivalent to `memcpy`.
