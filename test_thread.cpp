#include <iostream>
#include <thread>

using namespace std;

void thread_func() {
    cout << "Hello from thread\n";
}

int main() {
    thread t(thread_func);
    t.join();
    return 0;
}