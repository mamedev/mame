// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    strformat.h

    type-safe printf substitutes

***************************************************************************/

#include "strformat.h"
#include "vecstream.h"

#include <iostream>
#include <sstream>


namespace util {

namespace detail {

template class format_chars<char>;
template class format_chars<wchar_t>;

template void format_flags::apply(std::ostream &) const;
template void format_flags::apply(std::wostream &) const;
template void format_flags::apply(std::iostream &) const;
template void format_flags::apply(std::wiostream &) const;
template void format_flags::apply(std::ostringstream &) const;
template void format_flags::apply(std::wostringstream &) const;
template void format_flags::apply(std::stringstream &) const;
template void format_flags::apply(std::wstringstream &) const;
template void format_flags::apply(ovectorstream &) const;
template void format_flags::apply(wovectorstream &) const;
template void format_flags::apply(vectorstream &) const;
template void format_flags::apply(wvectorstream &) const;

template class format_argument<std::ostream>;
template class format_argument<std::wostream>;
template class format_argument<std::iostream>;
template class format_argument<std::wiostream>;
template class format_argument<std::ostringstream>;
template class format_argument<std::wostringstream>;
template class format_argument<std::stringstream>;
template class format_argument<std::wstringstream>;
template class format_argument<ovectorstream>;
template class format_argument<wovectorstream>;
template class format_argument<vectorstream>;
template class format_argument<wvectorstream>;

template class format_argument_pack<std::ostream>;
template class format_argument_pack<std::wostream>;
template class format_argument_pack<std::iostream>;
template class format_argument_pack<std::wiostream>;
template class format_argument_pack<std::ostringstream>;
template class format_argument_pack<std::wostringstream>;
template class format_argument_pack<std::stringstream>;
template class format_argument_pack<std::wstringstream>;
template class format_argument_pack<ovectorstream>;
template class format_argument_pack<wovectorstream>;
template class format_argument_pack<vectorstream>;
template class format_argument_pack<wvectorstream>;

template std::ostream::off_type stream_format(std::ostream &, format_argument_pack<std::ostream> const &);
template std::wostream::off_type stream_format(std::wostream &, format_argument_pack<std::wostream> const &);
template std::iostream::off_type stream_format(std::iostream &, format_argument_pack<std::ostream> const &);
template std::iostream::off_type stream_format(std::iostream &, format_argument_pack<std::iostream> const &);
template std::wiostream::off_type stream_format(std::wiostream &, format_argument_pack<std::wostream> const &);
template std::wiostream::off_type stream_format(std::wiostream &, format_argument_pack<std::wiostream> const &);
template std::ostringstream::off_type stream_format(std::ostringstream &, format_argument_pack<std::ostream> const &);
template std::ostringstream::off_type stream_format(std::ostringstream &, format_argument_pack<std::ostringstream> const &);
template std::wostringstream::off_type stream_format(std::wostringstream &, format_argument_pack<std::wostream> const &);
template std::wostringstream::off_type stream_format(std::wostringstream &, format_argument_pack<std::wostringstream> const &);
template std::stringstream::off_type stream_format(std::stringstream &, format_argument_pack<std::ostream> const &);
template std::stringstream::off_type stream_format(std::stringstream &, format_argument_pack<std::iostream> const &);
template std::stringstream::off_type stream_format(std::stringstream &, format_argument_pack<std::stringstream> const &);
template std::wstringstream::off_type stream_format(std::wstringstream &, format_argument_pack<std::wostream> const &);
template std::wstringstream::off_type stream_format(std::wstringstream &, format_argument_pack<std::wiostream> const &);
template std::wstringstream::off_type stream_format(std::wstringstream &, format_argument_pack<std::wstringstream> const &);
template ovectorstream::off_type stream_format(ovectorstream &, format_argument_pack<std::ostream> const &);
template ovectorstream::off_type stream_format(ovectorstream &, format_argument_pack<ovectorstream> const &);
template wovectorstream::off_type stream_format(wovectorstream &, format_argument_pack<std::wostream> const &);
template wovectorstream::off_type stream_format(wovectorstream &, format_argument_pack<wovectorstream> const &);
template vectorstream::off_type stream_format(vectorstream &, format_argument_pack<std::ostream> const &);
template vectorstream::off_type stream_format(vectorstream &, format_argument_pack<std::iostream> const &);
template vectorstream::off_type stream_format(vectorstream &, format_argument_pack<vectorstream> const &);
template wvectorstream::off_type stream_format(wvectorstream &, format_argument_pack<std::wostream> const &);
template wvectorstream::off_type stream_format(wvectorstream &, format_argument_pack<std::wiostream> const &);
template wvectorstream::off_type stream_format(wvectorstream &, format_argument_pack<wvectorstream> const &);

} // namespace detail

} // namespace util
