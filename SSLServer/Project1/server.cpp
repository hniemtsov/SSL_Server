//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2023 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "server.h"
#include <signal.h>
#include <utility>
#include <iostream>
//#include "connection.hpp"

namespace ba = boost::asio;

namespace http {
    namespace server2 {


        //class A {
        //public:
        //    explicit A(int i) : i_(i) {}
        //    ~A() {
        //        i_ = -10;
        //    }
        //    A(const A& rhs) = delete;
        //    A& operator=(const A& rhs) = delete;
        //    A(A&& rhs) noexcept {
        //        i_ = rhs.i_;
        //        rhs.i_ = 0;
        //    }
        //    A& operator=(A&& rhs) noexcept {
        //        i_ = rhs.i_;
        //        rhs.i_ = -1;
        //        return *this;
        //    }

        //    int i_;
        //};


        struct Impl {
            
        };

        //server::server() : m_pImpl(std::make_unique<Impl>()) {}
        server::~server(){}

        server::server(const std::string& address, const std::string& port,
            const std::string& doc_root, std::size_t io_context_pool_size)
            : m_pImpl(std::make_unique<Impl>())
            , m_ssl_ctx(std::make_unique<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12_server))
            , io_context_pool_(io_context_pool_size)
            //signals_(io_context_pool_.get_io_context()),
            , acceptor_(io_context_pool_.get_io_context())
            //request_handler_(doc_root)
        {
            boost::system::error_code ec;
            m_ssl_ctx->use_certificate_file("server.pem", boost::asio::ssl::context::pem,ec);
            if (ec)
                std::cout << "[" << std::this_thread::get_id() << "] serv[" << this << "]:"
                << "Error: use cert:" << ec.message().c_str() << std::endl;
            ec.clear();

            m_ssl_ctx->use_private_key_file("server.key", boost::asio::ssl::context::pem,ec);
            if (ec)
                std::cout << "[" << std::this_thread::get_id() << "] serv[" << this << "]:"
                << "Error: use key :" << ec.message().c_str() << std::endl;


            m_ssl_ctx->set_options(ba::ssl::context::default_workarounds |
                ba::ssl::context::no_sslv2 |
                ba::ssl::context::no_sslv3 |
                ba::ssl::context::single_dh_use);

            //m_ssl_ctx->set_verify_mode(boost::asio::ssl::verify_none);
            //m_ssl_ctx->set_verify_mode(boost::asio::ssl::verify_peer | boost::asio::ssl::verify_fail_if_no_peer_cert);
            //m_ssl_ctx->set_verify_callback([this](bool preverified, ba::ssl::verify_context& context)->bool {
            //    return true;// on_peer_verify(preverified, context);
            //    });


            // Register to handle the signals that indicate when the server should exit.
            // It is safe to register for the same signal multiple times in a program,
            // provided all registration for the specified signal is made through Asio.
           // signals_.add(SIGINT);
           // signals_.add(SIGTERM);
#if defined(SIGQUIT)
            signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)

            do_await_stop();

            // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
            boost::asio::ip::tcp::resolver resolver(acceptor_.get_executor());
            boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(address, port).begin();
            acceptor_.open(endpoint.protocol());
            //acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true)); DWN???
            acceptor_.bind(endpoint);
            acceptor_.listen();

            do_accept();
        }

        void server::run()
        {
            io_context_pool_.run();
        }

        void server::do_accept()
        {
            auto ssl_stream = std::make_shared<ba::ssl::stream<ba::ip::tcp::socket>>(io_context_pool_.get_io_context() , *m_ssl_ctx);
            //ssl_stream->set_verify_mode(boost::asio::ssl::verify_none);


            std::cout << "[" << std::this_thread::get_id() << "] serv[" << this << std::endl;


            acceptor_.async_accept(ssl_stream->lowest_layer()
                                  ,[ ssl_stream, this ](boost::system::error_code ec) mutable {

                    std::cout << "[" << std::this_thread::get_id() << "] serv[" << this << "]: on accepted" << std::endl;

                    if (!acceptor_.is_open())
                    {
                        return;
                    }

                    if (!ec)
                    {
                        connection_manager_.start(std::make_shared<connection>(ssl_stream, connection_manager_/*, request_handler_*/));
                    }
                    else
                    {
                        std::cout << "[" << std::this_thread::get_id() << "] serv[" << this << "]: on accepted: Error: " << ec.message().c_str() << std::endl;
                    }

                    do_accept();
                });
        }

        void server::do_await_stop()
        {
         //   signals_.async_wait(
         //       [this](boost::system::error_code /*ec*/, int /*signo*/)
         //       {
         //        //   io_context_pool_.stop();
         //       });
        }

    } // namespace server2
} // namespace http