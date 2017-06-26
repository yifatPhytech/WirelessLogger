////////////////start of General.c file//////////////
#include "define.h"

//extern eeprom char WakeupInterval;
eeprom int eTimeZoneOffset = 120;                         //offset (in minutes) from UTC
extern eeprom unsigned char eStartConnectionH;        //first connection hour
extern eeprom unsigned char eConnectionInDay;        //number on connectionsin a day
extern eeprom unsigned char eConnectIntervalH;        //intervalbetween connections (hours)
extern eeprom char eLoggerID[];
extern eeprom BYTE eNumSensors;
extern eeprom struct WirelessSensor WLSenArr[];
//const unsigned short days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
//const unsigned short days_in_month_leap_year[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
//extern  BYTE SensorType[];
//extern eeprom char eCombination;
extern int_bytes union_b2i;
long_bytes lb;
float_bytes fb;
//extern unsigned int pSens_ext_e2;	//pointer to current sensor control parameters in ext_e2
extern char e2_writeFlag;
extern char readClockBuf[];	         //buffer for data reading from clock
extern int NextByteIndex;
extern volatile BYTE mainTask;
extern BYTE objToMsr;
extern BYTE toDoList;
extern BYTE monitorCurTask;
extern BYTE bMonitorConnected;
extern BYTE bConnectOK;
extern BYTE msrCurTask;
extern BYTE modemCurTask;
extern BYTE dataSentOK;
extern BYTE prmSentOK;
extern BYTE timeout4Cnct;
extern  BYTE LedStatus[4];
//extern BYTE BlinkNum[4];
extern BYTE bModemType;
extern int BytesToSend;
extern BYTE waitingTask;
extern BYTE nFailureCntr;
extern BYTE iFirstConnToday;
extern unsigned int time_in_minutes;     //time from day start in ninutes
//extern BYTE bPwrRst;
extern BYTE bStlMdmStatus;
extern BYTE nSbdCntr;
extern struct AlertData Alerts[MAX_SEN_NUM];
extern volatile int iLastMsr[MAX_WL_SEN_NUM];
extern bit bCheckRxBuf;
extern bit bWaitForModemAnswer;
extern bit bWaitForMonitorCmd;
extern bit bExtReset;
extern bit bNeedToWait4Answer;
extern bit bEndOfMeasureTask;
extern bit bEndOfModemTask;
extern bit bEndOfMonitorTask;
extern volatile BYTE bEndOfGPSTask;
extern bit bReset;
extern char ComBuf[MAX_SBD_BUF_LEN];
extern unsigned int nextCompare;
extern int iVoltage;
extern BYTE modemCurSubTask;
extern char modemOnStartUp;
extern BYTE flagSucOrFail;
extern BYTE stlRegStatus;
extern BYTE ModemResponse;
extern BYTE arrSendAlldata[MAX_SEN_NUM];
extern BYTE strRegFailCnt;
extern BYTE nRegDenied;
extern const int MinInt;
extern int nTimeCnt;
//extern eeprom int gMin4UTC;
//extern eeprom int gHr4UTC;
extern BYTE btrStatus;

void InitVarsForConnecting()
{
    BYTE n;
    mainTask = TASK_MODEM;
    modemCurTask = TASK_NONE;
    bEndOfModemTask = FALSE;
    bConnectOK = FALSE;
    dataSentOK = FALSE;
    prmSentOK = FALSE;
    objToMsr = SENSOR1;
    flagSucOrFail = FAILURE;
    toDoList = DO_DATA;
    modemOnStartUp = FALSE;

    if ((bExtReset) || (iFirstConnToday == TRUE))
        toDoList = DO_DATA_N_PRMS;
     // if there are no defined sensors yet - do not measure
    if (eNumSensors == 0)
        toDoList = DO_PARAMS;

    if (toDoList == DO_PARAMS)
        waitingTask = SUB_TASK_MODEM_POST_PRM;
    else
        waitingTask = SUB_TASK_MODEM_POST_DATA;
    nSbdCntr = 0;
    nRegDenied = 0;
    nFailureCntr = 0;
    timeout4Cnct = 0;
//    MeasureBatt();
    for (n = 0/*SENSOR1*/; n <= eNumSensors; n++)
        arrSendAlldata[n] = 0; // init array
    ENABLE_UART0();
}

BYTE IsThresholdFound()
{
    int i;
//    #ifdef DebugMode
//    SendDebugMsg("\r\nIsThresholdFound");
//    #endif DebugMode
    for (i = SENSOR1; i <= eNumSensors; i++)
    {
        if ((Alerts[i].Status == TRHRESHOLD_CROSSED) && (Alerts[i].MsrCnt < 3))
        {
            #ifdef DebugMode
            SendDebugMsg("\r\nsensor crossedtreshold");
            #endif DebugMode
            return TRUE;
        }
    }
    return FALSE;
}

BYTE IsTimeToMeasure(void)
{
    char nMin;
	//read the rtc
    e2_writeFlag = 0; //enable reading the rtc
	delay_ms(10); //let stabilate the cpu oscilator
	GetRealTime();
    #ifdef DebugMode
    SendDebugMsg("\r\nnow:\0");
    putchar1(readClockBuf[0]);
    putchar1(readClockBuf[1]);
    putchar1(readClockBuf[2]);
    putchar1(readClockBuf[4]);
    putchar1(readClockBuf[5]);
    SendDebugMsg("\r\n\0");
    #endif DebugMode
//#ifdef DebugMode
//    if ((readClockBuf[5] % 10) == 0)
//        return TRUE;
//    #endif DebugMode
    nMin = GetMinInterval() * INTERVAL_PARAM;
    #ifdef DebugMode
//    SendDebugMsg("\r\nMinutes since midnight: \0");
//    PrintNum(time_in_minutes);
//    SendDebugMsg("\r\nMin interval: \0");
//    PrintNum(nMin);
    SendDebugMsg("\r\nnMinFromMidNight % Min: \0");
    PrintNum(time_in_minutes % nMin);
    #endif DebugMode
    // if u-normal value was found - measure every 10 minutes
//    if (IsThresholdFound())         // todo - put back if needed
//        nMin = 10;
//#ifdef DebugMode
//    if ((time_in_minutes % 15) == 0)
//    #else
    if ((time_in_minutes % nMin) == 0)
//#endif DebugMode
        return TRUE;
    return FALSE;
}

