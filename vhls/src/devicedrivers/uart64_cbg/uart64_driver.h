/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

#ifndef UARTDRIVER_H
#define UARTDRIVER_H


// zync has uarts at E000_0000 and E000_1000
// We use the second UART in this driver.
#ifdef M32
#define UART_BASE 0xE0001000
#else
#define UART_BASE 0xFFFFffffE0001000LLU
#endif

// Programmer's model (Xilinx Zynq compatible).
// Register names
#define UART2_CR  0        // Control register
#define UART2_MR  4        // Mode register
#define UART2_IER 8        // Interrupt enable - write a one to enable an int condition in the mask.
#define UART2_IDR 0xC      // Interrupt disable  - write a one to disable. 
#define UART2_IMR 0x10     // Interrupt mask.
#define UART2_ISR 0x14     // (chnl_int_sts_reg0) Interrupt status unmasked (write to clear)  
#define UART2_BAUDRATE     0x18
#define UART2_RCVR_TIMEOUT 0x1c
#define UART2_RXWM    0x20 // RX fifo trigger level
#define UART2_MCR     0x24 // H/W flow control
#define UART2_MSE     0x28 // H/W flow control
#define UART2_SR      0x2c  // (chnl_sts_reg0) Main status register - readonly.
#define UART2_FIFO    0x30  // Data FIFO (send and receive)
#define UART2_BAUDRATE_DIVIDER    0x34  // 
#define UART2_TXWM    0x44  // TX fifo trigger level


// Interrupt bit lanes - now same as Zync
#define UART2_INT_TX_EMPTY        (0x08)
#define UART2_INT_RX_FULL         (0x04)
#define UART2_INT_RX_EMPTY        (0x02)
#define UART2_INT_RX_TRIGGER      (0x01)

// Status register bits
#define UART2_STATUS_TX_FULL  (0x10)  //
#define UART2_STATUS_TX_EMPTY (0x08)  //
#define UART2_STATUS_RX_EMPTY (0x02)  //



// Big-endian processor (or1k): need to add on seven to get ls-byte from word64.
// Little-endian (x86) - want boffset zero
#ifdef OR1K
#define BOFFSET 3
#else
#define BOFFSET 0
#endif

#define  UART2_SEND(X)         (((volatile char *)(UART_BASE+UART2_FIFO))[BOFFSET] = (X))
#define  UART2_RECEIVE()       (((volatile char *)(UART_BASE+UART2_FIFO))[BOFFSET])
#define  UART2_INT_ENABLE(X)   (((volatile char *)(UART_BASE+UART2_IER))[BOFFSET] = (X))
#define  UART2_INT_DISABLE(X)  (((volatile char *)(UART_BASE+UART2_IDR))[BOFFSET] = (X))
#define  UART2_STATUS()        (((volatile char *)(UART_BASE+UART2_SR))[BOFFSET])


char uart_polled_read(void);
void uart_polled_write(char d);
char uart_read_char(void);
void uart_write_char(char c);
void uart_reset_and_enable(void);

#endif
