
#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <limits.h>
#include <stddef.h>

template <typename T, size_t BlockSize = 4096>
class MemoryPool
{
public:
    typedef T  value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    //typedef true_type propagate_on_container_move_assignment;
    //typedef true_type is_always_equal;

    template <typename U> struct rebind {
        typedef MemoryPool<U> other;
    };

    //construction functions
    MemoryPool() throw() {
        currentBlock_ = 0;
        currentSlot_ = 0;
        lastSlot_ = 0;
        freeSlots_ = 0;
    }
    MemoryPool(const MemoryPool& memoryPool) throw() {
        MemoryPool();
    }
    template<class U> MemoryPool(const MemoryPool<U>& memoryPool) throw() {
        MemoryPool();
    }

    //deconstruction function
    ~MemoryPool() throw() {
        slot_pointer_ curr = currentBlock_;
        while (curr) {
            slot_pointer_ prev = curr->next;
            //delete curr
            operator delete(reinterpret_cast<void*>(curr));
            curr = prev;
        }
    }

    //get the address
    pointer address(reference x) const throw() {
        return &x;
    }
    const_pointer address(const_reference x) const throw() {
        return &x;
    }

    //allocate function
    pointer allocate(size_type n = 1, const_pointer hint = 0) {
        if (freeSlots_ != 0) {
            pointer result = reinterpret_cast<pointer>(freeSlots_);
            freeSlots_ = freeSlots_->next;
            return result;
        }
        else {
            if (currentSlot_ >= lastSlot_)allocateBlock();
            return reinterpret_cast<pointer>(currentSlot_++);
        }
    }

    //deallocate function
    void deallocate(pointer p, size_type n = 1) {
        if (p != 0) {
            reinterpret_cast<slot_pointer_>(p)->next = freeSlots_;
            freeSlots_ = reinterpret_cast<slot_pointer_>(p);
        }
    }

    //calculate the maximum size
    size_type max_size() const throw() {
        size_type maxBlocks = -1 / BlockSize;
        return (BlockSize - sizeof(data_pointer_)) / sizeof(slot_type_) * maxBlocks;
    }

    //construct function
    void construct(pointer p, const_reference val) {
        new (p) value_type(val);
    }

    //destroy function
    void destroy(pointer p) {
        p->~value_type();
    }

    //bool operator==( const allocator<T1>& lhs, const allocator<T2>& rhs ) throw();
    //bool operator!=( const allocator<T1>& lhs, const allocator<T2>& rhs ) throw();

private:
    union Slot_ {
        value_type element;
        Slot_* next;
    };

    typedef char* data_pointer_;
    typedef Slot_ slot_type_;
    typedef Slot_* slot_pointer_;
    slot_pointer_ currentBlock_;
    slot_pointer_ currentSlot_;
    slot_pointer_ lastSlot_;
    slot_pointer_ freeSlots_;

    //apply for a new Block
    void allocateBlock() {
        // Allocate space for the new block and store a pointer to the previous one
        data_pointer_ newBlock = reinterpret_cast<data_pointer_>(operator new(BlockSize));
        reinterpret_cast<slot_pointer_>(newBlock)->next = currentBlock_;
        currentBlock_ = reinterpret_cast<slot_pointer_>(newBlock);
        data_pointer_ body = newBlock + sizeof(slot_pointer_);
        size_type bodyPadding = padPointer(body, sizeof(slot_type_));
        currentSlot_ = reinterpret_cast<slot_pointer_>(body + bodyPadding);
        lastSlot_ = reinterpret_cast<slot_pointer_>(newBlock + BlockSize - sizeof(slot_type_) + 1);
    }

    size_type padPointer(data_pointer_ p, size_type align) const throw() {
        size_t result = reinterpret_cast<size_t>(p);
        return ((align - result) % align);
    }

};


#endif // MEMORY_POOL_H