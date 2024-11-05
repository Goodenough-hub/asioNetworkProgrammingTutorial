#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <map>
#include <memory>
#include <boost/uuid/uuid_generators.hpp> // ����uuid
#include <boost/uuid/uuid_io.hpp> // uuid��ͷ�ļ�

using boost::asio::ip::tcp;

class Server;
class Session: public std::enable_shared_from_this<Session>
{
public:
	Session(boost::asio::io_context& ioc, Server* server) : _socket(ioc), _server(server) {
		boost::uuids::uuid a_uuid = boost::uuids::random_generator()(); // random_generator()��һ����������������operator�Ĳ���()()��һ������
		_uuid = boost::uuids::to_string(a_uuid);
	}
	tcp::socket& Socket() {
		return _socket;
	}

	~Session()
	{
		std::cout << "session destruct delete this" << this << endl;
	}

	void Start(); // ����
	std::string& GetUuid();

private:
	// �����ص�����
	void handle_read(const boost::system::error_code& error, size_t bytes_transferred, std::shared_ptr<Session> _self_shared); // ��һ������ʱ�����룬�ڶ�������ʱʵ�ʶ��˶��١��ڲ����õ���async_read_some()
	void handle_write(const boost::system::error_code& error, std::shared_ptr<Session> _self_shared); // �ڲ����õ���async_send()/async_write()����һ��ȫ�����꣬����ֻ��һ������

	tcp::socket _socket;
	enum {max_length = 1024};
	char _data[max_length];
	Server* _server;
	std::string _uuid;
};

class Server {
public:
	Server(boost::asio::io_context& ioc, short port);
	void ClearSession(std::string uuid);
private:
	void start_accept(); // ���������ģ�����һ��acceptor
	void handle_accept(std::shared_ptr<Session> new_session, const boost::system::error_code& error); // ���Է�������������ʱ���ᴥ������ص�����
	boost::asio::io_context& _ioc; // io_context�����������ƣ�����������������ó�һ�����õĳ�Ա����
	tcp::acceptor _acceptor;
	std::map<std::string, std::shared_ptr<Session>> _sessions; // ��map������session������ҪΨһid��ѩ���㷨��uuid
};

