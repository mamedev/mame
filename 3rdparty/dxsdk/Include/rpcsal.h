/****************************************************************\
*                                                                *
* rpcsal.h - markers for documenting the semantics of RPC APIs   *
*                                                                *
* Version 1.0                                                    *
*                                                                *
* Copyright (c) 2004 Microsoft Corporation. All rights reserved. *
*                                                                *
\****************************************************************/

// -------------------------------------------------------------------------------
// Introduction
//
// rpcsal.h provides a set of annotations to describe how RPC functions use their
// parameters - the assumptions it makes about them, adn the guarantees it makes 
// upon finishing.  These annotations are similar to those found in specstrings.h,
// but are designed to be used by the MIDL compiler when it generates annotations
// enabled header files.
//
// IDL authors do not need to annotate their functions declarations.  The MIDL compiler
// will interpret the IDL directives and use one of the annotations contained 
// in this header.  This documentation is intended to help those trying to  understand 
// the MIDL-generated header files or those who maintain their own copies of these files.
//
// -------------------------------------------------------------------------------
// Differences between rpcsal.h and specstrings.h
// 
// There are a few important differences between the annotations found in rpcsal.h and
// those in specstrings.h:
// 
// 1. [in] parameters are not marked as read-only.  They may be used for scratch space 
// at the server and changes will not affect the client.
// 2. String versions of each macro alleviates the need for a special type definition
//
// -------------------------------------------------------------------------------
// Interpreting RPC Annotations
//
// These annotations are interpreted precisely in the same way as those in specstrings.h.  
// Please refer to that header for information related to general usage in annotations. 
//
// To construct an RPC annotation, concatenate the appropriate value from each category
// along with a leading __RPC_.  A typical annotation looks like "__RPC__in_string".
//
// |----------------------------------------------------------------------------------|
// | RPC Annotations                                                                  |
// |------------|------------|---------|--------|----------|----------|---------------|
// |   Level    |   Usage    |  Size   | Output | Optional |  String  |  Parameters   |
// |------------|------------|---------|--------|----------|----------|---------------|
// | <>         | <>         | <>      | <>     | <>       | <>       | <>            |
// | _deref     | _in        | _ecount | _full  | _opt     | _string  | (size)        |
// | _deref_opt | _out       | _bcount | _part  |          |          | (size,length) |
// |            | _inout     |         |        |          |          |               |
// |            |            |         |        |          |          |               |
// |------------|------------|---------|--------|----------|----------|---------------|
//
// Level: Describes the buffer pointer's level of indirection from the parameter or
//          return value 'p'.
//
// <>         : p is the buffer pointer.
// _deref     : *p is the buffer pointer. p must not be NULL.
// _deref_opt : *p may be the buffer pointer. p may be NULL, in which case the rest of
//                the annotation is ignored.
//
// Usage: Describes how the function uses the buffer.
//
// <>     : The buffer is not accessed. If used on the return value or with _deref, the
//            function will provide the buffer, and it will be uninitialized at exit.
//            Otherwise, the caller must provide the buffer. This should only be used
//            for alloc and free functions.
// _in    : The function will only read from the buffer. The caller must provide the
//            buffer and initialize it. Cannot be used with _deref.
// _out   : The function will only write to the buffer. If used on the return value or
//            with _deref, the function will provide the buffer and initialize it.
//            Otherwise, the caller must provide the buffer, and the function will
//            initialize it.
// _inout : The function may freely read from and write to the buffer. The caller must
//            provide the buffer and initialize it. If used with _deref, the buffer may
//            be reallocated by the function.
//
// Size: Describes the total size of the buffer. This may be less than the space actually
//         allocated for the buffer, in which case it describes the accessible amount.
//
// <>      : No buffer size is given. If the type specifies the buffer size (such as
//             with LPSTR and LPWSTR), that amount is used. Otherwise, the buffer is one
//             element long. Must be used with _in, _out, or _inout.
// _ecount : The buffer size is an explicit element count.
// _bcount : The buffer size is an explicit byte count.
//
// Output: Describes how much of the buffer will be initialized by the function. For
//           _inout buffers, this also describes how much is initialized at entry. Omit this
//           category for _in buffers; they must be fully initialized by the caller.
//
// <>    : The type specifies how much is initialized. For instance, a function initializing
//           an LPWSTR must NULL-terminate the string.
// _full : The function initializes the entire buffer.
// _part : The function initializes part of the buffer, and explicitly indicates how much.
//
// Optional: Describes if the buffer itself is optional.
//
// <>   : The pointer to the buffer must not be NULL.
// _opt : The pointer to the buffer might be NULL. It will be checked before being dereferenced.
//
// String: Describes if the buffer is NULL terminated
//
// <>      : The buffer is not assumed to be NULL terminated
// _string : The buffer is assumed to be NULL terminated once it has been initialized
//
// Parameters: Gives explicit counts for the size and length of the buffer.
//
// <>            : There is no explicit count. Use when neither _ecount nor _bcount is used.
// (size)        : Only the buffer's total size is given. Use with _ecount or _bcount but not _part.
// (size,length) : The buffer's total size and initialized length are given. Use with _ecount_part
//                   and _bcount_part.
//
// Notes:
//
// 1. Specifying two buffer annotations on a single parameter results in unspecified behavior
//    (e.g. __RPC__in_bcount(5) __RPC__out_bcount(6)
// 
// 2. The size of the buffer and the amount that has been initialized are separate concepts.  
//    Specify the size using _ecount or _bcount.  Specify the amount that is initialized using 
//    _full, _part, or _string.  As a special case, a single element buffer does not need 
//    _ecount, _bcount, _full, or _part
// 
// 3. The count may be less than the total size of the buffer in which case it describes the 
//    accessible portion. 
// 
// 4. "__RPC__opt" and "__RPC_deref" are not valid annotations.
// 
// 5. The placement of _opt when using _deref is important:
//      __RPC__deref_opt_...      : Input may be NULL
//      __RPC__deref_..._opt      : Output may be NULL
//      __RPC__deref_opt_..._opt  : Both input and output may be NULL
//

