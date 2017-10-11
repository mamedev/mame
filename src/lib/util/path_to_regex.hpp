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

#ifndef PATH_TO_REGEX_HPP
#define PATH_TO_REGEX_HPP

#include <map>
#include <regex>
#include <vector>

namespace path2regex {

struct Token {
	std::string name      {};   // can be a string or an int (index)
	std::string prefix    {};
	std::string delimiter {};
	std::string pattern   {};
  bool        optional  {false};
  bool        repeat    {false};
  bool        partial   {false};
  bool        asterisk  {false};
	bool        is_string {false};  // If it is a string we only put/have a string in the name-attribute (path in parse-method)
					// So if this is true, we can ignore all attributes except name

  void set_string_token(const std::string& name_) {
	name = name_;
	is_string = true;
  }
}; //< struct Token

using Keys    = std::vector<Token>;
using Tokens  = std::vector<Token>;
using Options = std::map<std::string, bool>;

  /**
   *  Creates a path-regex from string input (path)
   *  Updates keys-vector (empty input parameter)
   *
   *  Calls parse-method and then tokens_to_regex-method based on the tokens returned from parse-method
   *  Puts the tokens that are keys (not string-tokens) into keys-vector
   *
   *  std::vector<Token> keys (empty)
   *    Is created outside the class and sent in as parameter
   *    One Token-object in the keys-vector per parameter found in the path
   *
   *  std::map<std::string, bool> options (optional)
   *    Can contain bool-values for the keys "sensitive", "strict" and/or "end"
   *    Default:
   *      strict = false
   *      sensitive = false
   *      end = true
   */
	std::regex path_to_regex(const std::string& path, Keys& keys, const Options& options = Options{});

  /**
   *  Creates a path-regex from string input (path)
   *
   *  Calls parse-method and then tokens_to_regex-method based on the tokens returned from parse-method
   *
   *  std::map<std::string, bool> options (optional)
   *    Can contain bool-values for the keys "sensitive", "strict" and/or "end"
   *    Default:
   *      strict = false
   *      sensitive = false
   *      end = true
   */
  std::regex path_to_regex(const std::string& path, const Options& options = Options{});

  /**
   *  Creates vector of tokens based on the given string (this vector of tokens can be sent as
   *  input to tokens_to_regex-method and includes tokens that are strings, not only tokens
   *  that are parameters in str)
   */
  Tokens parse(const std::string& str);

  /**
   *  Creates a regex based on the tokens and options (optional) given
   */
  std::regex tokens_to_regex(const Tokens& tokens, const Options& options = Options{});

  /**
   *  Goes through the tokens-vector and push all tokens that are not string-tokens
   *  onto keys-vector
   */
  void tokens_to_keys(const Tokens& tokens, Keys& keys);

} //< namespace path2regex

#endif //< PATH_TO_REGEX_HPP
