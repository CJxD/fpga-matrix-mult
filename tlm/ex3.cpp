#include <stdio.h>
#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_initiator_socket.h>
#include "matMul_2x2.h"


SC_MODULE(ex3)
{
  tlm_utils::simple_initiator_socket<ex3, BUS_WIDTH, base_types_t> socket;
  
  SC_CTOR(ex3) : socket("socket")
  {
    SC_THREAD(run);
  }

  private:
  void run() 
  {
		payload_t* p = new payload_t();
		sc_time delay = sc_time(30, SC_NS);

		unsigned char data[4];
		p->set_data_length(4); 
		p->set_data_ptr(data);  

		p->set_streaming_width(8);
		p->set_byte_enable_ptr(0);
		p->set_dmi_allowed(false);
		p->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
		 
		u32* val = (u32*) &data[0];
		u32 hRes = 0, lRes = 0;
		u32 matA = 0x01020304;
		u32 matB = 0x01000001;
		u32 membase = 0xE0002000;
		
		sc_core::sc_time d; 
		
		*val = matA; 
		p->set_write();
		p->set_address(membase);
		socket->b_transport(*p, d);  
		
		
		*val = matB; 
		p->set_address(membase+8);
		socket->b_transport(*p, d); 
		

		p->set_read();
		p->set_address(membase+16);
		socket->b_transport(*p, d); 
		lRes = *val;
		
		
		p->set_read();
		p->set_address(membase+24);
		socket->b_transport(*p, d); 
		hRes = *val;
		
		printf("matrix A:\n%d %d\n%d %d\n", matA & 0xFF, matA>>8 & 0xFF, matA>>16 & 0xFF, matA>>24 & 0xFF); 
		printf("matrix B:\n%d %d\n%d %d\n", matB & 0xFF, matB>>8 & 0xFF, matB>>16 & 0xFF, matB>>24 & 0xFF); 
		
		printf("res = 0x%08x 0x%08x\n", hRes, lRes);
		printf("matrix form:\n%d %d\n%d %d\n", lRes & 0xFFFF, lRes>>16 & 0xFFFF, hRes & 0xFFFF, hRes>>16 & 0xFFFF); 
	}
	
}; 
int sc_main(int argc, char *argv[])
{
  ex3* socex = new ex3("socex");
  matMul_2x2 device("matMul_2x2");
  socex->socket.bind(device.port0);
  sc_start();
  return 0;
}

  
