// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    strformat.h

    type-safe printf substitutes

***************************************************************************/

#include "strformat.h"

#include <iostream>
#include <sstream>


namespace util {

namespace detail {

template class format_chars<char>;
template class format_chars<wchar_t>;

template void format_flags::apply(std::ostream &) const;
template void format_flags::apply(std::wostream &) const;

template class format_argument<char>;
template void format_argument<char>::static_output<bool>(std::ostream &, format_flags const &, void const *);
template void format_argument<char>::static_output<char>(std::ostream &, format_flags const &, void const *);
template void format_argument<char>::static_output<signed char>(std::ostream &, format_flags const &, void const *);
template void format_argument<char>::static_output<unsigned char>(std::ostream &, format_flags const &, void const *);
template void format_argument<char>::static_output<short>(std::ostream &, format_flags const &, void const *);
template void format_argument<char>::static_output<unsigned short>(std::ostream &, format_flags const &, void const *);
template void format_argument<char>::static_output<int>(std::ostream &, format_flags const &, void const *);
template void format_argument<char>::static_output<unsigned int>(std::ostream &, format_flags const &, void const *);
template void format_argument<char>::static_output<long>(std::ostream &, format_flags const &, void const *);
template void format_argument<char>::static_output<unsigned long>(std::ostream &, format_flags const &, void const *);
template void format_argument<char>::static_output<long long>(std::ostream &, format_flags const &, void const *);
template void format_argument<char>::static_output<unsigned long long>(std::ostream &, format_flags const &, void const *);
template void format_argument<char>::static_output<char *>(std::ostream &, format_flags const &, void const *);
template void format_argument<char>::static_output<char const *>(std::ostream &, format_flags const &, void const *);
template void format_argument<char>::static_output<std::string>(std::ostream &, format_flags const &, void const *);
template void format_argument<char>::static_output<std::string_view>(std::ostream &, format_flags const &, void const *);
template bool format_argument<char>::static_make_integer<bool>(void const *, int &);
template bool format_argument<char>::static_make_integer<char>(void const *, int &);
template bool format_argument<char>::static_make_integer<signed char>(void const *, int &);
template bool format_argument<char>::static_make_integer<unsigned char>(void const *, int &);
template bool format_argument<char>::static_make_integer<short>(void const *, int &);
template bool format_argument<char>::static_make_integer<unsigned short>(void const *, int &);
template bool format_argument<char>::static_make_integer<int>(void const *, int &);
template bool format_argument<char>::static_make_integer<unsigned int>(void const *, int &);
template bool format_argument<char>::static_make_integer<long>(void const *, int &);
template bool format_argument<char>::static_make_integer<unsigned long>(void const *, int &);
template bool format_argument<char>::static_make_integer<long long>(void const *, int &);
template bool format_argument<char>::static_make_integer<unsigned long long>(void const *, int &);
template bool format_argument<char>::static_make_integer<char *>(void const *, int &);
template bool format_argument<char>::static_make_integer<char const *>(void const *, int &);
template bool format_argument<char>::static_make_integer<std::string>(void const *, int &);
template bool format_argument<char>::static_make_integer<std::string_view>(void const *, int &);
template void format_argument<char>::static_store_integer<bool>(void const *, std::streamoff);
template void format_argument<char>::static_store_integer<char>(void const *, std::streamoff);
template void format_argument<char>::static_store_integer<signed char>(void const *, std::streamoff);
template void format_argument<char>::static_store_integer<unsigned char>(void const *, std::streamoff);
template void format_argument<char>::static_store_integer<short>(void const *, std::streamoff);
template void format_argument<char>::static_store_integer<unsigned short>(void const *, std::streamoff);
template void format_argument<char>::static_store_integer<int>(void const *, std::streamoff);
template void format_argument<char>::static_store_integer<unsigned int>(void const *, std::streamoff);
template void format_argument<char>::static_store_integer<long>(void const *, std::streamoff);
template void format_argument<char>::static_store_integer<unsigned long>(void const *, std::streamoff);
template void format_argument<char>::static_store_integer<long long>(void const *, std::streamoff);
template void format_argument<char>::static_store_integer<unsigned long long>(void const *, std::streamoff);
template void format_argument<char>::static_store_integer<char *>(void const *, std::streamoff);
template void format_argument<char>::static_store_integer<char const *>(void const *, std::streamoff);
template void format_argument<char>::static_store_integer<std::string>(void const *, std::streamoff);
template void format_argument<char>::static_store_integer<std::string_view>(void const *, std::streamoff);

template class format_argument<wchar_t>;
template void format_argument<wchar_t>::static_output<bool>(std::wostream &, format_flags const &, void const *);
template void format_argument<wchar_t>::static_output<char>(std::wostream &, format_flags const &, void const *);
template void format_argument<wchar_t>::static_output<signed char>(std::wostream &, format_flags const &, void const *);
template void format_argument<wchar_t>::static_output<unsigned char>(std::wostream &, format_flags const &, void const *);
template void format_argument<wchar_t>::static_output<wchar_t>(std::wostream &, format_flags const &, void const *);
template void format_argument<wchar_t>::static_output<short>(std::wostream &, format_flags const &, void const *);
template void format_argument<wchar_t>::static_output<unsigned short>(std::wostream &, format_flags const &, void const *);
template void format_argument<wchar_t>::static_output<int>(std::wostream &, format_flags const &, void const *);
template void format_argument<wchar_t>::static_output<unsigned int>(std::wostream &, format_flags const &, void const *);
template void format_argument<wchar_t>::static_output<long>(std::wostream &, format_flags const &, void const *);
template void format_argument<wchar_t>::static_output<unsigned long>(std::wostream &, format_flags const &, void const *);
template void format_argument<wchar_t>::static_output<long long>(std::wostream &, format_flags const &, void const *);
template void format_argument<wchar_t>::static_output<unsigned long long>(std::wostream &, format_flags const &, void const *);
template void format_argument<wchar_t>::static_output<wchar_t *>(std::wostream &, format_flags const &, void const *);
template void format_argument<wchar_t>::static_output<wchar_t const *>(std::wostream &, format_flags const &, void const *);
template void format_argument<wchar_t>::static_output<std::wstring>(std::wostream &, format_flags const &, void const *);
template void format_argument<wchar_t>::static_output<std::wstring_view>(std::wostream &, format_flags const &, void const *);
template bool format_argument<wchar_t>::static_make_integer<bool>(void const *, int &);
template bool format_argument<wchar_t>::static_make_integer<char>(void const *, int &);
template bool format_argument<wchar_t>::static_make_integer<signed char>(void const *, int &);
template bool format_argument<wchar_t>::static_make_integer<unsigned char>(void const *, int &);
template bool format_argument<wchar_t>::static_make_integer<wchar_t>(void const *, int &);
template bool format_argument<wchar_t>::static_make_integer<short>(void const *, int &);
template bool format_argument<wchar_t>::static_make_integer<unsigned short>(void const *, int &);
template bool format_argument<wchar_t>::static_make_integer<int>(void const *, int &);
template bool format_argument<wchar_t>::static_make_integer<unsigned int>(void const *, int &);
template bool format_argument<wchar_t>::static_make_integer<long>(void const *, int &);
template bool format_argument<wchar_t>::static_make_integer<unsigned long>(void const *, int &);
template bool format_argument<wchar_t>::static_make_integer<long long>(void const *, int &);
template bool format_argument<wchar_t>::static_make_integer<unsigned long long>(void const *, int &);
template bool format_argument<wchar_t>::static_make_integer<wchar_t *>(void const *, int &);
template bool format_argument<wchar_t>::static_make_integer<wchar_t const *>(void const *, int &);
template bool format_argument<wchar_t>::static_make_integer<std::wstring>(void const *, int &);
template bool format_argument<wchar_t>::static_make_integer<std::wstring_view>(void const *, int &);
template void format_argument<wchar_t>::static_store_integer<bool>(void const *, std::streamoff);
template void format_argument<wchar_t>::static_store_integer<char>(void const *, std::streamoff);
template void format_argument<wchar_t>::static_store_integer<signed char>(void const *, std::streamoff);
template void format_argument<wchar_t>::static_store_integer<unsigned char>(void const *, std::streamoff);
template void format_argument<wchar_t>::static_store_integer<wchar_t>(void const *, std::streamoff);
template void format_argument<wchar_t>::static_store_integer<short>(void const *, std::streamoff);
template void format_argument<wchar_t>::static_store_integer<unsigned short>(void const *, std::streamoff);
template void format_argument<wchar_t>::static_store_integer<int>(void const *, std::streamoff);
template void format_argument<wchar_t>::static_store_integer<unsigned int>(void const *, std::streamoff);
template void format_argument<wchar_t>::static_store_integer<long>(void const *, std::streamoff);
template void format_argument<wchar_t>::static_store_integer<unsigned long>(void const *, std::streamoff);
template void format_argument<wchar_t>::static_store_integer<long long>(void const *, std::streamoff);
template void format_argument<wchar_t>::static_store_integer<unsigned long long>(void const *, std::streamoff);
template void format_argument<wchar_t>::static_store_integer<wchar_t *>(void const *, std::streamoff);
template void format_argument<wchar_t>::static_store_integer<wchar_t const *>(void const *, std::streamoff);
template void format_argument<wchar_t>::static_store_integer<std::wstring>(void const *, std::streamoff);
template void format_argument<wchar_t>::static_store_integer<std::wstring_view>(void const *, std::streamoff);

template class format_argument_pack<char>;
template class format_argument_pack<wchar_t>;

template std::ostream::off_type stream_format(std::ostream &, format_argument_pack<char> const &);
template std::wostream::off_type stream_format(std::wostream &, format_argument_pack<wchar_t> const &);

} // namespace detail

} // namespace util
