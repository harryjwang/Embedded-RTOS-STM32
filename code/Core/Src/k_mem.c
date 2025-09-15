#include "k_mem.h"    // Header defining memory API prototypes and metadata structure
#include "k_task.h"   // Header for task management (osGetTID, current_task) to enforce ownership
#include <stdio.h>    // For debugging/printf, allowed per lab policies

// Global pointers and size for heap management
metadata *heap_head = NULL;   // Head of the linked list of metadata blocks (initially none)
U32 *heap_start;              // Start address of heap region (set in init handler)
U32 *heap_end;                // End address of heap region
size_t heap_size;             // Total size of the heap in bytes

// Symbols provided by the linker script
extern U32 _img_end;         // End of program image in RAM
extern U32 _estack;          // Top of stack region (highest RAM address)
extern U32 _Min_Stack_Size;  // Minimum reserved stack size for initial stack allocation

metadata* free_head = NULL;


// Debug function that printed out the entire free list
//static void dump_lists(const char *tag) {
//    metadata *m;
//
//    printf("\r\n[%s] free-list:\r\n", tag);
//    for (m = free_head; m; m = m->next_free) {
//        printf("  %p (sz=%lu) ↔ prev_free=%p next_free=%p\r\n",
//               m, m->size, m->prev_free, m->next_free);
//    }
//    printf("\n");
//}


int k_mem_init() {
    int result;
    __asm volatile(
        "SVC #7\n"           // Trigger SVC7 -> k_mem_init_handler
        "MOV %[result], r0\n"
        : [result] "=r" (result)
        :
        :"r0"
    );
    return result;            // Returns RTX_OK on success, RTX_ERR otherwise
}


int k_mem_init_handler() {
    // Ensure kernel has been initialized (PSP_next set by osKernelInit)
    if (PSP_next == NULL) {
        return RTX_ERR;
    }
    // Prevent re-initialization
    if (heap_head) {
        return RTX_ERR;
    }

    // Determine heap boundaries using end of program image + 0x200
    heap_start = (U32*)((char*)&_img_end + 0x200);
    heap_end   = (U32*)((char*)&_estack - (char*)&_Min_Stack_Size);
    heap_size  = (size_t)((char*)heap_end - (char*)heap_start);

    // Create an ALLOCATED sentinel at head to anchor the list
    heap_head = (metadata*)heap_start;
    heap_head->size  = 0;
    heap_head->state = ALLOCATED;  // sentinel must remain allocated
    heap_head->owner = 0;
    heap_head->prev  = NULL;
    heap_head -> next = NULL;

    // Create initial free block covering the remainder of the heap
    metadata *init_free = (metadata*)((char*)heap_start + sizeof(metadata));
    init_free->size  = heap_size - sizeof(metadata);
    init_free->state = FREE;
    init_free->owner = 0;
    init_free->prev  = heap_head;
    init_free->next  = NULL;

    // Link sentinel to first free block
    heap_head->next = init_free;

    // free-list linked list initialization
    free_head = init_free;
    init_free -> next_free = NULL;
    init_free -> prev_free = NULL;

    return RTX_OK;
}


void *k_mem_alloc(size_t size) {

    if (size == 0) {
        return NULL;              // Zero-byte requests invalid (Ch5: size==0 -> NULL) fileciteturn0file1
    } else if (heap_head == NULL) {
        return NULL;              // Heap not initialized
    }
    void *result;

    __asm volatile(
        "MOV r1, %[size]\n"
        "SVC #8\n"           // Trigger SVC8 -> k_mem_alloc_handler (Ch5: Memory Allocation) fileciteturn0file1
        "MOV %[result], r0\n"
        : [result] "=r" (result)
        : [size] "r" (size)
        :"r1", "r0"
    );
    return result;
}


