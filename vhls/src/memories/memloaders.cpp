// memloaders.cpp
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2009-11 D J Greaves
// University of Cambridge, Computer Laboratory.
// ACS P35 SoC D/M Classes - $Id: sram64_cbg.cpp,v 1.18 2011/07/25 15:34:01 my294 Exp $
//

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define TRC(X) 
#define LDTRC(X) 
#define ELFTRC(X) 
#define DTBTRC(X) 

#include <endian.h>

#include "memloaders.h"
#include "argv_backdoor.h"


  //
  // Load elf image to buffer based at p.
  // Return hwm.
  // 
  // Byte4 is EL_CLASS which is 1 for 32 bit data and 2 for 64.
  // Byte5 is EL_DATA which  is 1 for little endian and 2 for big.
u64_t cbgelf::reader(bool &big_endian, const char *fn, u64_t mem_base, u64_t mem_size_bytes, u64_t *entrypt)
{
  int hwm = 0, fd = open(fn, O_RDONLY);
#if __BYTE_ORDER == __LITTLE_ENDIAN
  bool host_endianness = false;
#elif __BYTE_ORDER == __BIG_ENDIAN
  bool host_endianness = true;
#else
#error elsewise
#endif
  //tlm::tlm_endianness host_endianness = tlm::get_host_endianness();

  struct stat statbuf;
  if (fd < 0 || fstat(fd, &statbuf) < 0)
    {
      perror("");
      printf("cbgelf: load %s\n", fn);
      perror("cbgelf: cannot open input file");
      exit(1);
    }

  int xoffset = 0;

  const char *l = (const char*) mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (l == MAP_FAILED)
    {
      perror("");
      printf("cbgelf: load %s\n", fn);
      perror("cbgelf: cannot map input file");
      exit(1);
    }
  if (strncmp(l+1, "ELF", 3))
    {
      printf("cbgelf: load %s\n", fn);
      perror("cbgelf: input file is not ELF format");
      exit(1);
    }

  // Intel is little endian.
  // We always load in to memory in workstation endian to assist with DMI
  // The ELF file may be big endian if byte 5 is 2.
  if (l[EI_DATA] == ELFDATA2MSB) { big_endian = true /*tlm::TLM_BIG_ENDIAN*/; }

  if(l[EI_CLASS] == ELFCLASS32)
    {
      struct elf32_hdr *hdr = (struct elf32_hdr *) l;
      int magic_no = ((int *)l)[1];
      tenos_check_correct_magic_no(32, magic_no, true);

      if (entrypt) 
	{
	  *entrypt = ES32(big_endian, hdr->e_entry - xoffset);
	  ELFTRC(printf("Entry point %lx\n", *entrypt));
	}

      // Intel CPUs are little-endian.
      // Endian swap is needed when loading big-endian ELF binaries on an x86 little endian workstation.
      Elf32_Shdr *shdrs = (Elf32_Shdr *) (l + ES32(big_endian, hdr->e_shoff));
      char *strings = 0;
      for (int pass=0; pass<3; pass++) 
	for (int i=0; i < ES16(big_endian, hdr->e_shnum); i++)
	  {
	    int foffset = ES32(big_endian, shdrs[i].sh_offset);
	    unsigned int size = ES32(big_endian, shdrs[i].sh_size);
	    Elf32_Word type = ES32(big_endian, shdrs[i].sh_type);
	    int sh_addr = ES32(big_endian, shdrs[i].sh_addr);
	    ELFTRC(printf("ELF32 section %i: %s type=%i xoffset=%x foffset=%x sh_addr=%x size=%x\n", i, /*need string table: shdrs[i].e_ident*/"", type, xoffset, foffset, sh_addr, size));
	    if (pass == 2 && type == SHT_NOBITS) // bss - need to clear it
	      {
		ELFTRC(printf("Section %i is bss - cleared to zero at 0x%X size=0x%X\n", i, sh_addr, size));
		for (int k=0; k<size; k++)
		  {
		    unsigned char dd = 0;
		    int a = k + sh_addr - mem_base;
		    //printf("Clear bss %x\n", a);
		    par->ip_write(a, dd, host_endianness != big_endian);
		    if(a > hwm)
		    hwm = a;
		  }
	      }
	  if (pass == 0 && type == SHT_STRTAB) // symbol table
	    {
	      strings = (char *) (foffset + l);
	      //printf("Strings at foffset %x\n", foffset);
	    }
	  // need argv backdoor setup before elf load.
	  // printf ("pass == %i && type == %i cf %i && argv_backdoor_su %p && strings %p\n", pass, type, SHT_SYMTAB, argv_backdoor_su, strings);
	  
	  if (pass == 1 && type == SHT_SYMTAB && argv_backdoor_su && strings) // symbol table
	    {
	      int n_symbs = size/sizeof(Elf32_Sym);
	      ELFTRC(printf("ELF32 %i symbols in symbole table. foffset=%x sh_addr=%x\n", n_symbs, foffset, sh_addr));
	      Elf32_Sym *syms = (Elf32_Sym *)(foffset + l);
	      for (int k=0; k<n_symbs; k++) 
		{
		  if (syms[k].st_name) // some can be anon
		    {
		      //printf(" %llX %i\n", syms[k].st_value, syms[k].st_name);
		      char *symb = strings + ES32(big_endian, syms[k].st_name);
		      u32_t vale = ES32(big_endian, syms[k].st_value);
		      ELFTRC(printf("Define ELF32 symbol k=%i value=%X char*=%s\n", k, vale, symb));
		      argv_backdoor_su->add_symb(symb, vale, syms[k].st_info);
		    }
		}
	      //printf("symbols loaded\n");
	    }
	  }
      Elf32_Phdr *phdrs = (Elf32_Phdr *) (l + ES32(big_endian, hdr->e_phoff));
      for (int i=0; i < ES16(big_endian, hdr->e_phnum); i++)
	{
	  ELFTRC(int printlimt = 10000);
	  int vaddr = ES32(big_endian, phdrs[i].p_vaddr);
	  int foffset = ES32(big_endian, phdrs[i].p_offset);
          int paddr = ES32(big_endian, phdrs[i].p_paddr);
	  int size = ES32(big_endian, phdrs[i].p_filesz);
	  ELFTRC(printf("Prog section %i: type=%i offset=%x foffset=%x va=%x pa=%x size=%x\n", i, ES32(big_endian, phdrs[i].p_type), xoffset, foffset, vaddr, paddr, size));
	  for (int d = 0; d<size; d++)
	    {
	      int a = d + paddr - xoffset - mem_base;
	      if (a < 0 || a >= mem_size_bytes)
		{
		  printf("cbgelf32: %s load address " PFX64 " is outside of memory model " PFX64 " .. " PFX64 "\n",
			 fn, a+mem_base, mem_base, mem_base+mem_size_bytes);
			 //You may need the bug fix from the line above: ie dont use 'a_old'",
		  exit(1);
		}
	      else 
		{
		  unsigned char dd = l[foffset + d];
		  ELFTRC(if (printlimt-- > 0) printf("load RAM with byte RAM[%x]=%x\n", a, dd));
		  par->ip_write(a, dd,  host_endianness != big_endian);
		}
	      if (a>hwm) hwm=a;
	    }
	}
    }
  else if(l[4] == ELFCLASS64)
    {
      struct elf64_hdr *hdr = (struct elf64_hdr *) l;
      if (entrypt) 
	{
	  *entrypt = ES64(big_endian, hdr->e_entry - xoffset);
	  ELFTRC(printf("Entry point " PFX64 "\n", *entrypt));
	}

      int magic_no = ((int *)l)[1];
      tenos_check_correct_magic_no(64, magic_no, true);

      Elf64_Shdr *shdrs = (Elf64_Shdr *) (l + ES64(big_endian, hdr->e_shoff));
      char *strings = 0;
      for (int pass=0; pass<3; pass++) 
	for (int i=0; i < ES16(big_endian, hdr->e_shnum); i++)
	{
	  Elf64_Off foffset = ES64(big_endian, shdrs[i].sh_offset);
	  Elf64_Xword size = ES64(big_endian, shdrs[i].sh_size);
	  Elf64_Word type = ES32(big_endian, shdrs[i].sh_type);
	  Elf64_Xword sh_addr = ES64(big_endian, shdrs[i].sh_addr);
	  ELFTRC(printf("Section section %i: %s type=%i foffset=" "%llX" " at " "%llX" " size=" "%llX" "\n", i, /*need string table: shdrs[i].e_ident*/"", type, foffset, sh_addr, size));
	  if (pass == 2 && type == SHT_NOBITS) // bss - need to clear it
	    {
	      ELFTRC(printf("Section %i is bss - cleared to zero at 0x%X size=0x%X\n", i, sh_addr, size));
	      for (u64_t k=0; k<size; k++)
		{
		  unsigned char dd = 0;
		  u64_t a = k + sh_addr - mem_base;
		  //printf("Clear bss %x\n", a);
		  par->ip_write(a, dd, host_endianness != big_endian);
		  if(a > hwm)
		    hwm = a;
		}
	    }
	  if (pass == 0 && type == SHT_STRTAB) // symbol table
	    {
	      strings = (char *) (foffset + l);
	      ELFTRC(printf("elf64: Strings at foffset %lx\n", foffset));
	    }
	  //printf ("pass == %i && type == %i cf %i && argv_backdoor_su %p && strings %p\n", pass, type, SHT_SYMTAB, argv_backdoor_su, strings);
	  if (pass == 1 && type == SHT_SYMTAB && argv_backdoor_su && strings) // symbol table
	    {
	      int n_symbs = size/sizeof(Elf64_Sym);
	      ELFTRC(printf("%i symbols %lli %llx\n", n_symbs, foffset, sh_addr));
	      Elf64_Sym *syms = (Elf64_Sym *)(foffset + l);
	      for (int k=0; k<n_symbs; k++) 
		{
		  //printf(" %llX %i\n", syms[k].st_value, syms[k].st_name);
		  char * symb = strings + syms[k].st_name;
		  ELFTRC(printf("Define ELF symbol %lX %s\n", syms[k].st_value, symb));
		  argv_backdoor_su->add_symb(symb, syms[k].st_value, syms[k].st_info);
		}
	    }
	}
      
      for (int i=0; i < ES16(big_endian, hdr->e_phnum); i++)
	{
	  Elf64_Phdr *phdrs = (Elf64_Phdr *) (l + ES64(big_endian, hdr->e_phoff));
	  //	  Elf64_Phdr *phdrs = (Elf64_Phdr *)(l + sizeof(elf64_hdr)); 
	  Elf64_Off foffset = ES64(big_endian, phdrs[i].p_offset);
	  Elf64_Xword size = ES64(big_endian, phdrs[i].p_filesz);
	  u64_t v = ES64(big_endian, phdrs[i].p_vaddr);
	  ELFTRC(printf("Prog section %i: type=%i foffset=" "%llX" " va=" PFX64 " size=" "%llX" "\n", i, ES32(big_endian, phdrs[i].p_type), foffset, ES32(big_endian, v), size));

	  for (int d = 0; d<size; d++)
	    {
	      u64_t a = d + v - xoffset - mem_base;
	      if (a < 0LL || a >= mem_size_bytes)
		{
		  printf("cbgelf64: %s load address " PFX64 " is outside of memory model " PFX64 " .. " PFX64 "\n",
			 fn, a+mem_base, mem_base, mem_base+mem_size_bytes);
		  exit(1);
		}
	      else 
		{
		  unsigned char dd = l[foffset + d];
		  ELFTRC(printf("load RAM with byte RAM[" PFX64 "]=%x\n", a, dd));
		  par->ip_write(a, dd, host_endianness != big_endian);
		}
	      if (a>hwm) hwm=a;
	    }
	}
    }
  else assert(0); // not 32 or 64 bit!
  close(fd);
  return hwm;
}

