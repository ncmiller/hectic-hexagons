// Copyright (c) 2021 Nick Miller
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include <stdbool.h>
#include <stdlib.h>

/// Allocator that simply keeps track of an offset within a user-provided block of memory.
/// Allocation is O(1), and consists of simply incrementing an offset.
///
/// There is no way to free an individual allocation (calling bump_allocator_free does nothing).
/// However, you can free all allocations at once.
///
/// This can be useful as temporary dynamic storage during a single iteration of the game loop
/// (i.e. call bump_allocator_free_all() at the end of each game loop iteration).
///
/// Reference: https://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/

void bump_allocator_init(void* backing_buffer, size_t backing_buffer_size);
void* bump_allocator_alloc(size_t size);
void bump_allocator_free(void*);  // does nothing
void bump_allocator_free_all(void);
void bump_allocator_deinit(void);
size_t bump_allocator_num_allocations(void);
