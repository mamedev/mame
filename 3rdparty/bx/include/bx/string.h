/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_STRING_H_HEADER_GUARD
#define BX_STRING_H_HEADER_GUARD

#include "allocator.h"

namespace bx
{
	/// Units
	struct Units
	{
		enum Enum //!< Units:
		{
			Kilo, //!< SI units
			Kibi, //!< IEC prefix
		};
	};

	/// Zero-terminated string literal.
	///
	class StringLiteral
	{
	public:
		/// Construct default/empty string literal.
		///
		constexpr StringLiteral();

		/// Construct string literal from C-style string literal.
		///
		template<int32_t SizeT>
		constexpr StringLiteral(const char (&str)[SizeT]);

		/// Returns string length.
		///
		constexpr int32_t getLength() const;

		/// Returns zero-terminated C string pointer to string literal.
		///
		constexpr const char* getCPtr() const;

	private:
		const char* m_ptr;
		int32_t     m_len;
	};

	/// Non-zero-terminated string view.
	///
	class StringView
	{
	public:
		/// Construct default/empty string view.
		///
		StringView();

		/// Construct string view from string literal.
		///
		constexpr StringView(const StringLiteral& _str);

		///
		StringView(const StringView& _rhs);

		///
		StringView(const StringView& _rhs, int32_t _start, int32_t _len);

		///
		StringView& operator=(const char* _rhs);

		///
		StringView& operator=(const StringView& _rhs);

		///
		StringView(const char* _ptr);

		///
		StringView(const char* _ptr, int32_t _len);

		///
		StringView(const char* _ptr, const char* _term);

		///
		void set(const char* _ptr);

		///
		void set(const char* _ptr, int32_t _len);

		///
		void set(const char* _ptr, const char* _term);

		///
		void set(const StringView& _str);

		///
		void set(const StringView& _str, int32_t _start, int32_t _len);

		///
		void clear();

		/// Returns pointer to non-terminated string.
		///
		/// @attention Use of this pointer in standard C/C++ functions is not safe. You must use it
		///   in conjunction with `getTerm()` or getLength()`.
		///
		const char* getPtr() const;

		/// Returns pointer past last character in string view.
		///
		/// @attention Dereferencing this pointer is not safe.
		///
		const char* getTerm() const;

		/// Returns `true` if string is empty.
		///
		bool isEmpty() const;

		/// Returns string length.
		///
		int32_t getLength() const;

		/// Returns `true` if string is zero terminated.
		///
		bool is0Terminated() const;

	protected:
		const char* m_ptr;
		int32_t     m_len;
		bool        m_0terminated;
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
		StringT(const StringView& _rhs);

		///
		~StringT();

		///
		StringT<AllocatorT>& operator=(const StringT<AllocatorT>& _rhs);

		///
		void set(const StringView& _str);

		///
		void append(const StringView& _str);

		///
		void append(const char* _ptr, const char* _term);

		///
		void clear();

		/// Returns zero-terminated C string pointer.
		///
		const char* getCPtr() const;

	protected:
		int32_t m_capacity;
	};

	/// Returns true if character is part of white space set.
	///
	/// White space set is:
	///   ' '  - Space.
	///   '\t' - Horizontal tab.
	///   '\n' - Line feed / new line.
	///   '\r' - Carriage return.
	///   '\v' - Vertical tab.
	///   '\f' - Form feed / new page.
	///
	bool isSpace(char _ch);

	/// Returns true if string view contains only space characters.
	bool isSpace(const StringView& _str);

	/// Returns true if character is uppercase.
	bool isUpper(char _ch);

	/// Returns true if string view contains only uppercase characters.
	bool isUpper(const StringView& _str);

	/// Returns true if character is lowercase.
	bool isLower(char _ch);

	/// Returns true if string view contains only lowercase characters.
	bool isLower(const StringView& _str);

	/// Returns true if character is part of alphabet set.
	bool isAlpha(char _ch);

	/// Returns true if string view contains only alphabet characters.
	bool isAlpha(const StringView& _str);

