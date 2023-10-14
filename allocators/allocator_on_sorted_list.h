#ifndef COURSEWORK_SORTED_LIST_H
#define COURSEWORK_SORTED_LIST_H

#include <iostream>
#include "memory.h"

class allocator_sorted_list final : public memory{

public:
    enum class types_of_detour{
        first,
        best,
        worst
    };

private:
    void* global_memory;

    size_t* get_global_size() const{
        return reinterpret_cast<size_t*>(global_memory);
    }

    logger** get_logger() const{
        return reinterpret_cast<logger**>(reinterpret_cast<size_t*>(global_memory)+1);
    }

    void** get_first_free_block() const{
        return reinterpret_cast<void**>(reinterpret_cast<types_of_detour*>(reinterpret_cast<memory**>(reinterpret_cast<logger**>(reinterpret_cast<size_t*>(global_memory)+1)+1)+1)+1);
    }

    types_of_detour* get_type_of_detour() const{
        return reinterpret_cast<types_of_detour*>(reinterpret_cast<memory**>(reinterpret_cast<logger**>(reinterpret_cast<size_t*>(global_memory)+1)+1)+1);
    }

    size_t* get_size_of_block(void* block) const{
        return reinterpret_cast<size_t*>(block);
    }

    void** get_next_to_block(void* block) const{
        return reinterpret_cast<void**>(get_size_of_block(block) + 1);
    }

    void merge() const{
        logger* log = *get_logger();
        if (log != nullptr){
            log->log("Merging...", logger::severity::debug);
        }

        auto* cur_block = *get_first_free_block();
        void* next = nullptr;
        while(cur_block != nullptr){
            next = *get_next_to_block(cur_block);

            if((reinterpret_cast<unsigned char*>(next) - reinterpret_cast<unsigned char*>(cur_block)) == *get_size_of_block(cur_block)){
                auto** next_next = reinterpret_cast<void**>(reinterpret_cast<size_t*>(next) + 1);
                *get_next_to_block(cur_block) = *next_next;
                *get_size_of_block(cur_block) = (*get_size_of_block(cur_block)) + (*get_size_of_block(next));
                *get_next_to_block(next) = nullptr;
                *get_size_of_block(next) = 0;
                *next_next = nullptr;
            }else{
                cur_block = next;
            }
        }
        if (log != nullptr){
            log->log("Merging finished", logger::severity::debug);
        }
    }


public:
    //size-log-alloc-detour-void*
    allocator_sorted_list(size_t size, types_of_detour det = types_of_detour::first, logger* log = nullptr, memory* alloc = nullptr){
        if (log != nullptr){
            log->log("allocator_sorted_list has began its work", logger::severity::debug);
        }
        if (alloc == nullptr){
            global_memory = ::operator new(size + sizeof(types_of_detour) + sizeof(void*) + sizeof(memory*) + sizeof(logger*) + sizeof(size_t));
        }else{
            global_memory = alloc->allocate(size + sizeof(types_of_detour) + sizeof(void*) + sizeof(memory*) + sizeof(logger*) + sizeof(size_t));
        }
        *reinterpret_cast<size_t*>(global_memory) = size;
        *reinterpret_cast<logger**>(reinterpret_cast<size_t*>(global_memory)+1) = log;
        *reinterpret_cast<memory**>(reinterpret_cast<logger**>(reinterpret_cast<size_t*>(global_memory)+1)+1) = alloc;
        *reinterpret_cast<types_of_detour*>(reinterpret_cast<memory**>(reinterpret_cast<logger**>(reinterpret_cast<size_t*>(global_memory)+1)+1)+1) = det;

        auto* block =  reinterpret_cast<void**>(reinterpret_cast<void**>(reinterpret_cast<types_of_detour*>(reinterpret_cast<memory**>(reinterpret_cast<logger**>(reinterpret_cast<size_t*>(global_memory)+1)+1)+1)+1)+1);
        *reinterpret_cast<void**>(reinterpret_cast<types_of_detour*>(reinterpret_cast<memory**>(reinterpret_cast<logger**>(reinterpret_cast<size_t*>(global_memory)+1)+1)+1)+1) = block;

        *reinterpret_cast<size_t*>(block) = size - sizeof(size_t) - sizeof(logger*) - sizeof(memory*) - sizeof(types_of_detour) - sizeof(void*);
        *reinterpret_cast<void**>(reinterpret_cast<size_t*>(block)+1) = nullptr;
        if (log != nullptr){
            log->log("allocator_sorted_list was called(constructor finished)", logger::severity::debug);
        }
    }

