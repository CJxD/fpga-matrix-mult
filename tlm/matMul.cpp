#include <stdio.h>
#include <stdlib.h>

#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_initiator_socket.h>

#include "matMul_2x2.h"

SC_MODULE(matMul)
{
	tlm_utils::simple_initiator_socket<matMul, BUS_WIDTH, base_types_t> socket;
	u32 matA, matB;
	u32 iters;

	SC_CTOR(matMul) : socket("socket"), matA(0), matB(0), iters(1)
	{
		SC_THREAD(run);
	}

	private:
	void run() 
	{
		payload_t* p = new payload_t();

		unsigned char data[4];
		p->set_data_length(4); 
		p->set_data_ptr(data);

		p->set_streaming_width(8);
		p->set_byte_enable_ptr(0);
		p->set_dmi_allowed(false);
		p->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
		 
		u32* val = (u32*) &data[0];
		u32 hRes = 0, lRes = 0;

		u32 i;
		sc_core::sc_time d;

		for (i=0; i<iters; i++)
		{
			*val = matA;
			p->set_write();
			p->set_address(MEM_BASE + MATA);
			socket->b_transport(*p, d);

			*val = matB;
			p->set_address(MEM_BASE + MATB);
			socket->b_transport(*p, d); 

			p->set_read();
			p->set_address(MEM_BASE + RES_LO);
			socket->b_transport(*p, d); 
			lRes = *val;

			p->set_read();
			p->set_address(MEM_BASE + RES_HI);
			socket->b_transport(*p, d); 
			hRes = *val;
		}
	
		printf("res = 0x%08x\n", res);
		printf("matrix form:\n%d %d\n%d %d\n", res & 0xFF, res>>8 & 0xFF, res>>16 & 0xFF, res>>24 & 0xFF); 
		printf("time taken: %fs\n", d.to_seconds());
		
		#ifdef TLM_POWER3
		const sc_pwr::pw_energy l_global_energy = sc_pwr::pw_stat_observer_base::get_global_energy();
		printf("energy used: %fJ\n", l_global_energy.to_joules());
		#endif
	}
};

int sc_main(int argc, char *argv[])
{
	u32 i, iters = 1;
	u32 matA, matB;

	if (argc < 3 || argc > 5)
	{
		fprintf(stderr, "Usage: matMul <matrixA> <matrixB> [iterations]\n\n");
		return 1;
	}

	if (argc == 4)
	{
		iters = strtoul(argv[3], NULL, 10);
	}

	matA = strtoul(argv[1], NULL, 16);
	matB = strtoul(argv[2], NULL, 16);
	
	printf("input = 0x%08x 0x%08x\n", matA, matB);
	printf("matrix A:\n%d %d\n%d %d\n", matA & 0xFF, matA>>8 & 0xFF, matA>>16 & 0xFF, matA>>24 & 0xFF); 
	printf("matrix B:\n%d %d\n%d %d\n\n", matB & 0xFF, matB>>8 & 0xFF, matB>>16 & 0xFF, matB>>24 & 0xFF); 

	printf("iterations: %u\n", iters);

	matMul testbed("testbed");
	testbed.matA = matA;
	testbed.matB = matB;
	testbed.iters = iters;

	matMul_2x2 device("matMul_2x2");
	testbed.socket.bind(device.port0);

	sc_start();
	return 0;
}

  
