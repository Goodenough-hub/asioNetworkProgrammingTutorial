#include "CServer.h"

using namespace std;

CServer::CServer(boost::asio::io_context& ioc, short port) :_ioc(ioc), _acceptor(ioc, tcp::endpoint(tcp::v4(), port)) {
	cout << "Server start sucess, on port: " << port << endl;
	start_accept();
}


void CServer::start_accept()
{
	shared_ptr<CSession> new_session = make_shared<CSession>(_ioc, this);
	//Session* new_session = new Session(_ioc);
	_acceptor.async_accept(new_session->Socket(), std::bind(&CServer::handle_accept, this, new_session, placeholders::_1));

}

void CServer::handle_accept(shared_ptr<CSession> new_session, const boost::system::error_code& error)
{
	if (!error)
	{
		new_session->Start();
		_sessions.insert(make_pair(new_session->GetUuid(), new_session));
	}
	else
	{
		//delete new_session;
	}
	start_accept(); // 准备接纳新的客户端
}

void CServer::ClearSession(std::string uuid)
{
	_sessions.erase(uuid);
}