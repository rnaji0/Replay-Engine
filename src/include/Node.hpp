#ifndef NODE.HPP
#define NODE_HPP

template <typename T>
struct Node {
    T value;
    Node* next;
    Node(const T& val) : value(val), next(nullptr) {};

};

#endif