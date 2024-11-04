#include "Session.h"
#include <iostream>

using namespace std;

void Session::Start()
{
	memset(_data, 0, max_length);
	_socket.async_read_some(boost::asio::buffer(_data, max_length),
		std::bind(&Session::handle_read, this, placeholders::_1, placeholders::_2));
}

void Session::handle_read(const boost::system::error_code& error, size_t bytes_transferred)
{
	if (!error)
	{
		cout << "server receive data is " << _data << endl;

		boost::asio::async_write(_socket, boost::asio::buffer(_data, bytes_transferred),
			std::bind(&Session::handle_write, this, placeholders::_1)); // bindҲ������lambda���������[this](const auto& error){ handle_write(error); };
		// async_write��async_send������
		// async_send�ǳ�Ա������ֱ�ӵ��á�async_write��ȫ�ֺ�������socket��Ϊ��������
	}
	else
	{
		cout << "read error" << endl; // �Զ˹ر�Ҳ�ᴥ�����Ļص�������
		delete this; // ����Session�����ӶϿ�
	}
}

void Session::handle_write(const boost::system::error_code& error)
{
	if (!error)
	{
		memset(_data, 0, max_length);
		_socket.async_read_some(boost::asio::buffer(_data, max_length),
			std::bind(&Session::handle_read, this, placeholders::_1, placeholders::_2));
	}
	else
	{
		cout << "write error" << error.value() << endl;
		delete this;
	}
}

Server::Server(boost::asio::io_context& ioc, short port) :_ioc(ioc), _acceptor(ioc, tcp::endpoint(tcp::v4(), port)) {
	cout << "Server start sucess, on port: " << port << endl;
	start_accept();
}

void Server::start_accept()
{
	Session* new_session = new Session(_ioc);
	_acceptor.async_accept(new_session->Socket(), std::bind(&Server::handle_accept, this, new_session, placeholders::_1));

}

void Server::handle_accept(Session* new_session, const boost::system::error_code& error)
{
	if (!error)
	{
		new_session->Start();
	}
	else
	{
		delete new_session;
	}
	start_accept(); // ׼�������µĿͻ���
}