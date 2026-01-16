// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    x86log.h

    x86/x64 code logging helpers.

***************************************************************************/

#ifndef MAME_CPU_X86LOG_H
#define MAME_CPU_X86LOG_H

#pragma once

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <vector>


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// use x86code * to reference generated code
typedef uint8_t x86code;


// the code logging context
struct x86log_context
{
private:
	// comment parameters
	static inline constexpr int MAX_COMMENTS = 4000;
	static inline constexpr int MAX_DATA_RANGES = 1000;
	static inline constexpr int COMMENT_POOL_SIZE = MAX_COMMENTS * 40;

	// code logging info
	struct log_comment
	{
		x86code const *base;
		char const *string;
	};

	// data ranges
	struct data_range_t
	{
		x86code const *base;
		x86code const *end;
		int size;
	};

	std::string filename;                       // name of the file
	FILE *file = nullptr;                       // file we are logging to

	std::vector<data_range_t> data_range;       // list of data ranges

	std::vector<log_comment> comment_list;      // list of comments

	char comment_pool[COMMENT_POOL_SIZE];       // string pool to hold comments
	char *comment_pool_next = nullptr;          // pointer to next string pool location

	void reset_log() noexcept;

public:
	using ptr = std::unique_ptr<x86log_context>;

	// create a new context
	static ptr create(std::string_view filename);

	// release a context
	~x86log_context();

	// mark a given range as data for logging purposes
	void mark_as_data(x86code const *base, x86code const *end, int size) noexcept;

	// disassemble a range of code and reset accumulated information
	void disasm_code_range(const char *label, x86code const *start, x86code const *stop);


	/*-------------------------------------------------
	    add_comment - add a comment associated
	    with a given code pointer
	-------------------------------------------------*/

	template <typename... Ts>
	void add_comment(x86code const *base, const char *format, Ts &&... xs) noexcept
	{
		assert((comment_pool_next + strlen(format) + 256) < std::end(comment_pool));

		// we assume comments are registered in order; enforce this
		assert(comment_list.empty() || (base >= comment_list.back().base));

		// if we exceed the maxima, skip it
		if ((comment_pool_next + strlen(format) + 256) >= std::end(comment_pool))
			return;

		// do the printf to the string pool
		std::size_t const available = std::end(comment_pool) - comment_pool_next;
		auto const needed = std::snprintf(comment_pool_next, available, format, std::forward<Ts>(xs)...);
		if ((needed < 0) || ((needed + 1) > available))
			return;

		// fill in the new comment
		try
		{
			comment_list.emplace_back(log_comment{ base, comment_pool_next });
			comment_pool_next += needed + 1;
		}
		catch (std::bad_alloc const &)
		{
		}
	}


	/*-------------------------------------------------
	    printf - manually printf information to
	    the log file
	-------------------------------------------------*/

	template <typename... Ts>
	void printf(const char *format, Ts &&... xs)
	{
		if (!file)
		{
			file = std::fopen(filename.c_str(), "w");
			if (!file)
				return;
		}

		std::fprintf(file, format, std::forward<Ts>(xs)...);
		std::fflush(file);
	}
};

#endif // MAME_CPU_X86LOG_H
