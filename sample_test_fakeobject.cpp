TEST(handle_login_packet)
{
	somehandler handler;

	loginpacket packet;
	//blah blah...
	// packet.accountname = "hahaha";
	// ...

	//handler uses fakesession
	fakesession session_used;
	handler.handle_login_packet(packet);

	login_answer_packet* sendedpacket = session_used.getsendedpacket();

	// test has been passed or not
	ASSERT_EQUAL(sendedpacket.getaccoutname() == "hahaha");
}

class somehandler
{
public:
	void handle_login_packet(session* session_recved, loginpacket& packet)	// session_recved is fakesession
	{
		// blah ...

		login_answer_packet answer_packet;
		answer_packet.setaccountname(packet.getaccountname());

		session_recved->do_write(&answer_packet);
	}
};

class fakesession
{
private:
	login_answer_packet* sendedpacket_;

public:
	void do_write(packet* sendedpacket) override
	{
		sendedpacket_ = (login_answer_packet*)sendedpacket;
	}

	login_answer_packet* getsendedpacket() { return sendedpacket_; }
};