#pragma once

#include <map>
#include <functional>

#include "packet.h"



class packethandler
{
private:
	typedef std::map<packettype, std::function<bool(packetheader*)> >	HandlerMap;

	HandlerMap	handlers_;
public:
	void add_handler(packettype handlertype, std::function<bool(packetheader*)> handlefunc)
	{
		handlers_.insert(HandlerMap::value_type(handlertype, handlefunc));
	}

	bool handle_packet(packetheader* header)
	{
		if (header == nullptr)
		{
			std::cerr << "invalid header" << "\n";
			return false;
		}

		auto it = handlers_.find(header->type_);
		if (it == handlers_.end())
		{
			std::cerr << "no handler to handle packet[" << (int)header->type_ << "]\n";
			return false;
		}

		if (it->second(header) == false)
		{
			//false handler
			std::cerr << "packet handler failed" << "\n";
			return false;
		}

		return true;
	}
};
