// license:MIT
// copyright-holders:Ole Christian Eidheim, Miodrag Milanovic
#ifndef SERVER_HTTP_HPP
#define SERVER_HTTP_HPP

#if defined(_MSC_VER)
#pragma warning(disable:4503)
#endif

#include "asio.h"
#include "asio/system_timer.hpp"
#include "path_to_regex.hpp"

#include <map>
#include <unordered_map>
#include <thread>
#include <functional>
#include <iostream>
#include <sstream>
#include <regex>

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
#endif
namespace webpp {
	template <class socket_type>
	class Server;

	template <class socket_type>
	class ServerBase {
	public:
		virtual ~ServerBase() {}

		class Response {
			friend class ServerBase<socket_type>;

			asio::streambuf m_streambuf;

			std::shared_ptr<socket_type> m_socket;
			std::ostream m_ostream;
			std::stringstream m_header;
			explicit Response(const std::shared_ptr<socket_type> &socket) : m_socket(socket), m_ostream(&m_streambuf) {}

			static std::string statusToString(int status)
			{
				switch (status) {
					default:
					case 200: return "HTTP/1.0 200 OK\r\n";
					case 201: return "HTTP/1.0 201 Created\r\n";
					case 202: return "HTTP/1.0 202 Accepted\r\n";
					case 204: return "HTTP/1.0 204 No Content\r\n";
					case 300: return "HTTP/1.0 300 Multiple Choices\r\n";
					case 301: return "HTTP/1.0 301 Moved Permanently\r\n";
					case 302: return "HTTP/1.0 302 Moved Temporarily\r\n";
					case 304: return "HTTP/1.0 304 Not Modified\r\n";
					case 400: return "HTTP/1.0 400 Bad Request\r\n";
					case 401: return "HTTP/1.0 401 Unauthorized\r\n";
					case 403: return "HTTP/1.0 403 Forbidden\r\n";
					case 404: return "HTTP/1.0 404 Not Found\r\n";
					case 500: return "HTTP/1.0 500 Internal Server Error\r\n";
					case 501: return "HTTP/1.0 501 Not Implemented\r\n";
					case 502: return "HTTP/1.0 502 Bad Gateway\r\n";
					case 504: return "HTTP/1.0 503 Service Unavailable\r\n";
				}
			}
		public:
			Response& status(int number) { m_ostream << statusToString(number); return *this; }
			void type(std::string str) { m_header << "Content-Type: "<< str << "\r\n"; }
			void send(std::string str) { m_ostream << m_header.str() << "Content-Length: " << str.length() << "\r\n\r\n" << str; }
			size_t size() const { return m_streambuf.size(); }
			std::shared_ptr<socket_type> socket() { return m_socket; }
		};

		class Content : public std::istream {
			friend class ServerBase<socket_type>;
		public:
			size_t size() const {
				return streambuf.size();
			}
			std::string string() const {
				std::stringstream ss;
				ss << rdbuf();
				return ss.str();
			}
		private:
			asio::streambuf &streambuf;
			explicit Content(asio::streambuf &streambuf): std::istream(&streambuf), streambuf(streambuf) {}
		};

		class Request {
			friend class ServerBase<socket_type>;
			friend class Server<socket_type>;
		public:
			std::string method, path, http_version;

			Content content;

			std::unordered_multimap<std::string, std::string, case_insensitive_hash, case_insensitive_equals> header;

			path2regex::Keys keys;
			std::map<std::string, std::string> params;

			std::string remote_endpoint_address;
			unsigned short remote_endpoint_port;

		private:
			Request(const socket_type &socket): content(streambuf) {
				try {
					remote_endpoint_address=socket.lowest_layer().remote_endpoint().address().to_string();
					remote_endpoint_port=socket.lowest_layer().remote_endpoint().port();
				}
				catch(...) {}
			}
			asio::streambuf streambuf;
		};

		class Config {
			friend class ServerBase<socket_type>;

