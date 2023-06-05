#include <iostream>
#include <fstream>
#include <cstdlib>
#include <mutex>
#include "./node.hpp"

#define FILE_PATH "./store/dumpFile"

std::mutex mtx;
std::string delimiter = ":";

template<typename K, typename V>
class SkipList
{
public:
    SkipList(int max_level);
    ~SkipList();

    int get_size();
    int get_random_level();

    Node<K, V>* create_node(K k, V v, int level);
    bool search_element(K k);
    bool insert_element(K k, V v);
    bool delete_element(K k);

    void display_list();
    void dump_file();
    void load_file();

private:
    void get_key_value_from_string(const std::string &str, std::string &key, std::string &value);
    bool is_valid_string(const std::string &str);

    //把字符串key转化成二进制字符串，用来比较
    K to_binary_string(const K& str) const {
        K binary_str;
        for (auto c : str) {
            unsigned char byte = static_cast<unsigned char>(c);
            for (int i = 0; i < 8; ++i) {
                binary_str += (byte & 0x80) ? '1' : '0';
                byte <<= 1;
            }
        }
        return binary_str;
    }

private:
    Node<K, V> *_header;

    int _max_level;
    int _skip_list_level;
    int _element_count;

    std::ofstream _file_writer;
    std::ifstream _file_reader;
};

template<typename K, typename V>
SkipList<K, V>::SkipList(int max_level) {
    this->_max_level = max_level;
    this->_skip_list_level = 0;
    this->_element_count = 0;  
    K k;
    V v;
    this->_header = create_node(k, v, max_level); 
}

template<typename K, typename V>
SkipList<K, V>::~SkipList() {
    if(_file_writer.is_open()) {
        _file_writer.close();
    }
    if(_file_reader.is_open()) {
        _file_reader.close();
    }
    //删除当前跳表所有节点
    Node<K, V> *tmp = NULL;
    while (_header->forward[0] != NULL)
    {
        tmp = _header->forward[0];
        _header->forward[0] = _header->forward[0]->forward[0];
        delete tmp;
    }
    
    delete _header;
}

template<typename K, typename V>
int SkipList<K, V>:: get_size() {
    return this->_element_count;
}

template<typename K, typename V>
int SkipList<K, V>::get_random_level() {
    int cnt = 1;
    srand(time(NULL));
    while (rand() % 2)
    {
        ++cnt;
    }
    cnt = cnt > _max_level ? _max_level : cnt;
    return cnt;
}

template<typename K, typename V>
Node<K, V>* SkipList<K, V>::create_node(K k, V v, int level) {
    K b_k = to_binary_string(k);
    Node<K, V> *node = new Node<K, V>(k, v, b_k, level);
    return node; 
}

template<typename K, typename V>
bool SkipList<K, V>::search_element(K k) {
    K b_k = to_binary_string(k);
    Node<K, V> *cur = _header;
    for (int i = _skip_list_level - 1; i >= 0; i--)
    {
        while (cur->forward[i] != NULL && cur->forward[i]->get_b_key() < b_k) {
            cur = cur->forward[i];
        }
    }
    cur = cur->forward[0];
    if (cur != NULL && cur->get_b_key() == b_k) {
        std::cout << "Found key: " << k << ", value: " << cur->get_value() << std::endl;
        return true;
    }
    std::cout << "Not found key: " << k << std::endl;
    return false;
}

