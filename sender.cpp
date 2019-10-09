//
// sender.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "boost/bind.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <sstream>
#include <string>

class sender
{
public:
    sender(boost::asio::io_service &io_service,
           const boost::asio::ip::address &multicast_address, const short &multicast_port, const std::string &name)
        : endpoint_(multicast_address, multicast_port),
          socket_(io_service, endpoint_.protocol()),
          timer_(io_service),
          name_(name)
    {
        std::ostringstream os;
        os << "I'm " << name_ << " online @ " << getTime();
        message_ = os.str();
        std::cout << "sending " << message_ << std::endl;

        socket_.async_send_to(
            boost::asio::buffer(message_), endpoint_,
            boost::bind(&sender::handle_send_to, this,
                        boost::asio::placeholders::error));
    }

    void handle_send_to(const boost::system::error_code &error)
    {
        char buff[1024] = {0};
        if (!error)
        {
            socket_.receive_from(boost::asio::buffer(buff), endpoint_);
            std::cout << "server says " << std::string(buff) << std::endl;
            timer_.expires_from_now(boost::posix_time::seconds(20));

            timer_.async_wait(
                boost::bind(&sender::handle_timeout, this,
                            boost::asio::placeholders::error));
        }
    }

    void handle_timeout(const boost::system::error_code &error)
    {
        if (!error)
        {
            std::ostringstream os;
            os << "I'm " << name_ << " online @ " << getTime();
            message_ = os.str();
            std::cout << "sending " << message_ << std::endl;

            socket_.async_send_to(
                boost::asio::buffer(message_), endpoint_,
                boost::bind(&sender::handle_send_to, this,
                            boost::asio::placeholders::error));
        }
        /*useless code, when receiver is down, no error is encountered, udp protocol?*/
        else
        {
            std::cout << "error=" << error.value() << std::endl;
        }
    }

    std::string getTime()
    {
        char sys_time[15];
        time_t t;
        struct tm *local;
        struct timeval tv;
        t = time(NULL);
        gettimeofday(&tv, NULL);
        local = localtime(&t);
        sprintf(sys_time, "%02d:%02d:%02d.%03ld", local->tm_hour, local->tm_min, local->tm_sec, tv.tv_usec / 1000);
        return std::string(sys_time);
    }

private:
    boost::asio::ip::udp::endpoint endpoint_;
    boost::asio::ip::udp::socket socket_;
    boost::asio::deadline_timer timer_;
    std::string message_;
    const std::string name_;
};

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 4)
        {
            std::cerr << "Usage: sender <name> <multicast_ip> <multicast_port> \n";
            std::cerr << "  For IPv4, try:\n";
            std::cerr << "    sender bob 239.255.0.1  30001\n";
            // std::cerr << "  For IPv6, try:\n";
            // std::cerr << "    sender ff31::8000:1234\n";
            return 1;
        }

        boost::asio::io_service io_service;
        sender s(io_service, boost::asio::ip::address::from_string(argv[2]), atoi(argv[3]), argv[1]);
        io_service.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}