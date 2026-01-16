# Custom Memory Allocators in C

Simple memory allocators for research and learning purposes
---

## Allocators

### 1. Chunk Allocator
GLIBC Inspired, focusing on thread local heaps, and on minimizing external fragmentation.
* Uses `brk` for the main thread's data segment and `mmap` for secondary thread heaps.
* Provides each thread with its own heap managed via `mmap`.
* Implements chunk splitting and coalescing (merging) to maintain a compact heap.

### 2. Arena Allocator
Basic faster allocations, works similar to a regular stack
* Uses `mmap` for allocations
* Supports both random and user-defined alignment (similar to aligned_alloc())
---

## How to run

### Requirements
- C compiler (GCC or Clang)
- CMake 3.10 or newer
- Linux or Unix-like operating system (uses `brk` and `mmap`)

### Build steps
In the chosen allocator directory run:

```sh
mkdir build
cd build
cmake ..
cmake --build .
```

Than run the binary in the build file.
