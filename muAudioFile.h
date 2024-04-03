/*
muAudioFile.h - Muukid
Public domain single-file C library for reading and writing audio files.
No warranty implied; use at your own risk.

Licensed under MIT License or public domain, whichever you prefer.
More explicit license information at the end of file.

@MENTION mu_audio_file_get_format can guess based on inner file content signatures. Also, signature
checks will not be performed on excluded file formats, but extension checks will.
@MENTION MUAF_READ_CALL_FAILED is called for unexpected EOF usually.
@MENTION MU_AUDIO_DATA_UNKNOWN is valid to have as the 'data_type' member for muAudioFileInfo, as
it indicates that the data used for each chunk can differ.
@MENTION MUAF_WAV_MAX_CHUNK_SAMPLE_COUNT
@MENTION Pretty please do not change the contents of the muAudioFileInfo struct over multiple
function calls. Pretty please.
@MENTION 'mu_audio_chunk_write_init' wipes contents.

@TODO Add more upfront info to muAudioFileInfo.
@TODO Separate byte manipulation library.
@TODO Don't forget padding byte for wav data.
*/

/* muu header commit 96a78ce */

#ifndef MUU_H
	#define MUU_H
	
	#if !defined(MU_SECURE_WARNINGS) && !defined(_CRT_SECURE_NO_WARNINGS)
		#define _CRT_SECURE_NO_WARNINGS
	#endif

	#ifdef __cplusplus
	extern "C" { // }
	#endif

	#define MUU_VERSION_MAJOR 1
	#define MUU_VERSION_MINOR 0
	#define MUU_VERSION_PATCH 0

	/* C standard library dependencies */

		#if !defined(int8_m)   || \
			!defined(uint8_m)  || \
			!defined(int16_m)  || \
			!defined(uint16_m) || \
			!defined(int32_m)  || \
			!defined(uint32_m) || \
			!defined(int64_m)  || \
			!defined(uint64_m)

			#define __STDC_LIMIT_MACROS
			#define __STDC_CONSTANT_MACROS
			#include <stdint.h>

			#ifndef int8_m
				#ifdef INT8_MAX
					#define int8_m int8_t
				#else
					#define int8_m char
				#endif
			#endif

			#ifndef uint8_m
				#ifdef UINT8_MAX
					#define uint8_m uint8_t
				#else
					#define uint8_m unsigned char
				#endif
			#endif

			#ifndef int16_m
				#ifdef INT16_MAX
					#define int16_m int16_t
				#else
					#define int16_m short
				#endif
			#endif

			#ifndef uint16_m
				#ifdef UINT16_MAX
					#define uint16_m uint16_t
				#else
					#define uint16_m unsigned short
				#endif
			#endif

			#ifndef int32_m
				#ifdef INT32_MAX
					#define int32_m int32_t
				#else
					#define int32_m long
				#endif
			#endif

			#ifndef uint32_m
				#ifdef UINT32_MAX
					#define uint32_m uint32_t
				#else
					#define uint32_m unsigned long
				#endif
			#endif

			#ifndef int64_m
				#ifdef INT64_MAX
					#define int64_m int64_t
				#else
					#define int64_m long long
				#endif
			#endif

			#ifndef uint64_m
				#ifdef UINT64_MAX
					#define uint64_m uint64_t
				#else
					#define uint64_m unsigned long long
				#endif
			#endif

		#endif

		#if !defined(size_m)

			#include <stddef.h>

			#ifndef size_m
				#define size_m size_t
			#endif

		#endif

		#if !defined(MU_SIZE_MAX)

			#include <stdint.h>

			#ifndef MU_SIZE_MAX
				#define MU_SIZE_MAX SIZE_MAX
			#endif

		#endif

		#if !defined(muBool)   || \
			!defined(MU_TRUE)  || \
			!defined(MU_FALSE)

			#include <stdbool.h>

			#ifndef muBool
				#define muBool bool
			#endif

			#ifndef MU_TRUE
				#define MU_TRUE true
			#endif

			#ifndef MU_FALSE
				#define MU_FALSE false
			#endif

		#endif

	/* Useful macros */

		#ifndef MUDEF
			#ifdef MU_STATIC
				#define MUDEF static
			#else
				#define MUDEF extern
			#endif
		#endif

		#ifndef MU_ZERO_STRUCT
			#ifdef __cplusplus
				#define MU_ZERO_STRUCT(s) {}
			#else
				#define MU_ZERO_STRUCT(s) (s){0}
			#endif
		#endif

		#ifndef MU_ZERO_STRUCT_CONST
			#ifdef __cplusplus
				#define MU_ZERO_STRUCT_CONST(s) {}
			#else
				#define MU_ZERO_STRUCT_CONST(s) {0}
			#endif
		#endif

		#ifndef MU_NULL_PTR
			#define MU_NULL_PTR 0
		#endif

		#ifndef MU_NULL
			#define MU_NULL 0
		#endif

		#ifndef MU_NONE
			#define MU_NONE MU_SIZE_MAX
		#endif

		#ifndef MU_SET_RESULT
			#define MU_SET_RESULT(res, val) if(res!=MU_NULL_PTR){*res=val;}
		#endif

		#ifndef MU_ASSERT
			#define MU_ASSERT(cond, res, val, after) if(!(cond)){MU_SET_RESULT(res, val) after}
		#endif

		#define MU_ENUM(name, ...) enum _##name{__VA_ARGS__};typedef enum _##name _##name; typedef size_m name;

		#if !defined(MU_WIN32) && !defined(MU_UNIX)
			#if defined(WIN32) || defined(_WIN32)
				#define MU_WIN32
			#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
				#define MU_UNIX
			#endif
		#endif

		#define MU_HRARRAY_DEFAULT_FUNC(name) \
			muBool name##_comp(name t0, name t1) { \
				return t0.active == t1.active; \
			} \
			\
			void name##_on_creation(name* p) { \
				if (p != MU_NULL_PTR) { \
					MU_LOCK_CREATE(p->lock, p->lock_active) \
				} \
			} \
			void name##_on_destruction(name* p) { \
				if (p != MU_NULL_PTR) { \
					MU_LOCK_DESTROY(p->lock, p->lock_active) \
				} \
			} \
			void name##_on_hold(name* p) { \
				if (p != MU_NULL_PTR) { \
					MU_LOCK_LOCK(p->lock, p->lock_active) \
				} \
			} \
			void name##_on_release(name* p) { \
				if (p != MU_NULL_PTR) { \
					MU_LOCK_UNLOCK(p->lock, p->lock_active) \
				} \
			} \
			\
			mu_dynamic_hrarray_declaration( \
				name##_array, name, name##_, name##_comp, \
				name##_on_creation, name##_on_destruction, name##_on_hold, name##_on_release \
			)

		#define MU_SAFEFUNC(result, lib_prefix, context, fail_return) \
			MU_SET_RESULT(result, lib_prefix##SUCCESS) \
			MU_ASSERT(context != MU_NULL_PTR, result, lib_prefix##NOT_YET_INITIALIZED, fail_return) \

		#define MU_HOLD(result, item, da, context, lib_prefix, fail_return, da_prefix) \
			MU_ASSERT(item < da.length, result, lib_prefix##INVALID_ID, fail_return) \
			da_prefix##hold_element(0, &da, item); \
			MU_ASSERT(da.data[item].active, result, lib_prefix##INVALID_ID, da_prefix##release_element(0, &da, item); fail_return)

		#define MU_RELEASE(da, item, da_prefix) \
			da_prefix##release_element(0, &da, item);

	#ifdef __cplusplus
	}
	#endif

#endif /* MUU_H */

#ifndef MAUF_H
	#define MUAF_H

	#ifdef __cplusplus
	extern "C" { // }
	#endif

	#define MUAF_VERSION_MAJOR 1
	#define MUAF_VERSION_MINOR 0
	#define MUAF_VERSION_PATCH 0

	/* C standard library dependencies */

		#if !defined(mu_memcpy) || \
			!defined(mu_strlen)

			#include <string.h>

			#ifndef mu_memcpy
				#define mu_memcpy memcpy
			#endif

			#ifndef mu_strlen
				#define mu_strlen strlen
			#endif

		#endif

		#if !defined(FILE_M)    || \
			!defined(mu_fopen)  || \
			!defined(mu_fclose) || \
			!defined(mu_fseek)  || \
			!defined(mu_fread)  || \
			!defined(mu_ftell)  || \
			!defined(mu_fwrite) || \
			!defined(mu_remove)

			#include <stdio.h>

			#ifndef FILE_M
				#define FILE_M FILE
			#endif

			#ifndef mu_fopen
				#define mu_fopen fopen
			#endif

			#ifndef mu_fclose
				#define mu_fclose fclose
			#endif

			#ifndef mu_fseek
				#define mu_fseek fseek
			#endif

			#ifndef mu_fread
				#define mu_fread fread
			#endif

			#ifndef mu_ftell
				#define mu_ftell ftell
			#endif

			#ifndef mu_fwrite
				#define mu_fwrite fwrite
			#endif

			#ifndef mu_remove
				#define mu_remove remove
			#endif

		#endif

		#if !defined(mu_malloc) || \
			!defined(mu_free)

			#include <stdlib.h>

			#ifndef mu_malloc
				#define mu_malloc malloc
			#endif

			#ifndef mu_free
				#define mu_free free
			#endif

		#endif

		#if !defined(mu_floor)

			#include <math.h>

			#ifndef mu_floor
				#define mu_floor floor
			#endif

		#endif

	/* Macros */

		#define muAudioFileCompressionLevel size_m
		#ifndef muByte
			#define muByte unsigned char
		#endif

	/* Enums */

		MU_ENUM(muafResult,
			MUAF_SUCCESS,

			MUAF_ALLOCATION_FAILED,
			MUAF_FAILED_TO_OPEN_FILE,
			MUAF_READ_CALL_FAILED,
			MUAF_WRITE_CALL_FAILED,

			MUAF_INVALID_DATA_SIZE,
			MUAF_INVALID_SIGNATURE,
			MUAF_INVALID_CHUNK_INDEX,
			MUAF_INVALID_CHANNEL_COUNT,
			MUAF_INVALID_SAMPLE_RATE,

			MUAF_UNKNOWN_AUDIO_FILE_FORMAT,

			MUAF_WAV_INVALID_RIFF_HEADER_CHUNK_SIZE,
			MUAF_WAV_INVALID_RIFF_HEADER_FORMAT,
			MUAF_WAV_INVALID_FMT_SUBCHUNK_SUBCHUNK1ID,
			MUAF_WAV_INVALID_FMT_SUBCHUNK_SUBCHUNK1SIZE,
			MUAF_WAV_UNSUPPORTED_FMT_SUBCHUNK_AUDIO_FORMAT,
			MUAF_WAV_INVALID_FMT_SUBCHUNK_CHANNEL_COUNT,
			MUAF_WAV_INVALID_FMT_SUBCHUNK_SAMPLE_RATE,
			MUAF_WAV_INVALID_FMT_SUBCHUNK_BYTE_RATE_OR_BLOCK_ALIGN,
			MUAF_WAV_INVALID_FMT_SUBCHUNK_BITS_PER_SAMPLE,
			MUAF_WAV_UNSUPPORTED_FMT_SUBCHUNK_BITS_PER_SAMPLE,
			MUAF_WAV_INVALID_FMT_SUBCHUNK_SUBCHUNK2ID,
			MUAF_WAV_INVALID_DATA_SUBCHUNK_SUBCHUNK2SIZE,
			MUAF_WAV_MISSING_DATA_SUBCHUNK_PADDING,
		)

		MU_ENUM(muAudioFileFormat,
			MU_AUDIO_FILE_UNKNOWN,
			MU_AUDIO_FILE_WAV,
		)

		MU_ENUM(muAudioFileCompression,
			MU_AUDIO_COMPRESSION_NONE,
			MU_AUDIO_COMPERSSION_LOW,
			MU_AUDIO_COMPRESSION_MEDIUM,
			MU_AUDIO_COMPRESSION_HIGH,
			MU_AUDIO_COMPRESSION_MAX
		)

		MU_ENUM(muAudioDataType,
			MU_AUDIO_DATA_UNKNOWN,
			MU_AUDIO_DATA_UINT8,
			MU_AUDIO_DATA_INT16,
			MU_AUDIO_DATA_INT24,
			MU_AUDIO_DATA_INT32,
			MU_AUDIO_DATA_INT64,
			MU_AUDIO_DATA_FLOAT32,
			MU_AUDIO_DATA_FLOAT64,
		)

		MU_ENUM(muAudioChannelInterleaving,
			MU_CHANNEL_CONSECUTIVE
		)

	/* Struct */

		struct muAudioFileInfo {
			muAudioFileFormat format;
			muAudioDataType data_type;
			size_m sample_rate;
			size_m channel_count;
			size_m chunk_count;
			size_m max_chunk_size;
		};
		typedef struct muAudioFileInfo muAudioFileInfo;

		struct muAudioChunk {
			muAudioDataType data_type;
			muAudioChannelInterleaving channel_interleaving;
			muByte* data;
			size_m data_size;
		};
		typedef struct muAudioChunk muAudioChunk;

	/* Functions */

		#ifdef MUAF_NAMES
			MUDEF const char* muaf_result_get_name(muafResult result);
		#endif

		/* Read */

			MUDEF muAudioFileFormat mu_audio_file_get_format(muafResult* result, const char* filename);
			MUDEF muAudioFileInfo mu_audio_file_get_info(muafResult* result, const char* filename);

			MUDEF muAudioChunk mu_audio_file_read_chunks(muafResult* result, const char* filename, size_m chunk_index, size_m chunk_length, muByte* data);
			MUDEF muAudioChunk mu_audio_file_free_chunk(muafResult* result, muAudioChunk chunk);

		/* Write */

			MUDEF void mu_audio_chunk_write_init(muafResult* result, const char* filename, muAudioFileInfo info);
			MUDEF void mu_audio_chunk_write_add_chunk(muafResult* result, const char* filename, muAudioChunk audio_chunk);
			MUDEF void mu_audio_chunk_write_term(muafResult* result, const char* filename);

		/* Conversion */

			MUDEF muAudioChunk mu_audio_file_convert_chunk_type(muafResult* result, muAudioChunk audio_chunk, muAudioDataType desired_type);

	#ifdef __cplusplus
	}
	#endif

#endif /* MAUF_H */

#ifdef MUAF_IMPLEMENTATION

	#ifdef __cplusplus
	extern "C" { // }
	#endif

	/* Useful shared functions */

		/* LE/BE uintN_m/muByte conversion */

			// https://stackoverflow.com/a/22613092
			// ^ This post is actually amazing

			// I really need a separate library for byte manipulation lol

			// Still not sure if the BE functions are correct...

			uint16_m muaf_leuint16(muByte* b) {
				return (uint32_m)b[0] << 0 |
				       (uint32_m)b[1] << 8 ;
			}
			uint16_m muaf_beuint16(muByte* b) {
				return (uint32_m)b[1] << 0 |
				       (uint32_m)b[0] << 8 ;
			}
			uint32_m muaf_leuint32(muByte* b) {
				return (uint32_m)b[0] << 0  |
				       (uint32_m)b[1] << 8  |
				       (uint32_m)b[2] << 16 |
				       (uint32_m)b[3] << 24 ;
			}
			uint32_m muaf_beuint32(muByte* b) {
				return (uint32_m)b[3] << 0  |
				       (uint32_m)b[2] << 8  |
				       (uint32_m)b[1] << 16 |
				       (uint32_m)b[0] << 24 ;
			}

			void muaf_wleuint16(muByte* b, uint16_m i) {
				b[0] = (uint8_m)(i >> 0);
				b[1] = (uint8_m)(i >> 8);
			}
			void muaf_wbeuint16(muByte* b, uint16_m i) {
				b[1] = (uint8_m)(i >> 0);
				b[0] = (uint8_m)(i >> 8);
			}
			void muaf_wleuint32(muByte* b, uint32_m i) {
				b[0] = (uint8_m)(i >> 0);
				b[1] = (uint8_m)(i >> 8);
				b[2] = (uint8_m)(i >> 16);
				b[3] = (uint8_m)(i >> 24);
			}
			void muaf_wbeuint32(muByte* b, uint32_m i) {
				b[3] = (uint8_m)(i >> 0);
				b[2] = (uint8_m)(i >> 8);
				b[1] = (uint8_m)(i >> 16);
				b[0] = (uint8_m)(i >> 24);
			}

		/* Audio data type information queries */

			size_m muaf_get_byte_size_of_audio_data_type(muAudioDataType data_type) {
				switch (data_type) {
					default: case MU_AUDIO_DATA_UINT8: return 1; break;
					case MU_AUDIO_DATA_INT16: return 2; break;
					case MU_AUDIO_DATA_INT24: return 3; break;
					case MU_AUDIO_DATA_INT32: case MU_AUDIO_DATA_FLOAT32: return 4; break;
					case MU_AUDIO_DATA_INT64: case MU_AUDIO_DATA_FLOAT64: return 8; break;
				}
			}

			muBool muaf_is_audio_data_type_pcm(muAudioDataType data_type) {
				switch (data_type) {
					default: return MU_TRUE; break;
					case MU_AUDIO_DATA_FLOAT32: case MU_AUDIO_DATA_FLOAT64: return MU_FALSE; break;
				}
			}

	/* WAV */
	#ifndef MUAF_NO_WAV

		/* - Notes - */

		// * Only contains support for the following formats:
		//   * PCM (0x0001)
		//   * IEEE float (0x0003)
		// All other formats (including WAVE_FORMAT_EXTENSIBLE) will throw error 
		// "MUAF_WAV_UNSUPPORTED_FMT_SUBCHUNK_AUDIO_FORMAT" when attempting to read. I plan to
		// expand format support later.
		// * As of right now, this parser is unable to read wav files with multiple subchunks.
		// * As of right now, wav files are one big chunk. This is not THAT bad, as the maximum 
		// size of a wav is ~4 GiB, but it would be nice to add later. @TODO
		// * I'm assuming that IEEE float format wav files add a fmt subchunk extension with 
		// nothing just to remain standard compliant, but I could totally be wrong.

		/* - TODOs - */

		// @TODO Add cue, playlist, and associated data list chunk handling.

		/*  - References - */

		// https://www.mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
		// https://docs.fileformat.com/audio/wav/
		// https://isip.piconepress.com/projects/speech/software/tutorials/production/fundamentals/v1.0/section_02/s02_01_p05.html
		// http://soundfile.sapp.org/doc/WaveFormat/
		// https://www.recordingblogs.com/wiki/fact-chunk-of-a-wave-file
		// https://en.wikipedia.org/wiki/WAV

		// "MAX" because chunks sometimes need to be cut short
		#ifndef MUAF_WAV_MAX_CHUNK_SAMPLE_COUNT
			#define MUAF_WAV_MAX_CHUNK_SAMPLE_COUNT 128
		#endif

		/* Useful functions */

			muBool muaf_wav_signature_check(muByte* bytes, size_m bytelen) {
				if (bytelen < 4) {
					return MU_FALSE;
				}
				return bytes[0] == 0x52 && bytes[1] == 0x49 && bytes[2] == 0x46 && bytes[3] == 0x46;
			}

		/* Pre-API level functions */

			struct muaf_wav_info {
				FILE_M* file;
				size_m sample_size;
				size_m data_index;
			};
			typedef struct muaf_wav_info muaf_wav_info;

			muAudioFileInfo muaf_wav_audio_file_get_info(muafResult* result, const char* filename, muaf_wav_info* pinfo) {
				FILE_M* file = mu_fopen(filename, "rb");
				MU_ASSERT(file != 0, result, MUAF_FAILED_TO_OPEN_FILE, return MU_ZERO_STRUCT(muAudioFileInfo);)

				MU_ASSERT(mu_fseek(file, 0, SEEK_END) == 0, result, MUAF_READ_CALL_FAILED, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);)
				size_m filelen = mu_ftell(file);
				MU_ASSERT(mu_fseek(file, 0, SEEK_SET) == 0, result, MUAF_READ_CALL_FAILED, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);)
				MU_ASSERT(filelen >= 44, result, MUAF_INVALID_DATA_SIZE, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);)
				size_m offset = 0;

				/* RIFF header */

					muByte riff_header[12];
					MU_ASSERT(mu_fread(riff_header, 12, 1, file) == 1, result, MUAF_READ_CALL_FAILED, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);)
					offset += 12;
					MU_ASSERT(mu_fseek(file, offset, SEEK_SET) == 0, result, MUAF_READ_CALL_FAILED, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);)

					// ChunkID

					MU_ASSERT(riff_header[0] == 0x52 && riff_header[1] == 0x49 && 
						riff_header[2] == 0x46 && riff_header[3] == 0x46
						,result, MUAF_INVALID_SIGNATURE, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);
					)

					// ChunkSize

					MU_ASSERT(muaf_leuint32(&riff_header[4]) == (filelen-8)
						,result, MUAF_WAV_INVALID_RIFF_HEADER_CHUNK_SIZE, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);
					)

					// Format

					MU_ASSERT(riff_header[8] == 0x57 && riff_header[9] == 0x41 && 
						riff_header[10] == 0x56 && riff_header[11] == 0x45
						,result, MUAF_WAV_INVALID_RIFF_HEADER_FORMAT, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);
					)

				/* "fmt " subchunk */

					muByte fmt_subchunk[24];
					MU_ASSERT(mu_fread(fmt_subchunk, 24, 1, file) == 1, result, MUAF_READ_CALL_FAILED, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);)
					offset += 24;
					MU_ASSERT(mu_fseek(file, offset, SEEK_SET) == 0, result, MUAF_READ_CALL_FAILED, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);)

					// Subchunk1ID

					MU_ASSERT(fmt_subchunk[0] == 0x66 && fmt_subchunk[1] == 0x6d && 
						fmt_subchunk[2] == 0x74 && fmt_subchunk[3] == 0x20
						,result, MUAF_WAV_INVALID_FMT_SUBCHUNK_SUBCHUNK1ID, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);
					)

					// Subchunk1Size

					uint32_m Subchunk1Size = muaf_leuint32(&fmt_subchunk[4]);
					MU_ASSERT(Subchunk1Size == 16 || Subchunk1Size == 18 || Subchunk1Size == 40
						,result, MUAF_WAV_INVALID_FMT_SUBCHUNK_SUBCHUNK1SIZE, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);
					)

					// AudioFormat

					uint16_m AudioFormat = muaf_leuint16(&fmt_subchunk[8]);

					// NumChannels

					uint16_m NumChannels = muaf_leuint16(&fmt_subchunk[10]);
					MU_ASSERT(NumChannels != 0
						,result, MUAF_WAV_INVALID_FMT_SUBCHUNK_CHANNEL_COUNT, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);
					)

					// SampleRate

					uint32_m SampleRate = muaf_leuint32(&fmt_subchunk[12]);
					MU_ASSERT(SampleRate != 0
						,result, MUAF_WAV_INVALID_FMT_SUBCHUNK_SAMPLE_RATE, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);
					)

					// ByteRate

					uint32_m ByteRate = muaf_leuint32(&fmt_subchunk[16]);

					// BlockAlign

					uint16_m BlockAlign = muaf_leuint16(&fmt_subchunk[20]);
					// SampleRate * NumChannels * BitsPerSample / 8 (ByteRate)
					// --------------------------------------------              = SampleRate
					//              NumChannels * BitsPerSample / 8 (BlockAlign)
					MU_ASSERT((ByteRate / BlockAlign) == SampleRate
						,result, MUAF_WAV_INVALID_FMT_SUBCHUNK_BYTE_RATE_OR_BLOCK_ALIGN, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);
					)

					// BitsPerSample

					uint16_m BitsPerSample = muaf_leuint16(&fmt_subchunk[22]);
					MU_ASSERT(BitsPerSample % 8 == 0
						,result, MUAF_WAV_INVALID_FMT_SUBCHUNK_BITS_PER_SAMPLE, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);
					)
					MU_ASSERT((ByteRate == (SampleRate * NumChannels * (BitsPerSample / 8))) && (BlockAlign == (NumChannels * (BitsPerSample / 8)))
						,result, MUAF_WAV_INVALID_FMT_SUBCHUNK_BITS_PER_SAMPLE, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);
					)

				/* "fmt " subchunk extension */
				if (AudioFormat != 0x0001) {
					muByte extension[2];
					MU_ASSERT(mu_fread(extension, 2, 1, file) == 1, result, MUAF_READ_CALL_FAILED, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);)
					offset += 2;
					MU_ASSERT(mu_fseek(file, offset, SEEK_SET) == 0, result, MUAF_READ_CALL_FAILED, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);)

					uint16_m ExtensionSize = muaf_leuint16(&extension[0]);
					// If we're reading non-WAVE_FORMAT_EXTENSIBLE and this isn't 0... should I
					// throw an error? Hmm...
					if (ExtensionSize != 0) {
						offset += ExtensionSize;
						MU_ASSERT(mu_fseek(file, offset, SEEK_SET) == 0, result, MUAF_READ_CALL_FAILED, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);)
					}
				}

				muByte next_id[4];
				MU_ASSERT(mu_fread(next_id, 4, 1, file) == 1, result, MUAF_READ_CALL_FAILED, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);)
				offset += 4;
				MU_ASSERT(mu_fseek(file, offset, SEEK_SET) == 0, result, MUAF_READ_CALL_FAILED, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);)

				/* "The Rev. 3 documentation states that the Fact chunk "is required for all new 
				WAVE formats", but "is not required" for the standard WAVE_FORMAT_PCM file. One
				presumes that files with IEEE float data (introduced after the Rev. 3 documention)
				need a fact chunk." */
				// (https://www.mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html)

				/* Possible "fact" subchunk */

				if (next_id[0] == 0x66 && next_id[1] == 0x61 && next_id[2] == 0x63 && next_id[3] == 0x74) {
					muByte FactSizeB[4];
					MU_ASSERT(mu_fread(FactSizeB, 4, 1, file) == 1, result, MUAF_READ_CALL_FAILED, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);)
					offset += 4;
					MU_ASSERT(mu_fseek(file, offset, SEEK_SET) == 0, result, MUAF_READ_CALL_FAILED, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);)

					uint32_m FactSize = muaf_leuint32(FactSizeB);
					// We're just gonna ignore the fact subchunk for now
					if (FactSize != 0) {
						offset += FactSize;
						MU_ASSERT(mu_fseek(file, offset, SEEK_SET) == 0, result, MUAF_READ_CALL_FAILED, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);)
					}

					MU_ASSERT(mu_fread(next_id, 4, 1, file) == 1, result, MUAF_READ_CALL_FAILED, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);)
					offset += 4;
					MU_ASSERT(mu_fseek(file, offset, SEEK_SET) == 0, result, MUAF_READ_CALL_FAILED, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);)
				}

				/* "data" subchunk */

					// Subchunk2ID

					MU_ASSERT(next_id[0] == 0x64 && next_id[1] == 0x61 && 
						next_id[2] == 0x74 && next_id[3] == 0x61
						,result, MUAF_WAV_INVALID_FMT_SUBCHUNK_SUBCHUNK2ID, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);
					)

					// Subchunk2Size

					muByte Subchunk2SizeB[4];
					MU_ASSERT(mu_fread(Subchunk2SizeB, 4, 1, file) == 1, result, MUAF_READ_CALL_FAILED, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);)
					offset += 4;
					MU_ASSERT(mu_fseek(file, offset, SEEK_SET) == 0, result, MUAF_READ_CALL_FAILED, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);)
					uint32_m Subchunk2Size = muaf_leuint32(Subchunk2SizeB);
					MU_ASSERT(Subchunk2Size % (NumChannels * (BitsPerSample/8)) == 0
						,result, MUAF_WAV_INVALID_DATA_SUBCHUNK_SUBCHUNK2SIZE, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);
					)

					// NumSamples (Not actually part of subchunk)

					uint32_m NumSamples = Subchunk2Size / (NumChannels * (BitsPerSample/8));
					MU_ASSERT(NumSamples != 0
						,result, MUAF_WAV_INVALID_DATA_SUBCHUNK_SUBCHUNK2SIZE, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);
					)

					// Data

					size_m data_beg = mu_ftell(file);
					MU_ASSERT(mu_fseek(file, 0, SEEK_END) == 0, result, MUAF_READ_CALL_FAILED, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);)
					size_m data_end = mu_ftell(file);
					size_m data_size = data_end - data_beg;
					MU_ASSERT((data_size % 2 == 0) && (mu_ftell(file) % 2 == 0)
						,result, MUAF_WAV_MISSING_DATA_SUBCHUNK_PADDING, mu_fclose(file); return MU_ZERO_STRUCT(muAudioFileInfo);
					)

				muAudioFileInfo info = MU_ZERO_STRUCT(muAudioFileInfo);

				// Format

				info.format = MU_AUDIO_FILE_WAV;

				// Data type

				switch (AudioFormat) {
					default: MU_SET_RESULT(result, MUAF_WAV_UNSUPPORTED_FMT_SUBCHUNK_AUDIO_FORMAT) return MU_ZERO_STRUCT(muAudioFileInfo); break;

					// PCM
					case 0x0001: {
						switch (BitsPerSample) {
							default: MU_SET_RESULT(result, MUAF_WAV_UNSUPPORTED_FMT_SUBCHUNK_BITS_PER_SAMPLE) return MU_ZERO_STRUCT(muAudioFileInfo); break;
							case 8: info.data_type = MU_AUDIO_DATA_UINT8; break;
							case 16: info.data_type = MU_AUDIO_DATA_INT16; break;
							case 24: info.data_type = MU_AUDIO_DATA_INT24; break;
							case 32: info.data_type = MU_AUDIO_DATA_INT32; break;
							case 64: info.data_type = MU_AUDIO_DATA_INT64; break;
						}
					} break;

					// IEEE float
					case 0x0003: {
						switch (BitsPerSample) {
							default: MU_SET_RESULT(result, MUAF_WAV_UNSUPPORTED_FMT_SUBCHUNK_BITS_PER_SAMPLE) return MU_ZERO_STRUCT(muAudioFileInfo); break;
							case 32: info.data_type = MU_AUDIO_DATA_FLOAT32; break;
							case 64: info.data_type = MU_AUDIO_DATA_FLOAT64; break;
						}
					} break;
				}

				// Sample rate

				info.sample_rate = SampleRate;

				// Channel count

				info.channel_count = NumChannels;

				// Chunk count

				info.chunk_count = (size_m)mu_floor(
					(double)(Subchunk2Size / (NumChannels * (BitsPerSample/8))) // Amount of samples
					/
					(double)(MUAF_WAV_MAX_CHUNK_SAMPLE_COUNT) // Per-chunk sample count
				) + 1;

				// Max chunk size

				info.max_chunk_size = MUAF_WAV_MAX_CHUNK_SAMPLE_COUNT * NumChannels * (BitsPerSample/8);

				if (pinfo != 0) {
					pinfo->file = file;
					pinfo->sample_size = Subchunk2Size;
					pinfo->data_index = data_beg;
				} else {
					mu_fclose(file);
				}
				return info;
			}

			muAudioChunk muaf_wav_audio_file_read_chunks(muafResult* result, const char* filename, size_m chunk_index, size_m chunk_length, muByte* data) {
				MU_ASSERT(chunk_length >= 1, result, MUAF_INVALID_CHUNK_INDEX, return MU_ZERO_STRUCT(muAudioChunk);)

				muafResult res = MUAF_SUCCESS;
				muaf_wav_info winfo = MU_ZERO_STRUCT(muaf_wav_info);

				muAudioFileInfo info = muaf_wav_audio_file_get_info(&res, filename, &winfo);
				MU_ASSERT(res == MUAF_SUCCESS, result, res, return MU_ZERO_STRUCT(muAudioChunk);)
				MU_ASSERT((chunk_index + chunk_length) <= info.chunk_count, result, MUAF_INVALID_CHUNK_INDEX, mu_fclose(winfo.file); return MU_ZERO_STRUCT(muAudioChunk);)

				muAudioChunk chunk = MU_ZERO_STRUCT(muAudioChunk);
				chunk.data_type = info.data_type;
				chunk.channel_interleaving = MU_CHANNEL_CONSECUTIVE;

				size_m individual_chunk_size = MUAF_WAV_MAX_CHUNK_SAMPLE_COUNT * info.channel_count * muaf_get_byte_size_of_audio_data_type(chunk.data_type);
				chunk.data_size = individual_chunk_size * chunk_length;
				if ((chunk_index + chunk_length) == info.chunk_count) {
					chunk.data_size -= individual_chunk_size;
					chunk.data_size += (winfo.sample_size % MUAF_WAV_MAX_CHUNK_SAMPLE_COUNT) * info.channel_count * muaf_get_byte_size_of_audio_data_type(chunk.data_type);
				}
				chunk.data = data;

				MU_ASSERT(mu_fseek(winfo.file, winfo.data_index + (chunk_index * individual_chunk_size), SEEK_SET) == 0, result, MUAF_READ_CALL_FAILED, mu_free(chunk.data); mu_fclose(winfo.file); return MU_ZERO_STRUCT(muAudioChunk);)
				MU_ASSERT(mu_fread(chunk.data, chunk.data_size, 1, winfo.file) == 1, result, MUAF_READ_CALL_FAILED, mu_free(chunk.data); mu_fclose(winfo.file); return MU_ZERO_STRUCT(muAudioChunk);)

				mu_fclose(winfo.file);
				return chunk;
			}

			// Philosophy of init/add/term for wav:
			// * Initiation sets up the beginning headers of the wav file with no data, leaving the
			// fields ChunkSize, FactSampleLength, and Subchunk2Size empty, as they all depend on 
			// the amount of samples, something that isn't entirely known yet.
			// * Chunk addition adds a chunk to the wav file, incrementing the previously unknown
			// fields.
			// * Termination performs a final check on these fields, making sure that they add up,
			// and correcting them if they don't.

			void muaf_wav_audio_chunk_write_init(muafResult* result, const char* filename, muAudioFileInfo info) {
				MU_ASSERT(info.channel_count >= 1, result, MUAF_INVALID_CHANNEL_COUNT, return;)
				MU_ASSERT(info.sample_rate >= 1, result, MUAF_INVALID_SAMPLE_RATE, return;)

				uint16_m BitsPerSample = (uint16_m)(muaf_get_byte_size_of_audio_data_type(info.data_type) * (size_m)8);

				FILE_M* file = mu_fopen(filename, "wb");
				MU_ASSERT(file != 0, result, MUAF_FAILED_TO_OPEN_FILE, return;)
				size_m offset = 0;

				/* RIFF header */

					muByte riff_header[12] = {0};

					// ChunkID

					riff_header[0] = 0x52; riff_header[1] = 0x49;
					riff_header[2] = 0x46; riff_header[3] = 0x46;

					// ChunkSize (filled in muaf_wav_audio_chunk_write_term)

					// Format

					riff_header[8]  = 0x57; riff_header[9]  = 0x41;
					riff_header[10] = 0x56; riff_header[11] = 0x45;

					// Padding

					MU_ASSERT(mu_fwrite(riff_header, sizeof(riff_header), 1, file) == 1, result, MUAF_WRITE_CALL_FAILED, mu_fclose(file); mu_remove(filename); return;)
					offset += sizeof(riff_header);
					MU_ASSERT(mu_fseek(file, offset, SEEK_SET) == 0, result, MUAF_WRITE_CALL_FAILED, mu_fclose(file); mu_remove(filename); return;)

				/* "fmt " subchunk */

					muByte fmt_subchunk[24] = {0};

					// Subchunk1ID

					fmt_subchunk[0] = 0x66; fmt_subchunk[1] = 0x6d;
					fmt_subchunk[2] = 0x74; fmt_subchunk[3] = 0x20;

					// Subchunk1Size

					uint32_m Subchunk1Size = 16;
					if (muaf_is_audio_data_type_pcm(info.data_type)) {
						Subchunk1Size = 18;
					}
					muaf_wleuint32(&fmt_subchunk[4], Subchunk1Size);

					// AudioFormat

					uint16_m AudioFormat = 0;

					switch (info.data_type) {
						default: MU_SET_RESULT(result, MUAF_WAV_UNSUPPORTED_FMT_SUBCHUNK_AUDIO_FORMAT) mu_fclose(file); mu_remove(filename); return; break;

						case MU_AUDIO_DATA_UINT8: case MU_AUDIO_DATA_INT16: case MU_AUDIO_DATA_INT24:
						case MU_AUDIO_DATA_INT32: case MU_AUDIO_DATA_INT64:
						AudioFormat = 0x0001; break;

						case MU_AUDIO_DATA_FLOAT32: case MU_AUDIO_DATA_FLOAT64:
						AudioFormat = 0x0003; break;
					}

					muaf_wleuint16(&fmt_subchunk[8], AudioFormat);

					// NumChannels

					uint16_m NumChannels = (uint16_m)info.channel_count;
					muaf_wleuint16(&fmt_subchunk[10], NumChannels);

					// SampleRate

					uint32_m SampleRate = (uint32_m)info.sample_rate;
					muaf_wleuint32(&fmt_subchunk[12], SampleRate);

					// ByteRate

					uint32_m ByteRate = (uint32_m)(SampleRate * (uint32_m)NumChannels * (uint32_m)(BitsPerSample/8));
					muaf_wleuint32(&fmt_subchunk[16], ByteRate);

					// BlockAlign

					uint16_m BlockAlign = (uint16_m)(NumChannels * (BitsPerSample/8));
					muaf_wleuint16(&fmt_subchunk[20], BlockAlign);

					// BitsPerSample

					muaf_wleuint16(&fmt_subchunk[22], BitsPerSample);

					MU_ASSERT(mu_fwrite(fmt_subchunk, sizeof(fmt_subchunk), 1, file) == 1, result, MUAF_WRITE_CALL_FAILED, mu_fclose(file); mu_remove(filename); return;)
					offset += sizeof(fmt_subchunk);
					MU_ASSERT(mu_fseek(file, offset, SEEK_SET) == 0, result, MUAF_WRITE_CALL_FAILED, mu_fclose(file); mu_remove(filename); return;)

				/* "fmt " subchunk extension */
				if (AudioFormat != 0x0001) {
					muByte ext[2] = {0};

					uint16_m ExtensionSize = 0;
					muaf_wleuint16(ext, ExtensionSize);

					MU_ASSERT(mu_fwrite(ext, sizeof(ext), 1, file) == 1, result, MUAF_WRITE_CALL_FAILED, mu_fclose(file); mu_remove(filename); return;)
					offset += sizeof(ext);
					MU_ASSERT(mu_fseek(file, offset, SEEK_SET) == 0, result, MUAF_WRITE_CALL_FAILED, mu_fclose(file); mu_remove(filename); return;)
				}

				/* "The optional <fact-ck> chunk reports the number of samples for some compressed
				coding schemes." */
				// https://en.wikipedia.org/wiki/WAV

				/* "fact" subchunk */
				if (AudioFormat != 0x0001) {
					muByte fact_subchunk[8] = {0};

					// FactSize

					uint32_m FactSize = 4;
					muaf_wleuint32(fact_subchunk, FactSize);

					// FactSampleLength  (filled in muaf_wav_audio_chunk_write_term)

					MU_ASSERT(mu_fwrite(fact_subchunk, sizeof(fact_subchunk), 1, file) == 1, result, MUAF_WRITE_CALL_FAILED, mu_fclose(file); mu_remove(filename); return;)
					offset += sizeof(fact_subchunk);
					MU_ASSERT(mu_fseek(file, offset, SEEK_SET) == 0, result, MUAF_WRITE_CALL_FAILED, mu_fclose(file); mu_remove(filename); return;)
				}

				/* "data" subchunk */

					muByte data_presubchunk[8] = {0};

					// Subchunk2ID

					data_presubchunk[0] = 0x64; data_presubchunk[1] = 0x61;
					data_presubchunk[2] = 0x74; data_presubchunk[3] = 0x61;

					// Subchunk2Size (filled in muaf_wav_audio_chunk_write_term)

					MU_ASSERT(mu_fwrite(data_presubchunk, sizeof(data_presubchunk), 1, file) == 1, result, MUAF_WRITE_CALL_FAILED, mu_fclose(file); mu_remove(filename); return;)
					offset += sizeof(data_presubchunk);
					MU_ASSERT(mu_fseek(file, offset, SEEK_SET) == 0, result, MUAF_WRITE_CALL_FAILED, mu_fclose(file); mu_remove(filename); return;)

				mu_fclose(file);
			}

	#endif /* MUAF_NO_WAV */

	/* Useful functions */

		size_m muaf_next_sig(const char* s, size_m i) {
			i += 1;
			while (s[i] == ' '  || s[i] == '\t' || s[i] == '\n') {
				if (s[i] == '\0') {
					return MU_NONE;
				}
				i += 1;
			}
			if (s[i] == '\0') {
				return MU_NONE;
			}
			return i;
		}

		size_m muaf_next_sigi(const char* s, size_m i, size_m count) {
			for (size_m j = 0; j < count; j++) {
				i = muaf_next_sig(s, i);
			}
			return i;
		}

		muAudioFileFormat muaf_get_audio_file_format_from_filename(const char* filename) {
			size_m len = mu_strlen(filename);
			size_m period_index = MU_NONE;

			size_m i;
			for (i = len-1; i != 0; i -= 1) {
				if (filename[i] == '.') {
					period_index = i;
					break;
				}
			}
			if (i == 0) {
				if (filename[0] == '.') {
					period_index = 0;
				} else {
					return MU_AUDIO_FILE_UNKNOWN;
				}
			}

			size_m file_extension_literal_length = muaf_next_sig(filename, period_index);
			size_m file_extension_length = 0;
			while (file_extension_literal_length != MU_NONE) {
				file_extension_literal_length = muaf_next_sig(filename, file_extension_literal_length);
				file_extension_length += 1;
			}

			switch (file_extension_length) {
				default: return MU_AUDIO_FILE_UNKNOWN; break;
				case 3: {
					if (filename[muaf_next_sigi(filename, period_index, 1)] == 'w' &&
						filename[muaf_next_sigi(filename, period_index, 2)] == 'a' &&
						filename[muaf_next_sigi(filename, period_index, 3)] == 'v'
					) {
						return MU_AUDIO_FILE_WAV;
					}
				} break;
				case 4: {
					if (filename[muaf_next_sigi(filename, period_index, 1)] == 'w' &&
						filename[muaf_next_sigi(filename, period_index, 2)] == 'a' &&
						filename[muaf_next_sigi(filename, period_index, 3)] == 'v' &&
						filename[muaf_next_sigi(filename, period_index, 4)] == 'e'
					) {
						return MU_AUDIO_FILE_WAV;
					}
				} break;
			}

			return MU_AUDIO_FILE_UNKNOWN;
		}

		// Minimum currently is 4
		muAudioFileFormat muaf_get_audio_file_format_from_file(muByte* first_bytes, size_m len) { if (first_bytes) {} if (len) {}
			#ifndef MUAF_NO_WAV
				if (muaf_wav_signature_check(first_bytes, len) == MU_TRUE) {
					return MU_AUDIO_FILE_WAV;
				}
			#endif

			return MU_AUDIO_FILE_UNKNOWN;
		}

	/* API-level functions */

		/* Names */

			#ifdef MUAF_NAMES
				MUDEF const char* muaf_result_get_name(muafResult result) {
					switch (result) {
						default: return "MUAF_UNKNOWN"; break;
						case MUAF_SUCCESS: return "MUAF_SUCCESS"; break;
						case MUAF_ALLOCATION_FAILED: return "MUAF_ALLOCATION_FAILED"; break;
						case MUAF_READ_CALL_FAILED: return "MUAF_READ_CALL_FAILED"; break;
						case MUAF_WRITE_CALL_FAILED: return "MUAF_WRITE_CALL_FAILED"; break;
						case MUAF_FAILED_TO_OPEN_FILE: return "MUAF_FAILED_TO_OPEN_FILE"; break;
						case MUAF_INVALID_DATA_SIZE: return "MUAF_INVALID_DATA_SIZE"; break;
						case MUAF_INVALID_SIGNATURE: return "MUAF_INVALID_SIGNATURE"; break;
						case MUAF_INVALID_CHUNK_INDEX: return "MUAF_INVALID_CHUNK_INDEX"; break;
						case MUAF_INVALID_CHANNEL_COUNT: return "MUAF_INVALID_CHANNEL_COUNT"; break;
						case MUAF_INVALID_SAMPLE_RATE: return "MUAF_INVALID_SAMPLE_RATE"; break;
						case MUAF_UNKNOWN_AUDIO_FILE_FORMAT: return "MUAF_UNKNOWN_AUDIO_FILE_FORMAT"; break;
						case MUAF_WAV_INVALID_RIFF_HEADER_CHUNK_SIZE: return "MUAF_WAV_INVALID_RIFF_HEADER_CHUNK_SIZE"; break;
						case MUAF_WAV_INVALID_RIFF_HEADER_FORMAT: return "MUAF_WAV_INVALID_RIFF_HEADER_FORMAT"; break;
						case MUAF_WAV_INVALID_FMT_SUBCHUNK_SUBCHUNK1ID: return "MUAF_WAV_INVALID_FMT_SUBCHUNK_SUBCHUNK1ID"; break;
						case MUAF_WAV_INVALID_FMT_SUBCHUNK_SUBCHUNK1SIZE: return "MUAF_WAV_INVALID_FMT_SUBCHUNK_SUBCHUNK1SIZE"; break;
						case MUAF_WAV_UNSUPPORTED_FMT_SUBCHUNK_AUDIO_FORMAT: return "MUAF_WAV_UNSUPPORTED_FMT_SUBCHUNK_AUDIO_FORMAT"; break;
						case MUAF_WAV_INVALID_FMT_SUBCHUNK_CHANNEL_COUNT: return "MUAF_WAV_INVALID_FMT_SUBCHUNK_CHANNEL_COUNT"; break;
						case MUAF_WAV_INVALID_FMT_SUBCHUNK_SAMPLE_RATE: return "MUAF_WAV_INVALID_FMT_SUBCHUNK_SAMPLE_RATE"; break;
						case MUAF_WAV_INVALID_FMT_SUBCHUNK_BYTE_RATE_OR_BLOCK_ALIGN: return "MUAF_WAV_INVALID_FMT_SUBCHUNK_BYTE_RATE_OR_BLOCK_ALIGN"; break;
						case MUAF_WAV_INVALID_FMT_SUBCHUNK_BITS_PER_SAMPLE: return "MUAF_WAV_INVALID_FMT_SUBCHUNK_BITS_PER_SAMPLE"; break;
						case MUAF_WAV_UNSUPPORTED_FMT_SUBCHUNK_BITS_PER_SAMPLE: return "MUAF_WAV_UNSUPPORTED_FMT_SUBCHUNK_BITS_PER_SAMPLE"; break;
						case MUAF_WAV_INVALID_FMT_SUBCHUNK_SUBCHUNK2ID: return "MUAF_WAV_INVALID_FMT_SUBCHUNK_SUBCHUNK2ID"; break;
						case MUAF_WAV_INVALID_DATA_SUBCHUNK_SUBCHUNK2SIZE: return "MUAF_WAV_INVALID_DATA_SUBCHUNK_SUBCHUNK2SIZE"; break;
						case MUAF_WAV_MISSING_DATA_SUBCHUNK_PADDING: return "MUAF_WAV_MISSING_DATA_SUBCHUNK_PADDING"; break;
					}
				}
			#endif

		/* Read */

			MUDEF muAudioFileFormat mu_audio_file_get_format(muafResult* result, const char* filename) {
				MU_SET_RESULT(result, MUAF_SUCCESS)

				muAudioFileFormat format = muaf_get_audio_file_format_from_filename(filename);
				if (format != MU_AUDIO_FILE_UNKNOWN) {
					return format;
				}

				FILE_M* file = mu_fopen(filename, "rb");
				MU_ASSERT(file != 0, result, MUAF_FAILED_TO_OPEN_FILE, return MU_AUDIO_FILE_UNKNOWN;)

				MU_ASSERT(mu_fseek(file, 0, SEEK_END) == 0, result, MUAF_READ_CALL_FAILED, mu_fclose(file); return MU_AUDIO_FILE_UNKNOWN;)
				size_m filelen = mu_ftell(file);
				MU_ASSERT(mu_fseek(file, 0, SEEK_SET) == 0, result, MUAF_READ_CALL_FAILED, mu_fclose(file); return MU_AUDIO_FILE_UNKNOWN;)

				MU_ASSERT(filelen >= 4, result, MUAF_READ_CALL_FAILED, mu_fclose(file); return MU_AUDIO_FILE_UNKNOWN;)
				muByte first_four[4];
				MU_ASSERT(mu_fread(first_four, 4, 1, file) == 1, result, MUAF_READ_CALL_FAILED, mu_fclose(file); return MU_AUDIO_FILE_UNKNOWN;)

				mu_fclose(file);
				format = muaf_get_audio_file_format_from_file(first_four, 4);
				return format;
			}

			MUDEF muAudioFileInfo mu_audio_file_get_info(muafResult* result, const char* filename) {
				MU_SET_RESULT(result, MUAF_SUCCESS)

				muAudioFileFormat format = mu_audio_file_get_format(result, filename);

				switch (format) {
					default: MU_SET_RESULT(result, MUAF_UNKNOWN_AUDIO_FILE_FORMAT) return MU_ZERO_STRUCT(muAudioFileInfo); break;

					#ifndef MUAF_NO_WAV
						case MU_AUDIO_FILE_WAV: { return muaf_wav_audio_file_get_info(result, filename, 0); } break;
					#endif
				}
			}

			MUDEF muAudioChunk mu_audio_file_read_chunks(muafResult* result, const char* filename, size_m chunk_index, size_m chunk_length, muByte* data) {
				MU_SET_RESULT(result, MUAF_SUCCESS)

				muAudioFileFormat format = mu_audio_file_get_format(result, filename);

				switch (format) {
					default: MU_SET_RESULT(result, MUAF_UNKNOWN_AUDIO_FILE_FORMAT) return MU_ZERO_STRUCT(muAudioChunk); break;

					#ifndef MUAF_NO_WAV
						case MU_AUDIO_FILE_WAV: { return muaf_wav_audio_file_read_chunks(result, filename, chunk_index, chunk_length, data); } break;
					#endif
				}
			}

			MUDEF muAudioChunk mu_audio_file_free_chunk(muafResult* result, muAudioChunk chunk) {
				MU_SET_RESULT(result, MUAF_SUCCESS)

				if (chunk.data != 0) {
					mu_free(chunk.data);
				}

				return MU_ZERO_STRUCT(muAudioChunk);
			}

		/* Write */

			MUDEF void mu_audio_chunk_write_init(muafResult* result, const char* filename, muAudioFileInfo info) {
				MU_SET_RESULT(result, MUAF_SUCCESS)

				switch (info.format) {
					default: MU_SET_RESULT(result, MUAF_UNKNOWN_AUDIO_FILE_FORMAT) return; break;

					#ifndef MUAF_NO_WAV
						case MU_AUDIO_FILE_WAV: { muaf_wav_audio_chunk_write_init(result, filename, info); return; } break;
					#endif
				}
			}

	#ifdef __cplusplus
	}
	#endif

#endif /* MUAF_IMPLEMENTATION */

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

