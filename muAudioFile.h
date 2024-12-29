/*
muAudioFile.h - Muukid
Public domain single-file C library for reading and writing audio file data.
https://github.com/Muukid/muAudioFile
No warranty implied; use at your own risk.

Licensed under MIT License or public domain, whichever you prefer.
More explicit license information at the end of file.

@TODO Overview section, not just terminology
@TODO Add wave data size check (if haven't already)
@TODO More concise handling for audio files with no audio data (?)

@DOCBEGIN

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

@DOCEND

*/

#ifndef MUAF_H
	#define MUAF_H
	
	// @DOCLINE # Other library dependencies
		// @DOCLINE muaf has a dependency on:

		// @DOCLINE * [muUtility v2.0.1](https://github.com/Muukid/muUtility/releases/tag/v2.0.1).
		// @IGNORE
			#if !defined(MU_CHECK_VERSION_MISMATCHING) && defined(MUU_H) && \
				(MUU_VERSION_MAJOR != 2 || MUU_VERSION_MINOR != 0 || MUU_VERSION_PATCH != 1)

				#pragma message("[MUAF] muUtility's header has already been defined, but version doesn't match the version that this library is built for. This may lead to errors, warnings, or unexpected behavior. Define MU_CHECK_VERSION_MISMATCHING before this to turn off this message.")

			#endif

			#ifndef MUU_H
				#define MUU_H

				// @DOCLINE # Version
					// @DOCLINE The macros `MUU_VERSION_MAJOR`, `MUU_VERSION_MINOR`, and `MUU_VERSION_PATCH` are defined to match its respective release version, following the formatting of `MAJOR.MINOR.PATCH`.
					
					#define MUU_VERSION_MAJOR 2
					#define MUU_VERSION_MINOR 0
					#define MUU_VERSION_PATCH 1

				// @DOCLINE # `MUDEF`
					// @DOCLINE The `MUDEF` macro is used by virtually all mu libraries, and is generally added before a header-defined variable or function. Its default value is `extern`, but can be changed to `static` by defining `MU_STATIC` before the header section of muUtility is defined. Its value can also be overwritten entirely to anything else by directly defining `MUDEF`.
					
					#ifndef MUDEF
						#ifdef MU_STATIC
							#define MUDEF static
						#else
							#define MUDEF extern
						#endif
					#endif
				
				// @DOCLINE # Secure warnings
					// @DOCLINE mu libraries often use non-secure functions that will trigger warnings on certain compilers. These warnings are, to put it lightly, dumb, so muUtility defines `_CRT_SECURE_NO_WARNINGS`. However, it is not guaranteed that this definition will actually turn the warnings off, which, at that point, they have to be manually turned off by the user. This functionality can be turned off by defining `MU_SECURE_WARNINGS`.
					#if !defined(MU_SECURE_WARNINGS) && !defined(_CRT_SECURE_NO_WARNINGS)
						#define _CRT_SECURE_NO_WARNINGS
					#endif
				
				// @DOCLINE # C++ extern
					// @DOCLINE Every mu library's primary code externs "C" within the main chunks of their code; macros `MU_CPP_EXTERN_START` and `MU_CPP_EXTERN_END` are defined to make this process easier, and would read like this:
					/* @DOCBEGIN
					```
					MU_CPP_EXTERN_START

					// Library code goes here...

					MU_CPP_EXTERN_END
					```
					@DOCEND */
					#ifdef __cplusplus
						#define MU_CPP_EXTERN_START extern "C" {
						#define MU_CPP_EXTERN_END   }
					#else
						#define MU_CPP_EXTERN_START
						#define MU_CPP_EXTERN_END
					#endif
				
				MU_CPP_EXTERN_START

				// @DOCLINE # C standard library dependencies

					// @DOCLINE muUtility has several C standard library dependencies, all of which are overridable by defining them before the inclusion of the file. The following is a list of those dependencies.

					// @DOCLINE ## `stdint.h` dependencies
					#if !defined(int8_m) || \
						!defined(uint8_m) || \
						!defined(int16_m) || \
						!defined(uint16_m) || \
						!defined(int32_m) || \
						!defined(uint32_m) || \
						!defined(int64_m) || \
						!defined(uint64_m) || \
						!defined(MU_SIZE_MAX)

						#define __STDC_LIMIT_MACROS
						#define __STDC_CONSTANT_MACROS
						#include <stdint.h>
						
						// @DOCLINE * `int8_m` - equivalent to `int8_t` if `INT8_MAX` is defined; `char` if otherwise.
						#ifndef int8_m
							#ifdef INT8_MAX
								#define int8_m int8_t
							#else
								#define int8_m char
							#endif
						#endif

						// @DOCLINE * `uint8_m` - equivalent to `uint8_t` if `UINT8_MAX` is defined; `unsigned char` if otherwise.
						#ifndef uint8_m
							#ifdef UINT8_MAX
								#define uint8_m uint8_t
							#else
								#define uint8_m unsigned char
							#endif
						#endif

						// @DOCLINE * `int16_m` - equivalent to `int16_t` if `INT16_MAX` is defined; `short` if otherwise.
						#ifndef int16_m
							#ifdef INT16_MAX
								#define int16_m int16_t
							#else
								#define int16_m short
							#endif
						#endif

						// @DOCLINE * `uint16_m` - equivalent to `uint16_t` if `UINT16_MAX` is defined; `unsigned short` if otherwise.
						#ifndef uint16_m
							#ifdef UINT16_MAX
								#define uint16_m uint16_t
							#else
								#define uint16_m unsigned short
							#endif
						#endif

						// @DOCLINE * `int32_m` - equivalent to `int32_t` if `INT32_MAX` is defined; `long` if otherwise.
						#ifndef int32_m
							#ifdef INT32_MAX
								#define int32_m int32_t
							#else
								#define int32_m long
							#endif
						#endif

						// @DOCLINE * `uint32_m` - equivalent to `uint32_t` if `UINT32_MAX` is defined; `unsigned long` if otherwise.
						#ifndef uint32_m
							#ifdef UINT32_MAX
								#define uint32_m uint32_t
							#else
								#define uint32_m unsigned long
							#endif
						#endif

						// @DOCLINE * `int64_m` - equivalent to `int64_t` if `INT64_MAX` is defined; `long long` if otherwise.
						#ifndef int64_m
							#ifdef INT64_MAX
								#define int64_m int64_t
							#else
								#define int64_m long long
							#endif
						#endif

						// @DOCLINE * `uint64_m` - equivalent to `uint64_t` if `UINT64_MAX` is defined; `unsigned long long` if otherwise.
						#ifndef uint64_m
							#ifdef UINT64_MAX
								#define uint64_m uint64_t
							#else
								#define uint64_m unsigned long long
							#endif
						#endif

						// @DOCLINE * `MU_SIZE_MAX` - equivalent to `SIZE_MAX`.
						#ifndef MU_SIZE_MAX
							#define MU_SIZE_MAX SIZE_MAX
						#endif

					#endif /* stdint.h */

					// @DOCLINE ## `stddef.h` dependencies
					#if !defined(size_m)

						#include <stddef.h>

						// @DOCLINE * `size_m` - equivalent to `size_t`.
						#ifndef size_m
							#define size_m size_t
						#endif

					#endif /* stddef.h */

					// @DOCLINE ## `stdbool.h` dependencies
					#if !defined(muBool) || \
						!defined(MU_TRUE) || \
						!defined(MU_FALSE)

						#include <stdbool.h>

						// @DOCLINE * `muBool` - equivalent to `bool`.
						#ifndef muBool
							#define muBool bool
						#endif

						// @DOCLINE * `MU_TRUE` - equivalent to `true`.
						#ifndef MU_TRUE
							#define MU_TRUE true
						#endif

						// @DOCLINE * `MU_FALSE` - equivalent to `false`.
						#ifndef MU_FALSE
							#define MU_FALSE false
						#endif

					#endif /* stdbool.h */

				// @DOCLINE # Zero struct

					// @DOCLINE There are two macros, `MU_ZERO_STRUCT` and `MU_ZERO_STRUCT_CONST`, which are functions used to zero-out a struct's contents, with their only parameter being the struct type. The reason this needs to be defined is because the way C and C++ syntax handles an empty struct are different, and need to be adjusted for. These macros are overridable by defining them before `muUtility.h` is included.

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

				// @DOCLINE # Byte manipulation

					// @DOCLINE muUtility defines several inline functions that read a value from a given array of bytes. Internally, they're all defined with the prefix `muu_...`, and then a macro is defined for them as `MU_...` (with change in capitalization after the prefix as well). The macros for these functions can be overridden, and, in such case, the original function will go undefined. For example, the function `muu_rleu8` is primarily referenced via the macro `MU_RLEU8`, and if `MU_RLEU8` is overridden, `muu_rleu8` is never defined and is not referenceable.

					// @DOCLINE All reading functions take in a pointer of bytes as their only parameter and have a return type of the fixed-width size of bits in question; for example, `muu_rleu8` is defined as:

					/* @DOCBEGIN
					```
					MUDEF inline uint8_m muu_rleu8(muByte* b);
					```
					@DOCEND */

					// @DOCLINE All writing functions take in a pointer of bytes as their first parameter, the number to be written as the second parameter, and have a return type of void; for example, `muu_wleu8` is defined as:

					/* @DOCBEGIN
					```
					MUDEF inline void muu_wleu8(muByte* b, uint8_m n);
					```
					@DOCEND */

					// @DOCLINE The exception to this is 24-bit, in which case, the fixed-width types are 32-bit (`uint32_m` / `int32_m`).

					// @DOCLINE ## Byte type

						// @DOCLINE muUtility defines the type `muByte` to refer to a byte. It is defined as `uint8_m`, and is overridable.
						#ifndef muByte
							#define muByte uint8_m
						#endif

					// @DOCLINE ## 8-bit

						// @DOCLINE The following macros exist for byte manipulation regarding 8-bit integers:

						// @DOCLINE * `MU_RLEU8` - reads an unsigned 8-bit integer from little-endian byte data; overridable macro to `muu_rleu8`.
						#ifndef MU_RLEU8
							MUDEF inline uint8_m muu_rleu8(muByte* b) {
								return b[0];
							}
							#define MU_RLEU8 muu_rleu8
						#endif

						// @DOCLINE * `MU_WLEU8` - writes an unsigned 8-bit integer to little-endian byte data; overridable macro to `muu_wleu8`.
						#ifndef MU_WLEU8
							MUDEF inline void muu_wleu8(muByte* b, uint8_m n) {
								b[0] = n;
							}
							#define MU_WLEU8 muu_wleu8
						#endif

						// @DOCLINE * `MU_RLES8` - reads a signed 8-bit integer from little-endian byte data; overridable macro to `muu_rles8`.
						#ifndef MU_RLES8
							MUDEF inline int8_m muu_rles8(muByte* b) {
								// I'm PRETTY sure this is okay...
								return *(int8_m*)b;
							}
							#define MU_RLES8 muu_rles8
						#endif

						// @DOCLINE * `MU_WLES8` - writes a signed 8-bit integer to little-endian byte data; overridable macro to `muu_wles8`.
						#ifndef MU_WLES8
							MUDEF inline void muu_wles8(muByte* b, int8_m n) {
								((int8_m*)(b))[0] = n;
							}
							#define MU_WLES8 muu_wles8
						#endif

						// @DOCLINE * `MU_RBEU8` - reads an unsigned 8-bit integer from big-endian byte data; overridable macro to `muu_rbeu8`.
						#ifndef MU_RBEU8
							MUDEF inline uint8_m muu_rbeu8(muByte* b) {
								return b[0];
							}
							#define MU_RBEU8 muu_rbeu8
						#endif

						// @DOCLINE * `MU_WBEU8` - writes an unsigned 8-bit integer to big-endian byte data; overridable macro to `muu_wbeu8`.
						#ifndef MU_WBEU8
							MUDEF inline void muu_wbeu8(muByte* b, uint8_m n) {
								b[0] = n;
							}
							#define MU_WBEU8 muu_wbeu8
						#endif

						// @DOCLINE * `MU_RBES8` - reads a signed 8-bit integer from big-endian byte data; overridable macro to `muu_rbes8`.
						#ifndef MU_RBES8
							MUDEF inline int8_m muu_rbes8(muByte* b) {
								return *(int8_m*)b;
							}
							#define MU_RBES8 muu_rbes8
						#endif

						// @DOCLINE * `MU_WBES8` - writes a signed 8-bit integer to big-endian byte data; overridable macro to `muu_wbes8`.
						#ifndef MU_WBES8
							MUDEF inline void muu_wbes8(muByte* b, int8_m n) {
								((int8_m*)(b))[0] = n;
							}
							#define MU_WBES8 muu_wbes8
						#endif

					// @DOCLINE ## 16-bit

						// @DOCLINE The following macros exist for byte manipulation regarding 16-bit integers:

						// @DOCLINE * `MU_RLEU16` - reads an unsigned 16-bit integer from little-endian byte data; overridable macro to `muu_rleu16`.
						#ifndef MU_RLEU16
							MUDEF inline uint16_m muu_rleu16(muByte* b) {
								return (
									((uint16_m)(b[0]) << 0) |
									((uint16_m)(b[1]) << 8)
								);
							}
							#define MU_RLEU16 muu_rleu16
						#endif

						// @DOCLINE * `MU_WLEU16` - writes an unsigned 16-bit integer to little-endian byte data; overridable macro to `muu_wleu16`.
						#ifndef MU_WLEU16
							MUDEF inline void muu_wleu16(muByte* b, uint16_m n) {
								b[0] = (uint8_m)(n >> 0);
								b[1] = (uint8_m)(n >> 8);
							}
							#define MU_WLEU16 muu_wleu16
						#endif

						// @DOCLINE * `MU_RLES16` - reads a signed 16-bit integer from little-endian byte data; overridable macro to `muu_rles16`.
						#ifndef MU_RLES16
							MUDEF inline int16_m muu_rles16(muByte* b) {
								uint16_m u16 = muu_rleu16(b);
								return *(int16_m*)&u16;
							}
							#define MU_RLES16 muu_rles16
						#endif

						// @DOCLINE * `MU_WLES16` - writes a signed 16-bit integer to little-endian byte data; overridable macro to `muu_wles16`.
						#ifndef MU_WLES16
							MUDEF inline void muu_wles16(muByte* b, int16_m n) {
								uint16_m un = *(uint16_m*)&n;
								b[0] = (uint8_m)(un >> 0);
								b[1] = (uint8_m)(un >> 8);
							}
							#define MU_WLES16 muu_wles16
						#endif

						// @DOCLINE * `MU_RBEU16` - reads an unsigned 16-bit integer from big-endian byte data; overridable macro to `muu_rbeu16`.
						#ifndef MU_RBEU16
							MUDEF inline uint16_m muu_rbeu16(muByte* b) {
								return (
									((uint16_m)(b[1]) << 0) |
									((uint16_m)(b[0]) << 8)
								);
							}
							#define MU_RBEU16 muu_rbeu16
						#endif

						// @DOCLINE * `MU_WBEU16` - writes an unsigned 16-bit integer to big-endian byte data; overridable macro to `muu_wbeu16`.
						#ifndef MU_WBEU16
							MUDEF inline void muu_wbeu16(muByte* b, uint16_m n) {
								b[1] = (uint8_m)(n >> 0);
								b[0] = (uint8_m)(n >> 8);
							}
							#define MU_WBEU16 muu_wbeu16
						#endif

						// @DOCLINE * `MU_RBES16` - reads a signed 16-bit integer from big-endian byte data; overridable macro to `muu_rbes16`.
						#ifndef MU_RBES16
							MUDEF inline int16_m muu_rbes16(muByte* b) {
								uint16_m u16 = muu_rbeu16(b);
								return *(int16_m*)&u16;
							}
							#define MU_RBES16 muu_rbes16
						#endif

						// @DOCLINE * `MU_WBES16` - writes a signed 16-bit integer to big-endian byte data; overridable macro to `muu_wbes16`.
						#ifndef MU_WBES16
							MUDEF inline void muu_wbes16(muByte* b, int16_m n) {
								uint16_m un = *(uint16_m*)&n;
								b[1] = (uint8_m)(un >> 0);
								b[0] = (uint8_m)(un >> 8);
							}
							#define MU_WBES16 muu_wbes16
						#endif

					// @DOCLINE ## 24-bit

						// @DOCLINE The following macros exist for byte manipulation regarding 24-bit integers:

						// @DOCLINE * `MU_RLEU24` - reads an unsigned 24-bit integer from little-endian byte data; overridable macro to `muu_rleu24`.
						#ifndef MU_RLEU24
							MUDEF inline uint32_m muu_rleu24(muByte* b) {
								return (
									((uint32_m)(b[0]) << 0) |
									((uint32_m)(b[1]) << 8) |
									((uint32_m)(b[2]) << 16)
								);
							}
							#define MU_RLEU24 muu_rleu24
						#endif

						// @DOCLINE * `MU_WLEU24` - writes an unsigned 24-bit integer to little-endian byte data; overridable macro to `muu_wleu24`.
						#ifndef MU_WLEU24
							MUDEF inline void muu_wleu24(muByte* b, uint32_m n) {
								b[0] = (uint8_m)(n >> 0);
								b[1] = (uint8_m)(n >> 8);
								b[2] = (uint8_m)(n >> 16);
							}
							#define MU_WLEU24 muu_wleu24
						#endif

						// @DOCLINE * `MU_RLES24` - reads a signed 24-bit integer from little-endian byte data; overridable macro to `muu_rles24`.
						#ifndef MU_RLES24
							MUDEF inline int32_m muu_rles24(muByte* b) {
								uint32_m u24 = muu_rleu24(b);
								return *(int32_m*)&u24;
							}
							#define MU_RLES24 muu_rles24
						#endif

						// @DOCLINE * `MU_WLES24` - writes a signed 24-bit integer to little-endian byte data; overridable macro to `muu_wles24`.
						#ifndef MU_WLES24
							MUDEF inline void muu_wles24(muByte* b, int32_m n) {
								// Probably definitely doesn't work with signed integers; fix later
								uint32_m un = *(uint32_m*)&n;
								b[0] = (uint8_m)(un >> 0);
								b[1] = (uint8_m)(un >> 8);
								b[2] = (uint8_m)(un >> 16);
							}
							#define MU_WLES24 muu_wles24
						#endif

						// @DOCLINE * `MU_RBEU24` - reads an unsigned 24-bit integer from big-endian byte data; overridable macro to `muu_rbeu24`.
						#ifndef MU_RBEU24
							MUDEF inline uint32_m muu_rbeu24(muByte* b) {
								return (
									((uint32_m)(b[2]) << 0) |
									((uint32_m)(b[1]) << 8) |
									((uint32_m)(b[0]) << 16)
								);
							}
							#define MU_RBEU24 muu_rbeu24
						#endif

						// @DOCLINE * `MU_WBEU24` - writes an unsigned 24-bit integer to big-endian byte data; overridable macro to `muu_wbeu24`.
						#ifndef MU_WBEU24
							MUDEF inline void muu_wbeu24(muByte* b, uint32_m n) {
								b[2] = (uint8_m)(n >> 0);
								b[1] = (uint8_m)(n >> 8);
								b[0] = (uint8_m)(n >> 16);
							}
							#define MU_WBEU24 muu_wbeu24
						#endif

						// @DOCLINE * `MU_RBES24` - reads a signed 24-bit integer from big-endian byte data; overridable macro to `muu_rbes24`.
						#ifndef MU_RBES24
							MUDEF inline int32_m muu_rbes24(muByte* b) {
								uint32_m u24 = muu_rbeu24(b);
								return *(int32_m*)&u24;
							}
							#define MU_RBES24 muu_rbes24
						#endif

						// @DOCLINE * `MU_WBES24` - writes a signed 24-bit integer to big-endian byte data; overridable macro to `muu_wbes24`.
						#ifndef MU_WBES24
							MUDEF inline void muu_wbes24(muByte* b, int32_m n) {
								uint32_m un = *(uint32_m*)&n;
								b[2] = (uint8_m)(un >> 0);
								b[1] = (uint8_m)(un >> 8);
								b[0] = (uint8_m)(un >> 16);
							}
							#define MU_WBES24 muu_wbes24
						#endif

					// @DOCLINE ## 32-bit

						// @DOCLINE The following macros exist for byte manipulation regarding 32-bit integers:

						// @DOCLINE * `MU_RLEU32` - reads an unsigned 32-bit integer from little-endian byte data; overridable macro to `muu_rleu32`.
						#ifndef MU_RLEU32
							MUDEF inline uint32_m muu_rleu32(muByte* b) {
								return (
									((uint32_m)(b[0]) << 0)  |
									((uint32_m)(b[1]) << 8)  |
									((uint32_m)(b[2]) << 16) |
									((uint32_m)(b[3]) << 24)
								);
							}
							#define MU_RLEU32 muu_rleu32
						#endif

						// @DOCLINE * `MU_WLEU32` - writes an unsigned 32-bit integer to little-endian byte data; overridable macro to `muu_wleu32`.
						#ifndef MU_WLEU32
							MUDEF inline void muu_wleu32(muByte* b, uint32_m n) {
								b[0] = (uint8_m)(n >> 0);
								b[1] = (uint8_m)(n >> 8);
								b[2] = (uint8_m)(n >> 16);
								b[3] = (uint8_m)(n >> 24);
							}
							#define MU_WLEU32 muu_wleu32
						#endif

						// @DOCLINE * `MU_RLES32` - reads a signed 32-bit integer from little-endian byte data; overridable macro to `muu_rles32`.
						#ifndef MU_RLES32
							MUDEF inline int32_m muu_rles32(muByte* b) {
								uint32_m u32 = muu_rleu32(b);
								return *(int32_m*)&u32;
							}
							#define MU_RLES32 muu_rles32
						#endif

						// @DOCLINE * `MU_WLES32` - writes a signed 32-bit integer to little-endian byte data; overridable macro to `muu_wles32`.
						#ifndef MU_WLES32
							MUDEF inline void muu_wles32(muByte* b, int32_m n) {
								uint32_m un = *(uint32_m*)&n;
								b[0] = (uint8_m)(un >> 0);
								b[1] = (uint8_m)(un >> 8);
								b[2] = (uint8_m)(un >> 16);
								b[3] = (uint8_m)(un >> 24);
							}
							#define MU_WLES32 muu_wles32
						#endif

						// @DOCLINE * `MU_RBEU32` - reads an unsigned 32-bit integer from big-endian byte data; overridable macro to `muu_rbeu32`.
						#ifndef MU_RBEU32
							MUDEF inline uint32_m muu_rbeu32(muByte* b) {
								return (
									((uint32_m)(b[3]) << 0)  |
									((uint32_m)(b[2]) << 8)  |
									((uint32_m)(b[1]) << 16) |
									((uint32_m)(b[0]) << 24)
								);
							}
							#define MU_RBEU32 muu_rbeu32
						#endif

						// @DOCLINE * `MU_WBEU32` - writes an unsigned 32-bit integer to big-endian byte data; overridable macro to `muu_wbeu32`.
						#ifndef MU_WBEU32
							MUDEF inline void muu_wbeu32(muByte* b, uint32_m n) {
								b[3] = (uint8_m)(n >> 0);
								b[2] = (uint8_m)(n >> 8);
								b[1] = (uint8_m)(n >> 16);
								b[0] = (uint8_m)(n >> 24);
							}
							#define MU_WBEU32 muu_wbeu32
						#endif

						// @DOCLINE * `MU_RBES32` - reads a signed 32-bit integer from big-endian byte data; overridable macro to `muu_rbes32`.
						#ifndef MU_RBES32
							MUDEF inline int32_m muu_rbes32(muByte* b) {
								uint32_m u32 = muu_rbeu32(b);
								return *(int32_m*)&u32;
							}
							#define MU_RBES32 muu_rbes32
						#endif

						// @DOCLINE * `MU_WBES32` - writes a signed 32-bit integer to big-endian byte data; overridable macro to `muu_wbes32`.
						#ifndef MU_WBES32
							MUDEF inline void muu_wbes32(muByte* b, int32_m n) {
								uint32_m un = *(uint32_m*)&n;
								b[3] = (uint8_m)(un >> 0);
								b[2] = (uint8_m)(un >> 8);
								b[1] = (uint8_m)(un >> 16);
								b[0] = (uint8_m)(un >> 24);
							}
							#define MU_WBES32 muu_wbes32
						#endif

					// @DOCLINE ## 64-bit

						// @DOCLINE The following macros exist for byte manipulation regarding 64-bit integers:

						// @DOCLINE * `MU_RLEU64` - reads an unsigned 64-bit integer from little-endian byte data; overridable macro to `muu_rleu64`.
						#ifndef MU_RLEU64
							MUDEF inline uint64_m muu_rleu64(muByte* b) {
								return (
									((uint64_m)(b[0]) << 0)  |
									((uint64_m)(b[1]) << 8)  |
									((uint64_m)(b[2]) << 16) |
									((uint64_m)(b[3]) << 24) |
									((uint64_m)(b[4]) << 32) |
									((uint64_m)(b[5]) << 40) |
									((uint64_m)(b[6]) << 48) |
									((uint64_m)(b[7]) << 56)
								);
							}
							#define MU_RLEU64 muu_rleu64
						#endif

						// @DOCLINE * `MU_WLEU64` - writes an unsigned 64-bit integer to little-endian byte data; overridable macro to `muu_wleu64`.
						#ifndef MU_WLEU64
							MUDEF inline void muu_wleu64(muByte* b, uint64_m n) {
								b[0] = (uint8_m)(n >> 0);
								b[1] = (uint8_m)(n >> 8);
								b[2] = (uint8_m)(n >> 16);
								b[3] = (uint8_m)(n >> 24);
								b[4] = (uint8_m)(n >> 32);
								b[5] = (uint8_m)(n >> 40);
								b[6] = (uint8_m)(n >> 48);
								b[7] = (uint8_m)(n >> 56);
							}
							#define MU_WLEU64 muu_wleu64
						#endif

						// @DOCLINE * `MU_RLES64` - reads a signed 64-bit integer from little-endian byte data; overridable macro to `muu_rles64`.
						#ifndef MU_RLES64
							MUDEF inline int64_m muu_rles64(muByte* b) {
								uint64_m u64 = muu_rleu64(b);
								return *(int64_m*)&u64;
							}
							#define MU_RLES64 muu_rles64
						#endif

						// @DOCLINE * `MU_WLES64` - writes a signed 64-bit integer to little-endian byte data; overridable macro to `muu_wles64`.
						#ifndef MU_WLES64
							MUDEF inline void muu_wles64(muByte* b, int64_m n) {
								uint64_m un = *(uint64_m*)&n;
								b[0] = (uint8_m)(un >> 0);
								b[1] = (uint8_m)(un >> 8);
								b[2] = (uint8_m)(un >> 16);
								b[3] = (uint8_m)(un >> 24);
								b[4] = (uint8_m)(un >> 32);
								b[5] = (uint8_m)(un >> 40);
								b[6] = (uint8_m)(un >> 48);
								b[7] = (uint8_m)(un >> 56);
							}
							#define MU_WLES64 muu_wles64
						#endif

						// @DOCLINE * `MU_RBEU64` - reads an unsigned 64-bit integer from big-endian byte data; overridable macro to `muu_rbeu64`.
						#ifndef MU_RBEU64
							MUDEF inline uint64_m muu_rbeu64(muByte* b) {
								return (
									((uint64_m)(b[7]) << 0)  |
									((uint64_m)(b[6]) << 8)  |
									((uint64_m)(b[5]) << 16) |
									((uint64_m)(b[4]) << 24) |
									((uint64_m)(b[3]) << 32) |
									((uint64_m)(b[2]) << 40) |
									((uint64_m)(b[1]) << 48) |
									((uint64_m)(b[0]) << 56)
								);
							}
							#define MU_RBEU64 muu_rbeu64
						#endif

						// @DOCLINE * `MU_WBEU64` - writes an unsigned 64-bit integer to big-endian byte data; overridable macro to `muu_wbeu64`.
						#ifndef MU_WBEU64
							MUDEF inline void muu_wbeu64(muByte* b, uint64_m n) {
								b[7] = (uint8_m)(n >> 0);
								b[6] = (uint8_m)(n >> 8);
								b[5] = (uint8_m)(n >> 16);
								b[4] = (uint8_m)(n >> 24);
								b[3] = (uint8_m)(n >> 32);
								b[2] = (uint8_m)(n >> 40);
								b[1] = (uint8_m)(n >> 48);
								b[0] = (uint8_m)(n >> 56);
							}
							#define MU_WBEU64 muu_wbeu64
						#endif

						// @DOCLINE * `MU_RBES64` - reads a signed 64-bit integer from big-endian byte data; overridable macro to `muu_rbes64`.
						#ifndef MU_RBES64
							MUDEF inline int64_m muu_rbes64(muByte* b) {
								uint64_m u64 = muu_rbeu64(b);
								return *(int64_m*)&u64;
							}
							#define MU_RBES64 muu_rbes64
						#endif

						// @DOCLINE * `MU_WBES64` - writes a signed 64-bit integer to big-endian byte data; overridable macro to `muu_wbes64`.
						#ifndef MU_WBES64
							MUDEF inline void muu_wbes64(muByte* b, int64_m n) {
								uint64_m un = *(uint64_m*)&n;
								b[7] = (uint8_m)(un >> 0);
								b[6] = (uint8_m)(un >> 8);
								b[5] = (uint8_m)(un >> 16);
								b[4] = (uint8_m)(un >> 24);
								b[3] = (uint8_m)(un >> 32);
								b[2] = (uint8_m)(un >> 40);
								b[1] = (uint8_m)(un >> 48);
								b[0] = (uint8_m)(un >> 56);
							}
							#define MU_WBES64 muu_wbes64
						#endif

				// @DOCLINE # Set result

					/* @DOCBEGIN

					The `MU_SET_RESULT(res, val)` macro is an overridable function that checks if the given parameter `res` is a null pointer. If it is, it does nothing, but if it isn't, it dereferences the pointer and sets the value to `val`. This macro saves a lot of code, shrinking down what would be this:

					```c
					if (result) {
						*result = ...;
					}
					```

					into this:

					```c
					MU_SET_RESULT(result, ...)
					```

					@DOCEND */

					#ifndef MU_SET_RESULT
						#define MU_SET_RESULT(res, val) if(res){*res=val;}
					#endif

				// @DOCLINE # Enum

					/* @DOCBEGIN

					The `MU_ENUM(name, ...)` macro is an overridable function used to declare an enumerator. `name` is the name of the enumerator type, and `...` are all of the values. The reason why one would prefer this over the traditional way of declaring enumerators is because it actually makes it a `size_m`, which can avoid errors on certain compilers (looking at you, Microsoft) in regards to treating enumerators like values. It expands like this:

					```c
					enum _##name {
						__VA_ARGS__
					};
					typedef enum _##name _##name;
					typedef size_m name;
					```

					@DOCEND */

					#define MU_ENUM(name, ...) enum _##name{__VA_ARGS__};typedef enum _##name _##name;typedef size_m name;

				// @DOCLINE # Operating system recognition

					/* @DOCBEGIN

					The macros `MU_WIN32` or `MU_LINUX` are defined (if neither were defined before) in order to allow mu libraries to easily check if they're running on a Windows or Linux system.

					`MU_WIN32` will be defined if `WIN32` or `_WIN32` are defined, one of which is usually pre-defined on Windows systems.

					`MU_LINUX` will be defined if `__linux__` is defined.

					@DOCEND */

					#if !defined(MU_WIN32) && !defined(MU_LINUX)
						#if defined(WIN32) || defined(_WIN32)
							#define MU_WIN32
						#endif
						#if defined(__linux__)
							#define MU_LINUX
						#endif
					#endif

				MU_CPP_EXTERN_END

			#endif /* MUU_H */
		// @ATTENTION

		// @DOCLINE > Note that mu libraries store their dependencies within their files, so you don't need to import these dependencies yourself; this section's purpose is purely to provide more information about the contents that this file defines. The libraries listed may also have other dependencies that they also include that aren't explicitly listed here.

	MU_CPP_EXTERN_START

	typedef uint32_m muafResult;

	// @DOCLINE # Terminology

		// @DOCLINE This section provides a general overview of muaf's terminology in relation to audio and how audio data is handled in muaf, since a significant portion of audio terminology is not agreed upon.

		// @DOCLINE The smallest unit of audio data is a '***sample***', which represents a single point of audio signal amplitude at one moment in time. In an audio file, multiple samples can be provided for the same moment in time in the form of '***channels***', which can be used for [stereo audio](https://en.wikipedia.org/wiki/Stereophonic_sound) and such. A '***frame***' contains each sample per channel, interleaved one after the other. So, if a frame had two samples it in, the first sample would be for channel 0, and the second sample would be for channel 1.

		// @DOCLINE Audio files are expected to be played at a certain integer '***sample rate***', indicating how many samples *per channel* should be played each second. Since the sample rate value is based on one channel, the sample rate does not increase along with the amount of channels; the sample rate can more accurately described as the 'frame rate', as it more concisely indicates the amount of frames per second, but since the term 'frame rate' is already commonly associated with [video frame rate](https://en.wikipedia.org/wiki/Frame_rate), and the term 'sample rate' is fairly standardized, muaf uses the term 'sample rate'.

		// @DOCLINE An '***audio format***' refers to how audio data can be laid out in a manner recognized and supported by muaf. In an '***uncompressed***' audio format, frames are simply stored one after the other chronologically, and in a type whose sample values are directly readable as an integer or decimal value in C. In a '***compressed***' audio format, frames are *not* stored one after the other, but instead are organized into '***packets***', which can contain any number of '***explicit or implicit frames***'. If frames are explicitly listed in a packet, that means that all frames' values are listed out one after the other in an uncompressed manner within that packet. If frames are implicitly listed, that means that some frames' values are not directly laid out in the data, but are instead implied based on other values, and need to be worked out manually to retrieve any readable frame data.

		// @DOCLINE An audio file's '***raw data***' refers to the raw audio data based on the audio format. This means that, for example, if the audio format is compressed, the raw data describes the frames as they're listed in the data, explicit or implicit, leaving it to the user to work out the sample values for the frames manually if they are implicit.

		// @DOCLINE An audio file's '***decompressed data***' refers to uncompressed audio data, whether or not it was generated from compressed or uncompressed audio data. If the given audio data's format is uncompressed, then the decompressed data simply refers to the raw data. However, if the given audio data's format is compressed, then the decompressed data refers to the raw data's equivalent uncompressed audio data. In other words, the decompressed data refers to the decompressed version of the raw data (whether or not it was compressed in the first place).

		// @DOCLINE The terminology listed above for muaf does not apply when muaf is referencing the specification of another audio file format. In those instances, the specification's terminology applies.

	// @DOCLINE # Format-unspecific reading API

		// @DOCLINE muaf's reading API is split into two sections: the format-specific API, and the format-unspecific API. The unspecific API wraps around the specific API, encapsulating the specific API's functionality, and allowing you to retrieve general information about an audio file without having to directly consider what specific audio file format it is.

		// @DOCLINE ## Audio profile

			// @DOCLINE For each audio format, general information about the audio stored in the file can be retrieved. This general information is called the "profile". This profile is defined for several reasons:

			// @DOCLINE * It gives the user a way to load general information about the audio file first, before taking more processing time and memory to load more significant portions of the data stored within the audio file.

			// @DOCLINE * Having this information gives the user a way to load select portions of the audio data encoded in the file without ever having to load the whole audio file at once in memory, which can be useful or even necessary for larger audio files.

			// @DOCLINE * It allows the API to internally map how the file is formatted and hold onto that information for later, which can make subsequent calls to load information from the audio file quicker, since information about the file's general structure is held onto across function calls.

			// @DOCLINE In the format-specific API, the profile stores general information that very directly corresponds to the general information stored in the audio file itself. For example, [the WAVE profile](#wave-profile) stores information about what supported chunks are provided in the file and where they are. It also stores the information within certain chunks that provide general information about the audio, such as the fmt chunk, which provides information such as the number of channels.

			// @DOCLINE In the format-unspecific API, the profile primarily stores general information that all of the supported audio file formats within muaf have in common. This allows the user to retrieve information about the audio file without having to know how to retrieve it from the exact audio file format that it's in. The unspecific profile also loads and stores the specific profile information.

			typedef struct muafUnspecificProfile muafUnspecificProfile;
			// @DOCLINE An audio file's unspecific profile can be retrieved using the function `mu_get_audio_file_profile`, defined below: @NLNT
			MUDEF muafResult mu_get_audio_file_profile(const char* filename, muafUnspecificProfile* profile);

			// @DOCLINE Once an unspecific profile has been successfully or non-fatally retrieved (indicated by [the result return value](#result)), the profile is filled with data, some of which may be manually allocated automatically. To free this data, use the function `mu_free_audio_file_profile`, defined below: @NLNT
			MUDEF void mu_free_audio_file_profile(muafUnspecificProfile* profile);

			// @DOCLINE The unspecific profile is represented by the struct `muafUnspecificProfile`, which has the following members:

			// This union is described later, make sure it's updated properly!
			typedef struct muWAVEProfile muWAVEProfile;
			union muafSpecificProfile {
				muWAVEProfile* WAVE;
			};
			typedef union muafSpecificProfile muafSpecificProfile;

			typedef uint32_m muafSampleRate;
			typedef uint64_m muafFrames;
			typedef uint16_m muafChannels;
			typedef uint8_m muafFileFormat;
			typedef uint32_m muafAudioFormat;
			struct muafUnspecificProfile {
				// @DOCLINE * `@NLFT num_frames` - the number of frames (`@NLFT` typedef for `uint64_m`).
				muafFrames num_frames;
				// @DOCLINE * `@NLFT sample_rate` - the amount of samples that should be played every second per channel (`@NLFT` typedef for `uint32_m`).
				muafSampleRate sample_rate;
				// @DOCLINE * `@NLFT num_channels` - the number of channels (`@NLFT` typedef for `uint16_m`).
				muafChannels num_channels;
				// @DOCLINE * `@NLFT audio_format` - the [audio format](#audio-formats) (`@NLFT` typedef for `uint32_m`).
				muafAudioFormat audio_format;
				// @DOCLINE * `@NLFT file_format` - the [file format](#audio-file-formats) (`@NLFT` typedef for `uint8_m`).
				muafFileFormat file_format;
				// @DOCLINE * `@NLFT specific` - the [format-specific profile](#format-specific-profile).
				muafSpecificProfile specific;
			};

			// @DOCLINE ### Audio formats

				// @DOCLINE The type `muafAudioFormat` (typedef for `uint32_m`) represents an audio format supported in muaf. Each audio format has a corresponding type that represents how each sample/packet is stored. If an audio format's corresponding type is an integer/decimal type, then the audio format is uncompressed, and vice versa.

				// @DOCLINE The type `muafAudioFormat` has the following defined values:

				// @DOCLINE * `MUAF_FORMAT_UNKNOWN` - unknown or unsupported audio format.
				#define MUAF_FORMAT_UNKNOWN 0

				// @DOCLINE #### PCM audio formats
				// 1 to 63 //
					// @DOCLINE * `MUAF_FORMAT_PCM_U8` - unsigned 8-bit PCM (range 0 to 255, 0x00 to 0xFF). Corresponding type is `uint8_m`.
					#define MUAF_FORMAT_PCM_U8  1

					// #define MUAF_FORMAT_PCM_S8  2
					// #define MUAF_FORMAT_PCM_U16 3

					// @DOCLINE * `MUAF_FORMAT_PCM_S16` - signed 16-bit PCM (range -32768 to 32767, -0x8000 to 0x7FFF). Corresponding type is `int16_m`.
					#define MUAF_FORMAT_PCM_S16 4

					// #define MUAF_FORMAT_PCM_U24 5
					// #define MUAF_FORMAT_PCM_S24 6
					// #define MUAF_FORMAT_PCM_U32 7

					// @DOCLINE * `MUAF_FORMAT_PCM_S32` - signed 32-bit PCM (range -2147483648 to 2147483647, -0x80000000 to 0x7FFFFFFF). Corresponding type is `int32_m`.
					#define MUAF_FORMAT_PCM_S32 8

					// #define MUAF_FORMAT_PCM_U64 9

					// @DOCLINE * `MUAF_FORMAT_PCM_S64` - signed 64-bit PCM (range -9223372036854775808 to 9223372036854775807, -0x8000000000000000 to 0x7FFFFFFFFFFFFFFF). Corresponding type is `int64_m`.
					#define MUAF_FORMAT_PCM_S64 10

				// @DOCLINE #### Audio format names

					#ifdef MUAF_NAMES

					// @DOCLINE The name function `muaf_audio_format_get_name` returns a `const char*` representation of a given audio format (for example, `MUAF_FORMAT_PCM_U8` returns "MUAF_FORMAT_PCM_U8"), defined below: @NLNT
					MUDEF const char* muaf_audio_format_get_name(muafAudioFormat format);

					// @DOCLINE This function returns "MUAF_FORMAT_UNKNOWN" in the case that `format` is an unrecognized value.
					// @DOCLINE > This function is a "name" function, and therefore is only defined if `MUAF_NAMES` is also defined.

					#endif

				// @DOCLINE #### Audio format nice names

					#ifdef MUAF_NAMES

					// @DOCLINE The name function `muaf_audio_format_get_nice_name` returns a presentable `const char*` representation of a given audio format (for example, `MUAF_FORMAT_PCM_U8` returns "8-bit unsigned PCM"), defined below: @NLNT
					MUDEF const char* muaf_audio_format_get_nice_name(muafAudioFormat format);

					// @DOCLINE This function returns "Unknown" in the case that `format` is an unrecognized value.
					// @DOCLINE > This function is a "name" function, and therefore is only defined if `MUAF_NAMES` is also defined.

					#endif

				// @DOCLINE #### Get audio format compression

					// @DOCLINE The function `muaf_audio_format_compressed` returns if a given audio format is compressed, defined below: @NLNT
					MUDEF muBool muaf_audio_format_compressed(muafAudioFormat format);

					// @DOCLINE This function returns `MU_FALSE` if the given format is unrecognized.

				// @DOCLINE #### Get audio format sample size

					// @DOCLINE The function `muaf_audio_format_sample_size` returns the size of a single sample for an uncompressed audio format, defined below: @NLNT
					MUDEF size_m muaf_audio_format_sample_size(muafAudioFormat format);

					// @DOCLINE If the given format is compressed or unrecognized, this function returns 0.

			// @DOCLINE ### Audio file formats

				// @DOCLINE The type `muafFileFormat` (typedef for `uint8_m`) represents a file format supported in muaf, and has the following defined values:

				// @DOCLINE * `MUAF_UNKNOWN` - an unknown and/or unrecognized file format.
				#define MUAF_UNKNOWN 0
				// @DOCLINE * `MUAF_WAVE` - the [WAVE file format](#wave-api).
				#define MUAF_WAVE 1
				// @DOCLINE * `MUAF_FLAC` - the [FLAC file format](#flac-api).
				#define MUAF_FLAC 2

				// @DOCLINE #### Audio file format names

					#ifdef MUAF_NAMES

					// @DOCLINE The name function `muaf_audio_file_format_get_name` returns a `const char*` representation of a given audio file format (for example, `MUAF_WAVE` returns "MUAF_WAVE"), defined below: @NLNT
					MUDEF const char* muaf_audio_file_format_get_name(muafFileFormat format);

					// @DOCLINE This function returns "MUAF_UNKNOWN" in the case that `format` is an unrecognized value.
					// @DOCLINE > This function is a "name" function, and therefore is only defined if `MUAF_NAMES` is also defined.

					#endif

				// @DOCLINE #### Audio file format nice names

					#ifdef MUAF_NAMES

					// @DOCLINE The name function `muaf_audio_file_format_get_nice_name` returns a presentable `const char*` representation of a given audio file format (for example, `MUAF_WAVE` returns "WAVE (.wav, .wave)"), defined below: @NLNT
					MUDEF const char* muaf_audio_file_format_get_nice_name(muafFileFormat format);

					// @DOCLINE This function returns "Unknown" in the case that `format` is an unrecognized value.
					// @DOCLINE > This function is a "name" function, and therefore is only defined if `MUAF_NAMES` is also defined.

					#endif

			// @DOCLINE ### Format specific profile

				// @DOCLINE The union `muafSpecificProfile` acts as a container for the format-specific profile of some audio file, and has the following members:

				// @DOCLINE * `muWAVEProfile* WAVE` - the [WAVE profile](#wave-profile).

		// @DOCLINE ## Get audio file format

			// @DOCLINE The function `mu_audio_file_format` retrieves the [audio file format](#audio-file-formats) of a given file, defined below: @NLNT
			MUDEF muafFileFormat mu_audio_file_format(const char* filename);

			// @DOCLINE This function does not use the filename extension to identify the audio file format, but instead, the actual contents of the file itself. This function returns `MUAF_UNKNOWN` if rather the audio file format could not be identified to be anything supported in muaf, or muaf failed to retrieve the file's data.

		// @DOCLINE ## Reading audio data

			// @DOCLINE This section covers the functionality for reading audio data.

			// @DOCLINE ### Read uncompressed raw data

				// @DOCLINE The function `mu_read_uncompressed_audio_file` reads an uncompressed audio file's raw data, defined below: @NLNT
				MUDEF muafResult mu_read_uncompressed_audio_file(const char* filename, muafUnspecificProfile* profile, muafFrames beg_frame, muafFrames frame_len, void* data);

				// @DOCLINE The profile given must be a valid loaded profile from a file that has gone unchanged since the profile was initially loaded. The given audio file's audio format must be uncompressed. The frame range provided must be valid; this function does not check if the given frame range is valid. The given data must match the corresponding type of the profile's audio format (which itself must be a supported and defined value for `muafAudioFormat`), and `data` must be large enough to hold the requested amount of frames.

		// @DOCLINE ## Writing audio data

			// @DOCLINE This section covers the functionality for writing audio data.

			// @DOCLINE ### Audio wrapper

				typedef struct muWAVEWrapper muWAVEWrapper;
				// This union is described later. Make sure it stays updated!
				union muafSpecificWrapper {
					muWAVEWrapper* WAVE;
				};
				typedef union muafSpecificWrapper muafSpecificWrapper;

				typedef struct muafUnspecificWrapper muafUnspecificWrapper;

				// @DOCLINE muaf is designed to be able to write an audio file across multiple function calls. In particular, it is designed this way so that not all of the audio data needs to be readily available at one moment in memory in order for all of the audio to be successfully encoded into the audio file.

				// @DOCLINE The primary way that muaf achieves this is by encapsulating writing audio data into a ***wrapper***, which describes an audio file that may be in the process of being written to. An audio file being written using a wrapper is only successfully fully encoded when the wrapper has been created and, afterwards, all of the audio frames have been written once and once only. Audio frames do not need to be written in any particular order.

				// @DOCLINE The function `mu_create_audio_file_wrapper` creates an audio file with no audio encoded in it based on the given wrapper information, defined below: @NLNT
				MUDEF muafResult mu_create_audio_file_wrapper(const char* filename, muafUnspecificWrapper* wrapper);

				// @DOCLINE Once an unspecific wrapper has been successfully or non-fatally retrieved, the wrapper is filled with data, some of which may be manually allocated automatically. To free this data, use the function `mu_free_audio_file_wrapper`, defined below: @NLNT
				MUDEF void mu_free_audio_file_wrapper(muafUnspecificWrapper* wrapper);

				// @DOCLINE All of the parameters within `wrapper` meant to be set by the user should be set before calling this function. Once this function has successfully or non-fatally executed, the file will be created, but it is not guaranteed to be properly encoded until all audio frames have been written once and once only.

				// @DOCLINE The struct `muafUnspecificWrapper` represents an unspecific audio file wrapper, and has the following members:
				struct muafUnspecificWrapper {
					// @DOCLINE * `@NLFT num_frames` - the number of frames (`@NLFT` typedef for `uint64_m`).
					muafFrames num_frames;
					// @DOCLINE * `@NLFT sample_rate` - the amount of samples that should be played every second per channel (`@NLFT` typedef for `uint32_m`).
					muafSampleRate sample_rate;
					// @DOCLINE * `@NLFT num_channels` - the number of channels (`@NLFT` typedef for `uint16_m`).
					muafChannels num_channels;
					// @DOCLINE * `@NLFT audio_format` - the [audio format](#audio-formats) (`@NLFT` typedef for `uint32_m`).
					muafAudioFormat audio_format;
					// @DOCLINE * `@NLFT file_format` - the [file format](#audio-file-formats) (`@NLFT` typedef for `uint8_m`).
					muafFileFormat file_format;
					// @DOCLINE * `@NLFT specific` - the [format-specific wrapper](#format-specific-wrapper). This is filled in automatically after a call to `mu_create_audio_file_wrapper`.
					muafSpecificWrapper specific;
				};

			// @DOCLINE ### Format specific wrapper

				// @DOCLINE The union `muafSpecificWrapper` represents the wrapper of a particular audio file format, and has the following members:

				// @DOCLINE * `muWAVEWrapper* WAVE` - the [WAVE wrapper](#wave-wrapper).

			// @DOCLINE ### Write raw audio frames

				// @DOCLINE The function `mu_write_uncompressed_audio_file` writes frames to an uncompressed audio file, defined below: @NLNT
				MUDEF muafResult mu_write_uncompressed_audio_file(const char* filename, muafUnspecificWrapper* wrapper, muafFrames beg_frame, muafFrames frame_len, void* data);

				// @DOCLINE The given wrapper must have its contents unaltered by the user since its original call to [`mu_create_audio_file_wrapper`](#audio-wrapper), and its audio format must be uncompressed. Once all audio frames have been written to the audio file once and once only, the file should be properly encoded.

				// @DOCLINE The given data `data` may be altered during a call to this function, and may not be the same once the function has finished.

			// @DOCLINE ### Get wrapper from audio file

				// @DOCLINE The function `mu_get_audio_file_wrapper` fills in information for an audio file wrapper based on the contents of another audio file, defined below: @NLNT
				MUDEF muafResult mu_get_audio_file_wrapper(muafUnspecificProfile* profile, muafUnspecificWrapper* wrapper);

				// @DOCLINE The parameter `profile` should be [retrieved beforehand](#audio-profile) from an existing file, and the given audio file should have [a supported unspecific audio format equivalent](#wave-format-to-unspecific-audio-format).

				// @DOCLINE The members of `wrapper` will be filled in based on the given audio file described by `profile`, except for any members that are set by the function [`mu_create_audio_file_wrapper`](#audio-wrapper).

	// @DOCLINE # WAVE API

		// @DOCLINE This section describes muaf's API for the [Waveform Audio File Format](https://en.wikipedia.org/wiki/WAV), or WAVE. The code for this API is built based off of the original August 1991 specification for WAVE (specifically [this archive](https://www.mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/Docs/riffmci.pdf)), and this section of muaf's documentation will reference concepts that are defined in this specification.

		// @DOCLINE ## WAVE profile

			// This union is described later. Make sure it stays updated!
			typedef struct muWAVEPCM muWAVEPCM;
			union muWAVEFormatSpecificFields {
				muWAVEPCM* wave_pcm;
			};
			typedef union muWAVEFormatSpecificFields muWAVEFormatSpecificFields;

			// This struct is described later. Make sure it stays updated!
			struct muWAVEChunks {
				size_m fmt;
				uint32_m fmt_len;
				size_m wave;
				uint32_m wave_len;
				size_m fact;
				uint32_m fact_len;
				size_m cue;
				uint32_m cue_len;
				size_m playlist;
				uint32_m playlist_len;
				size_m assoc_data;
				uint32_m assoc_data_len;
			};
			typedef struct muWAVEChunks muWAVEChunks;

			// @DOCLINE A WAVE file's profile can be retrieved with the function `mu_get_WAVE_profile`, defined below: @NLNT
			MUDEF muafResult mu_get_WAVE_profile(const char* filename, muWAVEProfile* profile);

			// @DOCLINE Once retrieved, the profile must be deallocated at some point using the function `mu_free_WAVE_profile`, defined below: @NLNT
			MUDEF void mu_free_WAVE_profile(muWAVEProfile* profile);

			// @DOCLINE The struct `muWAVEProfile` represents the audio file profile of a WAVE file, and has the following members:
			struct muWAVEProfile {
				// @DOCLINE * `@NLFT format_tag` - the value of wFormatTag in fmt's common-fields; the format category.
				uint16_m format_tag;
				// @DOCLINE * `@NLFT channels` - the value of wChannels in fmt's common-fields; the number of channels.
				uint16_m channels;
				// @DOCLINE * `@NLFT samples_per_sec` - the value of dwSamplesPerSec in fmt's common-fields; the sample rate.
				uint32_m samples_per_sec;
				// @DOCLINE * `@NLFT avg_bytes_per_sec` - the value of dwAvgBytesPerSec in fmt's common-fields; used for buffer estimation.
				uint32_m avg_bytes_per_sec;
				// @DOCLINE * `@NLFT block_align` - the value of wBlockAlign in fmt's common-fields; the data block size.
				uint16_m block_align;
				// @DOCLINE * `@NLFT specific_fields` - any [WAVE format-specific information in the format-specific-fields portion](#wave-format-specific-fields) of fmt, corresponding to wFormatTag.
				muWAVEFormatSpecificFields specific_fields;
				// @DOCLINE * `@NLFT chunks` - the [index locations of each chunk](#wave-chunks) within the WAVE file.
				muWAVEChunks chunks;
			};

			// @DOCLINE ### WAVE format specific fields

				// @DOCLINE The union `muWAVEFormatSpecificFields` represents any data specific to a value for wFormatTag in the format-specific-fields portion of the fmt chunk. It has the following members:

				// @DOCLINE * `muWAVEPCM* wave_pcm` - the format-specific-fields data for the [WAVE PCM format](#wave-pcm-format).

			// @DOCLINE ### WAVE formats

				// @DOCLINE This section lists every supported WAVE format, including a macro for its supported wFormatTag value in WAVE's specification, and its [format-specific-fields data](#wave-format-specific-fields) (if any).

				// @DOCLINE #### WAVE PCM format

					// @DOCLINE The WAVE PCM format represents the wFormatTag value WAVE_FORMAT_PCM, Microsoft's pulse code modulation format. Its macro is `MU_WAVE_FORMAT_PCM`, which is defined as the value `0x0001`.
					#define MU_WAVE_FORMAT_PCM 0x0001

					// @DOCLINE This format does store data in the [format-specific-fields portion of the fmt chunk](#wave-format-specific-fields). This data is represented by the struct `muWAVEPCM`, which has the following member:
					struct muWAVEPCM {
						// @DOCLINE * `@NLFT bits_per_sample` - the value of wBitsPerSample in format-specific-fields's PCM-format-specific form; the size of each sample, in bits.
						uint16_m bits_per_sample;
					};

				// @DOCLINE ### WAVE format names

					#ifdef MUAF_NAMES

					// @DOCLINE The name function `mu_WAVE_format_get_name` returns a `const char*` representation of a given WAVE audio format (for example, `MU_WAVE_FORMAT_PCM` returns "MU_WAVE_FORMAT_PCM"), defined below: @NLNT
					MUDEF const char* mu_WAVE_format_get_name(uint16_m format_tag);

					// @DOCLINE This function returns "MU_UNKNOWN" in the case that `format_tag` is an unrecognized value.
					// @DOCLINE > This function is a "name" function, and therefore is only defined if `MUAF_NAMES` is also defined.

					#endif

			// @DOCLINE ### WAVE chunks

				// @DOCLINE The struct `muWAVEChunks` stores the index location of known chunks within the WAVE file. It has the following members:

				// @DOCLINE * `size_m fmt` - the fmt-ck chunk.
				// @DOCLINE * `uint32_m fmt_len` - the recorded length of the fmt-ck chunk.
				// @DOCLINE * `size_m wave` - the wave-data chunk.
				// @DOCLINE * `uint32_m wave_len` - the recorded length of the wave-data chunk.
				// @DOCLINE * `size_m fact` - the fact-ck chunk.
				// @DOCLINE * `uint32_m fact_len` - the recorded length of the fact-ck chunk.
				// @DOCLINE * `size_m cue` - the cue-ck chunk.
				// @DOCLINE * `uint32_m cue_len` - the recorded length of the cue-ck chunk.
				// @DOCLINE * `size_m playlist` - the playlist-ck chunk.
				// @DOCLINE * `uint32_m playlist_len` - the recorded length of the playlist-ck chunk.
				// @DOCLINE * `size_m assoc_data` - the assoc-data-list chunk.
				// @DOCLINE * `uint32_m assoc_data_len` - the recorded length of the assoc-data-list chunk.

				// @DOCLINE If one of the index values defined above is set to 0, that indicates that the chunk doesn't appear within the WAVE file. If it is not set to 0, the chunk does appear within the WAVE file, and the index value indicates where in the file, starting at 0 for the first byte within the file and on.

				// @DOCLINE The index values do *not* reference the index of the chunk (which would start at its ckID), but instead reference where the actual data for the chunk starts (which starts at ckData); the index values are an index to the chunk's ckData, not the chunk's ckID. This includes chunks that are wrapped in a LIST; chunks wrapped in a list (such as the assoc-data-list chunk) have their indexes and lengths referencing the LIST chunk itself.

				// @DOCLINE A specification-compliant WAVE file will always have the chunks fmt-ck and wave-data. Although the specification does state that fmt-ck has to be defined before wave-data, muaf ignores this, and can successfully load files that don't follow this. The ignoring of this rule only applies when muaf is *reading* from a WAVE file; muaf follows this (and all other rules laid out in the specification) when *writing* a WAVE file.

				// @DOCLINE Additionally, WAVE's specifications list chunks in a specific order (fmt-ck, fact-ck, cue-ck, playlist-ck, assoc-data-list, wave-data), but I am unaware if they necessarily *need* to be in this order (besides the previously mentioned rule that fmt-ck occurs before wave-data), so muaf permits WAVE files to have any chunks in any order, as long as fmt-ck and wave-data appear at some point. muaf writes WAVE files in the specified order, though.

		// @DOCLINE ## Reading WAVE audio data

			// @DOCLINE This section covers the functionality for reading WAVE audio data.

			// @DOCLINE ### Read uncompressed raw WAVE data

				// @DOCLINE The function `mu_read_WAVE_uncompressed` reads an uncompressed WAVE file's raw data, defined below: @NLNT
				MUDEF muafResult mu_read_WAVE_uncompressed(const char* filename, muWAVEProfile* profile, muafFrames beg_frame, muafFrames frame_len, void* data);

				// @DOCLINE Listed limitations from [the unspecific function to read raw audio frames](#read-uncompressed-raw-data) apply.

			// @DOCLINE ### WAVE format to unspecific audio format

				// @DOCLINE The function `mu_WAVE_unspecific_audio_format` converts a WAVE audio format to its supported [unspecific audio format](#audio-formats) equivalent, which muaf can then generally work with, defined below: @NLNT
				MUDEF muafAudioFormat mu_WAVE_unspecific_audio_format(uint16_m format_tag, muWAVEFormatSpecificFields specific_fields);

				// @DOCLINE If this function returns `MUAF_FORMAT_UNKNOWN`, then the given WAVE audio format information is unrecognized and/or unsupported, and muaf is unable to extract the data from it.

		// @DOCLINE ## Writing WAVE audio data

			// @DOCLINE This section covers the functionality for writing WAVE audio data.

			// @DOCLINE ### WAVE wrapper

				// @DOCLINE The function `mu_create_WAVE_wrapper` creates a WAVE file given WAVE wrapper information, defined below: @NLNT
				MUDEF muafResult mu_create_WAVE_wrapper(const char* filename, muWAVEWrapper* wrapper);

				// @DOCLINE The function `mu_free_WAVE_wrapper` frees any manually allocated data that might have been generated from its call to `mu_create_WAVE_wrapper`, defined below: @NLNT
				MUDEF void mu_free_WAVE_wrapper(muWAVEWrapper* wrapper);

				// @DOCLINE Listed limitations from [the unspecific functions to create/free a wrapper](#audio-wrapper) apply.

				// @DOCLINE The struct `muWAVEWrapper` represents a WAVE file wrapper, and has the following members:
				struct muWAVEWrapper {
					// @DOCLINE * `@NLFT audio_format` - the [audio format](#audio-formats) of the WAVE file.
					muafAudioFormat audio_format;
					// @DOCLINE * `@NLFT num_frames` - the amount of frames in the WAVE file.
					uint32_m num_frames;
					// @DOCLINE * `@NLFT num_channels` - the number of channels.
					uint16_m num_channels;
					// @DOCLINE * `@NLFT sample_rate` - the amount of samples that should be played every second per channel.
					uint32_m sample_rate;
					// @DOCLINE * `@NLFT chunks` - the location of the chunks in the audio file. This is used internally, and should not be filled in by the user.
					muWAVEChunks chunks;
				};

			// @DOCLINE ### Write raw WAVE audio frames

				// @DOCLINE The function `mu_write_WAVE_uncompressed` writes frames to an uncompressed WAVE file, defined below: @NLNT
				MUDEF muafResult mu_write_WAVE_uncompressed(const char* filename, muWAVEWrapper* wrapper, muafFrames beg_frame, muafFrames frame_len, void* data);

				// @DOCLINE Listed limitations from [the unspecific function to write raw audio frames](#write-raw-audio-frames) apply.

			// @DOCLINE ### Get WAVE wrapper from audio file

				// @DOCLINE The function `mu_get_WAVE_wrapper` fills in information for a WAVE wrapper based on the contents of another WAVE file, defined below: @NLNT
				MUDEF muafResult mu_get_WAVE_wrapper(muWAVEProfile* profile, muWAVEWrapper* wrapper);

				// @DOCLINE Listed limitations from [the unspecific function to get a wrapper from an audio file](#get-wrapper-from-audio-file) apply.

		// @DOCLINE ## WAVE known bugs and limitations

			// @DOCLINE This section covers the known bugs and limitations for the implementation of WAVE in muaf.

			// @DOCLINE ### Support for LIST wave-data

				// @DOCLINE muaf does not currently support wave-data that's provided in the form of a LIST. It only supports wave-data in the form of a data-ck chunk.

			// @DOCLINE ### Support for wBitsPerSample

				// @DOCLINE muaf does not currently support values for wBitsPerSample that are not divisible by 8.

	// @DOCLINE # FLAC API

		// @DOCLINE This section describes muaf's API for the [Free Lossless Audio Codec](https://en.wikipedia.org/wiki/FLAC), or FLAC. The code for this API is built based off of [the RFC 9639 specification](https://datatracker.ietf.org/doc/rfc9639/), and this section of muaf's documentation will reference concepts that are defined in this specification. Any quotes referenced in this section are from RFC 9639 unless it is stated otherwise.

		// @DOCLINE All values provided by the FLAC API by retrieving information from a FLAC audio file are checked and strictly guaranteed to be values permitted by the specification unless it is stated otherwise. These limitations are also strictly followed when encoding with no exceptions.

		// @DOCLINE ## FLAC profile

			typedef struct muFLACProfile muFLACProfile;

			// @DOCLINE A FLAC file's profile can be retrieved with the function `mu_get_FLAC_profile`, defined below: @NLNT
			MUDEF muafResult mu_get_FLAC_profile(const char* filename, muFLACProfile* profile);

			// @DOCLINE Once retrieved, the profile must be deallocated at some point using the function `mu_free_FLAC_profile`, defined below: @NLNT
			MUDEF void mu_free_FLAC_profile(muFLACProfile* profile);

			// @DOCLINE The struct `muFLACProfile` represents the audio file profile of a FLAC file, and has the following members:
			struct muFLACProfile {
				// @DOCLINE * `@NLFT contains_audio` - whether or not the given FLAC file has any audio data stored in it.
				muBool contains_audio;
				// @DOCLINE * `@NLFT min_block_size` - the first value in the streaminfo metadata block; "The minimum block size (in samples) used in the stream, excluding the last block."
				uint16_m min_block_size;
				// @DOCLINE * `@NLFT max_block_size` - the second value in the streaminfo metadata block; "The maximum block size (in samples) used in the stream."
				uint16_m max_block_size;
				// @DOCLINE * `@NLFT min_frame_size` - the third value in the streaminfo metadata block; "The minimum frame size (in bytes) used in the stream."
				uint32_m min_frame_size;
				// @DOCLINE * `@NLFT max_frame_size` - the fourth value in the streaminfo metadata block; "The maximum frame size (in bytes) used in the stream."
				uint32_m max_frame_size;
				// @DOCLINE * `@NLFT sample_rate` - the fifth value in the streaminfo metadata block; "Sample rate in Hz."
				uint32_m sample_rate;
				// @DOCLINE * `@NLFT num_channels` - the sixth value in the streaminfo metadata block; "(number of channels)-1."
				uint8_m num_channels;
				// @DOCLINE * `@NLFT bits_per_sample` - the seventh value in the streaminfo metadata block; "	(bits per sample)-1."
				uint8_m bits_per_sample;
				// @DOCLINE * `@NLFT num_samples` - the eight value in the streaminfo metadata block; "Total number of interchannel samples in the stream."
				uint64_m num_samples;
				// @DOCLINE * `@NLFT low_checksum` - the low bytes of the ninth value in the streaminfo metadata block; low bytes of "MD5 checksum of the unencoded audio data."
				uint64_m low_checksum;
				// @DOCLINE * `@NLFT high_checksum` - the high bytes of the ninth value in the streaminfo metadata block; high bytes of "MD5 checksum of the unencoded audio data."
				uint64_m high_checksum;
			};

	// @DOCLINE # Result

		// @DOCLINE The type `muafResult` (typedef for `uint32_m`) is defined to represent how a task went. Result values can be "fatal" (meaning that the task completely failed to execute, and the program will continue as if the task had never been attempted), "non-fatal" (meaning that the task partially failed, but was still able to complete the task), and "successful" (meaning that the task fully succeeded as intended).

		// @DOCLINE ## Result values

			// @DOCLINE The following values are defined for `muafResult` (all values not explicitly stated as being fatal, non-fatal, or successful are assumed to be fatal).

			// @DOCLINE ### General result values
			// 0 -> 1023 //

				// @DOCLINE * `MUAF_SUCCESS` - the task was successfully completed; real value 0.
				#define MUAF_SUCCESS 0
				// @DOCLINE * `MUAF_FAILED_MALLOC` - a vital call to malloc failed.
				#define MUAF_FAILED_MALLOC 1
				// @DOCLINE * `MUAF_FAILED_OPEN_FILE` - an attempt to open the file failed.
				#define MUAF_FAILED_OPEN_FILE 2
				// @DOCLINE * `MUAF_FAILED_UNSUPPORTED_AUDIO_FORMAT` - the task could not be completed due to the file being in an audio format that muaf does not support.
				#define MUAF_FAILED_UNSUPPORTED_AUDIO_FORMAT 3
				// @DOCLINE * `MUAF_FAILED_CREATE_FILE` - an attempt to create the file failed.
				#define MUAF_FAILED_CREATE_FILE 4
				// @DOCLINE * `MUAF_FAILED_UNSUPPORTED_AUDIO_FILE_FORMAT` - the task could not be completed due to the file being in an audio file format (or the user providing an audio file format value) that muaf does not support or could not recognize.
				#define MUAF_FAILED_UNSUPPORTED_AUDIO_FILE_FORMAT 5
				// @DOCLINE * `MUAF_FAILED_AUDIO_FILE_FORMAT_IDENTIFICATION` - an attempt to retrieve/write information from/to an audio file failed, as the assumed file format was not the case (such as trying to get the profile of a WAVE file using `mu_get_WAVE_profile` when the file in question doesn't appear to actually be a WAVE file).
				#define MUAF_FAILED_AUDIO_FILE_FORMAT_IDENTIFICATION 6

			// @DOCLINE ### WAVE result values
			// 1024 -> 2047 //

				// @DOCLINE * `MUAF_INVALID_WAVE_CHUNK_LENGTH_FOR_FILE` - a WAVE chunk has a recorded length that is out of range for the file's actual length.
				#define MUAF_INVALID_WAVE_CHUNK_LENGTH_FOR_FILE 1024
				// @DOCLINE * `MUAF_INVALID_WAVE_MISSING_FMT` - the required WAVE chunk fmt-ck was not found.
				#define MUAF_INVALID_WAVE_MISSING_FMT 1025
				// @DOCLINE * `MUAF_INVALID_WAVE_MISSING_WAVE_DATA` - the required WAVE chunk wave-data was not found.
				#define MUAF_INVALID_WAVE_MISSING_WAVE_DATA 1026
				// @DOCLINE * `MUAF_INVALID_WAVE_FMT_LENGTH` - the WAVE chunk fmt-ck has an invalid recorded length.
				#define MUAF_INVALID_WAVE_FMT_LENGTH 1027
				// @DOCLINE * `MUAF_INVALID_WAVE_FMT_CHANNELS` - the WAVE chunk fmt-ck's value 'wChannels' has an invalid value of 0.
				#define MUAF_INVALID_WAVE_FMT_CHANNELS 1028
				// @DOCLINE * `MUAF_INVALID_WAVE_FMT_SAMPLES_PER_SEC` - the WAVE chunk fmt-ck's value 'dwSamplesPerSec' has an invalid value of 0.
				#define MUAF_INVALID_WAVE_FMT_SAMPLES_PER_SEC 1029
				// @DOCLINE * `MUAF_INVALID_WAVE_FMT_PCM_BITS_PER_SAMPLE` - the WAVE chunk fmt-ck's PCM-format-specific value 'wBitsPerSample' has an invalid value; it's rather equal to 0, non-divisible by 8 without a remainder, doesn't evenly divide the length of the wave data, or doesn't align with the value for wBlockAlign.
				#define MUAF_INVALID_WAVE_FMT_PCM_BITS_PER_SAMPLE 1030
				// @DOCLINE * `MUAF_INVALID_WAVE_FILE_WRITE_SIZE` - the WAVE file could not be created, as the size of the WAVE file would be over the maximum file size of a WAVE file due to any of the limitations of how big certain values can be encoded in WAVE (such as the ckSize for the RIFF chunk).
				#define MUAF_INVALID_WAVE_FILE_WRITE_SIZE 1031

			// @DOCLINE ### FLAC result values
			// 2048 -> 3071 //

				// @DOCLINE * `MUAF_INVALID_FLAC_STREAMINFO_LENGTH` - the streaminfo metadata block has an invalid recorded length (not 34 bytes).
				#define MUAF_INVALID_FLAC_STREAMINFO_LENGTH 2048
				// @DOCLINE * `MUAF_INVALID_FLAC_STREAMINFO_BLOCK_SIZE_RANGE` - the listed minimum or maximum block size within streaminfo was not within the required value range of 16 to 65535.
				#define MUAF_INVALID_FLAC_STREAMINFO_BLOCK_SIZE_RANGE 2049
				// @DOCLINE * `MUAF_INVALID_FLAC_STREAMINFO_BLOCK_SIZE_MIN_MAX` - the listed minimum and maximum block size within streaminfo do not make sense, as the maximum is smaller than the minimum.
				#define MUAF_INVALID_FLAC_STREAMINFO_BLOCK_SIZE_MIN_MAX 2050

				// #define MUAF_INVALID_FLAC_STREAMINFO_FRAME_SIZE_RANGE 2051

				// @DOCLINE * `MUAF_INVALID_FLAC_STREAMINFO_FRAME_SIZE_MIN_MAX` - the listed minimum and maximum frame size within streaminfo do not make sense, as the maximum is smaller than the minimum.
				#define MUAF_INVALID_FLAC_STREAMINFO_FRAME_SIZE_MIN_MAX 2052
				// @DOCLINE * `MUAF_INVALID_FLAC_STREAMINFO_NUM_CHANNELS` - the listed number of channels within streaminfo (after accounting for subtraction) is not within the permitted range of 1 to 8. This value is permitted to be zero (after accounting for subtraction) if no audio is being stored.
				#define MUAF_INVALID_FLAC_STREAMINFO_NUM_CHANNELS 2053
				// @DOCLINE * `MUAF_INVALID_FLAC_STREAMINFO_BITS_PER_SAMPLE` - the listed bits per sample within streaminfo (after accounting for subtraction) is not within the permitted range of 4 to 32. This value is permitted to be zero (after accounting for subtraction) if no audio is being stored.
				#define MUAF_INVALID_FLAC_STREAMINFO_BITS_PER_SAMPLE 2054
				// @DOCLINE * `MUAF_INVALID_FLAC_STREAMINFO_SAMPLE_COUNT` - the listed amount of samples doesn't make sense, rather because the FLAC file doesn't contain audio and the sample count is over 0, or because the FLAC file does contain audio and the sample count is 0.
				#define MUAF_INVALID_FLAC_STREAMINFO_SAMPLE_COUNT 2055

		// @DOCLINE ## Check if result is fatal

			// @DOCLINE The function `muaf_result_is_fatal` returns whether or not a given `muafResult` value is fatal, defined below: @NLNT
			MUDEF muBool muaf_result_is_fatal(muafResult result);

			// @DOCLINE This function returns `MU_TRUE` if the value of `result` is invalid/unrecognized.

		// @DOCLINE ## Result name

			#ifdef MUAF_NAMES

			// @DOCLINE The function `muaf_result_get_name` returns a `const char*` representation of a given result value (for example, `MUAF_SUCCESS` returns "MUAF_SUCCESS"), defined below: @NLNT
			MUDEF const char* muaf_result_get_name(muafResult result);

			// @DOCLINE The function returns "MU_UNKNOWN" in the case that `result` is an invalid result value.

			// @DOCLINE > This function is a "name" function, and therefore is only defined if `MUAF_NAMES` is also defined.

			#endif

	// @DOCLINE # C standard library dependencies

		// @DOCLINE muaf has several C standard library dependencies, all of which are overridable by defining them before the inclusion of its header. The following is a list of those dependencies.

		#if !defined(FILE_M) || \
			!defined(mu_fopen) || \
			!defined(mu_fclose) || \
			!defined(mu_fread) || \
			!defined(mu_fseek) || \
			!defined(MU_SEEK_END) || \
			!defined(MU_SEEK_SET) || \
			!defined(mu_ftell) || \
			!defined(mu_fputc) || \
			!defined(mu_fwrite)

			// @DOCLINE ## `stdio.h` dependencies
			#include <stdio.h>

			// @DOCLINE * `FILE_M` - equivalent to `FILE`.
			#ifndef FILE_M
				#define FILE_M FILE
			#endif

			// @DOCLINE * `mu_fopen` - equivalent to `fopen`.
			#ifndef mu_fopen
				#define mu_fopen fopen
			#endif

			// @DOCLINE * `mu_fclose` - equivalent to `fclose`.
			#ifndef mu_fclose
				#define mu_fclose fclose
			#endif

			// @DOCLINE * `mu_fread` - equivalent to `fread`.
			#ifndef mu_fread
				#define mu_fread fread
			#endif

			// @DOCLINE * `mu_fseek` - equivalent to `fseek`.
			#ifndef mu_fseek
				#define mu_fseek fseek
			#endif

			// @DOCLINE * `MU_SEEK_END` - equivalent to `SEEK_END`.
			#ifndef MU_SEEK_END
				#define MU_SEEK_END SEEK_END
			#endif

			// @DOCLINE * `MU_SEEK_SET` - equivalent to `SEEK_SET`.
			#ifndef MU_SEEK_SET
				#define MU_SEEK_SET SEEK_SET
			#endif

			// @DOCLINE * `mu_ftell` - equivalent to `ftell`.
			#ifndef mu_ftell
				#define mu_ftell ftell
			#endif

			// @DOCLINE * `mu_fputc` - equivalent to `fputc`.
			#ifndef mu_fputc
				#define mu_fputc fputc
			#endif

			// @DOCLINE * `mu_fwrite` - equivalent to `fwrite`.
			#ifndef mu_fwrite
				#define mu_fwrite fwrite
			#endif

		#endif

		#if !defined(mu_malloc) || \
			!defined(mu_free)

			// @DOCLINE ## `stdlib.h` dependencies
			#include <stdlib.h>

			// @DOCLINE * `mu_malloc` - equivalent to `malloc`.
			#ifndef mu_malloc
				#define mu_malloc malloc
			#endif

			// @DOCLINE * `mu_free` - equivalent to `free`.
			#ifndef mu_free
				#define mu_free free
			#endif

		#endif

		#if !defined(mu_memset) || \
			!defined(mu_memcpy)

			// @DOCLINE ## `string.h` dependencies
			#include <string.h>

			// @DOCLINE * `mu_memset` - equivalent to `memset`.
			#ifndef mu_memset
				#define mu_memset memset
			#endif

			// @DOCLINE * `mu_memcpy` - equivalent to `memcpy`.
			#ifndef mu_memcpy
				#define mu_memcpy memcpy
			#endif

		#endif

	MU_CPP_EXTERN_END
