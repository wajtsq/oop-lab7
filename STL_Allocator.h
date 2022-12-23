#ifndef STL_ALLOCATOR_H
#define STL_ALLOCATOR_H
#include <iostream>

// Initial allocation
class Mallocate{
   public:
   // allocate the space to an arbitrary type of pointer
    static void *allocate(std::size_t n) {
        void *ptr = std::malloc(n);
        if (ptr == nullptr) throw std::bad_alloc();
        return ptr;
    }
    // release the space
    static void deallocate(void *ptr, std::size_t) { 
        std::free(ptr); 
    }
};

// the max size of small block in memory pool
enum { MAX_BYTES = 65536 };
// the align size of each block, i.e. the size could be divided by 64
enum { BOUND = 64 };
// the number of free lists
enum { COUNT_FREE_LISTS = MAX_BYTES / BOUND };
enum { FIRST_LIST_SIZE = 0 };

// the second allocator using memory pool
class Allocator{
   public:
   typedef std::size_t size_type;
   // the union object of free list
    union cell{
        union cell *free_list_link;
    };
// This function is used to allocate the n spaces memory from our memory pool.
    static void *allocate(size_type n) {
        cell *res, *ptr;
    // If the space to be allocated is greater than the maximum space set by memorypool, 
    // the general space allocation method is followed:
        if (n > (size_type)MAX_BYTES) 
            return Mallocate::allocate(n);
    // If the space to be allocated does not exceed the maximum space set by memorypool
        ptr = res = free_list[FIRST_LIST_SIZE];
        if (res == nullptr) {
            void *r = re_alloc(ceil_up(n));
    // refill the free list if there is no free list available
            return r;
        }
   // adjust the free list
        ptr = res->free_list_link;
        return res;
    }

    // This function is used to free up space in the memorypool for the application.
    static void deallocate(void *p, size_type n) {
        cell *q = (cell *)p;
        cell *ptr;
        // if n is large enough, call the first deallocator
        if (n > (size_type)MAX_BYTES) {
            Mallocate::deallocate(p, n);
            return;
        }
        if(q != nullptr){
        // Delete each item in the linked list in turn
            ptr = q->free_list_link;
            delete(q->free_list_link);
            q = ptr;
        }
    }
    private:
    // free_lists
    static cell *volatile free_list[COUNT_FREE_LISTS];
    // chunk allocation state
    static char *start_mem_pool;
    static char *end_mem_pool;
    static size_type heap_size;

    // align the block
    inline static size_type ceil_up(size_type bytes) {
        return (((bytes) + BOUND - 1) & ~(BOUND - 1));
    }

    /* This function is used to request a block space 
        and calculate the number of blocks that the vector needs to request, 
        while changing the values of the start position 
        and end position in the memorypool      */
    static char *mem_alloc(size_type size, int &cnt_cell) {
        char *ptr;
        cell *res;
        size_type total_bytes = size * cnt_cell;
        // the space left in the memory pool
        size_type bytes_left = end_mem_pool - start_mem_pool;
        // the intial value of end_mem_pool and start_mem_pool are nullptr
        if (bytes_left >= total_bytes) {
            // if the space left is enough
            ptr = start_mem_pool;
            start_mem_pool += total_bytes;
            return ptr;
        } else if (bytes_left >= size) {
            // the space is not enough for requirement
            // but enough for at least one block
            cnt_cell = bytes_left / size;
            total_bytes = size * cnt_cell;  // change the space
            ptr = start_mem_pool;
            start_mem_pool += total_bytes;
            return ptr;
        } 
        else {  
            // the space is even not enough for one block
            size_type bytes_to_get = total_bytes + ceil_up(heap_size >> 4);
            // use the scattered spaces
            if (bytes_to_get > 0 && start_mem_pool != nullptr) {
                // if there is still some space
                cell *my_free_list = free_list[FIRST_LIST_SIZE];
                // adjust the free list
                res->free_list_link = my_free_list;
                my_free_list = res;
            }
            // supply the memory pool
            start_mem_pool = (char *)malloc(bytes_to_get);
            heap_size += bytes_to_get;
            end_mem_pool = start_mem_pool + bytes_to_get;
            // adjust the cnt_cell
            return mem_alloc(size, cnt_cell);
        }
    }


    // This function is used to request space for n blocks
    static void *re_alloc(size_type n) {
        int cnt_cell = 20;
        // the initial blocks, cnt_cell is passed by reference
        cell *chunk = (cell*)mem_alloc(n, cnt_cell);
        cell *volatile *my_free_list;
        // the list head
        cell *current_cell, *next_cell;
        // if get only one block, then return it
        if (cnt_cell == 1) 
            return chunk;
        // prepare to adjust free list
        my_free_list = free_list + FIRST_LIST_SIZE;
        // the block to be returned
        *my_free_list = next_cell = (cell *)((char*)chunk + n);
        // link the remaining block
        for (int i = 1;; i++) {
            current_cell = next_cell;
            next_cell = (cell *)((char *)next_cell + n);
            if (cnt_cell - 1 == i) {
                current_cell->free_list_link = nullptr;
                break;
            } else {
                current_cell->free_list_link = next_cell;
            }
        }
        // use the linked list to build a series blocks
        return chunk;
    }
};

// initialize the second allocator
char *Allocator ::start_mem_pool = nullptr;
char *Allocator ::end_mem_pool = nullptr;
std::size_t Allocator::heap_size = 0;
// the last space is empty
Allocator::cell *volatile Allocator::free_list[COUNT_FREE_LISTS] = {nullptr};

// the interface of allocator
template <class T>
class Mallocator {
   public:
    typedef void _Not_user_specialized;
    typedef T value_type;
    typedef value_type *pointer;
    typedef const value_type *const_pointer;
    typedef value_type &reference;
    typedef const value_type &const_reference;
    typedef std::size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef std::true_type propagate_on_container_move_assignment;
    typedef std::true_type is_always_equal;
    Mallocator() = default;
    template <class U>
    Mallocator(const Mallocator<U> &other) noexcept{};
    pointer address(reference _Val) const noexcept { 
        return &_Val; 
    }
    const_pointer address(const_reference _Val) const noexcept {
         return &_Val; 
    }
    pointer allocate(size_type _Count) {
        if (_Count > size_type(-1) / sizeof(value_type)) throw std::bad_alloc();
        if (auto p = static_cast<pointer>(
                Allocator::allocate(_Count * sizeof(value_type))))
            return p;
        throw std::bad_alloc();
    }
    void deallocate(pointer _Ptr, size_type _Count) {
        Allocator::deallocate(_Ptr, _Count);
    }
    template <class _Uty>
    void destroy(_Uty *_Ptr) {
        _Ptr->~_Uty();
    }
    void construct(pointer p, const_reference val) {
        new (p) value_type(val);
    }
};

#endif