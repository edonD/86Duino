/*  twi.c - TWI/I2C library for Wiring & Arduino  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.  This library is free software; you can redistribute it and/or  modify it under the terms of the GNU Lesser General Public  License as published by the Free Software Foundation; either  version 2.1 of the License, or (at your option) any later version.  This library is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  Lesser General Public License for more details.  You should have received a copy of the GNU Lesser General Public  License along with this library; if not, write to the Free Software  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA  Modified 2012 by Todd Krein (todd@krein.org) to implement repeated starts*/#include <stdio.h>#include "io.h"#include "irq.h"#include "i2c.h"#include "i2cex.h"#include "twi.h"static volatile uint8_t twi_state;static volatile uint8_t twi_slarw;static volatile uint8_t twi_sendStop;			// should the transaction end with a stopstatic volatile uint8_t twi_inRepStart;			// in the middle of a repeated startstatic void (*twi_onSlaveTransmit)(void);static void (*twi_onSlaveReceive)(uint8_t*, int);static uint8_t twi_masterBuffer[TWI_BUFFER_LENGTH];static volatile uint8_t twi_masterBufferIndex;static volatile uint8_t twi_masterBufferLength;static uint8_t twi_txBuffer[TWI_BUFFER_LENGTH];static volatile uint8_t twi_txBufferIndex;static volatile uint8_t twi_txBufferLength;static uint8_t twi_rxBuffer[TWI_BUFFER_LENGTH];static volatile uint8_t twi_rxBufferIndex;static volatile uint8_t twi_error;//#define DEBUG_MODE // enable "printf" in this .cpp#define I2C_IOADDR       (0xFB00)#define I2C_GPIO_DATA    (0xF204)#define I2C_GPIO_DIR     (0xF206)#define CRSB_BIT_G0      (0x90)#define CRSB_BIT_G1      (0x98)#define CRSB_BIT_G2      (0xA0)#define CRSB_BIT_G3      (0xA8)#define I2C_PIN          (0x03) // GP10, GP11#define I2C0_IRQ         (0x0A)// Internal state machine#define TWI_READY    (0)#define TWI_MT_RX    (1)#define TWI_MT_TX    (2)#define TWI_SL_RX    (3)#define TWI_SL_TX    (4)#define DUMMY_VAL    (0x5A)static unsigned char int_routing_table[16] = {0xff, 0x08, 0xff, 0x02, 0x04, 0x05, 0x07, 0x06, 0xff, 0x01, 0x03, 0x09, 0x0b, 0xff, 0x0d, 0x0f};#if defined (DEBUG_MODE)static volatile int interrupt_times = 0;static volatile int temp_state = 0;#endif/*  * Function twi_init * Desc     readys twi pins and sets twi bitrate * Input    none * Output   none */// void twi_init(void) <= Arduino origin function typevoid _twi_init(uint32_t twi_speed){	int usedirq = 0, irqnum, i;	#if defined (DEBUG_MODE)	printf("I2C init start\n");	#endif	// initialize state	twi_state = TWI_READY;	twi_sendStop = true;		// default value	twi_inRepStart = false;		// set I2C base address	i2c_SetBaseAddress(I2C_IOADDR);		// set I2C IRQ	if((sb_Read8(0xD6) & 0x0F) == 0 || (sb_Read8(0xD6) & 0x0F) == 0x0F)	{		usedirq = I2C0_IRQ;		i2c_SetIRQ(int_routing_table[I2C0_IRQ], I2CIRQ_DISABLE);	}	else	{		irqnum = sb_Read8(0xD6) & 0x0F;		for(i=0; i<16; i++)			if(int_routing_table[i] == irqnum) break;		if(i != 16)			usedirq = i;		else		{			usedirq = I2C0_IRQ;			i2c_SetIRQ(int_routing_table[I2C0_IRQ], I2CIRQ_DISABLE);		}	}		// switch GPIO/I2C pins into GPIO pins	io_outpb(I2C_GPIO_DIR, io_inpb(I2C_GPIO_DIR) | I2C_PIN);	io_outpb(I2C_GPIO_DATA, io_inpb(I2C_GPIO_DATA) | I2C_PIN);	io_outpb(CRSB_BIT_G1 + 0, 0x01); // SCL(GP10)	io_outpb(CRSB_BIT_G1 + 1, 0x01); // SDA(GP11)					                                                                 	if (i2c_Reset(0) == false)  // assume the status of GPIO/I2C pins are GPIO "IN" or "OUT 1"	{	    //i2c_Close();	    #if defined (DEBUG_MODE)		printf("can't reset the I2C modules\n");		#endif		return;	}		io_outpb(I2C_IOADDR + 0x07, (io_inpb(I2C_IOADDR + 0x07) & 0xf0) | 0x07); // I2C0 Extra Control Register			                                                                 // DIMC:  Enaable  (0)			                                                                 // DI196: Disaable (1)			                                                                 // DIAR:  Enaable  (1)			                                                                 // DIDC:  Disaable (1)	                                                                	i2c_ClearSTAT(0, I2CSTAT_ALL);	i2c_DisableNoiseFilter(0);	i2c_DisableStandardHSM(0);		if(twi_speed <= 100000L)		i2c_SetSpeed(0, I2CMODE_AUTO, 100000L);	else if(twi_speed >= 400000L)		i2c_SetSpeed(0, I2CMODE_AUTO, 400000L);	else		i2c_SetSpeed(0, I2CMODE_AUTO, twi_speed);		i2cslave_SetAddr(0, 0x7f);	i2cslave_EnableACK(0);		io_outpb(CRSB_BIT_G1 + 0, 0x08); // SCL(RICH_IO)	io_outpb(CRSB_BIT_G1 + 1, 0x08); // SDA(RICH_IO)    if(irq_Init() == false)     {        #if defined (DEBUG_MODE)		printf("irq_init fail\n");		#endif		return;    }        if(irq_Setting(usedirq, IRQ_LEVEL_TRIGGER) == false)    {    	#if defined (DEBUG_MODE)        printf("%s\n", __FUNCTION__);		#endif		return;    }        if(irq_InstallISR(usedirq, I2C_ISR, NULL) == false)    {    	#if defined (DEBUG_MODE)        printf("irq_install fail\n");		#endif		return;    }    i2c_EnableINT(0, I2CINT_ALL);        /*    #if defined (DEBUG_MODE)	printf("I2C init success\n");	for(int i=0; i<256; i++)	{		if(i%16 == 0) printf("\n");		printf("%02X ", io_inpb(I2C_IOADDR + i));	}	#endif	*/}void twi_init(void) // Arduino compaliable function{	_twi_init(100000L);}void twi_init(uint32_t speed) // VertexEX's special function{	_twi_init(speed);}/*  * Function twi_slaveInit * Desc     sets slave address and enables interrupt * Input    none * Output   none */void twi_setAddress(uint8_t address){  // set twi slave address (skip over TWGCE bit)  i2cslave_SetAddr(0, address);}// Vortex86EX I2C status Register#define MT_RXRDY        (I2CSTAT_RXRDY + 0x03)#define MT_RXRDY_NOBUSY (I2CSTAT_RXRDY + 0x01)#define MT_TX_DONE  	(I2CSTAT_TXDONE + 0x03)#define MT_NACK_ERR 	(I2CSTAT_ACKERR +0x03)#define MT_ARB_LOST 	(I2CSTAT_ARLOSS + 0x03)#define SL_NEEDTOW  	(I2CSTAT_SLAVEWREQ + 0x02)#define SL_RXRDY    	(I2CSTAT_RXRDY + 0x02)#define SL_TX_DONE  	(I2CSTAT_TXDONE + 0x02)#define SL_NACK_ERR 	(I2CSTAT_ACKERR + 0x02)#define SL_ARB_LOST 	(I2CSTAT_ARLOSS + 0x02)#define SL_GET_STOP 	(I2CSTAT_SLAVESTOPPED + 0x01)// error code#define MT_ADDR_NACK    (0)#define MT_DATA_NACK    (1)#define MT_ARB_LT       (2)/*  * Function twi_readFrom * Desc     attempts to become twi bus master and read a *          series of bytes from a device on the bus * Input    address: 7bit i2c device address *          data: pointer to byte array *          length: number of bytes to read into array *          sendStop: Boolean indicating whether to send a stop at the end * Output   number of bytes read */uint8_t twi_readFrom(uint8_t address, uint8_t* data, uint8_t length, uint8_t sendStop){  uint8_t i;  #if defined (DEBUG_MODE)  printf("I2C readfrom start\n");  #endif  // ensure data will fit into buffer  if(TWI_BUFFER_LENGTH < length){    return 0;  }  // wait until twi is ready, become master receiver  while(TWI_READY != twi_state){    continue;  }  twi_state = TWI_MT_RX;  twi_sendStop = sendStop;  // reset error state (0xFF.. no error occured)  twi_error = 0xFF;  // initialize buffer iteration vars  twi_masterBufferIndex = 0;  twi_masterBufferLength = length;  // This is not intuitive, read on...  // On receive, the previously configured ACK/NACK setting is transmitted in  // response to the received byte before the interrupt is signalled.   // Therefor we must actually set NACK when the _next_ to last byte is  // received, causing that NACK to be sent in response to receiving the last  // expected byte of data.  // build sla+w, slave device address + w bit  twi_slarw = address;  if (true == twi_inRepStart) {    // if we're in the repeated start state, then we've already sent the start,    // (@@@ we hope), and the TWI statemachine is just waiting for the address byte.    // We need to remove ourselves from the repeated start state before we enable interrupts,    // since the ISR is ASYNC, and we could get confused if we hit the ISR before cleaning    // up. Also, don't enable the START interrupt. There may be one pending from the     // repeated start that we sent outselves, and that would really confuse things.    twi_inRepStart = false;			// remember, we're dealing with an ASYNC ISR  }  else  {    // send start condition    i2cmaster_WriteAddrREG(0, twi_slarw, I2C_READ);  }    // wait for read operation to complete  while(TWI_MT_RX == twi_state){    #if defined (DEBUG_MODE)	printf("twi_readFrom: interrupt times = %d status = 0x%02X\n", interrupt_times, temp_state);	#endif	continue;  }  if (twi_masterBufferIndex < length)    length = twi_masterBufferIndex;  // copy twi buffer to data  for(i = 0; i < length; ++i){    data[i] = twi_masterBuffer[i];  }  #if defined (DEBUG_MODE)  printf("I2C readfrom success\n");  #endif	  return length;}bool send_twi_addr = false;/*  * Function twi_writeTo * Desc     attempts to become twi bus master and write a *          series of bytes to a device on the bus * Input    address: 7bit i2c device address *          data: pointer to byte array *          length: number of bytes in array *          wait: boolean indicating to wait for write or not *          sendStop: boolean indicating whether or not to send a stop at the end * Output   0 .. success *          1 .. length to long for buffer *          2 .. address send, NACK received *          3 .. data send, NACK received *          4 .. other twi error (lost bus arbitration, bus error, ..) */uint8_t twi_writeTo(uint8_t address, uint8_t* data, uint8_t length, uint8_t wait, uint8_t sendStop){  uint8_t i;  #if defined (DEBUG_MODE)  printf("I2C writeto start\n");  #endif  // ensure data will fit into buffer  if(TWI_BUFFER_LENGTH < length){    return 1;  }  // wait until twi is ready, become master transmitter  while(TWI_READY != twi_state){    continue;  }  twi_state = TWI_MT_TX;  twi_sendStop = sendStop;  // reset error state (0xFF.. no error occured)  twi_error = 0xFF;  // initialize buffer iteration vars  twi_masterBufferIndex = 0;  twi_masterBufferLength = length;    // copy data to twi buffer  for(i = 0; i < length; ++i){    twi_masterBuffer[i] = data[i];  }    // build sla+w, slave device address + w bit  twi_slarw = address;    // if we're in a repeated start, then we've already sent the START  // in the ISR. Don't do it again.  //  if (true == twi_inRepStart) {    // if we're in the repeated start state, then we've already sent the start,    // (@@@ we hope), and the TWI statemachine is just waiting for the address byte.    // We need to remove ourselves from the repeated start state before we enable interrupts,    // since the ISR is ASYNC, and we could get confused if we hit the ISR before cleaning    // up. Also, don't enable the START interrupt. There may be one pending from the     // repeated start that we sent outselves, and that would really confuse things.    twi_inRepStart = false;			// remember, we're dealing with an ASYNC ISR  }  else  {  	send_twi_addr = true;    // send start condition    i2cmaster_WriteAddrREG(0, twi_slarw, I2C_WRITE);  }    // wait for write operation to complete  while(wait && (TWI_MT_TX == twi_state)){     	continue;  }  #if defined (DEBUG_MODE)  printf("I2C writeto success\n");  printf("Interrupt times = %d status = 0x%02X\n", interrupt_times, temp_state);  #endif    if (twi_error == 0xFF)    return 0;	// success  else if (twi_error == MT_ADDR_NACK)    return 2;	// error: address send, nack received  else if (twi_error == MT_DATA_NACK)    return 3;	// error: data send, nack received  else    return 4;	// other twi error}/*  * Function twi_transmit * Desc     fills slave tx buffer with data *          must be called in slave tx event callback * Input    data: pointer to byte array *          length: number of bytes in array * Output   1 length too long for buffer *          2 not slave transmitter *          0 ok */uint8_t twi_transmit(const uint8_t* data, uint8_t length){  uint8_t i;  #if defined (DEBUG_MODE)  printf("I2C transmit start\n");  #endif  // ensure data will fit into buffer  if(TWI_BUFFER_LENGTH < length){    return 1;  }    // ensure we are currently a slave transmitter  if(TWI_SL_TX != twi_state){    return 2;  }    // set length and copy data into tx buffer  twi_txBufferLength = length;  for(i = 0; i < length; ++i){    twi_txBuffer[i] = data[i];  }  #if defined (DEBUG_MODE)  printf("I2C transmit success\n");  #endif    return 0;}/*  * Function twi_attachSlaveRxEvent * Desc     sets function called before a slave read operation * Input    function: callback function to use * Output   none */void twi_attachSlaveRxEvent( void (*function)(uint8_t*, int) ){  twi_onSlaveReceive = function;}/*  * Function twi_attachSlaveTxEvent * Desc     sets function called before a slave write operation * Input    function: callback function to use * Output   none */void twi_attachSlaveTxEvent( void (*function)(void) ){  twi_onSlaveTransmit = function;}/*  * Function twi_stop (it only for i2c write operation, not read) * Desc     relinquishes bus master status * Input    none * Output   none */void twi_stop(void){  #if defined (DEBUG_MODE)  printf("I2C send stop start\n");  #endif  // send stop condition  i2cmaster_SetStopBit(0);  // wait for stop condition to be exectued on bus  // TWINT is not set after a stop condition!  while(i2cmaster_CheckStopBit(0) == true)  {  	#if defined (DEBUG_MODE)    printf("twi_stop: interrupt times = %d status = 0x%02X\n", interrupt_times, temp_state);    #endif  }  #if defined (DEBUG_MODE)  printf("I2C send stop sucess\n");  #endif  // update twi state  twi_state = TWI_READY;}/*  * Function twi_releaseBus * Desc     releases bus control * Input    none * Output   none */void twi_releaseBus(void){  // release bus  if(i2c_Reset(0) == false)  {    #if defined (DEBUG_MODE)    printf("i2c_Reset fail\n");    #endif    return;  }  #if defined (DEBUG_MODE)  printf("i2c_Reset success\n");  #endif  // update twi state  twi_state = TWI_READY;}int I2C_ISR(int irq, void* data){   unsigned char status = i2c_ReadStatREG(0);  i2c_ClearSTAT(0, I2CSTAT_ALL);  #if defined (DEBUG_MODE)  interrupt_times++; temp_state = status;  #endif  switch(status){    // Master Transmitter(It is named "TX_Done" flag in Vortex86EX CPU's "I2C0 Status Register")    case MT_TX_DONE: // Because Vortex86EX have no 4 modes like AVR serise, so we must distinguish them in I2C_ISR by myself when TX_Done occured	  if(twi_state == TWI_MT_RX)	  {	  	  if(twi_masterBufferLength > 1)	  	   	i2c_WriteDataREG(0, DUMMY_VAL);		  else		  {            if (twi_sendStop) i2cmaster_SetStopBit(0);		    else		    {		      twi_inRepStart = true;	// we're gonna send the START		      // don't enable the interrupt. We'll generate the start, but we 		      // avoid handling the interrupt until we're in the next transaction,		      // at the point where we would normally issue the start.		      i2cmaster_WriteAddrREG(0, twi_slarw, I2C_WRITE); // WRITE??		      send_twi_addr = true;		      twi_state = TWI_READY; // Enter next "WRITE" opration as soon as possible		    }			i2c_WriteDataREG(0, DUMMY_VAL);			if (twi_sendStop)			{				while(i2cmaster_CheckStopBit(0) == true); // wait for stop condition compile			    twi_state = TWI_READY;			}			// else drop work to MT_TX_DONE		  }		  break;	  }	  	  if(send_twi_addr == true) send_twi_addr = false;	  	  // if there is data to send, send it, otherwise stop       if(twi_masterBufferIndex < twi_masterBufferLength)	  {        // copy data to output register and ack        i2c_WriteDataREG(0, twi_masterBuffer[twi_masterBufferIndex++]);      }	  else	  {		  if (twi_sendStop) twi_stop();		  else		  {			  twi_inRepStart = true;	// we're gonna send the START			  // don't enable the interrupt. We'll generate the start, but we 			  // avoid handling the interrupt until we're in the next transaction,			  // at the point where we would normally issue the start.			  i2cmaster_WriteAddrREG(0, twi_slarw, I2C_READ); // I2C_READ??			  twi_state = TWI_READY; // Enter next "READ" opration as soon as possible	      }      }      break;    case MT_NACK_ERR:       if(send_twi_addr == true) twi_error = MT_ADDR_NACK; else twi_error = MT_DATA_NACK;      send_twi_addr = false;      twi_stop();      break;    case MT_ARB_LOST: // lost bus arbitration      twi_error = MT_ARB_LT;      twi_releaseBus();      break;    // Master Receiver    case MT_RXRDY: // data received, ack sent    case MT_RXRDY_NOBUSY:      // put byte into buffer      twi_masterBuffer[twi_masterBufferIndex++] = i2c_ReadDataREG(0);	  // Is the last data? if it is, set stopbit then do dummywrite	  if(twi_masterBufferIndex == (twi_masterBufferLength - 1))	  {        if (twi_sendStop) i2cmaster_SetStopBit(0);	    else	    {	      twi_inRepStart = true;	// we're gonna send the START	      // don't enable the interrupt. We'll generate the start, but we 	      // avoid handling the interrupt until we're in the next transaction,	      // at the point where we would normally issue the start.	      i2cmaster_WriteAddrREG(0, twi_slarw, I2C_WRITE); // WRITE??	      send_twi_addr = true;	      twi_state = TWI_READY; // Enter next "WRITE" opration as soon as possible	    }		i2c_WriteDataREG(0, DUMMY_VAL);		if (twi_sendStop)		{			while(i2cmaster_CheckStopBit(0) == true); // wait for stop condition compile		    twi_state = TWI_READY;		}		// else drop work to MT_TX_DONE		break;          }            if(twi_masterBufferIndex < twi_masterBufferLength)        i2c_WriteDataREG(0, DUMMY_VAL);            break;    // TW_MR_ARB_LOST handled by TW_MT_ARB_LOST case    // Slave Receiver    case SL_RXRDY:      if(twi_state != TWI_SL_RX)      {		  // enter slave receiver mode	      twi_state = TWI_SL_RX;	      // indicate that rx buffer can be overwritten and ack	      twi_rxBufferIndex = 0;	  }      // if there is still room in the rx buffer      if(twi_rxBufferIndex < TWI_BUFFER_LENGTH){        // put byte in buffer and ack        twi_rxBuffer[twi_rxBufferIndex++] = i2c_ReadDataREG(0);      }      //printf("SL_RXRDY ISR: interrupt times = %d status = 0x%02X\n", interrupt_times, temp_state);      break;      	case (I2CSTAT_RXRDY + SL_GET_STOP): // stop or repeated start condition received	case SL_GET_STOP:      // put a null char after data if there's room      if(twi_rxBufferIndex < TWI_BUFFER_LENGTH){        twi_rxBuffer[twi_rxBufferIndex] = '\0';      }      // callback to user defined callback      twi_onSlaveReceive(twi_rxBuffer, twi_rxBufferIndex);      // since we submit rx buffer to "wire" library, we can reset it      twi_rxBufferIndex = 0;      twi_state = TWI_READY;      //printf("SL_GET_STOP ISR: interrupt times = %d status = 0x%02X\n", interrupt_times, temp_state);      break;    case SL_NACK_ERR:      break;    case SL_ARB_LOST:      break;        // Slave Transmitter    case SL_NEEDTOW:      if(twi_state != TWI_SL_TX)      {		  // enter slave transmitter mode	      twi_state = TWI_SL_TX;	      // ready the tx buffer index for iteration	      twi_txBufferIndex = 0;	      // set tx buffer length to be zero, to verify if user changes it	      twi_txBufferLength = 0;	      // request for txBuffer to be filled and length to be set	      // note: user must call twi_transmit(bytes, length) to do this	      twi_onSlaveTransmit();	      // if they didn't change buffer & length, initialize it	      if(0 == twi_txBufferLength){	        twi_txBufferLength = 1;	        twi_txBuffer[0] = 0x00;	      }	  }      // transmit first byte from buffer, fall      i2c_WriteDataREG(0, twi_txBuffer[twi_txBufferIndex++]);      break;          case (I2CSTAT_SLAVEWREQ + SL_TX_DONE):      i2c_WriteDataREG(0, twi_txBuffer[twi_txBufferIndex++]);      // no break, continue...    case SL_TX_DONE: // byte sent, ack returned      // if there is more to send, ack, otherwise nack      if(twi_txBufferIndex >= twi_txBufferLength){        twi_txBufferIndex = twi_txBufferLength - 1; // if too many SL_NEEDTOW command, send the last byte to master      }      break;    #if defined (DEBUG_MODE)    default:    	printf("XXX ISR: interrupt times = %d status = 0x%02X\n", interrupt_times, temp_state);	#endif	  }  return ISR_HANDLED;}