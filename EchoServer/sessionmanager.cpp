#include "sessionmanager.h"


//void sessionmanager::add_session(std::shared_ptr<session> session_to_add)
//{
//	std::lock_guard<std::mutex> lock(mutex_);
//	sessions_.insert(SessionMap::value_type(session_to_add->getindex(), session_to_add));
//}

//void sessionmanager::remove_session(int session_idx)
//{
//	std::lock_guard<std::mutex> lock(mutex_);
//	auto it = sessions_.find(session_idx);
//	if (it != sessions_.end())
//	{
//		sessions_.erase(it);
//	}
//}

//void sessionmanager::broadcast(packetheader* header)
//{
//	if (header == nullptr)
//	{
//		return;
//	}
//
//	{
//		std::lock_guard<std::mutex> lock(mutex_);
//		for (auto& it : sessions_)
//		{
//			it.second->do_write(header);
//			++send_count_per_sec_;
//		}
//	}
//}

//std::shared_ptr<session> sessionmanager::get_session(int session_idx)
//{
//	std::lock_guard<std::mutex> lock(mutex_);
//	auto it = sessions_.find(session_idx);
//	if (it != sessions_.end())
//	{
//		return it->second;
//	}
//
//	return std::shared_ptr<session>();
//}

//size_t sessionmanager::get_session_count()
//{
//	std::lock_guard<std::mutex> lock(mutex_);
//
//	return sessions_.size();
//}

//void sessionmanager::init_send_count_per_sec()
//{
//	send_count_per_sec_ = 0;
//}

//void sessionmanager::release()
//{
//	std::lock_guard<std::mutex> lock(mutex_);
//	sessions_.clear();
//}