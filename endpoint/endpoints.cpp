#include "endpoint.h"
#include <boost/asio.hpp>
#include <iostream>

using namespace boost;

// 客户端
int client_end_point()
{
	std::string raw_ip_address = "127.4.8.1";
	unsigned short port_num = 3333;
	boost::system::error_code ec;
	asio::ip::address ip_address = asio::ip::address::from_string(raw_ip_address, ec);
	if (ec.value() != 0)
	{
		std::cout << "Failed to parse the IP address.Error code = " << ec.value() << ".Message is " << ec.message();
		return ec.value();
	}

	asio::ip::tcp::endpoint ep(ip_address, port_num);
	return 0;
}

// 服务端
int server_end_point()
{
	unsigned short port_num = 3333;
	asio::ip::address ip_address = asio::ip::address_v6::any();
	asio::ip::tcp::endpoint ep(ip_address, port_num);
	return 0;
}

// 创建socket——客户端
int create_tcp_socket()
{
	asio::io_context ioc;
	asio::ip::tcp protocol = asio::ip::tcp::v4();
	asio::ip::tcp::socket sock(ioc);

	// 打开socket----现在创建的socket会自动打开
	boost::system::error_code ec;
	sock.open(protocol, ec);
	if (ec.value() != 0) {
		std::cout << "Falied to open the socket! Error code = " << ec.value() << ".Message: " << ec.message();
		return ec.value();
	}
	return 0;
}

// 创建socket——服务器端
int create_acceptor_socket()
{
	asio::io_context ios;
	/*asio::ip::tcp::acceptor acceptor(ios);
	asio::ip::tcp protocol = asio::ip::tcp::v4();
	boost::system::error_code ec;
	acceptor.open(protocol, ec);
	if (ec.value() != 0)
	{
		std::cout << "Failed to open the acceptor socket!" << "Error code = " << ec.value() << ".Messgae: " << ec.message();
		return ec.value();
	}*/
	asio::ip::tcp::acceptor a(ios, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 3333)); // 新版本
	return 0;
}

int bind_acceptor_socket()
{
	unsigned short port_num = 3333;
	asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);
	asio::io_context ios;
	asio::ip::tcp::acceptor acceptor(ios, ep.protocol());
	boost::system::error_code ec;
	acceptor.bind(ep, ec);
	if (ec.value() != 0)
	{
		// Failed to bind the acceptor socket. Breaking
		// execution.
		std::cout << "Failed to bind the acceptor socket." << "Error code = " << ec.value() << ".Message: " << ec.message();
		return ec.value();
	}

	return 0;
}

int connect_to_end()
{
	std::string raw_ip_address = "192.168.1.124";
	unsigned short port_num = 3333;
	try {
		asio::ip::tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address), port_num);
		asio::io_context ios;
		asio::ip::tcp::socket sock(ios, ep.protocol());
		sock.connect(ep);
	}
	catch (system::system_error& e) {
		std::cout << "Error occured! Error code = " << e.code() << ".Message: " << e.what();
		return e.code().value();
	}
}

int dns_connect_to_end()
{
	std::string host = "llfc.club";
	std::string port_num = "333";
	asio::io_context ios;
	asio::ip::tcp::resolver::query resolver_query(host, port_num, asio::ip::tcp::resolver::query::numeric_service); // 用来查询
	asio::ip::tcp::resolver resolver(ios); // resolver解析器
	try {
		asio::ip::tcp::resolver::iterator it = resolver.resolve(resolver_query);
		asio::ip::tcp::socket sock(ios);
		asio::connect(sock, it);
	}
	catch (system::system_error& e) {
		std::cout << "Error occured!Error code = " << e.code() << ".Message:" << e.what();
		return e.code().value();
	}
}

int accept_new_connection()
{
	const int BACKLOG_SIZE = 30; // 监听队列的大小——设置30，最多能有60个
	unsigned short port_num = 3333;
	asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);
	asio::io_context ios;
	try {
		asio::ip::tcp::acceptor acceptor(ios, ep.protocol());
		acceptor.bind(ep);
		acceptor.listen(BACKLOG_SIZE);
		asio::ip::tcp::socket sock(ios); // 这个socket与acceptor不一样，是用来和客户端通信的
		acceptor.accept(sock); // 接收器接受新的连接，交给sock来处理
	}
	catch (system::system_error& e) {
		std::cout << "Error occured! Error code = " << e.code() << ".Message: " << e.what();

	}
}

