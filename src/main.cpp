#include<iostream>
#include<chrono>
#include"thread_pool.hpp"

using std::cout;
using std::endl;


void func()
{
	for(int n=0;n<100;++n)
	{ }
}

int main()
{
	thread_pool<void> th_pool(50);

	//clock begin
	auto start = std::chrono::system_clock::now();

	for (int n = 0; n < 50; ++n)
		th_pool.submit_task(func);

	//clock end
	auto end = std::chrono::system_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);


	cout << "time = " << (double)duration.count() * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den << 's' << endl;

	system("pause");
	return 0;
}