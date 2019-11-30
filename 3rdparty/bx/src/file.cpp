/*
 * Copyright 2010-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "bx_p.h"
#include <bx/file.h>

#ifndef BX_CONFIG_CRT_FILE_READER_WRITER
#	define BX_CONFIG_CRT_FILE_READER_WRITER !BX_CRT_NONE
#endif // BX_CONFIG_CRT_FILE_READER_WRITER

#ifndef BX_CONFIG_CRT_DIRECTORY_READER
#	define BX_CONFIG_CRT_DIRECTORY_READER (BX_PLATFORM_OS_DESKTOP && !BX_CRT_NONE)
#endif // BX_CONFIG_CRT_DIRECTORY_READER

#if BX_CRT_NONE
#	include "crt0.h"
#else
#	if BX_CONFIG_CRT_DIRECTORY_READER
#		include <dirent.h>
#	endif // BX_CONFIG_CRT_DIRECTORY_READER
#	include <stdio.h>      // remove
#	include <sys/stat.h>   // stat, mkdir
#	if BX_CRT_MSVC
#		include <direct.h> // _getcwd
#	else
#		include <unistd.h> // getcwd
#	endif // BX_CRT_MSVC
#endif // !BX_CRT_NONE

namespace bx
{
	class NoopWriterImpl : public FileWriterI
	{
	public:
		NoopWriterImpl(void*)
		{
		}

		virtual ~NoopWriterImpl()
		{
			close();
		}

		virtual bool open(const FilePath& _filePath, bool _append, Error* _err) override
		{
			BX_UNUSED(_filePath, _append, _err);
			return false;
		}

		virtual void close() override
		{
		}

		virtual int64_t seek(int64_t _offset, Whence::Enum _whence) override
		{
			BX_UNUSED(_offset, _whence);
			return 0;
		}

		virtual int32_t write(const void* _data, int32_t _size, Error* _err) override
		{
			BX_UNUSED(_data, _err);
			return _size;
		}
	};

#if BX_CONFIG_CRT_FILE_READER_WRITER

#	if BX_CRT_MSVC
#		define fseeko64 _fseeki64
#		define ftello64 _ftelli64
#	elif 0                   \
	  || BX_PLATFORM_ANDROID \
	  || BX_PLATFORM_BSD     \
	  || BX_PLATFORM_HAIKU   \
	  || BX_PLATFORM_IOS     \
	  || BX_PLATFORM_OSX
#		define fseeko64 fseeko
#		define ftello64 ftello
#	elif BX_PLATFORM_PS4
#		define fseeko64 fseek
#		define ftello64 ftell
#	endif // BX_

	class FileReaderImpl : public FileReaderI
	{
	public:
		FileReaderImpl(FILE* _file)
			: m_file(_file)
			, m_open(false)
		{
		}

		virtual ~FileReaderImpl()
		{
			close();
		}

		virtual bool open(const FilePath& _filePath, Error* _err) override
		{
			BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

			if (NULL != m_file)
			{
				BX_ERROR_SET(_err, BX_ERROR_READERWRITER_ALREADY_OPEN, "FileReader: File is already open.");
				return false;
			}

			m_file = fopen(_filePath.getCPtr(), "rb");
			if (NULL == m_file)
			{
				BX_ERROR_SET(_err, BX_ERROR_READERWRITER_OPEN, "FileReader: Failed to open file.");
				return false;
			}

			m_open = true;
			return true;
		}

		virtual void close() override
		{
			if (m_open
			&&  NULL != m_file)
			{
				fclose(m_file);
				m_file = NULL;
			}
		}

		virtual int64_t seek(int64_t _offset, Whence::Enum _whence) override
		{
			BX_CHECK(NULL != m_file, "Reader/Writer file is not open.");
			fseeko64(m_file, _offset, _whence);
			return ftello64(m_file);
		}

		virtual int32_t read(void* _data, int32_t _size, Error* _err) override
		{
			BX_CHECK(NULL != m_file, "Reader/Writer file is not open.");
			BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

			int32_t size = (int32_t)fread(_data, 1, _size, m_file);
			if (size != _size)
			{
				if (0 != feof(m_file) )
				{
					BX_ERROR_SET(_err, BX_ERROR_READERWRITER_EOF, "FileReader: EOF.");
				}
				else if (0 != ferror(m_file) )
				{
					BX_ERROR_SET(_err, BX_ERROR_READERWRITER_READ, "FileReader: read error.");
				}

				return size >= 0 ? size : 0;
			}

			return size;
		}

	private:
		FILE* m_file;
		bool  m_open;
	};

	class FileWriterImpl : public FileWriterI
	{
	public:
		FileWriterImpl(FILE* _file)
			: m_file(_file)
			, m_open(false)
		{
		}

		virtual ~FileWriterImpl()
		{
			close();
		}

		virtual bool open(const FilePath& _filePath, bool _append, Error* _err) override
		{
			BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

			if (NULL != m_file)
			{
				BX_ERROR_SET(_err, BX_ERROR_READERWRITER_ALREADY_OPEN, "FileReader: File is already open.");
				return false;
			}

			m_file = fopen(_filePath.getCPtr(), _append ? "ab" : "wb");

			if (NULL == m_file)
			{
				BX_ERROR_SET(_err, BX_ERROR_READERWRITER_OPEN, "FileWriter: Failed to open file.");
				return false;
			}

			m_open = true;
			return true;
		}

		virtual void close() override
		{
			if (m_open
			&&  NULL != m_file)
			{
				fclose(m_file);
				m_file = NULL;
			}
		}

		virtual int64_t seek(int64_t _offset, Whence::Enum _whence) override
		{
			BX_CHECK(NULL != m_file, "Reader/Writer file is not open.");
			fseeko64(m_file, _offset, _whence);
			return ftello64(m_file);
		}

		virtual int32_t write(const void* _data, int32_t _size, Error* _err) override
		{
			BX_CHECK(NULL != m_file, "Reader/Writer file is not open.");
			BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

			int32_t size = (int32_t)fwrite(_data, 1, _size, m_file);
			if (size != _size)
			{
				BX_ERROR_SET(_err, BX_ERROR_READERWRITER_WRITE, "FileWriter: write failed.");
				return size >= 0 ? size : 0;
			}

			return size;
		}

	private:
		FILE* m_file;
		bool  m_open;
	};

	ReaderI* getStdIn()
	{
		static FileReaderImpl s_stdIn(stdout);
		return &s_stdIn;
	}

	WriterI* getStdOut()
	{
		static FileWriterImpl s_stdOut(stdout);
		return &s_stdOut;
	}

	WriterI* getStdErr()
	{
		static FileWriterImpl s_stdOut(stderr);
		return &s_stdOut;
	}

#elif BX_CRT_NONE
	class FileReaderImpl : public FileReaderI
	{
	public:
		FileReaderImpl(void* _file)
			: m_fd(int32_t(intptr_t(_file) ) )
			, m_open(false)
		{
		}

		virtual ~FileReaderImpl()
		{
			close();
		}

		virtual bool open(const FilePath& _filePath, Error* _err) override
		{
			BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

			if (0 != m_fd)
			{
				BX_ERROR_SET(_err, BX_ERROR_READERWRITER_ALREADY_OPEN, "FileReader: File is already open.");
				return false;
			}

			m_fd = crt0::open(_filePath.get(), crt0::Open::Read, 0);

			if (0 >= m_fd)
			{
				BX_ERROR_SET(_err, BX_ERROR_READERWRITER_OPEN, "FileReader: Failed to open file.");
				return false;
			}

			m_open = true;
			return true;
		}

		virtual void close() override
		{
			if (m_open
			&&  0 != m_fd)
			{
				crt0::close(m_fd);
				m_fd = 0;
			}
		}

		virtual int64_t seek(int64_t _offset, Whence::Enum _whence) override
		{
			BX_CHECK(0 != m_fd, "Reader/Writer file is not open.");
			return crt0::seek(m_fd, _offset, crt0::Whence::Enum(_whence) );
		}

		virtual int32_t read(void* _data, int32_t _size, Error* _err) override
		{
			BX_CHECK(0 != m_fd, "Reader/Writer file is not open.");
			BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

			int32_t size = crt0::read(m_fd, _data, _size);
			if (size != _size)
			{
				BX_UNUSED(_err);
//				if (0 != feof(m_file) )
//				{
//					BX_ERROR_SET(_err, BX_ERROR_READERWRITER_EOF, "FileReader: EOF.");
//				}
//				else if (0 != ferror(m_file) )
//				{
//					BX_ERROR_SET(_err, BX_ERROR_READERWRITER_READ, "FileReader: read error.");
//				}

				return size >= 0 ? size : 0;
			}

			return size;
		}

	private:
		int32_t m_fd;
		bool    m_open;
	};

	class FileWriterImpl : public FileWriterI
	{
	public:
		FileWriterImpl(void* _file)
			: m_fd(int32_t(intptr_t(_file) ) )
			, m_open(false)
		{
		}

		virtual ~FileWriterImpl()
		{
			close();
		}

		virtual bool open(const FilePath& _filePath, bool _append, Error* _err) override
		{
			BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

			if (0 != m_fd)
			{
				BX_ERROR_SET(_err, BX_ERROR_READERWRITER_ALREADY_OPEN, "FileReader: File is already open.");
				return false;
			}

			m_fd = crt0::open(_filePath.get(), _append ? crt0::Open::Append : crt0::Open::Write, 0600);

			if (0 >= m_fd)
			{
				BX_ERROR_SET(_err, BX_ERROR_READERWRITER_OPEN, "FileWriter: Failed to open file.");
				return false;
			}

			m_open = true;
			return true;
		}

		virtual void close() override
		{
			if (m_open
			&&  0 != m_fd)
			{
				crt0::close(m_fd);
				m_fd = 0;
			}
		}

		virtual int64_t seek(int64_t _offset, Whence::Enum _whence) override
		{
			BX_CHECK(0 != m_fd, "Reader/Writer file is not open.");
			return crt0::seek(m_fd, _offset, crt0::Whence::Enum(_whence) );
		}

		virtual int32_t write(const void* _data, int32_t _size, Error* _err) override
		{
			BX_CHECK(0 != m_fd, "Reader/Writer file is not open.");
			BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

			int32_t size = crt0::write(m_fd, _data, _size);
			if (size != _size)
			{
				BX_ERROR_SET(_err, BX_ERROR_READERWRITER_WRITE, "FileWriter: write failed.");
				return size >= 0 ? size : 0;
			}

			return size;
		}

	private:
		int32_t m_fd;
		bool    m_open;
	};

	ReaderI* getStdIn()
	{
		static FileReaderImpl s_stdIn( (void*)intptr_t(crt0::Io::In) );
		return &s_stdIn;
	}

	WriterI* getStdOut()
	{
		static FileWriterImpl s_stdOut( (void*)intptr_t(crt0::Io::Out) );
		return &s_stdOut;
	}

	WriterI* getStdErr()
	{
		static FileWriterImpl s_stdOut( (void*)intptr_t(crt0::Io::Err) );
		return &s_stdOut;
	}

#else

	class FileReaderImpl : public FileReaderI
	{
	public:
		FileReaderImpl(void*)
		{
		}

		virtual ~FileReaderImpl()
		{
			close();
		}

		virtual bool open(const FilePath& _filePath, Error* _err) override
		{
			BX_UNUSED(_filePath, _err);
			return false;
		}

		virtual void close() override
		{
		}

		virtual int64_t seek(int64_t _offset, Whence::Enum _whence) override
		{
			BX_UNUSED(_offset, _whence);
			return 0;
		}

		virtual int32_t read(void* _data, int32_t _size, Error* _err) override
		{
			BX_UNUSED(_data, _size, _err);
			return 0;
		}
	};

	typedef NoopWriterImpl FileWriterImpl;

	ReaderI* getStdIn()
	{
		static FileReaderImpl s_stdIn(NULL);
		return &s_stdIn;
	}

	WriterI* getStdOut()
	{
		static FileWriterImpl s_stdOut(NULL);
		return &s_stdOut;
	}

	WriterI* getStdErr()
	{
		static FileWriterImpl s_stdOut(NULL);
		return &s_stdOut;
	}

#endif // BX_CONFIG_CRT_FILE_READER_WRITER

	WriterI* getNullOut()
	{
		static NoopWriterImpl s_nullOut(NULL);
		return &s_nullOut;
	}

	FileReader::FileReader()
	{
		BX_STATIC_ASSERT(sizeof(FileReaderImpl) <= sizeof(m_internal) );
		BX_PLACEMENT_NEW(m_internal, FileReaderImpl)(NULL);
	}

	FileReader::~FileReader()
	{
		FileReaderImpl* impl = reinterpret_cast<FileReaderImpl*>(m_internal);
		impl->~FileReaderImpl();
	}

	bool FileReader::open(const FilePath& _filePath, Error* _err)
	{
		FileReaderImpl* impl = reinterpret_cast<FileReaderImpl*>(m_internal);
		return impl->open(_filePath, _err);
	}

	void FileReader::close()
	{
		FileReaderImpl* impl = reinterpret_cast<FileReaderImpl*>(m_internal);
		impl->close();
	}

	int64_t FileReader::seek(int64_t _offset, Whence::Enum _whence)
	{
		FileReaderImpl* impl = reinterpret_cast<FileReaderImpl*>(m_internal);
		return impl->seek(_offset, _whence);
	}

	int32_t FileReader::read(void* _data, int32_t _size, Error* _err)
	{
		FileReaderImpl* impl = reinterpret_cast<FileReaderImpl*>(m_internal);
		return impl->read(_data, _size, _err);
	}

	FileWriter::FileWriter()
	{
		BX_STATIC_ASSERT(sizeof(FileWriterImpl) <= sizeof(m_internal) );
		BX_PLACEMENT_NEW(m_internal, FileWriterImpl)(NULL);
	}

	FileWriter::~FileWriter()
	{
		FileWriterImpl* impl = reinterpret_cast<FileWriterImpl*>(m_internal);
		impl->~FileWriterImpl();
	}

	bool FileWriter::open(const FilePath& _filePath, bool _append, Error* _err)
	{
		FileWriterImpl* impl = reinterpret_cast<FileWriterImpl*>(m_internal);
		return impl->open(_filePath, _append, _err);
	}

	void FileWriter::close()
	{
		FileWriterImpl* impl = reinterpret_cast<FileWriterImpl*>(m_internal);
		impl->close();
	}

	int64_t FileWriter::seek(int64_t _offset, Whence::Enum _whence)
	{
		FileWriterImpl* impl = reinterpret_cast<FileWriterImpl*>(m_internal);
		return impl->seek(_offset, _whence);
	}

	int32_t FileWriter::write(const void* _data, int32_t _size, Error* _err)
	{
		FileWriterImpl* impl = reinterpret_cast<FileWriterImpl*>(m_internal);
		return impl->write(_data, _size, _err);
	}

#if BX_CONFIG_CRT_DIRECTORY_READER

	class DirectoryReaderImpl : public ReaderOpenI, public CloserI, public ReaderI
	{
	public:
		DirectoryReaderImpl()
			: m_dir(NULL)
			, m_pos(0)
		{
		}

		virtual ~DirectoryReaderImpl()
		{
			close();
		}

		virtual bool open(const FilePath& _filePath, Error* _err) override
		{
			BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

			m_dir = opendir(_filePath.getCPtr() );

			if (NULL == m_dir)
			{
				BX_ERROR_SET(_err, BX_ERROR_READERWRITER_OPEN, "DirectoryReader: Failed to open directory.");
				return false;
			}

			m_pos = 0;

			return true;
		}

		virtual void close() override
		{
			if (NULL != m_dir)
			{
				closedir(m_dir);
				m_dir = NULL;
			}
		}

		virtual int32_t read(void* _data, int32_t _size, Error* _err) override
		{
			BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");

			int32_t total = 0;

			uint8_t* out = (uint8_t*)_data;

			while (0 < _size)
			{
				if (0 == m_pos)
				{
					if (!fetch(m_cache, m_dir) )
					{
						BX_ERROR_SET(_err, BX_ERROR_READERWRITER_EOF, "DirectoryReader: EOF.");
						return total;
					}
				}

				const uint8_t* src = (const uint8_t*)&m_cache;
				int32_t size = min<int32_t>(_size, sizeof(m_cache)-m_pos);
				memCopy(&out[total], &src[m_pos], size);
				total += size;
				_size -= size;

				m_pos += size;
				m_pos %= sizeof(m_cache);
			}

			return total;
		}

		static bool fetch(FileInfo& _out, DIR* _dir)
		{
			for (;;)
			{
				const dirent* item = readdir(_dir);

				if (NULL == item)
				{
					break;
				}

				if (0 != (item->d_type & DT_DIR) )
				{
					_out.type = FileType::Dir;
					_out.size = UINT64_MAX;
					_out.filePath.set(item->d_name);
					return true;
				}

				if (0 != (item->d_type & DT_REG) )
				{
					_out.type = FileType::File;
					_out.size = UINT64_MAX;
					_out.filePath.set(item->d_name);
					return true;
				}
			}

			return false;
		}

		FileInfo m_cache;
		DIR*     m_dir;
		int32_t  m_pos;
	};

#else

	class DirectoryReaderImpl : public ReaderOpenI, public CloserI, public ReaderI
	{
	public:
		DirectoryReaderImpl()
		{
		}

		virtual ~DirectoryReaderImpl()
		{
		}

		virtual bool open(const FilePath& _filePath, Error* _err) override
		{
			BX_UNUSED(_filePath);
			BX_ERROR_SET(_err, BX_ERROR_READERWRITER_OPEN, "DirectoryReader: Failed to open directory.");
			return false;
		}

		virtual void close() override
		{
		}

		virtual int32_t read(void* _data, int32_t _size, Error* _err) override
		{
			BX_UNUSED(_data, _size);
			BX_CHECK(NULL != _err, "Reader/Writer interface calling functions must handle errors.");
			BX_ERROR_SET(_err, BX_ERROR_READERWRITER_EOF, "DirectoryReader: EOF.");
			return 0;
		}
	};

#endif // BX_CONFIG_CRT_DIRECTORY_READER

	DirectoryReader::DirectoryReader()
	{
		BX_STATIC_ASSERT(sizeof(DirectoryReaderImpl) <= sizeof(m_internal) );
		BX_PLACEMENT_NEW(m_internal, DirectoryReaderImpl);
	}

	DirectoryReader::~DirectoryReader()
	{
		DirectoryReaderImpl* impl = reinterpret_cast<DirectoryReaderImpl*>(m_internal);
		impl->~DirectoryReaderImpl();
	}

	bool DirectoryReader::open(const FilePath& _filePath, Error* _err)
	{
		DirectoryReaderImpl* impl = reinterpret_cast<DirectoryReaderImpl*>(m_internal);
		return impl->open(_filePath, _err);
	}

	void DirectoryReader::close()
	{
		DirectoryReaderImpl* impl = reinterpret_cast<DirectoryReaderImpl*>(m_internal);
		impl->close();
	}

	int32_t DirectoryReader::read(void* _data, int32_t _size, Error* _err)
	{
		DirectoryReaderImpl* impl = reinterpret_cast<DirectoryReaderImpl*>(m_internal);
		return impl->read(_data, _size, _err);
	}

	bool stat(FileInfo& _outFileInfo, const FilePath& _filePath)
	{
#if BX_CRT_NONE
		BX_UNUSED(_filePath, _outFileInfo);
		return false;
#else
		_outFileInfo.size = 0;
		_outFileInfo.type = FileType::Count;

#	if BX_COMPILER_MSVC
		struct ::_stat64 st;
		int32_t result = ::_stat64(_filePath.getCPtr(), &st);

		if (0 != result)
		{
			return false;
		}

		if (0 != (st.st_mode & _S_IFREG) )
		{
			_outFileInfo.type = FileType::File;
		}
		else if (0 != (st.st_mode & _S_IFDIR) )
		{
			_outFileInfo.type = FileType::Dir;
		}
#	else
		struct ::stat st;
		int32_t result = ::stat(_filePath.getCPtr(), &st);
		if (0 != result)
		{
			return false;
		}

		if (0 != (st.st_mode & S_IFREG) )
		{
			_outFileInfo.type = FileType::File;
		}
		else if (0 != (st.st_mode & S_IFDIR) )
		{
			_outFileInfo.type = FileType::Dir;
		}
#	endif // BX_COMPILER_MSVC

		_outFileInfo.size = st.st_size;

		return true;
#endif // BX_CRT_NONE
	}

	bool make(const FilePath& _filePath, Error* _err)
	{
		BX_ERROR_SCOPE(_err);

		if (!_err->isOk() )
		{
			return false;
		}

#if BX_CRT_MSVC
		int32_t result = ::_mkdir(_filePath.getCPtr() );
#elif BX_CRT_MINGW
		int32_t result = ::mkdir(_filePath.getCPtr());
#elif BX_CRT_NONE
		BX_UNUSED(_filePath);
		int32_t result = -1;
#else
		int32_t result = ::mkdir(_filePath.getCPtr(), 0700);
#endif // BX_CRT_MSVC

		if (0 != result)
		{
			BX_ERROR_SET(_err, BX_ERROR_ACCESS, "The parent directory does not allow write permission to the process.");
			return false;
		}

		return true;
	}

	bool makeAll(const FilePath& _filePath, Error* _err)
	{
		BX_ERROR_SCOPE(_err);

		if (!_err->isOk() )
		{
			return false;
		}

		FileInfo fi;

		if (stat(fi, _filePath) )
		{
			if (FileType::Dir == fi.type)
			{
				return true;
			}

			BX_ERROR_SET(_err, BX_ERROR_NOT_DIRECTORY, "File already exist, and is not directory.");
			return false;
		}

		const StringView dir   = strRTrim(_filePath, "/");
		const StringView slash = strRFind(dir, '/');

		if (!slash.isEmpty()
		&&  slash.getPtr() - dir.getPtr() > 1)
		{
			if (!makeAll(StringView(dir.getPtr(), slash.getPtr() ), _err) )
			{
				return false;
			}
		}

		FilePath path(dir);
		return make(path, _err);
	}

	bool remove(const FilePath& _filePath, Error* _err)
	{
		BX_ERROR_SCOPE(_err);

		if (!_err->isOk() )
		{
			return false;
		}

#if BX_CRT_MSVC
		int32_t result = -1;
		FileInfo fi;
		if (stat(fi, _filePath) )
		{
			if (FileType::Dir == fi.type)
			{
				result = ::_rmdir(_filePath.getCPtr() );
			}
			else
			{
				result = ::remove(_filePath.getCPtr() );
			}
		}
#elif BX_CRT_NONE
		BX_UNUSED(_filePath);
		int32_t result = -1;
#else
		int32_t result = ::remove(_filePath.getCPtr() );
#endif // BX_CRT_MSVC

		if (0 != result)
		{
			BX_ERROR_SET(_err, BX_ERROR_ACCESS, "The parent directory does not allow write permission to the process.");
			return false;
		}

		return true;
	}

	bool removeAll(const FilePath& _filePath, Error* _err)
	{
		BX_ERROR_SCOPE(_err);

		if (remove(_filePath, _err) )
		{
			return true;
		}

		_err->reset();

		FileInfo fi;

		if (!stat(fi, _filePath) )
		{
			BX_ERROR_SET(_err, BX_ERROR_ACCESS, "The parent directory does not allow write permission to the process.");
			return false;
		}

		if (FileType::Dir != fi.type)
		{
			BX_ERROR_SET(_err, BX_ERROR_NOT_DIRECTORY, "File already exist, and is not directory.");
			return false;
		}

		Error err;
		DirectoryReader dr;

		if (!bx::open(&dr, _filePath) )
		{
			BX_ERROR_SET(_err, BX_ERROR_NOT_DIRECTORY, "File already exist, and is not directory.");
			return false;
		}

		while (err.isOk() )
		{
			bx::read(&dr, fi, &err);

			if (err.isOk() )
			{
				if (0 == strCmp(fi.filePath, ".")
				||  0 == strCmp(fi.filePath, "..") )
				{
					continue;
				}

				FilePath path(_filePath);
				path.join(fi.filePath);
				if (!removeAll(path, _err) )
				{
					_err->reset();
					break;
				}
			}
		}

		bx::close(&dr);

		return remove(_filePath, _err);
	}

} // namespace bx
