#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include "CSession.h"

using boost::asio::ip::tcp;

class CServer {
public:
	CServer(boost::asio::io_context& ioc, short port);
	void ClearSession(std::string uuid);
private:
	void start_accept(); // ���������ģ�����һ��acceptor
	void handle_accept(std::shared_ptr<CSession> new_session, const boost::system::error_code& error); // ���Է�������������ʱ���ᴥ������ص�����
	boost::asio::io_context& _ioc; // io_context�����������ƣ�����������������ó�һ�����õĳ�Ա����
	tcp::acceptor _acceptor;
	std::map<std::string, std::shared_ptr<CSession>> _sessions; // ��map������session������ҪΨһid��ѩ���㷨��uuid
};

