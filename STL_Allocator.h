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
//TODO:
//这个可以报告的时候具体写怎么算出的这两个数字


// the number of free lists
enum { COUNT_FREE_LISTS = MAX_BYTES / BOUND };
//TODO:分配空间变小，速度可能变快，但是总容量会变小
//template <typename D>
// the second allocator using memory pool
class Allocator{
   public:
   typedef std::size_t size_type;
    union cell{
        union cell *free_list_link;
        char client_data[1];
    };
    static void *allocate(size_type n) {
        cell *volatile *cur_free_list;
        cell *res;
        // If the space to be allocated is greater than the 
        // maximum space set by memorypool, 
        // the general space allocation method is followed
        if (n > (size_type)MAX_BYTES) 
            return Mallocate::allocate(n);
        // find the suitable free list
        cur_free_list = free_list + free_list_idx(n);
        //TODO:volatile修饰词
        res = *cur_free_list;
        // refill the free list if there is no free list available
        if (res == nullptr) {
            void *r = refill(ceil_up(n));
            return r;
        }
        // adjust the free list
        *cur_free_list = res->free_list_link;
        return res;
    }

    static void deallocate(void *p, size_type n) {
        cell *q = (cell *)p;
        cell *ptr;
        cell *volatile *cur_free_list;

        // if n is large enough, call the first deallocator
        if (n > (size_type)MAX_BYTES) {
            Mallocate::deallocate(p, n);
            return;
        }
        if(q != nullptr){
            ptr = q->free_list_link;
            delete(q->free_list_link);
            q = ptr;
        }
        // TODO:上面这种方法比下面这种要慢
        // find the suitable free list
        //cur_free_list = free_list + free_list_idx(n);
         // adjust the free list
       //q->free_list_link = *cur_free_list;
        //*cur_free_list = q;
    }
    private:
   // the union object of free list
    // free_lists
    static cell *volatile free_list[COUNT_FREE_LISTS];

    // chunk allocation state
    static char *start_mem_pool;
    static char *end_mem_pool;
    static size_type heap_size;

    // determine the index of free list
    inline static size_type free_list_idx(size_type bytes) {
        return ((bytes) + BOUND - 1) / BOUND - 1;
    }
//TODO:
//改变分配方法会导致结果发生较大变化

    // align the block
    inline static size_type ceil_up(size_type bytes) {
        return (((bytes) + BOUND - 1) & ~(BOUND - 1));
    }

    // allocate using the memory pool
    static char *chunk_alloc(size_type size, int &cnt_cell) {
        char *res;
        size_type total_bytes = size * cnt_cell;
        // the space left in the memory pool
        size_type bytes_left = end_mem_pool - start_mem_pool;

        if (bytes_left >= total_bytes) {
            // if the space left is enough
            res = start_mem_pool;
            start_mem_pool += total_bytes;
            return res;
        } else if (bytes_left >= size) {
            // the space is not enough for requirement
            // but enough for at least one block
            cnt_cell = bytes_left / size;
            total_bytes = size * cnt_cell;
            res = start_mem_pool;
            start_mem_pool += total_bytes;
            return res;
        } else {
            // the space is even not enough for one block
            size_type bytes_to_get =
                2 * total_bytes + ceil_up(heap_size >> 4);
            // try to make use of the scattered space
            if (bytes_to_get > 0 and start_mem_pool != nullptr) {
                // if there is still some space
                cell *volatile *my_free_list =
                    free_list + free_list_idx(bytes_left);
                // adjust the free list
                ((cell *)start_mem_pool)->free_list_link = *my_free_list;
                *my_free_list = (cell *)start_mem_pool;
            }

            // supply the memory pool
            start_mem_pool = (char *)malloc(bytes_to_get);
            heap_size += bytes_to_get;
            end_mem_pool = start_mem_pool + bytes_to_get;

            // adjust the cnt_cell
            return chunk_alloc(size, cnt_cell);
        }
    }

    // return an object consuming the space of n
    static void *refill(size_type n) {
        int cnt_cell = 20;
        //TODO:改变大小影响时间
        // cnt_cell is passed by reference
        char *chunk = chunk_alloc(n, cnt_cell);
        cell *volatile *my_free_list;
        cell *res;
        cell *current_cell, *next_cell;

        // if get only one block, return it
        if (cnt_cell == 1) return chunk;
        // prepare to adjust free list
        my_free_list = free_list + free_list_idx(n);
        // the block to be returned
        res = (cell *)chunk;
        *my_free_list = next_cell = (cell *)(chunk + n);

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
        return res;
    }

};

// initialize the second allocator
char *Allocator ::start_mem_pool = nullptr;

char *Allocator ::end_mem_pool = nullptr;

std::size_t Allocator::heap_size = 0;
// 最后一段空间为空
Allocator::cell *volatile Allocator::free_list[COUNT_FREE_LISTS] = {nullptr};
// TODO: 越小越慢

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