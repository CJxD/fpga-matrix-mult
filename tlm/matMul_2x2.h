#include <stdio.h>
#include <iostream>
#include <bitset>
#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>

#ifdef USE_PLUGINS
#define PLUGIN_NAME "matMul_2x2"
#define PLUGIN_MODULE matMul_2x2
#endif

#define BUS_WIDTH 64

#ifdef TLM_POWER3
#include <tlm_power>
typedef PW_TLM_TYPES base_types_t;
typedef PW_TLM_PAYTYPE payload_t;
#else
typedef tlm::tlm_base_protocol_types base_types_t;
typedef tlm::tlm_generic_payload payload_t;
#endif

#define u64 unsigned long long
#define u16 uint16_t
#define u8 uint8_t
#define u32 unsigned int

using namespace std;
/*
WIDTH of each matrix element = 8 bits
write mode
	addr 0xE0002000 -> write to Mat A
	addr 0xE0002008 -> wirte to Mat B
read mode
	addr 0xE0002010 -> read low 32 bits of res
	addr 0xE0002018 -> read high 32 bits of res
*/
#define ADDR1 0x0
#define ADDR2 ADDR1+8
#define ADDR3 ADDR2+8
#define ADDR4 ADDR3+8
  
struct matMul_2x2:
	public sc_module
#ifdef TLM_POWER3
  , public pw_module
#endif
{
public:

	tlm_utils::simple_target_socket<matMul_2x2, BUS_WIDTH, base_types_t> port0;	
  matMul_2x2(sc_module_name name): sc_module(name), port0("port0") 
  {
  	port0.register_b_transport(this, &matMul_2x2::b_transact);
  }
  
  u16 res[4] = {0, 0, 0, 0};
  u32 valA = 0;
  u32 valB = 0;
  
	void b_transact(payload_t &trans, sc_time &delay) 
  {
		unsigned char *data = trans.get_data_ptr();
		u64 addr = trans.get_address();
		int flag = addr & 0xFF;
		
		//printf("[dev] addr is %llx\n", addr);
		if(trans.is_write())
		{
			int i, j, k;
			*(u64*)res = 0;
			if(flag == ADDR1) valA = *(u32*) &data[0];
			else if (flag == ADDR2) valB = *(u32*) &data[0];
			
			u8* A = (u8*)&valA;
			u8* B = (u8*)&valB;
			for(i=0;i<2;i++)
		  	for(j=0;j<2;j++) 
		      for(k=0;k<2;k++) 
		      	 res[i*2+j] +=  A[i*2+k] * B[k*2+j];
		}
		else
		{
			if(flag == ADDR3) *(u32*)&data[0] = *(u32*) &res[0];
			else if(flag == ADDR4) *(u32*)&data[0] = *(u32*) &res[2];
		}
       
   	//printf("[dev] data is 0x%08x\n", *(u32*) data);
	  trans.set_response_status( tlm::TLM_OK_RESPONSE);
  }
  

};