template<typename K, typename V>
bool SkipList<K, V>::insert_element(K k, V v) {
    mtx.lock();

    K b_k = to_binary_string(k);
    Node<K, V> *cur = this->_header;

    Node<K, V> *update[_max_level];
    memset(update, 0, sizeof(Node<K, V>*) * _max_level);
    
    for (int i = _skip_list_level - 1; i >= 0; i--)
    {
        while (cur->forward[i] != NULL && cur->forward[i]->get_b_key() < b_k)
        {
            cur = cur->forward[i];
        }
        update[i] = cur;
    }

    cur = cur->forward[0];

    if (cur != NULL && cur->get_b_key() == b_k) {
        std::cout << "key: " << k << " have existed, update the value: " << cur->get_value() << " --> " << v << std::endl;
        cur->set_value(v);
        mtx.unlock();
        return false;
    }

    int random_level = get_random_level();

    if (random_level > _skip_list_level) {
        update[_skip_list_level] = _header;
        _skip_list_level += 1;
    }

    Node<K, V> *node = create_node(k, v, random_level);
    for (int i = 0; i < _skip_list_level; i++)
    {
        node->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = node;
    }
    std::cout << "Successfully inserted key:" << k << ", value:" << v << std::endl;
    ++_element_count;
    mtx.unlock();
    return true;
}

template<typename K, typename V>
bool SkipList<K, V>::delete_element(K k) {
    mtx.lock();

    K b_k = to_binary_string(k);
    Node<K, V> *cur = this->_header;
    Node<K, V> *update[_skip_list_level];
    memset(update, 0, sizeof(Node<K, V>*) * _skip_list_level);

    for (int i = _skip_list_level - 1; i >= 0; i--)
    {
        while (cur->forward[i] != NULL && cur->forward[i]->get_b_key() < b_k)
        {
            cur = cur->forward[i];
        }
        update[i] = cur;
    }
    
    cur = cur->forward[0];

    if (cur == NULL || cur->get_b_key() != b_k) {
        std::cout << "Failed, key: " << k << " not exist" << std::endl;
        mtx.unlock();
        return false;
    }

    for (int i = 0; i < _skip_list_level; i++)
    {
        if (update[i]->forward[i] != cur) break;
        update[i]->forward[i] = cur->forward[i];
    }
    while (_skip_list_level > 0 && _header->forward[_skip_list_level - 1] == NULL)
    {
        --_skip_list_level;
    }
    std::cout << "Successfully deleted key: "<< k << std::endl;
    --_element_count;
    delete cur;

    mtx.unlock();
    return true;
}

template<typename K, typename V>
void SkipList<K, V>::display_list() {
    std::cout << "\n*****Skip List*****" << std::endl; 
    for (int i = _skip_list_level - 1; i >= 0; i--)
    {
        Node<K, V> *node = this->_header->forward[i]; 
        std::cout << "Level " << i << ": ";
        while (node != NULL)
        {
            std::cout << "(" << node->get_key() << ", " << node->get_value() << ") ";
            node = node->forward[i];
        }
        std::cout << std::endl;
    }
    std::cout << "\n*******end*******" << std::endl; 
}

template<typename K, typename V>
void SkipList<K, V>::dump_file() {
    std::cout << "dump_file-----------------" << std::endl;
    this->_file_writer.open(FILE_PATH);

    Node<K, V> *node = this->_header->forward[0]; 
    while (node != NULL)
    {
        _file_writer << node->get_key() << delimiter << node->get_value() << "\n";
        std::cout << node->get_key() << delimiter << node->get_value() << "\n";
        node = node->forward[0];
    }
    _file_writer.flush();
    _file_writer.close();
}

template<typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string &str) {
    if (str.empty()) return false;
    if (str.find(delimiter) == std::string::npos) return false;
    return true;
}

template<typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(const std::string &str, std::string &key, std::string &value) {
    if (!is_valid_string(str)) return;

    key = str.substr(0, str.find(delimiter));
    value = str.substr(str.find(delimiter) + delimiter.length());
}

template<typename K, typename V>
void SkipList<K, V>::load_file() {
    std::cout << "load_file-----------------" << std::endl;
    this->_file_reader.open(FILE_PATH);

    std::string line = "";
    std::string key, value;
    while (getline(_file_reader, line))
    {
        get_key_value_from_string(line, key, value);
        if (key.empty() || value.empty()) {
            continue;
        }
        insert_element(key, value);
        std::cout << "key: " << key << " value: " << value << std::endl;
    }
    _file_reader.close();
}