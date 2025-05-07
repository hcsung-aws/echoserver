#include <iostream>

#include "server.h"

//void tcpserver::parsepacket(std::shared_ptr<session> session_recved, char* recv_buffer, size_t length)
//{
//	size_t recv_offset = 0;
//	while (true)
//	{
//		packetheader* header = reinterpret_cast<packetheader*>(recv_buffer + recv_offset);
//		if (header == nullptr)
//		{
//			break;
//		}
//		if (header->size_ > length)
//		{
//			// recv again to recv full packet
//			break;
//		}
//		if (handler_.handle_packet(session_recved.get(), header) == false)
//		{
//			std::cerr << "packet handle error" << "\n";
//			session_recved->do_close();
//			return;
//		}
//		recv_offset += header->size_;
//		length -= header->size_;
//
//		// init offset if empty - try not to it buffer end frequently
//		if (length == 0)
//		{
//			recv_offset = 0;
//			break;
//		}
//	}
//}

//void realserver::do_accept()
//{
//	acceptor_.async_accept(
//		[&](boost::system::error_code ec, tcp::socket socket)
//	{
//		if (!ec)
//		{
//			std::shared_ptr<session> session_accepted = std::make_shared<session>(std::move(socket), &handler_, [&](int session_idx)
//			{
//				disconnect_handler(session_idx);
//			});
//			//session_accepted->start();
//			session_accepted->do_read(std::bind(&tcpserver::parsepacket, this, session_accepted, std::placeholders::_1, std::placeholders::_2));
//			manager_.add_session(session_accepted);
//			//std::cout << "accepted" << "\n";
//		}
//
//		do_accept();
//	});
//}

//void realserver::run()
//{
//	io_context_.run();
//}