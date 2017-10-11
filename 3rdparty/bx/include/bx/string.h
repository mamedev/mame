/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_STRING_H_HEADER_GUARD
#define BX_STRING_H_HEADER_GUARD

#include "allocator.h"

#include <wchar.h> // wchar_t

namespace bx
{
	/// Non-zero-terminated string view.
	class StringView
	{
	public:
		///
		StringView();

		///
		StringView(const StringView& _rhs);

		///
		StringView& operator=(const StringView& _rhs);

		///
		StringView(const char* _ptr, int32_t _len = INT32_MAX);

		///
		void set(const char* _ptr, int32_t _len = INT32_MAX);

		///
		void clear();

		///
		const char* getPtr() const;

		///
		const char* getTerm() const;

		///
		bool isEmpty() const;

		///
		int32_t getLength() const;

	protected:
		const char* m_ptr;
		int32_t     m_len;
	};

	/// ASCII string
	template<bx::AllocatorI** AllocatorT>
	class StringT : public StringView
	{
	public:
		///
		StringT();

		///
		StringT(const StringT<AllocatorT>& _rhs);

		///
		StringT<AllocatorT>& operator=(const StringT<AllocatorT>& _rhs);

		///
		StringT(const char* _ptr, int32_t _len = INT32_MAX);

		///
		StringT(const StringView& _rhs);

		///
		~StringT();

		///
		void set(const char* _ptr, int32_t _len = INT32_MAX);

		///
		void append(const char* _ptr, int32_t _len = INT32_MAX);

		///
		void clear();
	};

	///
	bool isSpace(char _ch);

	///
	bool isUpper(char _ch);

	///
	bool isLower(char _ch);

	///
	bool isAlpha(char _ch);

	///
	bool isNumeric(char _ch);

	///
	bool isAlphaNum(char _ch);

	///
	bool isPrint(char _ch);

	///
	char toLower(char _ch);

	///
	void toLowerUnsafe(char* _inOutStr, int32_t _len);

	///
	void toLower(char* _inOutStr, int32_t _max = INT32_MAX);

	///
	char toUpper(char _ch);

	///
	void toUpperUnsafe(char* _inOutStr, int32_t _len);

	///
	void toUpper(char* _inOutStr, int32_t _max = INT32_MAX);

	///
	bool toBool(const char* _str);

	/// String compare.
	int32_t strncmp(const char* _lhs, const char* _rhs, int32_t _max = INT32_MAX);

	/// Case insensitive string compare.
	int32_t strincmp(const char* _lhs, const char* _rhs, int32_t _max = INT32_MAX);

	///
	int32_t strnlen(const char* _str, int32_t _max = INT32_MAX);

	/// Copy _num characters from string _src to _dst buffer of maximum _dstSize capacity
	/// including zero terminator. Copy will be terminated with '\0'.
	int32_t strlncpy(char* _dst, int32_t _dstSize, const char* _src, int32_t _num = INT32_MAX);

	///
	int32_t strlncat(char* _dst, int32_t _dstSize, const char* _src, int32_t _num = INT32_MAX);

	///
	const char* strnchr(const char* _str, char _ch, int32_t _max = INT32_MAX);

	///
	const char* strnrchr(const char* _str, char _ch, int32_t _max = INT32_MAX);

	/// Find substring in string. Limit search to _size.
	const char* strnstr(const char* _str, const char* _find, int32_t _max = INT32_MAX);

	/// Find substring in string. Case insensitive. Limit search to _max.
	const char* stristr(const char* _str, const char* _find, int32_t _max = INT32_MAX);

	/// Find new line. Returns pointer after new line terminator.
	const char* strnl(const char* _str);

	/// Find end of line. Retuns pointer to new line terminator.
	const char* streol(const char* _str);

	/// Skip whitespace.
	const char* strws(const char* _str);

	/// Skip non-whitespace.
	const char* strnws(const char* _str);

	/// Skip word.
	const char* strword(const char* _str);

	/// Find matching block.
	const char* strmb(const char* _str, char _open, char _close);

	// Normalize string to sane line endings.
	void eolLF(char* _out, int32_t _size, const char* _str);

	// Finds identifier.
	const char* findIdentifierMatch(const char* _str, const char* _word);

	/// Finds any identifier from NULL terminated array of identifiers.
	const char* findIdentifierMatch(const char* _str, const char* _words[]);

	/// Cross platform implementation of vsnprintf that returns number of
	/// characters which would have been written to the final string if
	/// enough space had been available.
	int32_t vsnprintf(char* _out, int32_t _max, const char* _format, va_list _argList);

	/// Cross platform implementation of vsnwprintf that returns number of
	/// characters which would have been written to the final string if
	/// enough space had been available.
	int32_t vsnwprintf(wchar_t* _out, int32_t _max, const wchar_t* _format, va_list _argList);

	///
	int32_t snprintf(char* _out, int32_t _max, const char* _format, ...);

	///
	int32_t swnprintf(wchar_t* _out, int32_t _max, const wchar_t* _format, ...);

	///
	template <typename Ty>
	void stringPrintfVargs(Ty& _out, const char* _format, va_list _argList);

	///
	template <typename Ty>
	void stringPrintf(Ty& _out, const char* _format, ...);

	/// Replace all instances of substring.
	template <typename Ty>
	Ty replaceAll(const Ty& _str, const char* _from, const char* _to);

	/// Extract base file name from file path.
	const char* baseName(const char* _filePath);

	/// Convert size in bytes to human readable string.
	void prettify(char* _out, int32_t _count, uint64_t _size);

	/// Copy src to string dst of size siz.  At most siz-1 characters
	/// will be copied.  Always NUL terminates (unless siz == 0).
	/// Returns strlen(src); if retval >= siz, truncation occurred.
	int32_t strlcpy(char* _dst, const char* _src, int32_t _max);

	/// Appends src to string dst of size siz (unlike strncat, siz is the
	/// full size of dst, not space left).  At most siz-1 characters
	/// will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
	/// Returns strlen(src) + MIN(siz, strlen(initial dst)).
	/// If retval >= siz, truncation occurred.
	int32_t strlcat(char* _dst, const char* _src, int32_t _max);

	///
	int32_t toString(char* _out, int32_t _max, double _value);

	///
	int32_t toString(char* _out, int32_t _max, int32_t _value, uint32_t _base = 10);

	///
	int32_t toString(char* _out, int32_t _max, int64_t _value, uint32_t _base = 10);

	///
	int32_t toString(char* _out, int32_t _max, uint32_t _value, uint32_t _base = 10);

	///
	int32_t toString(char* _out, int32_t _max, uint64_t _value, uint32_t _base = 10);

} // namespace bx

#include "inline/string.inl"

#endif // BX_STRING_H_HEADER_GUARD
