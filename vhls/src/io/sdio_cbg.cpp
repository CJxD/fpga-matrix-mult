// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>

#include "sdio_cbg.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define MBR_DISK_SIGNATURE_OFFSET 440

#define SDIO_CBG_DEBUG(X)

using namespace std;
using namespace tlm;

sdio_cbg::sdio_cbg(sc_module_name name, u32_t base, card_type_t card)
  : sc_module(name),
    m_data_vdd(0),
    m_card(card), 
    m_op_state(invalid),
    IRQ("sdio_irq"),
    m_interrupt_flag(false),
    m_base(base),
    m_system_address(0x0),
    m_block_size_and_count(0x0),
    m_argument(0x0),
    m_transfer_mode_command(0x0),
    m_buffer_data_port(0x0),
    m_present_state(0x1f20000),
    m_control(0x0),
    m_control_reset(0x0),
    m_interrupt_status(0x0),
    m_interrupt_status_enable(0x0),
    m_interrupt_signal_enable(0x0),
    m_error_status(0x0),
    m_capabilities(0x69ec0080),
    m_maximum_current_capabilities(0x1),
    m_force_event(0x0),
    m_ADMA_error_status(0x0),
    m_ADMA_system_address(0x0),
    m_boot_timeout_control(0x0),
    m_debug_selection(0x0),
    m_interrupt_support(0x0),
    m_slot_host(0x89010000),
    // Sets voltage window to be from 2.7 to 3.6, bits [23:15]
    m_ocr(0x00FF8000),
    m_next_acmd(false),
    m_rca(0xe624),
    m_block_size(0),
    m_block_count(0),
    m_RX_FIFO(NULL),
    m_RX_FIFO_idx(0)
{
  for(int i = 0; i < 4; i++)
    m_response[i] = 0x0;

  for(int i = 0; i < 256; i++)
    m_cccr[i] = 0x0;

  // we are stating that our card simulator has
  // bits[3:0] = 0b0010 -> CCCR/FBR Version 1.20
  // bits[7:4] = 0b0011 -> SDIO Specification Version 2.00
  m_cccr[0] = 0x32;

  // we are supporting CMD 53 -> bit[1] = 0b1
  m_cccr[8] = 0x02;

  // for our card simulation the common CIS is going
  // to start at address 0x001000
  m_cccr[9] = 0x00;
  m_cccr[10] = 0x10;
  m_cccr[11] = 0x00;

  uint8_t cis[] = {
      0x20, 4, // CISTPL_MANFID
      0x71, 0x02, // SDIO Card manufacturer code
      0x0a, 0x01, // Manufacturer information (Part No, Rev)

      0x22, 4, // CISTPL_FUNCE
      0x00, // extended data, function 0
      0x00, 0x08, // max block data for Fn0: 2048 (value << 8)
      0x62, // max transfer rate per line
      
      0xff, 0xff // CISPTL_END
  };

  m_cis = (uint8_t*)malloc(sizeof(cis)*sizeof(uint8_t));
  memcpy(m_cis, &cis[0], sizeof(cis));

  // SCR register
  // [63:60] - SCR structure, should be 0
  // [59:56] - Spec version
  // [55:55] - Data status after erases 
  // [54:52] - SD Security support
  // [51:48] - DAT Bus width supported 
  // [47:0] - Reserved
  //m_scr = 0x0104000000000000;
  m_scr = 0x0000000000000401;

  // SD register
  // [311:0] - these bits are reserved for manufacturer we set them to zero
  for(int i = 0; i < 39; i++)
      m_sd[i] = 0x0;
  // [447:312] - these bits are reserved
  for(int i = 39; i < 56; i++)
      m_sd[i] = 0x0;
  // [479:448] - size of protected area
  m_sd[56] = 0x00;
  m_sd[57] = 0x00;
  m_sd[58] = 0x00;
  m_sd[59] = 0x08;
  for(int i = 56; i < 60; i++) 
      m_sd[i] = 0x0;
  // [495:480] - card type => 00xx is SD memory card
  for(int i = 60; i < 62; i++)
      m_sd[i] = 0x0;
  // [508:496] - reserved
  m_sd[62] = 0x0;
  // [509] - secured mode
  // [511:510] - data bus width 00 = 1, 10 = 4 bit width
  m_sd[63] = 0x00;

  // reset all stats to zero
  SDIOSTATS(m_stats.reset());
  // set whether we are collecting stats or not
  SDIOSTATS(m_stats.collecting = true);

  port0.register_b_transport(this, &sdio_cbg::b_access);

  SC_THREAD(run);
}

void sdio_cbg::load(const char* vdd) {
    int fd;
    struct stat sbuf;

    if((fd = open(vdd, O_RDWR)) == -1) {
        fprintf(stderr, "Cannot open virtual disk drive %s\n", vdd);
        return;
    }

    if(stat(vdd, &sbuf) == -1) {
        fprintf(stderr, "Cannot stat virtual disk drive %s\n", vdd);
        return;
    }

    m_data_vdd = (char*)mmap((caddr_t)0, sbuf.st_size, PROT_WRITE, MAP_SHARED, fd, 0);
    if(m_data_vdd == (caddr_t)(-1)) {
        fprintf(stderr, "Cannot mmap virtual disk drive %s\n", m_data_vdd);
        return;
    }

    printf("%s: Virtual disk drive identifier: 0x%08x\n", name(), *(unsigned int*)(m_data_vdd + MBR_DISK_SIGNATURE_OFFSET));
}

void sdio_cbg::run() {
    while(true) {
        wait(m_interrupt_change_event);
        IRQ = m_interrupt_flag;
    }
}

