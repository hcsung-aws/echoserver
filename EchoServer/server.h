#pragma once

#include "sessionmanager.h"
#include "session.h"
#include "packethandler.h"

using boost::asio::ip::tcp;

template<typename _SESSION>
class udpserver
{
	//different codes
};

template<typename _SESSION, typename _SESSIONMGR>
class tcpserver
{
protected:
	sessionmanager<_SESSION> manager_;
	packethandler<_SESSION, _SESSIONMGR> handler_;
public:
	tcpserver(boost::asio::io_context& io_context, tcp::endpoint endpoint, _SESSIONMGR& manager)
		: handler_(manager)
	{}
	virtual ~tcpserver() = default;

	virtual void do_accept() {}
	virtual void disconnect_handler(int session_idx) { manager_.remove_session(session_idx); }
	//virtual void run() {}

	std::shared_ptr<_SESSION> get_session(int session_idx) { return manager_.get_session(session_idx); }
	size_t get_session_count() { return manager_.get_session_count(); }
	void init_send_count_per_sec() { manager_.init_send_count_per_sec(); }
	size_t get_send_count_per_sec() { return manager_.get_send_count_per_sec(); }
	packethandler<_SESSION, _SESSIONMGR>& get_packethandler() { return handler_; }

	// delete empty constructor & copy constructor(would like to use the class with RAII concept)
	tcpserver() = delete;
	tcpserver(const tcpserver&) = delete;
};

class realserver : public tcpserver<session, usersessionmanager<session> >
{
protected:
	tcp::acceptor acceptor_;
public:
	realserver(boost::asio::io_context& io_context, tcp::endpoint&& endpoint, usersessionmanager<session>& manager)
		: tcpserver(io_context, endpoint, manager)
		, acceptor_(io_context, endpoint)
	{
		acceptor_.set_option(tcp::no_delay(true));
		acceptor_.set_option(boost::asio::socket_base::linger(true, 0));
		acceptor_.set_option(boost::asio::socket_base::reuse_address(true));
	}

	virtual void do_accept() override
	{
		acceptor_.async_accept(
			[&](boost::system::error_code ec, tcp::socket socket)
		{
			if (!ec)
			{
				std::shared_ptr<session> session_accepted = std::make_shared<session>(std::move(socket), [&](int session_idx)
				{
					disconnect_handler(session_idx);
				});
				handler_.onaccept(session_accepted);
				//session_accepted->start();
				session_accepted->do_read(std::bind(&packethandler<session, usersessionmanager<session> >::parsepacket, &handler_, session_accepted, std::placeholders::_1, std::placeholders::_2));
				manager_.add_session(session_accepted);
				//std::cout << "accepted" << "\n";
			}

			do_accept();
		});
	}
	virtual void disconnect_handler(int session_idx) override
	{
		handler_.ondisconnect(session_idx);
		manager_.remove_session(session_idx);
	}
	//virtual void run();
};
