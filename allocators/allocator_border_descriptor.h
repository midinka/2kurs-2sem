
#ifndef COURSEWORK_BORDER_DESCRIPTOR_H
#define COURSEWORK_BORDER_DESCRIPTOR_H

#include <iostream>
#include "allocator_2.h"
#include <list>
#include <random>

#include <iostream>
#include <list>
#include <random>
#include "../logger/logger.h"
#include "memory.h"

//alloc log size det void*

class allocator_bord final : public memory{

public:

    enum class detour_type{
        first,
        best,
        worst
    };

private:

    void* global_memory;

    size_t* get_size() const {
        return reinterpret_cast<size_t*>(reinterpret_cast<logger**>(reinterpret_cast<memory**>(global_memory)+1)+1);
    }

    memory** get_memory() const {
        return reinterpret_cast<memory**>(global_memory);
    }

    logger** get_logger() const {
        return reinterpret_cast<logger**>(reinterpret_cast<memory**>(global_memory)+1);
    }

    detour_type* get_detour_type() const {
        return reinterpret_cast<detour_type*>(reinterpret_cast<size_t*>(reinterpret_cast<logger**>(reinterpret_cast<memory**>(global_memory)+1)+1)+1);
    }

    void** get_first_taken() const {
        return reinterpret_cast<void**>(reinterpret_cast<detour_type*>(reinterpret_cast<size_t*>(reinterpret_cast<logger**>(reinterpret_cast<memory**>(global_memory)+1)+1)+1)+1);
    }

    size_t get_service_part_size() const noexcept {
        return sizeof(memory*) + sizeof(logger*) + sizeof(size_t) + sizeof(allocator_bord::detour_type) + sizeof(void *);
    }


public:

    explicit allocator_bord(size_t size, logger* log = nullptr, memory* alloc = nullptr, detour_type det = detour_type::first){

        if (alloc == nullptr){
            global_memory = ::operator new (size + get_service_part_size());

        }
        else {
            global_memory = alloc->allocate(size + get_service_part_size());
        }


        *reinterpret_cast<memory**>(global_memory) = alloc;
        *reinterpret_cast<logger**>(reinterpret_cast<memory**>(global_memory)+1) = log;
        *reinterpret_cast<size_t*>(reinterpret_cast<logger**>(reinterpret_cast<memory**>(global_memory)+1)+1) = size;
        *reinterpret_cast<allocator_bord::detour_type*>(reinterpret_cast<size_t*>(reinterpret_cast<logger**>(reinterpret_cast<memory**>(global_memory)+1)+1)+1) = det;
        *reinterpret_cast<void**>(reinterpret_cast<allocator_bord::detour_type*>(reinterpret_cast<size_t*>(reinterpret_cast<logger**>(reinterpret_cast<memory**>(global_memory)+1)+1)+1)+1) = nullptr;

        //std::cout<<"--------------------------------"<<std::endl;
        //dump_allocator_state(false);
    }

private:

    size_t* get_block_size(void* ptr_to_block) const{
        return reinterpret_cast<size_t*>(ptr_to_block);
    }

    void** get_block_next(void* ptr_to_block) const{
        return reinterpret_cast<void**>(reinterpret_cast<size_t*>(ptr_to_block)+1);
    }

    void** get_block_prev(void* ptr_to_block) const{
        return reinterpret_cast<void**>(reinterpret_cast<size_t*>(ptr_to_block)+1)+1;
    }

    unsigned char block_service_size() const{
        return sizeof(size_t) + sizeof(void*) * 2;
    }

public:

