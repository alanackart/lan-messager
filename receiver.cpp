//
// receiver.cpp
// ~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "boost/bind.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <iterator>
#include <map>
#include <string>

const short multicast_port = 30001;

class receiver
{
public:
    receiver(boost::asio::io_service &io_service,
             const boost::asio::ip::address &listen_address,
             const boost::asio::ip::address &multicast_address)
        : socket_(io_service)
    {
        // Create the socket so that multiple may be bound to the same address.
        boost::asio::ip::udp::endpoint listen_endpoint(
            listen_address, multicast_port);
        socket_.open(listen_endpoint.protocol());
        socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
        socket_.bind(listen_endpoint);

        // Join the multicast group.
        socket_.set_option(
            boost::asio::ip::multicast::join_group(multicast_address));

        socket_.async_receive_from(

            boost::asio::buffer(data_, max_length), sender_endpoint_,
            boost::bind(&receiver::handle_receive_from, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
    }


    void handle_receive_from(const boost::system::error_code &error,
                             size_t bytes_recvd)
    {
        if (!error)
        {
            std::string msg_recv(data_, bytes_recvd);
            // std::cout.write(data_, bytes_recvd);
            // std::cout << std::endl;

            std::string ip_str = sender_endpoint_.address().to_string();
            std::cout << msg_recv << " " << ip_str << std::endl;

            // std::ostringstream os;
            // os << "echo " << msg_recv << " " << ip_str;
            // std::string message_ = os.str();
            users[ip_str] = msg_recv;
            std::cout << "map is" << to_string(users) << std::endl;
            socket_.async_receive_from(
                boost::asio::buffer(data_, max_length), sender_endpoint_,
                boost::bind(&receiver::handle_receive_from, this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
            socket_.send_to(boost::asio::buffer(to_string(users)), sender_endpoint_);
        }
    }

private:
    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint sender_endpoint_;
    std::map<std::string, std::string> users;
    enum
    {
        max_length = 1024
    };
    char data_[max_length];

    std::string to_string(std::map<std::string, std::string> m)
    {
        std::string result;
        std::string obj;
        for (auto it = m.cbegin(); it != m.cend(); ++it)
        {
            obj = "[" + it->first + ":" + it->second + "],";
            // std::cout << obj << std::endl;
            result += obj;
        }
//        std::cout << result << std::endl;
        return result;
    }
};

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 3)
        {
            std::cerr << "Usage: receiver <listen_address> <multicast_address>\n";
            std::cerr << "  For IPv4, try:\n";
            std::cerr << "    receiver 0.0.0.0 239.255.0.1\n";
            std::cerr << "  For IPv6, try:\n";
            std::cerr << "    receiver 0::0 ff31::8000:1234\n";
            return 1;
        }

        boost::asio::io_service io_service;
        receiver r(io_service,
                   boost::asio::ip::address::from_string(argv[1]),
                   boost::asio::ip::address::from_string(argv[2]));
        io_service.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
