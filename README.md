# oop lab7
## * Assignment 007 STL allocator + memory pool
oop大作业
### 题目
STL Allocator Interface
An allocator is used by standard library containers as a template parameter :
  template < class T, class Alloc = allocator<T> > class vector;
  template < class T, class Alloc = allocator<T> > class list;

What does an allocator class have? Typically, it possesses:
  typedef void _Not_user_specialized;
  typedef _Ty value_type;
  typedef value_type *pointer;
  typedef const value_type *const_pointer;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef true_type propagate_on_container_move_assignment;
  typedef true_type is_always_equal;

  pointer address(reference _Val) const _NOEXCEPT
  const_pointer address(const_reference _Val) const _NOEXCEPT
  void deallocate(pointer _Ptr, size_type _Count)
  _DECLSPEC_ALLOCATOR pointer allocate(size_type _Count)
  template<class _Uty> void destroy(_Uty *_Ptr)
  template<class _Objty, class _Types>
  void construct(_Objty *_Ptr, _Types&&... _Args)
The above interface is just shown for illustration, please refer to std::allocator for the latest specification.

Memory Pool

STL provides you a default std::allocator, but you can implement your own to replace it. For example, you can design a memory pool to speed up the dynamic allocation 
of a large number of small blocks (e.g., 8 bytes, 16 bytes, ...), and to reduce memory fragmentation.
![image](https://user-images.githubusercontent.com/95481309/208222404-73ebd67a-6bc9-4512-8d87-7076e0396f00.png)

### Requirements

* Implement your own memory allocator for STL vector.
* The allocator should optimize the memory allocation speed using memory pool.
* The allocator should support arbitrary memory size allocation request.

### test
How to Test Your Allocator
Basically, you should:
< Create more than ten thousand vectors with different number of elements.
< Pick up 1000 random vectors and resize the vectors with random sizes.
< Release all the vectors.
< Feel free to extend the following code skeleton for your own tests (good test is a bonus):
  #include <iostream>
  #include <random>
  #include <vector>

  // include header of your allocator here
  template<class T>
  using MyAllocator = std::allocator<T>; // replace the std::allocator with your allocator
  using Point2D = std::pair<int, int>;

  const int TestSize = 10000;
  const int PickSize = 1000;

  int main()
  {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, TestSize);

    // vector creation
    using IntVec = std::vector<int, MyAllocator<int>>;
    std::vector<IntVec, MyAllocator<IntVec>> vecints(TestSize);
    for (int i = 0; i < TestSize; i++)
      vecints[i].resize(dis(gen));

    using PointVec = std::vector<Point2D, MyAllocator<Point2D>>;
    std::vector<PointVec, MyAllocator<PointVec>> vecpts(TestSize);
    for (int i = 0; i < TestSize; i++)
      vecpts[i].resize(dis(gen));

    // vector resize
    for (int i = 0; i < PickSize; i++)
    {
      int idx = dis(gen) - 1;
      int size = dis(gen);
      vecints[idx].resize(size);
      vecpts[idx].resize(size);
    }

    // vector element assignment
    {
      int val = 10;
      int idx1 = dis(gen) - 1;
      int idx2 = vecints[idx1].size() / 2;
      vecints[idx1][idx2] = val;
      if (vecints[idx1][idx2] == val)
        std::cout << "correct assignment in vecints: " << idx1 << std::endl;
      else
        std::cout << "incorrect assignment in vecints: " << idx1 << std::endl;
    }
    {
      Point2D val(11, 15);
      int idx1 = dis(gen) - 1;
      int idx2 = vecpts[idx1].size() / 2;
      vecpts[idx1][idx2] = val;
      if (vecpts[idx1][idx2] == val)
        std::cout << "correct assignment in vecpts: " << idx1 << std::endl;
      else
        std::cout << "incorrect assignment in vecpts: " << idx1 << std::endl;
    }

    return 0;
  }

### submit
#### Evaluation Standard
1. c++ code quality (clean, compact and reasonable)
2. comments quality
3. correctness and running performance of the allocator

#### Files to Submit
1. Please prepare a .zip package including the following items：
2. the source code (including the testing code)
3. makefile (for Mac or Linux users) or .exes (for Windows users, with necessary .dlls if you use MinGW) or CMakeLists.txt