BYTE IsTimeToConnectSatellite()
{
    BYTE n;

    n = eLoggerID[0] / 60;
    // if from any reason the num is abnormal - set it to 2
    if (n > 4)
        n = 2;

    if (readClockBuf[4] == eStartConnectionH)   //strtHour)
    {
        //check minute value for startup
        if (readClockBuf[5] == (CONNECTING_TIME + n))
        {
            waitingTask = SUB_TASK_MODEM_POST_DATA;
            toDoList = DO_DATA;
            stlRegStatus = INITIAL_STATE;
            strRegFailCnt = 0;
//            nSBDFailureCntr = 0;
            #ifdef DebugMode
            SendDebugMsg("\r\nnFailureCntr=0");
            #endif DebugMode
            nFailureCntr = 0;
            return TRUE;
        }
        #ifdef DebugMode
        SendDebugMsg("\r\nIsTimeToConnectSatellite");
        SendDebugMsg("\r\nnFailureCntr=");
        putchar1(nFailureCntr);
        SendDebugMsg("\r\nflagSucOrFail=");
        putchar1(flagSucOrFail);
        #endif DebugMode
        if ((flagSucOrFail == FAILURE) && (readClockBuf[5] == (RETRY_CONNECTIN_TIME * nFailureCntr)))
            return TRUE;
    }
    return FALSE;
}

BYTE IsTimeToConnectGPRS()
{
    BYTE i, t, nextH, CyclesAday, n;
    iFirstConnToday = FALSE;

    if (bModemType == MODEM_SATELLITE)
        return IsTimeToConnectSatellite();

    //if recognize bluetooth connect - no need to try to connect by modem
    #ifdef BlueToothOption
        if (IsBlueToothConnect())
            return FALSE;
    #endif BlueToothOption
    CyclesAday = eConnectionInDay > 0 ? eConnectionInDay : 1;
    n = eLoggerID[0] / 60;
    // if from any reason the num is abnormal - set it to 2
    if (n > 4)
        n = 2;
    //calculate the next hour for wakeup:
    for (i = 0; i < CyclesAday; i++)
    {
        //nextH = (int)startH + ((int)CycleInterval * i);
        t = i * eConnectIntervalH;
        nextH = eStartConnectionH + t;
        //for debug:Store_rx_buff[i+6] = nextH;
        // if hour now is hour of uploading
        if (readClockBuf[4] == nextH)
        {
            if (i == 0) // if its first connection for today
                iFirstConnToday = TRUE;
            //check minute value for startup
            if (readClockBuf[5] == (CONNECTING_TIME + n))
            {
//                waitingTask = SUB_TASK_MODEM_POST_DATA;
                if (i == 0)
                    toDoList = DO_DATA_N_PRMS;
                else
                    toDoList = DO_DATA;
                nFailureCntr = 0;
                return TRUE;
            }
            // if last time connecting failed, and RETRY_CONNECTIN_TIME passed since than - connect now
            if ((bConnectOK == FALSE) && (readClockBuf[5] == RETRY_CONNECTIN_TIME))
            {
                // IF NEEDED TO DO BOTH MISSIONS DATA N PARAMS
                if (toDoList == DO_DATA_N_PRMS)
                {
                    //if one of the missions succeeded - do only the one that hasn't (in any other case do the original mission)
                    if ((dataSentOK == TRUE) || (prmSentOK == TRUE))
                    {
                        if (dataSentOK == FALSE)
                            toDoList = DO_DATA;
                        if (prmSentOK == FALSE)
                            toDoList = DO_PARAMS;
                    }
                }
                return TRUE;
            }
        }
    }
    return FALSE;
}

void SetUART1BaudRate(BYTE rate)
{
    if (rate == RATE9600)
        UBRR1L = 0x2F;
    if (rate == RATE38400)
        UBRR1L = 0x0B;
}

void InitPeripherals()
{
    // USART0 initialization
    // Communication Parameters: 8 Data, 1 Stop, No Parity
    // USART0 Receiver: On
    // USART0 Transmitter: On
    // USART0 Mode: Asynchronous
    // USART0 Baud Rate: 19200
    UCSR0A=(0<<RXC0) | (0<<TXC0) | (0<<UDRE0) | (0<<FE0) | (0<<DOR0) | (0<<UPE0) | (0<<U2X0) | (0<<MPCM0);
    DISABLE_UART0();
    UCSR0C=(0<<UMSEL01) | (0<<UMSEL00) | (0<<UPM01) | (0<<UPM00) | (0<<USBS0) | (1<<UCSZ01) | (1<<UCSZ00) | (0<<UCPOL0);
    UBRR0H=0x00;
    UBRR0L=0x0B;

    // USART1 initialization
    // Communication Parameters: 8 Data, 1 Stop, No Parity
    // USART1 Receiver: On
    // USART1 Transmitter: On
    // USART1 Mode: Asynchronous
    // USART1 Baud Rate: 19200
    UCSR1A=(0<<RXC1) | (0<<TXC1) | (0<<UDRE1) | (0<<FE1) | (0<<DOR1) | (0<<UPE1) | (1<<U2X1) | (0<<MPCM1);
//    DISABLE_UART1();
    UCSR1C=(0<<UMSEL11) | (0<<UMSEL10) | (0<<UPM11) | (0<<UPM10) | (0<<USBS1) | (1<<UCSZ11) | (1<<UCSZ10) | (0<<UCPOL1);
    UBRR1H=0x00;
//    UBRR1L=0x17;      //Baud Rate: 9600
//    UBRR1L=0x2F;        // 9600 x2
    SetUART1BaudRate(RATE38400);
//    UBRR1L=0x0B;        //Baudrate 38400 x2
//    UBRR1L=0x0B;       // Baud Rate: 19200

    // ADC initialization
    // ADC Clock frequency: 28.800 kHz
    // ADC Voltage Reference: 2.56V, cap. on AREF
    // ADC Auto Trigger Source: ADC Stopped
    // Digital input buffers on ADC0: On, ADC1: On, ADC2: On, ADC3: On, ADC4: On
    // ADC5: Off, ADC6: Off, ADC7: Off
    DIDR0=(1<<ADC7D) | (1<<ADC6D) | (1<<ADC5D) | (1<<ADC4D) | (1<<ADC3D) | (1<<ADC2D) | (0<<ADC1D) | (1<<ADC0D);
    ADMUX = ADC_VREF_TYPE;
    ADCSRA=(0<<ADEN) | (0<<ADSC) | (0<<ADATE) | (0<<ADIF) | (0<<ADIE) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
    ADCSRB=(0<<ADTS2) | (0<<ADTS1) | (0<<ADTS0);

    // SPI initialization
    // SPI disabled
    SPCR=(0<<SPIE) | (0<<SPE) | (0<<DORD) | (0<<MSTR) | (0<<CPOL) | (0<<CPHA) | (0<<SPR1) | (0<<SPR0);

    // TWI initialization
    // TWI disabled
    TWCR=(0<<TWEA) | (0<<TWSTA) | (0<<TWSTO) | (0<<TWEN) | (0<<TWIE);
}

