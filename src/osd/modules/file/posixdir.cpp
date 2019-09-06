// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Vas Crabb
//============================================================
//
//  sdldir.c - SDL core directory access functions
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================


#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#ifdef __linux__
#ifndef __USE_LARGEFILE64
#define __USE_LARGEFILE64
#endif
#ifndef __USE_BSD
#define __USE_BSD
#endif
#endif

#ifdef WIN32
#define _FILE_OFFSET_BITS 64
#endif

#if !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__OpenBSD__) && !defined(__bsdi__) && !defined(__DragonFly__)
#ifdef _XOPEN_SOURCE
#if _XOPEN_SOURCE < 500
#undef _XOPEN_SOURCE
#endif
#endif
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif
#endif

#define _DARWIN_C_SOURCE  // to get DT_xxx on OS X

#include "osdcore.h"
#include "modules/lib/osdlib.h"
#include "util/strformat.h"

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <utility>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>



namespace osd {
namespace {
//============================================================
//  CONSTANTS
//============================================================

#if defined(WIN32)
constexpr char PATHSEPCH = '\\';
#else
constexpr char PATHSEPCH = '/';
#endif

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__DragonFly__) || defined(__EMSCRIPTEN__) || defined(__ANDROID__) || defined(WIN32) || defined(SDLMAME_NO64BITIO)
using sdl_dirent = struct dirent;
using sdl_stat = struct stat;
#define sdl_readdir readdir
#define sdl_stat_fn stat
#else
using sdl_dirent = struct dirent64;
using sdl_stat = struct stat64;
#define sdl_readdir readdir64
#define sdl_stat_fn stat64
#endif

#if (defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__DragonFly__))
#define HAS_DT_XXX 1
#else
#define HAS_DT_XXX 0
#endif



//============================================================
//  TYPE DEFINITIONS
//============================================================

class posix_directory : public osd::directory
{
public:
	posix_directory();
	virtual ~posix_directory() override;

	virtual const entry *read() override;

	bool open_impl(std::string const &dirname);

private:
	typedef std::unique_ptr<DIR, int (*)(DIR *)>    dir_ptr;

	entry       m_entry;
	sdl_dirent  *m_data;
	dir_ptr     m_fd;
	std::string m_path;
};


//============================================================
//  posix_directory::~posix_directory
//============================================================

posix_directory::posix_directory()
	: m_entry()
	, m_data(nullptr)
	, m_fd(nullptr, &::closedir)
	, m_path()
{
}


//============================================================
//  posix_directory::~posix_directory
//============================================================

posix_directory::~posix_directory()
{
}


//============================================================
//  posix_directory::read
//============================================================

const osd::directory::entry *posix_directory::read()
{
	m_data = sdl_readdir(m_fd.get());
	if (!m_data)
		return nullptr;

	m_entry.name = m_data->d_name;

	sdl_stat st;
	bool stat_err(0 > sdl_stat_fn(util::string_format("%s%c%s", m_path, PATHSEPCH, m_data->d_name).c_str(), &st));

#if HAS_DT_XXX
	switch (m_data->d_type)
	{
	case DT_DIR:
		m_entry.type = entry::entry_type::DIR;
		break;
	case DT_REG:
		m_entry.type = entry::entry_type::FILE;
		break;
	case DT_UNKNOWN:
	case DT_LNK:
		if (stat_err)
			m_entry.type = entry::entry_type::OTHER;
		else if (S_ISDIR(st.st_mode))
			m_entry.type = entry::entry_type::DIR;
		else
			m_entry.type = entry::entry_type::FILE;
		break;
	default:
		m_entry.type = entry::entry_type::OTHER;
	}
#else
	if (stat_err)
		m_entry.type = entry::entry_type::OTHER;
	else if (S_ISDIR(st.st_mode))
		m_entry.type = entry::entry_type::DIR;
	else
		m_entry.type = entry::entry_type::FILE;
#endif
	m_entry.size = stat_err ? 0 : std::uint64_t(std::make_unsigned_t<decltype(st.st_size)>(st.st_size));
	m_entry.last_modified = std::chrono::system_clock::from_time_t(st.st_mtime);
	return &m_entry;
}


//============================================================
//  posix_directory::open_impl
//============================================================

bool posix_directory::open_impl(std::string const &dirname)
{
	assert(!m_fd);

	osd_subst_env(m_path, dirname);
	m_fd.reset(::opendir(m_path.c_str()));
	return bool(m_fd);
}

} // anonymous namespace


//============================================================
//  osd::directory::open
//============================================================

directory::ptr directory::open(std::string const &dirname)
{
	std::unique_ptr<posix_directory> dir;
	try { dir.reset(new posix_directory); }
	catch (...) { return nullptr; }

	if (!dir->open_impl(dirname))
		return nullptr;

	return ptr(std::move(dir));
}

} // namespace osd


//============================================================
//  osd_subst_env
//============================================================

void osd_subst_env(std::string &dst, std::string const &src)
{
	std::string result, var;
	auto start = src.begin();

	// a leading tilde expands as $HOME
	if ((src.end() != start) && ('~' == *start))
	{
		char const *const home = std::getenv("HOME");
		if (home)
		{
			++start;
			if ((src.end() == start) || (osd::PATHSEPCH == *start))
				result.append(home);
			else
				result.push_back('~');
		}
	}

	while (src.end() != start)
	{
		// find $ marking start of environment variable or end of string
		auto it = start;
		while ((src.end() != it) && ('$' != *it)) ++it;
		if (start != it) result.append(start, it);
		start = it;

		if (src.end() != start)
		{
			start = ++it;
			if ((src.end() != start) && ('{' == *start))
			{
				start = ++it;
				for (++it; (src.end() != it) && ('}' != *it); ++it) { }
				if (src.end() == it)
				{
					result.append("${").append(start, it);
					start = it;
				}
				else
				{
					var.assign(start, it);
					start = ++it;
					const char *const exp = std::getenv(var.c_str());
					if (exp)
						result.append(exp);
					else
						fprintf(stderr, "Warning: osd_subst_env variable %s not found.\n", var.c_str());
				}
			}
			else if ((src.end() != start) && (('_' == *start) || std::isalnum(*start)))
			{
				for (++it; (src.end() != it) && (('_' == *it) || std::isalnum(*it)); ++it) { }
				var.assign(start, it);
				start = it;
				const char *const exp = std::getenv(var.c_str());
				if (exp)
					result.append(exp);
				else
					fprintf(stderr, "Warning: osd_subst_env variable %s not found.\n", var.c_str());
			}
			else
			{
				result.push_back('$');
			}
		}
	}

	dst = std::move(result);
}
