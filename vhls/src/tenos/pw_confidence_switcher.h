/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

#ifndef PW_CONFIDENCE_SWITCHER_H
#define PW_CONFIDENCE_SWITCHER_H

#include <assert.h>
#include <string>
#include <iostream>
#include "pw_debug.h"


// This is NOT in the sc_pwr class since it is orthogonal.

/** @file pw_confidence_switcher.h
 * @brief Defines a generic API for measure/predict operation and implements the averaging metric.
 * @author David Greaves
 *
 */


template <class T> class confidence_switcher
{
  int n;       // Items logged
  int N;       // The major parameter
  int toskip;  // Leading occurrences to omit (as start up transient).
  int random;  // Occasional random retrials.
  T maximum;
  T minimum;
  T sum;
  T mean;
  T zero;
  bool mean_valid;

 public:
  std::string name1, name2;


  void report();


  // constructor
 confidence_switcher(T zero, std::string name1, std::string name2, int N) : 
  N(N), 
    zero(zero),
    name1(name1), name2(name2)
  {
    assert(N > 0);
    n = 0;
    toskip = N;
    random = 0;
    mean_valid = false;
    mean = zero;
    sum = zero;
  };
  
  bool measurement_needed() // Tell the user whether to make a measurement
  {
    if (n < N) return true;
    if (++random > 531) 
      {
	random = 0;
	return true;
      }
    return false;
  }

  T provide_estimate()
  {
    assert(n > 0);
    if (!mean_valid)
      {
	mean = sum/n; // We want to avoid this divide on a frequent basis.
	mean_valid = true;
      }
    return mean; 
  }

  // Record a new measurement
  T record_measurement(T v)
  {
    if (toskip > 0) // skip start ups
      {
	toskip -= 1;
      }
    else if (n>= N) // occasional monitoring
      { 
	if (v < minimum) 
	  {
	    pw_debug << name1 << ":" << name2 << " range=[" << minimum << ".." << maximum << "] mean=" << mean << " " << sum/n << " *** Below previous miniumum value=" << v << "\n";
	    minimum = v;
	  }
	if (v > maximum)
	  {
	    pw_debug << name1 << ":" << name2 << " range=[" << minimum << ".." << maximum << "] mean=" << mean << " " << sum/n << " *** Above previous maximum value=" << v << "\n";

	    maximum = v;
	  }
      }
    else // compute average
      {
	if (n==0 || v < minimum) minimum = v;
	if (n==0 || v > maximum) maximum = v;
	if (n < N)
	  {
	    n += 1;
	    sum += v;
	    if (v > zero && sum < zero) std::cout << "OVERFLOW IN SWITCHER " << name1 << name2 << "\n";
	  }
	bool verbose = false;
	if (verbose && n < N) pw_info << name1 << ":" << name2 << " range=[" << minimum << ".." << maximum << "] mean=" << sum/n << " v=" << v << "\n";
      }
    return v;
  }

};


#endif
// eof
