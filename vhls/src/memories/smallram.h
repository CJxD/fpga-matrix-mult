// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2014 M Puzovic + D J Greaves

#ifndef __SMALLRAM_H__
#define __SMALLRAM_H__

#ifdef TLM_POWER3
#include "tlm_power.h"
using namespace sc_pwr;
#define POWER3(X) X
#else
typedef tlm::tlm_base_protocol_types PW_TLM_TYPES;
typedef tlm::tlm_generic_payload PW_TLM_PAYTYPE;
typedef int pw_customer_acct; /* use a simple int as a placeholder */
#define PW_TLM3(X)
#define POWER3(X)
#endif

#include <systemc.h>

class smallramp {
  public:
#ifdef TLM_POWER3
    virtual void log_energy_use(pw_energy p) = 0;
#endif
};

template<class T>
class smallram : public sc_module
#ifdef TLM_POWER3
  , public pw_module
#endif
{
  public:
    void recompute_pvt_parameters() {
#ifdef TLM_POWER3
        u64_t l_bits = m_words * m_width * 8;
        m_read_energy_op = pw_energy(5.0 + 1.2e-4 / 8.0 * l_bits, pw_energy_unit::PW_pJ);
        m_write_energy_op = 2.0 * m_read_energy_op; // rule of thumb!
        pw_voltage vcc = get_vcc();
        m_sr_latency = m_sr_latency / vcc.to_volts();
#endif
    }

    T operator[] (u64_t idx) { 
        return read(idx); 
    }

    T read(u64_t idx) {
        POWER3(m_parent->log_energy_use(m_read_energy_op));
        sysc_assert(idx >= 0 && idx < m_width);
        return m_data[idx*m_words];
    }

    T* readp(u64_t idx) {
        POWER3(m_parent->log_energy_use(m_read_energy_op));
        sysc_assert(idx >= 0 && idx < m_width);
        return &(m_data[idx*m_words]);
    }

    T* readp(u64_t idx, u64_t offset) {
        POWER3(m_parent->log_energy_use(m_read_energy_op));
        sysc_assert(idx >= 0 && idx < m_width);
        sysc_assert(offset >= 0 && offset < m_words);
      
        return &(m_data[idx*m_words + offset]);
    }

    T* readpo(u64_t offset) {
        POWER3(m_parent->log_energy_use(m_read_energy_op));
	sysc_assert((offset >= 0) && (offset < sizeof(T) * m_width * m_words));
	return (m_data + offset);
    }

    void writepo(u64_t offset, T* data, u32_t length) {
      sysc_assert((offset >=0) && ((offset + length) <= m_width*m_words*sizeof(T)));
      for(int i = 0; i < length; i++)
	*(m_data + offset + i) = *(data + i);
    }

    void write(u64_t idx, int blen, T* data) {
        sysc_assert(idx >= 0 && idx < m_width*m_words);
        for(int w = 0; w < blen; w++) {
            POWER3(m_parent->log_energy_use(m_write_energy_op));
            sysc_assert(idx + w >= 0 && idx + w < m_words*m_width);
	    m_data[idx] = data[w];
        }
    }
    void print_content() {
        for(int i = 0; i < m_width; i++) {
            fprintf(stdout, "%d: ");
            for(int j = 0; j < m_words; j++)
                fprintf(stdout, "%x", m_data[i*m_words+j]);
            fprintf(stdout, "\n");
        }
    }

    // Constructor
  smallram(sc_core::sc_module_name name, u64_t words, u64_t width, smallramp* p) :
    sc_module(name),
#ifdef TLM_POWER3
    pw_module(),
#endif
    m_words(words),
    m_width(width),
    m_parent(p) {

        u64_t bits = m_words * m_width * 8;
#ifdef TLM_POWER3
        pw_power leakage = pw_power(82.0 * bits, PW_nW);
        set_static_power(leakage);
        set_fixed_area(pw_area(13359.0 + 4.93/8 * bits, PW_squm));
#endif
        m_sr_latency = sc_time(0.21 + 3.8e-4 * sqrt(float(bits)), SC_NS);
        recompute_pvt_parameters();
        m_data = (T*)malloc(sizeof(T)*m_words*m_width);
    }

    sc_time m_sr_latency;

    // energy
    POWER3(pw_energy m_write_energy_op);
    POWER3(pw_energy m_read_energy_op);

  private:
    // number of words (word = sizeof(T)) in a line
    u64_t m_words; 
    // number of lines
    u64_t m_width;
    // parent that implementes logging of energy
    smallramp* m_parent;
    // latency
    // data
    T *m_data;
};

#endif
