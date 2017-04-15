#include <stdio.h>
#include <stdint.h>

#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>

#ifdef USE_PLUGINS
#define PLUGIN_NAME "matMul_chained"
#define PLUGIN_MODULE matMul_chained
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

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t
#define ptr_t uintptr_t

using namespace std;

#define MEM_BASE 0xE0002000
#define MAT	0x00
#define RES	0x04
	
struct matMul_chained:
	public sc_module
#ifdef TLM_POWER3
	, public pw_module
#endif
{
	tlm_utils::simple_target_socket<matMul_chained, BUS_WIDTH, base_types_t> port0;

	// Containers for inputs and response
	u8 res[4] = {0, 0, 0, 0};
	u32 mat = 0;
	
	u8 B[4] = {1,0,0,1};
	u8 C[4] = {1,1,1,1};
	u8 D[4] = {1,0,0,1};
	
	matMul_chained(sc_module_name name) : sc_module(name), port0("port0")
	{
		port0.register_b_transport(this, &matMul_chained::b_transact);
	}
	
	void matMul(u8* MA, u8* MB, u8* MO)
	{
		for (int i = 0; i < 2; i++)
			for (int j = 0; j < 2; j++)
			{
				u8 res = 0;
				for (u8 k = 0; k < 2; k++)
					res += MA[i*2 + k] * MB[k*2 + j];
				MO[i*2 + j] = res;
			}
	}

	void matAdd(u8* MA, u8* MB, u8* MO)
	{
		for (int i = 0; i < 2; i++)
			for (int j = 0; j < 2; j++)
				MO[i*2 + j] = MA[i*2 + j] + MB[i*2 + j];
	}


	void NN(u8* MA, u8* MB, u8* MO)
	{
		u8 tmp1[4];
		u8 tmp2[4];
		matMul(MA,MB,tmp1);
		matAdd(tmp1,C,tmp2);
	
		for(int i=0;i<2;i++)
		{
			matMul(tmp2,D,tmp1);
			matAdd(tmp1,C,tmp2);
		}	
	
		matMul(tmp2,D,tmp1);
		matAdd(tmp1,C,MO);
	}

	private:
	void b_transact(payload_t &trans, sc_time &delay) 
	{
		unsigned char *data = trans.get_data_ptr();
		u64 addr = trans.get_address();
		int flag = addr ^ MEM_BASE;
		if(trans.is_write())
		{
			int i, j, k;
			*(u32*)res = 0;
			if(flag == MAT) mat = *(u32*) &data[0];
			u8* A = (u8*)&mat;
			
			NN(A, B, res);
		}
		else
		{
			if(flag == RES) *(u32*)&data[0] = *(u32*) &res[0];
		}
		
		trans.set_response_status( tlm::TLM_OK_RESPONSE);
	}
};