void use_const_buffer()
{
	std::string buf = "hello world";
	asio::const_buffer asio_buf(buf.c_str(), buf.length()); // 第一个参数是内存的首地址，第二个参数是长度
	std::vector<asio::const_buffer> buffers_sequence;
	buffers_sequence.push_back(asio_buf);

	// 伪代码： asio.send(buffers_sequence);
}

void use_buffer_str()
{
	// asio提出了buffer()函数，该函数接收多种形式的字节流，该函数返回asio::mutable_buffers_1 或者asio::const_buffers_1结构的对象
	asio::const_buffers_1 output_buf = asio::buffer("hello world");
}

void use_buffer_array()
{
	const size_t BUF_SIZE_BYTES = 20;
	std::unique_ptr<char[]> buf(new char[BUF_SIZE_BYTES]); // 创建一个数组，内存首地址交给unique_ptr来管理，就不用担心内存回收的问题了
	auto input_buf = asio::buffer(static_cast<void*>(buf.get()), BUF_SIZE_BYTES); // buf.get()获取裸指针，转为void*指针类型，长度是BUF_SIZE_BYTES
}


/*------------------写操作------------------------*/

void write_to_socket(asio::ip::tcp::socket& sock)
{
	std::string buf = "Hello World!";
	std::size_t total_bytes_written = 0;

	// 循环发送
	// writen_some 返回每次写入的字节数
	while (total_bytes_written != buf.length())
	{
		total_bytes_written += sock.write_some(asio::buffer(buf.c_str() + total_bytes_written,
			buf.length() - total_bytes_written)); // asio::buffer第一个参数：内存首地址=开始首地址+已发送的数据长度的一个偏移；第二个参数：长度=总长度-已发送的数据长度。
		// 每次把已发完的做个累加，未发完的做个累加
		// 为啥不能直接发送完所有数据？？？
		// 有buffer用户发送缓冲区和tcp发送缓冲区
		// tcp发送缓冲区中可能有上次未发送完成的数据。发送时的长度=上次未发送的数据长度+此次数据长度。若大于整个tcp缓冲区则会截取数据下次发送
	}
}

// 串联整个写的流程
// 
// 客户端发数据——同步方式：用循环多次发送数据——write_to_socket
int send_data_by_write_some()
{
	std::string raw_ip_address = "192.168.3.11";
	unsigned short port_num = 3333;
	try {
		asio::ip::tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address), port_num);
		asio::io_context ioc;
		asio::ip::tcp::socket sock(ioc, ep.protocol());
		sock.connect(ep);
		write_to_socket(sock);
	}
	catch (system::system_error& e)
	{
		std::cout << "Error occured! Error code = " << e.code() << ". Message: " << e.what();
		return e.code().value();
	}
	return 0;
}

// 客户端发数据——同步方式：一次性发送所有数据,没发完tcp就阻塞在那里。直到全部发完——send
int send_data_by_send()
{
	std::string raw_ip_address = "192.168.3.11";
	unsigned short port_num = 3333;
	try {
		asio::ip::tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address), port_num);
		asio::io_context ioc;
		asio::ip::tcp::socket sock(ioc, ep.protocol());
		sock.connect(ep);
		std::string buf = "Hello World!";
		int send_length = sock.send(asio::buffer(buf.c_str(), buf.length()));
		// send_length只会出现三种情况
		// <0: 出现系统级的错误
		// =0: 对端关闭
		// >0: 一定是buf.length()的长度，发送成功
		if (send_length <= 0)
		{
			return 0;
		}
	}
	catch (system::system_error& e)
	{
		std::cout << "Error occured! Error code = " << e.code() << ". Message: " << e.what();
		return e.code().value();
	}
	return 0;
}

