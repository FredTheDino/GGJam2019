// THIS FILE
// Handles all the memory of the entire game.
#define _PRINT_ALLOCATIONS_ 0
#define TEMP_MEMORY_BLOCK_SIZE 0xFFFFFF

struct MemoryAllocation
{
	u64 id;
	u64 size;
	MemoryAllocation *next;
};

struct MemoryState
{
	MemoryAllocation *head = NULL;
	s64 watermark = 0;

	u64 temp_watermark;
	void *temp;
	void *temp_a, *temp_b;
} memory;


// 
//	Memeory Untility
//

// Check how the memeory usage is looking in the engine.
void debug_check_memory_watermark();

// Check the memory in use, and free if specified.
void debug_check_memory(bool free=false);

// Copy a block of memory from "src" to "dest" with length "len".
void mem_copy(void *src, void *dest, u64 len);

// Zero a region of "size" at "ptr"
void zero_block(void *ptr, u64 size);
// Zero a region with the size of the struct
#define zero_struct(ptr, type) (zero_block(ptr, sizeof(type)))
// Zero a region with the size of the array
#define zero_array(ptr, type, num) (zero_block(ptr, sizeof(type) * num))

// 
//	Temporary Memory
//

// Allocate all the temporary memory
void initalize_temp_memory();
// Free all the temporary memory
void destroy_temp_memory();

// Swap the two temporary memeory buffers.
void swap_temp_memory();

// Quickly return a pointer to memory of the given size, valid for 2 frames.
void *temp_memory(u64 size); 
// Quickly return a pointer for a struct of type "type", valid for 2 frames.
#define temp_struct(type) ((type *) temp_push(sizeof(type)))
// Quickly return a pointer for an array of type "type" and length "Num", valid for 2 frames.
#define temp_array(type, num) ((type *) temp_push(sizeof(type) * num))

//
// Persistent Memory
//

// Allocate a block of memory with the size of "mem_size".
void *push_memory(u64 mem_size);
// Allocate a block of memory with the size of the type specified.
#define push_struct(type) ((type *) push_memory(sizeof(type)))
// Allocate a block of memory for the array of length "num" and the size of "type".
#define push_array(type, num) ((type *) push_memory(sizeof(type) * num))

// A wrapper function for the macro with similar name.
void *push_struct_and_copy_(u64 size, void *data);
// Take a struct and allocate memory for it on the heap and copy the struct.
#define push_struct_and_copy(type, data) ((type *) push_struct_and_copy_(sizeof(type), (void *) &data))

// Free a region of allocated persistent memory.
void pop_memory(void *data_ptr);

// Reallocate and move a region of persistent memory so it fits in the new size.
void *resize_memory(void *ptr, u64 new_size);
// Reallocate and move the array to fit in the new size.
#define resize_array(ptr, type, num) ((type *) resize_memory(ptr, sizeof(type) * num))

