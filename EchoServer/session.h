#pragma once

//#include <atomic>
#include <memory>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

#include "sessionmanager.h"

//class udpsession
//	: public std::enable_shared_from_this<session>
//{
//	udpsession(udp::socket socket, packethandler<session>* handler, std::function<void(int)> disconnect_handler);
//};
//
//sessionmanager<udpsession>
//packethandler<udpsession>

class session
	: public std::enable_shared_from_this<session>
{
public:
	session(tcp::socket socket, std::function<void(int)> disconnect_handler);
	virtual ~session();

	const int getindex() const { return index_; }

	// delete empty constructor & copy constructor(would like to use the class with RAII concept)
	session() = delete;
	session(const session&) = delete;

protected:
	bool parse_buffer(size_t length);

public:
	virtual void do_read(std::function<void(char*, size_t)> recv_handler);
	virtual void do_write(packetheader* header);
	virtual void do_close();

protected:
	tcp::socket socket_;
	char temp_buffer[max_packet_length] = { 0, };
	// simple recv buffer that stores recved stream before parse
	// recv_buffer_structure
	// |---|====================|----------------------------------------------|
	// 0  recv_offset          recv_offset + recv_size(length of =)           max_recv_buffer_size - 1
	// we can use circular buffer to do the same thing
	char* recv_buffer_ = nullptr;
	size_t recv_offset_ = 0;
	size_t recv_size_ = 0;
	int	index_ = 0;
	std::function<void(int)> disconnect_handler_;
};
