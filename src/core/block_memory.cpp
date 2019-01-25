void debug_check_memory_watermark()
{
	static s64 last = 0;
	if (last == memory.watermark) return;

	print("Current memory usage: %lli\n", memory.watermark);
	last = memory.watermark;
}

void debug_check_memory(bool free_all)
{
	MemoryAllocation *mem = memory.head;
	if (mem)
	{
		// NAGGING
		print("|| WARNING! UNFREED MEMEORY\n");
		u64 total = 0;
		u64 count = 0;
		while (mem)
		{
			total += mem->size;
			count++;
			print("|| \t(%p:%d) %d\n", mem, mem->id, mem->size);

			mem = mem->next;
		}
		print("|| TOTAL: %d leak(s) of %d bytes\n", count, total);
		
		// Leaking doesn't matter, so don't bother freeing stuff.
		// Don't bother, the OS is faster at this.
		if (free_all)
		{
			mem = memory.head;
			while (mem) 
			{
				void *ptr = mem;
				mem = mem->next;
				free(ptr);
			}
		}
	}
}

void mem_copy(void *src, void *dest, u64 len)
{
	// Is this a good idea? What if you want to copy 0
	// bytes? That's technicaly valid.
	ASSERT(dest);
	ASSERT(src);
	ASSERT(len);

	u8 *to = (u8 *) dest;
	u8 *from = (u8 *) src;
	while (len--) *(to++) = *(from++);
}

void zero_block(void *ptr, u64 size)
{
	u8 *data = (u8 *) ptr;
	for (; size > 0; size--)
		data[size - 1] = 0;
}

void initalize_temp_memory()
{
	// Initalize memory
	memory.temp_a = push_memory(TEMP_MEMORY_BLOCK_SIZE);
	memory.temp_b = push_memory(TEMP_MEMORY_BLOCK_SIZE);
	memory.temp = memory.temp_a;
}

void swap_temp_memory()
{
	debug_check_memory_watermark();

	if (memory.temp == memory.temp_a)
		memory.temp = memory.temp_b;
	else
		memory.temp = memory.temp_a;
	memory.temp_watermark = 0;
}

void destroy_temp_memory()
{
	pop_memory(memory.temp_a);
	pop_memory(memory.temp_b);
}

void *temp_memory(u64 size)
{
	void *ptr = (void *) ((u8 *) memory.temp + memory.temp_watermark);
	memory.temp_watermark += size;
	ASSERT(memory.temp_watermark < TEMP_MEMORY_BLOCK_SIZE);
	return ptr;
}

// Allocated memory
void *push_memory(u64 mem_size)
{
	ASSERT(mem_size != 0);
	static u64 id_counter = 1; // Increases with time.
	u64 size = sizeof(MemoryAllocation) + mem_size;
	void *ptr = malloc(size); // TODO: Potential speed up by changeing to malloc.
	memory.watermark += size;
	
	MemoryAllocation* alloc = (MemoryAllocation *) ptr;
	alloc->id = id_counter++;
	alloc->size = size;
	alloc->next = memory.head;

	// The data the user requested.
	void *data_ptr = (void *) (alloc + 1);

	// Insert into linked list.
	memory.head = alloc;

#if _PRINT_ALLOCATIONS_
	print("Allocating memory (%p:%u) %u\n", ptr, alloc->id, size);
#endif

	return data_ptr;
}

// Allocate an array with the correct type.
void *push_struct_and_copy_(u64 size, void *data)
{
	u8 *out = (u8 *) push_memory(size);
	u8 *p = out;
	u8 *d = (u8 *) data;
	while (size--) *(p++) = *(d++);
	return (void *) out;
}

// Free allocated memory
void pop_memory(void *data_ptr)
{
	ASSERT(data_ptr);
	MemoryAllocation *ptr = ((MemoryAllocation *) data_ptr) - 1;

	MemoryAllocation **prev = &memory.head;
	MemoryAllocation *it = memory.head;
	do 
	{
		if (ptr == it)
		{
			// Remove from linked list.
			*prev = it->next;
			break;
		}
		prev = &it->next; 
			    /* ASSIGN */ 
	} while ( (it = it->next) );
	ASSERT(it);

	memory.watermark -= ptr->size;
#if _PRINT_ALLOCATIONS_
	print("Freed (%p) %u\n", ptr, ptr->size);
#endif

	free(ptr);
}

void *resize_memory(void *ptr, u64 new_size)
{
	MemoryAllocation *alloc = ((MemoryAllocation *) ptr) - 1;
	if (new_size <= alloc->size)
		return ptr; // Maybe this shoudl raise an error?
	void *dest = push_memory(new_size);
	mem_copy(ptr, dest, alloc->size);
	pop_memory(ptr);
	return dest;
}

