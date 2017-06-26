#include "define.h"
volatile BYTE NoRecDataU0Cnt;
volatile BYTE NoRecDataU1Cnt;
unsigned int rx1_buff_len;
unsigned int buffLen;
extern int NextByteIndex;
extern char readClockBuf[];             //buffer for data reading from clock
extern char ComBuf[MAX_SBD_BUF_LEN];
extern volatile char RxUart0Buf[MAX_RX_BUF_LEN];
extern char RxUart1Buf[MAX_RX1_BUF_LEN];
extern char TxUart0Buf[MAX_UART0_TX_BUF_LEN];
extern BYTE LedStatus[4];
//extern BYTE BlinkNum[4];
extern int BytesToSend;
extern unsigned int rx0_buff_len;
extern BYTE ModemResponse;
extern volatile BYTE mainTask;
extern BYTE modemCurSubTask;
extern BYTE bMonitorConnected;
extern int TimeLeftForWaiting;
extern bit bCheckRxBuf;
extern bit longAnswerExpected;
extern bit overFlow;
extern int nTimeCnt;
extern bit bWaitForModemAnswer;
extern bit bWaitForMonitorCmd;
extern bit bWait4WLSensor;
extern bit bWaitForGPSData;
extern bit bNeedToWait4Answer;
extern bit bReset;
extern int nMaxWaitingTime;
extern unsigned int nextCompare;
#ifdef BlueToothOption
extern int nMaxWaitBtResponse;
#endif BlueToothOption
extern char tagSwitched;
static BYTE cntr = 0;
extern BYTE flgUart1Error;
#pragma used+
void putchar1(char c)
{
//#ifdef TestMode
    if (mainTask == TASK_MEASURE)
        return;
    #ifdef DebugOnUart0
    putchar0(c);
    #else
//    if (PINC.6 != 1)
    //UART_1_MONITOR();
    UART1Select(UART_DBG);
    while ((UCSR1A & DATA_REGISTER_EMPTY)==0);
    UDR1=c;
//    if (mainTask == TASK_MEASURE)
//        UART1Select(UART_RADIO_UHF);
        //UART_1_WIRELESS();

    #endif  DebugOnUart0
//#endif TestMode
}
#pragma used-

#pragma used+

void putchar0(char c)
{
    while ((UCSR0A & DATA_REGISTER_EMPTY)==0);
    UDR0 = c;
}
#pragma used-

// Timer 0 overflow interrupt service routine
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
    #ifdef DebugMode
    BYTE i;
    #endif DebugMode
    if (bWaitForModemAnswer)
    {
        NoRecDataU0Cnt++;
        if (NoRecDataU0Cnt > 2)
        {
            bWaitForModemAnswer = FALSE;
            bCheckRxBuf = TRUE;
            longAnswerExpected = 0;
            DISABLE_TIMER0();
            #ifdef DebugMode
            for (i = 0; i < rx0_buff_len ;i++)
                putchar1(RxUart0Buf[i]);
            #endif DebugMode
        }
    }
}

// Timer1 output compare A interrupt service routine - EVERY 100 ML SECOND
interrupt [TIM1_COMPA] void timer1_compa_isr(void)
{
    BYTE i;
    //reset next interupt
//    nextCompare += 0x168;
//    OCR1AH = (unsigned char)(nextCompare >> 8  )& 0xFF;       // add high byte of 0x168 (0x1) to the high byte
//    OCR1AL = (unsigned char) (nextCompare) & 0xFF;            // add low byte of 0x168 (0x68) to the low byte

    if(nTimeCnt > 0)
    {
//         #ifdef DebugMode
//         putchar1(nTimeCnt);
//         #endif DebugMode
         nTimeCnt--;
//         cpuWakeTimeCnt++;
//         if (cpuWakeTimeCnt > 60000)
//            cpuWakeTimeCnt = 0;
    }
    if ((bWaitForModemAnswer) && (TimeLeftForWaiting > 0))
    {
        TimeLeftForWaiting--;
    }

    if ((bWaitForMonitorCmd) && (TimeLeftForWaiting > 0))
    {
        TimeLeftForWaiting--;
//        #ifdef DebugMode
//        putchar1(TimeLeftForWaiting);
//        #endif DebugMode
    }
    #ifdef BlueToothOption
    if (nMaxWaitBtResponse > 0)
        nMaxWaitBtResponse--;
    #endif BlueToothOption

    // leds:
    cntr++;
    if (cntr > 5)
    {
//         #ifdef DebugMode
//            putchar1('&');
//         if (LedStatus[LED_3] == LED_BLINK)
//            putchar1('@');
////         PrintNum(nextCompare);
//         #endif DebugMode
        cntr = 0;
        for (i = 1; i <= 3; i++)
            if (LedStatus[i] == LED_BLINK)
            {
                if (i == LED_1)
                    PORTA.5 ^= 1;
                if (i ==  LED_2)
                    PORTA.6 ^= 1;
                if (i ==  LED_3)
                    PORTA.7 ^= 1;
            }
    }
}