#pragma once

#include <specstrings.h>

#ifndef __RPCSAL_H_VERSION__
#define __RPCSAL_H_VERSION__        ( 100 )
#endif // __RPCSAL_H_VERSION__

#ifdef __REQUIRED_RPCSAL_H_VERSION__
    #if ( __RPCSAL_H_VERSION__ < __REQUIRED_RPCSAL_H_VERSION__ )
        #error incorrect <rpcsal.h> version. Use the header that matches with the MIDL compiler.
    #endif
#endif


#ifdef  __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

#if (_MSC_VER >= 1000) && !defined(__midl) && defined(_PREFAST_)


// [in]
#define __RPC__in                                   __pre __valid
#define __RPC__in_string                            __RPC__in   __pre __nullterminated
#define __RPC__in_ecount(size)                      __RPC__in __pre __elem_readableTo(size)
#define __RPC__in_ecount_full(size)                 __RPC__in_ecount(size)
#define __RPC__in_ecount_full_string(size)          __RPC__in_ecount_full(size) __pre __nullterminated
#define __RPC__in_ecount_part(size, length)         __RPC__in_ecount(length) __pre __elem_writableTo(size)
#define __RPC__in_ecount_full_opt(size)             __RPC__in_ecount_full(size) __pre __exceptthat  __maybenull
#define __RPC__in_ecount_full_opt_string(size)      __RPC__in_ecount_full_opt(size) __pre __nullterminated
#define __RPC__in_ecount_part_opt(size, length)     __RPC__in_ecount_part(size, length) __pre __exceptthat __maybenull
#define __RPC__in_xcount(size)                      __RPC__in __pre __elem_readableTo(size)
#define __RPC__in_xcount_full(size)                 __RPC__in_ecount(size)
#define __RPC__in_xcount_full_string(size)          __RPC__in_ecount_full(size) __pre __nullterminated
#define __RPC__in_xcount_part(size, length)         __RPC__in_ecount(length) __pre __elem_writableTo(size)
#define __RPC__in_xcount_full_opt(size)             __RPC__in_ecount_full(size) __pre __exceptthat  __maybenull
#define __RPC__in_xcount_full_opt_string(size)      __RPC__in_ecount_full_opt(size) __pre __nullterminated
#define __RPC__in_xcount_part_opt(size, length)     __RPC__in_ecount_part(size, length) __pre __exceptthat __maybenull


