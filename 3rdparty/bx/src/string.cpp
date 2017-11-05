/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include <bx/allocator.h>
#include <bx/hash.h>
#include <bx/readerwriter.h>
#include <bx/string.h>

#if !BX_CRT_NONE
#	include <stdio.h> // vsnprintf, vsnwprintf
#endif // !BX_CRT_NONE

namespace bx
{
	bool isSpace(char _ch)
	{
		return ' '  == _ch
			|| '\t' == _ch
			|| '\n' == _ch
			|| '\v' == _ch
			|| '\f' == _ch
			|| '\r' == _ch
			;
	}

	bool isUpper(char _ch)
	{
		return _ch >= 'A' && _ch <= 'Z';
	}

	bool isLower(char _ch)
	{
		return _ch >= 'a' && _ch <= 'z';
	}

	bool isAlpha(char _ch)
	{
		return isLower(_ch) || isUpper(_ch);
	}

	bool isNumeric(char _ch)
	{
		return _ch >= '0' && _ch <= '9';
	}

	bool isAlphaNum(char _ch)
	{
		return isAlpha(_ch) || isNumeric(_ch);
	}

	bool isPrint(char _ch)
	{
		return isAlphaNum(_ch) || isSpace(_ch);
	}

	char toLower(char _ch)
	{
		return _ch + (isUpper(_ch) ? 0x20 : 0);
	}

	void toLowerUnsafe(char* _inOutStr, int32_t _len)
	{
		for (int32_t ii = 0; ii < _len; ++ii)
		{
			*_inOutStr = toLower(*_inOutStr);
		}
	}

	void toLower(char* _inOutStr, int32_t _max)
	{
		const int32_t len = strnlen(_inOutStr, _max);
		toLowerUnsafe(_inOutStr, len);
	}

	char toUpper(char _ch)
	{
		return _ch - (isLower(_ch) ? 0x20 : 0);
	}

	void toUpperUnsafe(char* _inOutStr, int32_t _len)
	{
		for (int32_t ii = 0; ii < _len; ++ii)
		{
			*_inOutStr = toUpper(*_inOutStr);
		}
	}

	void toUpper(char* _inOutStr, int32_t _max)
	{
		const int32_t len = strnlen(_inOutStr, _max);
		toUpperUnsafe(_inOutStr, len);
	}

	bool toBool(const char* _str)
	{
		char ch = toLower(_str[0]);
		return ch == 't' ||  ch == '1';
	}

	typedef char (*CharFn)(char _ch);

	inline char toNoop(char _ch)
	{
		return _ch;
	}

	template<CharFn fn>
	int32_t strCmp(const char* _lhs, const char* _rhs, int32_t _max)
	{
		for (
			; 0 < _max && fn(*_lhs) == fn(*_rhs)
			; ++_lhs, ++_rhs, --_max
			)
		{
			if (*_lhs == '\0'
			||  *_rhs == '\0')
			{
				break;
			}
		}

		return 0 == _max ? 0 : fn(*_lhs) - fn(*_rhs);
	}

	int32_t strncmp(const char* _lhs, const char* _rhs, int32_t _max)
	{
		return strCmp<toNoop>(_lhs, _rhs, _max);
	}

	int32_t strincmp(const char* _lhs, const char* _rhs, int32_t _max)
	{
		return strCmp<toLower>(_lhs, _rhs, _max);
	}

	int32_t strnlen(const char* _str, int32_t _max)
	{
		if (NULL == _str)
		{
			return 0;
		}

		const char* ptr = _str;
		for (; 0 < _max && *ptr != '\0'; ++ptr, --_max) {};
		return int32_t(ptr - _str);
	}

	int32_t strlncpy(char* _dst, int32_t _dstSize, const char* _src, int32_t _num)
	{
		BX_CHECK(NULL != _dst, "_dst can't be NULL!");
		BX_CHECK(NULL != _src, "_src can't be NULL!");
		BX_CHECK(0 < _dstSize, "_dstSize can't be 0!");

		const int32_t len = strnlen(_src, _num);
		const int32_t max = _dstSize-1;
		const int32_t num = (len < max ? len : max);
		memCopy(_dst, _src, num);
		_dst[num] = '\0';

		return num;
	}

	int32_t strlncat(char* _dst, int32_t _dstSize, const char* _src, int32_t _num)
	{
		BX_CHECK(NULL != _dst, "_dst can't be NULL!");
		BX_CHECK(NULL != _src, "_src can't be NULL!");
		BX_CHECK(0 < _dstSize, "_dstSize can't be 0!");

		const int32_t max = _dstSize;
		const int32_t len = strnlen(_dst, max);
		return strlncpy(&_dst[len], max-len, _src, _num);
	}