void WakeUpProcedure(void)
{
    BYTE i;
    BYTE prevBtrStatus;

    InitPeripherals();
    #ifdef DebugOnUart0
    ENABLE_UART0();
    #endif DebugOnUart0

//    TCNT1H=0x00;
//    TCNT1L=0x00;
//    OCR1AH=0x01;
//    OCR1AL=0x68;
//    nextCompare = 0x168;
//    ENABLE_TIMER1_COMPA();
    ENABLE_TIMER1_COMPA();
    //delay_ms(500);
    ENABLE_UART1();
    #ifdef DebugMode
    SendDebugMsg("\r\nWakeup\0");
    #endif DebugMode
    bEndOfMeasureTask = FALSE;
    bEndOfModemTask = FALSE;
    bEndOfMonitorTask = FALSE;
    bEndOfGPSTask = FALSE;
    //init led status

	//set condition for rtc communication
    SPCR=0x00; //reset spi control register
    if(IsPowerFlagOn()) //check if clock power flag is on
    {
        delay_ms(500);
//        #ifdef DebugMode
//        SendDebugMsg("InitRTC\r\n\0");
//        #endif DebugMode
        InitRTC();    //initiate the clock
        SetRtc24Hour(); //config rtc to am-pm mode
    }

    MeasureBatt();
    prevBtrStatus = btrStatus;

/*    if (iVoltage <= BTR_WIRELESS_LIMIT)
        btrStatus = BTR_EMPTY;
    else
        if (iVoltage > BTR_CNCT_LIMIT)
            btrStatus = BTR_FULL;
        else
            btrStatus = BTR_HALF;
*/
    if (iVoltage > (BTR_CNCT_LIMIT + BTR_CHARGE_STRIP))
            btrStatus = BTR_FULL;
    else
        if (iVoltage > BTR_CNCT_LIMIT)
        {
            if (prevBtrStatus == BTR_FULL)
                btrStatus = BTR_FULL;
            else
                btrStatus = BTR_HALF;
        }
        else
            if (iVoltage <= BTR_WIRELESS_LIMIT)
                btrStatus = BTR_EMPTY;
            else
            {
                btrStatus = BTR_HALF;
                if ((iVoltage < (BTR_WIRELESS_LIMIT + BTR_CHARGE_STRIP)) && (prevBtrStatus == BTR_EMPTY))
                    btrStatus = BTR_EMPTY;
            }

    if ((btrStatus == BTR_EMPTY) && (prevBtrStatus != btrStatus))  // if now battery is empty but wasnt before
    {
        WIRELESS_CTS_DISABLE();
//        WIRELESS_CTS_ON();
        WIRELESS_PWR_DISABLE();    // switch off wireless
        #ifdef DebugMode
        SendDebugMsg("\r\nbattery too low. disable wireless\0");
        #endif DebugMode
    }
    else
    {
        if ((btrStatus != BTR_EMPTY) && (prevBtrStatus == BTR_EMPTY))     // if battery was empty but no more
        {
            WIRELESS_CTS_ENABLE();
            WIRELESS_CTS_OFF();
            WIRELESS_PWR_ENABLE();                                      // switch on wireless
            #ifdef DebugMode
            SendDebugMsg("\r\nbattery back normal. enable wireless\0");
            #endif DebugMode
        }
    }
    mainTask = TASK_SLEEP;
//    if (!btrStatus)
//    {
        if (bExtReset == FALSE)
        {
            if ((IsTimeToMeasure()) && (btrStatus != BTR_EMPTY))
            {
                mainTask = TASK_MEASURE;
                msrCurTask = TASK_NONE;
//                objToMsr = SENSOR1;
                #ifdef DebugMode
                SendDebugMsg("\r\nTime to measure!\0");
                #endif DebugMode
            }
            else
                if ((IsTimeToConnectGPRS()) && (btrStatus == BTR_FULL))
                {
                    InitVarsForConnecting();
                    #ifdef DebugMode
                    SendDebugMsg("\r\nTime to connect!\0");
                    #endif DebugMode
                }
                else
                    if (timeout4Cnct > 0)
                    {
                        timeout4Cnct--;
                        #ifdef DebugMode
                        SendDebugMsg("\r\ntimeout4Cnct: \0");
                        putchar1(timeout4Cnct);
                        #endif DebugMode
                        if ((timeout4Cnct == 0) && (btrStatus == BTR_FULL))
                        {
                            bExtReset = TRUE;
                            InitVarsForConnecting();
                            #ifdef DebugMode
                            SendDebugMsg("\r\nconnect after reset connection failed!\0");
                            #endif DebugMode
                        }
                    }
        }
        else
        {
            mainTask = TASK_MONITOR;
            monitorCurTask = TASK_MONITOR_CONNECT;
//            mainTask = TASK_MEASURE;
//            msrCurTask = TASK_NONE;
//            delay_ms(1000);
//            objToMsr = SENSOR1;
        }
//   }

    bWaitForModemAnswer = FALSE;
    bWaitForMonitorCmd = FALSE;
    bCheckRxBuf = FALSE;
    bNeedToWait4Answer = TRUE;
    nTimeCnt = 0;

    if (mainTask != TASK_SLEEP)
    {
//        DISABLE_TAG_INT();
        #pragma optsize-
        #asm("wdr")
        WATCHDOG_ENABLE_STEP1();
        WATCHDOG_ENABLE_STEP2();
        #ifdef _OPTIMIZE_SIZE_
        #pragma optsize+
        #endif
    }
}

void WDT_off(void)
{
    //__disable_interrupt();
	#asm ("wdr"); 		//reset the watchdog
    /* Clear WDRF in MCUSR */
    MCUSR &= ~(1<<WDRF);
    /* Write logical one to WDCE and WDE */
    /* Keep old prescaler setting to prevent unintentional time-out */
    WDTCSR |= (1<<WDCE) | (1<<WDE);
    /* Turn off WDT */
    WDTCSR = 0x00;
    //__enable_interrupt();
}

BYTE IsLedWorking()
{
    //if ((BlinkNum[LED_1] > 0) || (BlinkNum[LED_2] > 0) || (BlinkNum[LED_3] > 0))
    //    return TRUE;
    if ((LedStatus[LED_1] == LED_ON) && (LedStatus[LED_2] == LED_ON) && (LedStatus[LED_3] == LED_ON))
        return TRUE;
    return FALSE;
}

