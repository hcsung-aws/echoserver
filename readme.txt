1. This code had been tested with:

Windows 7 Ultimate K with Service Pack 1 (64 bit)
Visual Studio Community 2017 -> 2022

2. Dependencies:

boost 1.78.0 with 64 bit multithread build

3. Directory structure:

[D: --- work --- vs ]-+- boost_1_67_0 -+- bin.v2
                                       +- boost
                                       +- doc
                                       +- lib64-msvc-14.1
                                       +- ...
                      +- EchoServer   -+- EchoClient
                                       +- EchoServer
                                       +- x64 (binaries)
                                       +- EchoServer.sln
                      +- gsl
                      +- gtest

4. Usage

EchoClient [<host> <port>] * n (ex: EchoClient 127.0.0.1 1238 127.0.0.1 1239 ...)
EchoServer <port> (ex: EchoServer 1238)


5. General idea

I wanted to make just simple echo server that works like real game server.

* I copied boost sample echo server & client to start easy. I did never use boost::asio yet so I wanted to start with working code.
(client: https://www.boost.org/doc/libs/1_47_0/doc/html/boost_asio/example/echo/blocking_tcp_echo_client.cpp)
(server: https://www.boost.org/doc/libs/1_67_0/doc/html/boost_asio/using.html)
* It receives streams and parses it to many single packets.
* It has simple packet handler and programmer can add their packet handlers.
* It has simple session manager to broadcast packets and manage sessions.
* It uses async recv/send function to recv/send packet. it could be useless in this kind of simple server - but we should use async recv/send function in a real game server development.

