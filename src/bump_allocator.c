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

#include "bump_allocator.h"

#include <stdint.h>
#include <string.h>
#include <macros.h>

static struct {
    uint8_t* buffer;
    size_t buffer_size;
    size_t offset;
    size_t num_allocations;
} _bump_allocator;

void bump_allocator_init(void* backing_buffer, size_t backing_buffer_size) {
    _bump_allocator.buffer = (uint8_t*)backing_buffer;
    _bump_allocator.buffer_size = backing_buffer_size;
    _bump_allocator.offset = 0;
}

void* bump_allocator_alloc(size_t size) {
    void* ptr = NULL;

    size_t space_remaining = _bump_allocator.buffer_size - _bump_allocator.offset;
    if (size > space_remaining) {
        ASSERT(false && "Allocation failed, not enough space in backing buffer");
        return NULL;
    }

    ptr = &_bump_allocator.buffer[_bump_allocator.offset];
    _bump_allocator.offset += size;
    _bump_allocator.num_allocations++;

    memset(ptr, 0, size);
    return ptr;
}

void bump_allocator_free(void* ptr) {
    // Do nothing
    return;
}

void bump_allocator_free_all(void) {
    _bump_allocator.offset = 0;
    _bump_allocator.num_allocations = 0;
}

void bump_allocator_deinit(void) {
    memset(&_bump_allocator, 0, sizeof(_bump_allocator));
}

size_t bump_allocator_num_allocations(void) {
    return _bump_allocator.num_allocations;
}
