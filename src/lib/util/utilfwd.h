// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**********************************************************************

    utilfwd.h

    Forward declarations of types

**********************************************************************/
#ifndef MAME_LIB_UTIL_UTILFWD_H
#define MAME_LIB_UTIL_UTILFWD_H

// chd.h
class chd_file;


namespace util {

// corefile.h
class core_file;

// ioprocs.h
class read_stream;
class write_stream;
class read_write_stream;
class random_access;
class random_read;
class random_write;
class random_read_write;

// unzip.h
class archive_file;

} // namespace util


namespace util::xml {

// xmlfile.h
class data_node;
class file;

} // namespace util::xml

#endif // MAME_LIB_UTIL_UTILFWD_H
