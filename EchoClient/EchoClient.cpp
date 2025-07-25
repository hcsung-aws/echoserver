//
// blocking_tcp_echo_client.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <functional>
#include <thread>
#include <chrono>
#include <boost/asio.hpp>
#include <iomanip>
#include <ctime>
#include <algorithm>

#include "packethandler.h"

using boost::asio::ip::tcp;

int g_recv_count = 0;
int g_recv_divider = 1;
std::atomic<size_t> g_connection_count = 0, g_total_recv_count_per_sec = 0, g_total_latency_per_sec_ms = 0;

bool on_recv_echo(packetheader* header)
{
	echopacket* packet = reinterpret_cast<echopacket*>(header);
	if (packet == nullptr)
	{
		return false;
	}

	//std::cout << "Reply is: ";
	//std::cout.write(packet->message_, sizeof(char) * strlen(packet->message_));
	//std::cout << "\n";
	++g_recv_count;
	++g_total_recv_count_per_sec;
	//if (g_recv_count % g_recv_divider == 0)
	//{
	//	std::cout << "recv count:" << g_recv_count << "\n";
	//	if (g_recv_divider < 1000)
	//	{
	//		g_recv_divider *= 10;
	//	}
	//}

	return true;
}

std::chrono::system_clock::time_point parsepacket(packethandler& handler, char* recv_buffer, char* temp_buffer, size_t& recv_offset, size_t& recv_size, size_t length)
{
	if (recv_size + length > max_recv_buffer_size)
	{
		// buffer overflow - can't handle packet
		std::cerr << "buffer overflow!" << "\n";
		return std::chrono::system_clock::now();
	}
	if (recv_offset + recv_size + length > max_recv_buffer_size)
	{
		// hit buffer end - move valid bufffer data to front
		char swap_buffer[max_recv_buffer_size] = { 0, };
		memcpy(swap_buffer, recv_buffer + recv_offset, recv_size);
		memcpy(recv_buffer, swap_buffer, recv_size);
		recv_offset = 0;
	}
	memcpy(recv_buffer + recv_offset + recv_size, temp_buffer, length);
	recv_size += length;

	while (true)
	{
		packetheader* header = reinterpret_cast<packetheader*>(recv_buffer + recv_offset);
		if (header == nullptr)
		{
			 break;
		}
		if (header->size_ > recv_size)
		{
			// recv again to recv full packet
			break;
		}
		if (handler.handle_packet(header) == false)
		{
			std::cerr << "packet handle error" << "\n";
			return std::chrono::system_clock::now();
		}
		recv_offset += header->size_;
		recv_size -= header->size_;
		if (recv_size == 0)
		{
			// init offset if empty
			recv_offset = 0;
			break;
		}
	}

	return std::chrono::system_clock::now();
}

bool try_connect(tcp::socket& s, tcp::resolver& resolver, const std::string& ip, const std::string& port)
{
	boost::system::error_code ec_connect;
	while (true)
	{
		boost::asio::connect(s, resolver.resolve(ip, port), ec_connect);
		if (!ec_connect)
		{
			std::cout << "connect success" << "\n";
			break;
		}
		else if ((boost::asio::error::eof == ec_connect) ||
			(boost::asio::error::connection_reset == ec_connect) ||
			(boost::asio::error::connection_refused == ec_connect))
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
			std::cerr << "reconnect failed. retry after 1 second..." << "\n";
		}
		else
		{
			// An error occurred.
			std::cerr << "error occurred[" << ec_connect.message() << "]\n";
			return false;
		}
	}

	s.set_option(tcp::no_delay(true));
	s.set_option(boost::asio::socket_base::linger(true, 0));
	s.set_option(boost::asio::socket_base::reuse_address(true));

	return true;
}

struct connect_info
{
	std::string ip_;
	std::string port_;
};

connect_info choose_connectinfo(int argc, char* argv[])
{
	std::vector<connect_info>	connections;
	for (int i = 1; i < argc; i += 2)
	{
		connect_info	info;
		info.ip_ = argv[i];
		info.port_ = argv[i + 1];
		connections.push_back(info);
	}
	if (connections.empty())
	{
		connect_info dummy;
		dummy.ip_ = "127.0.0.1";
		dummy.port_ = "1238";
		return dummy;
	}

	return connections[rand()%connections.size()];
}

