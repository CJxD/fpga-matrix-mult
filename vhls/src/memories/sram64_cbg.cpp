// cbg_sram64 : A TLM static ram with with ELF and Intel Hex load facility.
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2009-12 D J Greaves
// University of Cambridge, Computer Laboratory.
// ACS P35 SoC D/M Classes - $Id: sram64_cbg.cpp,v 1.18 2011/07/25 15:34:01 my294 Exp $
//


#include "sram64_cbg.h"


/*
   Cacti 45nanometer examples: http://www.hpl.hp.com/research/cacti/
   Read width 64 bits. Technology Size (nm):45 Vdd:1.0
   Number of banks:1 Read/Write Ports per bank:1
   Read Ports per bank:0 Write Ports per bank:0




Here's my four, rules of thumb single-ported SRAM scaling formulae -
created from simple linear regression applied to four runs from the
online CACTI version at HP labs.


Area = 13359.26+4.93/8*bits squm: gradient = 0.6 squm/bit.
Read energy = 5 + 1.2E-4 / 8 * bits pJ.
Leakage = 82nW per bit.
Random Latency = 0.21 + 3.8E-4(bits^0.5) nanoseconds * 1.0/supply voltage.

Another rule of thumb: 600 square lambda for an sram cell. 600.(45E-9)^2 = 1.2E-12 (1.2 squm/bit). [sqrt(600)=24]

//http://www.sooeet.com/math/linear-regression.php

Total RAM Size (bytes):1024 = 90x90
Access time (ns): 0.241078456616
Random cycle time (ns):0.149489052795
Multisubbank interleave cycle time (of data array) (ns):0.0937415165425
Total read dynamic energy per read port(nJ): 0.00261359459428
Total read dynamic power per read port at max freq (W): 0.0174835183274
Total standby leakage power per bank (W): 0.000736603299107
Refresh power (percentage of standby leakage power): 0.0
Total area (mm^2): 0.011414  - 1.3 squm per bit



Total RAM Size (bytes):4096 = 181x181
Access time (ns): 0.292769132576
Random cycle time (ns):0.164040033732
Multi-subbank interleave cycle time (of data array) (ns):0.11962694417
Total read dynamic energy per read port(nJ): 0.00385835474213
Total read dynamic power per read port at max freq (W): 0.0235208116846
Total standby leakage power per bank (W): 0.00267905713838
Total area (mm^2): 0.023157 - .7 squm per bit


Total RAM Size (bytes):65536 = 724x724
Access time (ns): 0.522592635435
Random cycle time (ns):0.235543638821
Multisubbank interleave cycle time (of data array) (ns):0.23041824284
Total read dynamic energy per read port(nJ): 0.0199826422296
Total read dynamic power per read port at max freq (W): 0.0848362635886
Total standby leakage power per bank (W): 0.0428741463937
Refresh power (percentage of standby leakage power): 0.0
Total area (mm^2): 0.359508  - .69 squm per bit



Total RAM Size (bytes):262144 =1448x1448
Access time (ns): 0.769208652429
Random cycle time (ns):0.320826207153
Multisubbank interleave cycle time (of data array) (ns):0.361111390109
Total read dynamic energy per read port(nJ): 0.0363319877424
Total read dynamic power per read port at max freq (W): 0.113245074537
Total standby leakage power per bank (W): 0.155484813656
Refresh power (percentage of standby leakage power): 0.0
Total area (mm^2): 1.300632  - .62 squm per bit

*/

void sram64_cbg::recompute_sram_pvt_parameters() // Called when Vcc is changed and so on.
{
  sc_time l_latency = sc_time(0.21 + 3.8e-4 *sqrt(float(m_bits)), SC_NS);
#ifdef TLM_POWER3
  pw_power l_leakage = pw_power(82.0 * m_bits, PW_nW);
  m_read_energy_op = pw_energy(5.0 + 1.2e-4 / 8.0 *m_bits, pw_energy_unit::PW_pJ);

  if (l_leakage > pw_power(50, PW_WATT))
    {
      std::cout << name() << "Infeasibly large SRAM: Ignore leakage power consumption: " << m_bits/8 << " byte SRAM: leakage_power=" << l_leakage.round3sf() << "\n";
      l_leakage = PW_ZERO_POWER;
    }
    else
    {
      std::cout << name() << " " << m_bits/8 << " byte SRAM: leakage_power=" << l_leakage.round3sf() << "\n";
      set_static_power(l_leakage);
    }


  m_write_energy_op = 2.0 * m_read_energy_op; // rule of thumb!
  m_llsc_excess_energy_op = m_write_energy_op;

  //std::ostringstream ops;
  //ops << "Read_op=" << m_read_energy_op  << " (" << m_read_energy_op.round3sf() << "), Write_op=" << m_read_energy_op;
  //std::cout << name () << ":" << kind() << " " << ops.str() << "\n";

  pw_voltage vcc = get_vcc();
  assert(vcc != PW_ZERO_VOLT);
  cout << name () << ":" << kind() << ": basic latency = " << l_latency << " before consider Vcc=" << vcc << "\n";
  l_latency = l_latency / vcc.to_volts();
#endif
  cout << name () << ":" << kind() << ": final latency = " << l_latency << "\n";
  set_latency(l_latency);
}



// Constructor
sram64_cbg::sram64_cbg(sc_core::sc_module_name name, u64_t mem_base, u64_t mem_size, u32_t width):
  generic_tlm_mem(name, mem_base, mem_size, width)
{
#ifdef TLM_POWER3
  set_fixed_area(pw_area(13359.0 + 4.93/8 * m_bits, PW_squm)); // or from tech file
#endif
  recompute_sram_pvt_parameters(); // Do this after m_bits is set.
}



TENOS_KIND_DEFINITION(sram64_cbg)

//eof

