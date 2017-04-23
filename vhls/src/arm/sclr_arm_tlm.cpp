// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>



#include "sclr_arm_tlm.h"
#include "tenos.h"

// System Controller?

#define SCLR_TRC(X)

using namespace tlm;

sclr_arm_tlm::sclr_arm_tlm(sc_module_name name) :
  sc_module(name)
#ifdef TLM_POWER3
  , pw_module(),
#if PW_TLM_PAYLOAD > 0
  read_bus_tracker(this)
#endif
#endif

{
  writeprotect = true;
  traceregions = 0;
  latency = sc_time(50, SC_NS);
  cpu_target0.register_b_transport(this, &sclr_arm_tlm::b_access);

  char lname[64];
  snprintf(lname, 64, "SLCR_Registers");
  m_registers = new smallram<u8_t>(lname,
				   4, // number of bytes in a single word
				   (int)((SLCR_LAST_REG_OFFSET+4)/4), // total number of words (registers)
				   this);
				   
  // set the reset values for each register
  // This information was obtained from Zynq-7000 AP SoC Technical Reference Manual
  // Appendix B.28 pp 1569-1574
  u32_t allzeros = 0x00000000;
  // SCL
  // SLCR_LOCK
  // SLCR_UNLOCK
  for(int i = 0; i < 3; i++) {
    m_registers->writepo(0x4*i, (u8_t*)(&allzeros), 4);
  }
  // SLCR_LOCKSTA
  u32_t one = 0x00000001;
  m_registers->writepo(0xC, (u8_t*)(&one), 4);

  u32_t pll_ctrl = 0x00028008;
  // *_PLL_CTRL
  for(int i = 0x100; i < 0x10C; i+=4)
    m_registers->writepo(i, (u8_t*)(&pll_ctrl), 4);

  // PLL_STATUS
  u32_t pll_status = 0x0000003F;
  m_registers->writepo(0x10C, (u8_t*)(&pll_status), 4);
  
  // *_PLL_CFG
  u32_t pll_cfg = 0x00177EA0;
  for(int i = 0x110; i <= 0x118; i+=4)
    m_registers->writepo(i, (u8_t*)(&pll_cfg), 4);

  // ARM_CLK_CTRL
  u32_t arm_clk_ctrl = 0x1F000200;
  m_registers->writepo(0x120, (u8_t*)(&arm_clk_ctrl), 4);

  // DDR_CLK_CTRL
  u32_t ddr_clk_ctrl = 0x18400003;
  m_registers->writepo(0x124, (u8_t*)(&ddr_clk_ctrl), 4);

  // DCI_CLK_CTRL
  u32_t dci_clk_ctrl = 0x01E03201;
  m_registers->writepo(0x128, (u8_t*)(&dci_clk_ctrl), 4);

  // APER_CLK_CTRL
  u32_t aper_clk_ctrl = 0x01FFCCCD;
  m_registers->writepo(0x12C, (u8_t*)(&aper_clk_ctrl), 4);

  // USB[0,1]_CLK_CTRL
  u32_t usb_clk_ctrl = 0x00101941;
  for(int i = 0x130; i <= 0x134; i+=4)
    m_registers->writepo(i, (u8_t*)(&usb_clk_ctrl), 4);

  // GEM[0,1]_RCLK_CTRL
  u32_t gem_rclk_ctrl = 0x00000001;
  for(int i = 0x138; i <= 0x13C; i+=4)
    m_registers->writepo(i, (u8_t*)(&gem_rclk_ctrl), 4);

  // GEM[0,1]_CLK_CTRL
  u32_t gem_clk_ctrl = 0x00003C01;
  for(int i = 0x140; i <= 0x144; i+=4)
    m_registers->writepo(i, (u8_t*)(&gem_clk_ctrl), 4);

  // SMC_CLK_CTRL
  u32_t smc_clk_ctrl = 0x00003C21;
  m_registers->writepo(0x148, (u8_t*)(&smc_clk_ctrl), 4);

  // LQSPI_CLK_CTRL
  u32_t lqspi_clk_ctrl = 0x00002821;
  m_registers->writepo(0x14C, (u8_t*)(&lqspi_clk_ctrl), 4);

  // SDIO_CLK_CTRL
  u32_t sdio_clk_ctrl = 0x00001E03;
  m_registers->writepo(0x150, (u8_t*)(&sdio_clk_ctrl), 4);

  // [UART|SPI]_CLK_CTRL
  u32_t uart_clk_ctrl = 0x00002821;
  for(int i = 0x154; i <= 0x158; i+=4)
    m_registers->writepo(i, (u8_t*)(&uart_clk_ctrl), 4);

  // CAN_CLK_CTRL
  u32_t can_clk_ctrl = 0x00501903;
  m_registers->writepo(0x15C, (u8_t*)(&can_clk_ctrl), 4);

  // CAN_MIOCLK_CTRL
  m_registers->writepo(0x160, (u8_t*)(&allzeros), 4);

  // DBG_CLK_CTRL
  u32_t dbg_clk_ctrl = 0x00000F03;
  m_registers->writepo(0x164, (u8_t*)(&dbg_clk_ctrl), 4);

  // PCAP_CLK_CTRL
  u32_t pcap_clk_ctrl = 0x00000F01;
  m_registers->writepo(0x168, (u8_t*)(&pcap_clk_ctrl), 4);
  
  // TOPSW_CLK_CTRL
  m_registers->writepo(0x16C, (u8_t*)(&allzeros), 4);

  // FPGA0_CLK_CTRL
  u32_t fpga_clk_ctrl = 0x00101800;
  m_registers->writepo(0x170, (u8_t*)(&fpga_clk_ctrl), 4);

  // FPGA0_THR_[CTRL|CNT]
  for(int i = 0x174; i <= 0x178; i+=4)
    m_registers->writepo(i, (u8_t*)(&allzeros), 4);

  // FPGA0_THR_STA
  u32_t fpga_thr_sta = 0x00010000;
  m_registers->writepo(0x17C, (u8_t*)(&fpga_thr_sta), 4);

  // FPGA1_CLK_CTRL
  m_registers->writepo(0x180, (u8_t*)(&fpga_clk_ctrl), 4);

  // FPGA1_THR_[CTRL|CNT]
  for(int i = 0x184; i <= 0x188; i+=4)
    m_registers->writepo(i, (u8_t*)(&allzeros), 4);

  // FPGA1_THR_STA
  m_registers->writepo(0x18C, (u8_t*)(&fpga_thr_sta), 4);

  // FPGA2_CLK_CTRL
  m_registers->writepo(0x190, (u8_t*)(&fpga_clk_ctrl), 4);

  // FPGA2_THR_[CTRL|CNT]
  for(int i = 0x194; i <= 0x198; i+=4)
    m_registers->writepo(i, (u8_t*)(&allzeros), 4);

  // FPGA2_THR_STA
  m_registers->writepo(0x19C, (u8_t*)(&fpga_thr_sta), 4);

  // FPGA3_CLK_CTRL
  m_registers->writepo(0x1A0, (u8_t*)(&fpga_clk_ctrl), 4);

  // FPGA3_THR_[CTRL|CNT]
  for(int i = 0x1A4; i <= 0x1A8; i+=4)
    m_registers->writepo(i, (u8_t*)(&allzeros), 4);

  // FPGA1_THR_STA
  m_registers->writepo(0x1AC, (u8_t*)(&fpga_thr_sta), 4);

  // CLK_621_TRUE
  m_registers->writepo(0x1C4, (u8_t*)(&one), 4);

  // Software reset control
  for(int i = 0x200; i <= 0x238; i+=4)
    m_registers->writepo(i, (u8_t*)(&allzeros), 4);

  // FPGA_RST_CTRL
  u32_t fpga_rst_ctrl = 0x01FF3F0F;
  m_registers->writepo(0x240, (u8_t*)(&fpga_rst_ctrl), 4);
  
  // A9_CPU_RST_CTRL
  m_registers->writepo(0x244, (u8_t*)(&allzeros), 4);

  // RS_AWDT_CTRL
  m_registers->writepo(0x24C, (u8_t*)(&allzeros), 4);

  // REBOOT_STATUS
  u32_t reboot_status = 0x00400000;
  m_registers->writepo(0x258, (u8_t*)(&reboot_status), 4);
  
  // BOOT_MODE
  m_registers->writepo(0x25C, (u8_t*)(&allzeros), 4);

  // APU_CTRL
  m_registers->writepo(0x300, (u8_t*)(&allzeros), 4);
  // WDT_CLK_SEL
  m_registers->writepo(0x304, (u8_t*)(&allzeros), 4);

  // TZ_DMA_NS
  m_registers->writepo(0x440, (u8_t*)(&allzeros), 4);
  // TZ_DMA_IRQ_NS
  m_registers->writepo(0x444, (u8_t*)(&allzeros), 4);
  // TZ_DMA_PERIPH_NS
  m_registers->writepo(0x448, (u8_t*)(&allzeros), 4);

  // PSS_IDCODE
  m_registers->writepo(0x530, (u8_t*)(&allzeros), 4);

  // DDR_*
  m_registers->writepo(0x600, (u8_t*)(&allzeros), 4);
  m_registers->writepo(0x60C, (u8_t*)(&allzeros), 4);
  for(int i = 0x614; i <= 0x620; i+=4) 
    m_registers->writepo(i, (u8_t*)(&allzeros), 4);

  // MIO_PIN_[00->01]
  u32_t mio_pin1 = 0x00001601;
  for(int i = 0x700; i <= 0x704; i++)
    m_registers->writepo(i, (u8_t*)(&mio_pin1), 4);
  u32_t mio_pin0 = 0x00000601;
  // MIO_PIN_[02->08]
  for(int i = 0x708; i <= 0x720; i++)
    m_registers->writepo(i, (u8_t*)(&mio_pin0), 4);
  // MIO_PIN_[09->53]
  for(int i = 0x724; i <= 0x7D4; i++)
    m_registers->writepo(i, (u8_t*)(&mio_pin1), 4);

  // MIO_LOOPBACK
  m_registers->writepo(0x804, (u8_t*)(&allzeros), 4);
  
  // MIO_MST_TRI0
  u32_t mio_mst_tri0 = 0xFFFFFFFF;
  m_registers->writepo(0x80C, (u8_t*)(&mio_mst_tri0), 4);

  // MIO_MST_TRI1
  u32_t mio_mst_tri1 = 0x003FFFFF;
  m_registers->writepo(0x810, (u8_t*)(&mio_mst_tri1), 4);

  // SD[0|1]_WP_CD_SEL
  for(int i = 0x830; i <= 0x834; i+=4)
    m_registers->writepo(i, (u8_t*)(&allzeros), 4);

  // LVL_SHFTR_EN
  m_registers->writepo(0x900, (u8_t*)(&allzeros), 4);
  
  // OCM_CFG
  m_registers->writepo(0x910, (u8_t*)(&allzeros), 4);

  // Reserved
  u32_t reserved = 0x00010101;
  m_registers->writepo(0xA1C, (u8_t*)(&reserved), 4);
  
  // GPIOB_*
  for(int i = 0xB00; i <= 0xB0C; i+=4)
    m_registers->writepo(i, (u8_t*)(&allzeros), 4);
  for(int i = 0xB14; i <= 0xB18; i+=4)
    m_registers->writepo(i, (u8_t*)(&allzeros), 4);
  for(int i = 0xB14; i <= 0xB18; i+=4)
    m_registers->writepo(i, (u8_t*)(&allzeros), 4);
  u32_t ddriob_addr = 0x00000800;
  for(int i = 0xB40; i <= 0xB58; i+= 4)
    m_registers->writepo(i, (u8_t*)(&ddriob_addr), 4);
  for(int i = 0xB5C; i <= 0xB6C; i+=4)
    m_registers->writepo(i, (u8_t*)(&allzeros), 4);
  u32_t ddriob_dci_ctrl = 0x00000020;
  m_registers->writepo(0xB70, (u8_t*)(&ddriob_dci_ctrl), 4);
  m_registers->writepo(0xB74, (u8_t*)(&allzeros), 4);
}

