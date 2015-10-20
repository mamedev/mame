// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    x86log.h

    x86/x64 code logging helpers.

***************************************************************************/

#pragma once

#ifndef __X86LOG_H__
#define __X86LOG_H__

#include "x86emit.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct x86log_context;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* create a new context */
x86log_context *x86log_create_context(const char *filename);

/* release a context */
void x86log_free_context(x86log_context *log);

/* add a comment associated with a given code pointer */
void x86log_add_comment(x86log_context *log, x86code *base, const char *format, ...) ATTR_PRINTF(3,4);

/* mark a given range as data for logging purposes */
void x86log_mark_as_data(x86log_context *log, x86code *base, x86code *end, int size);

/* disassemble a range of code and reset accumulated information */
void x86log_disasm_code_range(x86log_context *log, const char *label, x86code *start, x86code *stop);

/* manually printf information to the log file */
void x86log_printf(x86log_context *log, const char *format, ...) ATTR_PRINTF(2,3);


#endif  /* __X86LOG_H__ */
