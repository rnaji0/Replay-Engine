#ifndef NODE.HPP
#define NODE_HPP

template <typename T>
struct Node {
    Event value;
    Node* next;
    Node(const Event& val) : value(val), next(nullptr) {};

};

#endif