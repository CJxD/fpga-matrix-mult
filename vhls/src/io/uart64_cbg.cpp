// CBG TLM/ESL console UART model: in popup xterm or using stdin/out depending on the console mode
//
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2009-14 D J Greaves
// University of Cambridge, Computer Laboratory.
//
// ACS P35 SoC D/M Classes - Zynq Version $Id: $
//

#define UTRC(X) 

#include "uart64_cbg.h"




void *input_caller0(void *it)
{
  return ((uart64_cbg*)it)->input_process(0);
}

void *input_caller1(void *it)
{
  return ((uart64_cbg*)it)->input_process((void *)1);
}


void *uart64_cbg::input_process(void *arg)
{
  while (1) // Copy input characters from any source into the logged variable.
    {
      if (yielding || !read_next_char) 
	{
	  usleep(10000);
	  continue;
	}

      if (arg)
	{
	  unsigned char ch;
	  // printf("Hang in xterm read\n");
	  int c = read(xterm_fd, &ch, 1); 
          // printf("Read from xterm character %c\n", ch);
	  if (c && (control_register & (1 << 4)))
	    {
	      logged = ch | 256;
              read_next_char = false;
	      update_interrupt(); // Need to call after any write to the (back)logged variable.
	    }
	}
      else
	{
	  unsigned char c = getchar();
	  //	  printf("hang in getchar >%c<\n", c);
	  // Set bit 8 as a non zero value that is not part of the character.
	  logged = c | 256;
	  update_interrupt();  // Need to call after any write to the (back)logged variable.
	}
    }
}



// The input side is a bit promiscuous, reading from the xterm or stdin...
int uart64_cbg::testch()
{
  if (canned_input) return 1;
  if (xconsole && !xterm_opened) open_xterm();
  if (!threads_started)
    {
      threads_started = 1;
      if (use_stdin) pthread_create(&input_thread0, 0, input_caller0, (void *)this);
      if (xterm_fd >= 0) pthread_create(&input_thread1, 0, input_caller1, (void *)this);
      if (!use_stdin && xterm_fd < 0) printf("%s: Warning: No input devices\n", name());
    }
  if (!logged) 
    {
      idle_count += 1;
      if (idle_count == 1000)
	{
	  // Yield the processor if the input is idle, to save battery
	  // or be nice to other users.
	  yielding = 1;
	  usleep(100000);
	  yielding = 0;
	  idle_count = 0;
	}
    }
  return logged != 0;
}


// Note: Immediate string or file data for uart model input.
// Note: the uart model will handle input starting with a dot or a slash as a file to read from
void uart64_cbg::register_canned_input(const char *d)
{
  if (d && (d[0] == '/' ||d[0] == '.'))
    {
      FILE *fd = fopen(d, "r");
      if (!fd)
	{
	  perror("");
	  SC_REPORT_FATAL(name(), "Cannot open canned input file");
	}
      else canned_fd = fd;
    }
  canned_input = d;
}

char uart64_cbg::rdch_nonblock()
{
  char r = 0;
  if (canned_input)
    {
      if (canned_fd)
	{
	  r = getc(canned_fd); // input from file
	  if (feof(canned_fd))
	    {
	      fclose(canned_fd);
	      canned_fd = 0;
	      canned_input = 0;
	    }
	}
      else
	{
	  r = *canned_input; // immediate string input
	  canned_input ++;
	  if (!*canned_input) canned_input = 0;
	  if (r == '\\') // handle escapes
	    {
	      r = *canned_input; 
	      canned_input ++;
	      if (!*canned_input) canned_input = 0;
	      switch (r)
		{
		case 'q':
		  fprintf(stderr, "Backdoor quit sc_stop at %s\n", name());
		  sc_stop;
		  break;

		case 'n': r = '\n'; break;
		case 'r': r = '\r'; break;
		case 'b': r = '\b'; break;
		case 'f': r = '\f'; break;
		case '\\': r = '\\'; break;
		case 'v': r = '\v'; break;
		case 't': r = '\t'; break;
		default:  r &= 0x1F;  break;
		}
	    }
	}
    }
  else if (logged)
    {
      idle_count = 0;
      r = logged & 0xFF;
      logged = 0;
      update_interrupt(); // Need to call after any write to the (back)logged variable.
    }
  return r;
}

// Read input character - non-blocking
char uart64_cbg::rdch()
{
  while (1) if (testch()) return rdch_nonblock();
  return 0;
}