void sclr_arm_tlm::add_core(armcore_tlm* core) {
    cores.push_back(core);
}

// blocking transport method
void sclr_arm_tlm::b_access(int id, PRAZOR_GP_T &trans, sc_time &delay)
{
  tlm_command cmd = trans.get_command();

  POWER3(
    PW_TLM3(
      pw_agent_record l_agent = trans.pw_log_hop(
        this,  
	(cmd == TLM_READ_COMMAND ? PW_TGP_DATA: PW_TGP_NOFIELDS) | PW_TGP_ACCT_CKP,  
	&read_bus_tracker)));

  u32_t adr = ((u32_t)trans.get_address());
  u8_t*	ptr = trans.get_data_ptr();
  u32_t offset = adr - SLCR_BASE_ADDR;
      
  switch(cmd) {
    case TLM_READ_COMMAND: {
      memcpy(ptr, m_registers->readpo(offset), trans.get_data_length());

      SCLR_TRC(printf("%s:%s: read SCLR addr=%08x offset=%03X, data=%08X, data length=%d\n", 
		      name(), kind(), adr, offset, *((u32_t*)(ptr)), trans.get_data_length()));

      if (traceregions && traceregions->check(adr, TENOS_TRACE_IO_READ))
	printf("%s:%s: read SCLR addr=%08X offset=%03X, data=%08X, data length=%d\n", 
	       name(), kind(), adr, offset, *((u32_t*)(ptr)), trans.get_data_length());
      break;
    }

    case TLM_WRITE_COMMAND: {
      u32_t data = ((u64_t*)ptr)[0];
      u8_t* lanes = trans.get_byte_enable_ptr();
      assert(trans.get_data_length() == 8);
      if(lanes && lanes[4] == 0xFF) {
	// we need to increment address by 4
	// because data length is 8
	adr += 4;
	// since we have changed the address
	// we also need to recalculate the offset
	offset = adr - SLCR_BASE_ADDR;
      }

      u32_t old_data = data;
      // check for lock registers
      switch(offset) {
        case 0x4: {
	  if(data = 0x767B)
	    writeprotect = true;
	  data = 0;
	  break;
        }
        case 0x8: {
	  if(data == 0xDF0D)
	    writeprotect = false;
	  data = 0;
	  break;
        }
        case 0x100: 
        case 0x108: {
	  // ignored writes
	  data &= ~0xFFF80FE4;
	  break;
        }
        case 0x120: {
	  data &= ~0xE0FFC0CF;
	  break;
        }
        case 0x128: {
	  data &= ~0xFC0FC0FE;
	  break;
        }
        case 0x160: { // CAN_MIOCLK_CTRL
	  // writes are ignored for bits 23:31 and 15:7
	  // reads always return 0
	  data &= ~0xFF80FF80; 
	  break;
        }
        case 0x178:
        case 0x188:
        case 0x198:
        case 0x1A8: {
	  data &= ~0xFFFF0000;
	  break;
        }

        case 0x244: {
            u32_t current_data = *((u32_t*)(m_registers->readpo(0x244)));
            // writing to register A9_CPU_RST_CTRL
            // check to see if we are stopping CPU1
            if(((data >> 1) & 0x1) && (data & 0x20)) {
                // stop CPU1
                cores.at(1)->reset(false);
            }
            else if((!((current_data >> 1) & 0x1) )
                    && (current_data & 0x20)
                    && (!(data & 0x20))) {
	      cores.at(1)->reset(true);  // Zynq - reset second core - cannot be used for further cores.
            }
            break;
        }

        default:{
	  // write data as it is
	  break;
	}
      }

      if(offset != 0x4) {
	assert(!writeprotect);
	m_registers->writepo(offset, (u8_t*)(&data), 4);
      }

      SCLR_TRC(printf("%s:%s: write SCLR addr=%08x offset=%03X, data=%08X(%08X), data length=%d\n", 
		      name(), kind(), adr, offset, data, old_data, trans.get_data_length()));
      
      if (traceregions && traceregions->check(adr, TENOS_TRACE_IO_WRITE))
	printf("%s:%s: write SCLR addr=%08x offset=%03X, data=%08X(%08X), data length=%d\n", 
	       name(), kind(), adr, offset, data, old_data, trans.get_data_length());

      break;
    }
    default:
      assert(0); // should never hit this
  }
  
  AUGMENT_LT_DELAY(trans.ltd, delay, latency);
  
  trans.set_response_status(TLM_OK_RESPONSE);

#if PW_TLM_PAYLOAD > 0
  POWER3(l_agent.record_energy_use(pw_energy(1.0e-3 * 1.0e-6, PW_JOULE), &trans));
#else
  POWER3(record_energy_use(pw_energy(1.0e-3 * 1.0e-6, PW_JOULE), &trans));
#endif

  // Not all access cycles xfer a char however (especially when polled).
  // Very rough idea of SCLR energy per character 10 microseconds * 1 milliamp.
}

TENOS_KIND_DEFINITION(sclr_arm_tlm);

// eof


