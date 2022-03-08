#pragma once

#include<thread>
#include<vector>
#include<queue>
#include<mutex>
#include<condition_variable>
#include<future>

using std::vector;
using std::queue;
using std::thread;
using std::mutex;
using std::unique_lock;
using std::condition_variable;
using std::packaged_task;
using std::future;
using std::bind;

template<typename return_type>
class thread_pool
{
	using packaged_t = packaged_task<return_type()>;
	using future_t = future<return_type>;

	private:
		vector<thread> thread_arr;
		queue<packaged_t> tasks;

		mutex mute;
		condition_variable cv;

		size_t thread_num = 0;
		size_t tasks_num = 0;
		bool stop_flag = false;

	public:
		thread_pool(size_t num) : thread_num(num)
		{
			thread_set(thread_num);
		}

		~thread_pool()
		{
			if (stop_flag == false)
				stop();

			for (auto& p : thread_arr)
			{
				if (p.joinable())
					p.detach();
			}
		}
		
		template<typename func, typename... args>
		future_t submit_task(func&& f, args&&... arg)
		{
			packaged_t pt(bind(std::forward<func>(f), std::forward<args>(arg)...));
			future_t res = pt.get_future();

			if (stop_flag)
				return res;

			unique_lock<mutex> lock(mute);

			tasks.push(move(pt));
			lock.unlock();

			++tasks_num;
			while (tasks_num >= thread_num)
				thread_refill();

			cv.notify_one();
			return res;
		}

		void stop()
		{
			stop_flag = true;
			cv.notify_all();
		}

		void run()
		{
			stop_flag = false;
		}

		size_t worker_num()
		{
			return thread_num;
		}

		size_t task_num()
		{
			return tasks_num;
		}

	private:
		void thread_set(size_t num)
		{
			for (size_t n = 0; n < num; ++n)
				thread_arr.push_back(thread(&thread_pool::thread_worker, this));
		}

		void thread_worker()
		{
			while (1)
			{
				unique_lock<mutex> lock(mute);
				cv.wait(lock, [this] {return stop_flag || !tasks.empty(); });

				if (stop_flag == true || tasks.empty())
				{
					thread_num--;
					return;
				}

				auto task = move(tasks.front());
				if (task.valid())
				{
					tasks.pop();
					task();

					tasks_num--;
				}
			}
		}

		void thread_refill()
		{
			thread_set(thread_num);
			thread_num *= 2;
		}
};