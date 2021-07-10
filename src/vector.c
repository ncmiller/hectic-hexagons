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

#include "vector.h"
#include "bump_allocator.h"

#include <string.h>
#include <stdint.h>
#include <macros.h>
#include <SDL.h>

// First allocation will be at least this many bytes
#define FIRST_ALLOC_MIN_BYTES 32
#define DEFAULT_ALLOC_FN malloc
#define DEFAULT_FREE_FN free

struct _Vector {
    // Max number of items the vector can currently hold (will be resized as needed).
    size_t capacity;
    // Current number of items in the vector
    size_t size;
    // Size of each item, in bytes
    size_t item_size;
    // Contiguous memory holding vector items
    void* data;
    // Allocator
    AllocFn alloc_fn;
    FreeFn free_fn;
};


static int resize(Vector v, size_t new_capacity) {
    void* new_data = v->alloc_fn(v->item_size * new_capacity);
    if (new_data == NULL) {
        return -1;
    }

    if (v->size > 0) {
        memcpy(new_data, v->data, v->size * v->item_size);
        v->free_fn(v->data);
    }
    v->data = new_data;
    v->capacity = new_capacity;
    return 0;
}

Vector vector_create(size_t item_size) {
    return vector_create_with_allocator(item_size, DEFAULT_ALLOC_FN, DEFAULT_FREE_FN);
}

Vector vector_create_with_allocator(size_t item_size, AllocFn alloc_fn, FreeFn free_fn) {
    Vector instance = alloc_fn(sizeof(struct _Vector));
    if (instance) {
        instance->alloc_fn = alloc_fn;
        instance->free_fn = free_fn;
        instance->capacity = 0;
        instance->size = 0;
        instance->item_size = item_size;
        // Defer data allocation until the user calls reserve or adds something to the vector.
        instance->data = NULL;
    }
    return instance;
}

Vector vector_clone(Vector src) {
    Vector instance = src->alloc_fn(sizeof(struct _Vector));
    if (instance) {
        instance->alloc_fn = src->alloc_fn;
        instance->free_fn = src->free_fn;
        instance->capacity = src->capacity;
        instance->size = src->size;
        instance->item_size = src->item_size;
        instance->data = src->alloc_fn(src->capacity * src->item_size);
        memcpy(instance->data, src->data, src->size * src->item_size);
    }
    return instance;
}

int vector_reserve(Vector v, size_t num_items) {
    if (v->capacity >= num_items) {
        return 0;
    }
    return resize(v, num_items);
}

int vector_push_back(Vector v, const void* item) {
    if (v->capacity <= v->size) {
        size_t new_capacity = v->capacity * 2;
        if (v->capacity == 0) {
            if (v->item_size > FIRST_ALLOC_MIN_BYTES) {
                new_capacity = 1;
            } else {
                new_capacity = FIRST_ALLOC_MIN_BYTES / v->item_size;
            }
        }
        int resize_status = resize(v, new_capacity);
        if (resize_status != 0) {
            return resize_status;
        }
    }
    uint8_t* data_ptr = (uint8_t*)(v->data) + v->size * v->item_size;
    memcpy(data_ptr, item, v->item_size);
    v->size++;
    return 0;
}

int vector_pop_back(Vector v, void* popped_item) {
    if (v->size == 0) {
        return -1;
    }

    if (popped_item) {
        const uint8_t* last_item = (uint8_t*)(v->data) + (v->size - 1) * v->item_size;
        memcpy(popped_item, last_item, v->item_size);
    }
    v->size--;
    return 0;
}

void* vector_data_at(Vector v, size_t index) {
    if (index >= v->size) {
        return NULL;
    }
    uint8_t* data_ptr = (uint8_t*)(v->data) + index * v->item_size;
    return data_ptr;
}

int vector_erase(Vector v, size_t index) {
    if (index >= v->size) {
        return -1;
    }
    // Copy each item to previous index in array
    for (size_t i = index; i < v->size - 1; i++) {
        uint8_t* dest = (uint8_t*)(v->data) + i * v->item_size;
        const uint8_t* src = (const uint8_t*)(v->data) + (i + 1) * v->item_size;
        memcpy(dest, src, v->item_size);
    }

    v->size--;
    return 0;
}

int vector_insert(Vector v, const void* item, size_t index) {
    if (index >= v->size) {
        return -1;
    }

    if (v->capacity <= v->size) {
        size_t new_capacity = v->capacity * 2;
        if (v->capacity == 0) {
            if (v->item_size > FIRST_ALLOC_MIN_BYTES) {
                new_capacity = 1;
            } else {
                new_capacity = FIRST_ALLOC_MIN_BYTES / v->item_size;
            }
        }
        int resize_status = resize(v, new_capacity);
        if (resize_status != 0) {
            return -2;
        }
    }
    v->size++;

    // Copy each item to next index in array, starting from the end and working backwards.
    for (int i = v->size - 2; i >= index; i--) {
        const uint8_t* src = (const uint8_t*)(v->data) + i * v->item_size;
        uint8_t* dest = (uint8_t*)(v->data) + (i + 1) * v->item_size;
        memcpy(dest, src, v->item_size);
    }

    // Insert new item
    uint8_t* dest = (uint8_t*)(v->data) + index * v->item_size;
    memcpy(dest, item, v->item_size);

    return 0;
}

size_t vector_size(Vector v) {
    return v->size;
}

void vector_clear(Vector v) {
    v->size = 0;
}

void vector_print(Vector v, VectorPrintFn fn) {
    char item_print_buffer[80];
    SDL_Log("Vector size %zu", vector_size(v));
    for (size_t i = 0; i < vector_size(v); i++) {
        fn(vector_data_at(v, i), item_print_buffer, sizeof(item_print_buffer));
        item_print_buffer[sizeof(item_print_buffer) - 1] = 0;
        SDL_Log("   [%zu]: %s\n", i, item_print_buffer);
    }
}

void vector_erase_if(Vector v, VectorEraseFn erase_fn) {
    Vector temp = vector_create_with_allocator(
            v->item_size, bump_allocator_alloc, bump_allocator_free);
    vector_reserve(temp, v->capacity);

    // Copy non-erased items to a new vector
    for (size_t i = 0; i < vector_size(v); i++) {
        const void* item = vector_data_at(v, i);
        if (!erase_fn(item)) {
            vector_push_back(temp, item);
        }
    }
    ASSERT(temp->size <= v->size);

    v->size = temp->size;
    if (temp->size > 0) {
        memcpy(v->data, temp->data, temp->item_size * temp->size);
    }

    vector_destroy(temp);
}

void vector_destroy(Vector v) {
    if (v->data) {
        v->free_fn(v->data);
    }
    v->free_fn(v);
}
