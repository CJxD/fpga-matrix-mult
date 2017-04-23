// generic_tlm_mem : A TLM memory with with ELF and Intel Hex load facility.
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2009-12 D J Greaves
// University of Cambridge, Computer Laboratory.
// ACS P35 SoC D/M Classes - $Id: sram64_cbg.cpp,v 1.18 2011/07/25 15:34:01 my294 Exp $
//

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


#include "memloaders.h"

#include "generic_tlm_mem.h"



// Constructor
generic_tlm_mem::generic_tlm_mem(sc_core::sc_module_name name, 
				 u64_t mem_base, 
				 u64_t mem_size, 
				 u32_t width):
  sc_core::sc_module(name), 
  port0("port0"), 
  mem_base(mem_base), 
  mem_width(width), 
  llsc_locks(name),
  m_peq(this, &generic_tlm_mem::peq_cb)
#ifdef TLM_POWER3
  , pw_module("power_config_sram64.txt") 
#if PW_TLM_PAYLOAD > 0
  , read_bus_tracker(this)
#endif
#endif
{
  m_big_endian = tlm::TLM_LITTLE_ENDIAN;
  m_bits = (u64_t)mem_size * (u64_t)width;
  mem_size_bytes = mem_size / (width /8LLU);
  assert(width);
  assert(mem_size_bytes);

  int logbits = bound_log2(m_bits);
  u64_t bytes = m_bits/8LLU;
  printf("bits =" PFI64 "  bytes=" PFI64 " logbits=%i\n", m_bits, bytes, logbits);
  //printf("Mem %i %i bits hishift=%i mask=" PFX64 "   " PFX64 "\n", width, bound_log2(width), hishift, prazor_mask(width), prazor_mask(hishift));
  
  port0.register_b_transport(this, &generic_tlm_mem::b_access);
  port0.register_nb_transport_fw(this, &generic_tlm_mem::nb_access_fw);

  last_page = 0;
  port0.register_get_direct_mem_ptr(this, &generic_tlm_mem::get_direct_mem_ptr);
}



//
//
//
void generic_tlm_mem::loadme(const char *filename, bool elff, u64_t *entrypt)
{
  bool big_endian = false; // Is set by loader.
  if (elff)
    {
      eloader = new cbgelf(this);
      u64_t hwm = eloader->reader(big_endian, filename, mem_base, mem_size_bytes, entrypt);
      printf("%s: loaded elf file %s: hwm=0x" PFX64 "\n", name(), filename, hwm);
    }
  else 
    {
      iloader = new cbgihex(this);
      u64_t end = iloader->reader(big_endian, filename, mem_base, mem_size_bytes);
      printf("%s: loaded ihex file %s: " PFI64 " bytes\n", name(), filename, end);
    }


  m_big_endian = (big_endian) ? tlm::TLM_BIG_ENDIAN: tlm::TLM_LITTLE_ENDIAN;

  // host endianness (ie that of the modelling workstation)
  // tlm::tlm_endianness m_host_endianness = tlm::get_host_endianness();
  // cout << "m_endianness=" << m_endianness << "\n";

#if 0  
  // Little print out: uses ip_read8 but the ip_write used for load has had ES applied. 
  printf("%s: little printout (%s)\n", name(),
	  tlm::has_host_endianness(m_big_endian) ? "host endian matches loaded file endian":
	  "as stored on modelling workstation: models should see other endian");
#endif
  u64_t base = 0x80;
  if (base)
    for(int i=0;i<4;i++)
      {
	u64_t b = i*16LLU + base;
	printf("%s " PFX64 "::  ", name(), b);
	for (u64_t j=0;j<16;j++)
	  {
	    u8_t d;
	    bool present = ip_read(b+j, d);
	    if (present) printf("%02X ", d);
	    else printf("-- "); // print dashes for not present memory
	  }
	printf("\n");
      }
}


void generic_tlm_mem::peq_cb(PRAZOR_GP_T& trans, const tlm::tlm_phase& ph) {
  // this callback is invoked when delay argument that was passed to notify method
  // of m_peq has been reached
  
  // any operation on phase?
  switch(ph) {
    case tlm::BEGIN_REQ:
    case tlm::END_REQ:
    case tlm::BEGIN_RESP:
    case tlm::END_RESP:
      break;
    default:
      break;
  }

  sc_time delay = SC_ZERO_TIME;
  access(trans, delay);

  // once the call to method access() has finished the response can be
  // sent to the initator using bacward path if required
  
  tlm::tlm_phase bw_phase = tlm::END_REQ;
  // if nb_transport_bw is not registered on the initator socket
  // then this call should just simply return
  port0->nb_transport_bw(trans, bw_phase, delay);

}