	const char* strnchr(const char* _str, char _ch, int32_t _max)
	{
		for (int32_t ii = 0, len = strnlen(_str, _max); ii < len; ++ii)
		{
			if (_str[ii] == _ch)
			{
				return &_str[ii];
			}
		}

		return NULL;
	}

	const char* strnrchr(const char* _str, char _ch, int32_t _max)
	{
		for (int32_t ii = strnlen(_str, _max); 0 < ii; --ii)
		{
			if (_str[ii] == _ch)
			{
				return &_str[ii];
			}
		}

		return NULL;
	}

	template<CharFn fn>
	static const char* strStr(const char* _str, int32_t _strMax, const char* _find, int32_t _findMax)
	{
		const char* ptr = _str;

		int32_t       stringLen = strnlen(_str,  _strMax);
		const int32_t findLen   = strnlen(_find, _findMax);

		for (; stringLen >= findLen; ++ptr, --stringLen)
		{
			// Find start of the string.
			while (fn(*ptr) != fn(*_find) )
			{
				++ptr;
				--stringLen;

				// Search pattern lenght can't be longer than the string.
				if (findLen > stringLen)
				{
					return NULL;
				}
			}

			// Set pointers.
			const char* string = ptr;
			const char* search = _find;

			// Start comparing.
			while (fn(*string++) == fn(*search++) )
			{
				// If end of the 'search' string is reached, all characters match.
				if ('\0' == *search)
				{
					return ptr;
				}
			}
		}

		return NULL;
	}

	const char* strnstr(const char* _str, const char* _find, int32_t _max)
	{
		return strStr<toNoop>(_str, _max, _find, INT32_MAX);
	}

	const char* stristr(const char* _str, const char* _find, int32_t _max)
	{
		return strStr<toLower>(_str, _max, _find, INT32_MAX);
	}

	const char* strnl(const char* _str)
	{
		for (; '\0' != *_str; _str += strnlen(_str, 1024) )
		{
			const char* eol = strnstr(_str, "\r\n", 1024);
			if (NULL != eol)
			{
				return eol + 2;
			}

			eol = strnstr(_str, "\n", 1024);
			if (NULL != eol)
			{
				return eol + 1;
			}
		}

		return _str;
	}

	const char* streol(const char* _str)
	{
		for (; '\0' != *_str; _str += strnlen(_str, 1024) )
		{
			const char* eol = strnstr(_str, "\r\n", 1024);
			if (NULL != eol)
			{
				return eol;
			}

			eol = strnstr(_str, "\n", 1024);
			if (NULL != eol)
			{
				return eol;
			}
		}

		return _str;
	}

	const char* strws(const char* _str)
	{
		for (; isSpace(*_str); ++_str) {};
		return _str;
	}

	const char* strnws(const char* _str)
	{
		for (; !isSpace(*_str); ++_str) {};
		return _str;
	}

	const char* strword(const char* _str)
	{
		for (char ch = *_str++; isAlphaNum(ch) || '_' == ch; ch = *_str++) {};
		return _str-1;
	}

	const char* strmb(const char* _str, char _open, char _close)
	{
		int count = 0;
		for (char ch = *_str++; ch != '\0' && count >= 0; ch = *_str++)
		{
			if (ch == _open)
			{
				count++;
			}
			else if (ch == _close)
			{
				count--;
				if (0 == count)
				{
					return _str-1;
				}
			}
		}

		return NULL;
	}

	void eolLF(char* _out, int32_t _size, const char* _str)
	{
		if (0 < _size)
		{
			char* end = _out + _size - 1;
			for (char ch = *_str++; ch != '\0' && _out < end; ch = *_str++)
			{
				if ('\r' != ch)
				{
					*_out++ = ch;
				}
			}

			*_out = '\0';
		}
	}

	const char* findIdentifierMatch(const char* _str, const char* _word)
	{
		int32_t len = strnlen(_word);
		const char* ptr = strnstr(_str, _word);
		for (; NULL != ptr; ptr = strnstr(ptr + len, _word) )
		{
			if (ptr != _str)
			{
				char ch = *(ptr - 1);
				if (isAlphaNum(ch) || '_' == ch)
				{
					continue;
				}
			}

			char ch = ptr[len];
			if (isAlphaNum(ch) || '_' == ch)
			{
				continue;
			}

			return ptr;
		}

		return ptr;
	}

