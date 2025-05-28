#pragma once

#include <atomic>
#include <mutex>
#include <map>
#include <functional>

#include "packet.h"

template <typename _SESSION>
class usersessionmanager
{
public:
	virtual ~usersessionmanager() = default;
private:
	std::mutex mutex_;
	typedef std::map<int, std::weak_ptr<_SESSION> >	SessionMap;
	SessionMap	sessions_;
	std::atomic<size_t> send_count_per_sec_{ 0 };

public:
	void onaccept(std::shared_ptr<_SESSION> session)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		sessions_.insert(SessionMap::value_type(session->getindex(), session));
	}
	void ondisconnect(int session_idx)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		auto it = sessions_.find(session_idx);
		if (it != sessions_.end())
		{
			sessions_.erase(it);
		}
	}

	void broadcast(packetheader* header)
	{
		if (header == nullptr)
		{
			return;
		}

		{
			std::lock_guard<std::mutex> lock(mutex_);
			for (auto& it : sessions_)
			{
				std::shared_ptr<_SESSION> session = it.second.lock();
				if (session)
				{
					session->do_write(header);
					++send_count_per_sec_;
				}
			}
		}
	}

	std::weak_ptr<_SESSION> get_session(int session_idx)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		auto it = sessions_.find(session_idx);
		if (it != sessions_.end())
		{
			return it->second;
		}

		return std::weak_ptr<_SESSION>();
	}

	size_t get_session_count()
	{
		std::lock_guard<std::mutex> lock(mutex_);

		return sessions_.size();
	}

	void init_send_count_per_sec()
	{
		send_count_per_sec_ = 0;
	}

	size_t get_send_count_per_sec() { return send_count_per_sec_; }
	void increase_send_count_per_sec() { ++send_count_per_sec_; }

	void release()
	{
		std::lock_guard<std::mutex> lock(mutex_);
		sessions_.clear();
	}
};

template<typename _T, typename _SESSIONMGR>
class packethandler
{
private:
	typedef std::map<packettype, std::function<bool(_SESSIONMGR&, _T*, packetheader*)> >	HandlerMap;

	HandlerMap	handlers_;
	_SESSIONMGR& manager_;
public:
	packethandler(_SESSIONMGR& manager)
		: manager_(manager)
	{}

	void onaccept(std::shared_ptr<_T> session_accepted)
	{
		manager_.onaccept(session_accepted);
	}
	void ondisconnect(int session_idx)
	{
		manager_.ondisconnect(session_idx);
	}

	virtual void parsepacket(std::shared_ptr<_T> session_recved, char* recv_buffer, size_t length)
	{
		size_t recv_offset = 0;
		while (true)
		{
			packetheader* header = reinterpret_cast<packetheader*>(recv_buffer + recv_offset);
			if (header == nullptr)
			{
				break;
			}
			if (header->size_ > length)
			{
				// recv again to recv full packet
				break;
			}
			if (handle_packet(session_recved.get(), header) == false)
			{
				std::cerr << "packet handle error" << "\n";
				session_recved->do_close();
				return;
			}
			recv_offset += header->size_;
			length -= header->size_;

			// init offset if empty - try not to it buffer end frequently
			if (length == 0)
			{
				recv_offset = 0;
				break;
			}
		}
	}

	//void add_handler(packettype handlertype, std::function<bool(sessionmanager<_T>&, +T*, packetheader*)> handlefunc);
	//bool handle_packet(_T* recv_session, packetheader* header);
	void add_handler(packettype handlertype, std::function<bool(_SESSIONMGR&, _T*, packetheader*)> handlefunc)
	{
		handlers_.insert(HandlerMap::value_type(handlertype, handlefunc));
	}

	bool handle_packet(_T* recv_session, packetheader* header)
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

		if (it->second(manager_, recv_session, header) == false)
		{
			//false handler
			std::cerr << "packet handler failed" << "\n";
			return false;
		}

		return true;
	}

	void release()
	{
		handlers_.clear();
	}

	size_t get_send_count_per_sec() { return manager_.get_send_count_per_sec(); }

	// delete empty constructor & copy constructor(would like to use the class with RAII concept)
	packethandler() = delete;
	packethandler(const packethandler&) = delete;
};
