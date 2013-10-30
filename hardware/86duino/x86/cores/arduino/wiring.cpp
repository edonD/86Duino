#include "Arduino.h"
#include "io.h"
#define USE_COMMON
#include "common.h"
#include "usb.h"
#include "mcm.h"

void *USBDEV = NULL;
unsigned long millis() {
	return timer_nowtime();
}

void delay(unsigned long ms) {
	delay_ms(ms);
}

unsigned long micros() {
	#define CLOCKS_PER_MICROSEC (299UL)
	return (getclocks64()/CLOCKS_PER_MICROSEC);
}

void delayMicroseconds(unsigned long us) {
    delay_us(us);
}

bool init() {
	int i, crossbarBase, gpioBase;
	if(io_Init() == false) return false;

	//set corssbar Base Address
	crossbarBase = sb_Read16(SB_CROSSBASE) & 0xfffe;
	if(crossbarBase == 0 || crossbarBase == 0xfffe)
		sb_Write16(SB_CROSSBASE, CROSSBARBASE | 0x01);
	
	// Force set HIGH speed ISA on SB
	sb_Write(SB_FCREG, sb_Read(SB_FCREG) | 0x8000C000L);
	
	//set SB GPIO Base Address
	gpioBase = sb_Read16(SB_GPIOBASE) & 0xfffe;
	if(gpioBase == 0 || gpioBase == 0xfffe)
	{
		sb_Write16(SB_GPIOBASE, GPIOCTRLBASE | 0x01);
		gpioBase = GPIOCTRLBASE;
	}
	
	// Enable GPIO 0 ~ 7 
	io_outpdw(gpioBase, 0x00ff);
	
	// set GPIO Port 0~7 dircetion & data Address
	for(i=0;i<8;i++)
		io_outpdw(gpioBase + (i+1)*4,((GPIODIRBASE + i*4)<<16) + GPIODATABASE + i*4);
	  
	// set ADC Base Address
	sb_Write(0xbc, sb_Read(0xbc) & (~(1L<<28)));  // active adc
	sb1_Write16(0xde, sb1_Read16(0xde) | 0x02);   // not Available for 8051A Access ADC
	sb1_Write(0xe0, 0x0010fe00L); // baseaddr = 0xfe00, disable irq
	
	// set MCM Base Address
	set_MMIO();
	mc_setbaseaddr();
	for(i=0; i<4; i++)
		mc_SetMode(i, MCMODE_PWM_SIFB);
	
	//CDC
	USBDEV = CreateUSBDevice();
    if(USBDEV == NULL)
    {
        printf("init error\n");
        return false;
    }
    
    usb_SetUSBPins(USBDEV, 7, 0, 7, 1);
    if(usb_Init(USBDEV) == false)
    {
        printf("init2 error\n");
        return false;
    }
	
	//io_Close();
	return true;
}
