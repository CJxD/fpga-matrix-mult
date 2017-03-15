// 
// $Id: argv_backdoor.h, $
// Part of prazor-virtual-platform
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2010-15 DJ Greaves, University of Cambridge, Computer Laboratory.
//
// A backdoor so that programs can read the command line argv argument vector
//
// Also we have in here the symbolic_dis function that helps give textual names based on debugging information in ELF files etc..
//

#include "argv_backdoor.h"


bool tenos_symbol_order_pred(tenos_symbol *l, tenos_symbol *r) { return l->value < r->value; }


tenos_symbol *argv_backdoor_setup::find_best_symbol(u64_t akey) 
  {
    if (!vsymbols_sorted)
      {
	std::sort(vsymbols.begin(), vsymbols.end(), tenos_symbol_order_pred);
	vsymbols_sorted = true;
      }

    size_t left = 0 ;
    size_t right = vsymbols.size(); // one position passed the right end
    // It might be worthwhile cacheing the last looked-up answer and see if that is again the best answer.
    while (left < right) // binary chop to find
      {
	size_t mid = left + (right - left)/2;
	if (akey >= vsymbols[mid]->value &&  (mid+1 >= vsymbols.size() || akey < vsymbols[mid+1]->value)) return vsymbols[mid];
	if (akey > vsymbols[mid]->value)
	  {
	    left = mid+1;
	  }
	else
	  { 
	    right = mid;
	  }
      }

    return 0;
  }

void argv_backdoor_setup::init(const char* image_name) {
  if (!image_name)
    image_name = "noname.out";
  
  const int memsize = 65536;
  const int dsize = memsize/2;
  l = dsize * 2;
  a0[0] = 0x0;
  a0[1] = 0x100;
  a0[2] = 0x0 + dsize;
  a0[3] = 0x100 + dsize;
  
  a1[0] = 0x300;
  a1[1] = 0x300+dsize;
  
  argv_ram = new unsigned char [memsize];
  argv_ram_backdoor = argv_ram; 
}

void argv_backdoor_setup::insert(const char *s1, u64_t BE, u64_t LE, bool end) {
  assert(a0[0] < 0x100);       
  for (int d0 = 0; d0 < 2; d0++) {
    int be = (d0 > 0);
    for (int d1=0; d1 < 2; d1++) { // Do all four combinations of argv: be/le * m32/m64.
      int sz = (d1 > 0) ? 8:4;
      u64_t hi = (be) ? BE : LE; // Use 64 bit base addresses for 32 as well - top bits just ignored.
      // Although the same pointers could be potentially be used for both BE and LE,
      // they point at different versions of the strings.
      for (int q = 0; q < sz; q++) {
	u8_t dd = (a1[d0] | hi) >> (8 * ((be) ? (sz-1-q): q)); 
	put(dd, a0[d0*2+d1], be); 
      }
    }

    const char *s = s1;
    while (*s) put(*s++, a1[d0], be);
    if(end)
      put(*s, a1[d0], be);
    assert(a1[d0] < 2*l);
  }
}

argv_backdoor_setup::argv_backdoor_setup(const char *image_name, FILE* input)
  : m_argc1(0) {

  const int BUFFERSIZE = 255;
  char buffer[BUFFERSIZE];

  init(image_name);
  
  while(fgets(buffer, BUFFERSIZE, input)) {
    printf("STDIN: inserting buffer %s\n", buffer);
    insert(buffer, SOCDAM_STDIN_BASE_64_BE, SOCDAM_STDIN_BASE_64_LE, false);
  }

}

argv_backdoor_setup::argv_backdoor_setup(const char *image_name, int argc, char *argv[])
  : m_argc1(argc+1) { //constructor

  init(image_name);
  int d = -1;
  // Copy argv into a packed form in a little RAM to be mapped into backdoor space.
  // Provide both 32 and 64 bit versions.
  while (d < argc) {
    const char *s1 = (d < 0) ? (d++, image_name): argv[d++];
    insert(s1, SOCDAM_ARGV_BASE_64_BE, SOCDAM_ARGV_BASE_64_LE, true);
  }
}

void argv_backdoor_setup::add_symb(char *name, u64_t addr, unsigned char mode)
{
  if (!name || name[0] == '$') return; // Discard relative names.
  tenos_symbol *symb = new tenos_symbol(name, addr, mode);
  vsymbols.push_back(symb);
  vsymbols_sorted = false;
}

// Give textual names based on debugging information in ELF files etc..
int argv_backdoor_setup::symbolic_dis64(char *buf, int n, u64_t a)
{
  struct tenos_symbol *x = 0;
  if (a != 0 && vsymbols.size() > 0) x = find_best_symbol(a);
  
  if (x && x->value == a) 
    {
      snprintf(buf, n, "0x%lX (%s)", a, x->name);
      return 1;
    }
  else if (x) 
    {
      snprintf(buf, n, "0x%lX (%s+0x%lx)", a, x->name, a-x->value);
      return 1;
    }
  else 
    {
      snprintf(buf, n, "0x%lX", a);
      return 0;
    }
}

// eof
