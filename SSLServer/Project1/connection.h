#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <array>
#include <memory>
//#include "reply.hpp"
//#include "request.hpp"
//#include "request_handler.hpp"
//#include "request_parser.hpp"


namespace http {
    namespace server2 {

        class connection_manager;

//        /// Represents a single connection from a client.
        class connection : public std::enable_shared_from_this<connection>
        {
        public:
            connection(const connection&) = delete;
            connection& operator=(const connection&) = delete;

            /// Construct a connection with the given socket.
            explicit connection(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> ssl_socket
                , connection_manager& mngr
//                ,request_handler& handler
 );
//
            /// Start the first asynchronous operation for the connection.
            void start();

        private:
            
            bool on_peer_verify(bool preverified, boost::asio::ssl::verify_context& ctx);

            ///  Perform handshake
            void do_handshake();

            /// Perform an asynchronous read operation.
            void do_read();
//
            /// Perform an asynchronous write operation.
            void do_write();
//
            /// Socket for the connection.
            std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> ssl_socket_;
//
//            /// The handler used to process the incoming request.
//            request_handler& request_handler_;
//
            /// Buffer for incoming data.
            std::array<char, 8192> buffer_;
//
//            /// The incoming request.
//            request request_;
//
//            /// The parser for the incoming request.
//            request_parser request_parser_;
//
//            /// The reply to be sent back to the client.
//            reply reply_;

            /// The manager for this connection.
            connection_manager& connection_manager_;

            std::string endpoints_details;
            std::string response =
R"(HTTP/1.1 200 OK
Content - Type: text / plain
Content - Length : 13

Hello, World!
)";

        };

        typedef std::shared_ptr<connection> connection_ptr;

    } // namespace server2
} // namespace http

