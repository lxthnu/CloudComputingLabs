// main.c
#include <iostream>

#include "threadpool.h"
#include <memory>
#include <vector>
#include <unistd.h>
using namespace mythreadpool;
void printString(const std::string &str) {
    std::cout << str << std::endl;
    usleep(100 * 100);
}

void test(int maxSize) {
    ThreadPool pool("pool");
    pool.setqSize(maxSize);
    pool.start(5);
    for (int i = 0; i < 100; ++i) {
        std::string buf = "task" + std::to_string(i);
        pool.run(std::bind(printString, buf));
    }
    pool.stop();
    std::cout << "Done" << std::endl;
}

int main() {
    test(10);
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
