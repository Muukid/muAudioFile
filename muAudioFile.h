/*
muAudioFile.h - Muukid
Public domain single-file C library for reading and writing audio files.
No warranty implied; use at your own risk.

Licensed under MIT License or public domain, whichever you prefer.
More explicit license information at the end of file.
*/

/* @DOCBEGIN

# muAudioFile v1.0.0

muAudioFile (acrynomized to 'muaf') is a public domain header-only single-file C library for reading and writing audio files in various supported file formats. To use it, download the `muAudioFile.h` file, add it to your include path, and include it like so:

```c
#define MUAF_IMPLEMENTATION
#include "muAudioFile.h"
```

More information about the general structure of a mu library is provided at [the mu library information GitHub repository.](https://github.com/Muukid/mu-library-information)

# Demos

Demos that quickly show the gist of the library and how it works are available in the `demos` folder.

# Licensing

muaf is licensed under public domain or MIT, whichever you prefer. More information is provided in the accompanying file `license.md` and at the bottom of `muAudioFile.h`.

@DOCEND */

#ifndef MUAF_H
	#define MUAF_H
	
	// @DOCLINE # Other library dependencies
		// @DOCLINE muaf has a dependency on:
		
		// @DOCLINE [muUtility v1.1.0](https://github.com/Muukid/muUtility/releases/tag/v1.1.0).
		// @IGNORE
		
			#if !defined(MU_CHECK_VERSION_MISMATCHING) && defined(MUU_H) && \
				(MUU_VERSION_MAJOR != 1 || MUU_VERSION_MINOR != 1 || MUU_VERSION_PATCH != 0)

				#pragma message("[MUAF] muUtility's header has already been defined, but version doesn't match the version that this library is built for. This may lead to errors, warnings, or unexpected behavior. Define MU_CHECK_VERSION_MISMATCHING before this to turn off this message.")

			#endif

			#ifndef MUU_H
				#define MUU_H
				
				// @DOCLINE # Secure warnings
				// @DOCLINE mu libraries often use non-secure functions that will trigger warnings on certain compilers. These warnings are, to put it lightly, dumb, so the header section of muUtility defines `_CRT_SECURE_NO_WARNINGS`. However, it is not guaranteed that this definition will actually turn the warnings off, which at that point, they have to be manually turned off by the user.
				#if !defined(MU_SECURE_WARNINGS) && !defined(_CRT_SECURE_NO_WARNINGS)
					#define _CRT_SECURE_NO_WARNINGS
				#endif

				#ifdef __cplusplus
				extern "C" { // }
				#endif

				// @DOCLINE # C standard library dependencies

					// @DOCLINE muUtility has several C standard library dependencies, all of which are overridable by defining them before the inclusion of its header. The following is a list of those dependencies:

					// @DOCLINE ## `stdint.h` dependencies
					#if !defined(int8_m)      || \
						!defined(uint8_m)     || \
						!defined(int16_m)     || \
						!defined(uint16_m)    || \
						!defined(int32_m)     || \
						!defined(uint32_m)    || \
						!defined(int64_m)     || \
						!defined(uint64_m)    || \
						!defined(MU_SIZE_MAX)

						#define __STDC_LIMIT_MACROS
						#define __STDC_CONSTANT_MACROS
						#include <stdint.h>

						// @DOCLINE `int8_m`: equivalent to `int8_t` if `INT8_MAX` is defined, `char` if otherwise
						#ifndef int8_m
							#ifdef INT8_MAX
								#define int8_m int8_t
							#else
								#define int8_m char
							#endif
						#endif

						// @DOCLINE `uint8_m`: equivalent to `uint8_t` if `UINT8_MAX` is defined, `unsigned char` if otherwise
						#ifndef uint8_m
							#ifdef UINT8_MAX
								#define uint8_m uint8_t
							#else
								#define uint8_m unsigned char
							#endif
						#endif

						// @DOCLINE `int16_m`: equivalent to `int16_t` if `INT16_MAX` is defined, `short` if otherwise
						#ifndef int16_m
							#ifdef INT16_MAX
								#define int16_m int16_t
							#else
								#define int16_m short
							#endif
						#endif

						// @DOCLINE `uint16_m`: equivalent to `uint16_t` if `UINT16_MAX` is defined, `unsigned short` if otherwise
						#ifndef uint16_m
							#ifdef UINT16_MAX
								#define uint16_m uint16_t
							#else
								#define uint16_m unsigned short
							#endif
						#endif

						// @DOCLINE `int32_m`: equivalent to `int32_t` if `INT32_MAX` is defined, `long` if otherwise
						#ifndef int32_m
							#ifdef INT32_MAX
								#define int32_m int32_t
							#else
								#define int32_m long
							#endif
						#endif

						// @DOCLINE `uint32_m`: equivalent to `uint32_t` if `UINT32_MAX` is defined, `unsigned long` if otherwise
						#ifndef uint32_m
							#ifdef UINT32_MAX
								#define uint32_m uint32_t
							#else
								#define uint32_m unsigned long
							#endif
						#endif

						// @DOCLINE `int64_m`: equivalent to `int64_t` if `INT64_MAX` is defined, `long long` if otherwise
						#ifndef int64_m
							#ifdef INT64_MAX
								#define int64_m int64_t
							#else
								#define int64_m long long
							#endif
						#endif

						// @DOCLINE `uint64_m`: equivalent to `uint64_t` if `UINT64_MAX` is defined, `unsigned long long` if otherwise
						#ifndef uint64_m
							#ifdef UINT64_MAX
								#define uint64_m uint64_t
							#else
								#define uint64_m unsigned long long
							#endif
						#endif

						// @DOCLINE `MU_SIZE_MAX`: equivalent to `SIZE_MAX`
						#ifndef MU_SIZE_MAX
							#define MU_SIZE_MAX SIZE_MAX
						#endif

					#endif

					// @DOCLINE ## `stddef.h` dependencies
					#if !defined(size_m)

						#include <stddef.h>

						// @DOCLINE `size_m`: equivalent to `size_t`
						#ifndef size_m
							#define size_m size_t
						#endif

					#endif

					// @DOCLINE ## `stdbool.h` dependencies
					#if !defined(muBool)   || \
						!defined(MU_TRUE)  || \
						!defined(MU_FALSE)

						#include <stdbool.h>

						// @DOCLINE `muBool`: equivalent to `bool`
						#ifndef muBool
							#define muBool bool
						#endif

						// @DOCLINE `MU_TRUE`: equivalent to `true`
						#ifndef MU_TRUE
							#define MU_TRUE true
						#endif

						// @DOCLINE `MU_FALSE`: equivalent to `false`
						#ifndef MU_FALSE
							#define MU_FALSE false
						#endif

					#endif

				// @DOCLINE # Macros

					// @DOCLINE ## Version

						// @DOCLINE The macros `MUU_VERSION_MAJOR`, `MUU_VERSION_MINOR`, and `MUU_VERSION_PATCH` are defined to match its respective release version, following the formatting of `MAJOR.MINOR.PATCH`.

						#define MUU_VERSION_MAJOR 1
						#define MUU_VERSION_MINOR 1
						#define MUU_VERSION_PATCH 0

					// @DOCLINE ## `MUDEF`

						// @DOCLINE The `MUDEF` macro is a macro used by virtually all mu libraries, which is generally added before a header-defined variable or function. Its default value is `extern`, but can be changed to `static` by defining `MU_STATIC` before the header section of muUtility is defined. Its value can also be overwritten entirely to anything else by directly defining `MUDEF`.

						#ifndef MUDEF
							#ifdef MU_STATIC
								#define MUDEF static
							#else
								#define MUDEF extern
							#endif
						#endif

					// @DOCLINE ## Zero struct

						// @DOCLINE There are two macros, `MU_ZERO_STRUCT` and `MU_ZERO_STRUCT_CONST`, which are macro functions used to zero-out a struct's contents, with their only parameter being the struct type. The reason this needs to be defined is because the way C and C++ syntax handles an empty struct are different, and need to be adjusted for.

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

					// @DOCLINE ## Byte

						// @DOCLINE Several macros are defined to interact with the idea of a byte.

						// @DOCLINE The macro `muByte` exists as a macro for `uint8_m`, and represents a byte as a type.

						#ifndef muByte
							#define muByte uint8_m
						#endif

						// @DOCLINE There are also several macro functions used to read and write to bytes. These macros are:

						// @DOCLINE `mu_rle_uint8(b)`: Creates an expression of type `uint8_m` that reads the value stored in the 1-byte-length little-endian byte array `b`.
						#ifndef mu_rle_uint8
							#define mu_rle_uint8(b) ((uint8_m)b[0] << 0)
						#endif
						// @DOCLINE `mu_rbe_uint8(b)`: Creates an expression of type `uint8_m` that reads the value stored in the 1-byte-length big-endian byte array `b`.
						#ifndef mu_rbe_uint8
							#define mu_rbe_uint8(b) ((uint8_m)b[0] << 0)
						#endif

						// @DOCLINE `mu_rle_uint16(b)`: Creates an expression of type `uint16_m` that reads the value stored in the 2-byte-length little-endian byte array `b`.
						#ifndef mu_rle_uint16
							#define mu_rle_uint16(b) ((uint16_m)b[0] << 0 | (uint16_m)b[1] << 8)
						#endif
						// @DOCLINE `mu_rbe_uint16(b)`: Creates an expression of type `uint16_m` that reads the value stored in the 2-byte-length big-endian byte array `b`.
						#ifndef mu_rbe_uint16
							#define mu_rbe_uint16(b) ((uint16_m)b[1] << 0 | (uint16_m)b[0] << 8)
						#endif

						// @DOCLINE `mu_rle_uint32(b)`: Creates an expression of type `uint32_m` that reads the value stored in the 4-byte-length little-endian byte array `b`.
						#ifndef mu_rle_uint32
							#define mu_rle_uint32(b) ((uint32_m)b[0] << 0 | (uint32_m)b[1] << 8 | (uint32_m)b[2] << 16 | (uint32_m)b[3] << 24)
						#endif
						// @DOCLINE `mu_rbe_uint32(b)`: Creates an expression of type `uint32_m` that reads the value stored in the 4-byte-length big-endian byte array `b`.
						#ifndef mu_rbe_uint32
							#define mu_rbe_uint32(b) ((uint32_m)b[3] << 0 | (uint32_m)b[2] << 8 | (uint32_m)b[1] << 16 | (uint32_m)b[0] << 24)
						#endif

						// @DOCLINE `mu_rle_uint64(b)`: Creates an expression of type `uint64_m` that reads the value stored in the 8-byte-length little-endian byte array `b`.
						#ifndef mu_rle_uint64
							#define mu_rle_uint64(b) ((uint64_m)b[0] << 0 | (uint64_m)b[1] << 8 | (uint64_m)b[2] << 16 | (uint64_m)b[3] << 24 | (uint64_m)b[4] << 32 | (uint64_m)b[5] << 40 | (uint64_m)b[6] << 48 | (uint64_m)b[7] << 56)
						#endif
						// @DOCLINE `mu_rbe_uint64(b)`: Creates an expression of type `uint64_m` that reads the value stored in the 8-byte-length big-endian byte array `b`.
						#ifndef mu_rbe_uint64
							#define mu_rbe_uint64(b) ((uint64_m)b[7] << 0 | (uint64_m)b[6] << 8 | (uint64_m)b[5] << 16 | (uint64_m)b[4] << 24 | (uint64_m)b[3] << 32 | (uint64_m)b[2] << 40 | (uint64_m)b[1] << 48 | (uint64_m)b[0] << 56)
						#endif

						// @DOCLINE `mu_wle_uint8(b, i)`: Reads the given contents of the 1-byte-length little-endian byte array `b` into the given `uint8_m` variable `i`.
						#ifndef mu_wle_uint8
							#define mu_wle_uint8(b, i) b[0] = (uint8_m)(i >> 0);
						#endif
						// @DOCLINE `mu_wbe_uint8(b, i)`: Reads the given contents of the 1-byte-length big-endian byte array `b` into the given `uint8_m` variable `i`.
						#ifndef mu_wbe_uint8
							#define mu_wbe_uint8(b, i) b[0] = (uint8_m)(i >> 0);
						#endif

						// @DOCLINE `mu_wle_uint16(b, i)`: Reads the given contents of the 2-byte-length little-endian byte array `b` into the given `uint16_m` variable `i`.
						#ifndef mu_wle_uint16
							#define mu_wle_uint16(b, i) b[0] = (uint8_m)(i >> 0); b[1] = (uint8_m)(i >> 8);
						#endif
						// @DOCLINE `mu_wbe_uint16(b, i)`: Reads the given contents of the 2-byte-length big-endian byte array `b` into the given `uint16_m` variable `i`.
						#ifndef mu_wbe_uint16
							#define mu_wbe_uint16(b, i) b[1] = (uint8_m)(i >> 0); b[0] = (uint8_m)(i >> 8);
						#endif

						// @DOCLINE `mu_wle_uint32(b, i)`: Reads the given contents of the 4-byte-length little-endian byte array `b` into the given `uint32_m` variable `i`.
						#ifndef mu_wle_uint32
							#define mu_wle_uint32(b, i) b[0] = (uint8_m)(i >> 0); b[1] = (uint8_m)(i >> 8); b[2] = (uint8_m)(i >> 16); b[3] = (uint8_m)(i >> 24);
						#endif
						// @DOCLINE `mu_wbe_uint32(b, i)`: Reads the given contents of the 4-byte-length big-endian byte array `b` into the given `uint32_m` variable `i`.
						#ifndef mu_wbe_uint32
							#define mu_wbe_uint32(b, i) b[3] = (uint8_m)(i >> 0); b[2] = (uint8_m)(i >> 8); b[1] = (uint8_m)(i >> 16); b[0] = (uint8_m)(i >> 24);
						#endif

						// @DOCLINE `mu_wle_uint64(b, i)`: Reads the given contents of the 8-byte-length little-endian byte array `b` into the given `uint64_m` variable `i`.
						#ifndef mu_wle_uint64
							#define mu_wle_uint64(b, i) b[0] = (uint8_m)(i >> 0); b[1] = (uint8_m)(i >> 8); b[2] = (uint8_m)(i >> 16); b[3] = (uint8_m)(i >> 24); b[4] = (uint8_m)(i >> 32); b[5] = (uint8_m)(i >> 40); b[6] = (uint8_m)(i >> 48); b[7] = (uint8_m)(i >> 56);
						#endif
						// @DOCLINE `mu_wbe_uint64(b, i)`: Reads the given contents of the 8-byte-length big-endian byte array `b` into the given `uint64_m` variable `i`.
						#ifndef mu_wbe_uint64
							#define mu_wbe_uint64(b, i) b[7] = (uint8_m)(i >> 0); b[6] = (uint8_m)(i >> 8); b[5] = (uint8_m)(i >> 16); b[4] = (uint8_m)(i >> 24); b[3] = (uint8_m)(i >> 32); b[2] = (uint8_m)(i >> 40); b[1] = (uint8_m)(i >> 48); b[0] = (uint8_m)(i >> 56);
						#endif

					// @DOCLINE ## Null values

						// @DOCLINE The `MU_NULL_PTR` macro is a macro value used to represent a null pointer. It is simply 0.
						#ifndef MU_NULL_PTR
							#define MU_NULL_PTR 0
						#endif

						// @DOCLINE The `MU_NULL` macro is a macro value used to represent a null value. It is simply 0.
						#ifndef MU_NULL
							#define MU_NULL 0
						#endif

					// @DOCLINE ## Set result

						/* @DOCBEGIN The `MU_SET_RESULT(res, val)` macro is a macro function that checks if the given parameter `res` is a null pointer. If it is, it does nothing, but if it isn't, it sets its value to the given parameter `val`. This macro saves a lot of code, shrinking down what would be this:

						```c
						if (result != MU_NULL_PTR) {
						    *result = ...;
						}
						```

						into this:
						
						```c
						MU_SET_RESULT(result, ...)
						```

						@DOCEND */

						#ifndef MU_SET_RESULT
							#define MU_SET_RESULT(res, val) if(res!=MU_NULL_PTR){*res=val;}
						#endif

					// @DOCLINE ## Enum

						/* @DOCBEGIN

						The `MU_ENUM(name, ...)` macro is a macro function used to declare an enumerator. The reason why one would prefer this over the traditional way of declaring enumerators is because it actually makes it a `size_m` which can avoid errors on certain compilers (looking at you, Microsoft) in regards to treating enumerators like values. It expands like this:

						```c
						enum _##name{
						    __VA_ARGS__
						};
						typedef enum _##name _##name;
						typedef size_m name;
						```

						@DOCEND */

						#define MU_ENUM(name, ...) enum _##name{__VA_ARGS__};typedef enum _##name _##name; typedef size_m name;

					// @DOCLINE ## Operating system recognition

						/* @DOCBEGIN The macros `MU_WIN32` or `MU_UNIX` are defined (if neither were defined before) in order to allow mu libraries to easily check if they're running on a Windows or Unix system.

						`MU_WIN32` will be defined if `WIN32` or `_WIN32` are defined, one of which is usually pre-defined on Windows systems.

						`MU_UNIX` will be defined if `__unix__` is defined or both `__APPLE__` and `__MACH__` are defined.

						@DOCEND */

						#if !defined(MU_WIN32) && !defined(MU_UNIX)
							#if defined(WIN32) || defined(_WIN32)
								#define MU_WIN32
							#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
								#define MU_UNIX
							#endif
						#endif

				#ifdef __cplusplus
				}
				#endif

			#endif /* MUU_H */
		// @ATTENTION

		// @DOCLINE Note that mu libraries store their dependencies within their files, so you don't need to import these dependencies yourself.
		// @DOCLINE Note that the libraries listed may also have other dependencies that they also include that aren't listed here.

	#ifdef __cplusplus
	extern "C" { // }
	#endif

	// @DOCLINE # Macros

		// @DOCLINE ## Audio file inclusion

		// @DOCLINE muaf does not automatically define any API for any audio file formats; which audio file formats need to be defined is controlled by the user via a collection of macros defined before the inclusion of the header file.

		// @DOCLINE This is a list of all of those macros:

		// @DOCLINE `MUAF_WAVE`: defines the API for the [WAVE](https://en.wikipedia.org/wiki/WAV) audio file format.

		// @DOCLINE All of these macros can be automatically turned on via one macro, `MUAF_ALL_FORMATS`.

		#ifdef MUAF_ALL_FORMATS
			#define MUAF_WAVE
		#endif

		// @DOCLINE ## Version

			// @DOCLINE muaf defines three macros to define the version of muaf: `MUAF_VERSION_MAJOR`, `MUAF_VERSION_MINOR`, and `MUAF_VERSION_PATCH`, following the format of `vMAJOR.MINOR.PATCH`.

			#define MUAF_VERSION_MAJOR 1
			#define MUAF_VERSION_MINOR 0
			#define MUAF_VERSION_PATCH 0

	// @DOCLINE # C standard library dependencies

		// @DOCLINE muaf has several C standard library dependencies not provided by its other library dependencies, all of which are overridable by defining them before the inclusion of its header. This is a list of all of those dependencies.

		#if !defined(mu_fopen) || \
			!defined(mu_fclose) || \
			!defined(mu_fseek) || \
			!defined(mu_ftell) || \
			!defined(mu_fread) || \
			!defined(mu_fwrite) || \
			!defined(MU_SEEK_END) || \
			!defined(MU_SEEK_SET) || \
			!defined(MU_FILE)

			// @DOCLINE ## `stdio.h` dependencies
			#include <stdio.h>

			// @DOCLINE `mu_fopen`: equivalent to fopen
			#ifndef mu_fopen
				#define mu_fopen fopen
			#endif

			// @DOCLINE `mu_fclose`: equivalent to fclose
			#ifndef mu_fclose
				#define mu_fclose fclose
			#endif

			// @DOCLINE `mu_fseek`: equivalent to fseek
			#ifndef mu_fseek
				#define mu_fseek fseek
			#endif

			// @DOCLINE `mu_ftell`: equivalent to ftell
			#ifndef mu_ftell
				#define mu_ftell ftell
			#endif

			// @DOCLINE: `mu_fread`: equivalent to fread
			#ifndef mu_fread
				#define mu_fread fread
			#endif

			// @DOCLINE `mu_fwrite`: equivalent to fwrite
			#ifndef mu_fwrite
				#define mu_fwrite fwrite
			#endif

			// @DOCLINE `MU_SEEK_END`: equivalent to `SEEK_END`
			#ifndef MU_SEEK_END
				#define MU_SEEK_END SEEK_END
			#endif

			// @DOCLINE `MU_SEEK_SET`: equivalent to `SEEK_SET`
			#ifndef MU_SEEK_SET
				#define MU_SEEK_SET SEEK_SET
			#endif

			// @DOCLINE `MU_FILE`: equivalent to `FILE`
			#ifndef MU_FILE
				#define MU_FILE FILE
			#endif

		#endif

	// @DOCLINE # Result enumerator

		// @DOCLINE muaf uses the `muaFResult` enumerator to represent how a function went.

		// @DOCLINE ## Values

			MU_ENUM(muafResult,
				// @DOCLINE `@NLFT`: the task succeeded.
				MUAF_SUCCESS,
				// @DOCLINE `@NLFT`: a call to `mu_fopen` returned `NULL`.
				MUAF_FAILED_OPEN_FILE,
				// @DOCLINE `@NLFT`: a call to `mu_fseek` failed; this is often caused by an unexpected end-of-file, indicating that the file data is incomplete.
				MUAF_FAILED_FILE_SEEK,
				// @DOCLINE `@NLFT`: a call to `mu_fread` failed.
				MUAF_FAILED_FILE_READ,
				// @DOCLINE `@NLFT`: a call to `mu_fwrite` failed.
				MUAF_FAILED_FILE_WRITE,
				// @DOCLINE `@NLFT`: the audio file data unexpectedly ended.
				MUAF_UNEXPECTED_EOF,
				// @DOCLINE `@NLFT`: the RIFF header for the WAVE file is invalid; the data is most likely incomplete or is not a WAVE file at all.
				MUAF_WAVE_INVALID_RIFF,
				// @DOCLINE `@NLFT`: the "WAVE" marker was not found in the RIFF chunk; the file is most likely a RIFF file but not a WAVE file.
				MUAF_WAVE_INVALID_RIFF_WAVE,
				// @DOCLINE `@NLFT`: a duplicate chunk type was found.
				MUAF_WAVE_DUPLICATE_CHUNK,
				// @DOCLINE `@NLFT`: a format chunk was not provided in the WAVE file.
				MUAF_WAVE_MISSING_FMT_CK,
				// @DOCLINE `@NLFT`: a WAVE audio format is not supported for the given operation by muaf.
				MUAF_WAVE_UNSUPPORTED_FORMAT,
			)

		// @DOCLINE ## Result name function

			#ifdef MUAF_NAMES
			// @DOCLINE The function `muaf_result_get_name` converts a muaf result enumerator value into a `const char*` representation, defined below: @NLNT
			MUDEF const char* muaf_result_get_name(muafResult result);
			#endif
			// @DOCLINE Note that this function is only defined if `MUAF_NAMES` is defined before the inclusion of the header file.

	// @DOCLINE # Audio file stream (AFS)

		// @DOCLINE In order for muaf to be able to process an audio file, it needs it in data. This can be achieved with an audio file stream, or AFS for short, defined under the type `muAFS`, which encapsulates multiple ways to store and process an audio file.

		struct muAFS_file {
			MU_FILE* fptr;
			size_m size;
		}; typedef struct muAFS_file muAFS_file;

		struct muAFS_data {
			muByte* ptr;
			size_m size;
		}; typedef struct muAFS_data muAFS_data;

		union muAFS_container {
			muAFS_file file;
			muAFS_data data;
		}; typedef union muAFS_container muAFS_container;

		enum muAFS_type {
			MU_AFS_FILE,
			MU_AFS_DATA,
		}; typedef enum muAFS_type muAFS_type;

		struct muAFS {
			muAFS_type type;
			muAFS_container container;
		}; typedef struct muAFS muAFS;

		// @DOCLINE This part of the API is made so that optimizations for the amount of memory loaded when reading from a file can be done with the same API as reading directly from an audio file stored purely in RAM.

		// @DOCLINE ## Open file

			// @DOCLINE The function `mu_afs_open_file` opens a file and creates an AFS handle for it, defined below: @NLNT
			MUDEF muAFS mu_afs_open_file(muafResult* result, const char* filename);
			// @DOCLINE Note that once the file is open, it is not guaranteed that the file's contents will not change whilst it is open, meaning that it is up to the user to guarantee that the file's contents remain unchanged.

		// @DOCLINE ## Create AFS data handle

			// @DOCLINE The function `mu_afs_create_data_handle` creates an AFS handle based on audio data already stored in its entirety, defined below: @NLNT
			MUDEF muAFS mu_afs_create_data_handle(muByte* data, size_m size);

		// @DOCLINE ## Destroy AFS

			// @DOCLINE The function `mu_afs_destroy` destroys an AFS, defined below: @NLNT
			MUDEF void mu_afs_destroy(muAFS afs);

	#ifdef MUAF_WAVE
	// @DOCLINE # WAVE

		// @DOCLINE muaf has support for [the WAVE audio file format](https://en.wikipedia.org/wiki/WAV). This part of the documentation details muaf's API for interacting with WAVE.

		// @DOCLINE ## Support

			// @DOCLINE muaf's WAVE implementation currently does ***not*** support the following features:

			// @DOCLINE * Cue points and cue point playlists.

			// @DOCLINE * Associated data information.

			// @DOCLINE * Info lists.

			// @DOCLINE * Multiple pieces of audio data in one WAVE file (wave lists).

			// @DOCLINE Note that "not supporting" these features means that muaf will pretend like these don't exist when encountered in read operations (with the exception of wave lists since they're vital to the audio storage of a WAVE file) and not provide API support for them in write operations.

		// @DOCLINE ## Enumerators

			// @DOCLINE The WAVE API defines several enumerators. The following is a list of them.

			// @DOCLINE ### WAVE format

				// @DOCLINE The enum `muWAVEFormat` is used to define a list of audio data formats supported by WAVE and muaf's WAVE API. Its values are defined below.

				MU_ENUM(muWAVEFormat,
					// @DOCLINE `@NLFT`: raw [PCM](https://en.wikipedia.org/wiki/Pulse-code_modulation) audio data.
					MUAF_WAVE_FORMAT_PCM=0x0001,
				)

		// @DOCLINE ## File info structs

			// @DOCLINE muaf uses two structs, `muWAVEInfo` and `muWAVEAdvancedInfo`, to describe the contents of a WAVE file's data.

			// @DOCLINE ### WAVE info

				// @DOCLINE The `muWAVEInfo` struct is used to describe the surface level contents of a WAVE file, and is intended to be used the the user. The following is a list of its members.

				struct muWAVEInfo {
					// @DOCLINE `format`: the WAVE audio data format, defined below: @NLNT
					muWAVEFormat format;
					// @DOCLINE `channels`: the amount of channels in the audio data, defined below: @NLNT
					uint16_m channels;
					// @DOCLINE `sample_rate`: the sample rate of the audio, defined below: @NLNT
					uint32_m sample_rate;
					// @DOCLINE `sample_size`: the size of a single sample in bytes (used only for non-compressed formats), defined below: @NLNT
					uint16_m sample_size;
					// @DOCLINE `audio_data_count`: the amount of separate audio data pieces stored, defined below: @NLNT
					size_m audio_data_count;
					// @DOCLINE `audio_data_size`: the size of the overall stored audio data in bytes (if the WAVE file has multiple audio data pieces, this is all of their sizes combined), defined below: @NLNT
					uint32_m audio_data_size;
				};
				typedef struct muWAVEInfo muWAVEInfo;

			// @DOCLINE ### WAVE advanced info

				// @DOCLINE The `muWAVEAdvancedInfo` struct is used by muaf to keep track of a WAVE file's contents when passed between multiple functions. It is not meant to be used by the user.

				MU_ENUM(muWAVE_chunk_type,
					MU_WAVE_UNKNOWN_CK,
					MU_WAVE_FMT_CK,
					MU_WAVE_DATA_CK,
					MU_WAVE_FACT_CK,
					MU_WAVE_CUE_CK,
					MU_WAVE_PLAYLIST_CK,
					MU_WAVE_ASSOC_DATA_LIST,
					MU_WAVE_INFO_LIST,
				)
				#define MU_WAVE_CHUNK_TYPE_LEN 7
				const char* muWAVE_inner_chunk_type_names[MU_WAVE_CHUNK_TYPE_LEN] = {
					"fmt ", "data", "fact", "cue ", "????", "????", "????"
				};

				struct muWAVE_chunk_info {
					muWAVE_chunk_type type;
					size_m size;
				}; typedef struct muWAVE_chunk_info muWAVE_chunk_info;

				struct muWAVE_chunk {
					muBool exists;
					size_m byte_index;
				}; typedef struct muWAVE_chunk muWAVE_chunk;

				struct muWAVEAdvancedInfo {
					muWAVE_chunk fmt_ck;
					muWAVE_chunk fact_ck;
					muWAVE_chunk data_ck;
				}; typedef struct muWAVEAdvancedInfo muWAVEAdvancedInfo;

		// @DOCLINE ## Read functions

			// @DOCLINE The WAVE API has several functions used to obtain information about a pre-existing WAVE file. The following is a list of them.

			// @DOCLINE ### Get advanced info

				// @DOCLINE The function `mu_WAVE_get_advanced_info` gets the advanced information about a WAVE file which is then used by other functions, defined below: @NLNT
				MUDEF muWAVEAdvancedInfo mu_WAVE_get_advanced_info(muafResult* result, muAFS afs);

		// @DOCLINE ## Write functions

			// @DOCLINE The WAVE API can be used to write WAVE files. The following is a list of functions that can be used to do so.

			// @DOCLINE ## Write a WAVE file

				// @DOCLINE The function `mu_WAVE_write_file` is used to write a WAVE file in one function call, defined below: @NLNT
				MUDEF void mu_WAVE_write_file(muafResult* result, const char* filename, muWAVEInfo info, void* data);

	#endif /* MUAF_WAVE */

	#ifdef __cplusplus
	}
	#endif
