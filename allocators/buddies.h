
#ifndef COURSEWORK_BUDDIES_H
#define COURSEWORK_BUDDIES_H

#include <iostream>
#include "memory.h"
#include <cmath>

//size-logger-memory-firstfree
//size-nextfree-prevfree

class allocator_buddies final: public memory{

private:

    void* global_memory;

    logger** get_logger() const{
        return reinterpret_cast<logger**>(reinterpret_cast<int*>(global_memory)+1);
    }

    memory** get_allocator() const{
        return reinterpret_cast<memory**>(reinterpret_cast<logger**>(reinterpret_cast<int*>(global_memory)+1)+1);
    }

    void** get_pointer_to_first_free() const{
        return reinterpret_cast<void**>(reinterpret_cast<memory**>(reinterpret_cast<logger**>(reinterpret_cast<int*>(global_memory)+1)+1) + 1);
    }

    static unsigned char* get_size_of_block(void* block){
        return reinterpret_cast<unsigned char*>(block);
    }

    static void** get_pointer_to_next(void* block){
        return reinterpret_cast<void**>(reinterpret_cast<unsigned char*>(block) + 1);
    }

    static void** get_pointer_to_prev(void* block){
        return reinterpret_cast<void**>(reinterpret_cast<unsigned char*>(block) + 1) + 1;
    }

    static bool get_used_status(void* block){
        if ((static_cast<int>(*get_size_of_block(block)) & 1) == 0){
            return true;// block is free
        }else{
            return false;
        }
    }

    static void switch_used_status(void* block){
        if (get_used_status(block)){
            *reinterpret_cast<unsigned char*>(block) = static_cast<unsigned char>(static_cast<int>(*reinterpret_cast<unsigned char*>(block)) | 1);
        }else{
            *reinterpret_cast<unsigned char*>(block) = static_cast<unsigned char>((static_cast<int>(*reinterpret_cast<unsigned char*>(block)) >> 1) << 1);
        }
    }

    void* get_buddy(void* block)const{
        size_t degree = static_cast<int>(*get_size_of_block(block) >> 1 );
        if (*reinterpret_cast<int*>(global_memory) == degree) return nullptr;

        auto * const start_allocated_memory = get_pointer_to_first_free() + 1;
        size_t size = 1 << degree;

        size_t relative_address = reinterpret_cast<unsigned char*>(block) - reinterpret_cast<unsigned char*>(start_allocated_memory);
        size_t relative_address_buddies = relative_address ^ size;

        return reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(start_allocated_memory) + relative_address_buddies);
    }

public:

    explicit allocator_buddies(size_t size, logger* log = nullptr, memory* alloc = nullptr){
        if (log != nullptr){
            log->log("allocator_buddies has began its work", logger::severity::debug);
        }

        int degree = static_cast<int>(std::ceil(log2(size)));

        if (alloc == nullptr){
            global_memory = ::operator new(size + sizeof(int) + sizeof(void*) + sizeof(memory*) + sizeof(logger*));
        }else{
            alloc->allocate(size + sizeof(int) + sizeof(void*) + sizeof(memory*) + sizeof(logger*));
        }

        *reinterpret_cast<int*>(global_memory) = degree;
        *reinterpret_cast<logger**>(reinterpret_cast<int*>(global_memory)+1) = log;
        *reinterpret_cast<memory**>(reinterpret_cast<logger**>(reinterpret_cast<int*>(global_memory)+1)+1) = alloc;

        auto* first_block = reinterpret_cast<void**>(reinterpret_cast<memory**>(reinterpret_cast<logger**>(reinterpret_cast<int*>(global_memory)+1)+1)+1) + 1;
        *reinterpret_cast<void**>(reinterpret_cast<memory**>(reinterpret_cast<logger**>(reinterpret_cast<int*>(global_memory)+1)+1)+1) = first_block;
        *reinterpret_cast<unsigned char*>(first_block) = degree;
        *reinterpret_cast<void**>(reinterpret_cast<unsigned char*>(first_block) + 1) = nullptr;
        *reinterpret_cast<void**>(reinterpret_cast<void**>(reinterpret_cast<unsigned char*>(first_block) + 1) + 1) = nullptr;

    }

    void* allocate(size_t tar_size) const override{
        logger* log = *get_logger();
        if (log != nullptr){
            log->log("Starting allocate with buddies allocator", logger::severity::debug);
        }

        size_t need_size = tar_size + sizeof(void*)*2 + sizeof(unsigned char);
        auto* cur_block = *get_pointer_to_first_free();
        while (cur_block != nullptr){
            int degree = static_cast<int>((*get_size_of_block(cur_block)) >> 1);

            if (need_size <= (1 << degree)){
                auto* prev_to_cur_block = *get_pointer_to_prev(cur_block);
                auto* next_to_cur_block = *get_pointer_to_next(cur_block);

                while (((1<<degree) >> 1) > need_size) {
                    degree = degree - 1;
                    void* buddy = reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(cur_block) + (1 << degree));

                    if (next_to_cur_block != nullptr) *get_pointer_to_prev(next_to_cur_block) = buddy;
                    *get_size_of_block(buddy) = degree * 2;
                    *get_pointer_to_next(buddy) = reinterpret_cast<void*>(next_to_cur_block);
                    *get_pointer_to_prev(buddy) = reinterpret_cast<void*>(cur_block);

                    *get_size_of_block(cur_block) = degree * 2;
                    *get_pointer_to_next(cur_block) = reinterpret_cast<void*>(buddy);
                    next_to_cur_block = buddy;


                }

                switch_used_status(cur_block);

                if (prev_to_cur_block == nullptr) *get_pointer_to_first_free() = next_to_cur_block;
                else *get_pointer_to_next(prev_to_cur_block) = next_to_cur_block;

                if (next_to_cur_block != nullptr){
                    *get_pointer_to_prev(next_to_cur_block) = prev_to_cur_block;
                }

                if(log!=nullptr){
                    log->log("Allocating with buddies was finished successfully", logger::severity::debug);
                }
                return get_size_of_block(cur_block) + 1;

            }
            cur_block = *get_pointer_to_next(cur_block);
        }

        if (log!= nullptr){
            log->log("We couldn't allocate memory...", logger::severity::debug);
        }
        return nullptr;


    }

    void deallocate(void* target)const override{
        logger* log = *get_logger();
        if (log != nullptr){
            log->log("Starting deallocate in buddies allocator", logger::severity::debug);
        }

        if (target == nullptr) return;

        auto* cur_block = reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(target) - 1);
        //

