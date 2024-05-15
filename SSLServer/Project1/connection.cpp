#include "connection.h"
#include <utility>
#include <iostream>
#include <sstream>
//#include "request_handler.hpp"

namespace ba = boost::asio;

namespace http {
    namespace server2 {

        connection::connection(std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> ssl_socket
            , connection_manager& mngr
        )
            : ssl_socket_(ssl_socket)
            , connection_manager_(mngr)
        {
            // ssl verification and alert also depends how the client is configured
            // 
            // if 
            // .\openssl.exe s_client -connect 127.0.0.1:5688 -verify 1 -verify_return_error
            // will return error on handshake because client will send Alert that unknown (self-signed ) certificate
            //



            ssl_socket_->set_verify_mode(boost::asio::ssl::verify_none);
            //ssl_socket_->set_verify_mode(boost::asio::ssl::verify_peer | ba::ssl::verify_fail_if_no_peer_cert);
            ssl_socket_->set_verify_callback([this](bool preverified, ba::ssl::verify_context& context)->bool {	
                return on_peer_verify(preverified, context); 
                });
            boost::system::error_code ec;
            auto loc_ep = ssl_socket->lowest_layer().local_endpoint(ec);
            if (ec)
            {
                std::cout << "[" << std::this_thread::get_id() << "] conn[" << this << "]:"
                    << "Error: local ep:" << ec.message().c_str() << std::endl;
            }
            ec.clear();
            auto rem_ep = ssl_socket->lowest_layer().remote_endpoint(ec);
            if (ec)
            {
                std::cout << "[" << std::this_thread::get_id() << "] conn[" << this << "]:"
                    << "Error: local ep:" << ec.message().c_str() << std::endl;
            }

            std::stringstream ss;
            
            ss  << loc_ep.address().to_string().c_str() << ":" << loc_ep.port() << " <-- "
                << rem_ep.address().to_string().c_str() << ":" << rem_ep.port() ;
            endpoints_details = ss.str();

            std::cout << "[" << std::this_thread::get_id() << "] conn[" << this << "]:"
                <<endpoints_details << std::endl;
        }


        bool connection::on_peer_verify(bool preverified, ba::ssl::verify_context& ctx)
        {
            
            // The verify callback can be used to check whether the certificate that is
            // being presented is valid for the peer. For example, RFC 2818 describes
            // the steps involved in doing this for HTTPS. Consult the OpenSSL
            // documentation for more details. Note that the callback is called once
            // for each certificate in the certificate chain, starting from the root
            // certificate authority.

            // print the certificate's subject name.
            char subject_name[256];
            X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
            X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
            //trace(String::Format(__FUNCTION__" verifying %s", subject_name));

            // print the certificate's serial number.
            ASN1_INTEGER* serial = X509_get_serialNumber(cert);
            BIGNUM* bnser = ASN1_INTEGER_to_BN(serial, NULL);
            char* asciiHex = BN_bn2hex(bnser);
            //trace(String::Format(__FUNCTION__" SN: %s", asciiHex));

            //SSL_get_verify_result(m_ssl_stream->native_handle());
            //SSL_get_peer_certificate(ctx.native_handle());

            return preverified;
        }


        void connection::start()
        {
            do_handshake();
        }

        void connection::do_handshake()
        {
            auto self(shared_from_this());
            ssl_socket_->async_handshake(boost::asio::ssl::stream_base::handshake_type::server, 
                [this, self](boost::system::error_code const& ec)
            {
                    std::cout << "[" << std::this_thread::get_id() << "] conn[" << this << "]:"
                        << "shake: " << endpoints_details << std::endl;

                if (!ec) 
                {
                    //do_read();
                    //do_write();
                }
                else
                {
                    std::cout << "[" << std::this_thread::get_id() << "] conn[" << this << "]:"
                        << "Error: shake:" << ec.message().c_str() << " " << endpoints_details << std::endl;
                }

                do_read();

            });
        }


        void connection::do_read()
        {
            std::cout << "[" << std::this_thread::get_id() << "] conn[" << this << "]:" << "do_read: " << endpoints_details << std::endl;

            auto self(shared_from_this());
            ssl_socket_->async_read_some(boost::asio::buffer(buffer_),
                [this, self](boost::system::error_code ec, std::size_t bytes_received)
                {
                    if (!ec)
                    {

                        std::string s(buffer_.data(), bytes_received);
                        std::cout << "[" << std::this_thread::get_id() << "] conn[" << this << "]:"
                            << "on read: received: " << bytes_received << " bytes [" << s << "]" << std::endl;

                        do_write();
                        //request_parser::result_type result;
                        //std::tie(result, std::ignore) = request_parser_.parse(
                        //    request_, buffer_.data(), buffer_.data() + bytes_transferred);
                        //
                        //if (result == request_parser::good)
                        //{
                        //    request_handler_.handle_request(request_, reply_);
                        //    do_write();
                        //}
                        //else if (result == request_parser::bad)
                        //{
                        //    reply_ = reply::stock_reply(reply::bad_request);
                        //    do_write();
                        //}
                        //else
                        //{
                        //    do_read();
                        //}
                    }
                    else
                    {
                        std::cout << "[" << std::this_thread::get_id() << "] conn[" << this << "]:"
                            << "Error: on read:" << ec.message().c_str() << std::endl;
                    }

                    // If an error occurs then no new asynchronous operations are
                    // started. This means that all shared_ptr references to the
                    // connection object will disappear and the object will be
                    // destroyed automatically after this handler returns. The
                    // connection class's destructor closes the socket.
                });
        }
//
        void connection::do_write()
        {
            auto self(shared_from_this());

            ba::async_write(*ssl_socket_.get(), ba::buffer(response), 
                [this, self](boost::system::error_code ec, size_t bytes_transmitted) { 
                    //on_write(ec, bytes_transmitted);	
                    if (ec)
                    {
                        std::cout << "[" << std::this_thread::get_id() << "] conn[" << this << "]:"
                            << "Error: on write:" << ec.message().c_str() << std::endl;
                    }
                    else
                    {
//                        do_read();
                                    boost::system::error_code ignored_ec;
            ssl_socket_->lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both,  ignored_ec);

                    }

                });
            //ssl_socket_->asyn->asyncwrite boost::asio::async_write(socket_, reply_.to_buffers(),
            //    [this, self](boost::system::error_code ec, std::size_t)
            //    {
            //        if (!ec)
            //        {
            //            // Initiate graceful connection closure.
            //            boost::system::error_code ignored_ec;
            //            socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
            //                ignored_ec);
            //        }

            //        // No new asynchronous operations are started. This means that
            //        // all shared_ptr references to the connection object will
            //        // disappear and the object will be destroyed automatically after
            //        // this handler returns. The connection class's destructor closes
            //        // the socket.
            //    });
        }

    } // namespace server2
} // namespace http