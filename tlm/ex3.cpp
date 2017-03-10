#include <stdio.h>
#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_initiator_socket.h>
#include "matMul_2x2.h"


SC_MODULE(ex3)
{
  tlm_utils::simple_initiator_socket<ex3, BUSWIDTH, base_types_t> socket;
  
  SC_CTOR(ex3) : socket("socket")
  {
    SC_THREAD(run);
  }

  private:
  void run() 
  {
		tlm::tlm_generic_payload* p = new tlm::tlm_generic_payload();
		sc_time delay = sc_time(30, SC_NS);

		unsigned char data[16];
		p->set_data_length(16); 
		p->set_data_ptr(data);  

		p->set_streaming_width(8);
		p->set_byte_enable_ptr(0);
		p->set_dmi_allowed(false);
		p->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
		 
		u32* A = (u32*) &data[0];
		u32* B = (u32*) &data[4];
		u64* res = (u64*) &data[8];

		sc_core::sc_time d; 
		*A = 0x12345678; *B = 0x12345678;
		socket->b_transport(*p, d);  
		
		printf("matA = 0x%08x\n", *A);
		printf("matB = 0x%08x\n", *B);
		printf("res = 0x%016llx\nmatrix form:\n", *res);
		for(int i=0;i<2;i++)
		{
			for(int j=0;j<2;j++)
			{
				printf("%llu ", 0xFFFF&((*res)>>(i*2+j)*16));
			}
			printf("\n");
		}	
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

  
