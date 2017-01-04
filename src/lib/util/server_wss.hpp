// license:MIT
// copyright-holders:Ole Christian Eidheim, Miodrag Milanovic
#ifndef SERVER_WSS_HPP
#define SERVER_WSS_HPP

#include "server_ws.hpp"
#include "asio/ssl.hpp"
#include <openssl/ssl.h>
#include <algorithm>

namespace webpp {
	using WSS = asio::ssl::stream<asio::ip::tcp::socket>;

	template<>
	class SocketServer<WSS> : public SocketServerBase<WSS> {
		std::string session_id_context;
		bool set_session_id_context = false;
	public:
		SocketServer(const std::string& cert_file, const std::string& private_key_file,
					 const std::string& verify_file=std::string()) : SocketServerBase<WSS>(443), context(asio::ssl::context::tlsv12) {

			context.use_certificate_chain_file(cert_file);
			context.use_private_key_file(private_key_file, asio::ssl::context::pem);

			if (verify_file.size() > 0) {
				context.load_verify_file(verify_file);
				context.set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert |
										 asio::ssl::verify_client_once);
				set_session_id_context=true;
			}
		}

		void start() override {
			if(set_session_id_context) {
				// Creating session_id_context from address:port but reversed due to small SSL_MAX_SSL_SESSION_ID_LENGTH
				session_id_context=std::to_string(config.port)+':';
				session_id_context.append(config.address.rbegin(), config.address.rend());
				SSL_CTX_set_session_id_context(context.native_handle(), reinterpret_cast<const unsigned char*>(session_id_context.data()),
											   std::min<size_t>(session_id_context.size(), SSL_MAX_SSL_SESSION_ID_LENGTH));
			}
			SocketServerBase::start();
		  }
	protected:
		asio::ssl::context context;

		void accept() override {
			//Create new socket for this connection (stored in Connection::socket)
			//Shared_ptr is used to pass temporary objects to the asynchronous functions
			std::shared_ptr<Connection> connection(new Connection(new WSS(*io_context, context)));

			acceptor->async_accept(connection->socket->lowest_layer(), [this, connection](const std::error_code& ec) {
				//Immediately start accepting a new connection (if io_context hasn't been stopped)
				if (ec != asio::error::operation_aborted)
					accept();

				if(!ec) {
					asio::ip::tcp::no_delay option(true);
					connection->socket->lowest_layer().set_option(option);

					//Set timeout on the following asio::ssl::stream::async_handshake
					auto timer = get_timeout_timer(connection, config.timeout_request);
					connection->socket->async_handshake(asio::ssl::stream_base::server,
							[this, connection, timer](const std::error_code& ec) {
						if(timer)
							timer->cancel();
						if(!ec)
							read_handshake(connection);
					});
				}
			});
		}
	};
}


#endif  /* SERVER_WSS_HPP */

