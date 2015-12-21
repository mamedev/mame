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
void x86log_free_context(x86log_context *log) noexcept;

/* add a comment associated with a given code pointer */
template<typename... Ts>
inline void x86log_add_comment(x86log_context *log, x86code *base, const char *format, Ts&&... xs);

/* mark a given range as data for logging purposes */
void x86log_mark_as_data(x86log_context *log, x86code *base, x86code *end, int size) noexcept;

/* disassemble a range of code and reset accumulated information */
void x86log_disasm_code_range(x86log_context *log, const char *label, x86code *start, x86code *stop);

/* manually printf information to the log file */
template<typename... Ts>
inline void x86log_printf(x86log_context *log, const char *format, Ts&&... xs);


/*-------------------------------------------------
    x86log_add_comment - add a comment associated
    with a given code pointer
-------------------------------------------------*/

template<typename... Ts>
inline void x86log_add_comment(x86log_context *log, x86code *base, const char *format, Ts&&... xs)
{
	char *string = log->comment_pool_next;
	log_comment *comment;

	assert(log->comment_count < MAX_COMMENTS);
	assert(log->comment_pool_next + strlen(format) + 256 < log->comment_pool + COMMENT_POOL_SIZE);

	/* we assume comments are registered in order; enforce this */
	assert(log->comment_count == 0 || base >= log->comment_list[log->comment_count - 1].base);

	/* if we exceed the maxima, skip it */
	if (log->comment_count >= MAX_COMMENTS)
		return;
	if (log->comment_pool_next + strlen(format) + 256 >= log->comment_pool + COMMENT_POOL_SIZE)
		return;

	/* do the printf to the string pool */
	log->comment_pool_next += sprintf(log->comment_pool_next, format, std::forward<Ts>(xs)...) + 1;

	/* fill in the new comment */
	comment = &log->comment_list[log->comment_count++];
	comment->base = base;
	comment->string = string;
}


/*-------------------------------------------------
    x86log_printf - manually printf information to
    the log file
-------------------------------------------------*/

template<typename... Ts>
inline void x86log_printf(x86log_context *log, const char *format, Ts&&... xs)
{
	/* open the file, creating it if necessary */
	if (log->file == nullptr)
	{
		log->file = fopen(log->filename.c_str(), "w");
		
		if (log->file == nullptr)
			return;
	}

	assert(log->file != nullptr);

	/* do the printf */
	fprintf(log->file, format, std::forward<Ts>(xs)...);

	/* flush the file */
	fflush(log->file);
}

#endif  /* __X86LOG_H__ */
