#pragma once
#include <memory>
#include "boost/asio.hpp"
#include <iostream>
#include <queue>

using namespace std;
using namespace boost;

const int RECVSIZE = 1024;

class MsgNode {
public:
	// 发送节点的构造方式
	MsgNode(const char* msg, int total_len) :_total_len(total_len), _cur_len(0) {
		_msg = new char[total_len];
		memcpy(_msg, msg, total_len);
	}
	// 接收节点的构造方式
	MsgNode(int total_len) :_total_len(total_len), _cur_len(0) {
		_msg = new char[total_len];
	}

	~MsgNode() {
		delete[] _msg;
	}

	int _total_len;
	int _cur_len;
	char* _msg;
};
class Session
{
public:
	Session(std::shared_ptr<asio::ip::tcp::socket> socket);
	void Connect(const asio::ip::tcp::endpoint& ep);
	void WriteCallBackErr(const boost::system::error_code& ec, std::size_t bytes_transferred,
		std::shared_ptr<MsgNode> send_node); // 第三个参数是保证sendnode节点的存活性，作为参数指针指针的引用计数会加一
	void WriteToSocketErr(const std::string buf);

	void WriteCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred,
		std::shared_ptr<MsgNode> send_node); // 第三个参数是保证sendnode节点的存活性，作为参数指针指针的引用计数会加一
	void WriteToSocket(const std::string buf);

	void WriteAllToSocket(const std::string& buf);
	void WriteAllCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred);

	// 异步读
	void ReadFromSocket();
	void ReadCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred);

	void ReadAllFromSocket();
	void ReadAllCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred);
private:
	std::shared_ptr<asio::ip::tcp::socket> _socket;
	std::shared_ptr<MsgNode> _send_node;

	std::queue<std::shared_ptr<MsgNode>> _send_queue;
	bool _send_pending;
	// _send_pending 表示单个数据是否是发送的状态
	// _send_queue.empty()表示队列中是否有其他数据

	std::shared_ptr<MsgNode> _recv_node;
	bool _recv_pending;
};