	/// Returns true if character is part of numeric set.
	bool isNumeric(char _ch);

	/// Returns true if string view contains only numeric characters.
	bool isNumeric(const StringView& _str);

	/// Returns true if character is part of alpha numeric set.
	bool isAlphaNum(char _ch);

	/// Returns true if string view contains only alphanumeric characters.
	bool isAlphaNum(const StringView& _str);

	/// Returns true if character is part of hexadecimal set.
	bool isHexNum(char _ch);

	/// Returns true if string view contains only hexadecimal characters.
	bool isHexNum(const StringView& _str);

	/// Returns true if character is printable.
	bool isPrint(char _ch);

	/// Returns true if string vieww contains only printable characters.
	bool isPrint(const StringView& _str);

	/// Returns lower case character representing _ch.
	char toLower(char _ch);

	/// Lower case string in place assuming length passed is valid.
	void toLowerUnsafe(char* _inOutStr, int32_t _len);

	/// Lower case string in place.
	void toLower(char* _inOutStr, int32_t _max = INT32_MAX);

	/// Returns upper case character representing _ch.
	char toUpper(char _ch);

	/// Upper case string in place assuming length passed is valid.
	void toUpperUnsafe(char* _inOutStr, int32_t _len);

	/// Uppre case string in place.
	void toUpper(char* _inOutStr, int32_t _max = INT32_MAX);

	/// String compare.
	int32_t strCmp(const StringView& _lhs, const StringView& _rhs, int32_t _max = INT32_MAX);

	/// Case insensitive string compare.
	int32_t strCmpI(const StringView& _lhs, const StringView& _rhs, int32_t _max = INT32_MAX);

	// Compare as strings holding indices/version numbers.
	int32_t strCmpV(const StringView& _lhs, const StringView& _rhs, int32_t _max = INT32_MAX);

	/// Get string length.
	int32_t strLen(const char* _str, int32_t _max = INT32_MAX);

	/// Get string length.
	int32_t strLen(const StringView& _str, int32_t _max = INT32_MAX);

	/// Copy _num characters from string _src to _dst buffer of maximum _dstSize capacity
	/// including zero terminator. Copy will be terminated with '\0'.
	int32_t strCopy(char* _dst, int32_t _dstSize, const StringView& _str, int32_t _num = INT32_MAX);

	/// Concatenate string.
	int32_t strCat(char* _dst, int32_t _dstSize, const StringView& _str, int32_t _num = INT32_MAX);

	/// Test whether the string _str begins with prefix.
	bool hasPrefix(const StringView& _str, const StringView& _prefix);

	/// Test whether the string _str ends with suffix.
	bool hasSuffix(const StringView& _str, const StringView& _suffix);

	/// Find character in string. Limit search to _max characters.
	StringView strFind(const StringView& _str, char _ch);

	/// Find character in string in reverse. Limit search to _max characters.
	StringView strRFind(const StringView& _str, char _ch);

	/// Find substring in string. Limit search to _max characters.
	StringView strFind(const StringView& _str, const StringView& _find, int32_t _num = INT32_MAX);

	/// Find substring in string. Case insensitive. Limit search to _max characters.
	StringView strFindI(const StringView& _str, const StringView& _find, int32_t _num = INT32_MAX);

	/// Returns string view with characters _chars trimmed from left.
	StringView strLTrim(const StringView& _str, const StringView& _chars);

	/// Returns string view with whitespace characters trimmed from left.
	StringView strLTrimSpace(const StringView& _str);

	/// Returns string view with non-whitespace characters trimmed from left.
	StringView strLTrimNonSpace(const StringView& _str);

	/// Returns string view with characters _chars trimmed from right.
	StringView strRTrim(const StringView& _str, const StringView& _chars);

	/// Returns string view with whitespace characters trimmed from right.
	StringView strRTrimSpace(const StringView& _str);

	/// Returns string view with characters _chars trimmed from left and right.
	StringView strTrim(const StringView& _str, const StringView& _chars);

