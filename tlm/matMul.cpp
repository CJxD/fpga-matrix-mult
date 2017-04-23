#include <stdio.h>
#include <stdlib.h>

#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_initiator_socket.h>

#include "matMul_chained.h"

SC_MODULE(matMul)
{
	tlm_utils::simple_initiator_socket<matMul, BUS_WIDTH, base_types_t> socket;
	u32 mat;
	u32 iters;

	SC_CTOR(matMul) : socket("socket"), mat(0), iters(1)
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
		u32 res = 0;

		u32 i;
		sc_core::sc_time d;
		
		for (i=0; i<iters; i++)
		{
			*val = mat;
			p->set_write();
			p->set_address(MEM_BASE + MAT);
			socket->b_transport(*p, d);

			p->set_read();
			p->set_address(MEM_BASE + RES);
			socket->b_transport(*p, d); 
			res = *val;
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
	u32 mat;

	if (argc < 2 || argc > 4)
	{
		fprintf(stderr, "Usage: matMul <matrixA> [iterations]\n\n");
		return 1;
	}

	if (argc == 3)
	{
		iters = strtoul(argv[2], NULL, 10);
	}

	mat = strtoul(argv[1], NULL, 16);
	
	printf("input = 0x%08x\n", mat);
	printf("matrix form:\n%d %d\n%d %d\n", mat & 0xFF, mat>>8 & 0xFF, mat>>16 & 0xFF, mat>>24 & 0xFF); 

	printf("iterations: %u\n", iters);

	matMul testbed("testbed");
	testbed.mat = mat;
	testbed.iters = iters;

	matMul_chained device("matMul_chained");
	testbed.socket.bind(device.port0);

	sc_start();
	return 0;
}

  
