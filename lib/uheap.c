#include <inc/lib.h>

#define Num_Of_Pages ((USER_HEAP_MAX - USER_HEAP_START)) / PAGE_SIZE

struct virtAddresses {
	int size;
	uint32 virtAddress;
} arrOfVirtAdd[Num_Of_Pages];

int NumOfFramesAllHEAP[Num_Of_Pages] = {0};

uint32 hStart = USER_HEAP_START;
uint32 hEnd = USER_HEAP_MAX;
int indexOfAlloc = 0;

void* nextFit(int size) {

	uint32 startVaAdd;
	uint32 endVaAdd;
	int flag = 0;
	int numofAllocatedPages = 0;
	int numberOfPages = size / PAGE_SIZE;

	if (USER_HEAP_MAX - USER_HEAP_START < size) {
		return NULL;
	}

	for (uint32 heapItr = hStart; heapItr <= hEnd; heapItr += PAGE_SIZE) { //last allocation at kStart
		if (heapItr == hEnd && flag == 0) {
			heapItr = USER_HEAP_START;
			flag = 1;

		}

		if (heapItr == hEnd && flag == 1) {

			break;
		}

		// To get index of virt addr that is being looped on
		//To check if frame is empty or not and allocated by a continuous way

		int indexNeededForAllo = ((uint32) heapItr - USER_HEAP_START)
				/ PAGE_SIZE;

		//To check in the arr of all frames in heap
		//If the frame that is corresponding to virt add is empty or not

		if (NumOfFramesAllHEAP[indexNeededForAllo] == 0) {

			numofAllocatedPages++;

		} else {

			numofAllocatedPages = 0;
		}

		//The same way as in kHeap

		if (numofAllocatedPages == numberOfPages) {

			endVaAdd = heapItr + PAGE_SIZE;
			startVaAdd = endVaAdd - (numberOfPages * PAGE_SIZE);
			arrOfVirtAdd[indexOfAlloc].virtAddress = startVaAdd;
			arrOfVirtAdd[indexOfAlloc].size = numberOfPages;
			indexOfAlloc++;

			hStart = endVaAdd;

			return (void*) startVaAdd;

		}
	}

	return NULL;
}

// malloc()
//	This function use NEXT FIT strategy to allocate space in heap
//  with the given size and return void pointer to the start of the allocated space

//	To do this, we need to switch to the kernel, allocate the required space
//	in Page File then switch back to the user again.
//
//	We can use sys_allocateMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls allocateMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the allocateMem function is empty, make sure to implement it.

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

void* malloc(uint32 size) {
	//TODO: [PROJECT 2022 - [9] User Heap malloc()] [User Side]
	// Write your code here, remove the panic and write your code

	uint32* virtAdd;

	size = ROUNDUP(size, PAGE_SIZE);

	int NumberOfAllocatePages = size / PAGE_SIZE;

	virtAdd = nextFit(size);

	if (virtAdd == NULL) {
		return NULL;
	}

	sys_allocateMem((uint32) virtAdd, size);

	//To get index of first allocated virt add
	// As a counter to Update allocated frames in arr of all frames in heap

	int indexNeededForAllo = ((uint32) virtAdd - USER_HEAP_START) / PAGE_SIZE;

	//To Update allocated frames in arr of All frames in Heap

	for (int pagesItr = 0; pagesItr < NumberOfAllocatePages; pagesItr++) {

		NumOfFramesAllHEAP[indexNeededForAllo] = 1;
		indexNeededForAllo++;
	}

	return (uint32*) virtAdd;

	// Steps:
	//	1) Implement NEXT FIT strategy to search the heap for suitable space
	//		to the required allocation size (space should be on 4 KB BOUNDARY)
	//	2) if no suitable space found, return NULL
	//	 Else,
	//	3) Call sys_allocateMem to invoke the Kernel for allocation
	// 	4) Return pointer containing the virtual address of allocated space,
	//

	//This function should find the space of the required range
	// ******** ON 4KB BOUNDARY ******************* //

	//Use sys_isUHeapPlacementStrategyNEXTFIT() and
	//sys_isUHeapPlacementStrategyBESTFIT() for the bonus
	//to check the current strategy

}

void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable) {
	panic("smalloc() is not required ..!!");
	return NULL;
}

void* sget(int32 ownerEnvID, char *sharedVarName) {
	panic("sget() is not required ..!!");
	return 0;
}

// free():
//	This function frees the allocation of the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from page file and main memory then switch back to the user again.
//
//	We can use sys_freeMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls freeMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the freeMem function is empty, make sure to implement it.

void free(void* virtual_address) {
	//TODO: [PROJECT 2022 - [11] User Heap free()] [User Side]
	// Write your code here, remove the panic and write your code

	int indexOfFreedVaAdd = -1;
	int numOfFrames = 0;
	uint32 virtAddr;

	for (int reqAddress = 0; reqAddress < indexOfAlloc; reqAddress++) {

		if ((uint32) virtual_address == arrOfVirtAdd[reqAddress].virtAddress) {

			indexOfFreedVaAdd = reqAddress;

			break;
		}
	}

	numOfFrames = arrOfVirtAdd[indexOfFreedVaAdd].size;
	virtAddr = arrOfVirtAdd[indexOfFreedVaAdd].virtAddress;

	uint32 size = numOfFrames * PAGE_SIZE;
	size = ROUNDUP(size,PAGE_SIZE);


	// As a counter to Update allocated frames in arr of all frames in heap

	int indexNeededForAllo = ((uint32) virtAddr - USER_HEAP_START) / PAGE_SIZE;

	//To UnAllocate frames in arr of All frames in Heap
	for (int frameItr = 0; frameItr < numOfFrames; frameItr++) {

		NumOfFramesAllHEAP[indexNeededForAllo] = 0;
		indexNeededForAllo++;

	}

	sys_freeMem((uint32) virtual_address, size);

	arrOfVirtAdd[indexOfFreedVaAdd].virtAddress = (uint32) -1;
	arrOfVirtAdd[indexOfFreedVaAdd].size = 0;

	//you shold get the size of the given allocation using its address
	//you need to call sys_freeMem()
	//refer to the project presentation and documentation for details

}

void sfree(void* virtual_address) {
	panic("sfree() is not requried ..!!");
}

//===============
// [2] realloc():
//===============

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_moveMem(uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		which switches to the kernel mode, calls moveMem(struct Env* e, uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		in "memory_manager.c", then switch back to the user mode here
//	the moveMem function is empty, make sure to implement it.

void *realloc(void *virtual_address, uint32 new_size) {
	//TODO: [PROJECT 2022 - BONUS3] User Heap Realloc [User Side]
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");

	return NULL;
}
