#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class BlockingQueue {

private:

	size_t m_maxCount;
	std::queue<T> m_items;
	std::mutex m_lock;
	std::condition_variable m_notFull;
	std::condition_variable m_notEmpty;

public:

	explicit BlockingQueue(size_t maxCount)
		: m_maxCount(maxCount) {
	}

	void put(T&& e) {
		std::unique_lock<std::mutex> l(m_lock);
		m_notFull.wait(l, [&]() { return m_items.size() != m_maxCount; });

		m_items.push(std::forward<T>(e));
		m_notEmpty.notify_one();
	}

	T take() {
		std::unique_lock<std::mutex> l(m_lock);
		m_notEmpty.wait(l, [&]() { return m_items.size() != 0; });

		T r = std::move(m_items.front());
		m_items.pop();
		m_notFull.notify_one();
		return r;
	}

	void poll(T& out) {
		std::unique_lock<std::mutex> l(m_lock);
		if (!m_items.empty()) {
			out = std::move(m_items.front());
			m_items.pop();
			m_notFull.notify_one();
		}
	}

};
