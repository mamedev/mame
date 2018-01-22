// license:MIT
// copyright-holders:Ole Christian Eidheim, Miodrag Milanovic
#ifndef SERVER_HTTPS_HPP
#define SERVER_HTTPS_HPP

#include "server_http.hpp"
#include "asio/ssl.hpp"
#include <openssl/ssl.h>
#include <algorithm>

namespace webpp {
	using HTTPS = asio::ssl::stream<asio::ip::tcp::socket>;

	template<>
	class Server<HTTPS> : public ServerBase<HTTPS> {
		std::string session_id_context;
		bool set_session_id_context = false;
	public:
		Server(unsigned short port, size_t thread_pool_size, const std::string& cert_file, const std::string& private_key_file,
			long timeout_request = 5, long timeout_content = 300,
			const std::string& verify_file = std::string()) :
				Server(cert_file, private_key_file, verify_file) {
			 m_config.port=port;
			 m_config.thread_pool_size= thread_pool_size;
			 m_config.timeout_request=timeout_request;
			 m_config.timeout_content=timeout_content;
		 }

		Server(const std::string& cert_file, const std::string& private_key_file, const std::string& verify_file = std::string()) :
			ServerBase<HTTPS>::ServerBase(443), context(asio::ssl::context::tlsv12) {

			context.use_certificate_chain_file(cert_file);
			context.use_private_key_file(private_key_file, asio::ssl::context::pem);

			if (verify_file.size() > 0) {
				context.load_verify_file(verify_file);
				context.set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert |
					asio::ssl::verify_client_once);
				set_session_id_context = true;
			}
		}
		void start() override {
			if (set_session_id_context) {
								// Creating session_id_context from address:port but reversed due to small SSL_MAX_SSL_SESSION_ID_LENGTH
					session_id_context = std::to_string(m_config.port) + ':';
				session_id_context.append(m_config.address.rbegin(), m_config.address.rend());
				SSL_CTX_set_session_id_context(context.native_handle(), reinterpret_cast<const unsigned char*>(session_id_context.data()),
					std::min<size_t>(session_id_context.size(), SSL_MAX_SSL_SESSION_ID_LENGTH));

			}
			ServerBase::start();
		}

	protected:
		asio::ssl::context context;

		void accept() override {
			//Create new socket for this connection
			//Shared_ptr is used to pass temporary objects to the asynchronous functions
			auto socket = std::make_shared<HTTPS>(*m_io_context, context);

			acceptor->async_accept((*socket).lowest_layer(), [this, socket](const std::error_code& ec) {
				//Immediately start accepting a new connection (if io_context hasn't been stopped)
				if (ec != asio::error::operation_aborted)
					accept();


				if(!ec) {
					asio::ip::tcp::no_delay option(true);
					socket->lowest_layer().set_option(option);

					//Set timeout on the following asio::ssl::stream::async_handshake
					auto timer = get_timeout_timer(socket, m_config.timeout_request);
					socket->async_handshake(asio::ssl::stream_base::server, [this, socket, timer]
							(const std::error_code& ec) {
						if(timer)
							timer->cancel();
						if(!ec)
							read_request_and_content(socket);
						else if(on_error)
							on_error(std::shared_ptr<Request>(new Request(*socket)), ec);
					});
				}
				else if(on_error)
					on_error(std::shared_ptr<Request>(new Request(*socket)), ec);
			});
		}
	};

	class https_server : public Server<HTTPS> {
	public:
		explicit https_server(const std::string& cert_file, const std::string& private_key_file, const std::string& verify_file = std::string()) : Server<HTTPS>::Server(cert_file, private_key_file, verify_file) {}
	};

}


#endif  /* SERVER_HTTPS_HPP */