	const char* findIdentifierMatch(const char* _str, const char* _words[])
	{
		for (const char* word = *_words; NULL != word; ++_words, word = *_words)
		{
			const char* match = findIdentifierMatch(_str, word);
			if (NULL != match)
			{
				return match;
			}
		}

		return NULL;
	}

	namespace
	{
		struct Param
		{
			Param()
				: width(0)
				, base(10)
				, prec(6)
				, fill(' ')
				, bits(0)
				, left(false)
				, upper(false)
				, spec(false)
				, sign(false)
			{
			}

			int32_t width;
			uint32_t base;
			uint32_t prec;
			char fill;
			uint8_t bits;
			bool left;
			bool upper;
			bool spec;
			bool sign;
		};

		static int32_t write(WriterI* _writer, const char* _str, int32_t _len, const Param& _param, Error* _err)
		{
			int32_t size = 0;
			int32_t len = (int32_t)strnlen(_str, _len);
			int32_t padding = _param.width > len ? _param.width - len : 0;
			bool sign = _param.sign && len > 1 && _str[0] != '-';
			padding = padding > 0 ? padding - sign : 0;

			if (!_param.left)
			{
				size += writeRep(_writer, _param.fill, padding, _err);
			}

			if (NULL == _str)
			{
				size += write(_writer, "(null)", 6, _err);
			}
			else if (_param.upper)
			{
				for (int32_t ii = 0; ii < len; ++ii)
				{
					size += write(_writer, toUpper(_str[ii]), _err);
				}
			}
			else if (sign)
			{
				size += write(_writer, '+', _err);
				size += write(_writer, _str, len, _err);
			}
			else
			{
				size += write(_writer, _str, len, _err);
			}

			if (_param.left)
			{
				size += writeRep(_writer, _param.fill, padding, _err);
			}

			return size;
		}

		static int32_t write(WriterI* _writer, char _ch, const Param& _param, Error* _err)
		{
			return write(_writer, &_ch, 1, _param, _err);
		}

		static int32_t write(WriterI* _writer, const char* _str, const Param& _param, Error* _err)
		{
			return write(_writer, _str, INT32_MAX, _param, _err);
		}

		static int32_t write(WriterI* _writer, int32_t _i, const Param& _param, Error* _err)
		{
			char str[33];
			int32_t len = toString(str, sizeof(str), _i, _param.base);

			if (len == 0)
			{
				return 0;
			}

			return write(_writer, str, len, _param, _err);
		}

		static int32_t write(WriterI* _writer, int64_t _i, const Param& _param, Error* _err)
		{
			char str[33];
			int32_t len = toString(str, sizeof(str), _i, _param.base);

			if (len == 0)
			{
				return 0;
			}

			return write(_writer, str, len, _param, _err);
		}

		static int32_t write(WriterI* _writer, uint32_t _u, const Param& _param, Error* _err)
		{
			char str[33];
			int32_t len = toString(str, sizeof(str), _u, _param.base);

			if (len == 0)
			{
				return 0;
			}

			return write(_writer, str, len, _param, _err);
		}

		static int32_t write(WriterI* _writer, uint64_t _u, const Param& _param, Error* _err)
		{
			char str[33];
			int32_t len = toString(str, sizeof(str), _u, _param.base);

			if (len == 0)
			{
				return 0;
			}

			return write(_writer, str, len, _param, _err);
		}

		static int32_t write(WriterI* _writer, double _d, const Param& _param, Error* _err)
		{
			char str[1024];
			int32_t len = toString(str, sizeof(str), _d);

			if (len == 0)
			{
				return 0;
			}

			if (_param.upper)
			{
				toUpperUnsafe(str, len);
			}

			const char* dot = strnchr(str, '.');
			if (NULL != dot)
			{
				const int32_t precLen = int32_t(
						dot
						+ uint32_min(_param.prec + _param.spec, 1)
						+ _param.prec
						- str
						);
				if (precLen > len)
				{
					for (int32_t ii = len; ii < precLen; ++ii)
					{
						str[ii] = '0';
					}
					str[precLen] = '\0';
				}
				len = precLen;
			}

			return write(_writer, str, len, _param, _err);
		}

		static int32_t write(WriterI* _writer, const void* _ptr, const Param& _param, Error* _err)
		{
			char str[35] = "0x";
			int32_t len = toString(str + 2, sizeof(str) - 2, uint32_t(uintptr_t(_ptr) ), 16);

			if (len == 0)
			{
				return 0;
			}

			len += 2;
			return write(_writer, str, len, _param, _err);
		}
	} // anonymous namespace

