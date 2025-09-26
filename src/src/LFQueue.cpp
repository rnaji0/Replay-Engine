#include "LFQueue.hpp"
#include <atomic>
#include <iostream> 


LFQueue::LFQueue() {
    Node* dummy = new Node(T());
    head = dummy;
    tail = dummy;
}


LFQueue::~LFQueue() {
    Node* curr = head.load();
    while (curr != nullptr) {
        Node* next = curr->next;
        delete curr;
        curr = next;
    }
}


void LFQueue::enqueue(const T& event) {
    Node* newNode = new Node(event);
    while (true) {
        Node* tailPtr = tail.load();
        Node* nextPtr = tailPtr->next.load();

        if (nextPtr == nullptr) {
            if (tail->next.compare_exchange_weak(nextPtr, newNode)) {
                queueSize.fetch_add(1, std::memory_order_relaxed);
                break;
            }
        } else {
            tail.compare_exchange_weak(tailPtr, nextPtr);
        }
    }
}


bool LFQueue::dequeue(T& result) {

    Node* oldHead = head.load();
    Node* realFirst = oldHead->next.load();

    if (realFirst == nullptr) {
        return false; 
    }

    if (head.compare_exchange_strong(oldHead, realFirst)) {
        result = realFirst->value;
        delete oldHead;
        queueSize.fetch_sub(1, std::memory_order_relaxed);
    } else {
        return false; 
    }

    return true;
}



