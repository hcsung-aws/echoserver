#pragma once

#include <atomic>
#include <mutex>
#include <map>

#include "packet.h"
//#include "session.h"

template<typename _SESSION>
class sessionmanager
{
private:
	std::mutex mutex_;
	typedef std::map<int, std::shared_ptr<_SESSION> >	SessionMap;
	SessionMap	sessions_;
	std::atomic<size_t> send_count_per_sec_{ 0 };

public:
	void add_session(std::shared_ptr<_SESSION> session_to_add)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		sessions_.insert(SessionMap::value_type(session_to_add->getindex(), session_to_add));
	}
	void remove_session(int session_idx)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		auto it = sessions_.find(session_idx);
		if (it != sessions_.end())
		{
			sessions_.erase(it);
		}
	}

	void onaccept(std::shared_ptr<_SESSION> session)
	{

	}

	std::shared_ptr<_SESSION> get_session(int session_idx)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		auto it = sessions_.find(session_idx);
		if (it != sessions_.end())
		{
			return it->second;
		}

		return std::shared_ptr<_SESSION>();
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

	void release()
	{
		std::lock_guard<std::mutex> lock(mutex_);
		sessions_.clear();
	}
};