	int32_t write(WriterI* _writer, const char* _format, va_list _argList, Error* _err)
	{
		MemoryReader reader(_format, uint32_t(strnlen(_format) ) );

		int32_t size = 0;

		while (_err->isOk() )
		{
			char ch = '\0';

			Error err;
			read(&reader, ch, &err);

			if (!_err->isOk()
			||  !err.isOk() )
			{
				break;
			}
			else if ('%' == ch)
			{
				// %[flags][width][.precision][length sub-specifier]specifier
				read(&reader, ch);

				Param param;

				// flags
				while (' ' == ch
				||     '-' == ch
				||     '+' == ch
				||     '0' == ch
				||     '#' == ch)
				{
					switch (ch)
					{
						default:
						case ' ': param.fill = ' ';  break;
						case '-': param.left = true; break;
						case '+': param.sign = true; break;
						case '0': param.fill = '0';  break;
						case '#': param.spec = true; break;
					}

					read(&reader, ch);
				}

				if (param.left)
				{
					param.fill = ' ';
				}

				// width
				if ('*' == ch)
				{
					read(&reader, ch);
					param.width = va_arg(_argList, int32_t);

					if (0 > param.width)
					{
						param.left  = true;
						param.width = -param.width;
					}

				}
				else
				{
					while (isNumeric(ch) )
					{
						param.width = param.width * 10 + ch - '0';
						read(&reader, ch);
					}
				}

				// .precision
				if ('.' == ch)
				{
					read(&reader, ch);

					if ('*' == ch)
					{
						read(&reader, ch);
						param.prec = va_arg(_argList, int32_t);
					}
					else
					{
						param.prec = 0;
						while (isNumeric(ch) )
						{
							param.prec = param.prec * 10 + ch - '0';
							read(&reader, ch);
						}
					}
				}

				// length sub-specifier
				while ('h' == ch
				||     'I' == ch
				||     'l' == ch
				||     'j' == ch
				||     't' == ch
				||     'z' == ch)
				{
					switch (ch)
					{
						default: break;

						case 'j': param.bits = sizeof(intmax_t )*8; break;
						case 't': param.bits = sizeof(size_t   )*8; break;
						case 'z': param.bits = sizeof(ptrdiff_t)*8; break;

						case 'h': case 'I': case 'l':
							switch (ch)
							{
								case 'h': param.bits = sizeof(short int)*8; break;
								case 'l': param.bits = sizeof(long int )*8; break;
								default: break;
							}

							read(&reader, ch);
							switch (ch)
							{
								case 'h': param.bits = sizeof(signed char  )*8; break;
								case 'l': param.bits = sizeof(long long int)*8; break;
								case '3':
								case '6':
									read(&reader, ch);
									switch (ch)
									{
										case '2': param.bits = sizeof(int32_t)*8; break;
										case '4': param.bits = sizeof(int64_t)*8; break;
										default: break;
									}
									break;

								default: seek(&reader, -1); break;
							}
							break;
					}

					read(&reader, ch);
				}

				// specifier
				switch (toLower(ch) )
				{
					case 'c':
						size += write(_writer, char(va_arg(_argList, int32_t) ), param, _err);
						break;

					case 's':
						size += write(_writer, va_arg(_argList, const char*), param, _err);
						break;

					case 'o':
						param.base = 8;
						switch (param.bits)
						{
						default: size += write(_writer, va_arg(_argList, int32_t), param, _err); break;
						case 64: size += write(_writer, va_arg(_argList, int64_t), param, _err); break;
						}
						break;

					case 'i':
					case 'd':
						param.base = 10;
						switch (param.bits)
						{
						default: size += write(_writer, va_arg(_argList, int32_t), param, _err); break;
						case 64: size += write(_writer, va_arg(_argList, int64_t), param, _err); break;
						};
						break;

					case 'e':
					case 'f':
					case 'g':
						param.upper = isUpper(ch);
						size += write(_writer, va_arg(_argList, double), param, _err);
						break;

					case 'p':
						size += write(_writer, va_arg(_argList, void*), param, _err);
						break;

					case 'x':
						param.base  = 16;
						param.upper = isUpper(ch);
						switch (param.bits)
						{
						default: size += write(_writer, va_arg(_argList, uint32_t), param, _err); break;
						case 64: size += write(_writer, va_arg(_argList, uint64_t), param, _err); break;
						}
						break;

					case 'u':
						param.base = 10;
						switch (param.bits)
						{
						default: size += write(_writer, va_arg(_argList, uint32_t), param, _err); break;
						case 64: size += write(_writer, va_arg(_argList, uint64_t), param, _err); break;
						}
						break;

					default:
						size += write(_writer, ch, _err);
						break;
				}
			}
			else
			{
				size += write(_writer, ch, _err);
			}
		}

		size += write(_writer, '\0', _err);

		return size;
	}

