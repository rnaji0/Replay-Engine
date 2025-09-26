#include "LFQueue.hpp"
#include <atomic>
#include <iostream> 


LFQueue::LFQueue() {
   
}


LFQueue::~LFQueue() {

}


void LFQueue::push(const T& event) {

}


bool LFQueue::pop(T& result) {

    return false; 
}


bool LFQueue::isEmpty() const {
    return queueSize == 0;
}
