// license:MIT
// copyright-holders:Alfred Bratterud
// NOTE: Author allowed MAME project to distribute this file under MIT
//       license. Other projects need to do it under Apache 2 license
//
// This file is a part of the IncludeOS unikernel - www.includeos.org
//
// Copyright 2015-2016 Oslo and Akershus University College of Applied Sciences
// and Alfred Bratterud
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// https://github.com/pillarjs/path-to-regexp/blob/master/index.js

#include "path_to_regex.hpp"

namespace path2regex {

const std::regex PATH_REGEXP =
  std::regex{"((\\\\.)|(([\\/.])?(?:(?:\\:(\\w+)(?:\\(((?:\\\\.|[^\\\\()])+)\\))?|\\(((?:\\\\.|[^\\\\()])+)\\))([+*?])?|(\\*))))"};

std::regex path_to_regex(const std::string& path, Keys& keys, const Options& options) {
  Tokens all_tokens = parse(path);
  tokens_to_keys(all_tokens, keys); // fill keys with relevant tokens
  return tokens_to_regex(all_tokens, options);
}

std::regex path_to_regex(const std::string& path,  const Options& options) {
  return tokens_to_regex(parse(path), options);
}

// Parse a string for the raw tokens
std::vector<Token> parse(const std::string& str) {
  if (str.empty())
	return {};

  Tokens tokens;
  int key = 0;
  int index = 0;
  std::string path = "";
  std::smatch res;

  for (std::sregex_iterator i = std::sregex_iterator{str.begin(), str.end(), PATH_REGEXP};
	i != std::sregex_iterator{}; ++i) {

	res = *i;

	std::string m = res[0];       // the parameter, f.ex. /:test
	std::string escaped = res[2];
	int offset = res.position();

	// JS: path += str.slice(index, offset); from and included index to and included offset-1
	path += str.substr(index, (offset - index));  // from index, number of chars: offset - index

	index = offset + m.size();

	if (!escaped.empty()) {
	  path += escaped[1];   // if escaped == \a, escaped[1] == a (if str is "/\\a" f.ex.)
	  continue;
	}

	std::string next = (size_t(index) < str.size()) ? std::string{str.at(index)} : "";

	std::string prefix = res[4];  // f.ex. /
	std::string name = res[5];    // f.ex. test
	std::string capture = res[6]; // f.ex. \d+
	std::string group = res[7];   // f.ex. (users|admins)
	std::string modifier = res[8];  // f.ex. ?
	std::string asterisk = res[9];  // * if path is /*

	// Push the current path onto the tokens
	if (!path.empty()) {
	  Token stringToken;
	  stringToken.set_string_token(path);
	  tokens.push_back(stringToken);
	  path = "";
	}

	bool partial = (!prefix.empty()) && (!next.empty()) && (next != prefix);
	bool repeat = (modifier == "+") || (modifier == "*");
	bool optional = (modifier == "?") || (modifier == "*");
	std::string delimiter = (!prefix.empty()) ? prefix : "/";
	std::string pattern;

	if (!capture.empty())
	  pattern = capture;
	else if (!group.empty())
	  pattern = group;
	else
	  pattern = (!asterisk.empty()) ? ".*" : ("[^" + delimiter + "]+?");

	Token t;
	t.name = (!name.empty()) ? name : std::to_string(key++);
	t.prefix = prefix;
	t.delimiter = delimiter;
	t.optional = optional;
	t.repeat = repeat;
	t.partial = partial;
	t.asterisk = (asterisk == "*");
	t.pattern = pattern;
	t.is_string = false;
	tokens.push_back(t);
  }

  // Match any characters still remaining
  if (size_t(index) < str.size())
	path += str.substr(index);

  // If the path exists, push it onto the end
  if (!path.empty()) {
	Token stringToken;
	stringToken.set_string_token(path);
	tokens.push_back(stringToken);
  }

  return tokens;
}

// Creates a regex based on the given tokens and options (optional)
std::regex tokens_to_regex(const Tokens& tokens, const Options& options) {
  if (tokens.empty())
	return std::regex{""};

  // Set default values for options:
  bool strict = false;
  bool sensitive = false;
  bool end = true;

  if (!options.empty()) {
	auto it = options.find("strict");
	strict = (it != options.end()) ? options.find("strict")->second : false;

	it = options.find("sensitive");
	sensitive = (it != options.end()) ? options.find("sensitive")->second : false;

	it = options.find("end");
	end = (it != options.end()) ? options.find("end")->second : true;
  }

  std::string route = "";
  Token lastToken = tokens[tokens.size() - 1];
  std::regex re{"(.*\\/$)"};
  bool endsWithSlash = lastToken.is_string && std::regex_match(lastToken.name, re);
  // endsWithSlash if the last char in lastToken's name is a slash

  // Iterate over the tokens and create our regexp string
  for (size_t i = 0; i < tokens.size(); i++) {
	Token token = tokens[i];

	if (token.is_string) {
	  route += token.name;
	} else {
	  std::string prefix = token.prefix;
	  std::string capture = "(?:" + token.pattern + ")";

	  if (token.repeat)
		capture += "(?:" + prefix + capture + ")*";

	  if (token.optional) {

		if (!token.partial)
		  capture = "(?:" + prefix + "(" + capture + "))?";
		else
		  capture = prefix + "(" + capture + ")?";

	  } else {
		capture = prefix + "(" + capture + ")";
	  }

	  route += capture;
	}
  }

  // In non-strict mode we allow a slash at the end of match. If the path to
  // match already ends with a slash, we remove it for consistency. The slash
  // is valid at the end of a path match, not in the middle. This is important
  // in non-ending mode, where "/test/" shouldn't match "/test//route".

  if (!strict) {
	if (endsWithSlash)
	  route = route.substr(0, (route.size() - 1));

	route += "(?:\\/(?=$))?";
  }

  if (end) {
	route += "$";
  } else {
	// In non-ending mode, we need the capturing groups to match as much as
	// possible by using a positive lookahead to the end or next path segment
	if (!(strict && endsWithSlash))
	  route += "(?=\\/|$)";
  }

  if (sensitive)
	return std::regex{"^" + route};

  return std::regex{"^" + route, std::regex_constants::ECMAScript | std::regex_constants::icase};
}

void tokens_to_keys(const Tokens& tokens, Keys& keys) {
  for (const auto& token : tokens)
	if (!token.is_string)
	  keys.push_back(token);
}

} //< namespace route