// constructor
cbgelf::cbgelf(memloader_if *p) : par(p) {  }

// constructor
cbgihex::cbgihex(memloader_if *p) : par(p)
{
  csum = 0; 
  a = 0; 
  finished = 0;
  p = 0;
};

// constructor
cbgdtb::cbgdtb(memloader_if* p) : par(p) {}

int cbgihex::iread(int len)
{
  int r = 0;
  int i;
  for (i=0; i < len; i++)
  {
    char c = toupper(getc(fd));
    while (c == ' ') c = toupper(getc(fd));
    c = (c <= '9') ? c-'0': c-('0'+7);
    r = (r<<4) + c;
  }
  return r;
}

//
// Load intel hex file to buffer based at p.
// Return ending p.
// 

u64_t cbgihex::reader(bool &big_endian, const char *fn, u64_t mem_base, u64_t lenmax)
{
  int line = 1; 
  char bdata[256];
  fd = fopen(fn, "r");
  if (fd == 0) 
    {
      printf("cbgihex load %s\n", fn);
      perror("cbgihex: cant open input file");
      exit(1);
    }
  while (feof(fd)==0)
  {
    char c = getc(fd);
    int llen, laddr, ltype;
    char lsum;
    int wq;
    if (c == '\n') line++;
    if (c != ':') continue;
    llen = iread(2);
    laddr = iread(4);
    ltype = iread(2);

    lsum = llen + laddr + ltype + (laddr >> 8);

    for (wq=0; wq<llen; wq++)
    {
      char data = iread(2);
      lsum = lsum + data;
      bdata[wq] = data;
    }
    lsum += iread(2);
    if (lsum != 0) printf("Checksum error in line %i, residue %i\n", line, lsum);
    
    if (ltype == 1) finished = 1;
    else if (ltype == 2 && llen == 2) 
      {
	if (laddr) printf("ignored type2\n");
	if (bdata[0]) printf("ignored type2\n");
	if (bdata[1]) printf("ignored type2\n");
      }
    else if (ltype == 0)
      {
	int wq;
	if (a != -1 && laddr != a)
	  {
	    printf("Disjoint addresses %x and %x in line %i\n",
		   laddr, a, line);
	  }
	
	for (wq=0; wq<llen; wq++)
	  {
	    unsigned char data = bdata[wq];
	    csum += data;
	    par->ip_write(p++, data);
	    if (lenmax-- <= 0) assert(0); // Ran out of room.
	  }
	a = laddr + llen;
      }
    else
      {
	printf("Unknown type %i encountered in line %i\n", ltype, line);
	continue;
      }
  }
  fclose(fd);
  return p;
}