void uart64_cbg::end_of_simulation()
{
  flush();
  if (output_f)
    {
      fclose(output_f);
      output_f = 0;
    }
}


void uart64_cbg::flush()
{
  if (output_f) fflush(output_f); else fflush(stdout);
}


void uart64_cbg::open_xterm()
{
  xterm_opened = 1; // Set this first so that no second chance is attempted on fail.

  // If no display variable, then do not try - (but some UNIX envs do not use this var?)
  const char *DISPLAY = getenv("DISPLAY");
  if (!DISPLAY) return; 


  int fd = posix_openpt(/*"/dev/ptmx",*/ O_RDWR | O_NOCTTY);
  char ptsid[132], pts[132];
  if (fd < 0 || ptsname_r(fd, ptsid, 132))
    {
      perror("pt open failed");
    }
  UTRC(printf("pts %s fd=%i\n", ptsid, fd));
  sprintf(pts, "-Svt/%i", fd);
  unlockpt(fd);
  UTRC(printf("%s: Uart server is opening xterm\n", name()));
  char * args[5];
  char line0[132], line1[132], line2[132], line3[132];
  strcpy(line0, "/usr/bin/xterm");
  strcpy(line1, pts);
  strcpy(line2, "-title");
  strcpy(line3, name());
  args[0] = line0;
  args[1] = line1;
  args[2] = line2;
  args[3] = line3;
  args[4] = 0;
  for (int i=0; args[i]; i++) printf("%s ", args[i]);
  printf("\n");
  if (!fork())
    {
      execv(args[0], args);
    }
  else
    {
      usleep(50000);
      int fd1 = open(ptsid, O_RDWR /* O_NONBLOCK*/);
      if (fd1 < 0)
	{
	  printf("%s: Failed to open %s\n", name(), ptsid);
	  perror("cannot open uart terminal");
	  exit(1);
	}
      xterm_fd = fd1;
    }
  if (threads_started) printf("%s: error: threads started too soon\n", name());
}


// Write output character
void uart64_cbg::uart_wrch(char c)
{
  extern void tmp_trace_bd(const char *);
  if ((c & 0xFF) == 0377) tmp_trace_bd("uart 0370");

  if (xconsole)
    {
      int b = 0;
      if (!xterm_opened) open_xterm();
      if (xterm_fd >= 0 && transmit_char) { b = write(xterm_fd, &c, 1); }
    }
  if (output_f) putc(c, output_f);
  putchar(c);
  if (c == '\n')
    {
      flush();
      if (output_f) fflush(output_f);
    }
}


// Return 0 on ok.
int uart64_cbg::open_log_file(const char *fn)
{
  if (output_f) return -1;
  if (!fn) return -2;
  output_f = fopen(fn, "w");
  if (!output_f)
    {
      printf("%s: Could not open log file\n", name());
      perror("uart64_cbg:");
      return -1;
    }

  return 0;
}


// constructor
uart64_cbg::uart64_cbg(sc_module_name p_name, bool use_x11, bool u) : 
  sc_module(p_name), 
#ifdef TLM_POWER3
  pw_module(),
#if PW_TLM_PAYLOAD > 0
  read_bus_tracker(this),
#endif
#endif
  xconsole(use_x11), 
  use_stdin(u),
  IRQ("IRQ")
{
  interrupt_mask_register = 0;
  tx_interrupt_level = 3;
  rx_interrupt_level = 3;
  control_register = 0x28;
  read_next_char = true;
  raised_interrupt = false;
  transmit_char = true;
  canned_input = 0;
  canned_fd = 0;
  traceregions = 0;
  idle_count = 0;
  xterm_fd = -1;
  output_f = 0;
  xterm_opened = 0;
  logged = 0;
  yielding = 0;
  threads_started = 0;

  latency = sc_time(200, SC_NS); // Assume connected to a slow I/O external bus.

  port0.register_b_transport(this, &uart64_cbg::b_access);

  txready = true; // Always ready to send in this high-level model. 

#ifdef TLM_POWER3
  // based on: just made up!
  set_excess_area(pw_length(450, PW_um), pw_length(150,  PW_um));
#endif
  //printf("created uart %s %s\n", kind(), name());
}



