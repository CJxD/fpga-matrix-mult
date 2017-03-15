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
	addr 0xE0002008 -> write to Mat B
read mode
	addr 0xE0002010 -> read low 32 bits of res
	addr 0xE0002018 -> read high 32 bits of res
*/
#define MEMBASE 0xE0002000
#define MATA	0x00
#define MATB	0x08
#define RES_LO	0x10
#define RES_HI	0x18
  
struct matMul_2x2:
	public sc_module
#ifdef TLM_POWER3
  , public pw_module
#endif
{
	tlm_utils::simple_target_socket<matMul_2x2, BUS_WIDTH, base_types_t> port0;

	// Containers for inputs and response
	u8 matA[4] = {0, 0, 0, 0};
	u8 matB[4] = {0, 0, 0, 0};
	u16 res[4] = {0, 0, 0, 0};

	matMul_2x2(sc_module_name name) : sc_module(name), port0("port0")
	{
		port0.register_b_transport(this, &matMul_2x2::b_transact);
	}

	private:
	void b_transact(payload_t &trans, sc_time &delay) 
	{
		unsigned char *data = trans.get_data_ptr();
		u64 addr = trans.get_address();
		int offset = addr ^ MEMBASE;

		if(trans.is_write())
		{
			int i, j, k;
			*(u64*)res = 0;

			if (offset == MATA)
				*(u32*)matA = *(u32*) &data[0];
			else if (offset == MATB)
				*(u32*)matB = *(u32*) &data[0];

			for(i=0;i<2;i++)
				for(j=0;j<2;j++)
					for(k=0;k<2;k++)
						res[i*2+j] += matA[i*2+k] * matB[k*2+j];
		}
		else
		{
			if (offset == RES_LO)
				*(u32*)&data[0] = *(u32*) &res[0];
			else if(offset == RES_HI)
				*(u32*)&data[0] = *(u32*) &res[2];
		}

		trans.set_response_status(tlm::TLM_OK_RESPONSE);
	}
};