void sdio_cbg::b_access(int idx, PRAZOR_GP_T &trans, sc_time &delay) {
  tlm_command cmd = trans.get_command();
  u32_t adr = (u32_t)trans.get_address();
  u8_t* ptr = trans.get_data_ptr();

  switch(cmd) {
  case TLM_READ_COMMAND: {

    u32_t orig_adr = adr;
    bool unaligned = false;
    u8_t* lanes = trans.get_byte_enable_ptr();

    u32_t data[2];
    int length = 1;
    if(trans.get_data_length() == 8) 
      length = 2;

    for(int i = 0; i < length; i++, adr+=4) {
      u32_t offset = adr - m_base;

      switch(offset) {
      case 0x0: {
	SDIOSTATS(m_stats.ctrl_reads+=32);
	data[i] = m_system_address;
	break;
      }

      case 0x4: {
	SDIOSTATS(m_stats.ctrl_reads+=32);
	data[i] = m_block_size_and_count;
	break;
      }
	
      case 0x8: {
	SDIOSTATS(m_stats.ctrl_reads+=32);
	data[i] = m_argument;
	break;
      }
	
      case 0xC: {
	SDIOSTATS(m_stats.ctrl_reads+=32);
	data[i] = m_transfer_mode_command;
	break;
      }
	
      case 0x10:
      case 0x14:
      case 0x18:
      case 0x1C: {
	SDIOSTATS(m_stats.ctrl_reads+=32);
	int idx = floor((offset - 0x10)/4);
	data[i] = m_response[idx];
	break;
      }
	
      case 0x20: {
        assert(0);
	SDIOSTATS(m_stats.ctrl_reads+=32);
	data[i] = m_buffer_data_port;
	break;
      }
	
      case 0x24: {
	SDIOSTATS(m_stats.ctrl_reads+=25);
	data[i] = (u32_t)m_present_state;
	break;
      }
	
      case 0x28: {
	SDIOSTATS(m_stats.ctrl_reads+=32);
	data[i] = m_control;
	break;
      }
	
      case 0x2C: {
	SDIOSTATS(m_stats.ctrl_reads+=27);
	data[i] = (u32_t)m_control_reset;
	break;
      }
	
      case 0x30: {
	SDIOSTATS(m_stats.ctrl_reads+=30);
	data[i] = (u32_t)m_interrupt_status;
	break;
      }
	
      case 0x34: {
	SDIOSTATS(m_stats.ctrl_reads+=30);
	data[i] = (u32_t)m_interrupt_status_enable;
	break;
      }
	
      case 0x38: {
	SDIOSTATS(m_stats.ctrl_reads+=30);
	data[i] = (u32_t)m_interrupt_signal_enable;
	break;
      }
	
      case 0x3C: {
	SDIOSTATS(m_stats.ctrl_reads+=8);
	data[i] = (u32_t)m_error_status;
	break;
      }
	
      case 0x40: {
	SDIOSTATS(m_stats.ctrl_reads+=31);
	data[i] = (u32_t)m_capabilities;
	break;
      }
	
      case 0x48: {
	SDIOSTATS(m_stats.ctrl_reads+=24);
	data[i] = (u32_t)m_maximum_current_capabilities;
	break;
      }
	
      case 0x50: {
	SDIOSTATS(m_stats.ctrl_reads+=32);
	data[i] = (u32_t)m_force_event;
	break;
      }
	
      case 0x54: {
	SDIOSTATS(m_stats.ctrl_reads+=3);
	data[i] = (u32_t)m_ADMA_error_status;
	break;
      }

      case 0x58: {
	SDIOSTATS(m_stats.ctrl_reads+=32);
	data[i] = m_ADMA_system_address;
	break;
      }
	
      case 0x60: {
	SDIOSTATS(m_stats.ctrl_reads+=32);
	data[i] = m_boot_timeout_control;
	break;
      }
	
      case 0x68: {
	SDIOSTATS(m_stats.ctrl_reads+=1);
	data[i] = (u32_t)m_debug_selection;
	break;
      }
	
      case 0xF0: {
	SDIOSTATS(m_stats.ctrl_reads+=8);
	data[i] = (u32_t)m_interrupt_support;
	break;
      }
	
      case 0xFC: {
	SDIOSTATS(m_stats.ctrl_reads+=32);
	data[i] = m_slot_host;
	break;
      }
	
      default:
	data[i] = 0x0;
      }
    }


    if(length == 1) {
      u32_t data_to_copy = data[0];
      memcpy(ptr, (u8_t*)(&data_to_copy), 4);
      SDIO_CBG_DEBUG(printf("%s: read addr 0x%08x, data 0x%08x, length %d, lanes = ",
			    name(), orig_adr, *((u32_t*)ptr), trans.get_data_length()); \
                     if(lanes) {                                        \
                         for(int i = 0; i < trans.get_data_length(); i++) \
                             printf("%02x", lanes[i]);                  \
                     }                                                  \
                     else                                               \
                         printf("(null)");                              \
                     printf("\n"));

    }
    else {
      u64_t data_to_copy = (u64_t)(data[0]) | ((u64_t)(data[1]) << 32llu);
      memcpy(ptr, (u8_t*)(&data_to_copy), 8);
      SDIO_CBG_DEBUG(printf("%s: read addr 0x%08x, data 0x%lx, length %d\n",
			    name(), orig_adr, *((u64_t*)ptr), trans.get_data_length()));      
    }
    
    break;
  }
  case TLM_WRITE_COMMAND: {
    u32_t data = ((u64_t*)ptr)[0];
    u8_t* lanes = trans.get_byte_enable_ptr();
    assert(trans.get_data_length() == 8);

    if(lanes && 
       (lanes[0] == 0x0 && lanes[1] == 0x0 && lanes[2] == 0x0 && lanes[3] == 0x0) &&
       (lanes[4] == 0xFF || lanes[5] == 0xFF || lanes[6] == 0xFF || lanes[7] == 0xFF)) 
      adr += 4;
    
    u32_t offset = adr - m_base;

    SDIO_CBG_DEBUG(
        printf("%s: write addr 0x%08x, offset 0x%08x, data 0x%08x(0x%08x), length %d, lanes = ",
               name(), adr, offset, *((u32_t*)ptr), data, trans.get_data_length()); \
        for(int i = 0; i < trans.get_data_length(); i++)                \
            printf("%02x", lanes[i]);                                   \
        printf("\n"));


    switch(offset) {
    case 0x0: {
      SDIOSTATS(m_stats.ctrl_writes+=32;);
      m_system_address = data;
      break;
    }

    case 0x4: {
        for(int i = 0; i < 4; i++) {
            if(lanes[i+4] == 0x0)
                continue;

	    SDIOSTATS(m_stats.ctrl_writes+=8);
            m_block_size_and_count &= ~(lanes[i+4] << (i*8));
            m_block_size_and_count |= (data & (lanes[i+4] << (i*8)));
        }
      break;
    }

    case 0x8: {
        m_argument = data; 
	SDIOSTATS(m_stats.ctrl_writes+=32);
      break;
    }

    case 0xC: {
      m_transfer_mode_command = 
          (data & lanes[4]) |
          (data & (lanes[5] << 8)) |
          (data & (lanes[6] << 16)) |
          (data & (lanes[7] << 24));
      SDIOSTATS(
		u8_t lane1 = lanes[4] != 0 ? 8 : 0;
		u8_t lane2 = lanes[5] != 0 ? 8 : 0;
		u8_t lane3 = lanes[6] != 0 ? 8 : 0;
		u8_t lane4 = lanes[7] != 0 ? 8 : 0;
		m_stats.ctrl_writes = lane1 + lane2 + lane3 + lane4);
      if(lanes[7] == 0xFF) {
          // here we are writing to 0xF bit
          // in that case we need to set
          // Command_Inhibit_CMD
          m_present_state |= 0x1;
          
          // parse and execute the command
          bool success = parse_and_exec_cmd(m_transfer_mode_command, m_argument);
          if(success) {
              m_interrupt_flag = true;
              m_interrupt_change_event.notify();
          }
      }
      break;
    }

    case 0x10:
    case 0x14:
    case 0x18:
    case 0x1C: {
      int idx = floor((offset - 0x10)/4);
      SDIOSTATS(m_stats.ctrl_writes+=32);
      m_response[idx] = data;
      break;
    }

    case 0x20: {
        SDIOSTATS(m_stats.ctrl_writes+=32);
        data = m_buffer_data_port;
        break;
    }

    case 0x24: {
      SDIOSTATS(m_stats.ctrl_writes+=25);
      m_present_state = (u25_t)(data & 0x1FFFFFF);
      break;
    }

    case 0x28: {
        if(lanes[0] == 0xFF) {
            m_control &= ~(0xFF);
            m_control |= (data & 0xFF);
            // check to see if we are doing card detection
            if((data & (1 << 7)) == 0x1) {
                // we are going to say that card is inserted
                m_control |= (1 << 7);
            } else {
                // disable bit because detection is disabled
                m_control &= ~(1 << 7);
            }
	    SDIOSTATS(m_stats.ctrl_writes+=8);
        }

        if(lanes[1] == 0xFF) {
            // check to see if we are switching 
            // power on the bus, because if we are
            // we should update 
            bool bus_power_on = !!(((data & 0xFF00) >> 8) && 0x1);
            if(bus_power_on) {
                // set in 0x24 16th bit to 1 to indicate that card is insertd
                // set in 0x24 17th bit to 1 to indicate that card state is stable
                // set in 0x24 18th bit to 1 to indicate that card is present
                // set in 0x24 19th bit to 1 to indicate that card is write enable
                m_present_state |= (0xF << 16);                
            }

            m_control &= ~(0xFF00);
            m_control |= (data & 0xFF00);
	    SDIOSTATS(m_stats.ctrl_writes+=8);
        }

        if(lanes[2] == 0xFF || lanes[3] == 0xFF) {
            printf("%s: NOT IMPLEMENTED -> offset %08x, data %08x, lanes = ", name(), offset, data);
            for(int i = 0; i < 8; i++)
                printf("%02x", lanes[i]);
            printf("\n");
            assert(0);
        }
      break;
    }

    case 0x2C: {
        if(lanes[4] == 0xFF) {
            // clock operations
            if(data & 0x1) {
                // enabling clock, we should set 2nd bit
                // to indicate that clock is stable
                m_control_reset |= (1 << 0);
                m_control_reset |= (1 << 1);
            }
            if(data & 0x4) {
                m_control_reset |= (1 << 2);
            }

	    SDIOSTATS(m_stats.ctrl_writes+=8);
        }

        if(lanes[5] == 0xFF) {
            // ignore, these bits are reserverd
            // TODO: could check that value here is equal to 0?
	    SDIOSTATS(m_stats.ctrl_writes+=8);
        }

        if(lanes[6] == 0xFF) {
            // ignore, this writes to data timeout counter value
            // to determines the interval by whihch DATA line
            // timeouts are detected
            // TODO: consider doing it when we add timing
	    SDIOSTATS(m_stats.ctrl_writes+=8);
        }

        if(lanes[7] == 0xFF) {
            // we are writing to bytes from [26:24]
            assert(lanes[4] == 0x0 && lanes[5] == 0x0 && lanes[6] == 0x0);
            data = (data >> 24);
	    SDIOSTATS(m_stats.ctrl_writes+=3);
            switch(data) {
              case 1: {
                  // Software Reset For All
                  // we need to reset to 0x0 all registers that are not RO
                  m_system_address = 0x0;
                  m_block_size_and_count = 0x0;
                  m_argument = 0x0;
                  m_transfer_mode_command = 0x0;
                  m_buffer_data_port = 0x0;
                  m_control = 0x0;
                  m_control_reset = 0x0;
                  m_interrupt_status = 0x0;
                  m_interrupt_status_enable = 0x0; 
                  m_interrupt_signal_enable = 0x0;
                  m_force_event = 0x0;
                  m_ADMA_error_status = 0x0;
                  m_ADMA_system_address = 0x0;
                  m_boot_timeout_control = 0x0;
                  m_debug_selection = 0x0;
                  m_interrupt_support = 0x0;

		  SDIOSTATS(m_stats.ctrl_writes+=417);	       	    
                  // we do not write anything to this register
                  // as driver is able to access registers immediately
                  // TODO: what is time delay that we should add here?

                  break;
              }

              case 2: {
                  // Software reset for CMD line
                  m_present_state &= ~0x1; // CMD Inhibit
                  m_interrupt_status &= ~0x1; // Command Complete
		  SDIOSTATS(m_stats.ctrl_writes+=2);

                  break;
              }

              case 4: {
                  // Software reset for DAT line
                  m_buffer_data_port = 0x0;
                  m_present_state &= ~(1 << 11);
                  m_present_state &= ~(1 << 10);
                  m_present_state &= ~(1 << 9);
                  m_present_state &= ~(1 << 8);
                  m_present_state &= ~(1 << 2);
                  m_present_state &= ~(1 << 1);
                  m_control &= ~(1 << 17);
                  m_control &= ~(1 << 16);
                  m_interrupt_status &= ~(1 << 5);
                  m_interrupt_status &= ~(1 << 4);
                  m_interrupt_status &= ~(1 << 2);
                  m_interrupt_status &= ~(1 << 1);
		  
		  SDIOSTATS(m_stats.ctrl_writes+=44);

                  break;
              }
              default: {
                  printf("%s: NOT IMPLEMENTED -> offset %08x, data %08x\n", name(), offset, data);
                  assert(0);
              }
            }
        }

      break;
    }

    case 0x30: {
      bool command = false;
      if(data & 0x1) {
          // Received interrupt so clear up the interrupt flag
          SDIO_CBG_DEBUG(printf("%s: received interrupt for command complete\n", name()));
          command = true;
          data &= ~0x1;

          // And also clear command inhibit bit
          m_present_state &= ~0x1;
	  SDIOSTATS(m_stats.ctrl_writes+=2);

      }

      if(data & 0x2) {
          // Received interrupt for data transfer
          SDIO_CBG_DEBUG(printf("%s: received interrupt for transfer complete\n", name()));
          data &= ~0x2;
	  SDIOSTATS(m_stats.ctrl_writes+=1);

      }

      if(data & (1 << 16)) {
          // Received interrupt for command timeout
          SDIO_CBG_DEBUG(printf("%s: received interrupt for command timeout\n", name()));
          data &= ~(1 << 16);
	  SDIOSTATS(m_stats.ctrl_writes+=1);
      }

      if(data == 0x0) {
          m_interrupt_flag = false;
          m_interrupt_change_event.notify();          
      }

      m_interrupt_status = (u30_t)(data & 0x3FFFFFFF);
      SDIOSTATS(m_stats.ctrl_writes+=30);


      // check to see if we have some data 
      if(m_block_count > 0 && command) {
          do_adma2_fds();
          
          // we have finished writing directly to memory so
          // need to raise an interrupt saying that transfer is complete
          m_interrupt_status |= (1 << 1);
	  SDIOSTATS(m_stats.ctrl_writes+=1);
          m_interrupt_flag = true;
          m_interrupt_change_event.notify();
      }

      break;
    }

    case 0x34: {
      m_interrupt_status_enable = (u30_t)(data & 0x3FFFFFFF);
      SDIOSTATS(m_stats.ctrl_writes+=30);
      break;
    }

    case 0x38: {
      m_interrupt_signal_enable = (u30_t)(data & 0x3FFFFFFF);
      SDIOSTATS(m_stats.ctrl_writes+=30);
      break;
    }

    case 0x3C: {
      m_error_status = (u8_t)(data & 0xFF);
      SDIOSTATS(m_stats.ctrl_writes+=8);
      break;
    }

    case 0x40: {
      m_capabilities = (u31_t)(data & 0x7FFFFFFF);
      SDIOSTATS(m_stats.ctrl_writes+=31);
      break;
    }
      
    case 0x48: {
      m_maximum_current_capabilities = (u24_t)(data & 0xFFFFFF);
      break;
    }

    case 0x50: {
      m_force_event = data;
      SDIOSTATS(m_stats.ctrl_writes+=32);
      break;
    }
      
    case 0x54: {
      m_ADMA_error_status = (u3_t)(data & 0x3);
      SDIOSTATS(m_stats.ctrl_writes+=3);
      break;
    }

    case 0x58: {
      m_ADMA_system_address = data;
      SDIOSTATS(m_stats.ctrl_writes+=32);
      break;
    }

    case 0x60: {
      m_boot_timeout_control = data;
      SDIOSTATS(m_stats.ctrl_writes+=32);
      break;
    }

    case 0x68: {
      m_debug_selection = (u1_t)(data & 0x1);
      SDIOSTATS(m_stats.ctrl_writes+=1);
      break;
    }

    case 0xF0: {
      m_interrupt_support = (u8_t)(data & 0xFF);
      SDIOSTATS(m_stats.ctrl_writes+=8);
      break;
    }

    case 0xFC: {
      m_slot_host = data;
      SDIOSTATS(m_stats.ctrl_writes+=32);
      break;
    }

    default:
      printf("%s: WRITE ERROR! Unknown offset -> addr 0x%08x with offset %08x\n", name(), adr, offset);
      assert(0); // Unknown offset
    }        

    break;
  }
  default: 
    assert(0); // unknown command sent in transaction
  }

  trans.set_response_status(TLM_OK_RESPONSE);
}

