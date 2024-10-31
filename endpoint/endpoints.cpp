#include "endpoint.h"
#include <boost/asio.hpp>
#include <iostream>

using namespace boost;

// �ͻ���
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

// �����
int server_end_point()
{
	unsigned short port_num = 3333;
	asio::ip::address ip_address = asio::ip::address_v6::any();
	asio::ip::tcp::endpoint ep(ip_address, port_num);
	return 0;
}

// ����socket�����ͻ���
int create_tcp_socket()
{
	asio::io_context ioc;
	asio::ip::tcp protocol = asio::ip::tcp::v4();
	asio::ip::tcp::socket sock(ioc);

	// ��socket----���ڴ�����socket���Զ���
	boost::system::error_code ec;
	sock.open(protocol, ec);
	if (ec.value() != 0) {
		std::cout << "Falied to open the socket! Error code = " << ec.value() << ".Message: " << ec.message();
		return ec.value();
	}
	return 0;
}

// ����socket������������
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
	asio::ip::tcp::acceptor a(ios, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 3333)); // �°汾
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
	asio::ip::tcp::resolver::query resolver_query(host, port_num, asio::ip::tcp::resolver::query::numeric_service); // ������ѯ
	asio::ip::tcp::resolver resolver(ios); // resolver������
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
	const int BACKLOG_SIZE = 30; // �������еĴ�С��������30���������60��
	unsigned short port_num = 3333;
	asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);
	asio::io_context ios;
	try {
		asio::ip::tcp::acceptor acceptor(ios, ep.protocol());
		acceptor.bind(ep);
		acceptor.listen(BACKLOG_SIZE);
		asio::ip::tcp::socket sock(ios); // ���socket��acceptor��һ�����������Ϳͻ���ͨ�ŵ�
		acceptor.accept(sock); // �����������µ����ӣ�����sock������
	}
	catch (system::system_error& e) {
		std::cout << "Error occured! Error code = " << e.code() << ".Message: " << e.what();

	}
}

void use_const_buffer()
{
	std::string buf = "hello world";
	asio::const_buffer asio_buf(buf.c_str(), buf.length()); // ��һ���������ڴ���׵�ַ���ڶ��������ǳ���
	std::vector<asio::const_buffer> buffers_sequence;
	buffers_sequence.push_back(asio_buf);

	// α���룺 asio.send(buffers_sequence);
}

void use_buffer_str()
{
	// asio�����buffer()�������ú������ն�����ʽ���ֽ������ú�������asio::mutable_buffers_1 ����asio::const_buffers_1�ṹ�Ķ���
	asio::const_buffers_1 output_buf = asio::buffer("hello world");
}

void use_buffer_array()
{
	const size_t BUF_SIZE_BYTES = 20;
	std::unique_ptr<char[]> buf(new char[BUF_SIZE_BYTES]); // ����һ�����飬�ڴ��׵�ַ����unique_ptr�������Ͳ��õ����ڴ���յ�������
	auto input_buf = asio::buffer(static_cast<void*>(buf.get()), BUF_SIZE_BYTES); // buf.get()��ȡ��ָ�룬תΪvoid*ָ�����ͣ�������BUF_SIZE_BYTES
}


/*------------------д����------------------------*/

void write_to_socket(asio::ip::tcp::socket& sock)
{
	std::string buf = "Hello World!";
	std::size_t total_bytes_written = 0;

	// ѭ������
	// writen_some ����ÿ��д����ֽ���
	while (total_bytes_written != buf.length())
	{
		total_bytes_written += sock.write_some(asio::buffer(buf.c_str() + total_bytes_written,
			buf.length() - total_bytes_written)); // asio::buffer��һ���������ڴ��׵�ַ=��ʼ�׵�ַ+�ѷ��͵����ݳ��ȵ�һ��ƫ�ƣ��ڶ�������������=�ܳ���-�ѷ��͵����ݳ��ȡ�
		// ÿ�ΰ��ѷ���������ۼӣ�δ����������ۼ�
		// Ϊɶ����ֱ�ӷ������������ݣ�����
		// ��buffer�û����ͻ�������tcp���ͻ�����
		// tcp���ͻ������п������ϴ�δ������ɵ����ݡ�����ʱ�ĳ���=�ϴ�δ���͵����ݳ���+�˴����ݳ��ȡ�����������tcp����������ȡ�����´η���
	}
}

// ��������д������
// 
// �ͻ��˷����ݡ���ͬ����ʽ����ѭ����η������ݡ���write_to_socket
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

// �ͻ��˷����ݡ���ͬ����ʽ��һ���Է�����������,û����tcp�����������ֱ��ȫ�����ꡪ��send
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
		// send_lengthֻ������������
		// <0: ����ϵͳ���Ĵ���
		// =0: �Զ˹ر�
		// >0: һ����buf.length()�ĳ��ȣ����ͳɹ�
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

// �ͻ��˷����ݡ���ͬ����ʽ��һ���Է�����������,û����tcp�����������ֱ��ȫ�����ꡪ��ȫ�ֺ���write
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
		// send_lengthֻ������������
		// <0: ����ϵͳ���Ĵ���
		// =0: �Զ˹ر�
		// >0: һ����buf.length()�ĳ��ȣ����ͳɹ�
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


/*------------------������------------------------*/
std::string read_from_socket(asio::ip::tcp::socket& sock)
{
	const unsigned char MESSAGE_SIZE = 7; // ��Ϣ����
	char buf[MESSAGE_SIZE];
	std::size_t total_bytes_read = 0;
	// ѭ����
	while (total_bytes_read != MESSAGE_SIZE)
	{
		total_bytes_read += sock.read_some(asio::buffer(buf + total_bytes_read, MESSAGE_SIZE - total_bytes_read));
	}
	return std::string(buf, total_bytes_read);
}

// ����������������
//
// �ͻ��˶����ݡ���ͬ����ʽ������Է��������ݣ��ͻ�һֱ�����������read_some
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

// �ͻ��˶����ݡ���ͬ����ʽ������Է��������ݣ��ͻ�һֱ�����������read_some
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
		// ����ֵreceive_lengthֻ�����������
		// <0: ����ϵͳ���Ĵ���
		// =0: �Զ˹ر�
		// >0: �϶�����BUFF_SIZE.һ����ȫ����������
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

// �ͻ��˶����ݡ���ͬ����ʽ������Է��������ݣ��ͻ�һֱ�����������ȫ�ֺ���read
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
		// ����ֵreceive_lengthֻ�����������
		// <0: ����ϵͳ���Ĵ���
		// =0: �Զ˹ر�
		// >0: �϶�����BUFF_SIZE.һ����ȫ����������
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