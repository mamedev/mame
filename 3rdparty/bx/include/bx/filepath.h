/*
 * Copyright 2010-2020 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_FILEPATH_H_HEADER_GUARD
#define BX_FILEPATH_H_HEADER_GUARD

#include "error.h"
#include "string.h"

namespace bx
{
	BX_ERROR_RESULT(kErrorAccess,       BX_MAKEFOURCC('b', 'x', 1, 1) );
	BX_ERROR_RESULT(kErrorNotDirectory, BX_MAKEFOURCC('b', 'x', 1, 2) );

	constexpr int32_t kMaxFilePath = 1024;

	/// Special predefined OS directories.
	///
	struct Dir
	{
		/// Special OS directories:
		enum Enum
		{
			Current, //!< Current directory.
			Temp,    //!< Temporary directory.
			Home,    //!< User's home directory.

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
		/// Default constructor, creates empty file path.
		///
		FilePath();

		/// Construct file path from special OS directory.
		///
		FilePath(Dir::Enum _dir);

		/// Construct file path from C string.
		///
		FilePath(const char* _str);

		/// Construct file path from string.
		///
		FilePath(const StringView& _str);

		/// Assign file path from string.
		///
		FilePath& operator=(const StringView& _rhs);

		/// Clear file path.
		///
		void clear();

		/// Set file path from special OS directory.
		///
		void set(Dir::Enum _dir);

		/// Set file path.
		///
		void set(const StringView& _str);

		/// Join directory to file path.
		///
		void join(const StringView& _str);

		/// Implicitly converts FilePath to StringView.
		///
		operator StringView() const;

		/// Returns zero-terminated C string pointer to file path.
		///
		const char* getCPtr() const;

		/// If path is `/abv/gd/555/333/pod.mac` returns `/abv/gd/555/333/`.
		///
		StringView getPath() const;

		/// If path is `/abv/gd/555/333/pod.mac` returns `pod.mac`.
		///
		StringView getFileName() const;

		/// If path is `/abv/gd/555/333/pod.mac` returns `pod`.
		///
		StringView getBaseName() const;

		/// If path is `/abv/gd/555/333/pod.mac` returns `.mac`.
		///
		StringView getExt() const;

		/// Returns true if file path is absolute.
		///
		bool isAbsolute() const;

		/// Returns true if file path is empty.
		///
		bool isEmpty() const;

	private:
		char m_filePath[kMaxFilePath];
	};

} // namespace bx

#endif // BX_FILEPATH_H_HEADER_GUARD