#endif /* MUAF_H */

#ifdef MUAF_IMPLEMENTATION
	#ifdef __cplusplus
	extern "C" { // }
	#endif

	/* Enum names */

		#ifdef MUAF_NAMES
			MUDEF const char* muaf_result_get_name(muafResult result) {
				switch (result) {
					default: return "MUAF_UNKNOWN"; break;
					case MUAF_SUCCESS: return "MUAF_SUCCESS"; break;
					case MUAF_FAILED_OPEN_FILE: return "MUAF_FAILED_OPEN_FILE"; break;
					case MUAF_FAILED_FILE_SEEK: return "MUAF_FAILED_FILE_SEEK"; break;
					case MUAF_FAILED_FILE_READ: return "MUAF_FAILED_FILE_READ"; break;
					case MUAF_FAILED_FILE_WRITE: return "MUAF_FAILED_FILE_WRITE"; break;
					case MUAF_UNEXPECTED_EOF: return "MUAF_UNEXPECTED_EOF"; break;
					case MUAF_WAVE_INVALID_RIFF: return "MUAF_WAVE_INVALID_RIFF"; break;
					case MUAF_WAVE_INVALID_RIFF_WAVE: return "MUAF_WAVE_INVALID_RIFF_WAVE"; break;
					case MUAF_WAVE_DUPLICATE_CHUNK: return "MUAF_WAVE_DUPLICATE_CHUNK"; break;
					case MUAF_WAVE_MISSING_FMT_CK: return "MUAF_WAVE_MISSING_FMT_CK"; break;
					case MUAF_WAVE_UNSUPPORTED_FORMAT: return "MUAF_WAVE_UNSUPPORTED_FORMAT"; break;
				}
			}
		#endif

	/* AFS */

		/* File */

			MUDEF muAFS mu_afs_open_file(muafResult* result, const char* filename) {
				muAFS afs;
				afs.type = MU_AFS_FILE;
				afs.container.file.fptr = mu_fopen(filename, "rb");
				if (afs.container.file.fptr == NULL) {
					MU_SET_RESULT(result, MUAF_FAILED_OPEN_FILE)
					return MU_ZERO_STRUCT(muAFS);
				}

				if (mu_fseek(afs.container.file.fptr, 0L, MU_SEEK_END) != 0) {
					MU_SET_RESULT(result, MUAF_FAILED_FILE_SEEK)
					mu_fclose(afs.container.file.fptr);
					return MU_ZERO_STRUCT(muAFS);
				}
				afs.container.file.size = (size_m)mu_ftell(afs.container.file.fptr);

				if (mu_fseek(afs.container.file.fptr, 0L, MU_SEEK_SET) != 0) {
					MU_SET_RESULT(result, MUAF_FAILED_FILE_SEEK)
					mu_fclose(afs.container.file.fptr);
					return MU_ZERO_STRUCT(muAFS);
				}

				return afs;
			}

			void mu_inner_afs_close_file(muAFS afs) {
				mu_fclose(afs.container.file.fptr);
			}

			void mu_inner_afs_get_file_data(muafResult* result, muAFS afs, size_m beg_byte, size_m byte_length, muByte* data) {
				if (mu_fseek(afs.container.file.fptr, (long int)beg_byte, MU_SEEK_SET) != 0) {
					MU_SET_RESULT(result, MUAF_FAILED_FILE_SEEK)
					return;
				}

				if (mu_fread(data, 1, byte_length, afs.container.file.fptr) != byte_length) {
					MU_SET_RESULT(result, MUAF_FAILED_FILE_READ)
					return;
				}
			}

		/* Data */

			MUDEF muAFS mu_afs_create_data_handle(muByte* data, size_m size) {
				muAFS afs;
				afs.type = MU_AFS_DATA;
				afs.container.data.ptr = data;
				afs.container.data.size = size;
				return afs;
			}

			void mu_inner_afs_get_data_data(muafResult* result, muAFS afs, size_m beg_byte, size_m byte_length, muByte** data) {
				if ((beg_byte + byte_length) > afs.container.data.size) {
					MU_SET_RESULT(result, MUAF_UNEXPECTED_EOF)
					return;
				}
				*data = &afs.container.data.ptr[beg_byte];
			}

		/* General functions */

			MUDEF void mu_afs_destroy(muAFS afs) {
				switch (afs.type) {
					default: break;
					case MU_AFS_FILE: mu_inner_afs_close_file(afs); break;
					case MU_AFS_DATA: /*...*/ break;
				}
			}

			#define mu_inner_afs_get_data(res, b, afs, offset, len) muByte _##b[len]; \
				if (afs.type == MU_AFS_FILE) { \
					mu_inner_afs_get_file_data(&res, afs, offset, len, _##b); \
					b = _##b; \
				} else { \
					mu_inner_afs_get_data_data(&res, afs, 0, 8, &b); \
				}

			size_m mu_inner_afs_size(muAFS afs) {
				if (afs.type == MU_AFS_FILE) {
					return afs.container.file.size;
				} else {
					return afs.container.data.size;
				}
			}

	/* WAVE */
	#ifdef MUAF_WAVE

		/* Chunk info */

			muWAVE_chunk_info muWAVE_inner_get_chunk_info(muByte* ck8) {
				muWAVE_chunk_info info;
				info.type = MU_WAVE_UNKNOWN_CK;
				info.size = mu_rle_uint32((&ck8[4]));

				muBool invalid[MU_WAVE_CHUNK_TYPE_LEN] = { 0 };
				for (size_m i = 0; i < MU_WAVE_CHUNK_TYPE_LEN; i++) {
					for (size_m j = 0; j < 4; j++) {
						if (!invalid[i] && ck8[j] != (muByte)muWAVE_inner_chunk_type_names[i][j]) {
							invalid[i] = 1;
						} else if (!invalid[i] && j == 3) {
							info.type = i+1;
							return info;
						}
					}
				}

				return info;
			}

		/* Adv. info */

			MUDEF muWAVEAdvancedInfo mu_WAVE_get_advanced_info(muafResult* result, muAFS afs) {
				muafResult res = MUAF_SUCCESS;

				muWAVEAdvancedInfo adv_info;
				adv_info.fmt_ck.exists = MU_FALSE;
				adv_info.fact_ck.exists = MU_FALSE;
				adv_info.data_ck.exists = MU_FALSE;

				/* RIFF chunk + WAVE */
				{
					muByte* b12;
					mu_inner_afs_get_data(res, b12, afs, 0, 12);
					if (res != MUAF_SUCCESS) {
						MU_SET_RESULT(result, res)
						return MU_ZERO_STRUCT(muWAVEAdvancedInfo);
					}

					// R I F F
					if (b12[0] != 0x52 || b12[1] != 0x49 || b12[2] != 0x46 || b12[3] != 0x46) {
						MU_SET_RESULT(result, MUAF_WAVE_INVALID_RIFF)
						return MU_ZERO_STRUCT(muWAVEAdvancedInfo);
					}
					// W A V E
					if (b12[8] != 0x57 || b12[9] != 0x41 || b12[10] != 0x56 || b12[11] != 0x45) {
						MU_SET_RESULT(result, MUAF_WAVE_INVALID_RIFF_WAVE)
						return MU_ZERO_STRUCT(muWAVEAdvancedInfo);
					}

					uint32_m riff_size = mu_rle_uint32((&b12[4]));
					if (riff_size != (uint32_m)(mu_inner_afs_size(afs)-8)) {
						MU_SET_RESULT(result, MUAF_WAVE_INVALID_RIFF)
						return MU_ZERO_STRUCT(muWAVEAdvancedInfo);
					}
				}

				/* Chunk info */
				{
					size_m i = 12;
					size_m afs_size = mu_inner_afs_size(afs);

					while (i < afs_size) {
						muByte* b8;
						mu_inner_afs_get_data(res, b8, afs, i, 8);
						if (res != MUAF_SUCCESS) {
							MU_SET_RESULT(result, res)
							return MU_ZERO_STRUCT(muWAVEAdvancedInfo);
						}

						muWAVE_chunk_info cki = muWAVE_inner_get_chunk_info(b8);
						switch (cki.type) {
							default: break;

							case MU_WAVE_FMT_CK: {
								if (adv_info.fmt_ck.exists) {
									MU_SET_RESULT(result, MUAF_WAVE_DUPLICATE_CHUNK)
									return MU_ZERO_STRUCT(muWAVEAdvancedInfo);
								}
								adv_info.fmt_ck.exists = MU_TRUE;
								adv_info.fmt_ck.byte_index = i;
							} break;

							case MU_WAVE_FACT_CK: {
								if (adv_info.fact_ck.exists) {
									MU_SET_RESULT(result, MUAF_WAVE_DUPLICATE_CHUNK)
									return MU_ZERO_STRUCT(muWAVEAdvancedInfo);
								}
								adv_info.fact_ck.exists = MU_TRUE;
								adv_info.fact_ck.byte_index = i;
							} break;

							case MU_WAVE_DATA_CK: {
								if (adv_info.data_ck.exists) {
									MU_SET_RESULT(result, MUAF_WAVE_DUPLICATE_CHUNK)
									return MU_ZERO_STRUCT(muWAVEAdvancedInfo);
								}
								adv_info.data_ck.exists = MU_TRUE;
								adv_info.data_ck.byte_index = i;
							} break;
						}
						
						i += cki.size + 8;
						if ((cki.size % 2) != 0) {
							// Note: this implicitly allows WAVE files to not have a padding byte at the end.
							i += 1;
						}
					}
				}

				/* Checks */
				{
					if (!adv_info.fmt_ck.exists) {
						MU_SET_RESULT(result, MUAF_WAVE_MISSING_FMT_CK)
						return MU_ZERO_STRUCT(muWAVEAdvancedInfo);
					}
					// @TODO Wave checks
				}

				return adv_info;
			}

		/* Writing */

			/* Small functions */

				/* RIFF */

					// NOTE: doesn't write RIFF chunk size yet
					void muWAVE_inner_write_riff_wave(muafResult* result, MU_FILE* fptr) {
						static const muByte riff_beg[12] = {
							// RIFF
							0x52, 0x49, 0x46, 0x46,
							// Size (0 for now)
							0, 0, 0, 0,
							// WAVE
							0x57, 0x41, 0x56, 0x45,
						};

						if (mu_fwrite(riff_beg, 1, sizeof(riff_beg), fptr) != sizeof(riff_beg)) {
							MU_SET_RESULT(result, MUAF_FAILED_FILE_WRITE)
							return;
						}
					}

					void muWAVE_inner_correct_riff_wave(muafResult* result, MU_FILE* fptr) {
						if (mu_fseek(fptr, 0, MU_SEEK_END) != 0) { MU_SET_RESULT(result, MUAF_FAILED_FILE_SEEK) return; }

						uint32_m size = (uint32_m)(mu_ftell(fptr) - 8);
						muByte size_b[4];
						mu_wle_uint32(size_b, size)

						if (mu_fseek(fptr, 4, MU_SEEK_SET) != 0) { MU_SET_RESULT(result, MUAF_FAILED_FILE_SEEK) return; }
						if (mu_fwrite(size_b, 1, sizeof(size_b), fptr) != sizeof(size_b)) {
							MU_SET_RESULT(result, MUAF_FAILED_FILE_WRITE)
							return;
						}
					}

				/* fmt */

					void muWAVE_inner_write_fmt_ck_pcm(muafResult* result, MU_FILE* fptr, muWAVEInfo info) {
						// "fmt " (4) + size (4) + wFormatTag (2) + wChannels (2) + dwSamplesPerSec (4) + dwAvgBytesPerSec (4) + wBlockAlign (2) = 22 bytes
						// wBitsPerSample (2) = 2 bytes

						/* fmt */
						{
							muByte fmt_[22];

							// "fmt "
							fmt_[0] = 0x66; fmt_[1] = 0x6D; fmt_[2] = 0x74; fmt_[3] = 0x20;

							// size
							uint32_m size = 24-8;
							mu_wle_uint32((&fmt_[4]), size)

							// wFormatTag
							mu_wle_uint16((&fmt_[8]), (uint16_m)info.format)
							// wChannels
							mu_wle_uint16((&fmt_[10]), (uint16_m)info.channels)
							// dwSamplesPerSec
							mu_wle_uint32((&fmt_[12]), (uint32_m)info.sample_rate)

							// dwAvgBytesPerSec
							uint32_m bps = (uint32_m)info.sample_size * (uint32_m)info.sample_rate;
							mu_wle_uint32((&fmt_[16]), bps)

							// wBlockAlign
							uint16_m ba = (uint16_m)info.channels + (uint16_m)info.sample_size;
							mu_wle_uint16((&fmt_[20]), ba);

							if (mu_fwrite(fmt_, 1, sizeof(fmt_), fptr) != sizeof(fmt_)) {
								MU_SET_RESULT(result, MUAF_FAILED_FILE_WRITE)
								return;
							}
						}

						/* PCM-specific fields */
						{
							muByte fsf[2];

							// wBitsPerSample
							uint16_m bps = (uint16_m)(info.sample_size * 8);
							mu_wle_uint16(fsf, bps)

							if (mu_fwrite(fsf, 1, sizeof(fsf), fptr) != sizeof(fsf)) {
								MU_SET_RESULT(result, MUAF_FAILED_FILE_WRITE)
								return;
							}
						}
					}

					void muWAVE_inner_write_fmt_ck(muafResult* result, MU_FILE* fptr, muWAVEInfo info) {
						switch (info.format) {
							default: MU_SET_RESULT(result, MUAF_WAVE_UNSUPPORTED_FORMAT) return; break;
							case MUAF_WAVE_FORMAT_PCM: muWAVE_inner_write_fmt_ck_pcm(result, fptr, info); break;
						}
					}

				/* data */

					void muWAVE_inner_write_data_ck_pcm(muafResult* result, MU_FILE* fptr, muWAVEInfo info, void* data) {
						/* ck */
						{
							muByte ck[8];

							// "data"
							ck[0] = 0x64; ck[1] = 0x61; ck[2] = 0x74; ck[3] = 0x61;

							// Size
							mu_wle_uint32((&ck[4]), (uint32_m)info.audio_data_size)

							if (mu_fwrite(ck, 1, sizeof(ck), fptr) != sizeof(ck)) {
								MU_SET_RESULT(result, MUAF_FAILED_FILE_WRITE)
								return;
							}
						}

						/* Actual data */
						{
							if (mu_fwrite(data, 1, info.audio_data_size, fptr) != info.audio_data_size) {
								MU_SET_RESULT(result, MUAF_FAILED_FILE_WRITE)
								return;
							}
						}
					}

					void muWAVE_inner_write_data(muafResult*result, MU_FILE* fptr, muWAVEInfo info, void* data) {
						switch (info.format) {
							default: MU_SET_RESULT(result, MUAF_WAVE_UNSUPPORTED_FORMAT) return; break;
							case MUAF_WAVE_FORMAT_PCM: muWAVE_inner_write_data_ck_pcm(result, fptr, info, data); break;
						}
					}

			/* Big functions */

				MUDEF void mu_WAVE_write_file(muafResult* result, const char* filename, muWAVEInfo info, void* data) {
					MU_FILE* fptr = mu_fopen(filename, "wb");
					if (fptr == NULL) { MU_SET_RESULT(result, MUAF_FAILED_OPEN_FILE) return;}

					muafResult res = MUAF_SUCCESS;

					muWAVE_inner_write_riff_wave(&res, fptr);
					if (res != MUAF_SUCCESS) { MU_SET_RESULT(result, res) return; mu_fclose(fptr); }

					muWAVE_inner_write_fmt_ck(&res, fptr, info);
					if (res != MUAF_SUCCESS) { MU_SET_RESULT(result, res) return; mu_fclose(fptr); }

					muWAVE_inner_write_data(&res, fptr, info, data);
					if (res != MUAF_SUCCESS) { MU_SET_RESULT(result, res) return; mu_fclose(fptr); }

					mu_fclose(fptr);

					fptr = mu_fopen(filename, "rb+");
					if (fptr == NULL) { MU_SET_RESULT(result, MUAF_FAILED_OPEN_FILE) return;}

					muWAVE_inner_correct_riff_wave(&res, fptr);
					if (res != MUAF_SUCCESS) { MU_SET_RESULT(result, res) return; mu_fclose(fptr); }

					mu_fclose(fptr);
					return; if (data) {}
				}

	#endif /* MUAF_WAVE */

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

