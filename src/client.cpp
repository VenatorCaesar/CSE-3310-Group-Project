#include <sstream>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include "asio.hpp"
#include "chat_message.hpp"
#include "json.hpp"
#include "Player.hpp"

using asio::ip::tcp;

typedef std::deque<chat_message> chat_message_queue;

class chat_client
{
	public:
		chat_client(asio::io_context& io_context,const tcp::resolver::results_type& endpoints): io_context_(io_context),socket_(io_context)
		{
			do_connect(endpoints);
		}
	  	void write(const chat_message& msg)
	  	{
	    		asio::post(io_context_,[this, msg]()
				{
					bool write_in_progress = !write_msgs_.empty();
					write_msgs_.push_back(msg);
					if (!write_in_progress)
					{
						do_write();
					}
				});
	  	}

		void close()
  		{
    			asio::post(io_context_, [this]() { socket_.close(); });
  		}

	private:
  		void do_connect(const tcp::resolver::results_type& endpoints)
		{
    			asio::async_connect(socket_, endpoints,[this](std::error_code ec, tcp::endpoint) // CSE3310 This is where the connection is established with the server
        		{
          			if (!ec)
          			{
            				do_read_header();
          			}
        		});
  		}

  		void do_read_header()
  		{
    			asio::async_read(socket_,asio::buffer(read_msg_.data(), chat_message::header_length),
        			[this](std::error_code ec, std::size_t /*length*/)
        		{
          			if (!ec && read_msg_.decode_header())
          			{
            				do_read_body();
          			}
          			else
          			{
            				socket_.close();
          			}
        		});
  		}

  		void do_read_body()
  		{
    			asio::async_read(socket_,asio::buffer(read_msg_.body(), read_msg_.body_length()),[this](std::error_code ec, std::size_t /*length*/) // CSE3310 This is where the message is received from the server
        		{
          			if (!ec)
          			{
				    	std::cout.write(read_msg_.body(), read_msg_.body_length());
				    	std::cout << "\n";
						do_read_header();
          			}
          			else
          			{
            			socket_.close();
          			}
        		});
  		}

 		void do_write()
  		{
    			asio::async_write(socket_,asio::buffer(write_msgs_.front().data(),write_msgs_.front().length()),[this](std::error_code ec, std::size_t /*length*/)
        		{
          			if (!ec)
          			{
            				write_msgs_.pop_front();
            				if (!write_msgs_.empty())
            				{
              					do_write();
            				}
          			}
          			else
          			{
            				socket_.close();
          			}
        		});
  		}

	private:
		asio::io_context& io_context_;
		tcp::socket socket_;
		chat_message read_msg_;
		chat_message_queue write_msgs_;
};

int main(int argc, char** argv)
{
	try
	{
		if (argc != 3)
		{
			std::cerr << "Usage: client <host> <port>\n";
			return 1;
		}

		asio::io_context io_context;

		tcp::resolver resolver(io_context);
		auto endpoints = resolver.resolve(argv[1], argv[2]);
		chat_client c(io_context, endpoints);

		std::thread t([&io_context](){ io_context.run(); });
		
		char line[chat_message::max_body_length + 1]; // CSE3310 This is where the maximum chat size of the body is defined
		while (std::cin.getline(line, chat_message::max_body_length + 1)) // CSE3310 This is where it accepts input through cin
		{
			std::chrono::system_clock::time_point rn = std::chrono::system_clock::now();
			time_t tt;
			tt = std::chrono::system_clock::to_time_t(rn);
			std::stringstream ct;
			ct << ctime(&tt);
			std::string time = ct.str();
			int length = time.size();
			time = time.substr(0,length-1);

			nlohmann::json::object_t object_value = {{"chrono",time},{"id","1001588696"},{"name","Hunter Redhead"}};//
			nlohmann::json j_object_value(object_value);//

			chat_message msg;
			std::stringstream ss;//
			ss << j_object_value << line;//
			std::string js = ss.str();//
			msg.body_length(/*std::strlen(line) + */std::strlen(js.c_str()));//
			std::memcpy(msg.body(), /*line*/js.c_str(), msg.body_length());//
			msg.encode_header();
			c.write(msg); // CSE3310 This is where it sends the message to the server
		}

		c.close();
		t.join();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
