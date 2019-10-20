/*
 * Copyright 2010-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_FILE_H_HEADER_GUARD
#define BX_FILE_H_HEADER_GUARD

#include "filepath.h"
#include "readerwriter.h"

namespace bx
{
	/// Returns standard input reader.
	ReaderI* getStdIn();

	/// Returns standard output writer.
	WriterI* getStdOut();

	/// Returns standard error writer.
	WriterI* getStdErr();

	/// Returns null output writer.
	WriterI* getNullOut();

	/// File reader.
	class FileReader : public FileReaderI
	{
	public:
		///
		FileReader();

		///
		virtual ~FileReader();

		///
		virtual bool open(const FilePath& _filePath, Error* _err) override;

		///
		virtual void close() override;

		///
		virtual int64_t seek(int64_t _offset = 0, Whence::Enum _whence = Whence::Current) override;

		///
		virtual int32_t read(void* _data, int32_t _size, Error* _err) override;

	private:
		BX_ALIGN_DECL(16, uint8_t) m_internal[64];
	};

	/// File writer.
	class FileWriter : public FileWriterI
	{
	public:
		///
		FileWriter();

		///
		virtual ~FileWriter();

		///
		virtual bool open(const FilePath& _filePath, bool _append, Error* _err) override;

		///
		virtual void close() override;

		///
		virtual int64_t seek(int64_t _offset = 0, Whence::Enum _whence = Whence::Current) override;

		///
		virtual int32_t write(const void* _data, int32_t _size, Error* _err) override;

	private:
		BX_ALIGN_DECL(16, uint8_t) m_internal[64];
	};

	/// File type.
	struct FileType
	{
		/// File types:
		enum Enum
		{
			File, //!< File.
			Dir,  //!< Directory.

			Count
		};
	};

	/// File info.
	struct FileInfo
	{
		FilePath       filePath; //!< File path.
		uint64_t       size;     //!< File size.
		FileType::Enum type;     //!< File type.
	};

	/// Directory reader.
	class DirectoryReader : public ReaderOpenI, public CloserI, public ReaderI
	{
	public:
		///
		DirectoryReader();

		///
		virtual ~DirectoryReader();

		///
		virtual bool open(const FilePath& _filePath, Error* _err) override;

		///
		virtual void close() override;

		///
		virtual int32_t read(void* _data, int32_t _size, Error* _err) override;

	private:
		BX_ALIGN_DECL(16, uint8_t) m_internal[sizeof(FilePath)+sizeof(FileInfo)+16];
	};

	/// FIle stat.
	bool stat(FileInfo& _outFileInfo, const FilePath& _filePath);

	/// Creates a directory named `_filePath`.
	///
	bool make(const FilePath& _filePath, Error* _err = NULL);

	/// Creates a directory named `_filePath` along with all necessary parents.
	///
	bool makeAll(const FilePath& _filePath, Error* _err = NULL);

	/// Removes file or directory.
	///
	bool remove(const FilePath& _filePath, Error* _err = NULL);

	/// Removes file or directory recursivelly.
	///
	bool removeAll(const FilePath& _filePath, Error* _err = NULL);

} // namespace bx

#endif // BX_FILE_H_HEADER_GUARD
