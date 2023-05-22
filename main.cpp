#include <iostream>
#include "allocator_4.2.h"
#include <random>
#include <list>

void testing_allocator(
        memory *allocator,
        logger* mlog,
        unsigned int iterations_count)
{
    std::list<void*> allocated_blocks;
    //i want to eat

//    std::random_device rd;
//    auto mtgen = std::mt19937(12345);
//    auto ud = std::uniform_int_distribution<>{0, 2};

    srand(time(nullptr));


    for (size_t i = 0; i < iterations_count; ++i)
    {
        std::string kkk = "--- iter #";
        kkk += std::to_string(i+1);
        kkk += " ---";
        mlog->log(kkk, logger::severity::debug);
        void * ptr;
        size_t yes;
        switch (rand() % 2)
        {
            case 0:
                try
                {
                    yes = rand() % 81 + 20;
                    ptr = reinterpret_cast<void *>(allocator->allocate(yes)); // разность макс и мин с включенными границами + минимальное
                    allocated_blocks.push_back(ptr);
                    std::string msg = "allocation succeeded, size: ";
                    msg += std::to_string(yes);
                    mlog->log(msg, logger::severity::debug);
                }
                catch (std::exception const &ex)
                {
                    std::cout << ex.what() << std::endl;
                }
                break;
            case 1:

                if (allocated_blocks.empty())
                {
                    mlog->log("No blocks to dealloc", logger::severity::debug);
                    break;
                }

                try
                {
                    auto iter = allocated_blocks.begin();
                    std::advance(iter, rand() % allocated_blocks.size());
                    allocator->deallocate(*iter);
                    allocated_blocks.erase(iter);
                    mlog->log("deallocation succeeded", logger::severity::debug);
                }
                catch (std::exception const &ex)
                {
                    std::cout << ex.what() << std::endl;
                }
                break;
        }


    }

    mlog->log("--- END OF ITERATIONS ---", logger::severity::debug);

    while (!allocated_blocks.empty())
    {
        try
        {
            auto iter = allocated_blocks.begin();
            allocator->deallocate(*iter);
            allocated_blocks.erase(iter);
            mlog->log("deallocation succeeded", logger::severity::debug);
        }
        catch (std::exception const &ex)
        {
            std::cout << ex.what() << std::endl;
        }
    }
}

int main(){
    logger_builder* builder = new logger_builder_concrete();
    logger* mylogger = builder->add_stream("console", logger::severity::debug)->add_stream("log.txt", logger::severity::debug)->construct();
    memory* alloc= new allocator_bord(200, mylogger, nullptr, allocator_bord::detour_type::first );//I want to sleep
    testing_allocator(alloc, mylogger,  150);

    mylogger->log("~~~~~~~~~~~~~~~    ~~~~~~~~~~~~~~~~~~   ~~~~~~~~~~~~~~~~~~   ~~~~~~~~~~~~~", logger::severity::debug);

    delete alloc;
    delete mylogger;
    delete builder;


//    int* arr = reinterpret_cast<int *>(alloc->allocate(sizeof(int) * 10));
//    for (int i = 0; i < 10; i++){
//        arr[i] = i*100;
//        std::cout << arr[i] << std::endl;
//    }
//    int* arr1 = reinterpret_cast<int *>(alloc->allocate(sizeof(int) * 10));
//    for (int i = 0; i < 10; i++){
//        arr1[i] = i*100;
//        std::cout << arr1[i] << std::endl;
//    }
//    char *str = reinterpret_cast<char *>(alloc->allocate(sizeof(char) * 11));
//    std::strcpy(str, "0123456789");
//    alloc->deallocate(arr1);
//    int* arr2 = reinterpret_cast<int *>(alloc->allocate(sizeof(int) * 10));
//    for (int i = 0; i < 10; i++){
//        arr2[i] = i*100;
//        std::cout << arr2[i] << std::endl;
//    }
//    alloc->deallocate(arr);
//    alloc->deallocate(arr2);
//    alloc->deallocate(str);
//    delete alloc;
//    delete builder;
//    delete mylogger;
 }