	int32_t write(WriterI* _writer, Error* _err, const char* _format, ...)
	{
		va_list argList;
		va_start(argList, _format);
		int32_t size = write(_writer, _format, argList, _err);
		va_end(argList);
		return size;
	}

	int32_t vsnprintfRef(char* _out, int32_t _max, const char* _format, va_list _argList)
	{
		if (1 < _max)
		{
			StaticMemoryBlockWriter writer(_out, uint32_t(_max-1) );
			_out[_max-1] = '\0';

			Error err;
			va_list argListCopy;
			va_copy(argListCopy, _argList);
			int32_t size = write(&writer, _format, argListCopy, &err);
			va_end(argListCopy);

			if (err.isOk() )
			{
				return size;
			}
		}

		Error err;
		SizerWriter sizer;
		va_list argListCopy;
		va_copy(argListCopy, _argList);
		int32_t size = write(&sizer, _format, argListCopy, &err);
		va_end(argListCopy);

		return size - 1 /* size without '\0' terminator */;
	}

	int32_t vsnprintf(char* _out, int32_t _max, const char* _format, va_list _argList)
	{
#if BX_CRT_NONE
		return vsnprintfRef(_out, _max, _format, _argList);
#elif BX_CRT_MSVC
		int32_t len = -1;
		if (NULL != _out)
		{
			va_list argListCopy;
			va_copy(argListCopy, _argList);
			len = ::vsnprintf_s(_out, _max, size_t(-1), _format, argListCopy);
			va_end(argListCopy);
		}
		return -1 == len ? ::_vscprintf(_format, _argList) : len;
#else
		return ::vsnprintf(_out, _max, _format, _argList);
#endif // BX_COMPILER_MSVC
	}

	int32_t snprintf(char* _out, int32_t _max, const char* _format, ...)
	{
		va_list argList;
		va_start(argList, _format);
		int32_t len = vsnprintf(_out, _max, _format, argList);
		va_end(argList);
		return len;
	}

	int32_t vsnwprintf(wchar_t* _out, int32_t _max, const wchar_t* _format, va_list _argList)
	{
#if BX_CRT_NONE
		BX_UNUSED(_out, _max, _format, _argList);
		return 0;
#elif BX_CRT_MSVC
		int32_t len = -1;
		if (NULL != _out)
		{
			va_list argListCopy;
			va_copy(argListCopy, _argList);
			len = ::_vsnwprintf_s(_out, _max, size_t(-1), _format, argListCopy);
			va_end(argListCopy);
		}
		return -1 == len ? ::_vscwprintf(_format, _argList) : len;
#elif BX_CRT_MINGW
		return ::vsnwprintf(_out, _max, _format, _argList);
#else
		return ::vswprintf(_out, _max, _format, _argList);
#endif // BX_COMPILER_MSVC
	}

	int32_t swnprintf(wchar_t* _out, int32_t _max, const wchar_t* _format, ...)
	{
		va_list argList;
		va_start(argList, _format);
		int32_t len = vsnwprintf(_out, _max, _format, argList);
		va_end(argList);
		return len;
	}

	const char* baseName(const char* _filePath)
	{
		const char* bs       = strnrchr(_filePath, '\\');
		const char* fs       = strnrchr(_filePath, '/');
		const char* slash    = (bs > fs ? bs : fs);
		const char* colon    = strnrchr(_filePath, ':');
		const char* basename = slash > colon ? slash : colon;
		if (NULL != basename)
		{
			return basename+1;
		}

		return _filePath;
	}

	void prettify(char* _out, int32_t _count, uint64_t _size)
	{
		uint8_t idx = 0;
		double size = double(_size);
		while (_size != (_size&0x7ff)
		&&     idx < 9)
		{
			_size >>= 10;
			size *= 1.0/1024.0;
			++idx;
		}

		snprintf(_out, _count, "%0.2f %c%c", size, "BkMGTPEZY"[idx], idx > 0 ? 'B' : '\0');
	}

	int32_t strlcpy(char* _dst, const char* _src, int32_t _max)
	{
		return strlncpy(_dst, _max, _src);
	}

	int32_t strlcat(char* _dst, const char* _src, int32_t _max)
	{
		return strlncat(_dst, _max, _src);
	}

} // namespace bx
