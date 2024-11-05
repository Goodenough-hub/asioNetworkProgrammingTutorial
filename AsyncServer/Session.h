#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <map>
#include <memory>
#include <boost/uuid/uuid_generators.hpp> // 生成uuid
#include <boost/uuid/uuid_io.hpp> // uuid的头文件

using boost::asio::ip::tcp;

class Server;
class Session: public std::enable_shared_from_this<Session>
{
public:
	Session(boost::asio::io_context& ioc, Server* server) : _socket(ioc), _server(server) {
		boost::uuids::uuid a_uuid = boost::uuids::random_generator()(); // random_generator()是一个函数对象，重载了operator的操作()()是一个函数
		_uuid = boost::uuids::to_string(a_uuid);
	}
	tcp::socket& Socket() {
		return _socket;
	}

	~Session()
	{
		std::cout << "session destruct delete this" << this << endl;
	}

	void Start(); // 监听
	std::string& GetUuid();

private:
	// 两个回调函数
	void handle_read(const boost::system::error_code& error, size_t bytes_transferred, std::shared_ptr<Session> _self_shared); // 第一个参数时错误码，第二个参数时实际读了多少。内部调用的是async_read_some()
	void handle_write(const boost::system::error_code& error, std::shared_ptr<Session> _self_shared); // 内部调用的是async_send()/async_write()可以一次全部发完，所以只用一个参数

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
	void start_accept(); // 用来监听的，创建一个acceptor
	void handle_accept(std::shared_ptr<Session> new_session, const boost::system::error_code& error); // 当对方有连接连过来时，会触发这个回调函数
	boost::asio::io_context& _ioc; // io_context不允许做复制，构造操作，所以设置成一个引用的成员变量
	tcp::acceptor _acceptor;
	std::map<std::string, std::shared_ptr<Session>> _sessions; // 用map来管理session，就需要唯一id。雪花算法、uuid
};

