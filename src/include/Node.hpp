#ifndef NODE_HPP
#define NODE_HPP

template <typename T>
struct Node {
    T value;
    std::atomic<Node*> next;
    Node(const T& val) : value(val), next(nullptr) {}
};

#endif