void *k_mem_alloc_handler(size_t size) {

//	dump_lists("before alloc\r\n");

    // Round size up to 4-byte alignment
    size_t aligned_size = (size + 3) & ~3;

    // start the free_head block and traverse heap for first fit allocation
    metadata* check_block = free_head;
    metadata* lowest_block = NULL;

    // First-fit search
    while (check_block != NULL) {
        if (check_block->size >= aligned_size) {
            // Found a suitable block
            if (lowest_block == NULL || check_block < lowest_block) {
                lowest_block = check_block;
            }
        }
        // udpate check_block for next while iteration to check next free block
        check_block = check_block -> next_free;
    }

    //no free block exists
    if (lowest_block == NULL) {
        return NULL;
    }

    //too lazy to change variable names
    check_block = lowest_block;

    size_t remaining = check_block->size - aligned_size;
    void *user_ptr = (char*)check_block + sizeof(metadata);

    if(check_block -> next_free != NULL){
        check_block -> next_free -> prev_free = check_block -> prev_free;
    }

    if (check_block -> prev_free != NULL){
        check_block -> prev_free -> next_free = check_block -> next_free;
    } else if (check_block -> prev_free == NULL){
        free_head = check_block -> next_free;
    }

    check_block -> prev_free = NULL;
    check_block -> next_free = NULL;

    if (remaining >= sizeof(metadata) + 1) {
        // Split the block: allocate front portion, leave remainder
        metadata *new_meta = (metadata*)((char*)check_block + sizeof(metadata) + aligned_size);
        new_meta->size  = remaining - sizeof(metadata);
        new_meta->state = FREE;
        new_meta->owner = 0;
        new_meta->prev  = check_block;
        new_meta->next  = check_block->next;
        if (check_block->next != NULL) {
            check_block->next->prev = new_meta;
        }
        check_block->next = new_meta;
        check_block->size = aligned_size;

        new_meta -> prev_free = NULL;
        new_meta -> next_free = free_head;

        if (free_head != NULL){
            free_head -> prev_free = new_meta;
        }

        free_head = new_meta;
    }

    // Allocate entire block (either perfect fit or small sliver)
    check_block->state = ALLOCATED;
    check_block->owner = current_task;

    return user_ptr;
}


int k_mem_dealloc(void* ptr) {
    int result;
    __asm volatile(
        "MOV r1, %[ptr]\n"
        "SVC #9\n"           // Trigger SVC9 -> k_mem_dealloc_handler (Ch5: Deallocation) fileciteturn0file1
        "MOV %[result], r0\n"
        : [result] "=r" (result)
        : [ptr] "r" (ptr)
        :"r1", "r0"
    );
    return result;
}


int k_mem_dealloc_handler(void *ptr) {

//	dump_lists("before de-alloc\r\n");

    metadata *current = (metadata*)ptr - 1;
    metadata *prev = current -> prev;
    metadata *next = current -> next;

    // Validate pointer
    if ((char*)current < (char*)heap_start || (char*)(current + 1) > (char*)heap_end) {
        return RTX_ERR;
    }
    // Ownership and state checks
    if (current -> owner != current_task || current -> state != ALLOCATED || prev == NULL) {
        return RTX_ERR;
    }

   // Mark free
   current -> state = FREE;
   current -> owner = 0;

   // 1) Coalesce forward
   if (next && next -> state == FREE) {
	   if (next -> prev_free)
		   next -> prev_free -> next_free = next -> next_free;
	   else
		   free_head = next->next_free;
	   if (next -> next_free)
		   next -> next_free -> prev_free = next -> prev_free;

	   current -> size += sizeof(metadata) + next -> size;
	   current -> next  = next -> next;
	   if (next -> next)
		   next -> next->prev = current;
   }

   int did_backward_merge = 0;
   if (prev && prev->state == FREE) {
	   // Grow prev in-place (prev is already on free-list)
	   prev->size += sizeof(metadata) + current->size;
	   prev->next  = current->next;
	   if (current->next) current->next->prev = prev;
	   current = prev;
	   did_backward_merge = 1;
   }

   if (!did_backward_merge) {
	   current->prev_free = NULL;
	   current->next_free = free_head;
	   if (free_head) {
		   free_head->prev_free = current;
	   }
	   free_head = current;
   }

   // Debug code
//       metadata* m = heap_head;
//       printf(" [k_mem] full list:");
//       while(m) {
//         printf(" ->[%p sz=%lu st=%s]", (void*)m, m->size, (m->state==FREE)?"FREE":"ALLOC");
//         m = m->next;
//       }
//       printf("\r\n");

   return RTX_OK;
}



int k_mem_count_extfrag(size_t size) {
    if (size == 0 || heap_head == NULL) {
        return 0;            // Per spec: return 0 if uninitialized or invalid size fileciteturn0file1
    }
    int result;
    __asm volatile(
        "MOV r1, %[size]\n"
        "SVC #10\n"          // Trigger SVC10 -> handler below (Ch5: Utility Function)
        "MOV %[result], r0\n"
        : [result] "=r" (result)
        : [size] "r" (size)
        :"r1", "r0"
    );
    return result;
}


int k_mem_count_extfrag_handler(size_t size) {
    metadata *curr = heap_head;
    int count = 0;
    while (curr->next) {
        if (curr->state == FREE && (curr->size + sizeof(metadata)) < size) {
            ++count;
        }
        curr = curr->next;
    }
    return count;
}