#define __RPC__deref_in                             __RPC__in __deref __notnull 
#define __RPC__deref_in_string                      __RPC__in   __pre __deref __nullterminated
#define __RPC__deref_in_opt                         __RPC__deref_in __deref __exceptthat __maybenull
#define __RPC__deref_in_opt_string                  __RPC__deref_in_opt __pre __deref __nullterminated
#define __RPC__deref_opt_in                         __RPC__in __exceptthat __maybenull 
#define __RPC__deref_opt_in_string                  __RPC__deref_opt_in __pre __deref __nullterminated 
#define __RPC__deref_opt_in_opt                     __RPC__deref_opt_in  __pre __deref __exceptthat __maybenull
#define __RPC__deref_opt_in_opt_string              __RPC__deref_opt_in_opt  __pre __deref __nullterminated
#define __RPC__deref_in_ecount(size)                __RPC__in __pre __deref __elem_readableTo(size)
#define __RPC__deref_in_ecount_part(size, length)   __RPC__deref_in_ecount(size)  __pre __deref __elem_readableTo(length)
#define __RPC__deref_in_ecount_full(size)           __RPC__deref_in_ecount_part(size, size)
#define __RPC__deref_in_ecount_full_opt(size)       __RPC__deref_in_ecount_full(size) __pre __deref __exceptthat __maybenull
#define __RPC__deref_in_ecount_full_opt_string(size) __RPC__deref_in_ecount_full_opt(size) __pre __deref __nullterminated
#define __RPC__deref_in_ecount_full_string(size)    __RPC__deref_in_ecount_full(size) __pre __deref __nullterminated
#define __RPC__deref_in_ecount_opt(size)            __RPC__deref_in_ecount(size) __pre __deref __exceptthat __maybenull
#define __RPC__deref_in_ecount_opt_string(size)     __RPC__deref_in_ecount_opt(size) __pre __deref __nullterminated
#define __RPC__deref_in_ecount_part_opt(size, length) __RPC__deref_in_ecount_opt(size) __pre __deref __elem_readableTo(length)
#define __RPC__deref_in_xcount(size)                __RPC__in __pre __deref __elem_readableTo(size)
#define __RPC__deref_in_xcount_part(size, length)   __RPC__deref_in_ecount(size)  __pre __deref __elem_readableTo(length)
#define __RPC__deref_in_xcount_full(size)           __RPC__deref_in_ecount_part(size, size)
#define __RPC__deref_in_xcount_full_opt(size)       __RPC__deref_in_ecount_full(size) __pre __deref __exceptthat __maybenull
#define __RPC__deref_in_xcount_full_opt_string(size) __RPC__deref_in_ecount_full_opt(size) __pre __deref __nullterminated
#define __RPC__deref_in_xcount_full_string(size)    __RPC__deref_in_ecount_full(size) __pre __deref __nullterminated
#define __RPC__deref_in_xcount_opt(size)            __RPC__deref_in_ecount(size) __pre __deref __exceptthat __maybenull
#define __RPC__deref_in_xcount_opt_string(size)     __RPC__deref_in_ecount_opt(size) __pre __deref __nullterminated
#define __RPC__deref_in_xcount_part_opt(size, length) __RPC__deref_in_ecount_opt(size) __pre __deref __elem_readableTo(length)

// [out]
#define __RPC__out                                  __out
#define __RPC__out_ecount(size)                     __out_ecount(size)  __post  __elem_writableTo(size)
#define __RPC__out_ecount_string(size)              __RPC__out_ecount(size) __post __nullterminated
#define __RPC__out_ecount_part(size, length)        __RPC__out_ecount(size)  __post  __elem_readableTo(length)
#define __RPC__out_ecount_full(size)                __RPC__out_ecount_part(size, size)
#define __RPC__out_ecount_full_string(size)         __RPC__out_ecount_full(size) __post  __nullterminated
#define __RPC__out_xcount(size)                     __out
#define __RPC__out_xcount_string(size)              __RPC__out __post __nullterminated
#define __RPC__out_xcount_part(size, length)        __RPC__out
#define __RPC__out_xcount_full(size)                __RPC__out
#define __RPC__out_xcount_full_string(size)         __RPC__out __post __nullterminated

