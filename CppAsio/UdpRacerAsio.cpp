// UdpRacerAsio.cpp : Defines the entry point for the console application.
//


#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include <boost/thread.hpp>

#include <iostream>
#include <vector>

typedef struct {
	unsigned int NumberHosts;
	unsigned int currIdx;
	char	hosts[200];		
} payload;

volatile long counter = 0;
short port = 41234;

class server
{
private:
  boost::asio::io_service&			io_service_;
  boost::asio::ip::udp::socket		socket_;
  boost::asio::ip::udp::endpoint	sender_endpoint_;

  enum { max_length = 1024 };
  //char data_[max_length];
  payload data_;

public:
  server(boost::asio::io_service& io_service, short port)
    : io_service_(io_service),
      socket_(io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port))
	{
		std::cout << "start listening..." << std::endl;
		socket_.async_receive_from(
			boost::asio::buffer(&data_, max_length), 
			sender_endpoint_,
			boost::bind(&server::handle_receive_from, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
		);
  }

  void handle_receive_from(const boost::system::error_code& error,
      size_t bytes_recvd)
  {
	if (!error && bytes_recvd > 0)
    {
		//std::cout << "received from [" << sender_endpoint_.address().to_string() << "]" << std::endl;

		data_.currIdx += 1;
		if ( data_.currIdx >= data_.NumberHosts ) {
			data_.currIdx = 0;
		}

		char* nextHost = &(data_.hosts[ data_.currIdx * 20 ]);

		boost::asio::ip::address_v4 destIP( boost::asio::ip::address_v4::from_string( nextHost ) );
		boost::asio::ip::udp::endpoint destEndpoint( destIP, port );

		//std::cout << "send to [" << destEndpoint.address().to_string() << "]" << std::endl;

		InterlockedIncrement( &counter );

		socket_.async_send_to(
          boost::asio::buffer(&data_, bytes_recvd), 
		  destEndpoint,
          boost::bind(&server::handle_send_to, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }
    else
    {
		std::cout << "error: message [" << error.message() << "] code [" << error.value() << "]" << std::endl;
		socket_.async_receive_from(
			boost::asio::buffer(&data_, max_length), sender_endpoint_,
			boost::bind(&server::handle_receive_from, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }
  }

  void handle_send_to(const boost::system::error_code& /*error*/,
      size_t /*bytes_sent*/)
  {
    socket_.async_receive_from(
        boost::asio::buffer(&data_, max_length), sender_endpoint_,
        boost::bind(&server::handle_receive_from, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
  }

};

void timerFunc() {

	while (true) {
		Sleep(1000);
		ULONG help = counter;
		InterlockedExchange( &counter, 0 );
		std::cout << "sends per second [" << help << "]" << std::endl;
	}
}

int wmain(int argc, WCHAR* argv[])
{
	try
	{
		boost::thread performanceTicker(timerFunc);
		performanceTicker.detach();

		boost::asio::io_service io_service;
		server s(io_service, port);
		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
	return 0;
}