//        size_t size = static_cast<int>((*get_size_of_block(cur_block)) >> 1);
//        std::string result_log = "[ ";
//        for (int i = 0; i < 1<<size; i++) {
//            unsigned char a = *(reinterpret_cast<unsigned char *>(target) + i);
//            result_log += std::to_string(static_cast<unsigned short>(a));
//            result_log += " ";
//        }
//        std::string result_msg = "Block to deallocate: " + result_log + "]";
//        if (log != nullptr) {
//            log->log(result_msg, logger::severity::debug);
//        }
        //

        void* prev_block = nullptr;
        void* next_block = *get_pointer_to_first_free();
        while (next_block != nullptr && (reinterpret_cast<unsigned char*>(cur_block) - reinterpret_cast<unsigned char*>(next_block) > 0)){
            prev_block = next_block;
            next_block = *get_pointer_to_next(next_block);
        }
        switch_used_status(cur_block);

        *get_pointer_to_next(cur_block) = next_block;
        *get_pointer_to_prev(cur_block) = prev_block;

        (prev_block == nullptr) ? *get_pointer_to_first_free() = cur_block : *get_pointer_to_next(prev_block) = cur_block;

        if (next_block != nullptr) *get_pointer_to_prev(next_block) = cur_block;

        void* buddy = get_buddy(cur_block);

        while (buddy != nullptr && get_used_status(buddy) && static_cast<int>((*get_size_of_block(cur_block)) >> 1) == static_cast<int>((*get_size_of_block(buddy)) >> 1)){
            if (reinterpret_cast<unsigned char*>(buddy) - reinterpret_cast<unsigned char*>(cur_block) < 0){
                auto *tmp = cur_block;
                cur_block = buddy;
                buddy = tmp;
            }

            *get_pointer_to_next(cur_block) = *get_pointer_to_next(buddy);
            *get_size_of_block(cur_block) = 2 * (static_cast<int>((*get_size_of_block(cur_block)) >> 1) + 1);

            if (*get_pointer_to_next(buddy) != nullptr) *get_pointer_to_prev(*get_pointer_to_next(buddy)) = cur_block;

            buddy = get_buddy(cur_block);
        }
        if (log != nullptr ){
            log->log("Deallocating in buddies allocator finished", logger::severity::debug);
        }



    }

    ~allocator_buddies() override{
        logger* log = *get_logger();
        memory* alloc = *get_allocator();
        if (log != nullptr){
            log->log("Destructing buddies allocator", logger::severity::debug);
        }

        (alloc == nullptr) ? ::operator delete(global_memory) : alloc->deallocate(global_memory);

        if (log != nullptr){
            log->log("Destructing buddies allocator finished", logger::severity::debug);
        }


    }





};


#endif //COURSEWORK_BUDDIES_H