			Config(unsigned short port) : port(port) {}
		public:
			/// Port number to use. Defaults to 80 for HTTP and 443 for HTTPS.
			unsigned short port;
			/// Number of threads that the server will use when start() is called. Defaults to 1 thread.
			size_t thread_pool_size=1;
			/// Timeout on request handling. Defaults to 5 seconds.
			size_t timeout_request=5;
			/// Timeout on content handling. Defaults to 300 seconds.
			size_t timeout_content=300;
			/// IPv4 address in dotted decimal form or IPv6 address in hexadecimal notation.
			/// If empty, the address will be any address.
			std::string address;
			/// Set to false to avoid binding the socket to an address that is already in use. Defaults to true.
			bool reuse_address=true;
		};
		///Set before calling start().
		Config m_config;
		private:
		  class regex_orderable : public std::regex {
			  std::string str;
		  public:
			  regex_orderable(std::regex reg, const std::string &regex_str) : std::regex(reg), str(regex_str) {}
			  bool operator<(const regex_orderable &rhs) const {
				  return str<rhs.str;
			  }
			  std::string getstr() const { return str; }
		  };
		using http_handler = std::function<void(std::shared_ptr<Response>, std::shared_ptr<Request>)>;

	public:
		template<class T> void on_get(std::string regex, T&& func) { std::lock_guard<std::mutex> lock(m_resource_mutex); path2regex::Keys keys; auto reg = path2regex::path_to_regex(regex, keys); m_resource[regex_orderable(reg,regex)]["GET"] = std::make_tuple(std::move(keys), func); }
		template<class T> void on_get(T&& func) { std::lock_guard<std::mutex> lock(m_resource_mutex); m_default_resource["GET"] = func; }
		template<class T> void on_post(std::string regex, T&& func) { std::lock_guard<std::mutex> lock(m_resource_mutex); path2regex::Keys keys; auto reg = path2regex::path_to_regex(regex, keys); m_resource[regex_orderable(reg, regex)]["POST"] = std::make_tuple(std::move(keys), func); }
		template<class T> void on_post(T&& func) { std::lock_guard<std::mutex> lock(m_resource_mutex); m_default_resource["POST"] = func; }
		template<class T> void on_put(std::string regex, T&& func) { std::lock_guard<std::mutex> lock(m_resource_mutex);  path2regex::Keys keys; auto reg = path2regex::path_to_regex(regex, keys); m_resource[regex_orderable(reg, regex)]["PUT"] = std::make_tuple(std::move(keys), func); }
		template<class T> void on_put(T&& func) { std::lock_guard<std::mutex> lock(m_resource_mutex);  m_default_resource["PUT"] = func; }
		template<class T> void on_patch(std::string regex, T&& func) { std::lock_guard<std::mutex> lock(m_resource_mutex); path2regex::Keys keys; auto reg = path2regex::path_to_regex(regex, keys); m_resource[regex_orderable(reg, regex)]["PATCH"] = std::make_tuple(std::move(keys), func); }
		template<class T> void on_patch(T&& func) { std::lock_guard<std::mutex> lock(m_resource_mutex); m_default_resource["PATCH"] = func; }
		template<class T> void on_delete(std::string regex, T&& func) { std::lock_guard<std::mutex> lock(m_resource_mutex); path2regex::Keys keys; auto reg = path2regex::path_to_regex(regex, keys); m_resource[regex_orderable(reg, regex)]["DELETE"] = std::make_tuple(std::move(keys), func); }
		template<class T> void on_delete(T&& func) { std::lock_guard<std::mutex> lock(m_resource_mutex); m_default_resource["DELETE"] = func; }

		void remove_handler(std::string regex)
		{
			std::lock_guard<std::mutex> lock(m_resource_mutex);
			for (auto &it = m_resource.begin(); it != m_resource.end(); ++it)
			{
				if (it->first.getstr() == regex)
				{
					m_resource.erase(it);
					break;
				}
			}
		}

		std::function<void(std::shared_ptr<typename ServerBase<socket_type>::Request>, const std::error_code&)> on_error;

		std::function<void(std::shared_ptr<socket_type> socket, std::shared_ptr<typename ServerBase<socket_type>::Request>)> on_upgrade;
	private:
		/// Warning: do not add or remove resources after start() is called
		std::map<regex_orderable, std::map<std::string, std::tuple<path2regex::Keys, http_handler>>>  m_resource;

		std::map<std::string, http_handler> m_default_resource;

		std::mutex m_resource_mutex;
	public:
		virtual void start() {
			if(!m_io_context)
				m_io_context=std::make_shared<asio::io_context>();

			if(m_io_context->stopped())
				m_io_context.reset();

			asio::ip::tcp::endpoint endpoint;
			if(m_config.address.size()>0)
				endpoint=asio::ip::tcp::endpoint(asio::ip::make_address(m_config.address), m_config.port);
			else
				endpoint=asio::ip::tcp::endpoint(asio::ip::tcp::v4(), m_config.port);

			if(!acceptor)
				acceptor= std::make_unique<asio::ip::tcp::acceptor>(*m_io_context);
			acceptor->open(endpoint.protocol());
			acceptor->set_option(asio::socket_base::reuse_address(m_config.reuse_address));
			acceptor->bind(endpoint);
			acceptor->listen();

			accept();

			if (!m_external_context)
				m_io_context->run();
		}

