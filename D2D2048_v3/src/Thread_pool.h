#pragma once
#include <iostream>
#include <functional>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

template<class T>
class Thread_Safe_Queue
{
public:
	Thread_Safe_Queue() {}
	~Thread_Safe_Queue() {}

	// 是否为空
	bool IsEmpty()
	{
		std::lock_guard<std::mutex> lock(m_queMutex);
		return m_queue.empty();
	}
	// 获取大小
	int GetSize()
	{
		std::lock_guard<std::mutex> lock(m_queMutex);
		return m_queue.size();
	}
	// 添加任务
	void Add(T t)
	{
		std::lock_guard<std::mutex> lock(m_queMutex);
		m_queue.push(std::move(t));
	}
	// 获取任务
	T Get()
	{
		std::lock_guard<std::mutex> lock(m_queMutex);
		auto t = std::move(m_queue.front());
		m_queue.pop();
		return t;
	}

	std::queue<T> m_queue;
	std::mutex m_queMutex;
};

class Thread_pool
{
public:
	// 任务
	using Task = std::function<void()>;

	Thread_pool(int size)
		: m_size(size)
	{
		for (size_t i = 0; i < size; i++)
		{
			m_work_threads.push_back(std::thread([this]() {
				while (true)
				{
					Task task;

					{
						std::unique_lock<std::mutex> lock(m_mutex);
						cv.wait(lock, [this]() { return !m_task.IsEmpty() || is_close; });
						if (is_close == true && m_task.IsEmpty()) return;
						task = std::move(m_task.Get());
					}

					task();
				}
			}));
		}
	}
	~Thread_pool() { this->Close(); }

	void AddTask(Task task)
	{
		m_task.Add(std::move(task));
		cv.notify_one();
	}

	void Close()
	{
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			is_close = true;
		}
		cv.notify_all();
		for (auto& it : m_work_threads) { if (it.joinable()) it.join(); };
	}

private:

	std::mutex m_mutex;
	Thread_Safe_Queue<Task> m_task;	// 任务队列
	std::vector<std::thread> m_work_threads;	// 工作线程
	std::atomic<bool> is_close = false;	// 是否关闭
	std::condition_variable cv;	// 条件变量

	int m_size = 0;
};
