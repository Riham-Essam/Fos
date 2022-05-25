#include <inc/memlayout.h>
#include <kern/kheap.h>
#include <kern/memory_manager.h>

//2022: NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)

#define Num_Of_Pages ((KERNEL_HEAP_MAX - KERNEL_HEAP_START)) / PAGE_SIZE

struct virtAddresses {
	int size;
	uint32 virtAddress;
} arrOfVirtAdd[Num_Of_Pages];

uint32 kStart = KERNEL_HEAP_START;
uint32 kEnd = KERNEL_HEAP_MAX;

//unsigned int reqSizeArr[Num_Of_Pages];

int indexOfAlloc = 0;

void* kmalloc(unsigned int size) {

	//TODO: [PROJECT 2022 - [1] Kernel Heap] kmalloc()
	// Write your code here, remove the panic and write your code

	//Pages should be allocated continuously ,, frames may be allocated at anywhere in mem

	uint32 startVaAdd;
	uint32 endVaAdd;
	int flag = 0;
	size = ROUNDUP(size, PAGE_SIZE);
	//uint32* currAddressPtr = NULL;
	int numofAllocatedPages = 0;
	int numberOfPages = size / PAGE_SIZE;

	struct Frame_Info* theFreeFrameptr;

	if (KERNEL_HEAP_MAX - KERNEL_HEAP_START < size) {
		return NULL;
	}

	for (uint32 kHeapItr = kStart; kHeapItr <= kEnd; kHeapItr += PAGE_SIZE) { //last allocation at kStart
		if (kHeapItr == kEnd && flag == 0) {

			kHeapItr = KERNEL_HEAP_START;
			flag = 1;

		}

		if (kHeapItr == kEnd && flag == 1) {

			break;
		}

		uint32* ptrTable = NULL;

		struct Frame_Info* the_Free_Frame_ptr = get_frame_info(ptr_page_directory,
				(void*) kHeapItr, &ptrTable);

		if (the_Free_Frame_ptr == NULL) {

			numofAllocatedPages++;

		} else {

			numofAllocatedPages = 0;
		}

		if (numofAllocatedPages == numberOfPages) {

			endVaAdd = kHeapItr + PAGE_SIZE;
			startVaAdd = endVaAdd - (numberOfPages * PAGE_SIZE);

			arrOfVirtAdd[indexOfAlloc].virtAddress = startVaAdd;
			arrOfVirtAdd[indexOfAlloc].size = numberOfPages;

			indexOfAlloc++;

			for (uint32 mapItr = startVaAdd; mapItr < endVaAdd; mapItr +=
			PAGE_SIZE) {

				int ret = allocate_frame(&theFreeFrameptr);

				if (ret != E_NO_MEM) { //To allocate frame
					map_frame(ptr_page_directory, theFreeFrameptr, (void*) mapItr,
					PERM_WRITEABLE);
				}
			}

			kStart = endVaAdd;

			return (void*) startVaAdd;

		}

	}

	return NULL;

//NOTE: Allocation using NEXTFIT strategy
//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
//refer to the project presentation and documentation for details

//TODO: [PROJECT 2022 - BONUS1] Implement a Kernel allocation strategy
// Instead of the Next allocation/deallocation, implement
// BEST FIT strategy
// use "isKHeapPlacementStrategyBESTFIT() ..."
// and "isKHeapPlacementStrategyNEXTFIT() ..."
//functions to check the current strategy
//change this "return" according to your answer

}

void kfree(void* virtual_address) {
//TODO: [PROJECT 2022 - [2] Kernel Heap] kfree()
// Write your code here, remove the panic and write your code

//To be tested ...

//unsigned int kheapArr[((KERNEL_HEAP_MAX - KERNEL_HEAP_START)) / PAGE_SIZE]; //to know size of heap that we will loop on
	int indexOfFreedVaAdd = -1;
	for (int reqAddress = 0; reqAddress < indexOfAlloc; reqAddress++) {
		if (virtual_address == (void*) arrOfVirtAdd[reqAddress].virtAddress) {
			indexOfFreedVaAdd = reqAddress;
			break;
		}
	}

	for (int i = 0; i < arrOfVirtAdd[indexOfFreedVaAdd].size; i++) {
		uint32* ptrToBeFreed = NULL;
		unmap_frame(ptr_page_directory, (void*) virtual_address);
		get_page_table(ptr_page_directory, (void*) virtual_address,
				&ptrToBeFreed);
		ptrToBeFreed[PTX((void* )virtual_address)] = 0;
		virtual_address += PAGE_SIZE;

	}

	arrOfVirtAdd[indexOfFreedVaAdd].virtAddress = (uint32) -1;
	arrOfVirtAdd[indexOfFreedVaAdd].size = 0;
//you need to get the size of the given allocation using its address
//refer to the project presentation and documentation for details

}

unsigned int kheap_virtual_address(unsigned int physical_address) {
//TODO: [PROJECT 2022 - [3] Kernel Heap] kheap_virtual_address()
// Write your code here, remove the panic and write your code
//panic("kheap_virtual_address() is not implemented yet...!!");

	struct Frame_Info *frameGivenVA = to_frame_info(physical_address);

	for (int addrItr = 0; addrItr < indexOfAlloc; addrItr++) {

		uint32* ptr = NULL;

		uint32 searchedAddress = arrOfVirtAdd[addrItr].virtAddress;
		int pageSize = arrOfVirtAdd[addrItr].size;

		for (int PageItr = 0; PageItr < pageSize; PageItr++) {

			//For the one in Arrary
			struct Frame_Info *frameSearchedVA = get_frame_info(
					ptr_page_directory, (void*) searchedAddress, &ptr);

			if (frameGivenVA != frameSearchedVA) {

				searchedAddress += PAGE_SIZE;

			} else {
				return searchedAddress;
			}
		}
	}

	return 0;
}

unsigned int kheap_physical_address(unsigned int virtual_address) {
//TODO: [PROJECT 2022 - [4] Kernel Heap] kheap_physical_address()
// Write your code here, remove the panic and write your code
//panic("kheap_physical_address() is not implemented yet...!!");
	uint32* input_virtual_address = (uint32*) virtual_address;
	uint32* page = NULL;
	get_page_table(ptr_page_directory, input_virtual_address, &page);
	if (page != NULL) {
		uint32 physical_address = (page[PTX(input_virtual_address)] >> 12)
				* PAGE_SIZE;
		return (physical_address);
	}
//return the physical address corresponding to given virtual_address
//refer to the project presentation and documentation for details

//change this "return" according to your answer
	return 0;
}

