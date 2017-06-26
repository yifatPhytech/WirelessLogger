#include "define.h"

flash unsigned char IdentificationStr[] = "BEEPBEEP@";
extern eeprom unsigned char eNumSensors;
extern eeprom int eTimeZoneOffset;                         //offset (in minutes) from UTC
extern eeprom date_time_t ee_epoch_time;
BYTE requestType;
BYTE requestLen;
BYTE requestIndex;
BYTE setResult;
long lTimeFromLastTask;
extern BYTE monitorCurTask;
extern eeprom char eLoggerID[]; //sensors id
extern eeprom unsigned char eStartConnectionH;        //first connection hour
extern eeprom unsigned char eConnectionInDay;        //number on connectionsin a day
extern eeprom unsigned char eConnectIntervalH;        //intervalbetween connections (hours)
//extern eeprom char sensor_type_1[];
//extern eeprom char WakeupInterval;
extern eeprom unsigned char eUseCntrCode;
extern eeprom unsigned char eMobileNetCode[];
extern eeprom unsigned char eMobileCntryCode[];
extern eeprom unsigned char eIPorURLval1[]; //32 bytes  //amazon 2
extern eeprom unsigned char ePORTval1[];                             //4 bytes
extern eeprom unsigned char eAPN[];     //"internetm2m.air.com#000000000000";
extern flash unsigned char RomVersion[];
extern unsigned int nextCompare;
//extern int SensorResult;       //save the measuring result into variable
extern eeprom struct WirelessSensor WLSenArr[MAX_WL_SEN_NUM];
extern BYTE objToMsr;
extern int iVoltage;
extern volatile int iLastMsr[];
extern BYTE rssi_val;
extern char ComBuf[MAX_SBD_BUF_LEN];
extern char RxUart1Buf[MAX_RX1_BUF_LEN];
extern char readClockBuf[];	         //buffer for data reading from clock
//extern BYTE rx1_buff_len;
extern int buffLen;
extern int BytesToSend;
extern int TimeLeftForWaiting;
extern char clockBuf[7]; 		 //buffer for all clock operation need
extern bit bCheckRxBuf;
extern bit bWaitForMonitorCmd;
extern bit bEndOfMonitorTask;
extern BYTE bMonitorConnected;
//extern BYTE mainTask;
//extern BYTE msrCurTask;

void SendConnectString()
{
    BytesToSend = CopyFlashToBuf(ComBuf, IdentificationStr);
    delay_ms(20);
    TransmitBuf(1);
    delay_ms(10);
//    DISABLE_TIMER1();
//    TCNT1H=0x00;
//    TCNT1L=0x00;
//    OCR1AH=0x01;
//    OCR1AL=0x68;
//    nextCompare = 0x168;
//    ENABLE_TIMER1();
}

// checks if ID of sensor is the same as got from monitor:
// return value:
// 1- same ID
// 2 - id is 0
// 0 - different ID
BYTE IsSameID()
{
    if ((eLoggerID[0] == RxUart1Buf[3]) &&
        (eLoggerID[1] == RxUart1Buf[4]) &&
        (eLoggerID[2] == RxUart1Buf[5]) &&
        (eLoggerID[3] == RxUart1Buf[6]))
        return 1;
    if ((RxUart1Buf[3] == 0) &&
        (RxUart1Buf[4] == 0) &&
        (RxUart1Buf[5] == 0) &&
        (RxUart1Buf[6] == 0))
        {
            return 2;
        }
    return 0;
}