// non blocking transport interface
tlm::tlm_sync_enum generic_tlm_mem::nb_access_fw(PRAZOR_GP_T &trans, 
						 tlm::tlm_phase &phase, 
						 sc_time &delay) 
{
  // here the correct delay needs to be calculated
  AUGMENT_LT_DELAY(trans.ltd, delay,  m_latency / 2);
  m_peq.notify(trans, phase, delay);

  return tlm::TLM_ACCEPTED;
}


void generic_tlm_mem::b_access(PRAZOR_GP_T &trans, sc_time &delay)
{
  AUGMENT_LT_DELAY(trans.ltd, delay, m_latency);
  access(trans, delay);
}

void generic_tlm_mem::access(PRAZOR_GP_T &trans, sc_time &delay)
{
  tlm::tlm_command cmd = trans.get_command();
  // std::cout << "Tmp debug k " << kind() << " n=" << name() << "\n";
  POWER3(PW_TLM3(pw_agent_record l_agent = PW_TLM3(trans.pw_log_hop(this,  (cmd==tlm::TLM_READ_COMMAND ? PW_TGP_DATA: PW_TGP_NOFIELDS) | PW_TGP_ACCT_CKP,  &read_bus_tracker))));

  // TODO cannot pass in tracker if missing when PAYTYPE<3

  u64_t      adr = ((u64_t)trans.get_address()) - mem_base;
  //  printf("%s addr= " PFX64 " - " PFX64 " = " PFX64 "  < " PFX64 "\n",  name(), ((u64_t)trans.get_address()), mem_base, adr, mem_size_bytes);
  u8_t*      ptr = trans.get_data_ptr();
  u32_t      len = trans.get_data_length();
  u8_t*      lanes = trans.get_byte_enable_ptr();
  //u32_t   wid = trans.get_streaming_width();

  //cout << name() << " b_access tmp debug " << m_latency << " now total " << delay <<"\n";

    
  // Obliged to check address range and check for unsupported features,
  //   i.e. byte enables, streaming, and bursts
  // Can ignore DMI hint and extensions
  // Using the SystemC report handler is an acceptable way of signalling an error

  if (adr >= mem_size_bytes)
      {
	printf("%s: RAM address 0x" PFX64 " out of range : 0x" PFX64 " .. 0x" PFX64 "\n", name(), adr+mem_base, mem_base, mem_base+mem_size_bytes);
	tenos_diagnostics.checkpoint(2, name(), "out of range");
	trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
	return;
      }

   
  if (len % 8) 
      {
	printf("%s: Illegal len or wid: len=%i wid=%i\n", name(),len, 64);
	trans.set_response_status(tlm::TLM_BURST_ERROR_RESPONSE);
	return;
      }

    // Illustrates that b_transport may block
    if (0)
      {
	wait(delay);
	delay = SC_ZERO_TIME; // Reset timing annotation after waiting
      }

#ifdef USE_PRESENCE_BITS
  mamba_extension * tastac;
  trans.get_extension(tastac);
  if (tastac && tastac->mcmd==mamba_extension::mamba_cmd_t::tastac)
    {
      if (traceregions && traceregions->check(adrr, TENOS_TRACE_MEM_WRITE))
	printf("%s:%s: Presence bit setclear=%i at " PFX64 "\n", name(), kind(), tastac->presbit, adr);
      u64_t temp = ip_presence(adr, tastac->presbit);
      *(u64_t *)ptr = temp; // reinterpret_cast<u64_t &>(ptr)
      trans.set_response_status(tlm::TLM_OK_RESPONSE);
      return;
    }
  else if (tastac && tastac->mcmd==mamba_extension::mamba_cmd_t::swa)
    {
      if (traceregions && traceregions->check(adrr, TENOS_TRACE_MEM_WRITE))
	printf("%s:%s: Word of presence setclear=%i at " PFX64 "\n", name(), kind(), tastac->presbit, adr);
      adr &= ~31;
      u64_t tmp = ((u64_t *)(ptr))[0];
      for (int v=0;v<32;v++) { ip_presence(adr + v, (tmp & 1LLU)); tmp >>= 1LLU; }
      trans.set_response_status(tlm::TLM_OK_RESPONSE);
      return;
    }
#endif

  llsc_extension * llsc;
  trans.get_extension(llsc);
  POWER3(pw_energy op_energy = PW_ZERO_ENERGY);
  int words = (len+7)/8;
  // Obliged to implement read and write commands
  if (cmd == tlm::TLM_READ_COMMAND)
    {
      stats.read_ops += 1;
      stats.read_words += words;
      if (llsc && llsc->loadlocked_fail(&llsc_locks, adr)) // The load linked phase should never normally fail.
	{
	  printf("%s:%s: load conditional read missed at " PFX64 "\n", name(), kind(), adr);
	  trans.set_response_status( tlm::TLM_ADDRESS_ERROR_RESPONSE );
	  return;
	}
      POWER3(op_energy += m_read_energy_op * words);
      int dd;
      for (dd=0; dd<len; dd+=8)
	{
	  u64_t temp;
	  bool ok = ip_read(adr+dd, temp);
	  *(u64_t *)(ptr+dd) = temp; // reinterpret_cast<u64_t &>(ptr)
	  if (traceregions && traceregions->check(adr, TENOS_TRACE_MEM_READ))
	    printf("%s: %s: Read %s a=" PFX64 " " PFX64 "\n", name(), kind(), llsc? "linked":"", adr, temp);
	}
#ifdef USE_PRESENCE_BITS
      // Want to return TLM_INCOMPLETE_RESPONSE when presence bits are missing
      // give TLM_INCOMPLETE_RESPONSE if not present for now
      if (!ok) 
	{
	  printf("Set incomplete response on read to " PFX64 "\n", adr);
	  trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
	}
      trans.set_response_status(tlm::TLM_OK_RESPONSE);
      return;
#endif
    }
  else if (cmd == tlm::TLM_WRITE_COMMAND)
    {
      stats.write_ops += 1;
      if (llsc)
	{
	  POWER3(op_energy += m_llsc_excess_energy_op);
	  if (llsc->storeconditional_fail(&llsc_locks, adr))
	    {
	      if (traceregions && traceregions->check(adr, TENOS_TRACE_MEM_READ))
		printf("%s:%s: store conditional write missed at " PFX64 "\n", name(), kind(), adr);
	      trans.set_response_status( tlm::TLM_ADDRESS_ERROR_RESPONSE );
	      return; // Return without doing the store: client will try again.
	    }
	}

      int bel = trans.get_byte_enable_length();
      POWER3(op_energy += m_write_energy_op * words);
      for (int dd =0; dd<len; dd+=8)
	{
	  if (!lanes || bel==0 || ((u64_t *)lanes)[(dd % bel)/8] == 0xFFFFffffFFFFffffLLU)
	    {
	      if (traceregions && traceregions->check(adr, TENOS_TRACE_MEM_WRITE))
		printf("%s:%s: (%i,%i,%p) Store64 %s " PFX64 "  %02x%02x%02x%02x  %02x%02x%02x%02x\n", name(), kind(), dd, bel, lanes, llsc?"conditional":"", adr, ptr[7], ptr[6], ptr[5], ptr[4], ptr[3], ptr[2], ptr[1], ptr[0]); 
	      u64_t *tmp = (u64_t *)(ptr);
	      //	  printf("%s b_access w64 " PFX64 "\n", name(), adr);
	      ip_write(adr+dd, tmp[dd/8]);
	    }
	  else  
	    {
	      if (traceregions && traceregions->check(adr, TENOS_TRACE_MEM_WRITE))
		printf("%s: %s: Store %s byte(s) lanes=" PFX64 " at " PFX64 " %02x %02x %02x %02x  %02x %02x %02x %02x\n", name(), kind(), llsc?"conditional":"", ((u64_t*)lanes)[0], adr, ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7]); 
	      // We put data in the RAM model in the endianness of the modelling workstation (little for x86).
	      // This requires a swap when the workstation and the system being modelled are different, but
	      // this swap is implemented in the processor model.
	      for (int l=0;l<8;l++) if (!lanes || lanes[(dd+l)%bel])  ip_write(adr+l+dd, ptr[l+dd]); 
	    }
	}
    }

