#include "CSession.h"
#include <iostream>
#include "CServer.h"

using namespace std;

void CSession::Send(char* msg, int max_length)
{
	bool pending = false; // pending为true表示上次的数据没有发完；pending为false表示数据已经发完了。
	std::lock_guard<std::mutex> lock(_send_lock);
	if (_send_que.size() > 0)
	{
		pending = true;
	}
	_send_que.push(make_shared<MsgNode>(msg, max_length));
	if (pending)
	{
		return;
	}
	boost::asio::async_write(_socket, boost::asio::buffer(msg, max_length),
		std::bind(&CSession::handle_write, this, std::placeholders::_1, shared_from_this()));
}
void CSession::Start()
{
	memset(_data, 0, max_length);
	_socket.async_read_some(boost::asio::buffer(_data, max_length),
		std::bind(&CSession::handle_read, this, placeholders::_1, placeholders::_2, shared_from_this())); // 最后的参数不能用shared_ptr<Session>(this)生成智能指针，会生成两个智能指针，两个的引用计数是不同的；会造成两个智能指针同时管理一块内存的操作。必须要用shared_from_this()函数，这样才是同一个指针指针。
}

void CSession::handle_read(const boost::system::error_code& error, size_t bytes_transferred, shared_ptr<CSession> _self_shared)
{
	if (!error)
	{
		cout << "server receive data is " << _data << endl;
		Send(_data, bytes_transferred);
		memset(_data, 0, max_length);
		_socket.async_read_some(boost::asio::buffer(_data, max_length),
			std::bind(&CSession::handle_read, this, placeholders::_1, placeholders::_2, _self_shared));

		/*boost::asio::async_write(_socket, boost::asio::buffer(_data, bytes_transferred),
			std::bind(&CSession::handle_write, this, placeholders::_1, _self_shared));*/ // bind也可以用lambda函数替代：[this](const auto& error){ handle_write(error); };
		// async_write和async_send的区别：
		// async_send是成员函数，直接调用。async_write是全局函数，把socket作为参数传入
	}
	else
	{
		cout << "read error" << endl; // 对端关闭也会触发读的回调函数。
		//delete this; // 销毁Session，连接断开
		_server->ClearSession(_uuid);
	}
}

std::string& CSession::GetUuid()
{
	return _uuid;
}

void CSession::handle_write(const boost::system::error_code& error, shared_ptr<CSession> _self_shared)
{
	if (!error)
	{
		std::lock_guard<std::mutex> lock(_send_lock);
		_send_que.pop();
		if (!_send_que.empty())
		{
			auto& msgnode = _send_que.front();
			boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_max_len),
				std::bind(&CSession::handle_write, this, std::placeholders::_1, _self_shared));
		}
	}
	else
	{
		cout << "handle write error failed " << error.value() << ", error is" << error.what() << endl;
		//delete this;
		_server->ClearSession(_uuid);
	}
}

