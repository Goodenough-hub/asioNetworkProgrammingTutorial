#include <iostream>
#include <boost/asio.hpp>
#include <set>
#include <memory>
using boost::asio::ip::tcp;
const int max_length = 1024;
typedef std::shared_ptr<tcp::socket> socket_ptr;
std::set<std::shared_ptr<std::thread>> thread_set;
using namespace std;

// session跑在线程里
void session(socket_ptr sock)
{
	try {
		for (;;) {
			char data[max_length];
			memset(data, '\0', max_length);
			boost::system::error_code error;
			//size_t length = boost::asio::read(sock, boost::asio::buffer(data, max_length), error); //读出的数据长度一定是max_length

			size_t length = sock->read_some(boost::asio::buffer(data, max_length), error); // 收到多少就读出多少
			// 接收的少，可能是上次的还没有发完，需要先发完上次的数据
			// 接收的多，可能是客户端堆积了一定量的数据才发送过来

			if (error == boost::asio::error::eof) // 表示是对端关闭的错误
			{
				std::cout << "connection closed by peer" << endl;
				break;
			}
			else if (error)
			{
				throw boost::system::system_error(error);
			}

			cout << "receive from " << sock->remote_endpoint().address().to_string() << endl;
			cout << "receive message is " << data << endl;
			// 回传给对方
			boost::asio::write(*sock, boost::asio::buffer(data, length));
		}
	}
	catch (exception& e)
	{
		std::cerr << "Exception in thread: " << e.what() << "\n" << std::endl;
	}
}

void server(boost::asio::io_context& io_context, unsigned short port)
{
	tcp::acceptor a(io_context, tcp::endpoint(tcp::v4(), port));
	for (;;)
	{
		socket_ptr socket(new tcp::socket(io_context));
		a.accept(*socket);
		auto t = std::make_shared<std::thread>(session, socket); // 创建一个线程，这个线程做session的工作，session的参数是socket
		//session(socket);// 若是放在主线程里会影响主线程的工作

		thread_set.insert(t); // 为了防止线程被释放而没有执行session函数，把线程放入线程集合里，这样线程的引用计数就会加一，直到被清掉
	}
}
int main()
{
	// 主线程若是直接退出，会直接导致子进程退出。可能会出现子进程的任务没有完成。
	// 
	// 主线程一定要等到所有子进程全部退出才能退出
	try {
		boost::asio::io_context ioc;
		server(ioc, 10086);
		
		for (auto& t : thread_set) // 调用join操作，主线程只有等所有子进程都执行完了，才进行下一步操作
		{
			t->join();
		}
	}
	catch (std::exception& e)
	{
		cerr << "Exception " << e.what() << "\n";
	}
	return 0;
}