void do_write(tcp::socket& s, std::chrono::system_clock::time_point& last_send_time)
{
	while (true)
	{
		echopacket echo;
		sprintf_s(echo.message_, "hahaha");
		size_t request_length = std::strlen(echo.message_);
		if (s.is_open() == true)
		{
			boost::system::error_code ec;
			size_t send_size = boost::asio::write(s, boost::asio::buffer(&echo, echo.size_), ec);
			if (!ec)
			{
				//send success;
			}
		}
		last_send_time = std::chrono::system_clock::now();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

// main 함수의 매개변수 처리
struct test_config {
	std::string ip_;
	std::string port_;
	int connection_count_;
};

test_config parse_args(int argc, char* argv[]) {
	test_config config;
	if (argc < 4) {
		std::cerr << "Usage: " << argv[0] << " <host> <port> <connection_count>" << std::endl;
		exit(1);
	}
	config.ip_ = argv[1];
	config.port_ = argv[2];
	config.connection_count_ = std::max(1, std::min(std::atoi(argv[3]), 5000));
	return config;
}

int main(int argc, char* argv[]) {
	test_config config = parse_args(argc, argv);
	boost::asio::io_context io_context;
	std::vector<std::thread> client_threads;
	std::atomic<bool> is_live{ true };

	std::thread thread_init_recv_count
	{
		[&]()
		{
			while (is_live)
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				auto now = std::chrono::system_clock::now();
				auto now_c = std::chrono::system_clock::to_time_t(now);
				auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
					now.time_since_epoch()) % 1000;

				struct tm time_info;
				localtime_s(&time_info, &now_c);

				//std::cout << "session count: " << simple_server.get_session_count() << ", send count per sec: " << simple_server.get_send_count_per_sec() << "\n";
				std::cout << std::put_time(&time_info, "[%Y-%m-%d %H:%M:%S.")
					<< std::setfill('0') << std::setw(3) << ms.count() << "] "
					<< "session count: " << g_connection_count
					<< ", recv count per sec: " << g_total_recv_count_per_sec
					<< ", latency per sec: " << g_total_latency_per_sec_ms / (g_total_recv_count_per_sec == 0 ? (1) : (int)g_total_recv_count_per_sec)
					<< "\n";

				g_total_recv_count_per_sec = 0;
				g_total_latency_per_sec_ms = 0;
			}
		}
	};

	// connection_count 만큼 클라이언트 생성
	for (int i = 0; i < config.connection_count_; ++i) {
		client_threads.emplace_back([&io_context, &config, &is_live, i]() {
			try {
				tcp::socket s(io_context);
				tcp::resolver resolver(io_context);
				packethandler handler;
				handler.add_handler(packettype::echo_type, on_recv_echo);

				if (!try_connect(s, resolver, config.ip_, config.port_)) {
					std::cerr << "Connection failed for client " << i << std::endl;
					return;
				}

				++g_connection_count;

				// latency check 용도
				auto last_send_time = std::chrono::system_clock::now();

				// 수신 스레드
				std::thread recv_thread{ [&]() {
					char recv_buffer[max_recv_buffer_size] = {0,};
					char temp_buffer[max_packet_length] = {0,};
					size_t recv_offset = 0, recv_size = 0;

					while (is_live) {
						boost::system::error_code ec;
						size_t length = boost::asio::read(s,
							boost::asio::buffer(temp_buffer, max_packet_length),
							boost::asio::transfer_at_least(sizeof(packetheader)),
							ec);

						if (!ec) {
							auto last_recv_time = parsepacket(handler, recv_buffer, temp_buffer,
									  recv_offset, recv_size, length);
							g_total_latency_per_sec_ms += std::chrono::duration_cast<std::chrono::milliseconds>(
								last_recv_time - last_send_time).count();
						}
						else if ((boost::asio::error::eof == ec) ||
								(boost::asio::error::connection_reset == ec)) {
							std::cerr << "Client " << i << " disconnected\n";
							--g_connection_count;
							if (!try_connect(s, resolver, config.ip_, config.port_)) {
								break;
							}
						}
					}
				} };

				// 송신 부분
				do_write(s, last_send_time);
				recv_thread.join();
			}
			catch (std::exception& e) {
				std::cerr << "Client " << i << " exception: " << e.what() << "\n";
			}
			});
	}

	// 모든 클라이언트 스레드 종료 대기
	for (auto& thread : client_threads) {
		thread.join();
	}

	return 0;
}

/*
int main(int argc, char* argv[])
{
	try
	{
		if (argc < 3)
		{
			std::cerr << "Usage: EchoClient [<host> <port>] * n\n";
			return 1;
		}

		srand((unsigned int)time(nullptr));

		packethandler	handler;
		handler.add_handler(packettype::echo_type, on_recv_echo);

		boost::asio::io_context io_context;

		tcp::socket s(io_context);
		tcp::resolver resolver(io_context);
		connect_info conn_info = choose_connectinfo(argc, argv);
		try_connect(s, resolver, conn_info.ip_, conn_info.port_);

		//recv thread
		bool is_live = true;
		std::thread recv_thread
		{ [&io_context, &s, &is_live, &handler, &resolver, &conn_info]()
			{
				char recv_buffer[max_recv_buffer_size] = { 0, }, temp_buffer[max_packet_length] = { 0, };
				size_t recv_offset = 0, recv_size = 0;
				while (is_live == true)
				{
					boost::system::error_code ec;
					size_t length = boost::asio::read(s, boost::asio::buffer(temp_buffer, max_packet_length),
						boost::asio::transfer_at_least(sizeof(packetheader)), ec);

					if (!ec)
					{
						parsepacket(handler, recv_buffer, temp_buffer, recv_offset, recv_size, length);
					}
					else if ((boost::asio::error::eof == ec) ||
						(boost::asio::error::connection_reset == ec))
					{
						// disconnect
						// try reconnect
						std::cerr << "disconnected by peer. try reconnect." << "\n";
						if (try_connect(s, resolver, conn_info.ip_, conn_info.port_) == false)
						{
							is_live = false;
							return;
						}
					}
				}
			}
		};

		// send
		do_write(s);

		recv_thread.join();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
*/