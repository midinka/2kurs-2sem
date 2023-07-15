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