bool sdio_cbg::parse_and_exec_cmd(u32_t cmd, u32_t arg) {
    // find out what is command id
    u6_t cmd_id = (cmd >> 24) & 0x3F;
    if(!m_next_acmd) {
        // CMD commands
        SDIOSTATS(m_stats.cmds_issued++);
        switch(cmd_id) {
          case 0: {
              // This command just simply moves to idle mode
              SDIO_CBG_DEBUG(printf("%s: CMD0: Going to idle mode\n", name()));
              // we are sending response that we are now in idle mode
              m_response[0] = 0x1;
              m_interrupt_status |= (1 << 0);
	      SDIOSTATS(m_stats.ctrl_writes+=33);
              break;
          }

          case 2: {
              // This is supposed to read Card IDentification register (CID)
              // TODO: for now this is just some random data found on the net
              // Data that is supposed to be here is:
              // Manufacturer ID - width 8 (binary)
              // OEM/Application ID - width 16 (ascii)
              // Product name - width 40 (ascii)
              // Product revision - width 8 (BCD)
              // Product serial number - width 32 (Binary)
              // Reserved - width 4
              // Manufacturing date - width 12 (BCD)
              // CRC7 checksum - width 7 (Binary)
              // not used, always '1' - width 1
              
              u8_t mid = 0x03;
              u16_t oid = 0x5344; // "SD"
              u40_t pnm = 0x534C333247; // "SL32G";
              u8_t prv = 0x30;
              u32_t psn = 0x12345678;
              u4_t reserved = 0x0;
              u12_t mdt = 0x014; // Apr 2001
              u7_t crc = 0x3F; 
              u1_t one = 0x1;

              m_response[0] = 0x56780014;
              m_response[1] = 0x47301234;
              m_response[2] = 0x534C3332;
              m_response[3] = 0x00035344;

              m_interrupt_status |= (1 << 0);

	      SDIOSTATS(m_stats.card_regs_reads+=16);
	      SDIOSTATS(m_stats.ctrl_writes+=129);
              break;
          }

          case 3: {
              m_response[0] = (m_rca << 16) | 0;
              m_interrupt_status |= (1 << 0);
              // we move to stand-by state
              m_op_state = stby;
	      SDIOSTATS(m_stats.card_regs_reads+=2);
	      SDIOSTATS(m_stats.ctrl_writes+=33);
              break;
          }
            
          case 5: {
              if(m_card == SD) {
                  // for the SD card we are not executing this command
                  // thus we are going to set interrupt status to error
                  m_interrupt_status |= (1 << 16); // command timeout error
              } else {
                  if(arg == 0x0) {
                      SDIO_CBG_DEBUG(printf("%s: CMD5: Returning value of OCR register %08x\n", name(), (m_ocr & 0xFFFFFF)));
		      SDIOSTATS(m_stats.card_regs_reads+=3);
                  } else {
                      SDIO_CBG_DEBUG(printf("%s: CMD5: Writing value to OCR register %08x\n", name(), (arg & 0xFFFFFF)));
                      m_ocr = arg & 0xFFFFFF;
		      SDIOSTATS(m_stats.card_regs_writes+=3);
                  }
                  // only first 24 bits are used from OCR
                  u32_t data = (m_ocr & 0xFFFFFF);
                  // stuff bits are zero
                  u32_t stuff = 0x0 << 24;
                  // memory present is true
                  u32_t memory_present = 1 << 27;
                  // how many I/O functions should example card support? bits [30:28]
                  u32_t io_funcs = 0;
                  // card is ready to operate after initialisation
                  u32_t c = 1 << 31;
                  
                  m_response[0] = c | io_funcs | memory_present | stuff | data;
                  m_interrupt_status |= (1 << 0);
		  SDIOSTATS(m_stats.ctrl_writes+=33);
              }
              break;
          }

          case 6: {
              SDIO_CBG_DEBUG(printf("%s: CMD6: Executing switching bus mode\n", name()));
              // parse argument
              u1_t mode = (m_argument >> 31) & 0x1; // 0: check function, 1 switch function
              // [30:24] reserved (all 0)
              // [23:20] reserved for function group 6 (all '0' or 0xF)
              // [19:16] reserved for function group 5 (all '0' or 0xF)
              // [15:12] reserved for function group 4 (all '0' or 0xF)
              // [11:8] reserved for function group 3 (all '0' or 0xF)
              // [7:4] function group 2 for command system
              // [3:0] function group 1 for access mode
              parse_block_size_and_count();
              // for this case here we should have block size set to
              // be 8 bytes and block count to be 1
              assert(m_block_count == 1);
              assert(m_block_size == 64);
              m_RX_FIFO = (u32_t*)malloc(sizeof(u32_t*)*(m_block_count*m_block_size/4));
              switch(mode) {
                case 0: {
                    for(int i = 0; i < 16; i++)
                        if(i == 3) 
                            m_RX_FIFO[i] = 0x00000200; // high speed card
                        else
                            m_RX_FIFO[i] = 0x0;
                    
                    break;
                }
                case 1: {
                    for(int i = 0; i < 16; i++) {
                        if(i == 4)
                            m_RX_FIFO[i] = 0x00000001;
                        else
                            m_RX_FIFO[i] = 0x0;
                    }
                    break;
                }
              }

              m_op_state = data;

              m_response[0] = 0x0;
              m_interrupt_status |= (1 << 0);              

	      SDIOSTATS(m_stats.card_regs_reads+=8);
	      SDIOSTATS(m_stats.ctrl_writes+=33);

              break;
          }

          case 7: {
              // selects card 
              SDIO_CBG_DEBUG(printf("%s: CMD7: selects card; moves from state %d to state %d\n", name(), m_op_state, tran));
              m_response[0] = 0x0;
              m_interrupt_status |= (1 << 0);
	      SDIOSTATS(m_stats.ctrl_writes+=33);
              // moving to a different state
              switch(m_op_state) {
                case stby:
                  m_op_state = tran;
                  break;
                case tran:
                case data:
                  m_op_state = stby;
                default:
                  break;
              }

              break;
          }
            
          case 8: {
              // This command checks voltage, the requested 
              // voltage is in bits [11:8] of the argument:
              // 0000b - not defined
              // 0001b - 2.7-3.6V
              // 0010b - reserved for low voltage range
              // 0100b - reserved
              // 1000b - reserved
              // others - not defined
              u4_t voltage = (arg & 0xF00) >> 8;
              
              // bits [7:0] is a check pattern
              u8_t check_pattern = arg & 0xFF;
              SDIO_CBG_DEBUG(printf("%s: CMD8: with voltage %08x and check pattern %08x\n", name(), voltage, check_pattern));
              
              // the correct return from this command is to
              // replicate what was passed
              m_response[0] = (u32_t)(arg & 0xFFF);
              m_interrupt_status |= (1 << 0);
	      SDIOSTATS(m_stats.ctrl_writes+=33);

              break;
              
          }

          case 9: {
              // CSD registers field:
              // CSD structure (CSD_STRUCTURE) - 2 - [127:126] - CSD Version 2.0
              // MMC version - 4 - [125:122]
              // Reserved - 2 - [121:120] -
              // Data read access-time - (TAAC) - 8 - [119:112] 1000.00 us
              // Data read access-time in CLK cycles (NSAC*100) - (NSAC) - 8 - [111:104] - 0 cycles
              // Max. Data transfer rate - (TRAN_SPEED)- 8 - [103:96] - 25 Mbit/s or 50 Mbit/s
              // Card command classes - CCC - 12 - [95:84] - class 0 2 4 5 7 8 10
              // Max. read data block length - (READ_BL_LEN) - 4 - [83:80] - 512 bytes
              // Partial blocks for read allowed - (READ_BL_PARTIAL) - 1 - [79:79] - 0
              // Write block misalignment - (WRITE_BLK_MISALIGN) - 1 - [78:78] - 0
              // Read block misalignment - (READ_BLK_MISALIGN) - 1 - [77:77] - 0
              // DSR implemented - DSR_IMP - 1 - [76:76] - 0
              // Reserved - 6 - [75:70] -
              // Device size - C_SIZE - 22 - [69:48] - 7579 15191 30531
              // Reserved - 1 - [47:47] -
              // Erase single block enable - (ERASE_BLK_EN) - 1 - [46:46] - 1
              // Erase sector size - (SECTOR_SIZE) - 7 - [45:39] - 128 blocks
              // Write protect group size - (WP_GRP_SIZE) - 7 - [38:32] - 1 sectors
              // Write protect group enable - (WP_GRP_ENABLE) - 1 - [31:31] - 0
              // Reserved - 2 - [30:29] -
              // Write speed factor - (R2W_FACTOR) - 3 - [28:26] - 4
              // Max. write data block length - (WRITE_BL_LEN) - 4 - [25:22] - 512 bytes
              // Partial blocks for write allowed - (WRITE_BL_PARTIAL) - 1 - [21:21] - 0
              // Reserved - 5 - [20:16] -
              // File format group - (FILE_FORMAT_GRP) - 1 - [15:15] - 0
              // Copy flag (OTP) - COPY - 1 - [14:14] - 0
              // Permanent write protection - PERM_WRITE_PROTECT - 1 - [13:13] - 0
              // Temporary write protection - TMP_WRITE_PROTECT - 1 - [12:12] - 0
              // File format - (FILE_FORMAT) - 2 - [11:10] - 0
              // Reserved - 2 - [9:8] -
              // CRC - CRC - 7 - [7:1] -
              // Not used, always ’1’
              
              m_response[0] = 0x00000000;
              m_response[1] = 0x00EE1600;
              m_response[2] = 0x5A5F5000;
              m_response[3] = 0x00480000;
                  
              
              m_interrupt_status |= (1 << 0);
	      SDIOSTATS(m_stats.card_regs_reads+=16);
	      SDIOSTATS(m_stats.ctrl_writes+=129);

              break;
          }

          case 12: {
              SDIO_CBG_DEBUG(printf("%s: CMD12: stoping transmission to/from card; current state %d\n", name(), m_op_state));
              switch(m_op_state) {
                case data:
                  m_op_state = tran;
                  break;
                case rcv:
                  // according to documentation we should move to prg
                  // state and then once operation is finished move to
                  // tran state but in our simulation when this command
                  // is issued we are already done with operation
                  m_op_state = tran;
                default:
                  break;
              }

              // if there is any data remove it
              if(m_block_count > 0) {
                  m_RX_FIFO_idx = 0;
                  m_block_count = 0;
                  m_block_size = 0;
                  free(m_RX_FIFO);
                  m_RX_FIFO = NULL;
              }

              m_response[0] = 0x0;
              m_interrupt_status |= (1 << 0);
              // we need also to send that transfer is complete
              m_interrupt_status |= (1 << 1);
	      SDIOSTATS(m_stats.ctrl_writes+=34);

              break;

          }

          case 13: {
              SDIO_CBG_DEBUG(printf("%s: CMD13 command issued\n", name()));
              
              // we are always ready for data 
              // bit 8 is set to be high
              m_response[0] = (1 << 8);
              m_interrupt_status |= (1 << 0);
	      SDIOSTATS(m_stats.ctrl_writes+=33);

              break;
          }

          case 18: {
              // This command is supposed to read from SD/SDIO card
              // The argument is block offset (one block is 512 bytes)
              parse_block_size_and_count();
              u32_t one_block_bytes = 512;
              m_block_offset = m_argument * one_block_bytes;              
              SDIO_CBG_DEBUG(printf("%s: CMD18: block count = %d, block size = %d, block offset = %d\n", name(), m_block_count, m_block_size, m_block_offset));
              // total number of bytes to transfer
              u32_t total_bytes = m_block_count * m_block_size;
              m_RX_FIFO = (u32_t*)malloc(sizeof(u32_t*)*(total_bytes/4));

              for(int i = 0; i < total_bytes/4; i++) {
                  m_RX_FIFO[i] = 0x0;
                  m_RX_FIFO[i] = 
                      (m_data_vdd[m_block_offset+i*4] & 0xFF) |
                      ((m_data_vdd[m_block_offset+(i*4+1)] & 0xFF) << 8) |
                      ((m_data_vdd[m_block_offset+(i*4+2)] & 0xFF) << 16) |
                      ((m_data_vdd[m_block_offset+(i*4+3)] & 0xFF) << 24);
                      
              }

              m_RX_FIFO_idx = 0;

              m_op_state = data;
              
              m_response[0] = 0x0;
              m_interrupt_status |= (1 << 0);

	      SDIOSTATS(m_stats.ctrl_writes+=33);
	      SDIOSTATS(m_stats.io_reads+=total_bytes);

              break;
              
          }

          case 25: {
              // This command is supposed to write to SD/SDIO card
              // The argument is block offset (one block is 512 bytes)
              parse_block_size_and_count();
              u32_t one_block_bytes = 512;
              m_block_offset = m_argument * one_block_bytes;
              SDIO_CBG_DEBUG(printf("%s: CMD25: block count = %d, block_size = %d, block offset = %d\n", name(), m_block_count, m_block_size, m_block_offset));

              m_op_state = rcv;
              m_response[0] = 0x0;
              m_interrupt_status |= (1 << 0);

	      SDIOSTATS(m_stats.ctrl_writes+=33);
	      SDIOSTATS(m_stats.io_writes+=m_block_count*m_block_size);

              
              break;
          }
            
          case 52: {
              // parse arguments
              // check to see if it is read or write
              bool write = !!(arg & 0x80000000);
              // check to see if we need to read after write
              bool raw  = !!(arg & 0x08000000);
              u8_t write_data = arg & 0xFF;
              u1_t stuff = (arg >> 8) & 0x1;
              u17_t register_address = (arg >> 9) & 0x1FFFF;
              u3_t func_no = (arg >> 28) & 0x7;
              
              if(!write) {              
                  if(func_no == 0x0) {
                      if(register_address >= 0 && register_address < 256) {
                          m_response[0] = (u32_t)m_cccr[register_address];
                      }
                      else if(register_address >= 0x1000 && register_address < 0x17FFF) {
                          u32_t offset = register_address - 0x1000;
                          m_response[0] = (u32_t)m_cis[offset];
                      }
                      else {
                          printf("%s: CMD52: Reading unknown offset %08x at function %d\n", name(), register_address, func_no);
                          assert(0);
                      }
                  } else {
                      printf("%s: CMD52: Reading func_no %d is NOT IMPLEMENTED\n", name(), func_no);
                      assert(0);
                  }
                  SDIO_CBG_DEBUG(printf("%s: CMD52: Read %08x from register %05x and function %d\n", name(), (m_response[0] & 0xFF), register_address, func_no));
	      SDIOSTATS(m_stats.ctrl_writes+=32);
	      SDIOSTATS(m_stats.card_regs_reads+=4);
              } else {                          
                  SDIO_CBG_DEBUG(printf("%s: CMD52: Write %08x to register %05x and function %d\n", name(), write_data, register_address, func_no));
                  if(func_no == 0x0) {
                      m_cccr[register_address] = write_data;
                      if(raw) {
                          m_response[0] = (u32_t)m_cccr[register_address];
                      }
                      else 
                          m_response[0] = (u32_t)write_data;
                  }
                  else {
                      printf("%s: CMD52: Writing func_no %d is NOT IMPLEMENTED\n", name(), func_no);
                      assert(0);
                  }
	      SDIOSTATS(m_stats.ctrl_writes+=32);
	      SDIOSTATS(m_stats.card_regs_writes+=4);
              }
              
              m_interrupt_status |= (1 << 0);
	      SDIOSTATS(m_stats.ctrl_writes+=1);
              break;
          }
          case 55: {
              // we are expecting ACMD command
              m_next_acmd = true;
              // we set the 5th bit to indicate that we are ready for
              // ACMD command
              m_response[0] = 1 << 5; 
              m_interrupt_status |= (1 << 0);
	      SDIOSTATS(m_stats.ctrl_writes+=33);
              break;
          }
            
          default: {
              printf("%s: Unknown CMD %d!\n", name(), cmd_id);
              assert(0);
          }
            
            
        }
    } else {
        // ACMD commands
        m_next_acmd = false;        
	SDIOSTATS(m_stats.acmds_issued++);
        switch(cmd_id) {
          case 6: {
              u4_t bus_width = (u4_t)(m_argument);
              if(bus_width == 4)
                  SDIO_CBG_DEBUG(printf("%s: ACMD6: wide bus selection\n", name()));
              else
                  SDIO_CBG_DEBUG(printf("%s: ACMD6: wide bus deselection\n", name()));
              
              m_response[0] = 0x0;
              m_interrupt_status |= (1 << 0);
	      SDIOSTATS(m_stats.ctrl_writes+=33);

              break;
          }

          case 13: {
              SDIO_CBG_DEBUG(printf("%s: ACMD13: gets SD status register\n", name()));
              parse_block_size_and_count();
              assert(m_block_count == 1);
              assert(m_block_size == 64);
              m_RX_FIFO = (u32_t*)malloc(sizeof(u32_t*)*(m_block_count*m_block_size/4));
              for(int i = 0; i < 16; i++) {
                  m_RX_FIFO[i] = 0;
                  for(int j = 0; j < 4; j++) {
                      m_RX_FIFO[i] |= (((u32_t)m_sd[i*4+j]) << (j*8));
                  }
              }

              m_op_state = data;
              m_response[0] = 0x0;
              m_interrupt_status |= (1 << 0);
	      SDIOSTATS(m_stats.card_regs_reads+=64);
	      SDIOSTATS(m_stats.ctrl_writes+=33);

              break;
          }
            
          case 41: {
              // check to see if voltage window field [23:0] is not equal to zero
              u24_t voltage_window_field = arg & 0xFFFFFF;
              if(arg == 0) {
                  voltage_window_field = m_ocr & 0xFFFFFF;
              }
              // TODO: implement support for switching to 1.8V request (not needed for now)
              bool s18r = !!(arg & (1 << 24));
              assert(!s18r);
              
              // checks for host capacity support
              // if this is on, we will return that we support SDHC
              bool hcs = !!(arg & (1 << 30));
              u32_t hcs_supported = hcs ? (1 << 30) : 0x0;
              
              m_response[0] = (1 << 31) | hcs_supported | voltage_window_field;
              m_interrupt_status |= (1 << 0);
	      SDIOSTATS(m_stats.card_regs_reads+=3);
	      SDIOSTATS(m_stats.ctrl_writes+=33);
              break;
              
          }
          case 51: {
              SDIO_CBG_DEBUG(printf("%s: ACMD51: gets SCR %llx, block_size_and_count %08x\n", name(), m_scr, m_block_size_and_count));

              parse_block_size_and_count();
              // for this case here we should have block size set to
              // be 8 bytes and block count to be 1
              assert(m_block_count == 1);
              m_RX_FIFO = (u32_t*)malloc(sizeof(u32_t*)*(m_block_count*m_block_size/4));

              m_RX_FIFO[0] = (u32_t)((m_scr << 32) >> 32);
              m_RX_FIFO[1] = (u32_t)(m_scr >> 32);

              m_op_state = data;

              m_response[0] = 0x0;
              m_interrupt_status |= (1 << 0);
	      SDIOSTATS(m_stats.card_regs_reads+=8);
	      SDIOSTATS(m_stats.ctrl_writes+=33);
              break;
              
          }
          default: {
              printf("%s: Unknown ACMD %d!\n", name(), cmd_id);
              assert(0);
          }

        }
    }

    return true;

}

