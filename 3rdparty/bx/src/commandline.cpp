/*
 * Copyright 2010-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "bx_p.h"
#include <bx/commandline.h>
#include <bx/string.h>

namespace bx
{
	// Reference(s):
	// - https://web.archive.org/web/20180629044234/https://msdn.microsoft.com/en-us/library/a1y7w461.aspx
	//
	StringView tokenizeCommandLine(const StringView& _commandLine, char* _buffer, uint32_t& _bufferSize, int32_t& _argc, char* _argv[], int32_t _maxArgvs, char _term)
	{
		int32_t argc = 0;
		const char* curr = _commandLine.getPtr();
		const char* end  = _commandLine.getTerm();
		char* currOut = _buffer;
		char term = ' ';
		bool sub = false;

		enum ParserState
		{
			SkipWhitespace,
			SetTerm,
			Copy,
			Escape,
			End,
		};

		ParserState state = SkipWhitespace;

		while (end != curr
		&&     _term != *curr
		&&     argc < _maxArgvs)
		{
			switch (state)
			{
				case SkipWhitespace:
					for (; isSpace(*curr) && *curr!=_term; ++curr) {}; // skip whitespace
					state = SetTerm;
					break;

				case SetTerm:
					if ('"' == *curr)
					{
						term = '"';
						++curr; // skip begining quote
					}
					else
					{
						term = ' ';
					}

					_argv[argc] = currOut;
					++argc;

					state = Copy;
					break;

				case Copy:
					if ('\\' == *curr)
					{
						state = Escape;
					}
					else if ('"' == *curr
					     &&  '"' != term)
					{
						sub = !sub;
					}
					else if (isSpace(*curr) && !sub)
					{
						state = End;
					}
					else if (term != *curr || sub)
					{
						*currOut = *curr;
						++currOut;
					}
					else
					{
						state = End;
					}
					++curr;
					break;

				case Escape:
					{
						const char* start = --curr;
						for (; '\\' == *curr; ++curr) {};

						if ('"' != *curr)
						{
							int32_t count = (int32_t)(curr-start);

							curr = start;
							for (int32_t ii = 0; ii < count; ++ii)
							{
								*currOut = *curr;
								++currOut;
								++curr;
							}
						}
						else
						{
							curr = start+1;
							*currOut = *curr;
							++currOut;
							++curr;
						}
					}
					state = Copy;
					break;

				case End:
					*currOut = '\0';
					++currOut;
					state = SkipWhitespace;
					break;
			}
		}

		*currOut = '\0';
		if (0 < argc
		&&  '\0' == _argv[argc-1][0])
		{
			--argc;
		}

		_bufferSize = (uint32_t)(currOut - _buffer);
		_argc = argc;

		if ('\0' != *curr)
		{
			++curr;
		}

		return StringView(curr, _commandLine.getTerm() );
	}

	CommandLine::CommandLine(int32_t _argc, char const* const* _argv)
		: m_argc(_argc)
		, m_argv(_argv)
	{
	}

	const char* CommandLine::findOption(const char* _long, const char* _default) const
	{
		const char* result = find(0, '\0', _long, 1);
		return result == NULL ? _default : result;
	}

	const char* CommandLine::findOption(const char _short, const char* _long, const char* _default) const
	{
		const char* result = find(0, _short, _long, 1);
		return result == NULL ? _default : result;
	}

	const char* CommandLine::findOption(const char* _long, int32_t _numParams) const
	{
		const char* result = find(0, '\0', _long, _numParams);
		return result;
	}

	const char* CommandLine::findOption(const char _short, const char* _long, int32_t _numParams) const
	{
		const char* result = find(0, _short, _long, _numParams);
		return result;
	}

	const char* CommandLine::findOption(int32_t _skip, const char _short, const char* _long, int32_t _numParams) const
	{
		const char* result = find(_skip, _short, _long, _numParams);
		return result;
	}

	bool CommandLine::hasArg(const char _short, const char* _long) const
	{
		const char* arg = findOption(_short, _long, int32_t(0) );
		return NULL != arg;
	}

	bool CommandLine::hasArg(const char* _long) const
	{
		const char* arg = findOption('\0', _long, int32_t(0) );
		return NULL != arg;
	}

	bool CommandLine::hasArg(const char*& _value, const char _short, const char* _long) const
	{
		const char* arg = findOption(_short, _long, 1);
		_value = arg;
		return NULL != arg;
	}

	bool CommandLine::hasArg(int32_t& _value, const char _short, const char* _long) const
	{
		const char* arg = findOption(_short, _long, 1);
		if (NULL != arg)
		{
			fromString(&_value, arg);
			return true;
		}

		return false;
	}

	bool CommandLine::hasArg(uint32_t& _value, const char _short, const char* _long) const
	{
		const char* arg = findOption(_short, _long, 1);
		if (NULL != arg)
		{
			fromString(&_value, arg);
			return true;
		}

		return false;
	}

	bool CommandLine::hasArg(float& _value, const char _short, const char* _long) const
	{
		const char* arg = findOption(_short, _long, 1);
		if (NULL != arg)
		{
			fromString(&_value, arg);
			return true;
		}

		return false;
	}

	bool CommandLine::hasArg(double& _value, const char _short, const char* _long) const
	{
		const char* arg = findOption(_short, _long, 1);
		if (NULL != arg)
		{
			fromString(&_value, arg);
			return true;
		}

		return false;
	}

	bool CommandLine::hasArg(bool& _value, const char _short, const char* _long) const
	{
		const char* arg = findOption(_short, _long, 1);
		if (NULL != arg)
		{
			if ('0' == *arg || (0 == strCmpI(arg, "false") ) )
			{
				_value = false;
			}
			else if ('0' != *arg || (0 == strCmpI(arg, "true") ) )
			{
				_value = true;
			}

			return true;
		}

		return false;
	}

	const char* CommandLine::find(int32_t _skip, const char _short, const char* _long, int32_t _numParams) const
	{
		for (int32_t ii = 0; ii < m_argc && 0 != strCmp(m_argv[ii], "--"); ++ii)
		{
			const char* arg = m_argv[ii];
			if ('-' == *arg)
			{
				++arg;
				if (_short == *arg)
				{
					if (1 == strLen(arg) )
					{
						if (0 == _skip)
						{
							if (0 == _numParams)
							{
								return "";
							}
							else if (ii+_numParams < m_argc
								 && '-' != *m_argv[ii+1] )
							{
								return m_argv[ii+1];
							}

							return NULL;
						}

						--_skip;
						ii += _numParams;
					}
				}
				else if (NULL != _long
					 &&  '-'  == *arg
					 &&  0 == strCmpI(arg+1, _long) )
				{
					if (0 == _skip)
					{
						if (0 == _numParams)
						{
							return "";
						}
						else if (ii+_numParams < m_argc
							 &&  '-' != *m_argv[ii+1] )
						{
							return m_argv[ii+1];
						}

						return NULL;
					}

					--_skip;
					ii += _numParams;
				}
			}
		}

		return NULL;
	}

	int32_t CommandLine::getNum() const
	{
		return m_argc;
	}

	char const* CommandLine::get(int32_t _idx) const
	{
		return m_argv[_idx];
	}

} // namespace bx
