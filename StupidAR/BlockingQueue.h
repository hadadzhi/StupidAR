#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

/// Fixed-size, blocking queue with support for flushing
template <typename T>
class BlockingQueue {
private:
    bool m_flushing;
    size_t m_size;
    std::deque<T> m_items;
    std::mutex m_lock;
    std::condition_variable m_not_full;
    std::condition_variable m_not_empty;

public:
    explicit BlockingQueue(size_t size)
        : m_size(size), m_flushing(false) {
    }

    /// Blocks until not full or closed
    /// Returns `true` if `e` was succesfully added to the queue
    template <typename E, typename enable_if = typename std::enable_if_t<std::is_constructible_v<T, E>>>
    bool put(E&& e) {
        std::unique_lock<std::mutex> l(m_lock);
        m_not_full.wait(l, [&]() { return m_items.size() != m_size || m_flushing; });

        if (m_flushing) {
            return false;
        }

        m_items.push_back(std::forward<E>(e));
        m_not_empty.notify_one();

        return true;
    }

    /// Non-blocking put
    /// Returns `true` if `e` was succesfully added to the queue
    template <typename E, typename enable_if = typename std::enable_if_t<std::is_constructible_v<T, E>>>
    bool offer(E&& e) {
        std::unique_lock<std::mutex> l(m_lock);

        if (m_items.size() == m_size || m_flushing) {
            return false;
        }

        m_items.push_back(std::forward<E>(e));
        m_not_empty.notify_one();

        return true;
    }

    /// Blocks until not empty or closed
    /// Returns `true` if `out` was succesfully initialized from the front of the queue
    bool take(T& out) {
        std::unique_lock<std::mutex> l(m_lock);
        m_not_empty.wait(l, [&]() { return m_items.size() != 0 || m_flushing; });

        if (m_flushing) {
            return false;
        }

        out = std::move(m_items.front());
        m_items.pop_front();
        m_not_full.notify_one();

        return true;
    }

    /// Non-blocking take
    /// Returns `true` if `out` was succesfully initialized from the front of the queue
    bool poll(T& out) {
        std::unique_lock<std::mutex> l(m_lock);

        if (!m_items.empty()) {
            out = std::move(m_items.front());
            m_items.pop_front();
            m_not_full.notify_one();

            return true;
        }

        return false;
    }

    /// Closes the queue (adding/retrieving items will fail until `end_flush()` is called)
    /// Removes all enqueued items
    /// Releases all threads currently blocked in `take()` or `put()`
    void begin_flush() {
        std::unique_lock<std::mutex> l(m_lock);

        if (m_flushing) {
            return;
        }

        m_flushing = true;
        m_items.clear();

        m_not_full.notify_all();
        m_not_empty.notify_all();
    }

    /// Opens the queue
    void end_flush() {
        std::unique_lock<std::mutex> l(m_lock);
        m_flushing = false;
    }
};
