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

#include <stdlib.h>
#include <stdbool.h>

// Opaque handle to vector
typedef struct _Vector* Vector;

typedef void* (*AllocFn)(size_t);
typedef void (*FreeFn)(void*);

// Creates a new vector.
//
// Returns:
//   On success, a handle to the created vector
//   On failure, returns NULL
Vector vector_create(size_t item_size);

// Creates a vector with custom alloc and free functions.
Vector vector_create_with_allocator(size_t item_size, AllocFn alloc_fn, FreeFn free_fn);

// Clone a vector.
//
// Allocates a completely new Vector instance and copies all data from the
// src Vector to the new instance. The source vector is not modified.
//
// Returns:
//   On success, a handle to the created vector
//   On failure, returns NULL
Vector vector_clone(Vector src);

// Reserve enough space in the vector to store at least num_items.
// May (re)allocate if the current capacity is not enough.
//
// This function has no effect on the vector size.
//
// Returns:
//   0:  success
//  -1:  allocation failure
int vector_reserve(Vector, size_t num_items);

// Pushes a new item to the back of the vector.
// Item is copied to the vector.
// May allocate if there is not space for the item.
//
// Returns:
//   0: success
//  -1: allocation failure
int vector_push_back(Vector, const void* item);

// Pops an item from the back of the vector.
// If popped_item is non-NULL, the item is copied into popped_item.
//
// Returns:
//   0: success
//  -1: vector has no items, unable to pop
int vector_pop_back(Vector, void* popped_item);

// Returns a raw pointer to the item inside of the vector.
//
// WARNING: The returned pointer can become invalid if the vector is resized, or
// an item is erased or inserted, so calling code should be careful to only use this
// pointer temporarily, in scenarios where it is known that the vector
// will not resize/insert/erase (e.g. in a loop that doesn't modify the vector).
//
// Returns:
//  On success, Pointer to the item at index
//  NULL, index invalid, greater than or equal to the vector size
void* vector_data_at(Vector, size_t index);

// Erases the item at index, reducing the vector size by 1.
//
// Operation is O(N) because it requires all elements after index to be relocated
// by copying to the previous index.
//
// Returns:
//   0: success
//  -1: index invalid, greater than or equal to the vector size
int vector_erase(Vector, size_t index);

// Inserts an item by copying it into the specified index, increasing the vector size by 1.
// May allocate if there is not space for the item.
//
// Operation is O(N) because it requires all elements from index and after to
// be relocated by copying to the next index.
//
// Returns:
//   0: success
//  -1: index invalid, greater than or equal to the vector size
//  -2: allocation failure
int vector_insert(Vector, const void* item, size_t index);

// Returns the number of items currently in the vector.
size_t vector_size(Vector);

// Clears the vector, size reset to 0
void vector_clear(Vector);

// Erase all items in the vector where fn(item) return true.
typedef bool (*VectorEraseFn)(const void* item);
void vector_erase_if(Vector, VectorEraseFn fn);

typedef void (*VectorPrintFn)(const void* item, char* buffer, size_t max_len);
void vector_print(Vector, VectorPrintFn fn);

// Destroys the vector. The Vector handle is no longer usable after calling this.
void vector_destroy(Vector);
