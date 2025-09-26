#ifndef LFQUEUE_HPP
#define LFQUEUE_HPP

#include <atomic>
#include "Node.hpp"

template <typename T>
class LFQueue {
public:
    LFQueue();
    ~LFQueue();

    void push(const T& value);
    bool pop(T& result);

private:
    std::atomic<Node<T>*> head;
    std::atomic<Node<T>*> tail;
    std::atomic<size_t> queueSize;
};

#endif