		void stop() const
		{
			acceptor->close();
			if (!m_external_context)
				m_io_context->stop();
		}

		///Use this function if you need to recursively send parts of a longer message
		void send(const std::shared_ptr<Response> &response, const std::function<void(const std::error_code&)>& callback=nullptr) const {
			asio::async_write(*response->socket(), response->m_streambuf, [this, response, callback](const std::error_code& ec, size_t /*bytes_transferred*/) {
				if(callback)
					callback(ec);
			});
		}

		void set_io_context(std::shared_ptr<asio::io_context> new_io_context)
		{
			m_io_context = new_io_context;
			m_external_context = true;
		}
	protected:
		std::shared_ptr<asio::io_context> m_io_context;
		bool m_external_context;
		std::unique_ptr<asio::ip::tcp::acceptor> acceptor;
		std::vector<std::thread> threads;

		ServerBase(unsigned short port) : m_config(port), m_external_context(false) {}

		virtual void accept()=0;

		std::shared_ptr<asio::system_timer> get_timeout_timer(const std::shared_ptr<socket_type> &socket, long seconds) {
			if(seconds==0)
				return nullptr;
			auto timer = std::make_shared<asio::system_timer>(*m_io_context);
			timer->expires_at(std::chrono::system_clock::now() + std::chrono::seconds(seconds));
			timer->async_wait([socket](const std::error_code& ec){
				if(!ec) {
					std::error_code newec = ec;
					socket->lowest_layer().shutdown(asio::ip::tcp::socket::shutdown_both, newec);
					socket->lowest_layer().close();
				}
			});
			return timer;
		}

		void read_request_and_content(const std::shared_ptr<socket_type> &socket) {
			//Create new streambuf (Request::streambuf) for async_read_until()
			//shared_ptr is used to pass temporary objects to the asynchronous functions
			std::shared_ptr<Request> request(new Request(*socket));

			//Set timeout on the following asio::async-read or write function
			auto timer = get_timeout_timer(socket, m_config.timeout_request);

			asio::async_read_until(*socket, request->streambuf, "\r\n\r\n",
					[this, socket, request, timer](const std::error_code& ec, size_t bytes_transferred) {
				if(timer)
					timer->cancel();
				if(!ec) {
					//request->streambuf.size() is not necessarily the same as bytes_transferred, from Boost-docs:
					//"After a successful async_read_until operation, the streambuf may contain additional data beyond the delimiter"
					//The chosen solution is to extract lines from the stream directly when parsing the header. What is left of the
					//streambuf (maybe some bytes of the content) is appended to in the async_read-function below (for retrieving content).
					size_t num_additional_bytes=request->streambuf.size()-bytes_transferred;

					if (!parse_request(request))
						return;

					//If content, read that as well
					auto it = request->header.find("Content-Length");
					if (it != request->header.end()) {
						unsigned long long content_length;
						try {
							content_length = stoull(it->second);
						}
						catch (const std::exception &) {
							if (on_error)
								on_error(request, std::error_code(EPROTO, std::generic_category()));
							return;
						}
						if (content_length > num_additional_bytes) {
							//Set timeout on the following asio::async-read or write function
							auto timer2 = get_timeout_timer(socket, m_config.timeout_content);
							asio::async_read(*socket, request->streambuf,
								asio::transfer_exactly(size_t(content_length) - num_additional_bytes),
								[this, socket, request, timer2]
							(const std::error_code& ec, size_t /*bytes_transferred*/) {
								if (timer2)
									timer2->cancel();
								if (!ec)
									find_resource(socket, request);
								else if (on_error)
									on_error(request, ec);
							});
						}
						else {
							find_resource(socket, request);
						}
					}
					else {
						find_resource(socket, request);
					}
				}
				else if (on_error)
					on_error(request, ec);
			});
		}