	/// Returns string view with whitespace characters trimmed from left and right.
	StringView strTrimSpace(const StringView& _str);

	/// Returns string view with prefix trimmed.
	StringView strTrimPrefix(const StringView& _str, const StringView& _prefix);

	/// Returns string view with suffix trimmed.
	StringView strTrimSuffix(const StringView& _str, const StringView& _suffix);

	/// Find new line. Returns pointer after new line terminator.
	StringView strFindNl(const StringView& _str);

	/// Find end of line. Returns pointer to new line terminator.
	StringView strFindEol(const StringView& _str);

	/// Returns StringView of word or empty.
	StringView strWord(const StringView& _str);

	/// Returns substring in string.
	StringView strSubstr(const StringView& _str, int32_t _start, int32_t _len = INT32_MAX);

	/// Find matching block.
	StringView strFindBlock(const StringView& _str, char _open, char _close);

	// Normalize string to sane line endings.
	StringView normalizeEolLf(char* _out, int32_t _size, const StringView& _str);

	// Finds identifier.
	StringView findIdentifierMatch(const StringView& _str, const StringView& _word);

	/// Finds any identifier from NULL terminated array of identifiers.
	StringView findIdentifierMatch(const StringView& _str, const char** _words, int32_t _num = INT32_MAX);

	/// Cross platform implementation of vsnprintf that returns number of
	/// characters which would have been written to the final string if
	/// enough space had been available.
	int32_t vsnprintf(char* _out, int32_t _max, const char* _format, va_list _argList);

	/// Cross platform implementation of snprintf that returns number of
	/// characters which would have been written to the final string if
	/// enough space had been available.
	int32_t snprintf(char* _out, int32_t _max, const char* _format, ...);

	///
	int32_t vprintf(const char* _format, va_list _argList);

	///
	int32_t printf(const char* _format, ...);

	/// Templatized snprintf.
	template <typename Ty>
	void stringPrintfVargs(Ty& _out, const char* _format, va_list _argList);

	/// Templatized snprintf.
	template <typename Ty>
	void stringPrintf(Ty& _out, const char* _format, ...);

	/// Convert size in bytes to human readable string kibi units.
	int32_t prettify(char* _out, int32_t _count, uint64_t _value, Units::Enum _units = Units::Kibi);

	/// Converts bool value to string.
	int32_t toString(char* _out, int32_t _max, bool _value);

	/// Converts double value to string.
	int32_t toString(char* _out, int32_t _max, double _value);

	/// Converts 32-bit integer value to string.
	int32_t toString(char* _out, int32_t _max, int32_t _value, uint32_t _base = 10, char _separator = '\0');

	/// Converts 64-bit integer value to string.
	int32_t toString(char* _out, int32_t _max, int64_t _value, uint32_t _base = 10, char _separator = '\0');

	/// Converts 32-bit unsigned integer value to string.
	int32_t toString(char* _out, int32_t _max, uint32_t _value, uint32_t _base = 10, char _separator = '\0');

	/// Converts 64-bit unsigned integer value to string.
	int32_t toString(char* _out, int32_t _max, uint64_t _value, uint32_t _base = 10, char _separator = '\0');

	/// Converts string to bool value.
	bool fromString(bool* _out, const StringView& _str);

	/// Converts string to float value.
	bool fromString(float* _out, const StringView& _str);

	/// Converts string to double value.
	bool fromString(double* _out, const StringView& _str);

	/// Converts string to 32-bit integer value.
	bool fromString(int32_t* _out, const StringView& _str);

	/// Converts string to 32-bit unsigned integer value.
	bool fromString(uint32_t* _out, const StringView& _str);

	///
	class LineReader
	{
	public:
		///
		LineReader(const StringView& _str);

		///
		void reset();

		///
		StringView next();

		///
		bool isDone() const;

		///
		uint32_t getLine() const;

	private:
		const StringView m_str;
		StringView m_curr;
		uint32_t m_line;
	};

} // namespace bx

#include "inline/string.inl"

#endif // BX_STRING_H_HEADER_GUARD