BYTE CheckMonitorRequest()
{
    BYTE i, cs;
    requestLen = 0;
    requestType = 0;
    requestIndex = 0;

    if (!((RxUart1Buf[0] == 0xff) && (RxUart1Buf[1] == 0xff)))
        return FALSE;
    // if size of request is more than bytes arrived:
    requestLen = RxUart1Buf[2];
    if (requestLen > buffLen)//rx1_buff_len)
        return FALSE;
    i = IsSameID();
    if (i == 0)
        return FALSE;
    requestType = RxUart1Buf[7];     // get or set
    requestIndex = RxUart1Buf[8];
    if (requestIndex < 10)
    {
        if (requestIndex == 0)    //0 for logger 1 and up - for the sensors
        {
            requestIndex = REQ_ID;
        }
        if (requestIndex > 0)    //0 for logger 1 and up - for the sensors
        {
            objToMsr = requestIndex;
            requestIndex = REQ_WL_SEN_ID;
        }
    }
    else
    {
        if (requestIndex < 20)
        {
            objToMsr = requestIndex - 10;
            requestIndex = REQ_MSR;
        }
        else
        {
            if (requestIndex < 30)
            {
                objToMsr = requestIndex - 20;
                requestIndex = REQ_INTRVL;
            }
            else
                if (requestIndex < 40)
                {
                    objToMsr = requestIndex - 30;
                    requestIndex = REQ_TYPE;
                }
        }
    }

    cs = CheckSum(RxUart1Buf, requestLen, 1);
    if (cs != RxUart1Buf[requestLen])
        return FALSE;
    return TRUE;
    // monitor ask for ID:
//    if ((i == 2) && (requestType == GET_REQUEST) && (requestIndex < 10))
//    {
//        return TRUE;
//    }
}

void ExecuteGetCommand()
{
//    int n;
    BYTE len = 0, cs;
    BYTE b[4];
    ComBuf[0] = 0xff;
    ComBuf[1] = 0xff;
    cpu_e2_to_MemCopy( &ComBuf[3], &eLoggerID[0], 4);
    ComBuf[7] = requestIndex;
//    #ifdef TestMonitor
//    PORTA.5 = 1;
//    delay_ms(10);
//    PORTA.5 = 0;
//    delay_ms(10);
//    #endif TestMonitor
    switch (requestIndex)
    {
        case REQ_ID:
            len = 4;
            cpu_e2_to_MemCopy( &ComBuf[8], &eLoggerID[0], 4);
        break;
        case REQ_WL_SEN_ID:
            len = 4;
            Long2Bytes(WLSenArr[objToMsr].Id, b);
            MemCopy(&ComBuf[8], b, 4);
        break;
        case REQ_MSR:
            len = 2;
            ComBuf[8] = (unsigned char)(iLastMsr[objToMsr]) ;                 //address low
            ComBuf[9] = (unsigned char)((iLastMsr[objToMsr] >> 8) & 0xFF);     //address high
        break;
        case REQ_INTRVL:
            ComBuf[8] = GetSensorInterval(WLSenArr[objToMsr].Type);//WakeupInterval;
            len = 1;
        break;
        case REQ_TYPE:
            ComBuf[8] = GetSensorType();
            len = 1;
        break;
        case REQ_TIME:
            GetRealTime();
            ComBuf[8] = readClockBuf[0]; //year
            ComBuf[9] = readClockBuf[1]; //month
            ComBuf[10] = readClockBuf[2]; //day
            ComBuf[11] = readClockBuf[4]; //hour
            ComBuf[12] = readClockBuf[5]; //minute
            len = 5;
        break;
        case REQ_IP:
            cpu_e2_to_MemCopy( &ComBuf[8], eIPorURLval1, 32);
            len = 32;
        break;
        case REQ_PORT:
            cpu_e2_to_MemCopy( &ComBuf[8], ePORTval1, 4);
            len = 4;
        break;
        case REQ_APN:
            cpu_e2_to_MemCopy( &ComBuf[8], eAPN, 32);
            len = 32;
        break;
        case REQ_MCC:
            cpu_e2_to_MemCopy( &ComBuf[8], eMobileCntryCode, 4);
            len = 4;
        break;
        case REQ_MNC:
            cpu_e2_to_MemCopy( &ComBuf[8], eMobileNetCode, 4);
            len = 4;
        break;
        case REQ_ROAMING:
            ComBuf[8] = !eUseCntrCode;
            len = 1;
        break;
        case REQ_SCH:
            ComBuf[8] = eStartConnectionH;
            len = 1;
        break;
        case REQ_CPD:
            ComBuf[8] = eConnectionInDay;
            len = 1;
        break;
        case REQ_CI:
            ComBuf[8] = eConnectIntervalH;
            len = 1;
        break;
        case REQ_BTR:
            ComBuf[8] = (unsigned char)(iVoltage) ;                 //address low
            ComBuf[9] = (unsigned char)((iVoltage >> 8) & 0xFF);     //address high
             len = 2;
        break;
        case REQ_RSSI:
            ComBuf[8] = (unsigned char)(rssi_val) ;                 //address low
            ComBuf[9] = (unsigned char)((rssi_val >> 8) & 0xFF);     //address high
            len = 2;
        break;
        case REQ_TIMEZONE:
            int2bytes(eTimeZoneOffset, &ComBuf[8]);
            len = 2;
        break;
        case REQ_VER:
            cpu_flash_to_MemCopy( &ComBuf[8], RomVersion, 4);
            len = 4;
        break;
        case REQ_NUM_SEN:
            ComBuf[8] = eNumSensors;
            len = 1;
        break;
        case REQ_EPOCH:
            ComBuf[8] = ee_epoch_time.second;   //seconds
            ComBuf[9] = ee_epoch_time.minute;   //minutes
            ComBuf[10] = ee_epoch_time.hour;   //hours
            ComBuf[11] = ee_epoch_time.day;   //day
            ComBuf[12] = ee_epoch_time.month;   //month
            ComBuf[13] = ee_epoch_time.year;   //year
            len = 6;
        break;
        default:
    }
    ComBuf[2] = len + 8;
    cs = CheckSum(ComBuf, len + 8, 1);
    ComBuf[len+8] = cs;
    BytesToSend = len + 9;
    TransmitBuf(1);
}

