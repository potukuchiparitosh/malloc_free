**MALLOC and FREE** : 
This program implements the malloc and free functions available in linux. It makes use of the sbrk system call to make changes to the Program Break. The way this program works is briefed below : 
**malloc_v1()** : This is the function that works just like malloc()
**free_v1()** : This is the function that works just like free()

We maintain a linked list for all the free blocks. When we run malloc, we first check whether there is anything in the free list. If there is some block in the free list which has a size greater than or equal to the size being requested by malloc, we should use this free block to allocate memory while giving back the remaining to the free block. Most of this is achieved through some trivial linked list pointer manipulation.
As for the free function, it just adds the block of memory to the free list.


# Snippets from the code

Below are structures for a free list item, named **free_list_item**, and an alloated block of memory called **allocated_item**

```
struct free_list_item {
        size_t len;
        void *block;
        struct free_list_item *next;
        struct free_list_item *prev;
};
struct allocated_item {
        size_t len;
        void *block;
};
```

**Note**: When memory is allocated using **malloc()** the first few bytes is reserved to store the size of the the memory being allocated. This is really important to follow and take care of when adding blocks to the free list and taking blocks from the free list to be allocated through malloc. This section of the code is very prone, and if you try to program it be careful with the pointer addition and subtraction.

# Bibliography

You can refer to the book **The Linux Programming Interface** by  Michael Kerrisk for more information about how to implement a memory allocator. You can find the book at https://man7.org/tlpi/
 Please feel free to commit changes to the code if you feel like more edge cases can be handled :)
