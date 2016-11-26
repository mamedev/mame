/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_COMMANDLINE_H_HEADER_GUARD
#define BX_COMMANDLINE_H_HEADER_GUARD

#include "bx.h"
#include "string.h"

namespace bx
{
	class CommandLine
	{
	public:
		CommandLine(int _argc, char const* const* _argv)
			: m_argc(_argc)
			, m_argv(_argv)
		{
		}

		const char* findOption(const char* _long, const char* _default) const
		{
			const char* result = find(0, '\0', _long, 1);
			return result == NULL ? _default : result;
		}

		const char* findOption(const char _short, const char* _long, const char* _default) const
		{
			const char* result = find(0, _short, _long, 1);
			return result == NULL ? _default : result;
		}

		const char* findOption(const char* _long, int _numParams = 1) const
		{
			const char* result = find(0, '\0', _long, _numParams);
			return result;
		}

		const char* findOption(const char _short, const char* _long = NULL, int _numParams = 1) const
		{
			const char* result = find(0, _short, _long, _numParams);
			return result;
		}

		const char* findOption(int _skip, const char _short, const char* _long = NULL, int _numParams = 1) const
		{
			const char* result = find(_skip, _short, _long, _numParams);
			return result;
		}

		bool hasArg(const char _short, const char* _long = NULL) const
		{
			const char* arg = findOption(_short, _long, 0);
			return NULL != arg;
		}

		bool hasArg(const char* _long) const
		{
			const char* arg = findOption('\0', _long, 0);
			return NULL != arg;
		}

		bool hasArg(const char*& _value, const char _short, const char* _long = NULL) const
		{
			const char* arg = findOption(_short, _long, 1);
			_value = arg;
			return NULL != arg;
		}

		bool hasArg(int& _value, const char _short, const char* _long = NULL) const
		{
			const char* arg = findOption(_short, _long, 1);
			if (NULL != arg)
			{
				_value = atoi(arg);
				return true;
			}

			return false;
		}

		bool hasArg(unsigned int& _value, const char _short, const char* _long = NULL) const
		{
			const char* arg = findOption(_short, _long, 1);
			if (NULL != arg)
			{
				_value = atoi(arg);
				return true;
			}

			return false;
		}

		bool hasArg(float& _value, const char _short, const char* _long = NULL) const
		{
			const char* arg = findOption(_short, _long, 1);
			if (NULL != arg)
			{
				_value = float(atof(arg));
				return true;
			}

			return false;
		}

		bool hasArg(double& _value, const char _short, const char* _long = NULL) const
		{
			const char* arg = findOption(_short, _long, 1);
			if (NULL != arg)
			{
				_value = atof(arg);
				return true;
			}

			return false;
		}

		bool hasArg(bool& _value, const char _short, const char* _long = NULL) const
		{
			const char* arg = findOption(_short, _long, 1);
			if (NULL != arg)
			{
				if ('0' == *arg || (0 == stricmp(arg, "false") ) )
				{
					_value = false;
				}
				else if ('0' != *arg || (0 == stricmp(arg, "true") ) )
				{
					_value = true;
				}

				return true;
			}

			return false;
		}

	private:
		const char* find(int _skip, const char _short, const char* _long, int _numParams) const
		{
			for (int ii = 0; ii < m_argc; ++ii)
			{
				const char* arg = m_argv[ii];
				if ('-' == *arg)
				{
					++arg;
					if (_short == *arg)
					{
						if (1 == strlen(arg) )
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
						 &&  '-' == *arg
						 &&  0 == stricmp(arg+1, _long) )
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

		int m_argc;
		char const* const* m_argv;
	};

} // namespace bx

#endif /// BX_COMMANDLINE_H_HEADER_GUARD