void SendBackResult()
{
    BYTE  cs;
    ComBuf[0] = 255;
    ComBuf[1] = 255;
    ComBuf[2] = 9;
    cpu_e2_to_MemCopy( &ComBuf[3], &eLoggerID[0], 4);
    ComBuf[7] = requestIndex;
    ComBuf[8] = setResult;
    cs = CheckSum(ComBuf, 9, 1);
    ComBuf[9] = cs;
    BytesToSend = 10;
    TransmitBuf(1);

//    #ifdef TestMonitor
//    PORTA.7 = 1;
//    delay_ms(200);
//    PORTA.7 = 0;
//    delay_ms(200);
//    #endif TestMonitor
}

void ExecuteSetCommand()
{
    BYTE n;
//    long l;
//    char buf[4];
    int t;

    setResult = TRUE;
//    #ifdef TestMonitor
//    PORTA.7 = 1;
//    delay_ms(50);
//    PORTA.7 = 0;
//    delay_ms(100);
//    #endif TestMonitor
    switch (requestIndex)
    {
        case REQ_ID:
                MemCopy_to_cpu_e2(&eLoggerID[0], &RxUart1Buf[9], 4);
        break;
        case REQ_INTRVL:
//            if (SetNewInterval(RxUart1Buf[9]) == FALSE)
//                setResult = FALSE;
        break;
        case REQ_TYPE:
//            if (SetSensorType(RxUart1Buf[9]) == FALSE)
//                setResult = FALSE;
        break;
        case REQ_TIME:
            MemCopy( clockBuf, &RxUart1Buf[9], 3 ); //copy year month day
            MemCopy( &clockBuf[4], &RxUart1Buf[12], 2 );  //copy hour minute
            if(SetRealTime() == FAILURE)
            {
                setResult = FALSE;
            }
        break;
        case REQ_IP:
            MemCopy_to_cpu_e2(eIPorURLval1, &RxUart1Buf[9], 32);
        break;
        case REQ_PORT:
            for (n = 0; n < 4; n++)
                if ((RxUart1Buf[n+9] < '0') || (RxUart1Buf[n+9] > '9'))  //if port is not a number
                    setResult = FALSE;
            if (setResult)
                MemCopy_to_cpu_e2(ePORTval1, &RxUart1Buf[9], 4);
        break;
        case REQ_APN:
            MemCopy_to_cpu_e2(eAPN, &RxUart1Buf[9], 32);
        break;
        case REQ_MCC:
            MemCopy_to_cpu_e2(eMobileCntryCode, &RxUart1Buf[9], 4);
        break;
        case REQ_MNC:
            //eMobileNetCode = bytes2int(&RxUart1Buf[9]);
            MemCopy_to_cpu_e2(eMobileNetCode, &RxUart1Buf[9], 4);
        break;
        case REQ_ROAMING:
            // the opposite 0=>1 1=>0
            if (RxUart1Buf[9] == 0)
                eUseCntrCode = 1;
            else
                if (RxUart1Buf[9] == 1)
                    eUseCntrCode = 0;
                else
                    setResult = FALSE;
        break;
        case REQ_SCH:
            if ((RxUart1Buf[9] >= 0) && (RxUart1Buf[9] < 23))
                eStartConnectionH = RxUart1Buf[9];
            else
                setResult = FALSE;
        break;
        case REQ_CPD:
            if ((RxUart1Buf[9] > 0) && (RxUart1Buf[9] <= 24))
                eConnectionInDay = RxUart1Buf[9];
            else
                setResult = FALSE;
        break;
        case REQ_CI:
            if ((RxUart1Buf[9] >= 0) && (RxUart1Buf[9] < 24))
                eConnectIntervalH = RxUart1Buf[9];
            else
                setResult = FALSE;
        break;
        case REQ_TIMEZONE:
            t = bytes2int(&RxUart1Buf[9]);
            if ((t >= -720) && (t <= 720))
                eTimeZoneOffset = t;
            else
                setResult = FALSE;
        break;
        case REQ_VER:    //NOT AVAILABLE TO SET
            break;
        case REQ_EPOCH:
            if (UpdateEpoch(&RxUart1Buf[9]) == FALSE)
                setResult = FALSE;
        break;
        case REQ_DISCNCT:
            bMonitorConnected = FALSE;
            bWaitForMonitorCmd = FALSE;
            bEndOfMonitorTask = TRUE;
            return;
        break;
        default:
            setResult = FALSE;
    }
    SendBackResult();
}

