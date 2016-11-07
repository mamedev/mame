// license:Boost
// copyright-holders:Christopher M. Kohlhoff

#ifndef HTTP_HEADER_HPP
#define HTTP_HEADER_HPP

#include <string>

namespace http {
namespace server {

struct header
{
  std::string name;
  std::string value;
};

} // namespace server
} // namespace http

#endif // HTTP_HEADER_HPP
