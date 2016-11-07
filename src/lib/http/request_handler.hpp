// license:Boost
// copyright-holders:Christopher M. Kohlhoff

#ifndef HTTP_REQUEST_HANDLER_HPP
#define HTTP_REQUEST_HANDLER_HPP

#include <string>

namespace http {
namespace server {

struct reply;
struct request;

/// The common handler for all incoming requests.
class request_handler
{
public:
  request_handler(const request_handler&) = delete;
  request_handler& operator=(const request_handler&) = delete;

  /// Construct with a directory containing files to be served.
  explicit request_handler(const std::string& doc_root);

	/// Handle a request and produce a reply.
  void handle_request(const request& req, reply& rep) const;

  bool serve_static_content(std::string& request_path, reply& rep) const;

private:
  /// The directory containing the files to be served.
  std::string m_doc_root;

  /// Perform URL-decoding on a string. Returns false if the encoding was
  /// invalid.
  static bool url_decode(const std::string& in, std::string& out);
};

} // namespace server
} // namespace http

#endif // HTTP_REQUEST_HANDLER_HPP