		bool parse_request(const std::shared_ptr<Request> &request) const {
			std::string line;
			getline(request->content, line);
			size_t method_end;
			if((method_end=line.find(' '))!=std::string::npos) {
				size_t path_end;
				if((path_end=line.find(' ', method_end+1))!=std::string::npos) {
					request->method=line.substr(0, method_end);
					request->path=line.substr(method_end+1, path_end-method_end-1);

					size_t protocol_end;
					if((protocol_end=line.find('/', path_end+1))!=std::string::npos) {
						if(line.compare(path_end+1, protocol_end-path_end-1, "HTTP")!=0)
							return false;
						request->http_version=line.substr(protocol_end+1, line.size()-protocol_end-2);
					}
					else
						return false;

					getline(request->content, line);
					size_t param_end;
					while((param_end=line.find(':'))!=std::string::npos) {
						size_t value_start=param_end+1;
						if((value_start)<line.size()) {
							if(line[value_start]==' ')
								value_start++;
							if(value_start<line.size())
								request->header.emplace(line.substr(0, param_end), line.substr(value_start, line.size() - value_start - 1));
						}

						getline(request->content, line);
					}
				}
				else
					return false;
			}
			else
				return false;
			return true;
		}

		void find_resource(const std::shared_ptr<socket_type> &socket, const std::shared_ptr<Request> &request) {
			std::lock_guard<std::mutex> lock(m_resource_mutex);
			//Upgrade connection
			if(on_upgrade) {
				auto it=request->header.find("Upgrade");
				if(it!=request->header.end()) {
					on_upgrade(socket, request);
					return;
				}
			}
			//Find path- and method-match, and call write_response
			for(auto& regex_method : m_resource) {
				auto it = regex_method.second.find(request->method);
				if (it != regex_method.second.end()) {
						std::smatch sm_res;
						if (std::regex_match(request->path, sm_res, regex_method.first)) {
							request->keys = std::get<0>(it->second);
							for (size_t i = 0; i < request->keys.size(); i++) {
								request->params.insert(std::pair<std::string, std::string>(request->keys[i].name, sm_res[i + 1]));
							}
							write_response(socket, request, std::get<1>(it->second));
							return;
						}
				}
			}
			auto it=m_default_resource.find(request->method);
			if(it!=m_default_resource.end()) {
				write_response(socket, request, it->second);
			}
		}

		void write_response(const std::shared_ptr<socket_type> &socket, const std::shared_ptr<Request> &request, http_handler& resource_function) {
			//Set timeout on the following asio::async-read or write function
			auto timer = get_timeout_timer(socket, m_config.timeout_content);

			auto response=std::shared_ptr<Response>(new Response(socket), [this, request, timer](Response *response_ptr) {
				auto response=std::shared_ptr<Response>(response_ptr);
				send(response, [this, response, request, timer](const std::error_code& ec) {
					if (timer)
						timer->cancel();
					if (!ec) {
						float http_version;
						try {
							http_version = stof(request->http_version);
						}
						catch (const std::exception &) {
							if (on_error)
								on_error(request, std::error_code(EPROTO, std::generic_category()));
							return;
						}

						auto range = request->header.equal_range("Connection");
						case_insensitive_equals check;
						for (auto it = range.first; it != range.second; ++it) {
							if (check(it->second, "close"))
								return;
						}
						if (http_version > 1.05)
							read_request_and_content(response->socket());
					}
					else if (on_error)
						on_error(request, ec);
				});
			});

			try {
				resource_function(response, request);
			}
			catch(const std::exception &) {
				if (on_error)
					on_error(request, std::error_code(EPROTO, std::generic_category()));
			}
		}
	};

	template<class socket_type>
	class Server : public ServerBase<socket_type> {
	public:
		Server(unsigned short port, size_t num_threads, long timeout_request, long timeout_send_or_receive)
			: ServerBase<socket_type>(port, num_threads, timeout_request, timeout_send_or_receive)
		{
		}
	};

	using HTTP = asio::ip::tcp::socket;

	template<>
	class Server<HTTP> : public ServerBase<HTTP> {
	public:
		Server() : ServerBase<HTTP>::ServerBase(80) {}
	protected:
		void accept() override {
			//Create new socket for this connection
			//Shared_ptr is used to pass temporary objects to the asynchronous functions
			auto socket = std::make_shared<HTTP>(*m_io_context);

			acceptor->async_accept(*socket, [this, socket](const std::error_code& ec){
				//Immediately start accepting a new connection (if io_context hasn't been stopped)
				if (ec != asio::error::operation_aborted)
					accept();

				if(!ec) {
					asio::ip::tcp::no_delay option(true);
					socket->set_option(option);

					read_request_and_content(socket);
				} else if (on_error)
					on_error(std::shared_ptr<Request>(new Request(*socket)), ec);
			});
		}
	};

	class http_server : public Server<HTTP> {
	public:
		http_server() : Server<HTTP>::Server() {}
	};
}
#endif  /* SERVER_HTTP_HPP */
