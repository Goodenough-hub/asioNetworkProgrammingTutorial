#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <map>
#include <memory>
#include <boost/uuid/uuid_generators.hpp> // ����uuid
#include <boost/uuid/uuid_io.hpp> // uuid��ͷ�ļ�
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
		boost::uuids::uuid a_uuid = boost::uuids::random_generator()(); // random_generator()��һ����������������operator�Ĳ���()()��һ������
		_uuid = boost::uuids::to_string(a_uuid);
	}
	tcp::socket& Socket() {
		return _socket;
	}

	~CSession()
	{
		std::cout << "session destruct delete this" << this << std::endl;
	}

	void Start(); // ����
	std::string& GetUuid();

	void Send(char* msg, int max_length);

private:
	// �����ص�����
	void handle_read(const boost::system::error_code& error, size_t bytes_transferred, std::shared_ptr<CSession> _self_shared); // ��һ������ʱ�����룬�ڶ�������ʱʵ�ʶ��˶��١��ڲ����õ���async_read_some()
	void handle_write(const boost::system::error_code& error, std::shared_ptr<CSession> _self_shared); // �ڲ����õ���async_send()/async_write()����һ��ȫ�����꣬����ֻ��һ������

	tcp::socket _socket;
	enum {max_length = 1024};
	char _data[max_length];
	CServer* _server;
	std::string _uuid;

	std::queue<std::shared_ptr<MsgNode>> _send_que;
	std::mutex _send_lock;
};



