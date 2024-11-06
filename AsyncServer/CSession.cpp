#include "CSession.h"
#include <iostream>
#include "CServer.h"

using namespace std;

void CSession::Send(char* msg, int max_length)
{
	bool pending = false; // pendingΪtrue��ʾ�ϴε�����û�з��ꣻpendingΪfalse��ʾ�����Ѿ������ˡ�
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
		std::bind(&CSession::handle_read, this, placeholders::_1, placeholders::_2, shared_from_this())); // ���Ĳ���������shared_ptr<Session>(this)��������ָ�룬��������������ָ�룬���������ü����ǲ�ͬ�ģ��������������ָ��ͬʱ����һ���ڴ�Ĳ���������Ҫ��shared_from_this()��������������ͬһ��ָ��ָ�롣
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
			std::bind(&CSession::handle_write, this, placeholders::_1, _self_shared));*/ // bindҲ������lambda���������[this](const auto& error){ handle_write(error); };
		// async_write��async_send������
		// async_send�ǳ�Ա������ֱ�ӵ��á�async_write��ȫ�ֺ�������socket��Ϊ��������
	}
	else
	{
		cout << "read error" << endl; // �Զ˹ر�Ҳ�ᴥ�����Ļص�������
		//delete this; // ����Session�����ӶϿ�
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