    void* allocate(size_t target_size) const override{
        logger* log = *get_logger();
        if (log != nullptr){
            log->log("Allocating with allocator on sorted list has begun", logger::severity::debug);
        }
        size_t need_size = target_size + sizeof(size_t) + sizeof(void*);
        void* cur_block = *get_first_free_block();
        void* prev_cur_block = nullptr;
        void* res_block = nullptr;
        void* prev_res_block = nullptr;

        if(*get_type_of_detour() == types_of_detour::first){
            while(cur_block != nullptr){
                if (*get_size_of_block(cur_block) >= need_size){
                    res_block = cur_block;
                    prev_res_block = prev_cur_block;
                    break;
                }
                prev_cur_block = cur_block;
                cur_block = *get_next_to_block(cur_block);
            }
        }else if(*get_type_of_detour() == types_of_detour::best){
            size_t res_size = *get_global_size();
            while(cur_block != nullptr){
                if(((*get_size_of_block(cur_block)) >= need_size) && (*get_size_of_block(cur_block)) < res_size){
                    res_block = cur_block;
                    prev_res_block = prev_cur_block;
                    res_size = *get_size_of_block(cur_block);
                }
                prev_cur_block = cur_block;
                cur_block = *get_next_to_block(cur_block);
            }
        }else if(*get_type_of_detour() == types_of_detour::worst){
            size_t res_size = 0;
            while(cur_block != nullptr){
                if((*get_size_of_block(cur_block)) >= need_size && (*get_size_of_block(cur_block)) > res_size){
                    res_block = cur_block;
                    prev_res_block = prev_cur_block;
                    res_size = *get_size_of_block(cur_block);
                }
                prev_cur_block = cur_block;
                cur_block = *get_next_to_block(cur_block);
            }
        }

        if (res_block == nullptr) throw std::bad_alloc();

        size_t size_of_res_block = *get_size_of_block(res_block);

        if (prev_res_block == nullptr){
            if (size_of_res_block - need_size < sizeof(void*) + sizeof(size_t)){
                *get_first_free_block() = *get_next_to_block(res_block);
            }else{
                auto* new_next_block = reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(res_block) + need_size);
                *get_size_of_block(res_block) = target_size;
                *get_size_of_block(new_next_block) = size_of_res_block - need_size;
                *get_next_to_block(new_next_block) = *get_next_to_block(res_block);
                *get_first_free_block() = new_next_block;
            }
            *get_next_to_block(res_block) = nullptr;
        }else{
            if (size_of_res_block - need_size < sizeof(void*) + sizeof(size_t)){
                *get_next_to_block(prev_res_block) = *get_next_to_block(res_block);
            }else{
                auto* new_next_block = reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(res_block) + need_size);
                *get_size_of_block(res_block) = target_size;
                *get_size_of_block(new_next_block) = size_of_res_block - need_size;
                *get_next_to_block(new_next_block) = *get_next_to_block(res_block);
                *get_next_to_block(prev_res_block) = new_next_block;
            }
            *get_next_to_block(res_block) = nullptr;
        }
        if (log != nullptr){
            log->log("Allocating with allocator on sorted list has finished successfully!", logger::severity::debug);
        }

        return reinterpret_cast<void*>(reinterpret_cast<size_t*>(res_block) + 1);
    }

    void deallocate(void* block_to_delete) const override{
        logger* log = *get_logger();
        if (log != nullptr){
            log->log("Deallocating with allocator on sorted list has begun", logger::severity::debug);
        }
        if (block_to_delete == nullptr) return;

        auto* block = reinterpret_cast<size_t*>(block_to_delete) - 1;
        auto* cur_block = *get_first_free_block();
        void* prev_block = nullptr;
        size_t size = *get_size_of_block(block);
        std::string result_log = "[ ";
        for (int i = 0; i < size; i++) {
            unsigned char a = *(reinterpret_cast<unsigned char *>(block_to_delete) + i);
            result_log += std::to_string(static_cast<unsigned short>(a));
            result_log += " ";
        }
        std::string result_msg = "Block to deallocate: " + result_log + "]";
        if (log != nullptr) {
            log->log(result_msg, logger::severity::debug);
        }

        while ((reinterpret_cast<unsigned char*>(block) - reinterpret_cast<unsigned char*>(cur_block) > 0) && cur_block!=
                                                                                                              nullptr){
            prev_block = cur_block;
            cur_block = *get_next_to_block(cur_block);
        }

        if (prev_block == nullptr){
            *get_first_free_block() = block;
            *get_next_to_block(block) = cur_block;
        }else{
            *get_next_to_block(prev_block) = block;
            *get_next_to_block(block) = cur_block;
        }
        merge();
        if (log != nullptr){
            log->log("Deallocating with allocator on sorted list has finished", logger::severity::debug);
        }
    }

    ~allocator_sorted_list () final{
        logger* log = *get_logger();
        auto* alloc = *(reinterpret_cast<memory**>(get_logger() + 1));
        if (log != nullptr){
            log->log("Deallocating allocator started", logger::severity::debug);
        }
        if (alloc == nullptr){
            ::operator delete(global_memory);
        }else{
            alloc->deallocate(global_memory);
        }
        if (log != nullptr){
            log->log("Deallocating allocator finished", logger::severity::debug);
        }
    }

};

#endif //COURSEWORK_SORTED_LIST_H