    void* allocate(size_t tarsize) const override {
        if (*get_first_taken() == nullptr){
            if (tarsize + sizeof(size_t) + 2*sizeof(void*) <= *get_size()){
                auto block = reinterpret_cast<void*>(get_first_taken() + 1);
                *get_block_size(block) = tarsize;
                *get_block_next(block) = nullptr;
                *get_block_prev(block) = nullptr;
                *get_first_taken() = block;
                //dump_allocator_state(true);
                return reinterpret_cast<void*>(get_block_prev(block) + 1);
            }else{
                //return nullptr;
                throw std::logic_error("You want too much place!");
            }
        }

        void* res_block = nullptr;
        void* res_block_next = nullptr;
        void* res_block_prev = nullptr;
        size_t res_block_size_best = *get_size();
        size_t res_block_size_worst = 0;

        size_t current_free_block_size = static_cast<size_t>(reinterpret_cast<unsigned char*>(*get_first_taken()) - reinterpret_cast<unsigned char*>(get_first_taken() + 1));

        if (current_free_block_size >= tarsize + block_service_size())
        {
            res_block = reinterpret_cast<void*>(get_first_taken() + 1);
            res_block_next = *get_first_taken();
            res_block_prev = nullptr;
            res_block_size_best = res_block_size_worst = current_free_block_size;
        }

        auto ptr_to_next = get_block_next(*get_first_taken());
        auto detour_type = *get_detour_type();

        while (*ptr_to_next != nullptr)
        {
            current_free_block_size = reinterpret_cast<unsigned char*>(*ptr_to_next) - (reinterpret_cast<unsigned char*>(ptr_to_next+2)+*get_block_size(reinterpret_cast<unsigned char*>(ptr_to_next) -sizeof(size_t)));

            if (current_free_block_size >= tarsize + block_service_size())
            {
                if (detour_type == detour_type::first || detour_type == detour_type::best && (current_free_block_size < res_block_size_best) || detour_type == detour_type::worst && (current_free_block_size > res_block_size_worst)){
                    res_block = reinterpret_cast<unsigned char*>(ptr_to_next+2)+*get_block_size(reinterpret_cast<unsigned char*>(ptr_to_next) -sizeof(size_t));
                    res_block_next = *ptr_to_next;
                    res_block_prev = reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(ptr_to_next) -sizeof(size_t));
                    res_block_size_best = res_block_size_worst = current_free_block_size;
                    if (detour_type == detour_type::first){
                        break;
                    }
                }
            }

            ptr_to_next = reinterpret_cast<void**>(reinterpret_cast<size_t*>(*ptr_to_next) + 1);

        }