// [in,out] 
#define __RPC__inout                                __inout
#define __RPC__inout_string                         __RPC__inout  __pre __nullterminated __post __nullterminated
#define __RPC__inout_ecount(size)                   __inout_ecount(size)
#define __RPC__inout_ecount_part(size, length)      __inout_ecount_part(size, length)
#define __RPC__inout_ecount_full(size)              __RPC__inout_ecount_part(size, size)
#define __RPC__inout_ecount_full_string(size)       __RPC__inout_ecount_full(size) __pre __nullterminated __post __nullterminated
#define __RPC__inout_xcount(size)                   __inout
#define __RPC__inout_xcount_part(size, length)      __inout
#define __RPC__inout_xcount_full(size)              __RPC__inout
#define __RPC__inout_xcount_full_string(size)       __RPC__inout __pre __nullterminated __post __nullterminated

// [in,unique] 
#define __RPC__in_opt                               __RPC__in __pre __exceptthat __maybenull
#define __RPC__in_opt_string                        __RPC__in_opt   __pre __nullterminated
#define __RPC__in_ecount_opt(size)                  __RPC__in_ecount(size) __pre __exceptthat __maybenull
#define __RPC__in_ecount_opt_string(size)           __RPC__in_ecount_opt(size) __pre __nullterminated
#define __RPC__in_xcount_opt(size)                  __RPC__in_ecount(size) __pre __exceptthat __maybenull
#define __RPC__in_xcount_opt_string(size)           __RPC__in_ecount_opt(size) __pre __nullterminated

// [in,out,unique] 
#define __RPC__inout_opt                            __inout_opt
#define __RPC__inout_opt_string                     __RPC__inout_opt  __pre __nullterminated
#define __RPC__inout_ecount_opt(size)               __inout_ecount_opt(size)
#define __RPC__inout_ecount_part_opt(size, length)  __inout_ecount_part_opt(size, length)
#define __RPC__inout_ecount_full_opt(size)          __RPC__inout_ecount_part_opt(size, size)
#define __RPC__inout_ecount_full_opt_string(size)   __RPC__inout_ecount_full_opt(size)  __pre __nullterminated __post __nullterminated
#define __RPC__inout_xcount_opt(size)               __inout_opt
#define __RPC__inout_xcount_part_opt(size, length)  __inout_opt
#define __RPC__inout_xcount_full_opt(size)          __RPC__inout_opt
#define __RPC__inout_xcount_full_opt_string(size)   __RPC__inout_opt __pre __nullterminated __post __nullterminated

// [out] **
#define __RPC__deref_out                            __deref_out
#define __RPC__deref_out_string                     __RPC__deref_out    __post __deref __nullterminated
// Removed "__post __deref __exceptthat __maybenull" so return values from QueryInterface and the like can be trusted without an explicit NULL check.
// This is a temporary fix until midl.exe can be rev'd to produce more accurate annotations.
#define __RPC__deref_out_opt                        __RPC__deref_out
#define __RPC__deref_out_opt_string                 __RPC__deref_out_opt  __post __deref __nullterminated __pre __deref __null
#define __RPC__deref_out_ecount(size)               __deref_out_ecount(size) __post __deref __elem_writableTo(size)
#define __RPC__deref_out_ecount_part(size, length)  __RPC__deref_out_ecount(size) __post __deref __elem_readableTo(length)
#define __RPC__deref_out_ecount_full(size)          __RPC__deref_out_ecount_part(size,size)
#define __RPC__deref_out_ecount_full_string(size)   __RPC__deref_out_ecount_full(size) __post __deref __nullterminated
#define __RPC__deref_out_xcount(size)               __deref_out __post __deref
#define __RPC__deref_out_xcount_part(size, length)  __RPC__deref_out __post __deref
#define __RPC__deref_out_xcount_full(size)          __RPC__deref_out
#define __RPC__deref_out_xcount_full_string(size)   __RPC__deref_out __post __deref __nullterminated

