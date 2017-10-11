// license:MIT
// copyright-holders:Ole Christian Eidheim, Miodrag Milanovic
#ifndef MAME_LIB_UTIL_SERVER_HTTP_HPP
#define MAME_LIB_UTIL_SERVER_HTTP_HPP

#include "path_to_regex.hpp"

#include <map>
#include <unordered_map>
#include <thread>
#include <functional>
#include <iostream>
#include <sstream>
#include <regex>
#include <cctype>

#ifndef CASE_INSENSITIVE_EQUALS_AND_HASH
#define CASE_INSENSITIVE_EQUALS_AND_HASH
class case_insensitive_equals {
public:
	bool operator()(const std::string &key1, const std::string &key2) const {
		return key1.size() == key2.size()
			&& equal(key1.cbegin(), key1.cend(), key2.cbegin(),
				[](std::string::value_type key1v, std::string::value_type key2v)
		{ return tolower(key1v) == tolower(key2v); });
	}
};
class case_insensitive_hash {
public:
	size_t operator()(const std::string &key) const {
		size_t seed = 0;
		for (auto &c : key) {
			std::hash<char> hasher;
			seed ^= hasher(std::tolower(c)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		return seed;
	}
};
#endif /* CASE_INSENSITIVE_EQUALS_AND_HASH */

namespace webpp {
	struct Request {
		std::string method, path, http_version;

		std::unordered_multimap<std::string, std::string, case_insensitive_hash, case_insensitive_equals> header;

		path2regex::Keys keys;
		std::map<std::string, std::string> params;

		std::string remote_endpoint_address;
		unsigned short remote_endpoint_port;

		virtual ~Request() {}
	};

	struct Response {
		virtual Response& status(int number) = 0;
		virtual void type(std::string str) = 0;
		virtual void send(std::string str) = 0;
		virtual size_t size() const = 0;

		virtual ~Response() {}
	};

	class http_server;
}
#endif  /* MAME_LIB_UTIL_SERVER_HTTP_HPP */