void PowerDownSleep( void )
{
//    char n = 0;

    #ifdef DebugMode
    SendDebugMsg("\r\nGo2Sleep\0");
    #endif DebugMode
//    if (nTimeCnt > 0)// && (bExtReset == TRUE))
//        return;
    bExtReset = FALSE;
    WDT_off();
//    TurnOnLed(LED_3, LED_ON);
    // wait till leds finish complete 30 sec of on together
    if (IsLedWorking() == TRUE)// && (bExtReset == TRUE))// && (n < 50))
    {
//        nTimeCnt = 10 * 10;
//        return;
        delay_ms(30000);
//        n++;
    }
    TurnOffLed();
    //Direction - 0 = Input, 1= Output
    //State - if direction is Input:
    //                  1 = enable Pull up
    //                  0 = disable Pull up, become tri state
    // Input/Output Ports initialization for sleep
    // Port A initialization
    // Function: Bit7=Out Bit6=Out Bit5=Out Bit4=In Bit3=In Bit2=In Bit1=In Bit0=In
    DDRA=(1<<DDA7) | (1<<DDA6) | (1<<DDA5) | (0<<DDA4) | (0<<DDA3) | (0<<DDA2) | (0<<DDA1) | (0<<DDA0);
    // State: Bit7=0 Bit6=0 Bit5=0 Bit4=T Bit3=T Bit2=T Bit1=T Bit0=T
    PORTA=(0<<PORTA7) | (0<<PORTA6) | (0<<PORTA5) | (0<<PORTA4) | (0<<PORTA3) | (0<<PORTA2) | (0<<PORTA1) | (0<<PORTA0);

    // Port B initialization
    // Function: Bit7=Out Bit6=Out Bit5=Out Bit4=In Bit3=In Bit2=In Bit1=Out Bit0=In
    //DDRB=(1<<DDB7) | (1<<DDB6) | (1<<DDB5) | (0<<DDB4) | (0<<DDB3) |  (1<<DDB1) | (0<<DDB0);
//    DDRB &= ~( (1<<DDB4));
//    DDRB |= (1<<DDB7) | (1<<DDB6) | (1<<DDB5) |  (0<<DDB1) | (1<<DDB3) | (0<<DDB0);
//    DDRB = ((1<<DDB7) | (1<<DDB6) | (1<<DDB5) | (0<<DDB4) | (1<<DDB3) | (0<<DDB2) |(0<<DDB1)  | (0<<DDB0)); // mark on 19/6/16
    DDRB = ((0<<DDB7) | (0<<DDB6) | (0<<DDB5) | (0<<DDB4) | (0<<DDB3) | (0<<DDB2) |(1<<DDB1)  | (1<<DDB0));
    // State: Bit7=0 Bit6=0 Bit5=0 Bit4=P Bit3=T Bit2=T Bit1=0 Bit0=T
//    PORTB = ((0<<PORTB7) | (0<<PORTB6) | (0<<PORTB5) | (1<<PORTB4) | (0<<PORTB3)  | (1<<PORTB2) | (1<<PORTB1) | (1<<PORTB0)); // mark on 19/6/16
    PORTB = ((0<<PORTB7) | (0<<PORTB6) | (0<<PORTB5) | (0<<PORTB4) | (0<<PORTB3)  | (1<<PORTB2) | (0<<PORTB1) | (0<<PORTB0));

    // Port C initialization
    // Function: Bit7=Out Bit6=Out Bit5=Out Bit4=Out Bit3=Out Bit2=Out Bit1=Out Bit0=Out
//    DDRC=(0<<DDC7) | (1<<DDC6) | (1<<DDC5) | (1<<DDC4) | (1<<DDC3) | (1<<DDC2) | (1<<DDC1) | (1<<DDC0);    // mark on 19/6/16
//    DDRC=(1<<DDC7) | (1<<DDC6) | (1<<DDC5) | (1<<DDC4) | (1<<DDC3) | (1<<DDC2) | (1<<DDC1) | (1<<DDC0);
    // State: Bit7=0 Bit6=0 Bit5=0 Bit4=0 Bit3=0 Bit2=0 Bit1=1 Bit0=1
    DDRC=(0<<DDC7) | (1<<DDC6) | (1<<DDC5) | (1<<DDC4) | (0<<DDC3) | (1<<DDC2) | (1<<DDC1) | (1<<DDC0);
    PORTC=(1<<PORTC7) | (0<<PORTC6) | (0<<PORTC5) | (0<<PORTC4) | (1<<PORTC3) | (0<<PORTC2) | (1<<PORTC1) | (1<<PORTC0);

    // Port D initialization
    // Function: Bit7=Out Bit6=Out Bit5=Out Bit4=Out Bit3=In Bit2=In Bit1=Out Bit0=In
    if (btrStatus == BTR_EMPTY)
    {
        DDRD=(0<<DDD7) | (1<<DDD6) | (1<<DDD5) | (1<<DDD4) | (1<<DDD3) | (0<<DDD2) | (1<<DDD1) | (0<<DDD0);
    // State: Bit7=0 Bit6=0 Bit5=0 Bit4=0 Bit3=0 Bit2=T Bit1=0 Bit0=T
        PORTD=(0<<PORTD7) | (0<<PORTD6) | (1<<PORTD5) | (1<<PORTD4) | (0<<PORTD3) | (0<<PORTD2) | (0<<PORTD1) | (0<<PORTD0);  //(0<<PORTD0);
    }
    else
    {
        DDRD=(1<<DDD7) | (1<<DDD6) | (1<<DDD5) | (1<<DDD4) | (1<<DDD3) | (0<<DDD2) | (1<<DDD1) | (0<<DDD0);
        PORTD=(1<<PORTD7) | (1<<PORTD6) | (1<<PORTD5) | (1<<PORTD4) | (0<<PORTD3) | (0<<PORTD2) | (0<<PORTD1) | (0<<PORTD0);  //(0<<PORTD0);
    }
    DISABLE_UART1();
//    ENABLE_TAG_INT();
    DISABLE_TIMER1_COMPA();
    EIFR |= (1<<INTF2); //reset clock flag interrupt
    DisableClockIntr();
    ResetClockIntr();//(0);

    SMCR |= 4;
    SMCR |= 1;
    PRR = 0xFF;
    ENABLE_CLOCK_INT(); // enable external interrupt2

    #asm
    sleep
    #endasm

    #asm ("nop\nop"); 		//wakeup from sleep mode
}

void TurnOnLed(BYTE led, BYTE type)
{
    if (bExtReset)
    {
        if (led ==  LED_1)
            PORTA.5 = 1;
        if (led ==  LED_2)
            PORTA.6 = 1;
        if (led ==  LED_3)
            PORTA.7 = 1;
//        if (type == LED_ON)
            LedStatus[led] = type;//LED_ON;
//        else
//        {
//            LedStatus[led] = LED_BLINK;
//            BlinkNum[led] = 19;
//        }
    }
//    else
//    {
//        LedStatus[led] = LED_BLINK;
//        if (type == SUCCESS)
//            BlinkNum[led] = 1;
//        else
//            BlinkNum[led] = 5;
//    }
}

void TurnOffLed()
{
    int i;
    PORTA.5 = 0;
    PORTA.6 = 0;
    PORTA.7 = 0;
    for (i = 1; i <= 3; i++)
    {
        LedStatus[i] = LED_OFF;
    }
}

//the function recieve pointer to buf
//buf[0]= lo_byte, buf[1]=  hi_byte
//the function return int (address) combined from 2 bytes
// LITTLE ENDIAN
int bytes2int(char* buf)
{
	//set 2 bytes into union
	union_b2i.bval[0] = buf[0];
	union_b2i.bval[1] = buf[1];

	return union_b2i.ival;
}

