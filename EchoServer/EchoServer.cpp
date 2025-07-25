//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#define _CRT_SECURE_NO_WARNINGS

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <thread>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <boost/asio.hpp>

#include "sessionmanager.h"
#include "server.h"

using boost::asio::ip::tcp;


//sessionmanager	g_manager;

template<typename _T>
bool on_recv_echo(_T& manager, session* recv_session, packetheader* header)
{
	echopacket* packet = reinterpret_cast<echopacket*>(header);
	if (recv_session == nullptr || packet == nullptr)
	{
		std::cerr << "recv_echo failed" << "\n";
		return false;
	}

	recv_session->do_write(packet);
	manager.increase_send_count_per_sec();

	return true;
}

class gamesessionmanager : public usersessionmanager<session>
{
public:
};

int main(int argc, char* argv[])
{
	try
	{
		if (argc != 2)
		{
			std::cerr << "Usage: EchoServer <port>\n";
			return 1;
		}

		boost::asio::io_context io_context;
		usersessionmanager<session> manager;

		realserver simple_server(io_context, tcp::endpoint(tcp::v4(), std::atoi(argv[1])), manager);

		simple_server.get_packethandler().add_handler(packettype::echo_type, on_recv_echo<usersessionmanager<session> >);

		simple_server.do_accept();

		// multi workerthread
		std::thread thread1{ [&io_context]() { io_context.run(); } };
		std::thread thread2{ [&io_context]() { io_context.run(); } };
		std::thread thread3{ [&io_context]() { io_context.run(); } };
		std::thread thread4{ [&io_context]() { io_context.run(); } };
		std::thread thread5{ [&io_context]() { io_context.run(); } };
		std::thread thread6{ [&io_context]() { io_context.run(); } };
		std::thread thread7{ [&io_context]() { io_context.run(); } };
		std::thread thread8{ [&io_context]() { io_context.run(); } };
		std::thread thread9{ [&io_context]() { io_context.run(); } };
		std::thread thread10{ [&io_context]() { io_context.run(); } };
		std::thread thread11{ [&io_context]() { io_context.run(); } };
		std::thread thread12{ [&io_context]() { io_context.run(); } };
		std::thread thread13{ [&io_context]() { io_context.run(); } };
		std::thread thread14{ [&io_context]() { io_context.run(); } };
		std::thread thread15{ [&io_context]() { io_context.run(); } };
		std::thread thread16{ [&io_context]() { io_context.run(); } };
		std::thread thread17{ [&io_context]() { io_context.run(); } };
		std::thread thread18{ [&io_context]() { io_context.run(); } };
		std::thread thread19{ [&io_context]() { io_context.run(); } };
		std::thread thread20{ [&io_context]() { io_context.run(); } };

		// calc send count per second - not exactly but simple.
		// if it has main logic thread, this function can be placed in there.
		std::thread thread_init_send_count
		{
			[&]()
			{
				int log_count = 0;
				while (true)
				{
					std::this_thread::sleep_for(std::chrono::seconds(1));
					++log_count;
					//if (log_count % 5 == 0)
					{
						auto now = std::chrono::system_clock::now();
						auto now_c = std::chrono::system_clock::to_time_t(now);
						auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
							now.time_since_epoch()) % 1000;
						//std::cout << "session count: " << simple_server.get_session_count() << ", send count per sec: " << simple_server.get_send_count_per_sec() << "\n";
						std::cout << std::put_time(std::localtime(&now_c), "[%Y-%m-%d %H:%M:%S.")
							<< std::setfill('0') << std::setw(3) << ms.count() << "] "
							<< "session count: " << manager.get_session_count()
							<< ", send count per sec: " << manager.get_send_count_per_sec() << "\n";
					}

					manager.init_send_count_per_sec();
				}
			}
		};

		// status log
		while (true)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		thread1.join();
		thread2.join();
		thread3.join();
		thread4.join();
		thread5.join();
		thread6.join();
		thread7.join();
		thread8.join();
		thread9.join();
		thread10.join();
		thread11.join();
		thread12.join();
		thread13.join();
		thread14.join();
		thread15.join();
		thread16.join();
		thread17.join();
		thread18.join();
		thread19.join();
		thread20.join();
		thread_init_send_count.join();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
