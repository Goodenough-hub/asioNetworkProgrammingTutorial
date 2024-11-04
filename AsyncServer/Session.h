#pragma once
#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class Session
{
public:
	Session(boost::asio::io_context& ioc) : _socket(ioc) {

	}
	tcp::socket& Socket() {
		return _socket;
	}

	void Start(); // 监听

private:
	// 两个回调函数
	void handle_read(const boost::system::error_code& error, size_t bytes_transferred); // 第一个参数时错误码，第二个参数时实际读了多少。内部调用的是async_read_some()
	void handle_write(const boost::system::error_code& error); // 内部调用的是async_send()/async_write()可以一次全部发完，所以只用一个参数

	tcp::socket _socket;
	enum {max_length = 1024};
	char _data[max_length];
};

class Server {
public:
	Server(boost::asio::io_context& ioc, short port);
private:
	void start_accept(); // 用来监听的，创建一个acceptor
	void handle_accept(Session* new_session, const boost::system::error_code& error); // 当对方有连接连过来时，会触发这个回调函数
	boost::asio::io_context& _ioc; // io_context不允许做复制，构造操作，所以设置成一个引用的成员变量
	tcp::acceptor _acceptor;
};

