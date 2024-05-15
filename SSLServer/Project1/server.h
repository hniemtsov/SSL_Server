#pragma once
//
// server.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2023 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//


#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <string>
#include "io_context_pool.h"
//#include "connection_manager.h"
//#include "request_handler.hpp"
#include <memory>
#include <set>
#include "connection.h"
#include "connection_manager.h"
namespace http {
    namespace server2 {

        struct Impl;

        /// The top-level class of the HTTP server.
        class server
        {
        public:
            /// Construct the server to listen on the specified TCP address and port, and
            /// serve up files from the given directory.
            explicit server(const std::string& address, const std::string& port,
                const std::string& doc_root, std::size_t io_context_pool_size);

            //server();
            ~server();

            /// Run the server's io_context loop.
            void run();

        private:
           std::unique_ptr<Impl> m_pImpl;
            /// Perform an asynchronous accept operation.
            void do_accept();

            /// Wait for a request to stop the server.
            void do_await_stop();

            /// The pool of io_context objects used to perform asynchronous operations.
            io_context_pool io_context_pool_;

            /// The signal_set is used to register for process termination notifications.
            //boost::asio::signal_set signals_;

            /// Acceptor used to listen for incoming connections.
           boost::asio::ip::tcp::acceptor acceptor_;

            /// The handler for all incoming requests.
            //request_handler request_handler_;
            
           /// The connection manager which owns all live connections.
           connection_manager connection_manager_;
            
            std::unique_ptr<boost::asio::ssl::context> m_ssl_ctx;
            std::set<connection_ptr> t;
        };

    } // namespace server2
} // namespace http
