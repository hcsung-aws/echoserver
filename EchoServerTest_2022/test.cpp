#include "pch.h"
/*
TEST(TestCaseName, TestName) {
  EXPECT_EQ(1, 1);
  EXPECT_TRUE(true);
}*/
//#include "pch.h"
#include "gtest/gtest.h"

#include <functional>
#include <memory>
#include <gsl/span>
#include "../EchoServer/sessionmanager.h"
#include "../EchoServer/server.h"

using boost::asio::ip::tcp;

//TEST(TestCaseName, TestName) {
//  EXPECT_EQ(1, 1);
//  EXPECT_TRUE(true);
//}

void disconn_handler_dummy(int)
{}

template<typename _T>
bool on_recv_echo_fake(usersessionmanager<_T>& manager, _T* recv_session, packetheader* header)
{
	echopacket* packet = reinterpret_cast<echopacket*>(header);
	if (recv_session == nullptr || packet == nullptr)
	{
		std::cerr << "recv_echo failed" << "\n";
		return false;
	}

	manager.broadcast(packet);

	return true;
}

using boost::asio::ip::tcp;

/*extern*/ std::atomic<int>	session_idx(1);

class fakesession
	: public std::enable_shared_from_this<fakesession>
{
private:
	packetheader* header_ = nullptr;
public:
	fakesession(tcp::socket socket, std::function<void(int)> disconnect_handler)
		: socket_(std::move(socket))
		, index_(session_idx++)
		, disconnect_handler_(disconnect_handler)
	{
		recv_buffer_ = (char*)calloc(sizeof(char), max_recv_buffer_size);
	}

	~fakesession()
	{
		free(recv_buffer_);
	}

	void add_dummypacket(packetheader* header) { header_ = header; }
	void do_read(std::function<void(char*, size_t)> recv_handler)
	{
		if (header_ == nullptr)
		{
			return;
		}

		memset(temp_buffer, 0, sizeof(temp_buffer));
		memcpy(temp_buffer, header_, header_->size_);
		parse_buffer(header_->size_);
		recv_handler(recv_buffer_ + recv_offset_, recv_size_);
		//do_read(recv_handler);
	}
	void do_write(packetheader* header)
	{
		header_ = header;
	}
	void do_close()
	{
		header_ = nullptr;
	}

	packetheader* getheader() { return header_; }

	const int getindex() const { return index_; }

	fakesession() = delete;
	fakesession(const fakesession&) = delete;

protected:
	bool parse_buffer(size_t length)
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

class fakeserver : public tcpserver<fakesession, usersessionmanager<fakesession> >
{
public:
	fakeserver(boost::asio::io_context& io_context, tcp::endpoint& endpoint, usersessionmanager<fakesession>& manager)
		: tcpserver(io_context, endpoint, manager)
	{}

	void do_accept() override
	{
		boost::asio::io_context io_context;

		std::shared_ptr<fakesession> created_session = std::make_shared<fakesession>(tcp::socket(io_context), disconn_handler_dummy);
		manager_.add_session(created_session);
	}

	packethandler<fakesession, usersessionmanager<fakesession> >& get_packethandler() { return handler_; }
};

void parsepacket_test(std::shared_ptr<fakesession> session_recved, packethandler<fakesession, usersessionmanager<fakesession> >& handler, char* recv_buffer, size_t length)
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
		if (handler.handle_packet(session_recved.get(), header) == false)
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

class HandlerTest : public ::testing::Test
{
protected:
	usersessionmanager<fakesession>	manager_;
	packethandler<fakesession, usersessionmanager<fakesession> >	handler_;
	std::shared_ptr <fakesession> created_session_;
	echopacket	testpacket_;
	std::string teststring_ = "ahaha";
	fakeserver server_;
	boost::asio::io_context io_context_;

protected:
	HandlerTest()
		: handler_(manager_)
		, server_(io_context_, tcp::endpoint(tcp::v4(), std::atoi("127.0.0.1")), manager_)
	{

	}
	virtual ~HandlerTest() = default;

	virtual void SetUp()
	{
		server_.get_packethandler().add_handler(packettype::echo_type, on_recv_echo_fake<fakesession>);
		server_.do_accept();
		EXPECT_EQ(server_.get_session_count(), 1);
		created_session_ = server_.get_session(1);
		EXPECT_NE(created_session_, nullptr);
		memcpy(testpacket_.message_, teststring_.c_str(), teststring_.size());
	}

	virtual void TearDown()
	{
		manager_.release();
		handler_.release();
	}
};

TEST_F(HandlerTest, recv_echo_function)
{
	//std::shared_ptr<fakesession> test_session = created_session_;
	//EXPECT_NE(test_session, nullptr);
	//test_session->add_dummypacket(&testpacket_);
	created_session_->add_dummypacket(&testpacket_);
	created_session_->do_read(std::bind(&packethandler<fakesession, usersessionmanager<fakesession> >::parsepacket/*parsepacket_test*/, &server_.get_packethandler(), created_session_, std::placeholders::_1, std::placeholders::_2));

	//echopacket* recvedpacket = (echopacket*)test_session->getheader();
	echopacket* recvedpacket = (echopacket*)created_session_->getheader();
	EXPECT_NE(recvedpacket, nullptr);
	EXPECT_EQ(teststring_, recvedpacket->message_);

}

//TEST_F(HandlerTest, do_accept_test)
//{
//	tcp::acceptor acceptor(io_cont_, tcp::endpoint(tcp::v4(), std::atoi("127.0.0.1")));
//	server_.do_accept(manager_, acceptor, handler_);
//	EXPECT_EQ(manager_.get_session_count(), 1);
//}