#include "Session.h"

Session::Session(std::shared_ptr<asio::ip::tcp::socket> socket) :_socket(socket), _send_pending(false), _recv_pending(false) {

}

void Session::Connect(const asio::ip::tcp::endpoint& ep) {
	_socket->connect(ep);
}

void Session::WriteCallBackErr(const boost::system::error_code& ec, std::size_t bytes_transferred,
	std::shared_ptr<MsgNode> msg_node) // �����������Ǳ�֤sendnode�ڵ�Ĵ���ԣ���Ϊ����ָ��ָ������ü������һ
{
	if (bytes_transferred + msg_node->_cur_len < msg_node->_total_len) // û�з����꣬�������͡� `��ǰ������ɵĳ��� + ֮ǰ���͵ĳ��� = �Ѿ����͵��ܳ���` ��Ҫ���͵ĳ������Ա�
	{
		_send_node->_cur_len += bytes_transferred;
		this->_socket->async_write_some(asio::buffer(_send_node->_msg
			+ _send_node->_cur_len, _send_node->_total_len - _send_node->_cur_len), // Ҫ����ƫ����
			std::bind(&Session::WriteCallBackErr, this, std::placeholders::_1, std::placeholders::_2, _send_node));
	}
}

void Session::WriteToSocketErr(const std::string buf)
{
	_send_node = make_shared<MsgNode>(buf.c_str(), buf.length());
	this->_socket->async_write_some(asio::buffer(_send_node->_msg, _send_node->_total_len),
		std::bind(&Session::WriteCallBackErr, this, std::placeholders::_1, std::placeholders::_2,
			_send_node)); // д�ɹ���ͻ��������ص�����
}

void Session::WriteCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred,
	std::shared_ptr<MsgNode> send_node) // �����������Ǳ�֤sendnode�ڵ�Ĵ���ԣ���Ϊ����ָ��ָ������ü������һ
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
	if (_send_pending) // ��δ�����������
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

	this->_socket->async_send(asio::buffer(buf), std::bind(&Session::WriteAllCallBack, this, std::placeholders::_1, std::placeholders::_2)); // async_send��ε���async_write_some����
	// ��֤����һ��WriteAllCallBack

	_send_pending = true; // ���͵�״̬��Ϊtrue
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

	if (!_send_queue.empty()) // ������������
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
		std::bind(&Session::ReadAllCallBack, this, std::placeholders::_1, std::placeholders::_2)); // �ص�����ֻ����һ�Σ�����ʱ�����Ѿ��������ˡ�
	// async_receive��async_read_some���ܻ��ʹ�ã���Ϊasync_receive�ڲ���ε���async_read_some;��δ���callback����

	_recv_pending = true;
}
void Session::ReadAllCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
	_recv_node->_cur_len += bytes_transferred;
	_recv_node = nullptr;
	_recv_pending = false;
}

/*
* ��ʵ�ʵ���Ŀ�У�д����һ����async_send;������һ����async_read_some.
*/