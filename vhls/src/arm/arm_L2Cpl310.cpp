// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>


#include "arm_L2Cpl310.h"

// ARM (Zynq) L2 Cache

using namespace std;
using namespace tlm;


#define ARM_L2_CONTROL_TRC(X) 

// Constructor
arm_L2Cpl310::arm_L2Cpl310(
    sc_core::sc_module_name name,
    u32_t id,
    u32_t addrsize, 
    u32_t size, 
    u32_t linesize, 
    u8_t ways, 
    bool snooped,
    type ty,
    u8_t level) :
    arm_ccache(
        name,
	id, 
        addrsize, 
        size, 
        linesize, 
        ways, 
        snooped, 
        ty,
        level),
    reg0_cache_id(0x410000C8),
    reg0_cache_type(0x9E300300),
    reg1_control(0x00000000),
    reg1_aux_control(0x02060000),
    reg1_tag_ram_control(0x00000777),
    reg1_data_ram_control(0x00000777),
    reg2_ev_counter_ctrl(0x00000000),
    reg2_ev_counter1_cfg(0x00000000),
    reg2_ev_counter0_cfg(0x00000000),
    reg2_ev_counter1(0x00000000),
    reg2_ev_counter0(0x00000000),
    reg2_int_mask(0x00000000),
    reg2_int_mask_status(0x00000000),
    reg2_int_raw_status(0x00000000),
    reg2_int_clear(0x00000000),
    reg7_cache_sync(0x00000000),
    reg7_inv_pa(0x00000000),
    reg7_inv_way(0x00000000),
    reg7_clean_pa(0x00000000),
    reg7_clean_index(0x00000000),
    reg7_clean_way(0x00000000),
    reg7_clean_inv_pa(0x00000000),
    reg7_clean_inv_index(0x00000000),
    reg7_clean_inv_way(0x00000000),
    reg9_lock_line_en(0x00000000),
    reg9_unlock_way(0x00000000),
    reg12_addr_filtering_start(0x40000001),
    reg12_addr_filtering_end(0xFFF00000),
    reg15_debug_ctrl(0x00000000),
    reg15_prefetch_ctrl(0x00000000),
    reg15_power_ctrl(0x00000000) {

    for(int i = 0; i < 8; i++) {
        reg9_d_lockdown[i] = 0x0;
        reg9_i_lockdown[i] = 0x0;
    }
    
    controller_target.register_b_transport(this, &arm_L2Cpl310::b_access);
}

void arm_L2Cpl310::b_access(int idx, PW_TLM_PAYTYPE &trans, sc_time &delay) {
    tlm_command cmd = trans.get_command();
    u32_t adr = (u32_t)trans.get_address();
    u8_t* ptr = trans.get_data_ptr();

    u32_t offset = adr - ARM_L2CPL310_BASE;

    switch(cmd) {
      case TLM_READ_COMMAND: {
          u32_t data = 0x0;
          switch(offset) {
            case 0x0: {
                data = reg0_cache_id;
                break;
            }
            case 0x4: {
                data = reg0_cache_type;
                break;
            }
            case 0x100: {
                data = reg1_control;
                break;
            }
            case 0x104: {
                data = reg1_aux_control;
                break;
            }
            case 0x108: {
                data = reg1_tag_ram_control;
                break;
            }
            case 0x10C: {
                data = reg1_data_ram_control;
                break;
            }
            case 0x200: {
                data = reg2_ev_counter_ctrl;
                break;
            }
            case 0x204: {
                data = reg2_ev_counter1_cfg;
                break;
            }
            case 0x208: {
                data = reg2_ev_counter0_cfg;
                break;
            }
            case 0x20C: {
                data = reg2_ev_counter1;
                break;
            }
            case 0x210: {
                data = reg2_ev_counter0;
                break;
            }

            case 0x214: {
                data = reg2_int_mask;
                break;
            }

            case 0x218: {
                data = reg2_int_mask_status;
                break;
            }

            case 0x21C: {
                data = reg2_int_raw_status;
                break;
            }
            case 0x220: {
                data = reg2_int_clear;
                break;
            }

            case 0x730: {
                data = reg7_cache_sync;
                break;
            }
              
            case 0x770: {
                data = reg7_inv_pa;
                break;
            }

            case 0x77C: {
                data = reg7_inv_way;
                break;
            }

            case 0x7B0: {
                data = reg7_clean_pa;
                break;
            }

            case 0x7B8: {
                data = reg7_clean_index;
                break;
            }

            case 0x7BC: {
                data = reg7_clean_way;
                break;
            }

            case 0x7F0: {
                data = reg7_clean_inv_pa;
                break;
            }
              
            case 0x7F8: {
                data = reg7_clean_inv_index;
                break;
            }

            case 0x7FC: {
                data = reg7_clean_inv_way;
                break;
            }

            case 0x900:
            case 0x904:
            case 0x908:
            case 0x90C:
            case 0x910:
            case 0x914:
            case 0x918:
            case 0x91C:
            case 0x920:
            case 0x924:
            case 0x928:
            case 0x92C:
            case 0x930:
            case 0x934:
            case 0x938:
            case 0x93C: {
                u32_t real_offset = offset - 0x900;
                u32_t index = (u32_t)floor(offset/8);
                u32_t remainder = offset % 8;
                if(remainder == 0) {
                    data = reg9_d_lockdown[index];
                } else if(remainder == 4) {
                    data = reg9_i_lockdown[index];
                } else {
                    // we should not reach this
                    assert(0);
                }
                break;
            }

            case 0x950: {
                data = reg9_lock_line_en;
                break;
            }
              
            case 0x954: {
                data = reg9_unlock_way;
                break;
            }

            case 0xC00: {
                data = reg12_addr_filtering_start;
                break;
            }

            case 0xC04: {
                data = reg12_addr_filtering_end;
                break;
            }

            case 0xF40: {
                data = reg15_debug_ctrl;
                break;
            }
            case 0xF60: {
                data = reg15_prefetch_ctrl;
                break;
            }
              
            case 0xF80: {
                data = reg15_power_ctrl;
                break;
            }

            default: {
                fprintf(stderr, "ARM L2Cpl310 is reading unknown address 0x%08x\n", adr);
                assert(0);
            }
          }
	  ARM_L2_CONTROL_TRC(printf("%s: Read control reg 0x%x with data 0x%x\n", name(), adr, data));
          memcpy(ptr, (u8_t*)(&data), 4);
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
          }

	  ARM_L2_CONTROL_TRC(printf("%s: Write control reg 0x%x with data 0x%x\n", name(), adr, data));
          
          switch(offset) {
            case 0x108: {
                // sets up latencies for tag RAM
                // TODO: this doesn't have any effect
                // for now on our cache
                reg1_tag_ram_control = data;
                break;
            }

            case 0x10C: {
                // sets up latencies for data RAM
                // TODO: this doesn't have any effect
                // for now on our cache
                reg1_data_ram_control = data;
                break;
            }

            default: {
            }
          }

          break;
      }

      default:
        assert(0); // should not reach here
    }

    trans.set_response_status(tlm::TLM_OK_RESPONSE);
}

// eof
