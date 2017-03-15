// cbg_dram64 : A TLM RAM with with ELF and Intel Hex load facility.
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2009-15 D J Greaves
// University of Cambridge, Computer Laboratory.
// ACS P35 SoC D/M Classes - $Id: dram64_cbg.cpp,v 1.18 2011/07/25 15:34:01 my294 Exp $
//


#include "dram64_cbg.h"

void dram64_cbg::recompute_dram_pvt_parameters() // Called when Vcc is changed and so on.
{
  sc_time l_latency = sc_time(0.21 + 3.8e-4 *sqrt(float(m_bits)), SC_NS);
#ifdef TLM_POWER3


  //These are the SRAM figures  : CHANGEME
  pw_power l_leakage = pw_power(82.0 * m_bits, PW_nW);
  m_read_energy_op = pw_energy(5.0 + 1.2e-4 / 8.0 *m_bits, pw_energy_unit::PW_pJ);

  if (l_leakage > pw_power(50, PW_WATT))
    {
      std::cout << name() << "Infeasibly large DRAM: Ignore power consumption: " << m_bits/8 << " byte DRAM: leakage_power=" << l_leakage.round3sf() << "\n";
      m_read_energy_op = PW_ZERO_ENERGY;
      l_leakage = PW_ZERO_POWER;
    }
    else
    {
      std::cout << name() << " " << m_bits/8 << " byte DRAM: leakage_power=" << l_leakage.round3sf() << "\n";
      set_static_power(l_leakage);
    }


  m_write_energy_op = 2.0 * m_read_energy_op; // rule of thumb!
  m_llsc_excess_energy_op = m_write_energy_op;

  pw_voltage vcc = get_vcc();
  assert(vcc != PW_ZERO_VOLT);
  cout << name () << ":" << kind() << ": basic latency = " << l_latency << " before consider Vcc=" << vcc << "\n";
  l_latency = l_latency / vcc.to_volts();
#endif
  cout << name () << ":" << kind() << ": final latency = " << l_latency << "\n";
  set_latency(l_latency);
}



// Constructor
dram64_cbg::dram64_cbg(sc_core::sc_module_name name, u64_t mem_base, u64_t mem_size, u32_t width):
  generic_tlm_mem(name, mem_base, mem_size, width)
{
  // NOT CORRECT AREA FORMULA: AREA NOT SUPPORTED FOR DRAM YET.
#ifdef TLM_POWER3
  set_fixed_area(pw_area(13359.0 + 4.93/8 * m_bits, PW_squm)); // or from tech file
#endif
  recompute_dram_pvt_parameters(); // Do this after m_bits is set.
}



TENOS_KIND_DEFINITION(dram64_cbg)

//eof

