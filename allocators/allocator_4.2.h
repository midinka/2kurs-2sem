#ifndef UNTITLED4_DIDI_H
#define UNTITLED4_DIDI_H
#include <iostream>
#include "allocator_2.h"
#include <list>
#include <random>

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

    void inner_deallocate(void* result) const{
        if (result == nullptr){
            //TODO:
        }

        auto block = reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(result) - sizeof(void**)*2 - sizeof(size_t));
        size_t size = *get_block_size(block);

        if ((*get_block_next(block) == nullptr) && (*get_block_prev(block) == nullptr)){
            *get_first_taken() = nullptr;
        }
        else if((*get_block_next(block) == nullptr) && (*get_block_prev(block) != nullptr)){
            *get_block_prev(*get_block_next(block)) = nullptr;
            *get_first_taken() = *get_block_next(block);
        }
        else if((*get_block_prev(block) != nullptr) && (*get_block_next(block) == nullptr)){
            *get_block_next(*get_block_prev(block)) = nullptr;
        } 
        else if((*get_block_prev(block) != nullptr) && (*get_block_next(block) != nullptr)){ 
            *get_block_prev(*get_block_next(block)) = *get_block_prev(block);
            *get_block_next(*get_block_prev(block)) = *get_block_next(block);
        }
    }


