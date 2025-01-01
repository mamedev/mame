/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#include <bx/file.h>
#include <bx/os.h>
#include <bx/readerwriter.h>

#if BX_CRT_MSVC
#	include <direct.h>   // _getcwd
#else
#	include <unistd.h>   // getcwd
#endif // BX_CRT_MSVC

#if BX_PLATFORM_WINDOWS
#if !defined(GetModuleFileName)
extern "C" __declspec(dllimport) unsigned long __stdcall GetModuleFileNameA(void* _module, char* _outFilePath, unsigned long _size);
#endif
extern "C" __declspec(dllimport) unsigned long __stdcall GetTempPathA(unsigned long _max, char* _outFilePath);
#elif BX_PLATFORM_OSX
extern "C" int _NSGetExecutablePath(char* _buf, uint32_t* _bufSize);
#endif // BX_PLATFORM_WINDOWS

namespace bx
{
	static bool isPathSeparator(char _ch)
	{
		return false
			|| '/'  == _ch
			|| '\\' == _ch
			;
	}

	static int32_t normalizeFilePath(char* _dst, int32_t _dstSize, const char* _src, int32_t _num)
	{
		// Reference(s):
		// - Lexical File Names in Plan 9 or Getting Dot-Dot Right
		//   https://web.archive.org/web/20180629044444/https://9p.io/sys/doc/lexnames.html

		const int32_t num = strLen(_src, _num);

		if (0 == num)
		{
			return strCopy(_dst, _dstSize, ".");
		}

		int32_t size = 0;

		StaticMemoryBlockWriter writer(_dst, _dstSize);
		Error err;

		int32_t idx      = 0;
		int32_t dotdot   = 0;

		if (2 <= num
		&&  ':' == _src[1])
		{
			size += write(&writer, toUpper(_src[idx]), &err);
			size += write(&writer, ':', &err);
			idx  += 2;
			dotdot = size;
		}

		const int32_t slashIdx = idx;

		bool rooted = isPathSeparator(_src[idx]);
		if (rooted)
		{
			size += write(&writer, '/', &err);
			++idx;
			dotdot = size;
		}

		bool trailingSlash = false;

		while (idx < num && err.isOk() )
		{
			switch (_src[idx])
			{
			case '/':
			case '\\':
				++idx;
				trailingSlash = idx == num;
				break;

			case '.':
				if (idx+1 == num
				||  isPathSeparator(_src[idx+1]) )
				{
					++idx;
					break;
				}

				if ('.' == _src[idx+1]
				&& (idx+2 == num || isPathSeparator(_src[idx+2]) ) )
				{
					idx += 2;

					if (dotdot < size)
					{
						for (--size
							; dotdot < size && !isPathSeparator(_dst[size])
							; --size)
						{
						}
						seek(&writer, size, Whence::Begin);
					}
					else if (!rooted)
					{
						if (0 < size)
						{
							size += write(&writer, '/', &err);
						}

						size += write(&writer, "..", &err);
						dotdot = size;
					}

					break;
				}
				[[fallthrough]];

			default:
				if ( ( rooted && slashIdx+1 != size)
				||   (!rooted &&          0 != size) )
				{
					size += write(&writer, '/', &err);
				}

				for (; idx < num && !isPathSeparator(_src[idx]); ++idx)
				{
					size += write(&writer, _src[idx], &err);
				}

				break;
			}
		}

		if (0 == size)
		{
			size += write(&writer, '.', &err);
		}

		if (trailingSlash)
		{
			size += write(&writer, '/', &err);
		}

		write(&writer, '\0', &err);

		return size;
	}

	static bool getEnv(char* _out, uint32_t* _inOutSize, const StringView& _name, FileType::Enum _type)
	{
		uint32_t len = *_inOutSize;
		*_out = '\0';

		if (getEnv(_out, &len, _name) )
		{
			FileInfo fi;
			if (stat(fi, _out)
			&&  _type == fi.type)
			{
				*_inOutSize = len;
				return true;
			}
		}

		return false;
	}

	static char* pwd(char* _buffer, uint32_t _size)
	{
#if BX_PLATFORM_PS4     \
 || BX_PLATFORM_XBOXONE \
 || BX_PLATFORM_WINRT   \
 || BX_CRT_NONE
		BX_UNUSED(_buffer, _size);
		return NULL;
#elif BX_CRT_MSVC
		return ::_getcwd(_buffer, (int32_t)_size);
#else
		return ::getcwd(_buffer, _size);
#endif // BX_PLATFORM_*
	}

	static bool getCurrentPath(char* _out, uint32_t* _inOutSize)
	{
		uint32_t len = *_inOutSize;
		if (NULL != pwd(_out, len))
		{
			*_inOutSize = strLen(_out);
			return true;
		}

		return false;
	}

	static bool getExecutablePath(char* _out, uint32_t* _inOutSize)
	{
#if BX_PLATFORM_WINDOWS
		uint32_t len = ::GetModuleFileNameA(NULL, _out, *_inOutSize);
		bool result = len != 0 && len < *_inOutSize;
		*_inOutSize = len;
		return result;
#elif BX_PLATFORM_LINUX
		char tmp[64];
		snprintf(tmp, sizeof(tmp), "/proc/%d/exe", getpid() );
		ssize_t result = readlink(tmp, _out, *_inOutSize);

		if (-1 < result)
		{
			*_inOutSize = uint32_t(result);
			return true;
		}

		return false;
#elif BX_PLATFORM_OSX
		uint32_t len = *_inOutSize;
		bool result = _NSGetExecutablePath(_out, &len);
		return 0 == result;
#else
		BX_UNUSED(_out, _inOutSize);
		return false;
#endif // BX_PLATFORM_*
	}

