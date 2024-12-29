

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

# Terminology

This section provides a general overview of muaf's terminology in relation to audio and how audio data is handled in muaf, since a significant portion of audio terminology is not agreed upon.

The smallest unit of audio data is a '***sample***', which represents a single point of audio signal amplitude at one moment in time. In an audio file, multiple samples can be provided for the same moment in time in the form of '***channels***', which can be used for [stereo audio](https://en.wikipedia.org/wiki/Stereophonic_sound) and such. A '***frame***' contains each sample per channel, interleaved one after the other. So, if a frame had two samples it in, the first sample would be for channel 0, and the second sample would be for channel 1.

Audio files are expected to be played at a certain integer '***sample rate***', indicating how many samples *per channel* should be played each second. Since the sample rate value is based on one channel, the sample rate does not increase along with the amount of channels; the sample rate can more accurately described as the 'frame rate', as it more concisely indicates the amount of frames per second, but since the term 'frame rate' is already commonly associated with [video frame rate](https://en.wikipedia.org/wiki/Frame_rate), and the term 'sample rate' is fairly standardized, muaf uses the term 'sample rate'.

An '***audio format***' refers to how audio data can be laid out in a manner recognized and supported by muaf. In an '***uncompressed***' audio format, frames are simply stored one after the other chronologically, and in a type whose sample values are directly readable as an integer or decimal value in C. In a '***compressed***' audio format, frames are *not* stored one after the other, but instead are organized into '***packets***', which can contain any number of '***explicit or implicit frames***'. If frames are explicitly listed in a packet, that means that all frames' values are listed out one after the other in an uncompressed manner within that packet. If frames are implicitly listed, that means that some frames' values are not directly laid out in the data, but are instead implied based on other values, and need to be worked out manually to retrieve any readable frame data.

An audio file's '***raw data***' refers to the raw audio data based on the audio format. This means that, for example, if the audio format is compressed, the raw data describes the frames as they're listed in the data, explicit or implicit, leaving it to the user to work out the sample values for the frames manually if they are implicit.

An audio file's '***decompressed data***' refers to uncompressed audio data, whether or not it was generated from compressed or uncompressed audio data. If the given audio data's format is uncompressed, then the decompressed data simply refers to the raw data. However, if the given audio data's format is compressed, then the decompressed data refers to the raw data's equivalent uncompressed audio data. In other words, the decompressed data refers to the decompressed version of the raw data (whether or not it was compressed in the first place).

The terminology listed above for muaf does not apply when muaf is referencing the specification of another audio file format. In those instances, the specification's terminology applies.

# Format-unspecific reading API

muaf's reading API is split into two sections: the format-specific API, and the format-unspecific API. The unspecific API wraps around the specific API, encapsulating the specific API's functionality, and allowing you to retrieve general information about an audio file without having to directly consider what specific audio file format it is.

## Audio profile

For each audio format, general information about the audio stored in the file can be retrieved. This general information is called the "profile". This profile is defined for several reasons:

* It gives the user a way to load general information about the audio file first, before taking more processing time and memory to load more significant portions of the data stored within the audio file.

* Having this information gives the user a way to load select portions of the audio data encoded in the file without ever having to load the whole audio file at once in memory, which can be useful or even necessary for larger audio files.

* It allows the API to internally map how the file is formatted and hold onto that information for later, which can make subsequent calls to load information from the audio file quicker, since information about the file's general structure is held onto across function calls.

In the format-specific API, the profile stores general information that very directly corresponds to the general information stored in the audio file itself. For example, [the WAVE profile](#wave-profile) stores information about what supported chunks are provided in the file and where they are. It also stores the information within certain chunks that provide general information about the audio, such as the fmt chunk, which provides information such as the number of channels.

In the format-unspecific API, the profile primarily stores general information that all of the supported audio file formats within muaf have in common. This allows the user to retrieve information about the audio file without having to know how to retrieve it from the exact audio file format that it's in. The unspecific profile also loads and stores the specific profile information.

An audio file's unspecific profile can be retrieved using the function `mu_get_audio_file_profile`, defined below: 

```c
MUDEF muafResult mu_get_audio_file_profile(const char* filename, muafUnspecificProfile* profile);
```


Once an unspecific profile has been successfully or non-fatally retrieved (indicated by [the result return value](#result)), the profile is filled with data, some of which may be manually allocated automatically. To free this data, use the function `mu_free_audio_file_profile`, defined below: 

```c
MUDEF void mu_free_audio_file_profile(muafUnspecificProfile* profile);
```


The unspecific profile is represented by the struct `muafUnspecificProfile`, which has the following members:

* `muafFrames num_frames` - the number of frames (`muafFrames` typedef for `uint64_m`).

* `muafSampleRate sample_rate` - the amount of samples that should be played every second per channel (`muafSampleRate` typedef for `uint32_m`).

* `muafChannels num_channels` - the number of channels (`muafChannels` typedef for `uint16_m`).

* `muafAudioFormat audio_format` - the [audio format](#audio-formats) (`muafAudioFormat` typedef for `uint32_m`).

* `muafFileFormat file_format` - the [file format](#audio-file-formats) (`muafFileFormat` typedef for `uint8_m`).

* `muafSpecificProfile specific` - the [format-specific profile](#format-specific-profile).

### Audio formats

The type `muafAudioFormat` (typedef for `uint32_m`) represents an audio format supported in muaf. Each audio format has a corresponding type that represents how each sample/packet is stored. If an audio format's corresponding type is an integer/decimal type, then the audio format is uncompressed, and vice versa.

The type `muafAudioFormat` has the following defined values:

* `MUAF_FORMAT_UNKNOWN` - unknown or unsupported audio format.

#### PCM audio formats

* `MUAF_FORMAT_PCM_U8` - unsigned 8-bit PCM (range 0 to 255, 0x00 to 0xFF). Corresponding type is `uint8_m`.

* `MUAF_FORMAT_PCM_S16` - signed 16-bit PCM (range -32768 to 32767, -0x8000 to 0x7FFF). Corresponding type is `int16_m`.

* `MUAF_FORMAT_PCM_S32` - signed 32-bit PCM (range -2147483648 to 2147483647, -0x80000000 to 0x7FFFFFFF). Corresponding type is `int32_m`.

* `MUAF_FORMAT_PCM_S64` - signed 64-bit PCM (range -9223372036854775808 to 9223372036854775807, -0x8000000000000000 to 0x7FFFFFFFFFFFFFFF). Corresponding type is `int64_m`.

#### Audio format names

The name function `muaf_audio_format_get_name` returns a `const char*` representation of a given audio format (for example, `MUAF_FORMAT_PCM_U8` returns "MUAF_FORMAT_PCM_U8"), defined below: 

```c
MUDEF const char* muaf_audio_format_get_name(muafAudioFormat format);
```


This function returns "MUAF_FORMAT_UNKNOWN" in the case that `format` is an unrecognized value.

> This function is a "name" function, and therefore is only defined if `MUAF_NAMES` is also defined.

#### Audio format nice names

The name function `muaf_audio_format_get_nice_name` returns a presentable `const char*` representation of a given audio format (for example, `MUAF_FORMAT_PCM_U8` returns "8-bit unsigned PCM"), defined below: 

```c
MUDEF const char* muaf_audio_format_get_nice_name(muafAudioFormat format);
```


This function returns "Unknown" in the case that `format` is an unrecognized value.

> This function is a "name" function, and therefore is only defined if `MUAF_NAMES` is also defined.

#### Get audio format compression

The function `muaf_audio_format_compressed` returns if a given audio format is compressed, defined below: 

```c
MUDEF muBool muaf_audio_format_compressed(muafAudioFormat format);
```


This function returns `MU_FALSE` if the given format is unrecognized.

#### Get audio format sample size

The function `muaf_audio_format_sample_size` returns the size of a single sample for an uncompressed audio format, defined below: 

```c
MUDEF size_m muaf_audio_format_sample_size(muafAudioFormat format);
```


If the given format is compressed or unrecognized, this function returns 0.

### Audio file formats

The type `muafFileFormat` (typedef for `uint8_m`) represents a file format supported in muaf, and has the following defined values:

* `MUAF_UNKNOWN` - an unknown and/or unrecognized file format.

* `MUAF_WAVE` - the [WAVE file format](#wave-api).

* `MUAF_FLAC` - the [FLAC file format](#flac-api).

#### Audio file format names

The name function `muaf_audio_file_format_get_name` returns a `const char*` representation of a given audio file format (for example, `MUAF_WAVE` returns "MUAF_WAVE"), defined below: 

```c
MUDEF const char* muaf_audio_file_format_get_name(muafFileFormat format);
```


This function returns "MUAF_UNKNOWN" in the case that `format` is an unrecognized value.

> This function is a "name" function, and therefore is only defined if `MUAF_NAMES` is also defined.

#### Audio file format nice names

The name function `muaf_audio_file_format_get_nice_name` returns a presentable `const char*` representation of a given audio file format (for example, `MUAF_WAVE` returns "WAVE (.wav, .wave)"), defined below: 

```c
MUDEF const char* muaf_audio_file_format_get_nice_name(muafFileFormat format);
```


This function returns "Unknown" in the case that `format` is an unrecognized value.

> This function is a "name" function, and therefore is only defined if `MUAF_NAMES` is also defined.

### Format specific profile

The union `muafSpecificProfile` acts as a container for the format-specific profile of some audio file, and has the following members:

* `muWAVEProfile* WAVE` - the [WAVE profile](#wave-profile).

## Get audio file format

The function `mu_audio_file_format` retrieves the [audio file format](#audio-file-formats) of a given file, defined below: 

```c
MUDEF muafFileFormat mu_audio_file_format(const char* filename);
```


This function does not use the filename extension to identify the audio file format, but instead, the actual contents of the file itself. This function returns `MUAF_UNKNOWN` if rather the audio file format could not be identified to be anything supported in muaf, or muaf failed to retrieve the file's data.

## Reading audio data

This section covers the functionality for reading audio data.

### Read uncompressed raw data

The function `mu_read_uncompressed_audio_file` reads an uncompressed audio file's raw data, defined below: 

```c
MUDEF muafResult mu_read_uncompressed_audio_file(const char* filename, muafUnspecificProfile* profile, muafFrames beg_frame, muafFrames frame_len, void* data);
```


The profile given must be a valid loaded profile from a file that has gone unchanged since the profile was initially loaded. The given audio file's audio format must be uncompressed. The frame range provided must be valid; this function does not check if the given frame range is valid. The given data must match the corresponding type of the profile's audio format (which itself must be a supported and defined value for `muafAudioFormat`), and `data` must be large enough to hold the requested amount of frames.

## Writing audio data

This section covers the functionality for writing audio data.

### Audio wrapper

muaf is designed to be able to write an audio file across multiple function calls. In particular, it is designed this way so that not all of the audio data needs to be readily available at one moment in memory in order for all of the audio to be successfully encoded into the audio file.

The primary way that muaf achieves this is by encapsulating writing audio data into a ***wrapper***, which describes an audio file that may be in the process of being written to. An audio file being written using a wrapper is only successfully fully encoded when the wrapper has been created and, afterwards, all of the audio frames have been written once and once only. Audio frames do not need to be written in any particular order.

The function `mu_create_audio_file_wrapper` creates an audio file with no audio encoded in it based on the given wrapper information, defined below: 

```c
MUDEF muafResult mu_create_audio_file_wrapper(const char* filename, muafUnspecificWrapper* wrapper);
```


Once an unspecific wrapper has been successfully or non-fatally retrieved, the wrapper is filled with data, some of which may be manually allocated automatically. To free this data, use the function `mu_free_audio_file_wrapper`, defined below: 

```c
MUDEF void mu_free_audio_file_wrapper(muafUnspecificWrapper* wrapper);
```


All of the parameters within `wrapper` meant to be set by the user should be set before calling this function. Once this function has successfully or non-fatally executed, the file will be created, but it is not guaranteed to be properly encoded until all audio frames have been written once and once only.

The struct `muafUnspecificWrapper` represents an unspecific audio file wrapper, and has the following members:

* `muafFrames num_frames` - the number of frames (`muafFrames` typedef for `uint64_m`).

* `muafSampleRate sample_rate` - the amount of samples that should be played every second per channel (`muafSampleRate` typedef for `uint32_m`).

* `muafChannels num_channels` - the number of channels (`muafChannels` typedef for `uint16_m`).

* `muafAudioFormat audio_format` - the [audio format](#audio-formats) (`muafAudioFormat` typedef for `uint32_m`).

* `muafFileFormat file_format` - the [file format](#audio-file-formats) (`muafFileFormat` typedef for `uint8_m`).

* `muafSpecificWrapper specific` - the [format-specific wrapper](#format-specific-wrapper). This is filled in automatically after a call to `mu_create_audio_file_wrapper`.

### Format specific wrapper

The union `muafSpecificWrapper` represents the wrapper of a particular audio file format, and has the following members:

* `muWAVEWrapper* WAVE` - the [WAVE wrapper](#wave-wrapper).

### Write raw audio frames

The function `mu_write_uncompressed_audio_file` writes frames to an uncompressed audio file, defined below: 

```c
MUDEF muafResult mu_write_uncompressed_audio_file(const char* filename, muafUnspecificWrapper* wrapper, muafFrames beg_frame, muafFrames frame_len, void* data);
```


The given wrapper must have its contents unaltered by the user since its original call to [`mu_create_audio_file_wrapper`](#audio-wrapper), and its audio format must be uncompressed. Once all audio frames have been written to the audio file once and once only, the file should be properly encoded.

The given data `data` may be altered during a call to this function, and may not be the same once the function has finished.

### Get wrapper from audio file

The function `mu_get_audio_file_wrapper` fills in information for an audio file wrapper based on the contents of another audio file, defined below: 

```c
MUDEF muafResult mu_get_audio_file_wrapper(muafUnspecificProfile* profile, muafUnspecificWrapper* wrapper);
```


The parameter `profile` should be [retrieved beforehand](#audio-profile) from an existing file, and the given audio file should have [a supported unspecific audio format equivalent](#wave-format-to-unspecific-audio-format).

The members of `wrapper` will be filled in based on the given audio file described by `profile`, except for any members that are set by the function [`mu_create_audio_file_wrapper`](#audio-wrapper).

# WAVE API

This section describes muaf's API for the [Waveform Audio File Format](https://en.wikipedia.org/wiki/WAV), or WAVE. The code for this API is built based off of the original August 1991 specification for WAVE (specifically [this archive](https://www.mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/Docs/riffmci.pdf)), and this section of muaf's documentation will reference concepts that are defined in this specification.

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

### WAVE format specific fields

The union `muWAVEFormatSpecificFields` represents any data specific to a value for wFormatTag in the format-specific-fields portion of the fmt chunk. It has the following members:

* `muWAVEPCM* wave_pcm` - the format-specific-fields data for the [WAVE PCM format](#wave-pcm-format).

### WAVE formats

This section lists every supported WAVE format, including a macro for its supported wFormatTag value in WAVE's specification, and its [format-specific-fields data](#wave-format-specific-fields) (if any).

#### WAVE PCM format

The WAVE PCM format represents the wFormatTag value WAVE_FORMAT_PCM, Microsoft's pulse code modulation format. Its macro is `MU_WAVE_FORMAT_PCM`, which is defined as the value `0x0001`.

This format does store data in the [format-specific-fields portion of the fmt chunk](#wave-format-specific-fields). This data is represented by the struct `muWAVEPCM`, which has the following member:

* `uint16_m bits_per_sample` - the value of wBitsPerSample in format-specific-fields's PCM-format-specific form; the size of each sample, in bits.

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

## Reading WAVE audio data

This section covers the functionality for reading WAVE audio data.

### Read uncompressed raw WAVE data

The function `mu_read_WAVE_uncompressed` reads an uncompressed WAVE file's raw data, defined below: 

```c
MUDEF muafResult mu_read_WAVE_uncompressed(const char* filename, muWAVEProfile* profile, muafFrames beg_frame, muafFrames frame_len, void* data);
```


Listed limitations from [the unspecific function to read raw audio frames](#read-uncompressed-raw-data) apply.

### WAVE format to unspecific audio format

The function `mu_WAVE_unspecific_audio_format` converts a WAVE audio format to its supported [unspecific audio format](#audio-formats) equivalent, which muaf can then generally work with, defined below: 

```c
MUDEF muafAudioFormat mu_WAVE_unspecific_audio_format(uint16_m format_tag, muWAVEFormatSpecificFields specific_fields);
```


If this function returns `MUAF_FORMAT_UNKNOWN`, then the given WAVE audio format information is unrecognized and/or unsupported, and muaf is unable to extract the data from it.

## Writing WAVE audio data

This section covers the functionality for writing WAVE audio data.

### WAVE wrapper

The function `mu_create_WAVE_wrapper` creates a WAVE file given WAVE wrapper information, defined below: 

```c
MUDEF muafResult mu_create_WAVE_wrapper(const char* filename, muWAVEWrapper* wrapper);
```


The function `mu_free_WAVE_wrapper` frees any manually allocated data that might have been generated from its call to `mu_create_WAVE_wrapper`, defined below: 

```c
MUDEF void mu_free_WAVE_wrapper(muWAVEWrapper* wrapper);
```


Listed limitations from [the unspecific functions to create/free a wrapper](#audio-wrapper) apply.

The struct `muWAVEWrapper` represents a WAVE file wrapper, and has the following members:

* `muafAudioFormat audio_format` - the [audio format](#audio-formats) of the WAVE file.

* `uint32_m num_frames` - the amount of frames in the WAVE file.

* `uint16_m num_channels` - the number of channels.

* `uint32_m sample_rate` - the amount of samples that should be played every second per channel.

* `muWAVEChunks chunks` - the location of the chunks in the audio file. This is used internally, and should not be filled in by the user.

### Write raw WAVE audio frames

The function `mu_write_WAVE_uncompressed` writes frames to an uncompressed WAVE file, defined below: 

```c
MUDEF muafResult mu_write_WAVE_uncompressed(const char* filename, muWAVEWrapper* wrapper, muafFrames beg_frame, muafFrames frame_len, void* data);
```


Listed limitations from [the unspecific function to write raw audio frames](#write-raw-audio-frames) apply.

### Get WAVE wrapper from audio file

The function `mu_get_WAVE_wrapper` fills in information for a WAVE wrapper based on the contents of another WAVE file, defined below: 

```c
MUDEF muafResult mu_get_WAVE_wrapper(muWAVEProfile* profile, muWAVEWrapper* wrapper);
```


Listed limitations from [the unspecific function to get a wrapper from an audio file](#get-wrapper-from-audio-file) apply.

## WAVE known bugs and limitations

This section covers the known bugs and limitations for the implementation of WAVE in muaf.

### Support for LIST wave-data

muaf does not currently support wave-data that's provided in the form of a LIST. It only supports wave-data in the form of a data-ck chunk.

### Support for wBitsPerSample

muaf does not currently support values for wBitsPerSample that are not divisible by 8.

# FLAC API

This section describes muaf's API for the [Free Lossless Audio Codec](https://en.wikipedia.org/wiki/FLAC), or FLAC. The code for this API is built based off of [the RFC 9639 specification](https://datatracker.ietf.org/doc/rfc9639/), and this section of muaf's documentation will reference concepts that are defined in this specification. Any quotes referenced in this section are from RFC 9639 unless it is stated otherwise.

All values provided by the FLAC API by retrieving information from a FLAC audio file are checked and strictly guaranteed to be values permitted by the specification unless it is stated otherwise. These limitations are also strictly followed when encoding with no exceptions.

## FLAC profile

A FLAC file's profile can be retrieved with the function `mu_get_FLAC_profile`, defined below: 

```c
MUDEF muafResult mu_get_FLAC_profile(const char* filename, muFLACProfile* profile);
```


Once retrieved, the profile must be deallocated at some point using the function `mu_free_FLAC_profile`, defined below: 

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

# Result

The type `muafResult` (typedef for `uint32_m`) is defined to represent how a task went. Result values can be "fatal" (meaning that the task completely failed to execute, and the program will continue as if the task had never been attempted), "non-fatal" (meaning that the task partially failed, but was still able to complete the task), and "successful" (meaning that the task fully succeeded as intended).

## Result values

The following values are defined for `muafResult` (all values not explicitly stated as being fatal, non-fatal, or successful are assumed to be fatal).

### General result values

* `MUAF_SUCCESS` - the task was successfully completed; real value 0.

* `MUAF_FAILED_MALLOC` - a vital call to malloc failed.

* `MUAF_FAILED_OPEN_FILE` - an attempt to open the file failed.

* `MUAF_FAILED_UNSUPPORTED_AUDIO_FORMAT` - the task could not be completed due to the file being in an audio format that muaf does not support.

* `MUAF_FAILED_CREATE_FILE` - an attempt to create the file failed.

* `MUAF_FAILED_UNSUPPORTED_AUDIO_FILE_FORMAT` - the task could not be completed due to the file being in an audio file format (or the user providing an audio file format value) that muaf does not support or could not recognize.

* `MUAF_FAILED_AUDIO_FILE_FORMAT_IDENTIFICATION` - an attempt to retrieve/write information from/to an audio file failed, as the assumed file format was not the case (such as trying to get the profile of a WAVE file using `mu_get_WAVE_profile` when the file in question doesn't appear to actually be a WAVE file).

### WAVE result values

* `MUAF_INVALID_WAVE_CHUNK_LENGTH_FOR_FILE` - a WAVE chunk has a recorded length that is out of range for the file's actual length.

* `MUAF_INVALID_WAVE_MISSING_FMT` - the required WAVE chunk fmt-ck was not found.

* `MUAF_INVALID_WAVE_MISSING_WAVE_DATA` - the required WAVE chunk wave-data was not found.

* `MUAF_INVALID_WAVE_FMT_LENGTH` - the WAVE chunk fmt-ck has an invalid recorded length.

* `MUAF_INVALID_WAVE_FMT_CHANNELS` - the WAVE chunk fmt-ck's value 'wChannels' has an invalid value of 0.

* `MUAF_INVALID_WAVE_FMT_SAMPLES_PER_SEC` - the WAVE chunk fmt-ck's value 'dwSamplesPerSec' has an invalid value of 0.

* `MUAF_INVALID_WAVE_FMT_PCM_BITS_PER_SAMPLE` - the WAVE chunk fmt-ck's PCM-format-specific value 'wBitsPerSample' has an invalid value; it's rather equal to 0, non-divisible by 8 without a remainder, doesn't evenly divide the length of the wave data, or doesn't align with the value for wBlockAlign.

* `MUAF_INVALID_WAVE_FILE_WRITE_SIZE` - the WAVE file could not be created, as the size of the WAVE file would be over the maximum file size of a WAVE file due to any of the limitations of how big certain values can be encoded in WAVE (such as the ckSize for the RIFF chunk).

### FLAC result values

* `MUAF_INVALID_FLAC_STREAMINFO_LENGTH` - the streaminfo metadata block has an invalid recorded length (not 34 bytes).

* `MUAF_INVALID_FLAC_STREAMINFO_BLOCK_SIZE_RANGE` - the listed minimum or maximum block size within streaminfo was not within the required value range of 16 to 65535.

* `MUAF_INVALID_FLAC_STREAMINFO_BLOCK_SIZE_MIN_MAX` - the listed minimum and maximum block size within streaminfo do not make sense, as the maximum is smaller than the minimum.

* `MUAF_INVALID_FLAC_STREAMINFO_FRAME_SIZE_MIN_MAX` - the listed minimum and maximum frame size within streaminfo do not make sense, as the maximum is smaller than the minimum.

* `MUAF_INVALID_FLAC_STREAMINFO_NUM_CHANNELS` - the listed number of channels within streaminfo (after accounting for subtraction) is not within the permitted range of 1 to 8. This value is permitted to be zero (after accounting for subtraction) if no audio is being stored.

* `MUAF_INVALID_FLAC_STREAMINFO_BITS_PER_SAMPLE` - the listed bits per sample within streaminfo (after accounting for subtraction) is not within the permitted range of 4 to 32. This value is permitted to be zero (after accounting for subtraction) if no audio is being stored.

* `MUAF_INVALID_FLAC_STREAMINFO_SAMPLE_COUNT` - the listed amount of samples doesn't make sense, rather because the FLAC file doesn't contain audio and the sample count is over 0, or because the FLAC file does contain audio and the sample count is 0.

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

## `string.h` dependencies

* `mu_memset` - equivalent to `memset`.

* `mu_memcpy` - equivalent to `memcpy`.