/*// BIG ENDIAN
int bytes2intBigEnd(char* buf)
{
	//set 2 bytes into union
	union_b2i.bval[0] = buf[1];
	union_b2i.bval[1] = buf[0];

	return union_b2i.ival;
}
*/
//the function recieve int (address) and pointer to buf
//the function set into buf[0] the hi_address_byte
//and into buf[1] the lo_address_byte

void int2bytes(int int_pointer, char* buf)
{
	union_b2i.ival = int_pointer;
	//set 2 bytes into buf
	buf[0] = union_b2i.bval[0];
	buf[1] = union_b2i.bval[1];
}

// #monitor
//the function recieve pointer to buf
//buf[0]=hi_byte, buf[1]=lo_byte
//the function return int (address) combined from 2 bytes
long Bytes2Long(char* buf)
{
	//set 4 bytes into union
	lb.bVal[0] = buf[0];
	lb.bVal[1] = buf[1];
	lb.bVal[2] = buf[2];
	lb.bVal[3] = buf[3];

	return lb.lVal;
}

//the function recieve int (address) and pointer to buf
//the function set into buf[0] the hi_address_byte
//and into buf[1] the lo_address_byte
void Long2Bytes(long l, char* buf)
{
	lb.lVal = l;
	//set 2 bytes into buf
	buf[0] = lb.bVal[0];
	buf[1] = lb.bVal[1];
	buf[2] = lb.bVal[2];
	buf[3] = lb.bVal[3];
}

//the function recieve float (address) and pointer to buf
//the function set into buf[0] the hi_address_byte
//and into buf[1] the lo_address_byte
void Float2Bytes(float f, char* buf)
{
	fb.fVal = f;
	//set 4 bytes into buf
	buf[0] = fb.bVal[0];
	buf[1] = fb.bVal[1];
	buf[2] = fb.bVal[2];
	buf[3] = fb.bVal[3];
}


float Bytes2Float(char* buf)
{
	//set 4 bytes into buf
	fb.bVal[0] = buf[0];
	fb.bVal[1] = buf[1];
	fb.bVal[2] = buf[2];
	fb.bVal[3] = buf[3];
    return fb.fVal;
}

//copy from cpu e2 into buf
void cpu_e2_to_MemCopy( BYTE* to, char eeprom* from, BYTE length)
{
	BYTE i;
	for(i = 0; i < length; i++)
		to[i] = from[i];
}

//copy from ram buf into cpu e2
void MemCopy_to_cpu_e2( char eeprom* to, BYTE* from, BYTE length)
{

	BYTE i;
	for(i = 0; i < length; i++)
		to[i] = from[i];
}

void MemCopy( BYTE* to, BYTE* from, BYTE length)
{
	BYTE i;
	for(i = 0; i < length; i++)
		to[i] = from[i];
}

//copy from cpu flash into buf
void cpu_flash_to_MemCopy( BYTE* to, char flash* from, BYTE length)
{
	BYTE i;
	for(i = 0; i < length; i++)
		to[i] = from[i];
}

//copy flash buf to buffer. return num of  copy bytes
BYTE CopyFlashToBuf( BYTE* to, char flash* from)
{
    BYTE index = 0;
    while (from[index] != '@')
    {
        to[index] = from[index];                      //"425#"
        index++;
    }
    return index;
}

//checksum with parameter
// 0 = check_sum ^
// 1 = check_sum +
BYTE CheckSum( BYTE *buff, BYTE length, BYTE param )
{
    BYTE check_sum;

    check_sum = 0;
    while (length--)
    {
        //filter the "246 (F6)" for gprs modem ??
        if(param)
        {
            check_sum += *buff++;
        }
        else
            check_sum ^= *buff++;
    }
    return (check_sum);
}

int GetCheckSum( BYTE *buff, int length)
{
    int check_sum;

    check_sum = 0;
    while (length--)
    {
        check_sum += *buff++;
    }

    return (check_sum);
}

