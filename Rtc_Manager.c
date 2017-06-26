#ifndef __RTC_C
#define __RTC_C
#include <bcd.h>    //for bcd 2 bin conversion
#include "define.h"

//declare local & global variables
char clockBuf[7]; 		 //buffer for all clock operation need
char cmdByte;			 //buffer for the command byte
extern char e2_writeFlag;
extern char readClockBuf[];	         //buffer for data reading from clock
extern unsigned int time_in_minutes;     //time from day start in ninutes
//extern char err_buf[ERR_BUF_SIZE];

void RtcConfig(char cmd, char prm)
{
    do
    {
        clockBuf[0] = prm;
        SendBuf(cmd , 1, clockBuf);
        cmdByte = cmd + 1;	    // change command to read
        clockBuf[0] = 0x00;	    // init buf
        GetBuf(cmdByte, 1, clockBuf);      // read rtc
//        #ifdef DebugMode
//        SendDebugMsg("\r\nclock config result:\0");
//        putchar1(clockBuf[0]);
//        putchar1('&');
//        #endif DebugMode
    }
    while (!(clockBuf[0] == prm));             // check config was set OK
}
//return the buf with upsidedown bits order for the use of data "writen" into the rtc
unsigned char ByteUpsideDown(unsigned char byteBuf)
{
    unsigned char i;
    unsigned char temp = 0;
    unsigned char bufTemp;
    bufTemp = byteBuf;
    for(i = 0; i < 8; i++)
    {
        temp = temp << 1;
        temp |=  (bufTemp & 0x01);
        bufTemp = bufTemp >> 1;
    }
	return temp;
}

//arrange the clock buf before setiing time into rtc
void SetClockBuf(void)
{
	unsigned char i, temp;

	/*clockBuf[0] = (0x0D); //year data = 00
	clockBuf[1] = (0x0A); //month data = 01
	clockBuf[2] = (0x07); //day data = 01
	clockBuf[3] = (0x02); //day of week data = 01
	clockBuf[4] = (0x0D); //hour = 14 pm
	clockBuf[5] = (0x20); //minute data = 55
	clockBuf[6] = (0x00); //second data = 30
    #ifdef DebugMode
    SendDebugMsg("\r\nSetClock");
    #endif DebugMode         */
	//the clockBuf will be set by the communication module
	//change the fuffer to bcd format and up side down
	for(i = 0; i < 7; i++)
	{
		temp = clockBuf[i];
	 	clockBuf[i] = bin2bcd(temp);
		temp = clockBuf[i];
		clockBuf[i] = ByteUpsideDown(temp);
	}
}

//power on initialization ruthin
//check if power flag is on, if yes you should preform reset to r.t.cclock
unsigned char IsPowerFlagOn(void)
{
    BYTE res = FALSE;
    cmdByte = 0x61;	 //status register 1
//    #ifdef DebugMode
//    SendDebugMsg("\r\nIsPowerFlagOn: \0");
//    #endif DebugMode

    do
    {
        res = GetBuf(cmdByte, 1, readClockBuf);
    }
    while (res == FALSE);

//	readClockBuf[0] = ByteUpsideDown(readClockBuf[0]);
//    if(readClockBuf[0] & 0x80)		//if power flag is on

    if (readClockBuf[0] & 0x01)
        return TRUE;

	return FALSE;
}

void ResetCommand(void)
{
//    #ifdef DebugMode
//    SendDebugMsg("\r\nreset rtc \0");
//    #endif DebugMode
    do
    {
        cmdByte = 0x60;	 //status register 1 access
        //preper B7=1 (reset command) B6=1(24 hours clock)
        clockBuf[0] = 0xC0;
        SendBuf(cmdByte , 1, clockBuf); // FALSE)	 //if SendBuf function faild
        cmdByte = 0x61;
        clockBuf[0] = 0;
        GetBuf(cmdByte , 1, clockBuf);
//        #ifdef DebugMode
//        SendDebugMsg("\r\nreset result: \0");
//        PrintNum(clockBuf[0]);
//        #endif DebugMode
        //readClockBuf[0] = ByteUpsideDown(readClockBuf[0]);
    }
    while (!(clockBuf[0] & 0x40));		//   24 hours config is set
}