// 客户端发数据——同步方式：一次性发送所有数据,没发完tcp就阻塞在那里。直到全部发完——全局函数write
int send_data_by_write()
{
	std::string raw_ip_address = "192.168.3.11";
	unsigned short port_num = 3333;
	try {
		asio::ip::tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address), port_num);
		asio::io_context ioc;
		asio::ip::tcp::socket sock(ioc, ep.protocol());
		sock.connect(ep);
		std::string buf = "Hello World!";
		int send_length = asio::write(sock, asio::buffer(buf.c_str(), buf.length()));
		// send_length只会出现三种情况
		// <0: 出现系统级的错误
		// =0: 对端关闭
		// >0: 一定是buf.length()的长度，发送成功
		if (send_length <= 0)
		{
			return 0;
		}
	}
	catch (system::system_error& e)
	{
		std::cout << "Error occured! Error code = " << e.code() << ". Message: " << e.what();
		return e.code().value();
	}
	return 0;
}


/*------------------读操作------------------------*/
std::string read_from_socket(asio::ip::tcp::socket& sock)
{
	const unsigned char MESSAGE_SIZE = 7; // 消息长度
	char buf[MESSAGE_SIZE];
	std::size_t total_bytes_read = 0;
	// 循环读
	while (total_bytes_read != MESSAGE_SIZE)
	{
		total_bytes_read += sock.read_some(asio::buffer(buf + total_bytes_read, MESSAGE_SIZE - total_bytes_read));
	}
	return std::string(buf, total_bytes_read);
}

// 串联整个读的流程
//
// 客户端读数据——同步方式：如果对方不发数据，就会一直阻塞在这里——read_some
int read_data_by_read_some()
{
	std::string raw_ip_address = "127.0.0.1";
	unsigned short port_num = 3333;
	try {
		asio::ip::tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address), port_num);
		asio::io_context ioc;
		asio::ip::tcp::socket sock(ioc, ep.protocol());
		read_from_socket(sock);
	}
	catch (system::system_error& e){
		std::cout << "Error occured! Error code = " << e.code() << ". Message: " << e.what();
		return e.code().value();
	}
	return 0;
}

// 客户端读数据——同步方式：如果对方不发数据，就会一直阻塞在这里——read_some
int read_data_by_read_receive()
{
	std::string raw_ip_address = "127.0.0.1";
	unsigned short port_num = 3333;
	try {
		asio::ip::tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address), port_num);
		asio::io_context ioc;
		asio::ip::tcp::socket sock(ioc, ep.protocol());
		const unsigned char BUFF_SIZE = 7;
		char buffer_receive[BUFF_SIZE];
		int receive_length = sock.receive(asio::buffer(buffer_receive, BUFF_SIZE));
		// 返回值receive_length只有三种情况：
		// <0: 发生系统级的错误
		// =0: 对端关闭
		// >0: 肯定等于BUFF_SIZE.一次性全部读完数据
		if (receive_length <= 0)
		{
			std::cout << "receive failed" << std::endl;
		}
	}
	catch (system::system_error& e) {
		std::cout << "Error occured! Error code = " << e.code() << ". Message: " << e.what();
		return e.code().value();
	}
	return 0;
}

// 客户端读数据——同步方式：如果对方不发数据，就会一直阻塞在这里——全局函数read
int read_data_by_read_read()
{
	std::string raw_ip_address = "127.0.0.1";
	unsigned short port_num = 3333;
	try {
		asio::ip::tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address), port_num);
		asio::io_context ioc;
		asio::ip::tcp::socket sock(ioc, ep.protocol());
		const unsigned char BUFF_SIZE = 7;
		char buffer_receive[BUFF_SIZE];
		int receive_length = asio::read(sock, asio::buffer(buffer_receive, BUFF_SIZE));
		// 返回值receive_length只有三种情况：
		// <0: 发生系统级的错误
		// =0: 对端关闭
		// >0: 肯定等于BUFF_SIZE.一次性全部读完数据
		if (receive_length <= 0)
		{
			std::cout << "receive failed" << std::endl;
		}
	}
	catch (system::system_error& e) {
		std::cout << "Error occured! Error code = " << e.code() << ". Message: " << e.what();
		return e.code().value();
	}
	return 0;
}