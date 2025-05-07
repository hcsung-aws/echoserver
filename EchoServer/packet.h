#pragma once

enum { max_message_length = 1024, max_packet_length = 2048, max_recv_buffer_size = 6000000 };

enum packettype
{
	header_type = 0,
	echo_type = 1,
};


struct packetheader
{
	int size_ = 0;	// include header size itself
	packettype type_ = packettype::header_type;
};

// |size|type|message|
struct echopacket : public packetheader
{
	char message_[max_message_length+1] = { 0, };
	echopacket()
	{
		size_ = sizeof(echopacket);
		type_ = packettype::echo_type;
	}
};