// [in,out] **, second pointer decoration. 
#define __RPC__deref_inout                          __deref_inout
#define __RPC__deref_inout_string                   __RPC__deref_inout __pre __deref __nullterminated __post __deref __nullterminated
#define __RPC__deref_inout_opt                      __deref_inout_opt
#define __RPC__deref_inout_opt_string               __RPC__deref_inout_opt __deref __nullterminated 
#define __RPC__deref_inout_ecount_opt(size)         __deref_inout_ecount_opt(size)
#define __RPC__deref_inout_ecount_part_opt(size, length) __deref_inout_ecount_part_opt(size , length)
#define __RPC__deref_inout_ecount_full_opt(size)    __RPC__deref_inout_ecount_part_opt(size, size)
#define __RPC__deref_inout_ecount_full(size)        __deref_inout_ecount_full(size)
#define __RPC__deref_inout_ecount_full_string(size) __RPC__deref_inout_ecount_full(size) __post __deref __nullterminated
#define __RPC__deref_inout_ecount_full_opt_string(size) __RPC__deref_inout_ecount_full_opt(size) __pre __deref __nullterminated __post __deref __nullterminated
#define __RPC__deref_inout_xcount_opt(size)         __deref_inout_opt
#define __RPC__deref_inout_xcount_part_opt(size, length) __deref_inout_opt
#define __RPC__deref_inout_xcount_full_opt(size)    __RPC__deref_inout_opt
#define __RPC__deref_inout_xcount_full(size)        __deref_inout
#define __RPC__deref_inout_xcount_full_string(size) __RPC__deref_inout __post __deref __nullterminated
#define __RPC__deref_inout_xcount_full_opt_string(size) __RPC__deref_inout_opt __pre __deref __nullterminated __post __deref __nullterminated


// #define __RPC_out_opt    out_opt is not allowed in rpc

// [in,out,unique] 
#define __RPC__deref_opt_inout                          __deref_opt_inout
#define __RPC__deref_opt_inout_ecount(size)             __deref_opt_inout_ecount(size)
#define __RPC__deref_opt_inout_string                   __RPC__deref_opt_inout __pre __deref __nullterminated __post __deref __nullterminated
#define __RPC__deref_opt_inout_ecount_part(size, length) __deref_opt_inout_ecount_part(size, length)
#define __RPC__deref_opt_inout_ecount_full(size)        __deref_opt_inout_ecount_full(size)
#define __RPC__deref_opt_inout_ecount_full_string(size)  __RPC__deref_opt_inout_ecount_full(size) __pre __deref __nullterminated __post __deref __nullterminated
#define __RPC__deref_opt_inout_xcount_part(size, length) __deref_opt_inout
#define __RPC__deref_opt_inout_xcount_full(size)        __deref_opt_inout
#define __RPC__deref_opt_inout_xcount_full_string(size)  __RPC__deref_opt_inout __pre __deref __nullterminated __post __deref __nullterminated


// We don't need to specify __pre __deref __exceptthat __maybenull : this is default behavior. While this might not hold in SAL 1.1 syntax, SAL team 
// believes it's OK. We can revisit if SAL 1.1 can survive. 
#define __RPC__deref_out_ecount_opt(size)               __RPC__out_ecount(size) __post __deref __exceptthat __maybenull __pre __deref __null 
#define __RPC__deref_out_ecount_part_opt(size, length)  __RPC__deref_out_ecount_part(size, length) __post __deref __exceptthat __maybenull __pre __deref __null
#define __RPC__deref_out_ecount_full_opt(size)          __RPC__deref_out_ecount_part_opt(size, size) __pre __deref __null
#define __RPC__deref_out_ecount_full_opt_string(size)   __RPC__deref_out_ecount_part_opt(size, size) __post __deref __nullterminated __pre __deref __null
#define __RPC__deref_out_xcount_opt(size)               __RPC__out __post __deref __exceptthat __maybenull __pre __deref __null 
#define __RPC__deref_out_xcount_part_opt(size, length)  __RPC__deref_out __post __deref __exceptthat __maybenull __pre __deref __null
#define __RPC__deref_out_xcount_full_opt(size)          __RPC__deref_out_opt __pre __deref __null
#define __RPC__deref_out_xcount_full_opt_string(size)   __RPC__deref_out_opt __post __deref __nullterminated __pre __deref __null

#define __RPC__deref_opt_inout_opt                      __deref_opt_inout_opt
#define __RPC__deref_opt_inout_opt_string               __RPC__deref_opt_inout_opt __pre __deref __nullterminated  __post __deref __nullterminated
#define __RPC__deref_opt_inout_ecount_opt(size)         __deref_opt_inout_ecount_opt(size)  
#define __RPC__deref_opt_inout_ecount_part_opt(size, length) __deref_opt_inout_ecount_part_opt(size, length)
#define __RPC__deref_opt_inout_ecount_full_opt(size)    __RPC__deref_opt_inout_ecount_part_opt(size, size)
#define __RPC__deref_opt_inout_ecount_full_opt_string(size)  __RPC__deref_opt_inout_ecount_full_opt(size) __pre __deref __nullterminated __post __deref __nullterminated
#define __RPC__deref_opt_inout_xcount_opt(size)         __deref_opt_inout_opt  
#define __RPC__deref_opt_inout_xcount_part_opt(size, length) __deref_opt_inout_opt
#define __RPC__deref_opt_inout_xcount_full_opt(size)    __RPC__deref_opt_inout_opt
#define __RPC__deref_opt_inout_xcount_full_opt_string(size)  __RPC__deref_opt_inout_opt __pre __deref __nullterminated __post __deref __nullterminated

