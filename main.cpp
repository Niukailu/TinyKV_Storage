#include <iostream>
#include "./src/skip_list.hpp"

#define DUMP_FILE_PATH "./store/dumpFile"


int main() {
    SkipList<std::string, std::string> skipList(6);
	// skipList.insert_element("11", "学"); 
	// skipList.insert_element("12", "算法"); 
	// skipList.insert_element("34", "认准"); 
	// skipList.insert_element("1aa", "代码"); 
	// skipList.insert_element("1ab", "学习"); 
	// skipList.insert_element("ab", "不迷路"); 
	// skipList.insert_element("abc", "关注"); 

    // skipList.dump_file();

    skipList.load_file();

    std::cout << "skipList size:" << skipList.get_size() << std::endl;

    skipList.search_element("34");
    skipList.search_element("aa");


    skipList.display_list();

    skipList.delete_element("1aa");
    skipList.delete_element("12");

    std::cout << "skipList size:" << skipList.get_size() << std::endl;

    skipList.display_list();

    return 0;
}

