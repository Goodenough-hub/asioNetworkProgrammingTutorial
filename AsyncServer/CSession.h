#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <map>
#include <memory>
#include <boost/uuid/uuid_generators.hpp> // 生成uuid
#include <boost/uuid/uuid_io.hpp> // uuid的头文件
#include <queue>

using boost::asio::ip::tcp;

class CServer;

class MsgNode
{
	friend class CSession;
public:
	MsgNode(char* msg, int max_len)
	{
		_data = new char[max_len];
		memcpy(_data, msg, max_len);
	}
	~MsgNode()
	{
		delete[] _data;
	}
private:
	int _cur_len;
	int _max_len;
	char* _data;
};

class CSession: public std::enable_shared_from_this<CSession>
{
public:
	CSession(boost::asio::io_context& ioc, CServer* server) : _socket(ioc), _server(server) {
		boost::uuids::uuid a_uuid = boost::uuids::random_generator()(); // random_generator()是一个函数对象，重载了operator的操作()()是一个函数
		_uuid = boost::uuids::to_string(a_uuid);
	}
	tcp::socket& Socket() {
		return _socket;
	}

	~CSession()
	{
		std::cout << "session destruct delete this" << this << std::endl;
	}

	void Start(); // 监听
	std::string& GetUuid();

	void Send(char* msg, int max_length);

private:
	// 两个回调函数
	void handle_read(const boost::system::error_code& error, size_t bytes_transferred, std::shared_ptr<CSession> _self_shared); // 第一个参数时错误码，第二个参数时实际读了多少。内部调用的是async_read_some()
	void handle_write(const boost::system::error_code& error, std::shared_ptr<CSession> _self_shared); // 内部调用的是async_send()/async_write()可以一次全部发完，所以只用一个参数

	tcp::socket _socket;
	enum {max_length = 1024};
	char _data[max_length];
	CServer* _server;
	std::string _uuid;

	std::queue<std::shared_ptr<MsgNode>> _send_que;
	std::mutex _send_lock;
};