#define __RPC_full_pointer                              __maybenull 
#define __RPC_unique_pointer                            __maybenull
#define __RPC_ref_pointer                               __notnull
#define __RPC_string                                    __nullterminated

#define __RPC__range(min,max)                           __range(min,max)
#define __RPC__in_range(min,max)                        __in_range(min,max)

#else   // not prefast

#define __RPC__range(min,max)
#define __RPC__in_range(min,max)

#define __RPC__in           
#define __RPC__in_string
#define __RPC__in_opt_string
#define __RPC__in_ecount(size) 
#define __RPC__in_ecount_full(size)
#define __RPC__in_ecount_full_string(size)
#define __RPC__in_ecount_part(size, length)
#define __RPC__in_ecount_full_opt(size)
#define __RPC__in_ecount_full_opt_string(size)
#define __RPC__inout_ecount_full_opt_string(size)
#define __RPC__in_ecount_part_opt(size, length)
#define __RPC__in_xcount(size) 
#define __RPC__in_xcount_full(size)
#define __RPC__in_xcount_full_string(size)
#define __RPC__in_xcount_part(size, length)
#define __RPC__in_xcount_full_opt(size)
#define __RPC__in_xcount_full_opt_string(size)
#define __RPC__inout_xcount_full_opt_string(size)
#define __RPC__in_xcount_part_opt(size, length)

#define __RPC__deref_in 
#define __RPC__deref_in_string
#define __RPC__deref_in_opt
#define __RPC__deref_in_opt_string
#define __RPC__deref_opt_in
#define __RPC__deref_opt_in_string
#define __RPC__deref_opt_in_opt
#define __RPC__deref_opt_in_opt_string
#define __RPC__deref_in_ecount(size) 
#define __RPC__deref_in_ecount_part(size, length) 
#define __RPC__deref_in_ecount_full(size) 
#define __RPC__deref_in_ecount_full_opt(size)
#define __RPC__deref_in_ecount_full_string(size)
#define __RPC__deref_in_ecount_full_opt_string(size)
#define __RPC__deref_in_ecount_opt(size) 
#define __RPC__deref_in_ecount_opt_string(size)
#define __RPC__deref_in_ecount_part_opt(size, length) 
#define __RPC__deref_in_xcount(size) 
#define __RPC__deref_in_xcount_part(size, length) 
#define __RPC__deref_in_xcount_full(size) 
#define __RPC__deref_in_xcount_full_opt(size)
#define __RPC__deref_in_xcount_full_string(size)
#define __RPC__deref_in_xcount_full_opt_string(size)
#define __RPC__deref_in_xcount_opt(size) 
#define __RPC__deref_in_xcount_opt_string(size)
#define __RPC__deref_in_xcount_part_opt(size, length) 

// [out]
#define __RPC__out     
#define __RPC__out_ecount(size) 
#define __RPC__out_ecount_part(size, length) 
#define __RPC__out_ecount_full(size)
#define __RPC__out_ecount_full_string(size)
#define __RPC__out_xcount(size) 
#define __RPC__out_xcount_part(size, length) 
#define __RPC__out_xcount_full(size)
#define __RPC__out_xcount_full_string(size)

// [in,out] 
#define __RPC__inout                                   
#define __RPC__inout_string
#define __RPC__opt_inout
#define __RPC__inout_ecount(size)                     
#define __RPC__inout_ecount_part(size, length)    
#define __RPC__inout_ecount_full(size)          
#define __RPC__inout_ecount_full_string(size)          
#define __RPC__inout_xcount(size)                     
#define __RPC__inout_xcount_part(size, length)    
#define __RPC__inout_xcount_full(size)          
#define __RPC__inout_xcount_full_string(size)          

