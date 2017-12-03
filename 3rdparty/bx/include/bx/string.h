/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_STRING_H_HEADER_GUARD
#define BX_STRING_H_HEADER_GUARD

#include "allocator.h"

namespace bx
{
	struct Units
	{
		enum Enum
		{
			Kilo,
			Kibi,
		};
	};

	/// Non-zero-terminated string view.
	class StringView
	{
	public:
		///
		StringView();

		///
		StringView(const StringView& _rhs);

		///
		StringView& operator=(const char* _rhs);

		///
		StringView& operator=(const StringView& _rhs);

		///
		StringView(const char* _ptr, int32_t _len = INT32_MAX);

		///
		StringView(const char* _ptr, const char* _term);

		///
		void set(const char* _ptr, int32_t _len = INT32_MAX);

		///
		void set(const char* _ptr, const char* _term);

		///
		void set(const StringView& _str);

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
		StringT(const StringView& _rhs);

		///
		~StringT();

		///
		void set(const StringView& _str);

		///
		void append(const StringView& _str);

		///
		void clear();
	};

	///
	bool isSpace(char _ch);

	///
	bool isSpace(const StringView& _str);

	///
	bool isUpper(char _ch);

	///
	bool isUpper(const StringView& _str);

	///
	bool isLower(char _ch);

	///
	bool isLower(const StringView& _str);

	///
	bool isAlpha(char _ch);

	///
	bool isAlpha(const StringView& _str);

	///
	bool isNumeric(char _ch);

	///
	bool isNumeric(const StringView& _str);

	///
	bool isAlphaNum(char _ch);

	///
	bool isAlphaNum(const StringView& _str);

	///
	bool isPrint(char _ch);

	///
	bool isPrint(const StringView& _str);

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

	/// Concatinate string.
	int32_t strCat(char* _dst, int32_t _dstSize, const StringView& _str, int32_t _num = INT32_MAX);

	/// Find character in string. Limit search to _max characters.
	const char* strFind(const StringView& _str, char _ch);

	/// Find character in string in reverse. Limit search to _max characters.
	const char* strRFind(const StringView& _str, char _ch);

	/// Find substring in string. Limit search to _max characters.
	const char* strFind(const StringView& _str, const StringView& _find, int32_t _num = INT32_MAX);

	/// Find substring in string. Case insensitive. Limit search to _max characters.
	const char* strFindI(const StringView& _str, const StringView& _find, int32_t _num = INT32_MAX);

	///
	StringView strLTrim(const StringView& _str, const StringView& _chars);

	///
	StringView strRTrim(const StringView& _str, const StringView& _chars);

	///
	StringView strTrim(const StringView& _str, const StringView& _chars);

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

	/// Convert size in bytes to human readable string kibi units.
	int32_t prettify(char* _out, int32_t _count, uint64_t _value, Units::Enum _units = Units::Kibi);

	///
	int32_t toString(char* _out, int32_t _max, bool _value);

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

	///
	bool fromString(bool* _out, const StringView& _str);

	///
	bool fromString(float* _out, const StringView& _str);

	///
	bool fromString(double* _out, const StringView& _str);

	///
	bool fromString(int32_t* _out, const StringView& _str);

	///
	bool fromString(uint32_t* _out, const StringView& _str);

} // namespace bx

#include "inline/string.inl"

#endif // BX_STRING_H_HEADER_GUARD
