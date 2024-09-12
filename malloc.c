//#include "tlpi_hdr.h"
#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#define PAGE_SZ 2048
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
int f_count = 0;
int f_instance_count = 0;
int alloc_count = 0;
//static struct allocated_item alloc_instance[100];
static struct allocated_item alloc_instance;
static struct free_list_item f_instance[100];
static struct free_list_item f_free[100];

struct  free_list_item head = {.len = 0, .block = NULL, .next = NULL, .prev = NULL};
struct free_list_item tail = {.len = 0, .block=NULL, .next = NULL, .prev = NULL};
static struct free_list_item *f;
//static struct free_list_item *f = &f_instance[0];
void printf_free_list()
{
        struct free_list_item *f = &head;
        //for (f; f->next!=NULL;f=f->next) {
        for (f; f!=NULL;f=f->next) {
                printf("Free list item metadata is len = %lld, block = %p, prev = %p, next = %p : Address of f is %10p\n", f->len,f->block,f->prev,f->next, f); 
        }
}
/*
   One point to be noted is, so that free can pickup the length of the the block from the block address - (size of size_t), we need to place this value there and shift the value we are returning to the user by size_t.
   Be sure about also shifting the remaning accordingly, since this could end bad.
 */
void *malloc_v1(size_t size)
{
        size_t tester;
        void *program_break, *tmp;
        size_t *size_var = &size;
        int remaining;
        //struct free_list_item *f = &f_instance;
        struct free_list_item *f = &f_instance[f_instance_count];
        //struct allocated_item *alloc = &alloc_instance[alloc_count++];
        struct allocated_item *alloc = &alloc_instance;
        printf("\n\n -------------------------------------ENTERED MALLOC -------------------------------------\n \n");
        /*1.Check the free list for anything that might be there on it*/
        /*If freelist is empty
          move the program break to create more space- the increment needs to be decided
          else
          Search through the free list, pickup the first item which has len >=size, and give them remaining back to free list*/


        //printf_free_list();
        //Checking if the free list is empty
        if(head.next == &tail) {
                //Free list is empty and there is nothing init apart from the head
                printf("The free list is empty\n");
                program_break = sbrk(PAGE_SZ);
                printf("The address of the program break is %10p\n",program_break);
                //*((size_t *)program_break) = size;
                tester = *((size_t *)program_break);
                printf("tester = %d\n",tester);

                alloc->block = program_break + sizeof(size_t);//ptr has the value of the start of the new address space allocated in the heap
                                                              //                alloc->block = sbrk(0);//ptr has the value of the start of the new address space allocated in the heap
                alloc->len = size;
                //                program_break = (void *)size_var;
                printf("The pagesize if %d\n", PAGE_SZ);
                printf("\nThe address that alloc->block points to is %10p\n",alloc->block);
                printf("The size of the datatype is size_t is %d\n\n",sizeof(size_t));
                printf("Head points to %10p\n",&head);
                printf("Tail points to %10p\n",&tail);



                //Now give the remaining memory i.e. PAGE_SZ - size to the free list.
                //This is done so that subsequent calls to malloc can be served from the free list rather than resetting the program break again and wasting space.

                if(PAGE_SZ - size > 0) {//we are asking for memory lesser than the PAGE_SZ
                        remaining = PAGE_SZ - size - 2*sizeof(size_t);
                        printf("The value of remaining is %d\n",remaining);
                        f->len = remaining;
                        f->block = alloc->block + size + sizeof(size_t);
                        tmp = alloc->block+size;
                        printf("The value of the tmp variable inside teh first condition is %10p\n",tmp);
                        *((size_t *)tmp) = remaining;
                        f->next = &tail;
                        f->prev = tail.prev;
                        head.next = f;
                        tail.prev = f;
#if 0
                        if(tail.prev) {
                                tail.prev->next = f;
                                tail.prev = f;
                        } else {
                                head.next = f;
                                tail.prev = f;
                        }
#endif
                } 
                printf_free_list();

                return alloc->block;
        } else {//free list is not empty
                f = &head;
                for (f; f->next!=NULL; f=f->next) {
                        printf("Looping in the free list\n");
                        if (f->len > size) {
                                f = &f_instance[f_instance_count];
                                printf("f->len > size \n");
                                //alloc->block = f->block + sizeof(size_t);
                                alloc->block = f->block;
                                alloc->len = size;
                                tmp = alloc->block - sizeof(size_t);
                                *((size_t *)tmp) = alloc->len;
                                printf("Allocated Block is at %10p and has a length of %d\n",alloc->block,alloc->len);
                                printf("Allocated Blocks Length extracted from alloc->block - sizeof(size_t) is %d\n",*((size_t *)tmp));
                                remaining = f->len - size - sizeof(size_t); // recheck if f->len is being filled by the malloc function when adding items to the free list
                                printf("Remaining after allocation is %d\n",remaining);
                                //*((size_t *)f->block) = size;
                                //Adding the remaining to the free list
                                tmp  = f->block + size;
                                *((size_t *)tmp) = remaining;
                                f->block = f->block + size + sizeof(size_t);
                                f->len = remaining;
                                printf("Remaining Block inserted in the free list at Block Address %10p and length %d\n",f->block,f->len);
                                printf("Remaining Blocks Length extracted from f->blocks - sizeof(size_t) is %d\n",*((size_t *)tmp));
                                //what about f->next and f->prev

                                printf("\n\n    [IGNORE]   Performing a test to see I am able to get the value of length from before the address of alloc->block\n");
                                tester = alloc->block - sizeof(size_t);
                                printf("The value of that is %d\n",tester);
                                printf_free_list();
                                return alloc->block;

                        } else if (f->len == size + sizeof(size_t)) {
                                f = &f_instance[f_instance_count++];
                                printf("f->len == size\n");
                                alloc->block = f->block + sizeof(size_t);
                                //alloc->len = f->len - sizeof(size_t);
                                alloc->len = f->len;
                                //*((size_t *)tmp) = alloc->block - sizeof(size_t);
                                tmp = alloc->block - sizeof(size_t);
                                printf("Allocated Block is at %10p and has a length of %d\n",alloc->block,alloc->len);
                                printf("Allocated Blocks Length extracted from alloc->block - sizeof(size_t) is %d\n",*((size_t *)tmp));
                                //Clear up the free list of this item
                                f->prev->next = f->next;
                                f->next->prev = f->prev;
                                f->block = NULL;
                                printf_free_list();
                                return alloc->block;
                        } else {
                                printf("f->len < size so continue\n");
                                printf_free_list();
                                continue;
                        }
                }
        }
        printf("Things reached return NULL inside the malloc_v1 function\n");
        return NULL;
}
#if 0
void free_v1(void *ptr)
{
        /*
           Add the address of that to the free_list_item, along with the length.
           point the item to the end of the end just before the tail 
           make the current pointer NULL
         */
        //struct free_list_item *f = &f_instance;
        struct free_list_item *f = &f_free;
        struct allocated_item *alloc = &alloc_instance;
        struct free_list_item *end= &tail;
        struct free_list_item *en;

        printf("\n\n -------------------------------- Entered free -----------------------------\n\n");
        alloc->block = ptr;
        alloc->len = *((size_t *)(ptr - sizeof(size_t)));
        printf("Allocated Block is at %10p and has a length(EXTRACTRED FROM before the ptr) of %d\n",alloc->block,alloc->len);

        //f = tail.prev;
        f->block = alloc->block;
        f->len = alloc->len;
        f->prev = tail.prev;
        f->next = &tail;
        tail.prev = f;
        tail.prev->next = f;
        //        en = end->next;
        //      en->next = f;
        ptr = NULL;
        alloc->block = NULL;
        //printf_free_list();
        return ;
}
#endif
#if 1
void free_v1(void *ptr) {
        struct free_list_item *fr = &f_free[f_count++];
        fr->block = ptr;
        fr->len = *((size_t *)(ptr - sizeof(size_t)));
        fr->next = &tail;
        fr->prev = tail.prev;
        tail.prev->next = fr;
        tail.prev = fr;
}
#endif
int main(int argc, char *argv[])
{
        int t;
        void *ptr1, *ptr2, *ptr3;
        /*
           while (t = getopt(argc, argv, "a") != -1) {
           switch(t) {
           case 'a' : 

           };
           }*/
        head.next = &tail;
        tail.prev = &head;
        ptr1 = malloc_v1(100);
        if(ptr1 == NULL) {
                printf("Malloc1 returned NULL\n");
                return 0;
        }
        printf("The address of ptr1 is %10p\n",ptr1);
#if 1
        ptr2 = malloc_v1(100);
        if(ptr2 == NULL) {
                printf("Malloc2 returned NULL\n");
                return 0;
        }
        printf("The address of ptr2 is %10p\n",ptr2);
        ptr3 = malloc_v1(100);
        if(ptr3== NULL) {
                printf("Malloc3 returned NULL\n");
                return 0;
        }
        printf("The adrress of ptr3 is %10p\n",ptr3);
#endif
        printf("\n\nPrinting the free list after all MALLOCS\n\n");
        printf_free_list();


        printf("\n\nFreeing ptr1\n\n");
        free_v1(ptr1);
        printf("\nThe free list now looks like *****************\n");
        printf_free_list();


        printf("\n\nFreeing ptr2\n\n");
        free_v1(ptr2);
        printf("\nThe free list now looks like *****************\n");
        printf_free_list();
        return 0;
}