void uart64_cbg::b_access(int id, PRAZOR_GP_T &trans, sc_time &delay_)
{
  tlm::tlm_command cmd = trans.get_command();

  POWER3(PW_TLM3(pw_agent_record l_agent = trans.pw_log_hop(this,  (cmd==tlm::TLM_READ_COMMAND ? PW_TGP_DATA: PW_TGP_NOFIELDS) | PW_TGP_ACCT_CKP,  &read_bus_tracker)));

  u64_t    adr = ((u64_t)trans.get_address() & 0xfc);
  u8_t*	   ptr = trans.get_data_ptr();
  // u32_t    len = trans.get_data_length();
  // u8_t*    lanes = trans.get_byte_enable_ptr();
  // u32_t    wid = trans.get_streaming_width();
  
    
  // Obliged to check address range and check for unsupported features,
  //   i.e. byte enables, streaming, and bursts
  // Can ignore DMI hint and extensions
  // Using the SystemC report handler is an acceptable way of signalling an error
  
  u8_t reg_offset = adr & 0xFC; // Extract low bits of address bus for internal register.
  // Obliged to implement read and write commands

  tlm::tlm_response_status rc = tlm::TLM_ADDRESS_ERROR_RESPONSE;
  if (cmd == tlm::TLM_READ_COMMAND)
    {
      stats.reads += 1;
      u8_t r = 0x0;
      //printf("Uart read reg 0x%x\n", reg_offset);
      switch (reg_offset)
	{
	case UART2_FIFO & 0xFC:
	  r = rdch_nonblock();
	  rc = tlm::TLM_OK_RESPONSE;
	  break;
	    
	case UART2_MR & 0xFC:
	  r = mode_register;
	  rc = tlm::TLM_OK_RESPONSE;
	  break;
	  
	case UART2_CR & 0xFC:
	  r = control_register;
	  rc = tlm::TLM_OK_RESPONSE;
	  break;

	case UART2_IMR & 0xFC:
	  r = interrupt_mask_register;
	  rc = tlm::TLM_OK_RESPONSE;
	  break;

	case UART2_ISR & 0xFC:
	  r = interrupt_mask_register & interrupts_unmasked();
	  rc = tlm::TLM_OK_RESPONSE;
	  break;
	    
	case UART2_SR & 0xFC:
	  r = status_register_read();
	  rc = tlm::TLM_OK_RESPONSE;
	  break;

	case UART2_RXWM & 0xFC:
	  r = tx_interrupt_level;
	  rc = tlm::TLM_OK_RESPONSE;
	  break;

	case UART2_TXWM & 0xFC:
	  r = tx_interrupt_level;
	  rc = tlm::TLM_OK_RESPONSE;
	  break;
	  
	}

      // Since zync has a register at 2c which is not longword alligned we get away with ignoring address bit 2 and replicating the answer over the words.
      ((u64_t *)ptr)[0] = r | (((u64_t)r)<<32); // IO device uses endian-ness of hosting workstation.
      UTRC(printf("%s:%s: read uart addr=" PFX64 " reg_offset=%02X data=%02X\n", name(), kind(),  adr, reg_offset, r));
    }
    else if (cmd == tlm::TLM_WRITE_COMMAND)
      {
	stats.writes += 1;
	rc = tlm::TLM_OK_RESPONSE;// Default to success - ignore writes to unimplemented registers.
	u8_t d = ((u64_t *)ptr)[0];
	u8_t* lanes = trans.get_byte_enable_ptr();
	bool hi_wordf = false;
	if (lanes && lanes[4]) hi_wordf = true;
	if (hi_wordf) reg_offset += 4;

	switch (reg_offset)
	  {
	  case UART2_FIFO & 0xFC:
	    UTRC(printf("%s:%s: transmit wrch trace %x '%c'\n", name(), kind(), d, isprint(d) ? d: '.'));
	    uart_wrch(d);
	    stats.outs += 1;
	    break;
	    
	  case UART2_CR & 0xFC:
            if(d & 0x1) {
                // software reset for Rx data path
                logged = 0;
                if(xterm_fd >= 0)
                    lseek(xterm_fd, 0, SEEK_END);
                d &= ~(0x1);
            }

            if(d & 0x2) {
                // software reset for Tx data path
                d &= ~(0x2);
            }

            if (d & 0x4) {
                // flip it
                read_next_char = !read_next_char;
            }

	    control_register = d;
	    UTRC(printf("%s:%s write control reg 0x%x\n", name(), kind(), control_register));
	    break;

	  case UART2_MR & 0xFC:
	    mode_register = d;
	    UTRC(printf("%s:%s write mode control reg 0x%x\n", name(), kind(), mode_register));
	    break;

	  case UART2_IER & 0xFC:
	    interrupt_mask_register |= d;
            if(d & 0x8)
                transmit_char = true;
	    UTRC(printf("%s:%s write IER : set 0x%x giving 0x%x\n", name(), kind(), d, interrupt_mask_register));
	    break;

	  case UART2_IDR & 0xFC:
            if(d & 0x8)
                transmit_char = false;
	    interrupt_mask_register &= ~d;
	    UTRC(printf("%s:%s write IDR : clear 0x%x giving 0x%x\n", name(), kind(), d, interrupt_mask_register));
	    break;

	  case UART2_SR & 0xFC:
	    rc = tlm::TLM_GENERIC_ERROR_RESPONSE; //Should not write the status register.
	    break;

	  case UART2_ISR & 0xFC: // write to clear here
	    clear_interrupt(d);
	    break;

  	  case UART2_RXWM & 0xFC:
            tx_interrupt_level = d;
            break;

	  case UART2_RCVR_TIMEOUT:
	  case UART2_BAUDRATE:
	  case UART2_BAUDRATE_DIVIDER:
	    // The serial side of the UART is not modelled.  It might be worth storing the
	    // writes to these serial control registers in case the host wants to read them back
	    // but currently we just drop the written data.
	    break;

	  default:
	    printf("%s:%s: Write undefined register 0x%x  (wdata=0x%x)\n", name(), kind(), reg_offset, d);
	  }
      }
  
  AUGMENT_LT_DELAY(trans.ltd, delay, latency);
  //cout << "     uart lt_delay " << trans.ltd << "\n";
 
  trans.set_response_status(rc);
#if PW_TLM_PAYLOAD > 0
  POWER3(l_agent.record_energy_use(pw_energy(1.0e-3 * 1.0e-6, PW_JOULE), &trans));
#else
  POWER3(record_energy_use(pw_energy(1.0e-3 * 1.0e-6, PW_JOULE), &trans));
#endif

  // Not all access cycles xfer a char however (especially when polled).
  // Very rough idea of uart energy per character 10 microseconds * 1 milliamp.
}