void InitProgram(void)
{
    unsigned char  data_not_valid = 0;
    char l[4];
    char i;
    // Crystal Oscillator division factor: 1
    #pragma optsize-
    CLKPR=(1<<CLKPCE);
    CLKPR=(0<<CLKPCE) | (0<<CLKPS3) | (0<<CLKPS2) | (0<<CLKPS1) | (0<<CLKPS0);
    #ifdef _OPTIMIZE_SIZE_
    #pragma optsize+
    #endif

    //Direction - 0 = Input, 1= Output
    //State - if direction is Input:
    //                  1 = enable Pull up
    //                  0 = disable Pull up, become tri state
    // Input/Output Ports initialization
    // Port A initialization
    // Function: Bit7=Out Bit6=Out Bit5=Out Bit4=In Bit3=In Bit2=In Bit1=In Bit0=In
    DDRA=(1<<DDA7) | (1<<DDA6) | (1<<DDA5) | (0<<DDA4) | (0<<DDA3) | (0<<DDA2) | (0<<DDA1) | (0<<DDA0);
    // State: Bit7=0 Bit6=0 Bit5=0 Bit4=T Bit3=T Bit2=T Bit1=T Bit0=T
    PORTA=(0<<PORTA7) | (0<<PORTA6) | (0<<PORTA5) | (0<<PORTA4) | (0<<PORTA3) | (0<<PORTA2) | (0<<PORTA1) | (0<<PORTA0);

    // Port B initialization
    // Function: Bit7=In Bit6=In Bit5=In Bit4=In Bit3=In Bit2=In Bit1=out Bit0=Out
    DDRB=(0<<DDB7) | (0<<DDB6) | (0<<DDB5) | (0<<DDB4) | (0<<DDB3) | (0<<DDB2) | (1<<DDB1) | (1<<DDB0);
    // State: Bit7=T Bit6=T Bit5=T Bit4=T Bit3=T Bit2=T Bit1=T Bit0=T
    PORTB=(0<<PORTB7) | (0<<PORTB6) | (0<<PORTB5) | (0<<PORTB4) | (0<<PORTB3) | (0<<PORTB2) | (0<<PORTB1) | (0<<PORTB0);

    // Port C initialization
    // Function: Bit7=In Bit6=Out Bit5=Out Bit4=Out Bit3=Out Bit2=Out Bit1=In Bit0=In
    DDRC=(0<<DDC7) | (1<<DDC6) | (1<<DDC5) | (1<<DDC4) | (0<<DDC3) | (1<<DDC2) | (0<<DDC1) | (0<<DDC0);
    // State: Bit7=0 Bit6=0 Bit5=0 Bit4=0 Bit3=0 Bit2=0 Bit1=T Bit0=T
    PORTC=(1<<PORTC7) | (0<<PORTC6) | (0<<PORTC5) | (0<<PORTC4) | (1<<PORTC3) | (0<<PORTC2) | (0<<PORTC1) | (0<<PORTC0); //c4=0      c6=1   c3=1

    // Port D initialization
    // Function: Bit7=Out Bit6=Out Bit5=Out Bit4=Out Bit3=In Bit2=In Bit1=In Bit0=In
    DDRD=(1<<DDD7) | (1<<DDD6) | (1<<DDD5) | (1<<DDD4) | (1<<DDD3) | (0<<DDD2) | (1<<DDD1) | (0<<DDD0);
    // State: Bit7=0 Bit6=0 Bit5=0 Bit4=0 Bit3=T Bit2=T Bit1=T Bit0=T
    PORTD=(1<<PORTD7) | (0<<PORTD6) | (1<<PORTD5) | (1<<PORTD4) | (1<<PORTD3) | (0<<PORTD2) | (1<<PORTD1) | (0<<PORTD0);

    WIRELESS_PWR_ENABLE();
    // Timer/Counter 0 initialization
    // Clock source: System Clock
    // Clock value: 3.600 kHz
    // Mode: Normal top=0xFF
    // OC0A output: Disconnected
    // OC0B output: Disconnected
    // Timer Period: 71.111 ms
    TCCR0A=(0<<COM0A1) | (0<<COM0A0) | (0<<COM0B1) | (0<<COM0B0) | (0<<WGM01) | (0<<WGM00);
    DISABLE_TIMER0();
    //TCCR0B=(0<<WGM02) | (0<<CS02) | (0<<CS01) | (0<<CS00);
    TCNT0=0x00;
    OCR0A=0x00;
    OCR0B=0x00;
    /*
    // Timer/Counter 1 initialization
    // Clock source: System Clock
    // Clock value: 3.600 kHz
    // Mode: Normal top=0xFFFF
    // OC1A output: Disconnected
    // OC1B output: Disconnected
    // Noise Canceler: Off
    // Input Capture on Falling Edge
    // Timer Period: 18.204 s
    // Timer1 Overflow Interrupt: Off
    // Input Capture Interrupt: Off
    // Compare A Match Interrupt: On
    // Compare B Match Interrupt: Off
    TCCR1A=(0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (0<<WGM10);
    TCCR1B=(0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (0<<WGM12) | (1<<CS12) | (0<<CS11) | (1<<CS10);
    TCNT1H=0x00;
    TCNT1L=0x00;
    ICR1H=0x00;
    ICR1L=0x00;
    OCR1AH=0x01;
    OCR1AL=0x68;
    OCR1BH=0x00;
    OCR1BL=0x00;
    */
    // Timer/Counter 1 initialization
    // Clock source: System Clock
    // Clock value: 460.800 kHz
    // Mode: CTC top=OCR1A
    // OC1A output: Disconnected
    // OC1B output: Disconnected
    // Noise Canceler: Off
    // Input Capture on Falling Edge
    // Timer Period: 0.1 s
    // Timer1 Overflow Interrupt: Off
    // Input Capture Interrupt: Off
    // Compare A Match Interrupt: On
    // Compare B Match Interrupt: Off
    TCCR1A=(0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (0<<WGM10);
    TCCR1B=(0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (1<<WGM12) | (0<<CS12) | (1<<CS11) | (0<<CS10);
    TCNT1H=0x4C;
    TCNT1L=0x00;
    ICR1H=0x00;
    ICR1L=0x00;
    OCR1AH=0xB3;
    OCR1AL=0xFF;
    OCR1BH=0x00;
    OCR1BL=0x00;


    // Timer/Counter 2 initialization
    // Clock source: System Clock
    // Clock value: 3.600 kHz
    // Mode: Normal top=0xFF
    // OC2A output: Disconnected
    // OC2B output: Disconnected
    // Timer Period: 71.111 ms
    ASSR=(0<<EXCLK) | (0<<AS2);
    TCCR2A=(0<<COM2A1) | (0<<COM2A0) | (0<<COM2B1) | (0<<COM2B0) | (0<<WGM21) | (0<<WGM20);
    //TCCR2B=(0<<WGM22) | (0<<CS22) | (0<<CS21) | (0<<CS20);
    DISABLE_TIMER2();
    TCNT2=0x00;
    OCR2A=0x00;
    OCR2B=0x00;


    // Timer/Counter 0 Interrupt(s) initialization
    TIMSK0=(0<<OCIE0B) | (0<<OCIE0A) | (1<<TOIE0);

   // Timer/Counter 1 Interrupt(s) initialization
    TIMSK1=(0<<ICIE1) | (0<<OCIE1B) | (1<<OCIE1A) | (0<<TOIE1);

    // Timer/Counter 2 Interrupt(s) initialization
    TIMSK2=(0<<OCIE2B) | (0<<OCIE2A) | (1<<TOIE2);

    // External Interrupt(s) initialization
    // INT0: Off
    // INT1: Off
    // INT2: On
    // INT2 Mode: Falling Edge
    // Interrupt on any change on pins PCINT0-7: On
    // Interrupt on any change on pins PCINT8-15: Off
    // Interrupt on any change on pins PCINT16-23: Off
    // Interrupt on any change on pins PCINT24-31: Off
    EICRA=(1<<ISC21) | (0<<ISC20) | (0<<ISC11) | (0<<ISC10) | (0<<ISC01) | (0<<ISC00);
    EIMSK=(0<<INT2) | (0<<INT1) | (0<<INT0);
    EIFR=(1<<INTF2) | (0<<INTF1) | (0<<INTF0);
//    PCMSK1=(0<<PCINT15) | (0<<PCINT14) | (0<<PCINT13) | (1<<PCINT12) | (0<<PCINT11) | (0<<PCINT10) | (0<<PCINT9) | (0<<PCINT8);
//    PCICR=(0<<PCIE3) | (0<<PCIE2) | (1<<PCIE1) | (0<<PCIE0);
//    PCMSK0=(0<<PCINT7) | (0<<PCINT6) | (0<<PCINT5) | (0<<PCINT4) | (0<<PCINT3) | (1<<PCINT2) | (0<<PCINT1) | (0<<PCINT0);
    PCICR=(0<<PCIE3) | (0<<PCIE2) | (0<<PCIE1) | (0<<PCIE0);
//    PCIFR=(0<<PCIF3) | (0<<PCIF2) | (0<<PCIF1) | (1<<PCIF0);


    InitPeripherals();
    // Analog Comparator initialization
    // Analog Comparator: Off
    ACSR=(1<<ACD) | (0<<ACBG) | (0<<ACO) | (0<<ACI) | (0<<ACIE) | (0<<ACIC) | (0<<ACIS1) | (0<<ACIS0);
    ADCSRB=(0<<ACME);
    DIDR1=0x00;
    // Watchdog Timer initialization
    // Watchdog Timer Prescaler: OSC/1024k
    // Watchdog Timer interrupt: On
    MCUSR |= (1<<WDRF);
#pragma optsize-
    #asm("wdr")
    WATCHDOG_ENABLE_STEP1();
    WATCHDOG_ENABLE_STEP2();
#ifdef _OPTIMIZE_SIZE_
#pragma optsize+
#endif
    for (i = 1; i <= 3; i++)
    {
        LedStatus[i] = LED_OFF;
    }

    TurnOnLed(LED_1, LED_ON);
    #ifdef DebugMode
    ENABLE_UART1();
    SendDebugMsg("\r\nexternal reset!\0");
    SendDebugMsg("\r\nNumSensors =\0");
    putchar1(eNumSensors);
    #endif DebugMode

    	// if sensors data pointers validitation faild
	//pSens_ext_e2 = sens1_control_param;
    for (objToMsr = SENSOR1; objToMsr <= eNumSensors; objToMsr++)
    {
//        #ifdef DebugMode
//            SendDebugMsg("\r\nTest sensor \0");
//            putchar1(objToMsr + 0x30);
//            #endif DebugMode
//        SetCotrolParamAddress();
//        Test1(x);
        if (PointersValidate() == FALSE)
        {
            data_not_valid = 1;
            #ifdef DebugMode
            SendDebugMsg("\r\npointers not valid for sensor \0");
            putchar1(objToMsr + 0x30);
//            data_not_valid = 0;
            #endif DebugMode
            break;
        }
    }

	if ((data_not_valid) || (eNumSensors == 0))
	{
        //run the sensor initiation function
        InitDataBlocks();
        InitSensorArray();
        eNumSensors = 0;
	}

    mainTask = TASK_WAKEUP;
    // if modem is on from any reason - first of all turn it off
    if (IsModemOn() == TRUE)
    {
        mainTask = TASK_MODEM;
        modemCurTask = TASK_MODEM_CLOSE;
        modemCurSubTask = SUB_TASK_MODEM_CLOSE_MDM;
        ModemResponse = TASK_COMPLETE;
        modemOnStartUp = TRUE;
    }
    bMonitorConnected = FALSE;
    for (objToMsr = SENSOR1; objToMsr < MAX_SEN_NUM; objToMsr++)
    {
        Alerts[objToMsr].Status = ALERT_WAIT;
        Alerts[objToMsr].OutOfLmtCnt = 0;
        Alerts[objToMsr].MsrCnt = 0;
    }
    for (objToMsr = SENSOR1; objToMsr < MAX_WL_SEN_NUM; objToMsr++)
        iLastMsr[objToMsr] = MinInt;

    // treat battery as first sensor.
    for (i = 0; i < 4; i++)
        l[i] = eLoggerID[i];

    WLSenArr[BTR_INDEX].Id = Bytes2Long(l);
    WLSenArr[BTR_INDEX].Type = TYPE_BTR;    // meanwhile put it to b oxygen cos its measure unit is mV.

    btrStatus = BTR_FULL;
    bModemType = MODEM_GSM;
    #ifdef DebugOnUart0
    ENABLE_UART0();
    #endif DebugOnUart0
}

BYTE IsConnectingTimeClose()
{
    BYTE i, t, nextH, CyclesAday;
    // if its after connecting minute (must be less than 10 min of round hour)
    if (readClockBuf[5] >= 10)
        return FALSE;
    CyclesAday = eConnectionInDay > 0 ? eConnectionInDay : 1;
    //calculate the next hour for connecting:
    for (i = 0; i < CyclesAday; i++)
    {
        //nextH = (int)startH + ((int)CycleInterval * i);
        t = i * eConnectIntervalH;
        nextH = eStartConnectionH + t;
        //for debug:Store_rx_buff[i+6] = nextH;
        // if hour now is hour of uploading
        if (readClockBuf[4] == nextH)
            return TRUE;
    }
    return FALSE;
}

void UART1Select(BYTE uartTarget)
{
    unsigned char curState = PORTD;
    unsigned char newState, tmp;
    switch (uartTarget)
    {
    case UART_RADIO_UHF:
        newState = 0x0;
    break;
    case UART_GPS:
        newState = 0x10;
    break;
    case UART_DBG:
        newState = 0x20;
    break;
    case UART_NONE:
        newState = 0x30;
    break;
    }
    tmp = curState & 0x30;
    if (tmp == newState)
        return;
    curState = curState & ~0x30;
    newState = curState | newState;
    PORTD = newState;
}

/*
BYTE IsLeapYear(int year)
{
    if ((year % 4) != 0)
        return FALSE;
    if ((year % 100) != 0)
        return TRUE;
    if ((year % 400) != 0)
        return FALSE;
    return TRUE;
}

void ConvertCurTimeToUTC(unsigned char* time)
{
    signed char y,m,d,h,n;
//    unsigned char time[5];
    char b = 0;
#ifdef DebugMode
    SendDebugMsg("\r\nnow:\0");
    putchar1(readClockBuf[0]);
    putchar1(readClockBuf[1]);
    putchar1(readClockBuf[2]);
    putchar1(readClockBuf[4]);
    putchar1(readClockBuf[5]);
    SendDebugMsg("\r\n\0");
    SendDebugMsg("\r\nmin to move 4 UTC:\0");
    putchar1(gMin4UTC);
    SendDebugMsg("\r\nhours  to move 4 UTC:\0");
    putchar1(gHr4UTC);
#endif DebugMode

    y = readClockBuf[0];
    m = readClockBuf[1];
    d = readClockBuf[2];
    h = readClockBuf[4] + gHr4UTC;
    n = readClockBuf[5] + gMin4UTC;
    if (n < 0)
    {
        putchar1('!');
        putchar1(n);
        n += 60;
        h--;
    }
    if (n >= 60)
    {
        putchar1('@');
        putchar1(n);
        n -= 60;
        h++;
    }
    if (h < 0)
    {
        putchar1('#');
        putchar1(h);
        h += 24;
        d--;
    }
    if (h >= 24)
    {
        putchar1('$');
        putchar1(h);
        h -= 24;
        d++;
    }
    if (d == 0)
    {
        putchar1('%');
        m--;
        if (m == 0)
        {
            putchar1('^');
            m = 12;
            y--;
        }
        if (IsLeapYear(y) == FALSE)
            d = days_in_month[m];
        else
            d = days_in_month_leap_year[m];
    }
    else
        if ((m != 2) || (IsLeapYear(y) == FALSE))
        {
            putchar1('&');
            if (d > days_in_month[m])
                b = 1;
        }
        else
            if (d > days_in_month_leap_year[m])
                b = 1;
    // if day is over max days in month
    if (b == 1)
    {
        putchar1('*');
        // move to next month
        d = 1;
        m++;
        // if month is over 12 move to next year
        if (m > 12)
        {
            putchar1('+');
            m = 1;
            y++;
        }
    }
    time[0] = y;
    time[1] = m;
    time[2] = d;
    time[3] = h;
    time[4] = n;
#ifdef DebugMode
    SendDebugMsg("\r\nUTC now:\0");
    putchar1(time[0]);
    putchar1(time[1]);
    putchar1(time[2]);
    putchar1(time[3]);
    putchar1(time[4]);
#endif DebugMode

//    return time;
}   */

#ifdef DebugMode
//convert int less than 1000000 to string and send string to uart1
void PrintNum(long val)
{
//#ifdef TestMode
    char s[10];
    BYTE i = 0;
    if (mainTask == TASK_MEASURE)
        return;
    if (val < 0)
    {
        putchar1('-');
        val *= -1;
    }

    do
    {
        s[i++] = (char)(val % 10);
        val = val / 10;
    }
    while (val > 0);
    for (; i > 0; i--)
        putchar1(s[i-1] + 48);
    putchar1(HASHTAG);
    putchar1('\n');
//#endif TestMode
}

void SendDebugMsg(flash unsigned char *bufToSend)
{
//#ifdef TestMode
    BYTE i;
    if (mainTask == TASK_MEASURE)
        return;
    UART1Select(UART_DBG);
    delay_ms(50);
    i = 0;
    //copy flash string to buff
    while ((bufToSend[i] != '\0') && (i < MAX_SBD_BUF_LEN))
    {
         ComBuf[i] = bufToSend[i];
         //putchar1(ComBuf[i]);
         i++;
    }
    BytesToSend = i ;
    //copy the ComBuf into eeprom (for debug)
    //MemCopy_to_cpu_e2(&Store_tx_buff[0], ComBuf, BytesToSend);
    //transmitt to local modem port
    TransmitBuf(2);
    bWaitForModemAnswer = FALSE;
//    if ((mainTask == TASK_MEASURE) && (msrCurTask != TASK_MSR_SAVE))
//    {
//        //UART_1_WIRELESS();
//        UART1Select(UART_RADIO_UHF);
//        delay_ms(200);
//    }
//#endif TestMode
}
  /*
void PrintFltNum(float val)
{
//    char s[10];
    BYTE i = 0;

    delay_ms(100);
    if (val < 0)
    {
        putchar1('-');
        val *= -1;
    }

    do
    {
        if (val < 10)
        {
            putchar1(((int)val % 10) + 0x30);
        }
        else
            if (val < 100)
            {
                putchar1((int)(val / 10) + 0x30);
            }
            else
                if (val < 1000)
                {
                    putchar1(val / 100);
                }
                else
                    if (val < 10000)
                    {
                        putchar1(val / 1000);
                    }
                    else
                        if (val < 100000)
                        {
                            putchar1(val / 10000);
                        }
        val = val / 10;
    }
    while ((int)val > 0);
//    for (; i > 0; i--)
//        putchar1(s[i-1] + 48);
    putchar1('\n');
}
*/

#endif DebugMode

void DeepSleep( void )
{
    #ifdef DebugMode
    SendDebugMsg("\r\nGo2DeepSleep\0");
    #endif DebugMode
    bExtReset = FALSE;
    // Port A initialization
    // Function: Bit7=Out Bit6=Out Bit5=Out Bit4=In Bit3=In Bit2=In Bit1=In Bit0=In
    DDRA=(1<<DDA7) | (1<<DDA6) | (1<<DDA5) | (0<<DDA4) | (0<<DDA3) | (0<<DDA2) | (0<<DDA1) | (0<<DDA0);
    // State: Bit7=0 Bit6=0 Bit5=0 Bit4=T Bit3=T Bit2=T Bit1=T Bit0=T
    PORTA=(0<<PORTA7) | (0<<PORTA6) | (0<<PORTA5) | (0<<PORTA4) | (0<<PORTA3) | (0<<PORTA2) | (0<<PORTA1) | (0<<PORTA0);

    // Port B initialization
    // Function: Bit7=Out Bit6=Out Bit5=Out Bit4=In Bit3=In Bit2=In Bit1=Out Bit0=In
    DDRB = ((0<<DDB7) | (0<<DDB6) | (0<<DDB5) | (0<<DDB4) | (0<<DDB3) | (0<<DDB2) |(1<<DDB1)  | (1<<DDB0));
    // State: Bit7=0 Bit6=0 Bit5=0 Bit4=P Bit3=T Bit2=T Bit1=0 Bit0=T
    PORTB = ((0<<PORTB7) | (0<<PORTB6) | (0<<PORTB5) | (0<<PORTB4) | (0<<PORTB3)  | (0<<PORTB2) | (0<<PORTB1) | (0<<PORTB0));

    // Port C initialization
    // Function: Bit7=Out Bit6=Out Bit5=Out Bit4=Out Bit3=Out Bit2=Out Bit1=Out Bit0=Out
    DDRC=(0<<DDC7) | (1<<DDC6) | (1<<DDC5) | (1<<DDC4) | (0<<DDC3) | (1<<DDC2) | (1<<DDC1) | (1<<DDC0);
    // State: Bit7=0 Bit6=0 Bit5=0 Bit4=0 Bit3=0 Bit2=0 Bit1=1 Bit0=1
    PORTC=(0<<PORTC7) | (0<<PORTC6) | (0<<PORTC5) | (0<<PORTC4) | (0<<PORTC3) | (0<<PORTC2) | (1<<PORTC1) | (1<<PORTC0);

    // Port D initialization
    // Function: Bit7=In Bit6=Out Bit5=Out Bit4=Out Bit3=In Bit2=In Bit1=Out Bit0=In
    DDRD=(0<<DDD7) | (1<<DDD6) | (1<<DDD5) | (1<<DDD4) | (1<<DDD3) | (0<<DDD2) | (1<<DDD1) | (0<<DDD0);
    // State: Bit7=0 Bit6=0 Bit5=0 Bit4=0 Bit3=0 Bit2=T Bit1=0 Bit0=T
    PORTD=(0<<PORTD7) | (0<<PORTD6) | (1<<PORTD5) | (1<<PORTD4) | (0<<PORTD3) | (0<<PORTD2) | (0<<PORTD1) | (0<<PORTD0);  //(0<<PORTD0);

//    TurnOffLed();
    WDT_off();

    DISABLE_UART1();
    WIRELESS_PWR_DISABLE();
//    ENABLE_TAG_INT();
    DISABLE_TIMER1_COMPA();
    EIFR |= (1<<INTF2); //reset clock flag interrupt
    DisableClockIntr();

    SMCR |= 4;
    SMCR |= 1;
    PRR = 0xFF;

    #asm
    sleep
    #endasm

    #asm ("nop\nop"); 		//wakeup from sleep mode
}

/*void SetTimer0ForReceiver()
{
    // Timer/Counter 0 initialization
    // Clock source: System Clock
    // Clock value: 14.400 kHz
    // Mode: Normal top=0xFF
    // OC0A output: Disconnected
    // OC0B output: Disconnected
    // Timer Period: 10 ms
//    TCCR0A=(0<<COM0A1) | (0<<COM0A0) | (0<<COM0B1) | (0<<COM0B0) | (0<<WGM01) | (0<<WGM00);
    TCCR0B=(0<<WGM02) | (1<<CS02) | (0<<CS01) | (0<<CS00);
    TCNT0=0x70;
//    OCR0A=0x00;
//    OCR0B=0x00;
}

void ResetTimer0()
{
    DISABLE_TIMER0();
    TCNT0=0x00;
}
*/

////////////////end of general.c////////////////