#ifndef LFQUEUE_HPP
#define LFQUEUE_HPP

#include <atomic>
#include "event.hpp"

template <typename T>
class LFQueue {
public:
    struct Node {
        T value;
        std::atomic<Node*> next;
        Node(const T& v) : value(v), next(nullptr) {}
    };

    LFQueue();
    ~LFQueue();

    void enqueue(const T& event);
    bool dequeue(T& result);
    bool isEmpty() const;

private:
    std::atomic<Node*> head;
    std::atomic<Node*> tail;
    std::atomic<size_t> queueSize;
};


template <typename T>
LFQueue<T>::LFQueue() : queueSize(0) {
    Node* dummy = new Node(T{}); // dummy node
    head.store(dummy);
    tail.store(dummy);
}

template <typename T>
LFQueue<T>::~LFQueue() {
    Node* curr = head.load();
    while (curr) {
        Node* next = curr->next.load();
        delete curr;
        curr = next;
    }
}

template <typename T>
void LFQueue<T>::enqueue(const T& event) {
    Node* newNode = new Node(event);
    while (true) {
        Node* tailPtr = tail.load();
        Node* nextPtr = tailPtr->next.load();

        if (nextPtr == nullptr) {
            if (tailPtr->next.compare_exchange_weak(nextPtr, newNode)) {
                queueSize.fetch_add(1, std::memory_order_relaxed);
                break;
            }
        } else {
            tail.compare_exchange_weak(tailPtr, nextPtr);
        }
    }
}

template <typename T>
bool LFQueue<T>::dequeue(T& result) {
    Node* oldHead = head.load();
    Node* realFirst = oldHead->next.load();

    if (realFirst == nullptr) {
        return false;
    }

    if (head.compare_exchange_strong(oldHead, realFirst)) {
        result = realFirst->value;
        delete oldHead;
        queueSize.fetch_sub(1, std::memory_order_relaxed);
        return true;
    }
    return false;
}

template <typename T>
bool LFQueue<T>::isEmpty() const {
    return queueSize.load() == 0;
}

#endif 