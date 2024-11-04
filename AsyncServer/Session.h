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

	void Start(); // ����

private:
	// �����ص�����
	void handle_read(const boost::system::error_code& error, size_t bytes_transferred); // ��һ������ʱ�����룬�ڶ�������ʱʵ�ʶ��˶��١��ڲ����õ���async_read_some()
	void handle_write(const boost::system::error_code& error); // �ڲ����õ���async_send()/async_write()����һ��ȫ�����꣬����ֻ��һ������

	tcp::socket _socket;
	enum {max_length = 1024};
	char _data[max_length];
};

class Server {
public:
	Server(boost::asio::io_context& ioc, short port);
private:
	void start_accept(); // ���������ģ�����һ��acceptor
	void handle_accept(Session* new_session, const boost::system::error_code& error); // ���Է�������������ʱ���ᴥ������ص�����
	boost::asio::io_context& _ioc; // io_context�����������ƣ�����������������ó�һ�����õĳ�Ա����
	tcp::acceptor _acceptor;
};

