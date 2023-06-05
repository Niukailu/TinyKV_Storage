#include <cstring>


template<typename K, typename V>
class Node 
{
public:
    Node() {}
    Node(K k, V v, K b_k, int level);

    ~Node();

    K get_key() const;

    K get_b_key() const;

    V get_value() const;

    void set_value(V v);

    Node<K, V> **forward;

private:
    K key, binary_key;
    V value;
};

template<typename K, typename V>
Node<K, V>::Node(const K k, const V v, const K b_k, int level) {
    this->key = k;
    this->value = v;
    this->binary_key = b_k;
    this->forward = new Node<K, V>*[level];

    //将指针赋值为 0 实际上是将其赋值为空指针NULL
    //在大多数操作系统中，地址为 0 的内存空间是操作系统保留的，用于表示指针未初始化或者指向空地址
    memset(this->forward, 0, sizeof(Node<K, V>*) * level);
}

template<typename K, typename V>
Node<K, V>::~Node() {
    delete []forward;
}

template<typename K, typename V>
K Node<K, V>::get_key() const {
    return this->key;
}

template<typename K, typename V>
K Node<K, V>::get_b_key() const {
    return this->binary_key;
}

template<typename K, typename V>
V Node<K, V>::get_value() const {
    return this->value;
}

template<typename K, typename V>
void Node<K, V>::set_value(V v){
    this->value = v;
}



