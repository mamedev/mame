/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include <bx/bx.h>
#include <bx/readerwriter.h>
#include <bx/debug.h>

namespace bx
{
	void* memCopyRef(void* _dst, const void* _src, size_t _numBytes)
	{
		uint8_t* dst = (uint8_t*)_dst;
		const uint8_t* end = dst + _numBytes;
		const uint8_t* src = (const uint8_t*)_src;
		while (dst != end)
		{
			*dst++ = *src++;
		}

		return _dst;
	}

	void* memCopy(void* _dst, const void* _src, size_t _numBytes)
	{
#if BX_CRT_NONE
		return memCopyRef(_dst, _src, _numBytes);
#else
		return ::memcpy(_dst, _src, _numBytes);
#endif // BX_CRT_NONE
	}

	void memCopy(void* _dst, const void* _src, uint32_t _size, uint32_t _num, uint32_t _srcPitch, uint32_t _dstPitch)
	{
		const uint8_t* src = (const uint8_t*)_src;
		uint8_t* dst = (uint8_t*)_dst;

		for (uint32_t ii = 0; ii < _num; ++ii)
		{
			memCopy(dst, src, _size);
			src += _srcPitch;
			dst += _dstPitch;
		}
	}

	///
	void gather(void* _dst, const void* _src, uint32_t _size, uint32_t _num, uint32_t _srcPitch)
	{
		memCopy(_dst, _src, _size, _num, _srcPitch, _size);
	}

	///
	void scatter(void* _dst, const void* _src, uint32_t _size, uint32_t _num, uint32_t _dstPitch)
	{
		memCopy(_dst, _src, _size, _num, _size, _dstPitch);
	}

	void* memMoveRef(void* _dst, const void* _src, size_t _numBytes)
	{
		uint8_t* dst = (uint8_t*)_dst;
		const uint8_t* src = (const uint8_t*)_src;

		if (_numBytes == 0
		||  dst == src)
		{
			return dst;
		}

		//	if (src+_numBytes <= dst || end <= src)
		if (dst < src)
		{
			return memcpy(_dst, _src, _numBytes);
		}

		for (intptr_t ii = _numBytes-1; ii >= 0; --ii)
		{
			dst[ii] = src[ii];
		}

		return _dst;
	}

	void* memMove(void* _dst, const void* _src, size_t _numBytes)
	{
#if BX_CRT_NONE
		return memMoveRef(_dst, _src, _numBytes);
#else
		return ::memmove(_dst, _src, _numBytes);
#endif // BX_CRT_NONE
	}

	void* memSetRef(void* _dst, uint8_t _ch, size_t _numBytes)
	{
		uint8_t* dst = (uint8_t*)_dst;
		const uint8_t* end = dst + _numBytes;
		while (dst != end)
		{
			*dst++ = char(_ch);
		}

		return _dst;
	}

	void* memSet(void* _dst, uint8_t _ch, size_t _numBytes)
	{
#if BX_CRT_NONE
		return memSetRef(_dst, _ch, _numBytes);
#else
		return ::memset(_dst, _ch, _numBytes);
#endif // BX_CRT_NONE
	}

	namespace
	{
		struct Param
		{
			int32_t width;
			uint32_t base;
			uint32_t prec;
			char fill;
			bool left;
		};

		static int32_t write(WriterI* _writer, const char* _str, int32_t _len, const Param& _param, Error* _err)
		{
			int32_t size = 0;
			int32_t len = (int32_t)strnlen(_str, _len);
			int32_t padding = _param.width > len ? _param.width - len : 0;

			if (!_param.left)
			{
				size += writeRep(_writer, _param.fill, padding, _err);
			}

			size += write(_writer, _str, len, _err);

			if (_param.left)
			{
				size += writeRep(_writer, _param.fill, padding, _err);
			}

			return size;
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

		static int32_t write(WriterI* _writer, uint32_t _i, const Param& _param, Error* _err)
		{
			char str[33];
			int32_t len = toString(str, sizeof(str), _i, _param.base);

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

			const char* dot = strnchr(str, '.');
			const int32_t precLen = int32_t(dot + 1 + _param.prec - str);
			if (precLen > len)
			{
				for (int32_t ii = len; ii < precLen; ++ii)
				{
					str[ii] = '0';
				}
				str[precLen] = '\0';
			}
			len = precLen;

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
		MemoryReader reader(_format, strnlen(_format) );

		int32_t size = 0;

		while (_err->isOk() )
		{
			char ch = '\0';
			read(&reader, ch, _err);

			if (!_err->isOk() )
			{
				break;
			}
			else if ('%' == ch)
			{
				// %[ -0][<width>][.<precision>]
				read(&reader, ch);

				Param param;
				param.base  = 10;
				param.prec  = 6;
				param.left  = false;
				param.fill  = ' ';
				param.width = 0;

				while (' ' == ch
				||     '-' == ch
				||     '0' == ch)
				{
					switch (ch)
					{
						case '-': param.left = true; break;
						case ' ': param.fill = ' ';  break;
						case '0': param.fill = '0';  break;
					}

					if (param.left)
					{
						param.fill = ' ';
					}

					read(&reader, ch);
				}

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

				switch (toLower(ch) )
				{
					case 'c':
						size += write(_writer, char(va_arg(_argList, int32_t) ), _err);
						break;

					case 's':
						size += write(_writer, va_arg(_argList, const char*), param, _err);
						break;

					case 'd':
						param.base = 10;
						size += write(_writer, va_arg(_argList, int32_t), param, _err);
						break;

					case 'f':
						size += write(_writer, va_arg(_argList, double), param, _err);
						break;

					case 'p':
						size += write(_writer, va_arg(_argList, void*), param, _err);
						break;

					case 'x':
						param.base = 16;
						size += write(_writer, va_arg(_argList, uint32_t), param, _err);
						break;

					case 'u':
						param.base = 10;
						size += write(_writer, va_arg(_argList, uint32_t), param, _err);
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

} // namespace bx
