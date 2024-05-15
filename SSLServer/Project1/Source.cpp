#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <iostream>
#include <string>


struct async_op : boost::enable_shared_from_this<async_op>, private boost::noncopyable {
	typedef boost::function<void(boost::system::error_code)> completion_func;
	typedef boost::function<boost::system::error_code()> op_func;
	typedef boost::shared_ptr<async_op> ptr;
	static ptr new_() { return ptr(new async_op); }

	struct operation {
		operation(boost::asio::io_service& service, op_func op, completion_func completion)
			: service(&service), op(op), completion(completion)
			, work(new boost::asio::io_service::work(service))
		{}
		operation() : service(0) {}
		boost::asio::io_service* service;
		op_func op;
		completion_func completion;
		typedef boost::shared_ptr<boost::asio::io_service::work> work_ptr;
		work_ptr work;
	};

	void run() {
		while (true) 
		{
			{
				boost::recursive_mutex::scoped_lock lk(cs_);
				if (!started_) break;
			}
			boost::this_thread::sleep(boost::posix_time::millisec(10));
			operation cur;
			{
				boost::recursive_mutex::scoped_lock lk(cs_);
				if (!ops_.empty()) {
					cur = ops_[0]; ops_.erase(ops_.begin());
				}
			}
			if (cur.service)
				cur.service->post(boost::bind(cur.completion, cur.op()));
		}
		self_.reset();
	}

	void start() {
		{
			boost::recursive_mutex::scoped_lock lk(cs_);
			if (started_) return; 
			started_ = true;
		}
		boost::thread t(boost::bind(&async_op::run, this));
	}
	void add(boost::asio::io_service& service, op_func op, completion_func completion) {
		self_ = shared_from_this();
		boost::recursive_mutex::scoped_lock lk(cs_);
		ops_.push_back(operation(service, op, completion));
		if (!started_) 
			start();
	}

	void stop() {
		boost::recursive_mutex::scoped_lock lk(cs_);
		started_ = false; ops_.clear();
	}
private:
	boost::recursive_mutex cs_;
	std::vector<operation> ops_;
	bool started_;
	ptr self_;
};


size_t checksum = 0;

boost::system::error_code compute_file_checksum(std::string file_name)
{
	checksum += 5;
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	return boost::system::error_code(0, boost::system::generic_category());
}
void on_checksum(std::string file_name, boost::system::error_code) {
	std::cout << "checksum for " << file_name << "=" << checksum <<
		std::endl;
}
boost::asio::io_service service;
#include "server.h"


//A(A&& rhs) = delete;
	//A& operator=(A&& rhs) = delete;
//	A(const A& rhs) = delete;// : i_(rhs.i_) { std::cout << "A(const A&)" << std::endl; }
// A& operator=(const A& rhs) = delete;// { i_ = rhs.i_; std::cout << "A = A&" << std::endl; return *this; }

#include <functional>
#include <iostream>
#include <utility>

class A {
public:
	explicit A(int i) : i_(i) {}
	~A() {
		i_ = -10;
	}
	A(const A& rhs) = delete;
	A& operator=(const A& rhs) = delete;
	A(A&& rhs) noexcept {
		i_ = rhs.i_;
		rhs.i_ = 0;
	}
	A& operator=(A&& rhs) noexcept {
		i_ = rhs.i_;
		rhs.i_ = -1;
		return *this;
	}

	int i_;
};

struct B {
	void setFoo(std::function<void()> rt)
	{
		fooB = rt;
	}

	std::function<void()> fooB;
};

#include <regex>
int main()
{
	http::server2::server s("127.0.0.1", "5688", "./", 1);
	// Run the server until stopped.
	s.run();
	//std::function<void()> bar;
	{
		std::unique_ptr<int> p(new int[10]);
		auto foo = [x = 10]() mutable { ++x; };
		auto bar = [ptr = std::move(p)] {};
		auto baz = [p = std::move(p)] {};
	}
	//B b;
	//{
	//	//A a(4);
	//	std::unique_ptr<int> p(new int[10]);
	//	p.get()[0] = 6;

	//	b.setFoo([c = std::move(p)]() mutable {
	//		// Accessing c.i_ after std::move is not safe
	//		// Instead, you should use c.i_ before std::move
	//		std::cout << std::endl;
	//		});
	//}
	//// Attempting to invoke the lambda
	//b.fooB();

	return 0;
}

int main6()
{

	//B b;
	//{
	//	A a(4);
	//	b.setFoo([c = std::move(a)]() mutable {
	//		//std::cout << c.i_ << std::endl;
	//		});
	//}
	//b.fooB();



	{
		A aObject(42);

		// Lambda capturing the local object by moving its value
		auto lambda = [capturedA = std::move(aObject)]() mutable {
			std::cout << "Lambda: " << capturedA.i_ << std::endl;
			};

		// Call the lambda
		lambda();
	}

	//http::server2::server("0.0.0.0", "5688", "./",  1);
	return 1;
}

int main3() {
		A a1(1);
		A a2(std::move(a1));
	return 0;
}


int main2(){

	{
		A a1(1);
		A a2(std::move(a1));
		std::cout << "\nInitial:" << std::endl;
		std::cout << "a1: " << a1.i_ << " a2: " << a2.i_ << std::endl;

		//std::swap(a1, a2);  // Not the best way
		std::cout << "\nAfter swap:" << std::endl;
		std::cout << "a1: " << a1.i_ << " a2: " << a2.i_ << std::endl;
	}


	http::server2::server server("127.0.0.1", "5599", "./", 1);
	//http::server2::server server2(std::move(server));
	boost::shared_ptr<int> mPtr = boost::shared_ptr<int>(new int(3));

	std::string fn = "readme.txt";
	{
		//async_op::new_()->add(service, boost::bind(compute_file_checksum, fn), boost::bind(on_checksum, fn, _1));
	}
	//async_op::new_()->add(service, boost::bind(compute_file_checksum, fn), boost::bind(on_checksum, fn, _1));
	service.run();
	return 0;
}