void sdio_cbg::parse_block_size_and_count() {
    m_block_size = (u11_t)(m_block_size_and_count & 0x3FF);
    m_block_count = (u16_t)(m_block_size_and_count >> 16);
}


// In this state we fetch descriptor table
// by reading the first 64 bits at ADMA
// system address register
void sdio_cbg::do_adma2_fds() {    
    u32_t descriptor_adr = m_ADMA_system_address;

    PRAZOR_GP_T trans;
    u64_t descriptor_data = 0;
    trans.set_byte_enable_length(0);
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    trans.set_read();
    trans.set_address(descriptor_adr);
    trans.set_data_ptr((unsigned char*)(&descriptor_data));
    trans.set_data_length(8);
    
    sc_time delay = SC_ZERO_TIME;
    dma->b_transport(trans, delay);
    assert(!trans.is_response_error());

    // check attributes 
    bool valid = !!(descriptor_data & 1);
    bool end = !!((descriptor_data >> 1) & 1);
    bool interrupt = !!((descriptor_data >> 2) & 1);
    u2_t act = (descriptor_data >> 4) & 3;

    u16_t length = (descriptor_data >> 16) & 0xFFFF;
    u32_t addr = descriptor_data >> 32;

    SDIO_CBG_DEBUG(printf("%s: ADMA2 FDS addr=%08x len=%d act=%d, interrupt=%d, end=%d, valid=%d at address %08x (orig=%llx)\n", 
                          name(), addr, length, act, interrupt, end, valid, descriptor_adr, descriptor_data));


    // valid bit must be set to true
    // TODO: implement if valid is false to generate ADMA error interrupt
    assert(valid); 

    // move to CADR state
    SDIO_CBG_DEBUG(printf("%s: ADMA2 FDS state moves to CADR\n", name()));
    do_adma2_cadr(addr, length, act, end);
}

