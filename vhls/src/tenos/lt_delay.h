// Copyright (C) 2017 XPARCH, Ltd. <info@xparch.com>

#ifndef LT_DELAY_H
#define LT_DELAY_H

#include "systemc.h"

extern sc_time global_qk_quantum;

//
// Loosely-timed delay pair - used instead of the SystemC TLM2.0 delay parameter in the TLM signatures.
//
class lt_delay
{
  //  const bool trace1 = true;

public:
  sc_time datum_tnow;
  sc_time delay;

  lt_delay() // Constructor
    {
      datum_tnow = SC_ZERO_TIME;
      delay = SC_ZERO_TIME;
    }

  sc_time point() const { return datum_tnow + delay; };

  void force_sync()
  {
    sc_time tnow = sc_time_stamp();
    if (point() < tnow)
      {
	// cannot resynch if we are behind tnow, which is possible in future multi-threaded version.
      }
    else
      {
	wait(delay);
	delay = SC_ZERO_TIME;
	datum_tnow = tnow;
      }
  }

  void perhaps_sync()
  {
    //if (trace1) cout << "Compare " << delay << " with " << global_qk_quantum << " at " << sc_time_stamp() << "\n";
    if (delay > global_qk_quantum) force_sync();
  }

  lt_delay &operator+=(const sc_time delta) 
  {
    delay += delta;
    return *this;
  }

  // We do not provide a fork operator since fork is just a literal copy.

  // This is stalling join operator, which is an
  // associative max operator, but this implementation
  // is not commutative since the l.h.s. operand is updated with result.
  void operator <<(const lt_delay &second)
  {
    sc_time tleft = point();
    sc_time tright = second.point();
    sc_time max = (tleft > tright) ? tleft:tright;
    delay = max - datum_tnow;
  }


  sc_time operator-(const lt_delay &second)
  {
    return (datum_tnow + delay) - (second.datum_tnow + second.delay);
  }
};


std::ostream & operator<<(std::ostream &Str, lt_delay &v);

#endif


