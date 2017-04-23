// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>


//
// spEEDO is/was a project funded by the UK TSB to define a strawman debug and register API so that energy accounting information
// can be distributed throughout a SoC and its embedded operating system.
//

#include "speedo_api.h"

#ifdef WITH_SPEEDO

#define SPEEDO_TRC(X) 

#include "tenos.h"
#include "speedo_reg_mapping.h"

#include  "tlm_power.h"
#define POWER3(X) X
using namespace sc_pwr;


extern int backdoor_counter_ticks(lt_delay runahead);

const double e_unit_basis = 1.0e6; // Energy units - microJoules (uJ).
const u32_t UNIT_BASIS = SPEEDO_UNIT_uJ;


void speedo_ifc_t::snapshop_ctx(speedo_context_s &dest, lt_delay &runahead)
{
  const sc_pwr::pw_energy l_global_energy = pw_stat_observer_base::get_global_energy(); 
  SPEEDO_TRC(std::cout << "Global energy read  " << l_global_energy << "\n");
  float joules = l_global_energy.to_joules();
  dest.global_energy = (int)(joules * e_unit_basis); // Energy value returned as integer.
  dest.local_energy = parent.local_energy();
  dest.time = backdoor_counter_ticks(runahead);
}



int speedo_ifc_t::speedo_api(/*addr_t*/uint64_t addr, uint64_t &data, bool writef, lt_delay &runahead, int procID)
{
  SPEEDO_TRC(if (writef) printf("spEEDO API WRITE OP addr=" PFX64 " wdata=%lx\n", addr, data));
  int rc = SPEEDO_RC_NO_POWER_MONITOR;

#ifdef WITH_SPEEDO
  rc = SPEEDO_RC_INVALID_REG;
  int l_masked_addr = addr & 0xFF8;

  if (l_masked_addr < SPEEDO_REG_CTX0_BASE)
    {
      switch (l_masked_addr)
	{
	case SPEEDO_REG_MONICA:
	  data = ((uint64_t)'s' << 0) | ((uint64_t)'p' << 8) | ((uint64_t)'E' << 16) | ((uint64_t)'E' << 24) | ((uint64_t)'D' << 32) | ((uint64_t)'O' << 40);
	  rc = 0;
	  break;
    
	case SPEEDO_REG_ABI:
	  data = SPEEDO_API_ALPHA1;
	  rc = 0;
	  break;
	  
	case SPEEDO_REG_ENERGY_UNITS:
	  rc = 0;
	  data = UNIT_BASIS;
	  SPEEDO_TRC(std::cout << "spEEDO: live context: Energy units read  " << data << "\n");
	  break;
	  
	case SPEEDO_REG_TIME_UNITS:
	  data = 1000*1000*1000; // ticks clock frequency in femtoseconds - a one microsecond clock.  (1e-15 cf 1-e6) 
	  rc = 0;
	  break;
      
	case SPEEDO_REG_CMD_STATUS:
	  {
	    data = 0;
	  }
	  rc = 0;
	  
	case SPEEDO_REG_CORE_ID:
	  data = procID + (0xC0 << 24);
	  rc = 0;
	  break;
	  
	  
	case SPEEDO_REG_CTX_CTRL:

	  if (writef)
	    {
	      int dest_no;
	      if (data & (1U<<31))
		{
		  // If changing context, need to snapshot current real time values to current ctx
		  // and then change current ctx to user-specified value.
		  int d = (data >> 8) & 0xFF;
		  if (d >= SPEEDO_CONTEXTS) break;
		  dest_no = current_ctx;
		  current_ctx = d;
		  SPEEDO_TRC(std::cout << "spEEDO: Bit31 write cmd: dest_no=" << dest_no << " current_ctx=" << d << "\n");
		}
	      else
		{
		  // Default dest is no 1 and accumulate real-time on ctx 0.
		  dest_no = 1;
		  current_ctx = 0;
		  SPEEDO_TRC(std::cout << "spEEDO: simple checkpoint cmd: dest_no=" << dest_no << " current_ctx=" << current_ctx << "\n");
		}
	      
	      slaved_ctx_t &slaved_ctx = Slaved_ctxs[dest_no]; 
	      int  l_local_energy = parent.local_energy();
	      const sc_pwr::pw_energy l_global_energy = pw_stat_observer_base::get_global_energy(); 
	      SPEEDO_TRC(std::cout << "spEEDO: checkpoint: Global energy=" << l_global_energy << ";  Local energy=" << l_local_energy << "\n");
	      float l_g_joules = l_global_energy.to_joules();
	      SPEEDO_TRC(std::cout << "spEEDO: checkpoint: Global energy joules=" << l_g_joules << "\n");
	      slaved_ctx.global_energy = (int)(l_g_joules * e_unit_basis); // convert to the units used in this implementation.
	      slaved_ctx.local_energy = l_local_energy;
	      slaved_ctx.time = backdoor_counter_ticks(runahead);
	      parent.resume_accounting_alias(current_ctx); // Tell cpu_busaccess to use a new customer number from now on.
	    }
	  else data = SPEEDO_CONTEXTS | (current_ctx << 8);
	  rc = 0;
	  break;
	}
    }
  else
    {
      int ctx_no = (l_masked_addr - SPEEDO_REG_CTX0_BASE) / SPEEDO_REG_CTX_SPACING;
      int ctx_offset = l_masked_addr % SPEEDO_REG_CTX_SPACING;
      if (ctx_no == current_ctx) // reading live context
	{
	  switch(ctx_offset)
	    {
	      
	    case SPEEDO_CTX_REG_GLOBAL_ENERGY:
	      {
		const sc_pwr::pw_energy l_global_energy = pw_stat_observer_base::get_global_energy(); 
		SPEEDO_TRC(std::cout << "spEEDO: live context: Global energy read  " << l_global_energy << "\n");
		float joules = l_global_energy.to_joules();
		data = (int)(joules * e_unit_basis);
	      }
	      rc = 0;
	      break;
	      
	    case SPEEDO_CTX_REG_LOCAL_ENERGY:
	      {
		int new_e = parent.local_energy();
		SPEEDO_TRC(std::cout << "spEEDO: live context: Local energy read  " << new_e << "\n");
		data = new_e;
	      }
	      rc = 0;
	      break;
	      
	    case SPEEDO_CTX_REG_LOCAL_TIME:
	      {
		int new_time = backdoor_counter_ticks(runahead);
		data = new_time;
	      }
	      rc= 0;
	      break;
	    }
	}
      else if (ctx_no != current_ctx && ctx_no >= 0 && ctx_no < SPEEDO_CONTEXTS)
	{
	  slaved_ctx_t &slaved_ctx = Slaved_ctxs[ctx_no]; 
	  switch(ctx_offset)
	    {
	    case SPEEDO_CTX_REG_LOCAL_TIME:
	      data = slaved_ctx.time;
	      SPEEDO_TRC(std::cout << "spEEDO: slaved context "<< ctx_no << ": Local time read  " << data << "\n");
	      rc= 0;
	      break;
	      
	    case SPEEDO_CTX_REG_LOCAL_ENERGY:
	      data = slaved_ctx.local_energy;
	      SPEEDO_TRC(std::cout << "spEEDO: slaved context "<< ctx_no << ": Local energy read  " << data << "\n");
	      rc = 0;
	      break;
	      
	    case SPEEDO_CTX_REG_GLOBAL_ENERGY:
	      data = slaved_ctx.global_energy;
	      SPEEDO_TRC(std::cout << "spEEDO: slaved context "<< ctx_no << ": Global energy read  " << data << "\n");
	      rc = 0;
	      break;
	    }
	}
    }
#endif
  SPEEDO_TRC(if (!writef) printf("spEEDO API READ OP addr=" PFX64 " data=%lx\n", addr, data));
  return rc;
}

#else

// warning "+++ speedo_api is included in this build but TLM_POWER3 is disabled so there will be no energy results to read"

int speedo_ifc_t::speedo_api(/*addr_t*/uint64_t addr, uint64_t &data, bool writef, sc_time &delay, int procID)
{

  fprintf(stderr, "Cannot supply speedo API values without TLM_POWER3 and WITH_SPEEDO configured\n");
  return -101;
}


#endif
// eof


