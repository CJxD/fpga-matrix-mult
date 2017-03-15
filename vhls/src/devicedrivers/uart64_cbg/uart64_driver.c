// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// SoCDAM UART DEVICE DRIVER example code.   (C) 2009-14 DJ GREAVES.
// 
//
// UART device driver - for polled mode or with interrupts
// NB: interrupts have never been properly finished or tested!

#include "uart64_driver.h"

// Owing to the use of these global vars we can only run one uart at the moment with this driver.
int console_flags;



// Polled I/O routines:
static void local_wait()
{

  // On some architectures there is a special 'nop' used inside spin locks that 
  // pauses a number of clock cycles without consuming a lot of power.

  // _core_pause()

  // We don't have one here at the moment?
  // But our output 'uart' is a model of a fast write channel that does not really block, so we do not spin on output.
}


char uart_polled_read()
{
    while (UART2_STATUS() & UART2_STATUS_RX_EMPTY) local_wait();
    return UART2_RECEIVE();
}


void uart_polled_write(char d)
  {
     while ((UART2_STATUS() & 
	      UART2_STATUS_TX_FULL)) local_wait();
     UART2_SEND(d);
  }




// Owing to the use of these global vars we can only run one UART under interrupts at the moment.
// Interrupt-driven receive routine:
// Circular FIFO buffers.
char rx_buffer[256], tx_buffer[256];
int rx_inptr, rx_outptr, tx_inptr, tx_outptr;
int interrupts_enabled = 0;

void uart_reset_and_enable()
{
  rx_inptr = 0;
  rx_outptr = 0;
  tx_inptr = 0;
  tx_outptr = 0;
  interrupts_enabled = 1;
  UART2_INT_ENABLE(UART2_INT_RX_EMPTY); // Want to interrupt on rx being non-empty.
}


char uart_read_char()
{
  while (rx_inptr==rx_outptr) local_wait();
  char r = rx_buffer[rx_outptr];
  rx_outptr = (rx_outptr + 1) & 255;
  return r;
}

static int uart_rx_isr()  // interrupt service routine
{
  if (UART2_STATUS() & UART2_STATUS_RX_EMPTY) return 0;
  rx_buffer[rx_inptr] = UART2_RECEIVE();
  rx_inptr = (rx_inptr + 1) & 255;
  return 1;
}
// on return from the ISR, the processor context is restored, including
// any interrupt mask flag in the main processor control word. 



//
void uart_write_char(char c)
{
  while (((tx_inptr+1) & 255)==tx_outptr) local_wait();
  tx_buffer[tx_inptr] = c;
  tx_inptr = (tx_inptr + 1) & 255;

  UART2_INT_ENABLE(UART2_INT_TX_EMPTY);
}

static int uart_tx_isr()
{
  if (tx_inptr == tx_outptr) 
    {
      UART2_INT_DISABLE(UART2_INT_TX_EMPTY);
      return 0;
    }
  if ((UART2_STATUS() & UART2_STATUS_TX_FULL)) return 0;
  UART2_SEND(tx_buffer[tx_outptr]);
  tx_outptr = (tx_outptr + 1) & 255;
  return 1;
}


// Interrupt service routine:
// Uart ISR: this is called from a short assembler stub placed at the
// hardware interrupt vector location.  The assembler stub sets up the
// stack pointer and frame pointer and saves any registers that might 
// be in use in non-interrupt contexts.
void uart_isr_routine()
{
  while (1)
    {
      int work_done = 0;
      work_done |= uart_rx_isr();
      work_done |= uart_tx_isr();
      if (!work_done) break;
    }
}



int uart_start()
{
  console_flags = 0;
 
}

char craft_rdch() 
{ 
  return uart_polled_read();
}


static void dispatching_wrch(char ch)
{ 
  if (interrupts_enabled)
    {
      uart_write_char(ch);
    }
  else
    {
      uart_polled_write(ch);
    }
}


void craft_wrch(char ch)
{
  dispatching_wrch(ch);
  if (ch == '\n')
    {
      // Implement 'cooked' mode where newline is rendered as LF then CR.
      dispatching_wrch('\r');
    }
}



// END (C) 1995-2010 DJ Greaves
// eof

