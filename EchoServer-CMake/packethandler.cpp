#include <iostream>

#include "packethandler.h"


//void packethandler::add_handler(packettype handlertype, std::function<bool(session*, packetheader*)> handlefunc)
//{
//	handlers_.insert(HandlerMap::value_type(handlertype, handlefunc));
//}
//
//bool packethandler::handle_packet(session* recv_session, packetheader* header)
//{
//	if (header == nullptr)
//	{
//		std::cerr << "invalid header" << "\n";
//		return false;
//	}
//
//	auto it = handlers_.find(header->type_);
//	if (it == handlers_.end())
//	{
//		std::cerr << "no handler to handle packet[" << (int)header->type_ << "]\n";
//		return false;
//	}
//
//	if (it->second(recv_session, header) == false)
//	{
//		//false handler
//		std::cerr << "packet handler failed" << "\n";
//		return false;
//	}
//
//	return true;
//}
