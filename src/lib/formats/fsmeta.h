// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Filesystem metadata management

#ifndef MAME_FORMATS_FSMETA_H
#define MAME_FORMATS_FSMETA_H

#pragma once

#include "timeconv.h"

#include <cassert>
#include <functional>
#include <string>
#include <unordered_map>
#include <variant>

enum class fs_meta_name {
	basic,
	creation_date,
	length,
	loading_address,
	locked,
	modification_date,
	name,
	os_minimum_version,
	os_version,
	rsrc_length,
	sequential,
	size_in_blocks,
};

enum class fs_meta_type {
	date,
	flag,
	number,
	string,
};

class fs_meta {
public:
	static std::string to_string(fs_meta_type type, const fs_meta &m);
	static fs_meta from_string(fs_meta_type type, std::string value);

	fs_meta() { value = false; }
	fs_meta(std::string str) { value = str; }
	fs_meta(bool b) { value = b; }
	fs_meta(int32_t num) { value = uint64_t(num); }
	fs_meta(uint32_t num) { value = uint64_t(num); }
	fs_meta(int64_t num) { value = uint64_t(num); }
	fs_meta(uint64_t num) { value = num; }
	fs_meta(util::arbitrary_datetime dt) { value = dt; }

	util::arbitrary_datetime as_date() const { return *std::get_if<util::arbitrary_datetime>(&value); }
	bool as_flag() const { return *std::get_if<bool>(&value); }
	uint64_t as_number() const { return *std::get_if<uint64_t>(&value); }
	std::string as_string() const { return *std::get_if<std::string>(&value); }

private:
	std::variant<std::string, uint64_t, bool, util::arbitrary_datetime> value;
};

class fs_meta_data {
public:
	std::unordered_map<fs_meta_name, fs_meta> meta;

	static const char *entry_name(fs_meta_name name);

	bool has(fs_meta_name name) const { return meta.find(name) != meta.end(); }
	bool empty() const { return meta.empty(); }

	void set(fs_meta_name name, const fs_meta &val) { meta[name] = val; }
	void set(fs_meta_name name, std::string str) { set(name, fs_meta(str)); }
	void set(fs_meta_name name, bool b) { set(name, fs_meta(b)); }
	void set(fs_meta_name name, int32_t num) { set(name, fs_meta(num)); }
	void set(fs_meta_name name, uint32_t num) { set(name, fs_meta(num)); }
	void set(fs_meta_name name, int64_t num) { set(name, fs_meta(num)); }
	void set(fs_meta_name name, uint64_t num) { set(name, fs_meta(num)); }
	void set(fs_meta_name name, util::arbitrary_datetime dt) { set(name, fs_meta(dt)); }
	void set_now(fs_meta_name name) { set(name, fs_meta(util::arbitrary_datetime::now())); }

	fs_meta get(fs_meta_name name) const { auto i = meta.find(name);  assert(i != meta.end()); return i->second; }
	util::arbitrary_datetime get_date(fs_meta_name name, util::arbitrary_datetime def = util::arbitrary_datetime::now()) const { auto i = meta.find(name);  if(i == meta.end()) return def; else return i->second.as_date(); }
	bool get_flag(fs_meta_name name, bool def = false) const { auto i = meta.find(name);  if(i == meta.end()) return def; else return i->second.as_flag(); }
	uint64_t get_number(fs_meta_name name, uint64_t def = 0) const { auto i = meta.find(name);  if(i == meta.end()) return def; else return i->second.as_number(); }
	std::string get_string(fs_meta_name name, std::string def = "") const { auto i = meta.find(name);  if(i == meta.end()) return def; else return i->second.as_string(); }
};

struct fs_meta_description {
	fs_meta_name m_name;
	fs_meta_type m_type;
	fs_meta m_default;
	bool m_ro;
	std::function<void (const fs_meta &)> m_validator;
	const char *m_tooltip;

	fs_meta_description(fs_meta_name name, fs_meta_type type, int def, bool ro, std::function<void (fs_meta)> validator, const char *tooltip) :
		m_name(name), m_type(type), m_default(uint64_t(def)), m_ro(ro), m_validator(validator), m_tooltip(tooltip)
	{}

	template<typename T> fs_meta_description(fs_meta_name name, fs_meta_type type, T def, bool ro, std::function<void (fs_meta)> validator, const char *tooltip) :
		m_name(name), m_type(type), m_default(def), m_ro(ro), m_validator(validator), m_tooltip(tooltip)
	{}
};

#endif // MAME_FORMATS_FSMETA_H
