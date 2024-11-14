/*
 * Copyright 2010-2022 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_COMMANDLINE_H_HEADER_GUARD
#define BX_COMMANDLINE_H_HEADER_GUARD

#include "string.h"

namespace bx
{
	///
	StringView tokenizeCommandLine(const StringView& _commandLine, char* _buffer, uint32_t& _bufferSize, int32_t& _argc, char* _argv[], int32_t _maxArgvs, char _term = '\0');

	///
	class CommandLine
	{
	public:
		///
		CommandLine(int32_t _argc, char const* const* _argv);

		///
		const char* findOption(const char* _long, const char* _default) const;

		///
		const char* findOption(const char _short, const char* _long, const char* _default) const;

		///
		const char* findOption(const char* _long, int32_t _numParams = 1) const;

		///
		const char* findOption(const char _short, const char* _long = NULL, int32_t _numParams = 1) const;

		///
		const char* findOption(int32_t _skip, const char _short, const char* _long = NULL, int32_t _numParams = 1) const;

		///
		bool hasArg(const char _short, const char* _long = NULL) const;

		///
		bool hasArg(const char* _long) const;

		///
		bool hasArg(const char*& _value, const char _short, const char* _long = NULL) const;

		///
		bool hasArg(int32_t& _value, const char _short, const char* _long = NULL) const;

		///
		bool hasArg(uint32_t& _value, const char _short, const char* _long = NULL) const;

		///
		bool hasArg(float& _value, const char _short, const char* _long = NULL) const;

		///
		bool hasArg(double& _value, const char _short, const char* _long = NULL) const;

		///
		bool hasArg(bool& _value, const char _short, const char* _long = NULL) const;

		///
		int32_t getNum() const;

		///
		char const* get(int32_t _idx) const;

	private:
		///
		const char* find(int32_t _skip, const char _short, const char* _long, int32_t _numParams) const;

		int32_t m_argc;
		char const* const* m_argv;
	};

} // namespace bx

#endif /// BX_COMMANDLINE_H_HEADER_GUARD