void sdio_cbg::do_adma2_cadr(u32_t addr, u16_t length, u2_t act, bool end) {
    bool transfer = false;

    // check to see what is action that needs to be performed
    switch(act) {
      case 0:
      case 1: {
          // do not execute current line and go to next line
          m_ADMA_system_address += 8; // 64 bit lines in ADMA2 table descriptor
          break;
      }
      case 2: {
          m_ADMA_system_address += 8;
          transfer = true;
          break;
      }
        
      case 3: {
          m_ADMA_system_address = addr;
          break;
      }
    }

    if(transfer && !end) {
        // move to TFR state to transfer data
        SDIO_CBG_DEBUG(printf("%s: ADMA2 CADR state moves to TFR state with address %08x, length %d and end %d\n", name(), addr, length, end));
        do_adma2_tfr(addr, length, end);
    }
    else if(end && !transfer) {
        SDIO_CBG_DEBUG(printf("%s: ADMA2 CADR state finished\n", name()));
        return; // finish, we are done
    }
    else {
        // any other cases
        assert(0);
    }
}

void sdio_cbg::do_adma2_tfr(u32_t addr, u16_t len, bool end) {

    u32_t length = (len == 0) ? 65536 : len;
    // Are there any cases when length is bigger then
    // what we actually have to write
    bool write = m_op_state == rcv;
    assert(length <= m_block_size * m_block_count);
    // length should always be multiple of 8?
    assert((length % 8) == 0);
    for(int i = 0; i < (length/8); i++) {
        PRAZOR_GP_T trans;
        trans.set_byte_enable_length(0);
        trans.set_byte_enable_ptr(0);
        trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
        trans.set_address(addr+i*8);
        if(!write) {
            trans.set_write();
            u64_t data = (((u64_t)(m_RX_FIFO[2*i+1+m_RX_FIFO_idx])) << 32) | (m_RX_FIFO[2*i+m_RX_FIFO_idx]);
            SDIO_CBG_DEBUG(printf("%s: ADMA2 TFR reading data %llx using ADMA2 and writing to address %08x\n", name(), data, addr+i*8));
            trans.set_data_ptr((unsigned char*)(&data));
            trans.set_data_length(8);
            
            sc_time delay = SC_ZERO_TIME;
            dma->b_transport(trans, delay);
            assert(!trans.is_response_error());
        }
        else {
            trans.set_read();
            u64_t data;
            trans.set_data_ptr((unsigned char*)(&data));
            trans.set_data_length(8);
            sc_time delay = SC_ZERO_TIME;
            dma->b_transport(trans, delay);
            assert(!trans.is_response_error());
            
            SDIO_CBG_DEBUG(printf("%s: ADMA2 TFR writing data %llx at block_offset %d using ADMA2 from address %08x\n", name(), data, m_block_offset, addr+i*8));
            for(int i = m_block_offset; i < m_block_offset+8; i++) {
                m_data_vdd[m_block_offset+i] = (data >> i*8) & 0xFF;
            }
            m_block_offset+=8;
            
        }
    }

    m_RX_FIFO_idx += length/8*2;
    if(length < m_block_size * m_block_count) {
        assert((length % m_block_size) == 0);
        m_block_count -= (length/m_block_size);
    } else {
        assert(length == m_block_size * m_block_count);
        m_RX_FIFO_idx = 0;
        m_block_count = 0;
        m_block_size = 0;
        free(m_RX_FIFO);
        m_RX_FIFO = NULL;
    }

    // check to see if we have stop at block gap request
    if(m_control & (1 << 16)) {
        // TODO: what do we do if block gap bit is set
        assert(0);
    }

    if(!end) {
        // go to FDS state
        SDIO_CBG_DEBUG(printf("%s: ADMA2 TFR moves to FDS\n", name()));
        do_adma2_fds();
    }   
}