// [in,unique] 
#define __RPC__in_opt       
#define __RPC__in_ecount_opt(size)   
#define __RPC__in_xcount_opt(size)   


// [in,out,unique] 
#define __RPC__inout_opt    
#define __RPC__inout_opt_string    
#define __RPC__inout_ecount_opt(size)  
#define __RPC__inout_ecount_part_opt(size, length) 
#define __RPC__inout_ecount_full_opt(size)     
#define __RPC__inout_ecount_full_string(size)
#define __RPC__inout_xcount_opt(size)  
#define __RPC__inout_xcount_part_opt(size, length) 
#define __RPC__inout_xcount_full_opt(size)     
#define __RPC__inout_xcount_full_string(size)

// [out] **
#define __RPC__deref_out   
#define __RPC__deref_out_string
#define __RPC__deref_out_opt 
#define __RPC__deref_out_opt_string
#define __RPC__deref_out_ecount(size) 
#define __RPC__deref_out_ecount_part(size, length) 
#define __RPC__deref_out_ecount_full(size)  
#define __RPC__deref_out_ecount_full_string(size)
#define __RPC__deref_out_xcount(size) 
#define __RPC__deref_out_xcount_part(size, length) 
#define __RPC__deref_out_xcount_full(size)  
#define __RPC__deref_out_xcount_full_string(size)


// [in,out] **, second pointer decoration. 
#define __RPC__deref_inout    
#define __RPC__deref_inout_string
#define __RPC__deref_inout_opt 
#define __RPC__deref_inout_opt_string
#define __RPC__deref_inout_ecount_full(size)
#define __RPC__deref_inout_ecount_full_string(size)
#define __RPC__deref_inout_ecount_opt(size) 
#define __RPC__deref_inout_ecount_part_opt(size, length) 
#define __RPC__deref_inout_ecount_full_opt(size) 
#define __RPC__deref_inout_ecount_full_opt_string(size) 
#define __RPC__deref_inout_xcount_full(size)
#define __RPC__deref_inout_xcount_full_string(size)
#define __RPC__deref_inout_xcount_opt(size) 
#define __RPC__deref_inout_xcount_part_opt(size, length) 
#define __RPC__deref_inout_xcount_full_opt(size) 
#define __RPC__deref_inout_xcount_full_opt_string(size) 

// #define __RPC_out_opt    out_opt is not allowed in rpc

// [in,out,unique] 
#define __RPC__deref_opt_inout  
#define __RPC__deref_opt_inout_string
#define __RPC__deref_opt_inout_ecount(size)     
#define __RPC__deref_opt_inout_ecount_part(size, length) 
#define __RPC__deref_opt_inout_ecount_full(size) 
#define __RPC__deref_opt_inout_ecount_full_string(size)
#define __RPC__deref_opt_inout_xcount(size)     
#define __RPC__deref_opt_inout_xcount_part(size, length) 
#define __RPC__deref_opt_inout_xcount_full(size) 
#define __RPC__deref_opt_inout_xcount_full_string(size)

#define __RPC__deref_out_ecount_opt(size) 
#define __RPC__deref_out_ecount_part_opt(size, length) 
#define __RPC__deref_out_ecount_full_opt(size) 
#define __RPC__deref_out_ecount_full_opt_string(size)
#define __RPC__deref_out_xcount_opt(size) 
#define __RPC__deref_out_xcount_part_opt(size, length) 
#define __RPC__deref_out_xcount_full_opt(size) 
#define __RPC__deref_out_xcount_full_opt_string(size)

#define __RPC__deref_opt_inout_opt      
#define __RPC__deref_opt_inout_opt_string
#define __RPC__deref_opt_inout_ecount_opt(size)   
#define __RPC__deref_opt_inout_ecount_part_opt(size, length) 
#define __RPC__deref_opt_inout_ecount_full_opt(size) 
#define __RPC__deref_opt_inout_ecount_full_opt_string(size) 
#define __RPC__deref_opt_inout_xcount_opt(size)   
#define __RPC__deref_opt_inout_xcount_part_opt(size, length) 
#define __RPC__deref_opt_inout_xcount_full_opt(size) 
#define __RPC__deref_opt_inout_xcount_full_opt_string(size) 

#define __RPC_full_pointer  
#define __RPC_unique_pointer
#define __RPC_ref_pointer
#define __RPC_string                               


#endif

#ifdef  __cplusplus
}
#endif
