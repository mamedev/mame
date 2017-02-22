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

	char toUpper(char _ch)
	{
		return _ch - (isLower(_ch) ? 0x20 : 0);
	}

	bool toBool(const char* _str)
	{
		char ch = toLower(_str[0]);
		return ch == 't' ||  ch == '1';
	}

	int32_t strncmp(const char* _lhs, const char* _rhs, size_t _max)
	{
		for (
			; 0 < _max && *_lhs == *_rhs
			; ++_lhs, ++_rhs, --_max
			)
		{
			if (*_lhs == '\0'
			||  *_rhs == '\0')
			{
				break;
			}
		}

		return 0 == _max ? 0 : *_lhs - *_rhs;
	}

	int32_t strincmp(const char* _lhs, const char* _rhs, size_t _max)
	{
		for (
			; 0 < _max && toLower(*_lhs) == toLower(*_rhs)
			; ++_lhs, ++_rhs, --_max
			)
		{
			if (*_lhs == '\0'
			||  *_rhs == '\0')
			{
				break;
			}
		}

		return 0 == _max ? 0 : *_lhs - *_rhs;
	}

	size_t strnlen(const char* _str, size_t _max)
	{
		const char* ptr;
		for (ptr = _str; 0 < _max && *ptr != '\0'; ++ptr, --_max) {};
		return ptr - _str;
	}

	size_t strlncpy(char* _dst, size_t _dstSize, const char* _src, size_t _num)
	{
		BX_CHECK(NULL != _dst, "_dst can't be NULL!");
		BX_CHECK(NULL != _src, "_src can't be NULL!");
		BX_CHECK(0 < _dstSize, "_dstSize can't be 0!");

		const size_t len = strnlen(_src, _num);
		const size_t max = _dstSize-1;
		const size_t num = (len < max ? len : max);
		memcpy(_dst, _src, num);
		_dst[num] = '\0';

		return num;
	}

	size_t strlncat(char* _dst, size_t _dstSize, const char* _src, size_t _num)
	{
		BX_CHECK(NULL != _dst, "_dst can't be NULL!");
		BX_CHECK(NULL != _src, "_src can't be NULL!");
		BX_CHECK(0 < _dstSize, "_dstSize can't be 0!");

		const size_t max = _dstSize;
		const size_t len = strnlen(_dst, max);
		return strlncpy(&_dst[len], max-len, _src, _num);
	}

	const char* strnchr(const char* _str, char _ch, size_t _max)
	{
		for (size_t ii = 0, len = strnlen(_str, _max); ii < len; ++ii)
		{
			if (_str[ii] == _ch)
			{
				return &_str[ii];
			}
		}

		return NULL;
	}

	const char* strnrchr(const char* _str, char _ch, size_t _max)
	{
		for (size_t ii = strnlen(_str, _max); 0 < ii; --ii)
		{
			if (_str[ii] == _ch)
			{
				return &_str[ii];
			}
		}

		return NULL;
	}

	const char* strnstr(const char* _str, const char* _find, size_t _max)
	{
		const char* ptr = _str;

		size_t       stringLen = strnlen(_str, _max);
		const size_t findLen   = strnlen(_find);

		for (; stringLen >= findLen; ++ptr, --stringLen)
		{
			// Find start of the string.
			while (*ptr != *_find)
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
			while (*string++ == *search++)
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

	const char* stristr(const char* _str, const char* _find, size_t _max)
	{
		const char* ptr = _str;

		size_t       stringLen = strnlen(_str, _max);
		const size_t findLen   = strnlen(_find);

		for (; stringLen >= findLen; ++ptr, --stringLen)
		{
			// Find start of the string.
			while (toLower(*ptr) != toLower(*_find) )
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
			while (toLower(*string++) == toLower(*search++) )
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

	void eolLF(char* _out, size_t _size, const char* _str)
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
		size_t len = strnlen(_word);
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

	int32_t write(WriterI* _writer, const char* _format, va_list _argList, Error* _err);

	int32_t vsnprintfRef(char* _out, size_t _max, const char* _format, va_list _argList)
	{
		if (1 < _max)
		{
			StaticMemoryBlockWriter writer(_out, _max-1);
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

	int32_t vsnprintf(char* _out, size_t _max, const char* _format, va_list _argList)
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

	int32_t snprintf(char* _out, size_t _max, const char* _format, ...)
	{
		va_list argList;
		va_start(argList, _format);
		int32_t len = vsnprintf(_out, _max, _format, argList);
		va_end(argList);
		return len;
	}

	int32_t vsnwprintf(wchar_t* _out, size_t _max, const wchar_t* _format, va_list _argList)
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

	int32_t swnprintf(wchar_t* _out, size_t _max, const wchar_t* _format, ...)
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

	void prettify(char* _out, size_t _count, uint64_t _size)
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

	size_t strlcpy(char* _dst, const char* _src, size_t _max)
	{
		return strlncpy(_dst, _max, _src);
	}

	size_t strlcat(char* _dst, const char* _src, size_t _max)
	{
		return strlncat(_dst, _max, _src);
	}

} // namespace bx
