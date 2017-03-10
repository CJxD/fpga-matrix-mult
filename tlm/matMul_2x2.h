#include <stdio.h>
#include <iostream>
#include <bitset>
#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>

typedef tlm::tlm_base_protocol_types base_types_t;
 
#ifdef USE_PLUGINS
#define PLUGIN_NAME "matMul_2x2"
#define PLUGIN_MODULE matMul_2x2
#endif

#define BUSWIDTH 64

#define u64 unsigned long long
#define u16 uint16_t
#define u8 uint8_t
#define u32 unsigned int

using namespace std;

/*
WIDTH of each matrix element = 8 bits
payload structure (bits):
	Name		Start			End						Length			
	A		 		0 			 	4*WIDTH-1			4*WIDTH				4x 8-bit int				
	B		 		4*WIDTH 	8*WIDTH-1			4*WIDTH				4x 8-bit int
	Res			8*WIDTH		16*WIDTH-1		8*WIDTH				4x 16-bit int	
total length = 16*WIDTH (16 bytes)
*/
  
SC_MODULE (matMul_2x2)
{
public:

	tlm_utils::simple_target_socket<matMul_2x2, BUSWIDTH, base_types_t> port0;	
  matMul_2x2(sc_module_name name): sc_module(name), port0("port0") 
  {
  	port0.register_b_transport(this, &matMul_2x2::b_transact);
  }
  
	void b_transact(tlm::tlm_generic_payload &trans, sc_time &delay) 
  {
		unsigned char *data = trans.get_data_ptr();
		
		int i, j, k;
		u32 valA = *(u32*) &data[0];
		u32 valB = *(u32*) &data[4];
		
		u8* A = (u8*)&valA;
		u8* B = (u8*)&valB;

		u16 res[4] = {0, 0, 0, 0};

		for(i=0;i<2;i++)
    	for(j=0;j<2;j++) 
        for(k=0;k<2;k++) 
        	 res[i*2+j] +=  A[i*2+k] * B[k*2+j];
        	
		*(u64*)&data[8] = *(u64*) res;
	  trans.set_response_status( tlm::TLM_OK_RESPONSE);
  }
  

};