u64_t cbgdtb::reader(bool &big_endian, const char* blob, u64_t offset, u64_t mem_size_bytes) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
  bool host_endianness = false;
#elif __BYTE_ORDER == __BIG_ENDIAN
  bool host_endianness = true;
#else
#error elsewise
#endif
  int fd = open(blob, O_RDONLY);
  printf("cbgdtb: load %s\n", blob);

  if(fd == 0) {
    perror("cbgdtb: can't open input file\n");
    exit(1);
  }

  struct stat statbuf;
  if(fstat(fd, &statbuf) < 0) {
    perror("cbgdtb: cannot get input file statistics\n");
    exit(1);
  }

  const unsigned char *l = (const unsigned char*) mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);

  int a = 0;
  for(; a < statbuf.st_size; a++) {
    DTBTRC(printf("cbgdtb: load RAM with byte RAM[%x]=%x\n", a + offset, l[a]));
    par->ip_write(a + offset, l[a], host_endianness != big_endian);
  }

  close(fd);

  return a + offset;

}

void cbgihex::dump(char *p, int l)
{
  while (l > 0)
    {
      opbyte(*p++);
      l -= 1;
    }
  opfl();
}


void cbgihex::opfl()
{
  unsigned char csum = 0; 
  int i;
  if (p == 0) return;
  printf(":%02X%04X00", p, a & 0xFFFF);

  csum += p + a + (a >> 8);
  for (i=0; i<p; i++) 
    {
      csum += opd[i];
      printf("%02X", opd[i]);
    }
  printf("%02X\n", (256-csum) & 0xFF);
  a+=p;
  p = 0;
}


void cbgihex::opbyte(int x)
{
  opd[p++] = x;
  if (p >= 32) opfl();
}


// eof
