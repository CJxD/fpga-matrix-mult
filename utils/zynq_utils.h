#ifndef ZYNQ_UTILS_H
#define ZYNQ_UTILS_H

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

#define PAGE_SIZE 4096UL
#define PAGE_MASK (PAGE_SIZE - 1)

static void* map_page(off_t target)
{
	int fd = -1;
	unsigned int page = 0;
	void *map_base = 0;

	fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (fd < 0)
	{
		fprintf(stderr, "lowlevel_accessor failed %s:%i Am I running with root privelege?\n", __FILE__, __LINE__);
		return 0;
	}

	if (map_base) // check page is still the correct one                                                                                                                                                       
	{
		unsigned int page_primed = target & ~PAGE_MASK;
		if (page != page_primed)
		{
			fprintf(stderr, "lowlevel_accessor failed %s:%i new page requested : %x cf %x\n", __FILE__, __LINE__, page_primed, page);
			return 0;
		}
	}
	else
	{
		// Map one page
		page = target & ~PAGE_MASK;
		map_base = mmap(0, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, page);
		if (map_base == (void *) -1)
		{
			fprintf(stderr, "lowlevel_accessor failed %s:%i failed to map page\n", __FILE__, __LINE__);
			return 0;
		}
	}
	
	close(fd);

	return map_base;
}

static void unmap_page(void* page)
{
	if (page > 0)
		munmap(page, PAGE_SIZE);
}

static void _zynq_gic_ints_enable(int enable)
{
	off_t target = 0xF8F01000; // GIC master interrupt control register
	void *map_base = map_page(target);
	void *virt_addr = map_base + (target & PAGE_MASK);
	
	((unsigned long *) virt_addr)[0] = enable ? 1 : 0;
	
	unmap_page(map_base);
}

static void zynq_disable_interrupts()
{
	_zynq_gic_ints_enable(0);
}

static void zynq_enable_interrupts()
{
	_zynq_gic_ints_enable(1);
}

static void zynq_enable_caches()
{
#if defined(__arm__) || defined(__thumb__)
	// L1 Cache (see Zynq TRM for further details)
	__asm__("mov r0, #0x1000");
	__asm__("orr r0, r0, #4");
	__asm__("mcr p15, 0, r0, c1, c0, 0");

	// L2 Cache
	off_t l2target = 0xF8F02100;
	((unsigned long *) l2target)[0] = 1;
#else
	#warning skipping ARM/THUMB cache control code
#endif
}

#endif
