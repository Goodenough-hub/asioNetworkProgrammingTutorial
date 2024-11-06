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
	void start_accept(); // 用来监听的，创建一个acceptor
	void handle_accept(std::shared_ptr<CSession> new_session, const boost::system::error_code& error); // 当对方有连接连过来时，会触发这个回调函数
	boost::asio::io_context& _ioc; // io_context不允许做复制，构造操作，所以设置成一个引用的成员变量
	tcp::acceptor _acceptor;
	std::map<std::string, std::shared_ptr<CSession>> _sessions; // 用map来管理session，就需要唯一id。雪花算法、uuid
};