#if PW_TLM_PAYLOAD > 0
  POWER3(l_agent.record_energy_use(op_energy, &trans));
#else
  POWER3(record_energy_use(op_energy, &trans));
#endif


  // Set DMI hint to indicated whether DMI is encouraged.
  trans.set_dmi_allowed(true);
  // Obliged to set response status to indicate successful completion
  trans.set_response_status(tlm::TLM_OK_RESPONSE);
}


bool generic_tlm_mem::get_direct_mem_ptr(PRAZOR_GP_T&, tlm::tlm_dmi &dmi_data)
{
  if (last_page)
    {
      dmi_data.allow_read_write();
      dmi_data.set_start_address(last_page->base);
      dmi_data.set_end_address(last_page->base + PAGESIZE/8LLU - 1);
      dmi_data.set_dmi_ptr(reinterpret_cast<unsigned char *>(last_page->data));

      dmi_data.set_read_latency( m_latency /8 ); // latency per byte is wanted.
      dmi_data.set_write_latency( m_latency /8 );
    }
  return true;
}



void generic_tlm_mem::stat_report(const char *msg, FILE *fd, bool resetf)
{
  std::ostringstream ops;

#ifdef TLM_POWER3
  ops << "Read_op=" << m_read_energy_op  << " (" << m_read_energy_op.round3sf() << "), Write_op=" << m_read_energy_op;
#else
  ops << "";
#endif
  if (fd) fprintf(fd, "%s:%s: %s stats: %s read_words=" PFI64 ", read_ops=" PFI64 ", write_ops=" PFI64 "\n", name(), kind(), msg, ops.str().c_str(), stats.read_words, stats.read_ops, stats.write_ops);
  if (resetf) stats.reset();
}


//eof