// Timer2 overflow interrupt service routine
interrupt [TIM2_OVF] void timer2_ovf_isr(void)
{
    #ifdef BlueToothOption
    if ((mainTask == TASK_BLUETOOTH) || (bWaitForMonitorCmd) || (bWait4WLSensor))
    #else
    if ((bWaitForMonitorCmd) || (bWait4WLSensor))
    #endif BlueToothOption
    {
        NoRecDataU1Cnt++;
        if (NoRecDataU1Cnt >= 4)
        {
            WIRELESS_CTS_OFF();
            bWaitForMonitorCmd = FALSE;
            bWait4WLSensor = FALSE;
            bCheckRxBuf = TRUE;
            buffLen = rx1_buff_len;
            rx1_buff_len = 0;
            DISABLE_TIMER2();
            //delay_ms(1500); //
        }
    }
    else
        if (bWaitForGPSData == TRUE)
            if (rx1_buff_len >= MINMEA_MAX_LENGTH)
            {
                bCheckRxBuf = TRUE;
                bWaitForGPSData = FALSE;
                buffLen = rx1_buff_len;
                DISABLE_TIMER2();
            }
}

// USART0 Receiver interrupt service routine
interrupt [USART0_RXC] void usart0_rx_isr(void)
{
    BYTE LastRxByte;
    char status;
    status = UCSR0A;
    LastRxByte = UDR0;       //read uart data register
//    ENABLE_TIMER0();
    if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0)
    {
        //when getting answer for post what we need is end of data-not the begining so when reach to MAX_TX_BUF_LEN - start rewrite on the buffer from the begining.
        // the rx0_buff_len represent last index data got into it.
        if (longAnswerExpected)
        {
            // if reach to end of ComBuf - start from the begining
            if(/*lastByteIndex*/rx0_buff_len >= MAX_RX_BUF_LEN)
            {
                overFlow = 1;
                /*lastByteIndex*/rx0_buff_len = 0;
            }
            RxUart0Buf[/*lastByteIndex*/rx0_buff_len++] = LastRxByte;
        }
        else
        {
            //set the msg into ComBuf[] buffer
            if(rx0_buff_len >= MAX_RX_BUF_LEN)
                return; //exit the function
            RxUart0Buf[rx0_buff_len++] = LastRxByte;
        }
//        putchar1(LastRxByte);
        ENABLE_TIMER0();
    }
    NoRecDataU0Cnt = 0;
    // enable timer of receive
}

// USART1 Receiver interrupt service routine
interrupt [USART1_RXC] void usart1_rx_isr(void)
{
    char status, data;
    status = UCSR1A;
    data = UDR1;

    if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0)
    {
        //set the msg into RxUart1Buf[] buffer
        if(rx1_buff_len >= MAX_RX1_BUF_LEN)
            return; //exit the function
        RxUart1Buf[rx1_buff_len++] = data;
        ENABLE_TIMER2();
    }
    else
        flgUart1Error = status;
    NoRecDataU1Cnt = 0;
}

// External Interrupt 2 service routine  // clock int
interrupt [EXT_INT2] void ext_int2_isr(void)
{
    DISABLE_CLOCK_INT();
    PRR &= 0x00;
    mainTask = TASK_WAKEUP;
}