// Since the transmit never blocks in this model, the TX FIFO never goes full. TXstatus will be always TX_EMPTY
// and we never need to update the interrupt pin on a TX hardware event.
// RX behaviour is also fully enclosed in this function by making it call testch.
void uart64_cbg::update_interrupt()
{
  const int flip = UART2_INT_RX_EMPTY; // Want to interrupt when non-empty so flip polarity.

  u32_t unmasked = interrupts_unmasked();
  if(!raised_interrupt && ((interrupt_mask_register & (flip ^ unmasked)) != 0)) {
      raised_interrupt = true;
      IRQ = raised_interrupt;
  }
  else if(raised_interrupt && (unmasked & flip)) {
      raised_interrupt = false;
      IRQ = raised_interrupt;
  }

  UTRC(printf("%s:%s: Update interrupt %i\n", name(), kind(), raised_interrupt));
}


void uart64_cbg::clear_interrupt(u32_t wtc) // Write to clear - a one clears an event. 
{
  // nop
  // We do not currently implement any edge-triggered event flags so there is nothing to clear.
}


u32_t uart64_cbg::interrupts_unmasked()
{
  u32_t r = 0;
  bool exist = testch();
  r |= exist  ? UART2_INT_RX_FULL:0; // Trigger and mark full on any char (for now).
  r |= exist  ? UART2_INT_RX_TRIGGER:0;
  r |= !exist ? UART2_INT_RX_EMPTY:0;
  r |= (1)    ? UART2_INT_TX_EMPTY:0;  // always ready to send in this model. TX queue is non dropping and non blocking.
  return r;
}

u32_t uart64_cbg::status_register_read() // Return unmasked, raw status.
{
  u8_t r = 0;
  r |= !testch() ? UART2_STATUS_RX_EMPTY:0;
  r |= txready   ? UART2_STATUS_TX_EMPTY:0;  // always ready to send in this model. TX queue is non dropping and non blocking.
  return r;
}


void uart64_cbg::stat_report(const char *msg, FILE *fd, bool resetf)
{
  // no stats for now
  if (fd) fprintf(fd, "%s:%s: reads=%i, writes=%i, ins=%i, outs=%i\n", name(), kind(), stats.reads, stats.writes, stats.ins, stats.outs);
  if (resetf) stats.reset();
}

TENOS_KIND_DEFINITION(uart64_cbg)

// eof
