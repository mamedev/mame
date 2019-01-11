/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_FILEPATH_H_HEADER_GUARD
#define BX_FILEPATH_H_HEADER_GUARD

#include "error.h"
#include "string.h"

BX_ERROR_RESULT(BX_ERROR_ACCESS,        BX_MAKEFOURCC('b', 'x', 0, 0) );
BX_ERROR_RESULT(BX_ERROR_NOT_DIRECTORY, BX_MAKEFOURCC('b', 'x', 0, 1) );

namespace bx
{
	const int32_t kMaxFilePath = 1024;

	///
	struct Dir
	{
		enum Enum ///
		{
			Current,
			Temp,
			Home,

			Count
		};
	};

	/// FilePath parser and helper.
	///
	/// /abv/gd/555/333/pod.mac
	/// ppppppppppppppppbbbeeee
	/// ^               ^  ^
	/// +-path     base-+  +-ext
	///                 ^^^^^^^
	///                 +-filename
	///
	class FilePath
	{
	public:
		///
		FilePath();

		///
		FilePath(Dir::Enum _dir);

		///
		FilePath(const char* _str);

		///
		FilePath(const StringView& _str);

		///
		FilePath& operator=(const StringView& _rhs);

		///
		void set(Dir::Enum _dir);

		///
		void set(const StringView& _str);

		///
		void join(const StringView& _str);

		///
		const char* get() const;

		/// If path is `/abv/gd/555/333/pod.mac` returns `/abv/gd/555/333/`.
		///
		const StringView getPath() const;

		/// If path is `/abv/gd/555/333/pod.mac` returns `pod.mac`.
		///
		const StringView getFileName() const;

		/// If path is `/abv/gd/555/333/pod.mac` returns `pod`.
		///
		const StringView getBaseName() const;

		/// If path is `/abv/gd/555/333/pod.mac` returns `.mac`.
		///
		const StringView getExt() const;

		///
		bool isAbsolute() const;

	private:
		char m_filePath[kMaxFilePath];
	};

	/// Creates a directory named `_filePath`.
	bool make(const FilePath& _filePath, Error* _err = NULL);

	/// Creates a directory named `_filePath` along with all necessary parents.
	bool makeAll(const FilePath& _filePath, Error* _err = NULL);

	/// Removes file or directory.
	bool remove(const FilePath& _filePath, Error* _err = NULL);

	/// Removes file or directory recursivelly.
	bool removeAll(const FilePath& _filePath, Error* _err = NULL);

} // namespace bx

#endif // BX_FILEPATH_H_HEADER_GUARD