void SetRtc24Hour()
{
//    #ifdef DebugMode
//    SendDebugMsg("\r\nreset rtc 24\0");
//    #endif DebugMode
    RtcConfig(0x60, 0x40);
}

//set the status register for interrupt
void ResetClockIntr()
{
//    #ifdef DebugMode
//    SendDebugMsg("\r\nreset clk int\0");
//    #endif DebugMode
//    RtcConfig(0x62, 0x80);  // 1 hz - per sec int test
//    RtcConfig(0x68, 0x80);  // 1 hz - per sec int test. remove lines from SaveMeasurments func

//    RtcConfig(0x62, 0xC0);  // per min 50% test
    RtcConfig(0x62, 0x40);  // per minute original
}

void DisableClockIntr(void)
{
//    #ifdef DebugMode
//    SendDebugMsg("\r\ndisablr clock int\0");
//    #endif DebugMode
    RtcConfig(0x62, 0x00);
}

unsigned char SetRealTime(void)
{
    BYTE bCheck = FALSE;

	cmdByte = 0x64;   //preper real Time setting command
    // check if hour is in PM period
    if (clockBuf[4] > 11)
    {
        bCheck = TRUE;
//        #ifdef DebugMode
//        SendDebugMsg("\r\nServer sent PM hour\0");
//        #endif DebugMode
    }
    if (!((clockBuf[1] >= 1) && (clockBuf[1] <= 12)) && ((clockBuf[2] >= 1) && (clockBuf[2] <= 31))
    && ((clockBuf[4] >= 0) && (clockBuf[4] <= 24)) && ((clockBuf[5] >= 0) && (clockBuf[5] <= 60))) // valid month ,valid day in month ,valid hour, valid minute
    {
        #ifdef DebugMode
        SendDebugMsg("\r\ninvalid data\0");
        #endif DebugMode
        return FAILURE;
    }

    SetClockBuf(); //set clockBuf for time setting
	if(SendBuf(cmdByte , 7, clockBuf) == FALSE) //if SendBuf function faild
	{
		return FAILURE;
	}
    #ifdef DebugMode
//    return SUCCESS;
    #endif DebugMode
    if (bCheck)
    {
        if (ReadTime() == SUCCESS)
        {
            // if hour after setup is 0-i.e. clock couldn't get pm hour
            if (readClockBuf[4] == 0)
            {
//                #ifdef DebugMode
//                SendDebugMsg("\r\nRtc could set PM hour\0");
//                #endif DebugMode
                SetRtc24Hour(); //config rtc to am-pm mode
            }
        }
    }
	return SUCCESS;
}

//read the clock data into buffer
unsigned char ReadTime(void)
{
    unsigned char i, temp;
	cmdByte = 0x65;			//read time data registers

	if(GetBuf(cmdByte , 7, readClockBuf) == FALSE)	//if getBuf function faild
	{
        return FAILURE;
	}

	//change the hour reading (ignor the bit 7,6)
	readClockBuf[4] = readClockBuf[4] & 0xFC;
	//change the buffer to normal reading order
	for(i = 0; i < 7; i++)
	{
		readClockBuf[i] = ByteUpsideDown(readClockBuf[i]);
		temp = readClockBuf[i];
	 	readClockBuf[i] = bcd2bin(temp);
	}

	//calculate the time from day start in minutes
	time_in_minutes = ((int)readClockBuf[4] * 60) + (readClockBuf[5]);

	return SUCCESS;
}

//initiate the rtc at program startup
void InitRTC()
{
//    #ifdef DebugMode
//    SendDebugMsg("\r\ninit rtc\0");
//    #endif DebugMode
    ResetCommand();
    //SetRtc24Hour();
    ResetClockIntr(); //1 = freq.interupt; 0 = per minute adge interrupt
    ReadTime();
}

//read the rtc
void GetRealTime(void)
{
	if(e2_writeFlag) //ignor reading if ext_c2 is busy
		return;

    SPCR = 0x00; //reset spi control register

   	ReadTime();
}
#endif  __RTC_C
