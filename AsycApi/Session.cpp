#include "Session.h"

Session::Session(std::shared_ptr<asio::ip::tcp::socket> socket) :_socket(socket), _send_pending(false), _recv_pending(false) {

}

void Session::Connect(const asio::ip::tcp::endpoint& ep) {
	_socket->connect(ep);
}

void Session::WriteCallBackErr(const boost::system::error_code& ec, std::size_t bytes_transferred,
	std::shared_ptr<MsgNode> msg_node) // 第三个参数是保证sendnode节点的存活性，作为参数指针指针的引用计数会加一
{
	if (bytes_transferred + msg_node->_cur_len < msg_node->_total_len) // 没有发送完，继续发送。 `当前发送完成的长度 + 之前发送的长度 = 已经发送的总长度` 与要求发送的长度做对比
	{
		_send_node->_cur_len += bytes_transferred;
		this->_socket->async_write_some(asio::buffer(_send_node->_msg
			+ _send_node->_cur_len, _send_node->_total_len - _send_node->_cur_len), // 要更新偏移量
			std::bind(&Session::WriteCallBackErr, this, std::placeholders::_1, std::placeholders::_2, _send_node));
	}
}

void Session::WriteToSocketErr(const std::string buf)
{
	_send_node = make_shared<MsgNode>(buf.c_str(), buf.length());
	this->_socket->async_write_some(asio::buffer(_send_node->_msg, _send_node->_total_len),
		std::bind(&Session::WriteCallBackErr, this, std::placeholders::_1, std::placeholders::_2,
			_send_node)); // 写成功后就会调用这个回调函数
}

void Session::WriteCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred,
	std::shared_ptr<MsgNode> send_node) // 第三个参数是保证sendnode节点的存活性，作为参数指针指针的引用计数会加一
{
	if (ec.value() != 0)
	{
		std::cout << "Error, code is " << ec.value() << ". Message is " << ec.message();
		return;
	}

	auto& send_data = _send_queue.front();
	send_data->_cur_len += bytes_transferred;
	if (send_data->_cur_len < send_data->_total_len)
	{
		this->_socket->async_write_some(asio::buffer(send_data->_msg + send_data->_cur_len,
			send_data->_total_len - send_data->_cur_len),
			std::bind(&Session::WriteCallBack, this, std::placeholders::_1, std::placeholders::_2));
		return;
	}

	_send_queue.pop();

	if (_send_queue.empty())
	{
		_send_pending = false;
	}

	if (!_send_queue.empty())
	{
		auto& send_data = _send_queue.front();
		this->_socket->async_write_some(asio::buffer(send_data->_msg + _send_node->_cur_len,
			send_data->_total_len - send_data->_cur_len),
			std::bind(&Session::WriteCallBack, this, std::placeholders::_1, std::placeholders::_2));
	}
}
void Session::WriteToSocket(const std::string buf)
{
	_send_queue.emplace(new MsgNode(buf.c_str(), buf.length()));
	if (_send_pending) // 有未发送完的数据
	{
		return;
	}
	this->_socket->async_write_some(asio::buffer(buf),
		std::bind(&Session::WriteCallBack, this, std::placeholders::_1, std::placeholders::_2));

	_send_pending = true;
}

void Session::WriteAllToSocket(const std::string& buf)
{
	_send_queue.emplace(new MsgNode(buf.c_str(), buf.length()));
	if (_send_pending)
	{
		return;
	}

	this->_socket->async_send(asio::buffer(buf), std::bind(&Session::WriteAllCallBack, this, std::placeholders::_1, std::placeholders::_2)); // async_send多次调用async_write_some函数
	// 保证调用一次WriteAllCallBack

	_send_pending = true; // 发送的状态置为true
}
void Session::WriteAllCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	if (ec.value() != 0)
	{
		std::cout << "Error occured! Error code = " << ec.value() << ". Message:" << ec.value();
		return;
	}

	_send_queue.pop();
	if (_send_queue.empty())
	{
		_send_pending = false;
	}

	if (!_send_queue.empty()) // 队列中有数据
	{
		auto& send_data = _send_queue.front();
		this->_socket->async_send(asio::buffer(send_data->_msg + send_data->_cur_len, send_data->_total_len - send_data->_cur_len),
			std::bind(&Session::WriteAllCallBack, this, std::placeholders::_1, std::placeholders::_2));
	}
}

void Session::ReadFromSocket()
{
	if (_recv_pending)
	{
		return;
	}

	_recv_node = std::make_shared<MsgNode>(RECVSIZE);
	_socket->async_read_some(asio::buffer(_recv_node->_msg, _recv_node->_total_len),
		bind(&Session::ReadCallBack, this, std::placeholders::_1, std::placeholders::_2));

	_recv_pending = true;
}
void Session::ReadCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	_recv_node->_cur_len += bytes_transferred;
	if (_recv_node->_cur_len < _recv_node->_total_len)
	{
		_socket->async_read_some(asio::buffer(_recv_node->_msg + _recv_node->_cur_len, _recv_node->_total_len - _recv_node->_cur_len),
			bind(&Session::WriteAllCallBack, this, std::placeholders::_1, std::placeholders::_2));
		return;
	}
	_recv_pending = false;
	_recv_node = nullptr;
}


void Session::ReadAllFromSocket()
{
	if (_recv_pending)
	{
		return;
	}
	_recv_node = std::make_shared<MsgNode>(RECVSIZE);
	_socket->async_receive(asio::buffer(_recv_node->_msg, _recv_node->_total_len), 
		std::bind(&Session::ReadAllCallBack, this, std::placeholders::_1, std::placeholders::_2)); // 回调函数只触发一次，触发时数据已经接收完了。
	// async_receive与async_read_some不能混合使用，因为async_receive内部多次调用async_read_some;多次触发callback函数

	_recv_pending = true;
}
void Session::ReadAllCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	_recv_node->_cur_len += bytes_transferred;
	_recv_node = nullptr;
	_recv_pending = false;
}

/*
* 在实际的项目中，写操作一般用async_send;读操作一般用async_read_some.
*/