void sdio_cbg::stat_report(const char *msg, FILE *fd, bool reset) {
    if(fd) {
      fprintf(fd, "\nSD/SDIO disk id: %s\n", name());
      fprintf(fd, "---------------\n");
      fprintf(fd, "Commands:\n");
      fprintf(fd, "--------\n");
      fprintf(fd, "   CMD      ACMD        Total\n");
      fprintf(fd, "-----------------------------\n");
      fprintf(fd, "%6d%10d%13d\n",
	      m_stats.cmds_issued,
	      m_stats.acmds_issued,
	      m_stats.cmds_issued + m_stats.acmds_issued);
      fprintf(fd, "SD/SDIO Controller registers access (MBits)\n");
      fprintf(fd, "-------------------------------------------\n");
      fprintf(fd, "   Reads        Writes       Total\n");
      fprintf(fd, "----------------------------------\n");
      fprintf(fd, "%8.3f%14.3f%12.3lf\n",
	      m_stats.ctrl_reads/1e6,
	      m_stats.ctrl_writes/1e6,
	      (m_stats.ctrl_reads + m_stats.ctrl_writes)/1e6);
      fprintf(fd, "SD/SDIO card registers and I/O (in Kbytes)\n");
      fprintf(fd, "----------------------------------------------\n");
      fprintf(fd, "        | Registers         Data         Total\n");
      fprintf(fd, "Reads   |%10.3f%13.3f%14.3f\n",
	      m_stats.card_regs_reads/1e3,
	      m_stats.io_reads/1e3,
	      (m_stats.card_regs_reads+m_stats.io_reads)/1e3);
      fprintf(fd, "Writes  |%10.3f%13.3f%14.3f\n",
	      m_stats.card_regs_writes/1e3,
	      m_stats.io_writes/1e3,
	      (m_stats.card_regs_writes+m_stats.io_writes)/1e3);
      fprintf(fd, "--------|-------------------------------------\n");
      fprintf(fd, "Totals  |%10.3f%13.3f%14.3f\n",
	      (m_stats.card_regs_reads+m_stats.card_regs_writes)/1e3,
	      (m_stats.io_reads+m_stats.io_writes)/1e3,
	      (m_stats.io_reads+m_stats.io_writes+m_stats.card_regs_writes+m_stats.io_writes)/1e3);
    }

    if(reset) {
      m_stats.reset();
    }
      
}