        auto last_free_block_size = reinterpret_cast<unsigned char*>(get_first_taken() + 1)
                                    + *get_size()
                                    - (reinterpret_cast<unsigned char*>(ptr_to_next + 2) + *get_block_size(reinterpret_cast<unsigned char*>(ptr_to_next) - sizeof(size_t)));
        if (detour_type == detour_type::first && res_block == nullptr
            || detour_type == detour_type::best && last_free_block_size > current_free_block_size
            || detour_type == detour_type::worst && last_free_block_size > current_free_block_size)
        {
            if (last_free_block_size >= tarsize + block_service_size()){
                res_block = reinterpret_cast<unsigned char*>(ptr_to_next + 2) + *get_block_size((reinterpret_cast<unsigned char*>(ptr_to_next) - sizeof(size_t)));
                res_block_next = nullptr;
                res_block_prev = reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(ptr_to_next) -sizeof(size_t));
            }
        }

        //---------------------------------------------------------//
        if (res_block == nullptr){
            //return nullptr;
            throw std::logic_error("Error!");
        }



        auto block = res_block;
        *get_block_size(block) = tarsize;
        *get_block_next(block) = res_block_next;
        *get_block_prev(block) = res_block_prev;

        if (res_block_prev != nullptr)
        {
            *get_block_next(res_block_prev) = res_block;
        }
        else
        {
            *get_first_taken() = res_block;
        }

        if (res_block_next != nullptr)
        {
            *get_block_prev(res_block_next) = res_block;
        }

        //dump_allocator_state(true);

        //std::cout << *get_block_size(res_block) << std::endl;
        return reinterpret_cast<void *>(reinterpret_cast<unsigned char *>(res_block) + block_service_size());
    }

    void deallocate(void* result) const override{

        if (result == nullptr) {
            return;
        }


        auto block = reinterpret_cast<void *>(reinterpret_cast<unsigned char *>(result) - sizeof(void *) * 2 - sizeof(size_t));
        size_t size = *get_block_size(block);

        if ((*get_block_next(block) == nullptr) && (*get_block_prev(block) == nullptr)) {
            *get_first_taken() = nullptr;
        } else if ((*get_block_next(block) != nullptr) && (*get_block_prev(block) == nullptr)) {
            *get_block_prev(*get_block_next(block)) = nullptr;
            *get_first_taken() = *get_block_next(block);
        } else if ((*get_block_prev(block) != nullptr) && (*get_block_next(block) == nullptr)) {
            *get_block_next(*get_block_prev(block)) = nullptr;
        } else if ((*get_block_prev(block) != nullptr) && (*get_block_next(block) != nullptr)) {
            //std::cout << *get_block_size(block) << "--------------" << *get_block_prev(block) << std::endl;
            *get_block_prev(*get_block_next(block)) = *get_block_prev(block);
            *get_block_next(*get_block_prev(block)) = *get_block_next(block);
        }


        dump_allocator_state(false);

        std::string result_log = "[ ";
        for (int i = 0; i < size; i++) {
            unsigned char a = *(reinterpret_cast<unsigned char *>(result) + i);
            result_log += std::to_string(static_cast<unsigned short>(a));
            result_log += " ";
        }
        std::string result_msg = "Block to deallocate: " + result_log + "]";
        if (*get_logger() != nullptr) {
            (*get_logger())->log(result_msg, logger::severity::debug);
        }
    }

    void dump_allocator_state(bool is_allocate) const noexcept
    {
        auto *log = *get_logger();
        if (log == nullptr)
        {
            return;
        }

        auto allocator_service_part_size = get_service_part_size();

        std::string constructed_state_string("|");
        auto *current_occupied_block = *get_first_taken();

        if (current_occupied_block == nullptr)
        {
            constructed_state_string += "avlbl " + std::to_string(*get_size()) + '|';
        }
        else
        {
            void *previous_occupied_block = nullptr;

            while (true)
            {
                auto block_address_and_size = get_aviable_block_address_and_size(previous_occupied_block, current_occupied_block);
                auto aviable_block_address = block_address_and_size.first;
                auto aviable_block_size = block_address_and_size.second;

                if (aviable_block_address != nullptr && aviable_block_address != *get_first_taken() && aviable_block_size != 0)
                {
                    constructed_state_string += "avlbl " + std::to_string(aviable_block_size) + '|';
                }

                if (current_occupied_block != nullptr)
                {
                    constructed_state_string +=
                            "occup " +
                            std::to_string(*get_block_size(current_occupied_block) + sizeof(void *) * 2 + sizeof(size_t)) +
                            '|';
                }
                else
                {
                    break;
                }

                previous_occupied_block = current_occupied_block;
                current_occupied_block = *get_block_next(current_occupied_block);
            }
        }

        log->log(std::string("Memory state after ") + (is_allocate ? "" : "de") + "allocation operation: " + constructed_state_string, logger::severity::debug);
    }

    std::pair<void *, size_t> get_aviable_block_address_and_size(
            void * const previous_occupied_block_address,
            void * const next_occupied_block_address) const
    {
        size_t serv_occup_block_size = block_service_size();
        size_t allocator_size = *get_size();
        size_t service_part_allocator_size = get_service_part_size();
        auto previous_occupied_block_size = previous_occupied_block_address == nullptr
                                            ? 0
                                            : *get_block_size(previous_occupied_block_address);
        size_t block_size = 0;
        void *block_address = nullptr;

        if (previous_occupied_block_address == nullptr && next_occupied_block_address == nullptr)
        {
            block_address = nullptr;
            block_size = 0;
        }
        else if (previous_occupied_block_address == nullptr)
        {
            block_address = reinterpret_cast<void *>(reinterpret_cast<unsigned char *>(global_memory) + service_part_allocator_size);
            block_size = reinterpret_cast<unsigned char *>(next_occupied_block_address) - reinterpret_cast<unsigned char *>(block_address);
        }
        else if (next_occupied_block_address == nullptr)
        {
            block_address = reinterpret_cast<unsigned char *>(previous_occupied_block_address) + serv_occup_block_size + previous_occupied_block_size;
            block_size = reinterpret_cast<unsigned char *>(global_memory) + service_part_allocator_size + allocator_size - reinterpret_cast<unsigned char *>(block_address);
        }
        else
        {
            block_address = reinterpret_cast<unsigned char *>(previous_occupied_block_address) + serv_occup_block_size + previous_occupied_block_size;
            block_size = reinterpret_cast<unsigned char *>(next_occupied_block_address) - reinterpret_cast<unsigned char *>(block_address);
        }

        return { block_address, block_size };
    }

    ~allocator_bord(){
        if (*get_memory()== nullptr){
            ::operator delete(global_memory);
        }else{
            (*get_memory())->deallocate(global_memory);
        }
    }


};
#endif //ALLOCATORS_BORDER_DESCRIPTOR_H#endif //COURSEWORK_BORDER_DESCRIPTOR_H
