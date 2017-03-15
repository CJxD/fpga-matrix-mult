// TLM_POWER3: Energy-based for loosely-timed TLM.
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2011 DJ Greaves & MM Yasin, University of Cambridge Computer Laboratory.
// $Id: $
/*****************************************************************************
 *                       Copyright (c) 2010, CEA-LETI
 * 
 * TLM POWER2 is free software; you can redistribute it and/or modify it under 
 * the terms of the GNU Lesser General Public License as published by the Free 
 * Software Foundation; either version 2 of the License, or (at your option) 
 * any later version.
 *
 * TLM POWER2 has been developped in the framework of the MINALOGIC OpenTLM 
 * project.  For more information see http://www.opentlm.org
 *
 * For further information, questions or feedback on the delivery, please 
 * contact <pascal.vivet@cea.fr>
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public 
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License 
 * along with this library; if not, write to the Free Software Foundation, 
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 ****************************************************************************/

// $Id: pw_debug.h,v 1.1 2011/06/30 08:53:15 my294 Exp $

/** @file pw_debug.h
 * @brief Define a set of debugging macros and functions.
 * @author Cedric Koch-Hofer <cedric.koch-hofer@cea.fr>
 */

#ifndef _PW_DEBUG_
#define _PW_DEBUG_

#include <iostream>
#include <sstream>

#include "systemc"



// ===========================================================================
/**
 * @brief Logging macro for debug
 *
 * Macro which print the current filename, line, simulation time and a 
 * parameter message.
 */
#ifdef TLM_POWER_VERBOSE_DEBUG
#define pw_debug \
    ::std::cerr << "[LOG] " \
                << __FILE__ << "(" << __LINE__ << ", " \
                << sc_core::sc_time_stamp() << "): "
#else
#define pw_debug if(false) ::std::cerr
#endif


// ---------------------------------------------------------------------------
/**
 * @brief Logging macro for debug
 *
 * Macro which print a parameter message.
 */
#ifdef TLM_POWER_VERBOSE
#define pw_info \
    ::std::cout << "[INFO] "
#else
#define pw_info if(false) ::std::cout
#endif

// ---------------------------------------------------------------------------
/**
 * @brief Logging macro for errors
 *
 * Macro which prints error message
 */
#define pw_error					\
  ::std::cerr << "[ERROR] "				\
              << __FILE__ << "(" << __LINE__ << ", "	\
              << sc_core::sc_time_stamp() << "): "
  

// ---------------------------------------------------------------------------
//! Simple macros for namespace problems
#define pw_endl ::std::endl


// ---------------------------------------------------------------------------
// ! Wrapper of the sc_assert.
#define pw_assert(p_exp)                                                     \
{                                                                            \
    using sc_core::SC_ID_ASSERTION_FAILED_ ;                                 \
    sc_assert( (p_exp) ) ;                                                   \
}
                        

#endif // _PW_DEBUG_
