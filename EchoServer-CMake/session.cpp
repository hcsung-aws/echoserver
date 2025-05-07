#include <iostream>

#include "session.h"


std::atomic<int>	session_idx(1);

session::session(tcp::socket socket, std::function<void(int)> disconnect_handler)
	: socket_(std::move(socket))
	, index_(session_idx++)
	, disconnect_handler_(disconnect_handler)
{
	recv_buffer_ = (char*)calloc(sizeof(char), max_recv_buffer_size);
}
session::~session()
{
	free(recv_buffer_);
}

bool session::parse_buffer(size_t length)
{
	if (recv_size_ + length > max_recv_buffer_size)
	{
		// buffer overflow - can't handle packet
		std::cerr << "buffer overflow!" << "\n";
		return false;
	}
	if (recv_offset_ + recv_size_ + length > max_recv_buffer_size)
	{
		// hit buffer end - move data to front of recv_buffer_
		char* swap_buffer = (char*)calloc(sizeof(char), recv_size_);
		if (swap_buffer == nullptr)
		{
			std::cerr << "swap buffer alloc failed!" << "\n";
			return false;
		}
		memcpy(swap_buffer, recv_buffer_ + recv_offset_, recv_size_);
		memcpy(recv_buffer_, swap_buffer, recv_size_);
		free(swap_buffer);
		recv_offset_ = 0;
	}
	memcpy(recv_buffer_ + recv_offset_ + recv_size_, temp_buffer, length);
	recv_size_ += length;

	return true;
}

void session::do_close()
{
	socket_.close();
}

void session::do_read(std::function<void(char*, size_t)> recv_handler)
{
	auto self(shared_from_this());
	socket_.async_read_some(boost::asio::buffer(temp_buffer, max_packet_length),
		[=, &self](boost::system::error_code ec, std::size_t length)
	{
		if (!ec)
		{
			parse_buffer(length);
			recv_handler(recv_buffer_ + recv_offset_, recv_size_);
			do_read(recv_handler);
		}
		else if ((boost::asio::error::eof == ec) ||
			(boost::asio::error::connection_reset == ec))
		{
			disconnect_handler_(index_);
		}
	});
}

void session::do_write(packetheader* header)
{
	if (header == nullptr)
	{
		return;
	}

	auto self(shared_from_this());
	boost::asio::async_write(socket_, boost::asio::buffer(header, header->size_),
		[this, self](boost::system::error_code ec, std::size_t /*length*/)
	{
		if (!ec)
		{
			// if there is sendbuffer and send several packets at the same time(cache), should call do_write here to finish sending sendbuffer.
		}
	});
}