	static bool getHomePath(char* _out, uint32_t* _inOutSize)
	{
		return false
#if BX_PLATFORM_WINDOWS
			|| getEnv(_out, _inOutSize, "USERPROFILE", FileType::Dir)
#endif // BX_PLATFORM_WINDOWS
			|| getEnv(_out, _inOutSize, "HOME", FileType::Dir)
			;
	}

	static bool getTempPath(char* _out, uint32_t* _inOutSize)
	{
#if BX_PLATFORM_WINDOWS
		uint32_t len = ::GetTempPathA(*_inOutSize, _out);
		bool result = len != 0 && len < *_inOutSize;
		*_inOutSize = len;
		return result;
#else
		static const StringView s_tmp[] =
		{
			"TMPDIR",
			"TMP",
			"TEMP",
			"TEMPDIR",

			""
		};

		for (const StringView* tmp = s_tmp; !tmp->isEmpty(); ++tmp)
		{
			uint32_t len = *_inOutSize;
			*_out = '\0';
			bool ok = getEnv(_out, &len, *tmp, FileType::Dir);

			if (ok
			&&  len != 0
			&&  len < *_inOutSize)
			{
				*_inOutSize = len;
				return ok;
			}
		}

		FileInfo fi;
		if (stat(fi, "/tmp")
		&&  FileType::Dir == fi.type)
		{
			strCopy(_out, *_inOutSize, "/tmp");
			*_inOutSize = 4;
			return true;
		}

		return false;
#endif // BX_PLATFORM_*
	}

	FilePath::FilePath()
	{
		set("");
	}

	FilePath::FilePath(Dir::Enum _dir)
	{
		set(_dir);
	}

	FilePath::FilePath(const char* _rhs)
	{
		set(_rhs);
	}

	FilePath::FilePath(const StringView& _filePath)
	{
		set(_filePath);
	}

	FilePath& FilePath::operator=(const StringView& _rhs)
	{
		set(_rhs);
		return *this;
	}

	void FilePath::clear()
	{
		if (!isEmpty() )
		{
			set("");
		}
	}

	void FilePath::set(Dir::Enum _dir)
	{
		bool ok = false;
		char tmp[kMaxFilePath];
		uint32_t len = BX_COUNTOF(tmp);

		switch (_dir)
		{
		case Dir::Current:    ok = getCurrentPath(tmp, &len);    break;
		case Dir::Executable: ok = getExecutablePath(tmp, &len); break;
		case Dir::Home:       ok = getHomePath(tmp, &len);       break;
		case Dir::Temp:       ok = getTempPath(tmp, &len);       break;

		default: break;
		}

		len = ok ? len : 0;

		set(StringView(tmp, len) );
	}

	void FilePath::set(const StringView& _filePath)
	{
		normalizeFilePath(
			  m_filePath
			, BX_COUNTOF(m_filePath)
			, _filePath.getPtr()
			, _filePath.getLength()
			);
	}

	void FilePath::join(const StringView& _str)
	{
		char tmp[kMaxFilePath];
		strCopy(tmp, BX_COUNTOF(tmp), m_filePath);
		strCat(tmp, BX_COUNTOF(tmp), "/");
		strCat(tmp, BX_COUNTOF(tmp), _str);
		set(tmp);
	}

	FilePath::operator StringView() const
	{
		return StringView(m_filePath, strLen(m_filePath) );
	}

	const char* FilePath::getCPtr() const
	{
		return m_filePath;
	}

	StringView FilePath::getPath() const
	{
		StringView end = strRFind(m_filePath, '/');
		if (!end.isEmpty() )
		{
			return StringView(m_filePath, end.getPtr()+1);
		}

		return StringView();
	}

	StringView FilePath::getFileName() const
	{
		StringView fileName = strRFind(m_filePath, '/');
		if (!fileName.isEmpty() )
		{
			return StringView(fileName.getPtr()+1);
		}

		return getCPtr();
	}

	StringView FilePath::getBaseName() const
	{
		const StringView fileName = getFileName();
		if (!fileName.isEmpty() )
		{
			StringView ext = strFind(fileName, '.');
			if (!ext.isEmpty() )
			{
				return StringView(fileName.getPtr(), ext.getPtr() );
			}

			return fileName;
		}

		return StringView();
	}

	StringView FilePath::getExt() const
	{
		const StringView fileName = getFileName();
		if (!fileName.isEmpty() )
		{
			const StringView dot = strFind(fileName, '.');
			return StringView(dot.getPtr(), fileName.getTerm() );
		}

		return StringView();
	}

	bool FilePath::isAbsolute() const
	{
		return  '/' == m_filePath[0] // no drive letter
			|| (':' == m_filePath[1] && '/' == m_filePath[2]) // with drive letter
			;
	}

	bool FilePath::isEmpty() const
	{
		return 0 == strCmp(m_filePath, ".");
	}

} // namespace bx