public:

    explicit allocator_bord(size_t size, logger* log = nullptr, memory* alloc = nullptr, detour_type det = detour_type::first){

        if (alloc == nullptr){
            global_memory = ::operator new (sizeof(size_t) + size + sizeof(logger*) + sizeof(memory*) + sizeof(detour_type) + sizeof(void*));

        }
        else{
            global_memory = alloc->allocate(sizeof(size_t) + size + sizeof(logger*) + sizeof(memory*) + sizeof(detour_type) + sizeof(void*));
        }


        *reinterpret_cast<memory**>(global_memory) = alloc;
        *reinterpret_cast<logger**>(reinterpret_cast<memory**>(global_memory)+1) = log;
        *reinterpret_cast<size_t*>(reinterpret_cast<logger**>(reinterpret_cast<memory**>(global_memory)+1)+1) = size;
        *reinterpret_cast<allocator_bord::detour_type*>(reinterpret_cast<size_t*>(reinterpret_cast<logger**>(reinterpret_cast<memory**>(global_memory)+1)+1)+1) = det;
        *reinterpret_cast<void**>(reinterpret_cast<detour_type*>(reinterpret_cast<size_t*>(reinterpret_cast<logger**>(reinterpret_cast<memory**>(global_memory)+1)+1)+1)+1) = nullptr;

        std::cout<<"--------------------------------"<<std::endl;
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

public:

    void* allocate(size_t tarsize)const override {
        if (*get_first_taken() == nullptr){
            if (tarsize + sizeof(size_t) + 2*sizeof(void*) <= *get_size()){
                auto block = reinterpret_cast<void*>(get_first_taken() + 1);
                *get_block_size(block) = tarsize;
                *get_block_next(block) = nullptr;
                *get_block_prev(block) = nullptr;
                *get_first_taken() = block;
                return reinterpret_cast<void*>(get_block_prev(block) + 1);
            }else{
                return nullptr;
                //если требуемый размер больше
            }
        }
        switch (*get_detour_type()){

            case detour_type::first:{

                auto ptr_to_next = get_block_next(*get_first_taken());

                if ((reinterpret_cast<unsigned char*>(*get_first_taken()) - reinterpret_cast<unsigned char*>(get_first_taken() + 1)) >= tarsize + sizeof(size_t) + 2*sizeof(void*)){
                    auto block = reinterpret_cast<void*>(get_first_taken() + 1);
                    *get_block_size(block) = tarsize;
                    *get_block_next(block) = *get_first_taken();
                    *get_block_prev(*get_first_taken()) = block;
                    *get_block_prev(block) = nullptr;
                    *get_first_taken() = block;
                    return reinterpret_cast<void*>(get_block_prev(block) + 1);
                }

                while (*ptr_to_next != nullptr){
                    if (reinterpret_cast<unsigned char*>(*ptr_to_next) - (reinterpret_cast<unsigned char*>(ptr_to_next+2)+*get_block_size(reinterpret_cast<unsigned char*>(ptr_to_next) -sizeof(size_t)))>= tarsize + sizeof(void*)*2 + sizeof(size_t)){
                        auto block = reinterpret_cast<size_t*>(reinterpret_cast<unsigned char*>(ptr_to_next + 2) + *get_block_size(reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(ptr_to_next)-sizeof(size_t))));
                        *get_block_size(block) = tarsize;
                        *get_block_next(block) = *ptr_to_next;
                        *get_block_prev(block) = reinterpret_cast<size_t*>(reinterpret_cast<unsigned char*>(ptr_to_next)-sizeof(size_t));
                        *get_block_prev(*ptr_to_next) = block;
                        *ptr_to_next = block;

                        return reinterpret_cast<void*>(reinterpret_cast<void**>(block + 1) + 2);

                    }else {
                        ptr_to_next = reinterpret_cast<void**>(reinterpret_cast<size_t*>(*ptr_to_next) + 1);
                    }

                }

                if (reinterpret_cast<unsigned char*>(global_memory) + sizeof(size_t) + sizeof(logger*) + sizeof(memory*) + sizeof(void*) + sizeof(detour_type) + *get_size() - (reinterpret_cast<unsigned char*>(ptr_to_next +2) + *reinterpret_cast<size_t*>(reinterpret_cast<unsigned char*>(ptr_to_next) - sizeof(size_t))) >= tarsize + sizeof(size_t) + 2*sizeof(void*)){
                    auto block = reinterpret_cast<size_t*>(reinterpret_cast<unsigned char*>(ptr_to_next + 2)+ *get_block_size(reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(ptr_to_next)-sizeof(size_t))));
                    *get_block_size(block) = tarsize;
                    *get_block_next(block) = nullptr;
                    *get_block_prev(block) = reinterpret_cast<size_t*>(reinterpret_cast<unsigned char*>(ptr_to_next)-sizeof(size_t));

                    *ptr_to_next = block;

                    return reinterpret_cast<void*>(reinterpret_cast<void**>(block + 1) + 2);


                }
                return nullptr;
            }

            case detour_type::best:{
                int best_size = INT_MAX;
                void* result = nullptr;
                if(((tarsize + sizeof(size_t) + 2*sizeof(void*)) <= (reinterpret_cast<unsigned char*>(*get_first_taken()) - reinterpret_cast<unsigned char*>(get_first_taken() + 1))) && (reinterpret_cast<unsigned char*>(*get_first_taken()) - reinterpret_cast<unsigned char*>(get_first_taken() + 1))  < best_size){
                    auto block = reinterpret_cast<void*>(get_first_taken() + 1);
                    *get_block_size(block) = tarsize;
                    *get_block_prev(block) = nullptr;
                    *get_block_next(block) = *get_first_taken();
                    *get_block_prev(*get_first_taken()) = block;
                    *get_first_taken() = block;
                    result = reinterpret_cast<void*>(get_block_prev(block) + 1);
                    best_size = reinterpret_cast<unsigned char*>(*get_first_taken()) - reinterpret_cast<unsigned char*>((get_first_taken()+1));
                }

                auto curblock = *get_first_taken();
                while (*get_block_next(curblock) != nullptr){
                    if (reinterpret_cast<unsigned char*>(*get_block_next(curblock)) - (reinterpret_cast<unsigned char*>(get_block_prev(curblock) + 1) + *get_block_size(curblock)) >= tarsize + sizeof(size_t) + 2*sizeof(void*)){
                        if (reinterpret_cast<unsigned char*>(*get_block_next(curblock)) - (reinterpret_cast<unsigned char*>(get_block_prev(curblock) + 1) + *get_block_size(curblock)) < best_size){
                            if (result != nullptr){
                                inner_deallocate(result);
                            }
                            auto block = reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(get_block_prev(curblock) + 1) + *get_block_size(curblock));

                            *get_block_size(block) = tarsize;
                            *get_block_next(block) = *get_block_next(curblock);
                            *get_block_prev(block) = curblock;
                            *get_block_next(curblock) = block;
                            *get_block_prev(*get_block_next(block)) = block;
                            best_size = reinterpret_cast<unsigned char*>(*get_block_next(curblock)) - (reinterpret_cast<unsigned char*>(get_block_prev(curblock) + 1) + *get_block_size(curblock));
                            result = reinterpret_cast<void*>(get_block_prev(block) + 1);
                            curblock = *get_block_next(curblock);

                        }else{
                            curblock = *get_block_next(curblock);
                        }

                    }else{
                        curblock = *get_block_next(curblock);

                    }
                } //end of while


                if (*get_block_next(curblock) == nullptr && ( reinterpret_cast<unsigned char*>(get_first_taken() + 1) + *get_size() - reinterpret_cast<unsigned char*>(get_block_prev(curblock)+1) - *get_block_size(curblock)) >= tarsize + sizeof(void*)*2 + sizeof(size_t)){
                    if(( reinterpret_cast<unsigned char*>(reinterpret_cast<void**>(get_first_taken()) + 1) + *get_size() - reinterpret_cast<unsigned char*>(get_block_prev(curblock)+1) - *get_block_size(curblock)) < best_size){
                        if (result != nullptr){
                            inner_deallocate(result);
                        }
                        auto block = reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(get_block_prev(curblock) + 1) + *get_block_size(curblock));
                        *get_block_size(block) = tarsize;
                        *get_block_next(block) = nullptr;
                        *get_block_prev(block) = curblock;
                        *get_block_next(curblock) = block;
                        best_size = *get_block_size(block);
                        result = reinterpret_cast<void*>(get_block_prev(block) + 1);
                    }
                }

                return result;

            }

            case detour_type::worst:{
                int worst_size = 0;
                void* result = nullptr;
                auto curblock = *get_first_taken();
                if((tarsize + sizeof(size_t) + 2*sizeof(void*)) <= (reinterpret_cast<unsigned char*>(curblock) - reinterpret_cast<unsigned char*>(get_first_taken() + 1)) && (reinterpret_cast<unsigned char*>(*get_first_taken()) - reinterpret_cast<unsigned char*>(get_first_taken() + 1))  > worst_size){
                    auto block = reinterpret_cast<void*>(get_first_taken() + 1);
                    *get_block_size(block) = tarsize;
                    *get_block_prev(block) = nullptr;
                    *get_block_next(block) = *get_first_taken();
                    *get_block_prev(*get_first_taken()) = block;
                    *get_first_taken() = block;
                    result = reinterpret_cast<void*>(get_block_prev(block) + 1);
                    worst_size = reinterpret_cast<unsigned char*>(*get_first_taken()) - reinterpret_cast<unsigned char*>((get_first_taken()+1));
                }

                while (*get_block_next(curblock) != nullptr){
                    if (reinterpret_cast<unsigned char*>(*get_block_next(curblock)) - (reinterpret_cast<unsigned char*>(get_block_prev(curblock) + 1) + *get_block_size(curblock)) >= tarsize + sizeof(size_t) + 2*sizeof(void*)){
                        if (reinterpret_cast<unsigned char*>(*get_block_next(curblock)) - (reinterpret_cast<unsigned char*>(get_block_prev(curblock) + 1) + *get_block_size(curblock)) > worst_size){
                            if (result != nullptr){
                                inner_deallocate(result);
                            }
                            auto block = reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(get_block_prev(curblock) + 1) + *get_block_size(curblock));

                            *get_block_size(block) = tarsize;
                            *get_block_next(block) = *get_block_next(curblock);
                            *get_block_prev(block) = curblock;
                            *get_block_next(curblock) = block;
                            *get_block_prev(*get_block_next(block)) = block;
                            worst_size = reinterpret_cast<unsigned char*>(*get_block_next(curblock)) - (reinterpret_cast<unsigned char*>(get_block_prev(curblock) + 1) + *get_block_size(curblock));
                            result = reinterpret_cast<void*>(get_block_prev(block) + 1);
                            curblock = *get_block_next(curblock);

                        }else{
                            curblock = *get_block_next(curblock);
                        }

                    }else{
                        curblock = *get_block_next(curblock);

                    }
                } //end of while

                if (*get_block_next(curblock) == nullptr && ( reinterpret_cast<unsigned char*>(reinterpret_cast<void**>(get_first_taken()) + 1) + *get_size() - reinterpret_cast<unsigned char*>(get_block_prev(curblock)+1) - *get_block_size(curblock)) >= tarsize + sizeof(void*)*2 + sizeof(size_t)){
                    if(( reinterpret_cast<unsigned char*>(reinterpret_cast<void**>(get_first_taken()) + 1) + *get_size() - reinterpret_cast<unsigned char*>(get_block_prev(curblock)+1) - *get_block_size(curblock)) > worst_size){
                        if (result != nullptr){
                            inner_deallocate(result);
                        }
                        auto block = reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(get_block_prev(curblock) + 1) + *get_block_size(curblock));
                        *get_block_size(block) = tarsize;
                        *get_block_next(block) = nullptr;
                        *get_block_prev(block) = curblock;
                        *get_block_next(curblock) = block;
                        worst_size = *get_block_size(block);
                        result = reinterpret_cast<void*>(get_block_prev(block) + 1);
                    }
                }

                return result;
            }

        }
        return nullptr;


    }

    void deallocate(void* result)const override{

        if (result != nullptr) {


            auto block = reinterpret_cast<void *>(reinterpret_cast<unsigned char *>(result) - sizeof(void **) * 2 -sizeof(size_t));
            size_t size = *get_block_size(block);

            if ((*get_block_next(block) == nullptr) && (*get_block_prev(block) == nullptr)) {
                *get_first_taken() = nullptr;
            } else if ((*get_block_next(block) != nullptr) && (*get_block_prev(block) == nullptr)) {
                *get_block_prev(*get_block_next(block)) = nullptr;
                *get_first_taken() = *get_block_next(block);
            } else if ((*get_block_prev(block) != nullptr) && (*get_block_next(block) == nullptr)) {
                *get_block_next(*get_block_prev(block)) = nullptr;
            } else if ((*get_block_prev(block) != nullptr) && (*get_block_next(block) != nullptr)) {
                std::cout << *get_block_size(block) << "--------------" << *get_block_prev(block) << std::endl;
                *get_block_prev(*get_block_next(block)) = *get_block_prev(block);
                *get_block_next(*get_block_prev(block)) = *get_block_next(block);
            }

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

    }


    ~allocator_bord(){
        if (*get_memory()== nullptr){
            ::operator delete(global_memory);
        }else{
            (*get_memory())->deallocate(global_memory);
        }
    }


};
#endif //UNTITLED4_DIDI_H
