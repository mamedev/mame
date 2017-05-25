// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    vecstream.cpp

    streams with vector storage

***************************************************************************/

#include "vecstream.h"

namespace util {

template class basic_ivectorstream<char>;
template class basic_ivectorstream<wchar_t>;
template class basic_ovectorstream<char>;
template class basic_ovectorstream<wchar_t>;
template class basic_vectorstream<char>;
template class basic_vectorstream<wchar_t>;

} // namespace util