// Pin change 0-7 interrupt service routine
// check if battery is currently charged
//interrupt [PC_INT0] void pin_change_isr0(void)
//{
//  if (PINA.2 == 0)
//    tagSwitched = TRUE;
//  else
//    tagSwitched = FALSE;
//}

//void TransmitBuf(unsigned char* ComBuf, unsigned char BytesToSend, char iPortNum)
void TransmitBuf(char iPortNum)
{
    int i;
    #ifdef DebugMode
    int nCnt, nbi = 0;
    #endif DebugMode

    NextByteIndex = 0;	// reset for Tx
    if (iPortNum == 0)
	    while (bWaitForModemAnswer == TRUE); //wait until rx0 end
//    if (iPortNum == 1)
//	    while (bWaitForMonitorCmd == TRUE); //wait until rx1 end
//    DISABLE_RX_INT_UART0();

    if ((iPortNum == 1)) // sending to monitor
    {
        rx1_buff_len = 0;
        //DISABLE_RX_INT_UART1();
        while(BytesToSend-- )
        {
            while ((UCSR1A & DATA_REGISTER_EMPTY)==0);
 		    UDR1 = ComBuf[ NextByteIndex++];// send next byte..
        }
    }

	if (iPortNum == 0)
    {
        // clear RX buf
        for (i = 0; i < MAX_RX_BUF_LEN; i++)
            RxUart0Buf[i] = 0;
        rx0_buff_len = 0;
        //DISABLE_RX_INT_UART0();

        if (modemCurSubTask != SUB_TASK_SBD_SND/*_DATA*/)
        {
            #ifdef DebugMode         //todo- put under debug @@@
            nCnt = BytesToSend;
            while(nCnt-- )
            {
                // wait for UART's shift register to finish sending byte
                while(( UCSR1A & DATA_REGISTER_EMPTY)==0);
                UDR1 = ComBuf[ nbi++ ];// send next byte..
            }
             //delay_ms(500);
            #endif DebugMode      //@@@
            while(BytesToSend-- )
            {
                // wait for UART's shift register to finish sending byte
                while(( UCSR0A & DATA_REGISTER_EMPTY)==0);
                UDR0 = ComBuf[ NextByteIndex ];// send next byte..
                NextByteIndex++;
            }
        }
        else
        {
            while(BytesToSend-- )
            {
                // wait for UART's shift register to finish sending byte
                while(( UCSR0A & DATA_REGISTER_EMPTY)==0);
                UDR0 = TxUart0Buf[ NextByteIndex ];// send next byte..
//                #ifdef DebugMode
                UDR1 = TxUart0Buf[ NextByteIndex ];// send next byte..
//                #endif DebugMode
                NextByteIndex++;
            }
        }

        //ENABLE_RX_INT_UART0();
    }

    #ifdef DebugMode
    if (iPortNum == 2) //debug
    {
        //DISABLE_RX_INT_UART1();
        while(BytesToSend-- )
        {
            while ((UCSR1A & DATA_REGISTER_EMPTY)==0);
            //testAT[NextByteIndex] = ComBuf[ NextByteIndex ];  // 4 debug
            UDR1 = ComBuf[ NextByteIndex++ ];// send next byte..
        }
        rx1_buff_len = 0;
        //ENABLE_RX_INT_UART1();
    }
    #endif DebugMode
	NextByteIndex = 0;//prepare for Rx

//    if (iPortNum != 2)
//        ModemResponse = NO_ANSWER;
    if (iPortNum == 0)
    {
        ModemResponse = NO_ANSWER;
	    TimeLeftForWaiting = nMaxWaitingTime;
        NoRecDataU0Cnt = 0;
        if (bNeedToWait4Answer == TRUE)
        {
            bWaitForModemAnswer = TRUE;
        }
    }
    if (iPortNum == 1)//else
    {
        NoRecDataU1Cnt = 0;
        if(mainTask == TASK_MONITOR)
        {
            TimeLeftForWaiting = MAX_WAIT_MNTR_SEC * 10;
            bWaitForMonitorCmd = TRUE;
        }
    }
    bCheckRxBuf = FALSE;
//    ENABLE_RX_INT_UART0();
}

