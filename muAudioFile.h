/*
muAudioFile.h - Muukid
Public domain single-file C library for reading and writing audio file data.
https://github.com/Muukid/muAudioFile
No warranty implied; use at your own risk.

Licensed under MIT License or public domain, whichever you prefer.
More explicit license information at the end of file.

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

	// @DOCLINE # Format-inspecific API

		// @DOCLINE muaf's API is split into two sections: the format-specific API, and the format-inspecific API. The inspecific API wraps around the specific API, encapsulating the specific API's functionality, and allowing you to retrieve general information about an audio file without having to directly consider what specific audio file format it is.

		// @DOCLINE ## Audio profile

			// @DOCLINE For each audio format, general information about the audio stored in the file can be retrieved. This general information is called the "profile". This profile is defined for several reasons:

			// @DOCLINE * It gives the user a way to load general information about the audio file first, before taking more processing time and memory to load more significant portions of the data stored within the audio file.

			// @DOCLINE * Having this information gives the user a way to load select portions of the audio data encoded in the file without ever having to load the whole audio file at once in memory, which can be useful or even necessary for larger audio files.

			// @DOCLINE * It allows the API to internally map how the file is formatted and hold onto that information for later, which can make subsequent calls to load information from the audio file quicker, since information about the file's general structure is held onto across function calls.

			// @DOCLINE In the format-specific API, the profile stores general information that very directly corresponds to the general information stored in the audio file itself. For example, [the WAVE profile](#wave-profile) stores information about what supported chunks are provided in the file and where they are. It also stores the information within certain chunks that provide general information about the audio, such as the fmt chunk, which provides information such as the number of channels.

			// @DOCLINE In the format-inspecific API, the profile primarily stores general information that all of the supported audio file formats within muaf have in common. This allows the user to retrieve information about the audio file without having to know how to retrieve it from the exact audio file format that it's in. The inspecific profile also loads and stores the specific profile information.

			typedef struct muafInspecificProfile muafInspecificProfile;
			// @DOCLINE An audio file's inspecific profile can be retrieved using the function `mu_get_audio_file_profile`, defined below: @NLNT
			MUDEF muafResult mu_get_audio_file_profile(const char* filename, muafInspecificProfile* profile);

			// @DOCLINE Once an inspecific profile has been successfully or non-fatally retrieved (indicated by [the result return value](#result)), the profile is filled with data, some of which may be manually allocated automatically. To free this data, use the function `mu_free_audio_file_profile`, defined below: @NLNT
			MUDEF void mu_free_audio_file_profile(muafInspecificProfile* profile);

			// @DOCLINE The inspecific profile is represented by the struct `muafInspecificProfile`, which has the following members:

			// This union is described later, make sure it's updated properly!
			typedef struct muWAVEProfile muWAVEProfile;
			union muafSpecificProfile {
				muWAVEProfile* WAVE;
			};
			typedef union muafSpecificProfile muafSpecificProfile;

			typedef uint32_m muafSampleRate;
			typedef uint64_m muafSampleCount;
			typedef uint16_m muafChannels;
			typedef uint8_m muafFormat;
			struct muafInspecificProfile {
				// @DOCLINE * `@NLFT num_samples` - the number of samples.
				muafSampleCount num_samples;
				// @DOCLINE * `@NLFT sample_rate` - the rate of samples per second.
				muafSampleRate sample_rate;
				// @DOCLINE * `@NLFT num_channels` - the number of channels.
				muafChannels num_channels;
				// @DOCLINE * `@NLFT file_format` - the [file format](#audio-file-formats).
				muafFormat file_format;
				// @DOCLINE * `@NLFT specific` - the [format-specific profile](#format-specific-profile).
				muafSpecificProfile specific;
			};

			// @DOCLINE ### Audio file formats

				// @DOCLINE The type `muafFormat` represents a file format supported in muaf, and has the following defined values:

				// @DOCLINE * `MUAF_UNKNOWN` - an unknown/unrecognized file format.
				#define MUAF_UNKNOWN 0
				// @DOCLINE * `MUAF_WAVE` - the [WAVE file format](#wave-api).
				#define MUAF_WAVE 1

			// @DOCLINE ### Format specific profile

				// @DOCLINE The union `muafSpecificProfile` acts as a container for the format-specific profile of some audio file, and has the following members:

				// @DOCLINE * `muWAVEProfile* WAVE` - the [WAVE profile](#wave-profile).

		// @DOCLINE ## Get audio file format

			// @DOCLINE The function `mu_audio_file_format` retrieves the [audio file format](#audio-file-formats) of a given file, defined below: @NLNT
			MUDEF muafFormat mu_audio_file_format(const char* filename);

			// @DOCLINE This function does not use the filename extension to identify the audio file format, but instead, the actual contents of the file itself. This function returns `MUAF_UNKNOWN` if rather the audio file format could not be identified to be anything supported in muaf, or muaf failed to retrieve the file's data.

	// @DOCLINE # WAVE API

		// @DOCLINE This section describes muaf's API for the [Waveform Audio File Format](https://en.wikipedia.org/wiki/WAV), or WAVE. The code for this API is built based off of the original August 1991 specification for WAVE (specifically [this archive](https://www.mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/Docs/riffmci.pdf)), and this section of muaf's documentation will reference values that are defined in this specification.

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
				// @DOCLINE * `@NLFT samples_per_sec` - the value of dwSamplesPerSec in fmt's common-fields; the sampling rate.
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
						// @DOCLINE * `@NLFT bits_per_sample` - the value of wBitsPerSample in format-specific-fields's PCM-format-specific form; the sample size.
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

		// @DOCLINE ## WAVE known bugs and limitations

			// @DOCLINE This section covers the known bugs and limitations for the implementation of WAVE in muaf.

			// @DOCLINE ### Support for LIST wave-data

				// @DOCLINE muaf does not currently support wave-data that's provided in the form of a LIST. It only supports wave-data in the form of a data-ck chunk.

			// @DOCLINE ### Support for wBitsPerSample

				// @DOCLINE muaf does not currently support values for wBitsPerSample that are not divisible by 8.

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
				// @DOCLINE * `MUAF_INVALID_WAVE_FMT_PCM_BITS_PER_SAMPLE` - the WAVE chunk fmt-ck's PCM-format-specific value 'wBitsPerSample' has an invalid value; it's rather equal to 0, non-divisible by 8 without a remainder, or doesn't align with the value for wBlockAlign.
				#define MUAF_INVALID_WAVE_FMT_PCM_BITS_PER_SAMPLE 1030
				// @DOCLINE * `MUAF_FAILED_WAVE_IDENTIFICATION` - an attempt to retrieve WAVE information from a file that doesn't appear to be a WAVE file has occurred.
				#define MUAF_FAILED_WAVE_IDENTIFICATION 1031

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
			!defined(mu_ftell)

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

		#if !defined(mu_memset)

			// @DOCLINE ## `string.h` dependencies
			#include <string.h>

			// @DOCLINE * `mu_memset` - equivalent to `memset`.
			#ifndef mu_memset
				#define mu_memset memset
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
			//mu_fseek(file->fptr, 0L, MU_SEEK_SET);

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

	/* WAVE */

		// @TODO If listed ckSize is shorter than file length, set file length to it

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

		// Gets WAVE profile
		MUDEF muafResult mu_get_WAVE_profile(const char* filename, muWAVEProfile* profile) {
			// Open file
			muafInner_File file;
			if (muafInner_LoadFile(filename, &file) != 0) {
				return MUAF_FAILED_OPEN_FILE;
			}
			// Make sure it's WAVE
			if (!muafWAVE_IsWAVE(&file)) {
				return MUAF_FAILED_WAVE_IDENTIFICATION;
			}
			// Correct file length based on ckSize
			muafWAVE_HandleCkSize(&file);

			// Zero-out profile memory
			mu_memset(profile, 0, sizeof(muWAVEProfile));

			// Get chunk information
			muafResult res = muafWAVE_GetChunks(&file, profile);
			if (muaf_result_is_fatal(res)) {
				return res;
			}

			// Get fmt information
			res = muafWAVE_GetFmtInfo(&file, profile);
			if (muaf_result_is_fatal(res)) {
				mu_free_WAVE_profile(profile);
				return res;
			}

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

	/* Format-inspecific API */

		// Get audio file format
		MUDEF muafFormat mu_audio_file_format(const char* filename) {
			// Open file
			muafInner_File file;
			if (muafInner_LoadFile(filename, &file) != 0) {
				return MUAF_UNKNOWN;
			}

			// Try each format lol
			muafFormat format = MUAF_UNKNOWN;
			if (muafWAVE_IsWAVE(&file)) {
				format = MUAF_WAVE;
			}

			// Close file
			muafInner_DeloadFile(&file);
			return format;
		}

	/* Names */

		#ifdef MUAF_NAMES

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
				case MUAF_INVALID_WAVE_CHUNK_LENGTH_FOR_FILE: return "MUAF_INVALID_WAVE_CHUNK_LENGTH_FOR_FILE"; break;
				case MUAF_INVALID_WAVE_MISSING_FMT: return "MUAF_INVALID_WAVE_MISSING_FMT"; break;
				case MUAF_INVALID_WAVE_MISSING_WAVE_DATA: return "MUAF_INVALID_WAVE_MISSING_WAVE_DATA"; break;
				case MUAF_INVALID_WAVE_FMT_LENGTH: return "MUAF_INVALID_WAVE_FMT_LENGTH"; break;
				case MUAF_INVALID_WAVE_FMT_CHANNELS: return "MUAF_INVALID_WAVE_FMT_CHANNELS"; break;
				case MUAF_INVALID_WAVE_FMT_SAMPLES_PER_SEC: return "MUAF_INVALID_WAVE_FMT_SAMPLES_PER_SEC"; break;
				case MUAF_INVALID_WAVE_FMT_PCM_BITS_PER_SAMPLE: return "MUAF_INVALID_WAVE_FMT_PCM_BITS_PER_SAMPLE"; break;
				case MUAF_FAILED_WAVE_IDENTIFICATION: return "MUAF_FAILED_WAVE_IDENTIFICATION"; break;
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