#endif /* MUAF_H */

#ifdef MUAF_IMPLEMENTATION
	MU_CPP_EXTERN_START

	/* File reading stuff */

		// Struct representing a file
		struct muafInner_File {
			FILE_M* fptr;
			size_m len;
		};
		typedef struct muafInner_File muafInner_File;

		// Loads a file
		// Returns 0 on success
		int muafInner_LoadFile(const char* filename, muafInner_File* file) {
			// Open file
			file->fptr = mu_fopen(filename, "rb");
			if (!file->fptr) {
				return -1;
			}

			// Read length
			mu_fseek(file->fptr, 0L, MU_SEEK_END);
			file->len = mu_ftell(file->fptr);

			return 0;
		}

		// Creates a writable file
		// If length is 0, file is not created, but opened in a writable mode
		int muafInner_CreateFile(const char* filename, muafInner_File* file, size_m len) {
			// Creating
			if (len != 0) {
				// Create file
				file->fptr = mu_fopen(filename, "wb");
				if (!file->fptr) {
					return -1;
				}
				// Set length
				file->len = len;
				mu_fseek(file->fptr, len-1, MU_SEEK_SET);
				mu_fputc('\0', file->fptr);
			}

			// Opening writable
			else {
				// Open file
				file->fptr = mu_fopen(filename, "rb+");
				if (!file->fptr) {
					return -1;
				}
				// Retrieve length
				mu_fseek(file->fptr, 0L, MU_SEEK_END);
				file->len = mu_ftell(file->fptr);
			}

			return 0;
		}

		// Deloads a file
		void muafInner_DeloadFile(muafInner_File* file) {
			mu_fclose(file->fptr);
		}

		// Loads data from a file
		void muafInner_LoadFromFile(muafInner_File* file, size_m index, size_m len, muByte* data) {
			// Set to spot
			mu_fseek(file->fptr, index, MU_SEEK_SET);
			// Read data
			mu_fread(data, len, 1, file->fptr);
		}

		// Writes data to a file
		void muafInner_WriteToFile(muafInner_File* file, size_m index, size_m len, muByte* data) {
			// Set to spot
			mu_fseek(file->fptr, index, MU_SEEK_SET);
			// Write to file
			mu_fwrite((const void*)data, 1, len, file->fptr);
		}

	/* WAVE */

		/* Profiling */

			// Checks if given data is WAVE or not
			muBool muafWAVE_IsWAVE(muafInner_File* file) {
				// Minimum length check
				// Includes RIFF, ckSize, and WAVE
				if (file->len < 12) {
					return MU_FALSE;
				}

				// Load first 12 bytes
				muByte data[12];
				muafInner_LoadFromFile(file, 0, 12, data);

				// Check for RIFF
				if (data[0] != 0x52 || data[1] != 0x49 || data[2] != 0x46 || data[3] != 0x46) {
					return MU_FALSE;
				}

				// Skip size and check for WAVE
				if (data[8] != 0x57 || data[9] != 0x41 || data[10] != 0x56 || data[11] != 0x45) {
					return MU_FALSE;
				}

				// Has RIFF and WAVE, so likely WAVE.
				return MU_TRUE;
			}

			// Gets chunk information about WAVE file
			// Does check for required chunks
			muafResult muafWAVE_GetChunks(muafInner_File* file, muWAVEProfile* profile) {
				// Start after RIFF, ckSize, and WAVE
				size_m beg_i = 12;
				// Loop through each chunk
				while (MU_TRUE) {
					// Account for pad byte
					if (beg_i % 2 != 0) {
						beg_i += 1;
					}

					// Break out of loop if we're at end of file now
					// + ensure length for ckID and ckSize
					if (beg_i >= file->len || file->len-beg_i < 8) {
						break;
					}

					// Get data for ckID and ckSize
					muByte ck_data[8];
					muafInner_LoadFromFile(file, beg_i, 8, ck_data);

					// Read ID and length
					uint32_m ck_id = MU_RBEU32(ck_data);
					uint32_m len = MU_RLEU32(ck_data+4);
					// Verify length
					if (beg_i + 8 + len > file->len) {
						return MUAF_INVALID_WAVE_CHUNK_LENGTH_FOR_FILE;
					}

					size_m* pid = 0;
					uint32_m* plen = 0;
					switch (ck_id) {
						default: break;
						// fmt
						case 0x666D7420: {
							pid = &profile->chunks.fmt;
							plen = &profile->chunks.fmt_len;
						} break;
						// wave
						case 0x64617461: {
							pid = &profile->chunks.wave;
							plen = &profile->chunks.wave_len;
						} break;
						// fact
						case 0x66616374: {
							pid = &profile->chunks.fact;
							plen = &profile->chunks.fact_len;
						} break;
						// cue
						case 0x63756520: {
							pid = &profile->chunks.cue;
							plen = &profile->chunks.cue_len;
						} break;
						// playlist
						case 0x706C7374: {
							pid = &profile->chunks.playlist;
							plen = &profile->chunks.playlist_len;
						} break;
						// LIST
						case 0x4C495354: {
							// Read list type
							if (len > 4) {
								muByte list_type[4];
								muafInner_LoadFromFile(file, beg_i+8, 4, list_type);
								uint32_m ulist_type = MU_RBEU32(list_type);
								switch (ulist_type) {
									default: break;
									// assoc-data
									case 0x6164746C: {
										pid = &profile->chunks.assoc_data;
										plen = &profile->chunks.assoc_data_len;
									} break;
								}
							}
						} break;
					}

					// Record ID and length if necessary
					if (pid && plen) {
						*pid = beg_i + 8;
						*plen = len;
					}
					// Move to next chunk
					beg_i += 8 + len;
				}

				// Make sure fmt and wave data have been defined
				if (!profile->chunks.fmt) {
					return MUAF_INVALID_WAVE_MISSING_FMT;
				}
				if (!profile->chunks.wave) {
					return MUAF_INVALID_WAVE_MISSING_WAVE_DATA;
				}

				return MUAF_SUCCESS;
			}

			// Gets fmt WAVE PCM info
			muafResult muafWAVE_GetFmtPCMInfo(muafInner_File* file, muWAVEProfile* profile) {
				// Ensure extra length
				if (profile->chunks.fmt_len < 14+2) {
					return MUAF_INVALID_WAVE_FMT_LENGTH;
				}
				// Read PCM-format-specific data
				muByte pcm_format_spec[2];
				muafInner_LoadFromFile(file, profile->chunks.fmt+14, 2, pcm_format_spec);

				// Allocate struct
				profile->specific_fields.wave_pcm = (muWAVEPCM*)mu_malloc(sizeof(muWAVEPCM));
				if (!profile->specific_fields.wave_pcm) {
					return MUAF_FAILED_MALLOC;
				}

				// Read bits per sample
				profile->specific_fields.wave_pcm->bits_per_sample = MU_RLEU16(pcm_format_spec);
				// - Verify it doesn't equal 0
				if (profile->specific_fields.wave_pcm->bits_per_sample == 0) {
					return MUAF_INVALID_WAVE_FMT_PCM_BITS_PER_SAMPLE;
				}
				// - Verify it's divisible by 8
				if (profile->specific_fields.wave_pcm->bits_per_sample % 8 != 0) {
					return MUAF_INVALID_WAVE_FMT_PCM_BITS_PER_SAMPLE;
				}
				// - Verify it works with wBlockAlign
				if (((uint64_m)profile->channels) * (((uint64_m)profile->specific_fields.wave_pcm->bits_per_sample) / 8) != profile->block_align) {
					return MUAF_INVALID_WAVE_FMT_PCM_BITS_PER_SAMPLE;
				}
				// - Verify it works with wave data length
				if (profile->chunks.wave_len % (profile->specific_fields.wave_pcm->bits_per_sample / 8) != 0) {
					return MUAF_INVALID_WAVE_FMT_PCM_BITS_PER_SAMPLE;
				}

				return MUAF_SUCCESS;
			}

			// Gets fmt information from WAVE file
			// Chunks need to be loaded before this
			muafResult muafWAVE_GetFmtInfo(muafInner_File* file, muWAVEProfile* profile) {
				// Ensure minimum fmt length
				if (profile->chunks.fmt_len < 14) {
					return MUAF_INVALID_WAVE_FMT_LENGTH;
				}

				// Read fmt-ck's common-fields
				muByte fmt_common_fields[14];
				muafInner_LoadFromFile(file, profile->chunks.fmt, 14, fmt_common_fields);

				// Read values
				profile->format_tag = MU_RLEU16(fmt_common_fields);

				profile->channels = MU_RLEU16(fmt_common_fields + 2);
				if (profile->channels == 0) {
					return MUAF_INVALID_WAVE_FMT_CHANNELS;
				}

				profile->samples_per_sec = MU_RLEU32(fmt_common_fields + 4);
				if (profile->samples_per_sec == 0) {
					return MUAF_INVALID_WAVE_FMT_SAMPLES_PER_SEC;
				}

				// Make sure to verify these values if muaf ever uses them!
				profile->avg_bytes_per_sec = MU_RLEU32(fmt_common_fields + 8);
				profile->block_align = MU_RLEU16(fmt_common_fields + 12);

				// Allocate and read any format specific fields
				switch (profile->format_tag) {
					default: break;
					// WAVE PCM
					case MU_WAVE_FORMAT_PCM: {
						muafResult res = muafWAVE_GetFmtPCMInfo(file, profile);
						if (muaf_result_is_fatal(res)) {
							return res;
						}
					} break;
				}

				return MUAF_SUCCESS;
			}

			// Handles ckSize in case it's shorter than the file length
			// Call muafWAVE_IsWAVE BEFORE this!!
			void muafWAVE_HandleCkSize(muafInner_File* file) {
				// Read ckSize
				muByte data[4];
				muafInner_LoadFromFile(file, 4, 4, data);
				size_m ck_size = MU_RLEU32(data) + 8;
				if (ck_size < file->len) {
					file->len = ck_size;
				}
			}

			// Gets WAVE profile given inner file
			muafResult muafWAVE_GetWAVEProfile(muafInner_File* file, muWAVEProfile* profile) {
				// Make sure it's WAVE
				if (!muafWAVE_IsWAVE(file)) {
					return MUAF_FAILED_AUDIO_FILE_FORMAT_IDENTIFICATION;
				}
				// Correct file length based on ckSize
				muafWAVE_HandleCkSize(file);

				// Zero-out profile memory
				mu_memset(profile, 0, sizeof(muWAVEProfile));

				// Get chunk information
				muafResult res = muafWAVE_GetChunks(file, profile);
				if (muaf_result_is_fatal(res)) {
					return res;
				}
				// Get fmt information
				res = muafWAVE_GetFmtInfo(file, profile);
				if (muaf_result_is_fatal(res)) {
					mu_free_WAVE_profile(profile);
					return res;
				}

				return res;
			}

			// Gets WAVE profile
			MUDEF muafResult mu_get_WAVE_profile(const char* filename, muWAVEProfile* profile) {
				// Open file
				muafInner_File file;
				if (muafInner_LoadFile(filename, &file) != 0) {
					return MUAF_FAILED_OPEN_FILE;
				}
				
				// Get WAVE profile
				muafResult res = muafWAVE_GetWAVEProfile(&file, profile);

				// Close file
				muafInner_DeloadFile(&file);
				return res;
			}

			// Frees memory for WAVE profile
			MUDEF void mu_free_WAVE_profile(muWAVEProfile* profile) {
				// Free format specific fields if it exists
				// Any member will do for this check due to overlapping union memory
				if (profile->specific_fields.wave_pcm != 0) {
					mu_free(profile->specific_fields.wave_pcm);
				}
			}

		/* Reading */

			/* PCM reading */

				// MUAF_FORMAT_PCM_U8
				void muafWAVE_read_FORMAT_PCM_U8(muafInner_File* file, muWAVEProfile* profile, muafFrames beg_frame, muafFrames frame_len, uint8_m* data) {
					// Read data from file
					muafInner_LoadFromFile(file, profile->chunks.wave + (beg_frame * profile->channels), frame_len * profile->channels, data);
				}

				// MUAF_FORMAT_PCM_S16
				void muafWAVE_read_FORMAT_PCM_S16(muafInner_File* file, muWAVEProfile* profile, muafFrames beg_frame, muafFrames frame_len, int16_m* data) {
					// Read data from file
					muafInner_LoadFromFile(file, profile->chunks.wave + (beg_frame * 2 * profile->channels), frame_len * 2 * profile->channels, (muByte*)data);
					// Correct byte orders
					muafFrames sample_count = frame_len * profile->channels;
					for (muafFrames s = 0; s < sample_count; ++s) {
						int16_m sample = MU_RLES16(((muByte*)data)+(s*2));
						data[s] = sample;
					}
				}

				// MUAF_FORMAT_PCM_S32
				void muafWAVE_read_FORMAT_PCM_S32(muafInner_File* file, muWAVEProfile* profile, muafFrames beg_frame, muafFrames frame_len, int32_m* data) {
					// Read data from file
					muafInner_LoadFromFile(file, profile->chunks.wave + (beg_frame * 4 * profile->channels), frame_len * 4 * profile->channels, (muByte*)data);
					// Correct byte orders
					muafFrames sample_count = frame_len * profile->channels;
					for (muafFrames s = 0; s < sample_count; ++s) {
						int16_m sample = MU_RLES16(((muByte*)data)+(s*4));
						data[s] = sample;
					}
				}

				// MUAF_FORMAT_PCM_S64
				void muafWAVE_read_FORMAT_PCM_S64(muafInner_File* file, muWAVEProfile* profile, muafFrames beg_frame, muafFrames frame_len, int64_m* data) {
					// Read data from file
					muafInner_LoadFromFile(file, profile->chunks.wave + (beg_frame * 8 * profile->channels), frame_len * 8 * profile->channels, (muByte*)data);
					// Correct byte orders
					muafFrames sample_count = frame_len * profile->channels;
					for (muafFrames s = 0; s < sample_count; ++s) {
						int16_m sample = MU_RLES16(((muByte*)data)+(s*8));
						data[s] = sample;
					}
				}

			/* General reading */

				// Finds muaf supported equivalent for WAVE data storage
				MUDEF muafAudioFormat mu_WAVE_unspecific_audio_format(uint16_m format_tag, muWAVEFormatSpecificFields specific_fields) {
					switch (format_tag) {
						default: return MUAF_FORMAT_UNKNOWN; break;

						// WAVE PCM
						case MU_WAVE_FORMAT_PCM: {
							switch (specific_fields.wave_pcm->bits_per_sample) {
								default: return MUAF_FORMAT_UNKNOWN; break;
								case 8: return MUAF_FORMAT_PCM_U8; break;
								case 16: return MUAF_FORMAT_PCM_S16; break;
								case 32: return MUAF_FORMAT_PCM_S32; break;
								case 64: return MUAF_FORMAT_PCM_S64; break;
							}
						} break;
					}
				}

				// Reads uncompressed raw data given inner file and format
				muafResult muafWAVE_ReadUncompressed(muafInner_File* file, muWAVEProfile* profile, muafAudioFormat format, muafFrames beg_frame, muafFrames frame_len, void* data) {
					// Perform reading based on format
					switch (format) {
						// Unknown
						default: return MUAF_FAILED_UNSUPPORTED_AUDIO_FORMAT; break;
						// WAVE PCM
						case MUAF_FORMAT_PCM_U8:  muafWAVE_read_FORMAT_PCM_U8 (file, profile, beg_frame, frame_len, (uint8_m*)data); break;
						case MUAF_FORMAT_PCM_S16: muafWAVE_read_FORMAT_PCM_S16(file, profile, beg_frame, frame_len, (int16_m*)data); break;
						case MUAF_FORMAT_PCM_S32: muafWAVE_read_FORMAT_PCM_S32(file, profile, beg_frame, frame_len, (int32_m*)data); break;
						case MUAF_FORMAT_PCM_S64: muafWAVE_read_FORMAT_PCM_S64(file, profile, beg_frame, frame_len, (int64_m*)data); break;
					}

					return MUAF_SUCCESS;
				}

				// Reads uncompressed raw data
				MUDEF muafResult mu_read_WAVE_uncompressed(const char* filename, muWAVEProfile* profile, muafFrames beg_frame, muafFrames frame_len, void* data) {
					// Open file
					muafInner_File file;
					if (muafInner_LoadFile(filename, &file) != 0) {
						return MUAF_FAILED_OPEN_FILE;
					}

					// Get muaf supported audio format
					muafAudioFormat format = mu_WAVE_unspecific_audio_format(profile->format_tag, profile->specific_fields);
					// Perform reading
					muafResult res = muafWAVE_ReadUncompressed(&file, profile, format, beg_frame, frame_len, data);

					// Close file
					muafInner_DeloadFile(&file);
					return res;
				}

		/* Writing */

			/* PCM wrapper writing */

				// MUAF_FORMAT_PCM_U8
				muafResult muafWAVE_fmt_FORMAT_PCM_U8(muafInner_File* file, muWAVEWrapper* wrapper) {
					// Fill out info
					muByte fmt[16];

					// - wFormatTag
					MU_WLEU16(fmt, MU_WAVE_FORMAT_PCM);
					// - wChannels
					MU_WLEU16(fmt+2, wrapper->num_channels);
					// - dwSamplesPerSec
					MU_WLEU32(fmt+4, wrapper->sample_rate);
					// - dwAvgBytesPerSec
					uint64_m avg_bytes = ((uint64_m)wrapper->num_channels) * ((uint64_m)wrapper->sample_rate) * 1;
					if (avg_bytes > 0xFFFFFFFF) {
						return MUAF_INVALID_WAVE_FILE_WRITE_SIZE;
					}
					MU_WLEU32(fmt+8, avg_bytes);
					// - wBlockAlign
					MU_WLEU16(fmt+12, wrapper->num_channels);

					// - wBitsPerSample
					MU_WLEU16(fmt+14, 8);

					// Write to file
					muafInner_WriteToFile(file, wrapper->chunks.fmt, 16, fmt);
					return MUAF_SUCCESS;
				}

				// MUAF_FORMAT_PCM_S16
				muafResult muafWAVE_fmt_FORMAT_PCM_S16(muafInner_File* file, muWAVEWrapper* wrapper) {
					// Fill out info
					muByte fmt[16];

					// - wFormatTag
					MU_WLEU16(fmt, MU_WAVE_FORMAT_PCM);
					// - wChannels
					MU_WLEU16(fmt+2, wrapper->num_channels);
					// - dwSamplesPerSec
					MU_WLEU32(fmt+4, wrapper->sample_rate);
					// - dwAvgBytesPerSec
					uint64_m avg_bytes = ((uint64_m)wrapper->num_channels) * ((uint64_m)wrapper->sample_rate) * 2;
					if (avg_bytes > 0xFFFFFFFF) {
						return MUAF_INVALID_WAVE_FILE_WRITE_SIZE;
					}
					MU_WLEU32(fmt+8, avg_bytes);
					// - wBlockAlign
					uint32_m block_align = ((uint32_m)wrapper->num_channels) * 2;
					if (block_align > 0xFFFFFFFF) {
						return MUAF_INVALID_WAVE_FILE_WRITE_SIZE;
					}
					MU_WLEU16(fmt+12, block_align);

					// - wBitsPerSample
					MU_WLEU16(fmt+14, 16);

					// Write to file
					muafInner_WriteToFile(file, wrapper->chunks.fmt, 16, fmt);
					return MUAF_SUCCESS;
				}

				// MUAF_FORMAT_PCM_S32
				muafResult muafWAVE_fmt_FORMAT_PCM_S32(muafInner_File* file, muWAVEWrapper* wrapper) {
					// Fill out info
					muByte fmt[16];

					// - wFormatTag
					MU_WLEU16(fmt, MU_WAVE_FORMAT_PCM);
					// - wChannels
					MU_WLEU16(fmt+2, wrapper->num_channels);
					// - dwSamplesPerSec
					MU_WLEU32(fmt+4, wrapper->sample_rate);
					// - dwAvgBytesPerSec
					uint64_m avg_bytes = ((uint64_m)wrapper->num_channels) * ((uint64_m)wrapper->sample_rate) * 4;
					if (avg_bytes > 0xFFFFFFFF) {
						return MUAF_INVALID_WAVE_FILE_WRITE_SIZE;
					}
					MU_WLEU32(fmt+8, avg_bytes);
					// - wBlockAlign
					uint32_m block_align = ((uint32_m)wrapper->num_channels) * 4;
					if (block_align > 0xFFFFFFFF) {
						return MUAF_INVALID_WAVE_FILE_WRITE_SIZE;
					}
					MU_WLEU16(fmt+12, block_align);

					// - wBitsPerSample
					MU_WLEU16(fmt+14, 32);

					// Write to file
					muafInner_WriteToFile(file, wrapper->chunks.fmt, 16, fmt);
					return MUAF_SUCCESS;
				}

				// MUAF_FORMAT_PCM_S64
				muafResult muafWAVE_fmt_FORMAT_PCM_S64(muafInner_File* file, muWAVEWrapper* wrapper) {
					// Fill out info
					muByte fmt[16];

					// - wFormatTag
					MU_WLEU16(fmt, MU_WAVE_FORMAT_PCM);
					// - wChannels
					MU_WLEU16(fmt+2, wrapper->num_channels);
					// - dwSamplesPerSec
					MU_WLEU32(fmt+4, wrapper->sample_rate);
					// - dwAvgBytesPerSec
					uint64_m avg_bytes = ((uint64_m)wrapper->num_channels) * ((uint64_m)wrapper->sample_rate) * 8;
					if (avg_bytes > 0xFFFFFFFF) {
						return MUAF_INVALID_WAVE_FILE_WRITE_SIZE;
					}
					MU_WLEU32(fmt+8, avg_bytes);
					// - wBlockAlign
					uint32_m block_align = ((uint32_m)wrapper->num_channels) * 8;
					if (block_align > 0xFFFFFFFF) {
						return MUAF_INVALID_WAVE_FILE_WRITE_SIZE;
					}
					MU_WLEU16(fmt+12, block_align);

					// - wBitsPerSample
					MU_WLEU16(fmt+14, 64);

					// Write to file
					muafInner_WriteToFile(file, wrapper->chunks.fmt, 16, fmt);
					return MUAF_SUCCESS;
				}

				// Writes fmt given format to write it in
				muafResult muafWAVE_fmtWrite(muafInner_File* file, muWAVEWrapper* wrapper) {
					switch (wrapper->audio_format) {
						default: return MUAF_FAILED_UNSUPPORTED_AUDIO_FORMAT; break;
						case MUAF_FORMAT_PCM_U8 : return muafWAVE_fmt_FORMAT_PCM_U8 (file, wrapper); break;
						case MUAF_FORMAT_PCM_S16: return muafWAVE_fmt_FORMAT_PCM_S16(file, wrapper); break;
						case MUAF_FORMAT_PCM_S32: return muafWAVE_fmt_FORMAT_PCM_S32(file, wrapper); break;
						case MUAF_FORMAT_PCM_S64: return muafWAVE_fmt_FORMAT_PCM_S64(file, wrapper); break;
					}
				}

			/* Wrapper writing */

				// Fills out the information for the chunks of a WAVE wrapper
				// Writes the ultimate file length
				muafResult muafWAVE_FillChunks(muWAVEWrapper* wrapper, size_m* len) {
					// Initialize all chunk data to 0
					mu_memset(&wrapper->chunks, 0, sizeof(wrapper->chunks));

					// Write fmt chunk after RIFF (4), ckSize (4), WAVE (4), ckID (4), and ckSize (4)
					wrapper->chunks.fmt = 20;
					// Set initial length to common-fields
					wrapper->chunks.fmt_len = 14;

					// Add to fmt length based on format (and calculate sample size)
					uint64_m sample_size = 1;
					switch (wrapper->audio_format) {
						default: return MUAF_FAILED_UNSUPPORTED_AUDIO_FORMAT; break;

						// PCM
						case MUAF_FORMAT_PCM_U8: {
							wrapper->chunks.fmt_len += 2;
							sample_size = 1;
						} break;
						case MUAF_FORMAT_PCM_S16: {
							wrapper->chunks.fmt_len += 2;
							sample_size = 2;
						} break;
						case MUAF_FORMAT_PCM_S32: {
							wrapper->chunks.fmt_len += 2;
							sample_size = 4;
						} break;
						case MUAF_FORMAT_PCM_S64: {
							wrapper->chunks.fmt_len += 2;
							sample_size = 8;
						} break;
					}

					// Write WAVE chunk after fmt chunk, accounting for padding and WAVE's ckID and ckSize
					wrapper->chunks.wave = wrapper->chunks.fmt + wrapper->chunks.fmt_len + 8;
					if (wrapper->chunks.wave % 2 != 0) {
						wrapper->chunks.wave += 1;
					}
					// WAVE chunk length
					wrapper->chunks.wave_len = wrapper->num_frames * sample_size * wrapper->num_channels;

					// Write length
					*len = ((size_m)wrapper->chunks.wave) + ((size_m)wrapper->chunks.wave_len);
					if (*len % 2 != 0) {
						*len += 1;
					}

					return MUAF_SUCCESS;
				}

				// Writes RIFF and WAVE wrapper for file
				muafResult muafWAVE_WriteRIFFWrapper(muafInner_File* file) {
					// RIFF wrapper (+ WAVE)
					muByte wrapper[12] = {
						// RIFF
						0x52, 0x49, 0x46, 0x46,
						// ckSize (set in a second)
						0x00, 0x00, 0x00, 0x00,
						// WAVE
						0x57, 0x41, 0x56, 0x45
					};

					// Make sure file size is within u32 range, excluding RIFF and ckSize
					if ((file->len - 8) > 0xFFFFFFFF) {
						return MUAF_INVALID_WAVE_FILE_WRITE_SIZE;
					}
					// Write ckSize
					MU_WLEU32(wrapper+4, file->len - 8);

					// Write data to file
					muafInner_WriteToFile(file, 0, sizeof(wrapper), wrapper);

					return MUAF_SUCCESS;
				}

				// Writes a given chunk header
				void muafWAVE_WriteChunkHeader(muafInner_File* file, uint32_m index, uint32_m ck_id, uint32_m ck_len) {
					// Don't do anything if chunk doesn't exist
					if (index == 0) {
						return;
					}

					// Setup chunk header
					muByte header[8];
					MU_WBEU32(header, ck_id);
					MU_WLEU32(header+4, ck_len);

					// Write chunk header to file
					muafInner_WriteToFile(file, index-8, sizeof(header), header);
				}

				// Writes empty, initial chunk headers
				void muafWAVE_InitChunks(muafInner_File* file, muWAVEWrapper* wrapper) {
					muafWAVE_WriteChunkHeader(file, wrapper->chunks.fmt,        0x666D7420, wrapper->chunks.fmt_len);
					muafWAVE_WriteChunkHeader(file, wrapper->chunks.wave,       0x64617461, wrapper->chunks.wave_len);
					muafWAVE_WriteChunkHeader(file, wrapper->chunks.fact,       0x66616374, wrapper->chunks.fact_len);
					muafWAVE_WriteChunkHeader(file, wrapper->chunks.cue,        0x63756520, wrapper->chunks.cue_len);
					muafWAVE_WriteChunkHeader(file, wrapper->chunks.playlist,   0x706C7374, wrapper->chunks.playlist_len);
					muafWAVE_WriteChunkHeader(file, wrapper->chunks.assoc_data, 0x4C495354, wrapper->chunks.assoc_data_len);
				}

				// Creates WAVE file wrapper
				MUDEF muafResult mu_create_WAVE_wrapper(const char* filename, muWAVEWrapper* wrapper) {
					// Initialize chunk info
					size_m len;
					muafResult res = muafWAVE_FillChunks(wrapper, &len);
					if (muaf_result_is_fatal(res)) {
						return res;
					}

					// Create the file
					muafInner_File file;
					if (muafInner_CreateFile(filename, &file, len) != 0) {
						return MUAF_FAILED_CREATE_FILE;
					}

					// Write RIFF and WAVE wrapper
					res = muafWAVE_WriteRIFFWrapper(&file);
					if (muaf_result_is_fatal(res)) {
						muafInner_DeloadFile(&file);
						return res;
					}

					// Write empty chunks
					muafWAVE_InitChunks(&file, wrapper);

					// Write fmt chunk
					res = muafWAVE_fmtWrite(&file, wrapper);
					if (muaf_result_is_fatal(res)) {
						muafInner_DeloadFile(&file);
						return res;
					}

					// Close file
					muafInner_DeloadFile(&file);
					return res;
				}

				// Destroys WAVE file wrapper
				MUDEF void mu_free_WAVE_wrapper(muWAVEWrapper* wrapper) {
					// Does nothing currently (sorry lol :P)
					return; if (wrapper) {}
				}

				// Converts unspecific wrapper to WAVE wrapper
				void muafWAVE_UnspecificWrapperToWAVE(muafUnspecificWrapper* unspecific, muWAVEWrapper* wave) {
					wave->audio_format = unspecific->audio_format;
					wave->num_frames = unspecific->num_frames;
					wave->num_channels = unspecific->num_channels;
					wave->sample_rate = unspecific->sample_rate;
				}

			/* PCM writing */

				// MUAF_FORMAT_PCM_U8
				void muafWAVE_write_FORMAT_PCM_U8(muafInner_File* file, muWAVEWrapper* wrapper, muafFrames beg_frame, muafFrames frame_len, uint8_m* data) {
					// Write data over
					muafInner_WriteToFile(file, wrapper->chunks.wave + (beg_frame * wrapper->num_channels), frame_len * wrapper->num_channels, data);
				}

				// MUAF_FORMAT_PCM_S16
				void muafWAVE_write_FORMAT_PCM_S16(muafInner_File* file, muWAVEWrapper* wrapper, muafFrames beg_frame, muafFrames frame_len, int16_m* data) {
					// Convert each sample
					muafFrames sample_count = frame_len * wrapper->num_channels;
					for (muafFrames s = 0; s < sample_count; ++s) {
						muByte sample_data[2];
						MU_WLES16(sample_data, data[s]);
						mu_memcpy(&data[s], sample_data, 2);
					}

					// Write data over
					muafInner_WriteToFile(file, wrapper->chunks.wave + (beg_frame * 2 * wrapper->num_channels), frame_len * 2 * wrapper->num_channels, (muByte*)data);
				}

				// MUAF_FORMAT_PCM_S32
				void muafWAVE_write_FORMAT_PCM_S32(muafInner_File* file, muWAVEWrapper* wrapper, muafFrames beg_frame, muafFrames frame_len, int32_m* data) {
					// Convert each sample
					muafFrames sample_count = frame_len * wrapper->num_channels;
					for (muafFrames s = 0; s < sample_count; ++s) {
						muByte sample_data[4];
						MU_WLES32(sample_data, data[s]);
						mu_memcpy(&data[s], sample_data, 4);
					}

					// Write data over
					muafInner_WriteToFile(file, wrapper->chunks.wave + (beg_frame * 4 * wrapper->num_channels), frame_len * 4 * wrapper->num_channels, (muByte*)data);
				}

				// MUAF_FORMAT_PCM_S64
				void muafWAVE_write_FORMAT_PCM_S64(muafInner_File* file, muWAVEWrapper* wrapper, muafFrames beg_frame, muafFrames frame_len, int64_m* data) {
					// Convert each sample
					muafFrames sample_count = frame_len * wrapper->num_channels;
					for (muafFrames s = 0; s < sample_count; ++s) {
						muByte sample_data[8];
						MU_WLES64(sample_data, data[s]);
						mu_memcpy(&data[s], sample_data, 8);
					}

					// Write data over
					muafInner_WriteToFile(file, wrapper->chunks.wave + (beg_frame * 8 * wrapper->num_channels), frame_len * 8 * wrapper->num_channels, (muByte*)data);
				}

			/* Audio writing */

				// Writes uncompressed raw WAVE data given inner file
				muafResult muafWAVE_WriteUncompressed(muafInner_File* file, muWAVEWrapper* wrapper, muafFrames beg_frame, muafFrames frame_len, void* data) {
					// Perform writing based on format
					switch (wrapper->audio_format) {
						// Unknown
						default: return MUAF_FAILED_UNSUPPORTED_AUDIO_FORMAT; break;
						// WAVE PCM
						case MUAF_FORMAT_PCM_U8:  muafWAVE_write_FORMAT_PCM_U8 (file, wrapper, beg_frame, frame_len, (uint8_m*)data); break;
						case MUAF_FORMAT_PCM_S16: muafWAVE_write_FORMAT_PCM_S16(file, wrapper, beg_frame, frame_len, (int16_m*)data); break;
						case MUAF_FORMAT_PCM_S32: muafWAVE_write_FORMAT_PCM_S32(file, wrapper, beg_frame, frame_len, (int32_m*)data); break;
						case MUAF_FORMAT_PCM_S64: muafWAVE_write_FORMAT_PCM_S64(file, wrapper, beg_frame, frame_len, (int64_m*)data); break;
					}

					return MUAF_SUCCESS;
				}

				// Writes uncompressed raw WAVE data
				MUDEF muafResult mu_write_WAVE_uncompressed(const char* filename, muWAVEWrapper* wrapper, muafFrames beg_frame, muafFrames frame_len, void* data) {
					// Open file
					muafInner_File file;
					if (muafInner_CreateFile(filename, &file, 0) != 0) {
						return MUAF_FAILED_CREATE_FILE;
					}

					// Perform writing
					muafResult res = muafWAVE_WriteUncompressed(&file, wrapper, beg_frame, frame_len, data);

					// Close file
					muafInner_DeloadFile(&file);
					return res;
				}

				// Retrieves WAVE wrapper equivalent from existing file
				MUDEF muafResult mu_get_WAVE_wrapper(muWAVEProfile* profile, muWAVEWrapper* wrapper) {
					// Get audio format
					wrapper->audio_format = mu_WAVE_unspecific_audio_format(profile->format_tag, profile->specific_fields);
					// Calculate frame count
					wrapper->num_frames = profile->chunks.wave_len / profile->block_align;
					// Number of channels
					wrapper->num_channels = profile->channels;
					// Sample rate
					wrapper->sample_rate = profile->samples_per_sec;
					return MUAF_SUCCESS;
				}

				// Retrieves unspecific wrapper from WAVE file
				void muafWAVE_GetUnspecificWrapper(muWAVEProfile* profile, muafUnspecificWrapper* wrapper) {
					wrapper->num_frames = profile->chunks.wave_len / profile->block_align;
					wrapper->sample_rate = profile->samples_per_sec;
					wrapper->num_channels = profile->channels;
					wrapper->audio_format = mu_WAVE_unspecific_audio_format(profile->format_tag, profile->specific_fields);
					wrapper->file_format = MUAF_WAVE;
				}

		/* WAVE format-unspecific */

			// WAVE equivalent of mu_free_audio_file_profile
			void muafWAVE_FreeAudioFileProfile(muafUnspecificProfile* profile) {
				// If WAVE profile exists:
				if (profile->specific.WAVE) {
					// Destroy it
					mu_free_WAVE_profile(profile->specific.WAVE);
					// And then free it
					mu_free(profile->specific.WAVE);
				}
			}

			// WAVE equivalent of mu_get_audio_file_profile
			muafResult muafWAVE_GetAudioFileProfile(muafInner_File* file, muafUnspecificProfile* profile) {
				// Allocate WAVE profile
				profile->specific.WAVE = (muWAVEProfile*)mu_malloc(sizeof(muWAVEProfile));
				if (!profile->specific.WAVE) {
					return MUAF_FAILED_MALLOC;
				}

				// Get WAVE profile
				muafResult res = muafWAVE_GetWAVEProfile(file, profile->specific.WAVE);
				if (muaf_result_is_fatal(res)) {
					muafWAVE_FreeAudioFileProfile(profile);
					return res;
				}

				// Get frame count
				profile->num_frames = profile->specific.WAVE->chunks.wave_len / profile->specific.WAVE->block_align;
				// Get sample rate
				profile->sample_rate = profile->specific.WAVE->samples_per_sec;
				// Get channel count
				profile->num_channels = profile->specific.WAVE->channels;
				// Get audio format
				profile->audio_format = mu_WAVE_unspecific_audio_format(profile->specific.WAVE->format_tag, profile->specific.WAVE->specific_fields);
				// Set file format
				profile->file_format = MUAF_WAVE;

				return res;
			}

	/* FLAC */

		// @TODO Confirm frame values

		/* Profiling */

			// Checks if given data even is FLAC or not
			muBool muafFLAC_IsFLAC(muafInner_File* file) {
				// Minimum length check
				// Includes fLaC (4) and entire streaminfo metadata block (4 byte header + 34 descriptive bytes)
				if (file->len < 42) {
					return MU_FALSE;
				}

				// Load first 5 bytes
				muByte data[5];
				muafInner_LoadFromFile(file, 0, 5, data);

				// Check for fLaC
				if (MU_RBEU32(data) != 0x664C6143) {
					return MU_FALSE;
				}

				// Check for first metadata block being streaminfo
				if ((data[4] & 0x7F) != 0) {
					return MU_FALSE;
				}

				// Has fLaC and streaminfo metadata block, so likely FLAC.
				return MU_TRUE;
			}

			// Reads information from the streaminfo metadata block
			// Streaminfo's existence and its min. length should already be confirmed (muafFLAC_IsFLAC)
			muafResult muafFLAC_ProcessStreaminfo(muafInner_File* file, muFLACProfile* profile, muBool* more) {
				// Read streaminfo block (including header)
				muByte data[38];
				muafInner_LoadFromFile(file, 4, 38, data);

				// Read if there's more metadata blocks after this
				*more = (data[0] & 0x80) == 0;

				// Read and confirm streaminfo's listed length
				if (MU_RBEU24(data+1) != 34) {
					return MUAF_INVALID_FLAC_STREAMINFO_LENGTH;
				}

				// Read and confirm min. block size
				profile->min_block_size = MU_RBEU16(data+4);
				if (profile->min_block_size < 16) {
					return MUAF_INVALID_FLAC_STREAMINFO_BLOCK_SIZE_RANGE;
				}
				// Read and confirm max. block size
				profile->max_block_size = MU_RBEU16(data+6);
				if (profile->max_block_size < 16) {
					return MUAF_INVALID_FLAC_STREAMINFO_BLOCK_SIZE_RANGE;
				}
				if (profile->max_block_size < profile->min_block_size) {
					return MUAF_INVALID_FLAC_STREAMINFO_BLOCK_SIZE_MIN_MAX;
				}

				// Read min. frame size
				profile->min_frame_size = MU_RBEU24(data+8);
				// Read and confirm max. frame size
				profile->max_frame_size = MU_RBEU24(data+11);
				if (profile->min_frame_size != 0 && profile->max_frame_size != 0) {
					if (profile->max_frame_size < profile->min_frame_size) {
						return MUAF_INVALID_FLAC_STREAMINFO_FRAME_SIZE_MIN_MAX;
					}
				}

				// Read sample rate (b=14, l=20)
				profile->sample_rate = (MU_RBEU24(data+14) & 0xFFFFF0) >> 4;
				// Interpret sample rate
				profile->contains_audio = profile->sample_rate != 0;

				// Read and confirm number of channels (b=16, o=4, l=3)
				profile->num_channels = (*(data+16) & 0xE) >> 0x1;
				if (profile->num_channels++ == 0) {
					return MUAF_INVALID_FLAC_STREAMINFO_NUM_CHANNELS;
				}
				if (profile->contains_audio && profile->num_channels == 0) {
					return MUAF_INVALID_FLAC_STREAMINFO_NUM_CHANNELS;
				}

				// Read and confirm bits per sample (b=16, o=7, l=5)
				profile->bits_per_sample = (MU_RBEU16(data+16) & 0x1F0) >> 4;
				if (profile->bits_per_sample++ == 0) {
					return MUAF_INVALID_FLAC_STREAMINFO_BITS_PER_SAMPLE;
				}
				if (profile->contains_audio && profile->bits_per_sample == 0) {
					return MUAF_INVALID_FLAC_STREAMINFO_BITS_PER_SAMPLE;
				}

				// Read and confirm interchannel sample count (b=17, o=4, l=36)
				profile->num_samples = (MU_RBEU64(data+17) & 0xFFFFFFFFF000000) >> 24;
				if (
					( profile->contains_audio && profile->num_samples == 0) || 
					(!profile->contains_audio && profile->num_samples != 0)) {
					return MUAF_INVALID_FLAC_STREAMINFO_SAMPLE_COUNT;
				}

				// Read low and high bytes of checksum (b=22, l=128)
				profile->high_checksum = MU_RBEU64(data+22);
				profile->low_checksum = MU_RBEU64(data+30);
				return MUAF_SUCCESS;
			}

			// Gets FLAC profile given inner file
			muafResult muafFLAC_GetFLACProfile(muafInner_File* file, muFLACProfile* profile) {
				// Make sure it's FLAC
				if (!muafFLAC_IsFLAC(file)) {
					return MUAF_FAILED_AUDIO_FILE_FORMAT_IDENTIFICATION;
				}
				// Zero-out profile memory
				mu_memset(profile, 0, sizeof(muFLACProfile));

				// Get streaminfo
				muBool more;
				muafResult res = muafFLAC_ProcessStreaminfo(file, profile, &more);
				if (muaf_result_is_fatal(res)) {
					mu_free_FLAC_profile(profile);
					return res;
				}

				return res;
			}

			// Gets FLAC profile
			MUDEF muafResult mu_get_FLAC_profile(const char* filename, muFLACProfile* profile) {
				// Open file
				muafInner_File file;
				if (muafInner_LoadFile(filename, &file) != 0) {
					return MUAF_FAILED_OPEN_FILE;
				}

				// Get FLAC profile
				muafResult res = muafFLAC_GetFLACProfile(&file, profile);

				// Close file
				muafInner_DeloadFile(&file);
				return res;
			}

			// Frees FLAC profile
			MUDEF void mu_free_FLAC_profile(muFLACProfile* profile) {
				// Does nothing currently, sorry!
				return; if (profile) {}
			}

	/* Format-unspecific API */

		// Get audio file format
		MUDEF muafFileFormat mu_audio_file_format(const char* filename) {
			// Open file
			muafInner_File file;
			if (muafInner_LoadFile(filename, &file) != 0) {
				return MUAF_UNKNOWN;
			}

			// Try each format lol
			muafFileFormat format = MUAF_UNKNOWN;
			if (muafWAVE_IsWAVE(&file)) {
				format = MUAF_WAVE;
			}
			else if (muafFLAC_IsFLAC(&file)) {
				format = MUAF_FLAC;
			}

			// Close file
			muafInner_DeloadFile(&file);
			return format;
		}

		// Get unspecific profile
		MUDEF muafResult mu_get_audio_file_profile(const char* filename, muafUnspecificProfile* profile) {
			// Open file
			muafInner_File file;
			if (muafInner_LoadFile(filename, &file) != 0) {
				return MUAF_FAILED_OPEN_FILE;
			}

			// Perform based on file format
			muafResult res;
			// - WAVE
			if (muafWAVE_IsWAVE(&file)) {
				res = muafWAVE_GetAudioFileProfile(&file, profile);
			}
			// - Unrecognized
			else {
				res = MUAF_FAILED_UNSUPPORTED_AUDIO_FILE_FORMAT;
			}

			// Close file
			muafInner_DeloadFile(&file);
			return res;
		}

		// Frees unspecific profile
		MUDEF void mu_free_audio_file_profile(muafUnspecificProfile* profile) {
			// Perform based on format
			switch (profile->file_format) {
				case MUAF_WAVE: muafWAVE_FreeAudioFileProfile(profile); break;
			}
		}

		// Returns whether or not given audio format is compressed
		MUDEF muBool muaf_audio_format_compressed(muafAudioFormat format) {
			switch (format) {
				default: return MU_FALSE; break;
			}
		}

		// Returns uncompressed audio format sample size
		MUDEF size_m muaf_audio_format_sample_size(muafAudioFormat format) {
			// Perform based on format
			switch (format) {
				// Unknown
				default: return 0; break;

				// PCM
				case MUAF_FORMAT_PCM_U8 : return 1; break;
				case MUAF_FORMAT_PCM_S16: return 2; break;
				case MUAF_FORMAT_PCM_S32: return 4; break;
				case MUAF_FORMAT_PCM_S64: return 8; break;
			}
		}

		// Reads uncompressed raw data
		MUDEF muafResult mu_read_uncompressed_audio_file(const char* filename, muafUnspecificProfile* profile, muafFrames beg_frame, muafFrames frame_len, void* data) {
			// Open file
			muafInner_File file;
			if (muafInner_LoadFile(filename, &file) != 0) {
				return MUAF_FAILED_OPEN_FILE;
			}

			// Perform based on file format
			muafResult res;
			switch (profile->file_format) {
				default: res = MUAF_FAILED_UNSUPPORTED_AUDIO_FILE_FORMAT; break;
				case MUAF_WAVE: res = muafWAVE_ReadUncompressed(&file, profile->specific.WAVE, profile->audio_format, beg_frame, frame_len, data); break;
			}

			// Close file
			muafInner_DeloadFile(&file);
			return res;
		}

		// Creates an audio file wrapper
		MUDEF muafResult mu_create_audio_file_wrapper(const char* filename, muafUnspecificWrapper* wrapper) {
			// Perform based on file format
			switch (wrapper->file_format) {
				// Unknown
				default: return MUAF_FAILED_UNSUPPORTED_AUDIO_FILE_FORMAT; break;

				// WAVE
				case MUAF_WAVE: {
					// Allocate WAVE wrapper
					wrapper->specific.WAVE = (muWAVEWrapper*)mu_malloc(sizeof(muWAVEWrapper));
					if (!wrapper->specific.WAVE) {
						return MUAF_FAILED_MALLOC;
					}
					// Set WAVE wrapper equivalent
					muafWAVE_UnspecificWrapperToWAVE(wrapper, wrapper->specific.WAVE);

					// Create wrapper
					muafResult res = mu_create_WAVE_wrapper(filename, wrapper->specific.WAVE);
					if (muaf_result_is_fatal(res)) {
						mu_free_audio_file_wrapper(wrapper);
					}
					return res;
				} break;
			}
		}

		// Frees an audio file wrapper
		MUDEF void mu_free_audio_file_wrapper(muafUnspecificWrapper* wrapper) {
			// Perform based on file format
			switch (wrapper->file_format) {
				// Unknown
				default: break;

				// WAVE
				case MUAF_WAVE: {
					// If it exists:
					if (wrapper->specific.WAVE) {
						// Destroy it
						mu_free_WAVE_wrapper(wrapper->specific.WAVE);
						// And free it
						mu_free(wrapper->specific.WAVE);
					}
				} break;
			}
		}

		// Writes uncompressed raw data
		MUDEF muafResult mu_write_uncompressed_audio_file(const char* filename, muafUnspecificWrapper* wrapper, muafFrames beg_frame, muafFrames frame_len, void* data) {
			// Perform based on file format
			switch (wrapper->file_format) {
				default: return MUAF_FAILED_UNSUPPORTED_AUDIO_FILE_FORMAT; break;
				case MUAF_WAVE: return mu_write_WAVE_uncompressed(filename, wrapper->specific.WAVE, beg_frame, frame_len, data); break;
			}
		}

		// Gets wrapper from audio file
		MUDEF muafResult mu_get_audio_file_wrapper(muafUnspecificProfile* profile, muafUnspecificWrapper* wrapper) {
			// Perform based on file format
			switch (profile->file_format) {
				default: return MUAF_FAILED_UNSUPPORTED_AUDIO_FILE_FORMAT; break;
				case MUAF_WAVE: muafWAVE_GetUnspecificWrapper(profile->specific.WAVE, wrapper); break;
			}

			return MUAF_SUCCESS;
		}

	/* Names */

		#ifdef MUAF_NAMES

		MUDEF const char* muaf_audio_format_get_name(muafAudioFormat format) {
			switch (format) {
				default: return "MUAF_FORMAT_UNKNOWN"; break;
				case MUAF_FORMAT_PCM_U8: return "MUAF_FORMAT_PCM_U8"; break;
				case MUAF_FORMAT_PCM_S16: return "MUAF_FORMAT_PCM_S16"; break;
				case MUAF_FORMAT_PCM_S32: return "MUAF_FORMAT_PCM_S32"; break;
				case MUAF_FORMAT_PCM_S64: return "MUAF_FORMAT_PCM_S64"; break;
			}
		}

		MUDEF const char* muaf_audio_format_get_nice_name(muafAudioFormat format) {
			switch (format) {
				default: return "Unknown"; break;
				case MUAF_FORMAT_PCM_U8: return "8-bit unsigned PCM"; break;
				case MUAF_FORMAT_PCM_S16: return "16-bit signed PCM"; break;
				case MUAF_FORMAT_PCM_S32: return "32-bit signed PCM"; break;
				case MUAF_FORMAT_PCM_S64: return "64-bit signed PCM"; break;
			}
		}

		MUDEF const char* muaf_audio_file_format_get_name(muafFileFormat format) {
			switch (format) {
				default: return "MUAF_UNKNOWN"; break;
				case MUAF_WAVE: return "MUAF_WAVE"; break;
				case MUAF_FLAC: return "MUAF_FLAC"; break;
			}
		}

		MUDEF const char* muaf_audio_file_format_get_nice_name(muafFileFormat format) {
			switch (format) {
				default: return "Unknown"; break;
				case MUAF_WAVE: return "WAVE (.wav, .wave)"; break;
				case MUAF_FLAC: return "FLAC (.flac)"; break;
			}
		}

		MUDEF const char* mu_WAVE_format_get_name(uint16_m format_tag) {
			switch (format_tag) {
				default: return "MU_UNKNOWN"; break;
				case MU_WAVE_FORMAT_PCM: return "MU_WAVE_FORMAT_PCM"; break;
			}
		}

		MUDEF const char* muaf_result_get_name(muafResult result) {
			switch (result) {
				default: return "MU_UNKNOWN"; break;
				case MUAF_SUCCESS: return "MUAF_SUCCESS"; break;
				case MUAF_FAILED_MALLOC: return "MUAF_FAILED_MALLOC"; break;
				case MUAF_FAILED_OPEN_FILE: return "MUAF_FAILED_OPEN_FILE"; break;
				case MUAF_FAILED_UNSUPPORTED_AUDIO_FORMAT: return "MUAF_FAILED_UNSUPPORTED_AUDIO_FORMAT"; break;
				case MUAF_FAILED_CREATE_FILE: return "MUAF_FAILED_CREATE_FILE"; break;
				case MUAF_FAILED_UNSUPPORTED_AUDIO_FILE_FORMAT: return "MUAF_FAILED_UNSUPPORTED_AUDIO_FILE_FORMAT"; break;
				case MUAF_FAILED_AUDIO_FILE_FORMAT_IDENTIFICATION: return "MUAF_FAILED_AUDIO_FILE_FORMAT_IDENTIFICATION"; break;
				case MUAF_INVALID_WAVE_CHUNK_LENGTH_FOR_FILE: return "MUAF_INVALID_WAVE_CHUNK_LENGTH_FOR_FILE"; break;
				case MUAF_INVALID_WAVE_MISSING_FMT: return "MUAF_INVALID_WAVE_MISSING_FMT"; break;
				case MUAF_INVALID_WAVE_MISSING_WAVE_DATA: return "MUAF_INVALID_WAVE_MISSING_WAVE_DATA"; break;
				case MUAF_INVALID_WAVE_FMT_LENGTH: return "MUAF_INVALID_WAVE_FMT_LENGTH"; break;
				case MUAF_INVALID_WAVE_FMT_CHANNELS: return "MUAF_INVALID_WAVE_FMT_CHANNELS"; break;
				case MUAF_INVALID_WAVE_FMT_SAMPLES_PER_SEC: return "MUAF_INVALID_WAVE_FMT_SAMPLES_PER_SEC"; break;
				case MUAF_INVALID_WAVE_FMT_PCM_BITS_PER_SAMPLE: return "MUAF_INVALID_WAVE_FMT_PCM_BITS_PER_SAMPLE"; break;
				case MUAF_INVALID_WAVE_FILE_WRITE_SIZE: return "MUAF_INVALID_WAVE_FILE_WRITE_SIZE"; break;
				case MUAF_INVALID_FLAC_STREAMINFO_LENGTH: return "MUAF_INVALID_FLAC_STREAMINFO_LENGTH"; break;
				case MUAF_INVALID_FLAC_STREAMINFO_BLOCK_SIZE_RANGE: return "MUAF_INVALID_FLAC_STREAMINFO_BLOCK_SIZE_RANGE"; break;
				case MUAF_INVALID_FLAC_STREAMINFO_BLOCK_SIZE_MIN_MAX: return "MUAF_INVALID_FLAC_STREAMINFO_BLOCK_SIZE_MIN_MAX"; break;
				case MUAF_INVALID_FLAC_STREAMINFO_FRAME_SIZE_MIN_MAX: return "MUAF_INVALID_FLAC_STREAMINFO_FRAME_SIZE_MIN_MAX"; break;
				case MUAF_INVALID_FLAC_STREAMINFO_NUM_CHANNELS: return "MUAF_INVALID_FLAC_STREAMINFO_NUM_CHANNELS"; break;
				case MUAF_INVALID_FLAC_STREAMINFO_BITS_PER_SAMPLE: return "MUAF_INVALID_FLAC_STREAMINFO_BITS_PER_SAMPLE"; break;
				case MUAF_INVALID_FLAC_STREAMINFO_SAMPLE_COUNT: return "MUAF_INVALID_FLAC_STREAMINFO_SAMPLE_COUNT"; break;
			}
		}

		#endif

	/* Result */

		MUDEF muBool muaf_result_is_fatal(muafResult result) {
			switch (result) {
				default: return MU_TRUE; break;
				case MUAF_SUCCESS:
					return MU_FALSE;
				break;
			}
		}

	MU_CPP_EXTERN_END
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