void MonitorMain()
{
    if (monitorCurTask == TASK_MONITOR_CONNECT)
    {
        ENABLE_UART1();
        UART1Select(UART_DBG);
        bMonitorConnected = FALSE;
//        bWaitForMonitorCmd = FALSE;
        #ifdef DebugMode
        delay_ms(2000);
        #endif DebugMode
        SendConnectString();
        lTimeFromLastTask = 0;
//        delay_ms(500);
        monitorCurTask = TASK_MONITOR_WAIT;
    }
    else
        if (monitorCurTask == TASK_MONITOR_WAIT)
        {
            if ((bWaitForMonitorCmd == TRUE) && (TimeLeftForWaiting == 0) && (bMonitorConnected == FALSE))
            {
                bCheckRxBuf = FALSE;
                bWaitForMonitorCmd = FALSE;
                bEndOfMonitorTask = TRUE;
//                #ifdef DebugMode
//                SendDebugMsg("\r\nMonitor hasn't connected\0");
//                #endif DebugMode
            }
            // if flag of end of rx received is on
            if (bCheckRxBuf == TRUE)
            {
                bCheckRxBuf = FALSE;
                lTimeFromLastTask = 0;
                if (CheckMonitorRequest() == FALSE)
                {
                    if (bMonitorConnected == FALSE)     //maybe always on wrong buffer get out??
                        bEndOfMonitorTask = TRUE;
                    setResult = FALSE;
                }
                else
                {
                    bMonitorConnected = TRUE;
                    if (requestType == GET_REQUEST)
                        ExecuteGetCommand();
                    else
                        if (requestType == SET_REQUEST)
                            ExecuteSetCommand();
                }
            }
            else
            {
                lTimeFromLastTask++;
                if (lTimeFromLastTask >= 20000000)  // counter will be ~2,000,000 in one minute. after 10 minutes ~20,000,000.
                {
                    bWaitForMonitorCmd = FALSE;
                    bEndOfMonitorTask = TRUE;
                    bMonitorConnected = FALSE;
                }
            }
        }
}

/*
BYTE GetNextTask()
{
    // first task-
    if (monitorCurTask == TASK_NONE)
    {
        modemCurTask = TASK_MONITOR_CONNECT;
        return  CONTINUE;
    }
    if ((bWaitForMonitorCmd == TRUE) && (TimeLeftForWaiting == 0))
    {
        bWaitForMonitorCmd = FALSE;
        MonitorResponse = TASK_FAILED;
    }
    // if flag of end of rx received is on
    if (bCheckRxBuf == TRUE)
    {
        bCheckRxBuf = FALSE;
        if (CheckMonitorRequest() == FALSE)
            MonitorResponse = TASK_FAILED;
        else
            MonitorResponse = TASK_COMPLETE;
    }
}
*/