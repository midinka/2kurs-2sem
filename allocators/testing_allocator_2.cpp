#include <iostream>
#include <string>
#include <cstring>
#include "memory.h"
#include "logger_builder_concrete.h"
class allocator final:public memory
{
private:
    logger *_log;

public:

    allocator(logger *log = nullptr): _log(log) {}

    void* allocate(size_t target_size) const override{
        auto * result =::operator new(sizeof(size_t) + target_size);
        *reinterpret_cast<size_t *>(result) = target_size;
        std::ostringstream oss;
        oss<<result;

        if (_log != nullptr)
        {
            _log->log(oss.str(), logger::severity::debug);
        }
        
        return reinterpret_cast<void *>(reinterpret_cast<size_t *>(result) + 1);
        // return reinterpret_cast<void *>(reinterpret_cast<unsigned char *>(result) + sizeof(size_t));
    }

    void deallocate(void* result) const override {
        result = reinterpret_cast<size_t *>(result) - 1;
        auto memory_size = *reinterpret_cast<size_t *>(result);
        std::string result_log = "[ ";

        for (int i = 0; i < memory_size; i++){
            unsigned char a = *(reinterpret_cast<unsigned char*>(result) + sizeof(size_t) + i);
            result_log = result_log + std::to_string(static_cast<unsigned short>(a)) + " ";
        }

        result_log = "Dump of block to deallocate: " + result_log + "]";

        if (_log != nullptr)
        {
            _log->log(result_log, logger::severity::debug);
        }

        :: operator delete(result);
    }
};


int main(){
    logger_builder* builder = new logger_builder_concrete();
    logger* mylogger = builder->add_stream("console", logger::severity::debug)->construct();
    delete builder;
    memory* alloc = new allocator(mylogger);

    int* arr = reinterpret_cast<int *>(alloc->allocate(sizeof(int) * 10));
    for (int i = 0; i < 10; i++){
        arr[i] = i*10;
        std::cout << arr[i] << std::endl;
    }
    alloc->deallocate(arr);

    char *str = reinterpret_cast<char *>(alloc->allocate(sizeof(char) * 11));
    std::strcpy(str, "0123456789");
    alloc->deallocate(str);

    delete alloc;
    delete mylogger;
    return 0;
}
