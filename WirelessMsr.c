//measurements
#include <stdio.h>
#include "define.h"
extern eeprom unsigned char eNumSensors;
char MsrStatus[MAX_WL_SEN_NUM];
eeprom struct WirelessSensor WLSenArr[MAX_WL_SEN_NUM];
//char AtoDSen[] = {DER, FI3, SD, TNS, FI_01, GNRL_PLANT, SD_02};
extern const int MinInt;

//#ifdef WLDebug
//eeprom char test[10] = {0,0,0,0,0,0,0,0,0,0};//,
//                        0,0,0,0,0,0,0,0,0,0,
//                        0,0,0,0,0,0,0,0,0,0,
//                        0,0,0,0,0,0,0,0,0,0,
//                        0,0,0,0,0,0,0,0,0,0,
//                        0,0,0,0,0,0,0,0,0,0,
//                        0,0,0,0,0,0,0,0,0,0,
//                        0,0,0,0,0,0,0,0,0,0,
//                        0,0,0,0,0,0,0,0,0,0,
//                        0,0,0,0,0,0,0,0,0,0,
//                        0,0,0,0,0,0,0,0,0,0,
//                        0,0,0,0,0,0,0,0,0,0,
//                        0,0,0,0,0,0,0,0,0,0,
//                        0,0,0,0,0,0,0,0,0,0,
//                        0,0,0,0,0,0,0,0,0,0,
//                        0,0,0,0,0,0,0,0,0,0,
//                        0,0,0,0,0,0,0,0,0,0,
//                        0,0,0,0,0,0,0,0,0,0};
//#endif WLDebug
eeprom int MIN_LIMIT[MAX_WL_SEN_NUM] = {MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT};
eeprom int MAX_LIMIT[MAX_WL_SEN_NUM] = {MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT};
eeprom int MIN_STRIP[MAX_WL_SEN_NUM] = {MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT, MIN_INT};
eeprom int MAX_STRIP[MAX_WL_SEN_NUM] = {MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT, MAX_INT};
flash unsigned char ANSWER_ACK[] = "ACKACKACK@";
//flash unsigned char ANSWER_NACK[] = "NOK\r\n\0@";

extern bit bWait4WLSensor;
extern bit bConnectNow;
extern struct AlertData Alerts[MAX_SEN_NUM];
extern BYTE objToMsr;
extern bit bExtReset;
extern volatile int iLastMsr[MAX_WL_SEN_NUM];
extern bit bEndOfMeasureTask;
extern bit bCheckRxBuf;
extern char RxUart1Buf[MAX_RX1_BUF_LEN];
extern char readClockBuf[];	         //buffer for data reading from clock
extern int nTimeCnt;
extern BYTE msrCurTask;
extern unsigned int buffLen;
extern int measure_time;
extern unsigned int time_in_minutes;     //time from day start in ninutes
//char tagData[15];
//extern unsigned int pBread;		//pointer to last read sensor data block in ext_e2
//char pureBuf[90];
extern char ComBuf[MAX_SBD_BUF_LEN];
extern int BytesToSend;
extern bit bWaitForModemAnswer;
extern int iVoltage;
extern unsigned int rx1_buff_len;
#ifdef DebugMode
extern volatile BYTE mainTask;
#endif DebugMode
BYTE fGotData;
BYTE nCntNoDataCycle;
BYTE nDataok;
BYTE nBadAnswer;
BYTE nCycles;
BYTE nParseAns;
extern BYTE flgUart1Error;

void InitSensorArray()
{
    for (objToMsr = SENSOR1; objToMsr < MAX_WL_SEN_NUM; objToMsr++)
    {
        WLSenArr[objToMsr].Id = 0;
        WLSenArr[objToMsr].Type = 0;
        WLSenArr[objToMsr].Volt = 0;
        WLSenArr[objToMsr].Rssi = 0;
//        WLSenArr[objToMsr].Client = 0;
        WLSenArr[objToMsr].Index = 0;
//        WLSenArr[objToMsr].LocationStatus = NO_LOCATION;
    }
    eNumSensors = 0;
}

BYTE MeasuringTime(char i)
{
    if ((time_in_minutes % (GetSensorInterval(WLSenArr[i].Type) * INTERVAL_PARAM)) == 0)
        return TRUE;
    return FALSE;
}

BYTE GetSensorType()
{
    return WLSenArr[objToMsr].Type;
}

void Initialization()
{
    char i;
//    #ifdef DebugMode
//    mainTask = TASK_MONITOR;
//    SendDebugMsg("\r\nInit \0");
//    #endif DebugMode
    for (i = SENSOR1; i < MAX_WL_SEN_NUM; i++)
        MsrStatus[i] = MSR_INIT;    //MSR_NEEDED
    for (i = SENSOR1; i <= eNumSensors; i++)
        if ((MeasuringTime(i)) || (bExtReset))
        {
            MsrStatus[i] = MSR_NEEDED;
            WLSenArr[i].Index = 0;
            #ifdef DebugMode
            SendDebugMsg("\r\nNeed to msr sensor: \0");
            PrintNum(WLSenArr[i].Id);
            //putchar1(i+0x30);
            #endif DebugMode
            //if (i == SENSOR1)   // if its battery
              //  iLastMsr[0] = iVoltage;
        }
    bCheckRxBuf = FALSE;
    bWait4WLSensor = TRUE;
    bConnectNow = FALSE;
    //read the rtc
    GetRealTime();
    //set the measuring timing (from day start) into measure_time variable
    measure_time = time_in_minutes;
    //UART_1_WIRELESS();
//    #ifdef DebugMode
//    //delay_ms(500);
//    mainTask = TASK_MEASURE;
//    #endif DebugMode

    ENABLE_UART1();
    UART1Select(UART_RADIO_UHF);
    fGotData = 0;
    nBadAnswer = 0;
    nDataok = 0;
    nCycles = 0;
    if (bExtReset)
        nCntNoDataCycle = 0;
}

BYTE IsKnownSen(long id)
{
    char i;

    for (i = 1; i <= eNumSensors/*MAX_WL_SEN_NUM*/; i++)
        if (WLSenArr[i].Id == id)
            return i;
    return 0;

}

/*BYTE GetNextLocation2Send()
{
    char i;

    for (i = 1; i <= eNumSensors; i++)
        if (WLSenArr[i].LocationStatus == GOT_LOCATION)
            return i;
    return 0;
}   */

BYTE GetMinInterval()
{
    char i, num;

    if (eNumSensors == 0)
        return 1;
    num = GetSensorInterval(WLSenArr[1].Type);
    for (i = 2; i <= eNumSensors; i++)
        if (num > GetSensorInterval(WLSenArr[i].Type))
            num = GetSensorInterval(WLSenArr[i].Type);
    return num;
}

void ReadData()
{
    unsigned int i;
    #ifdef DebugMode
    SendDebugMsg("\r\nStart read data");
    #endif DebugMode
    bWait4WLSensor = TRUE;
    //UART_1_WIRELESS();
    UART1Select(UART_RADIO_UHF);
    delay_ms(100);
    nTimeCnt = 20;  // 2 sec timeout
    nCycles++;
    for (i = 0; i < MAX_RX1_BUF_LEN; i++)
        RxUart1Buf[i] = 0;
    rx1_buff_len = 0;
    flgUart1Error = FALSE;
    WIRELESS_CTS_ON();        // allow receiver send data
}


int AnalyzeSensorRealValue(int RowData)
{
    int sr;//, airTmp;
    long TempResult1;
    switch (WLSenArr[objToMsr].Type)
    {
        case TYPE_PLANT:
            // linear equation of vishay sensors: 0.0025 * (x-500)
            // slope is 0.0025 , intercept is (500, 0)
            TempResult1 = (long)RowData - 500;
            TempResult1 = TempResult1 * 25;
            sr = TempResult1 / 10;
        break;
        case TYPE_TENS:
        // P = (MVop + 95*Vs) / (9*Vs) --> Vop = rawdata in mv. Vs = 5
        if (RowData < 200)
            sr = 0;
        else
        {
            RowData -= 200;
            RowData *= 10;
            RowData /= 45;
            if (RowData > 860)
                RowData = 860;
            sr = 860 - RowData;
        }
        /*
// old calculation:

            TempResult2 = RowData;
            if(RowData > 152)
            {
                //calculate sensor real value
                TempResult2 = RowData - 152;
                TempResult2 *= 10;
                TempResult2 /= 22;
            }
            else
            {
                TempResult2 = 0;
            }
            if(TempResult2 > 840)
                TempResult2 = 840;
            //set measurments in cBar
            sr = 840 - TempResult2; */
        break;
        case TYPE_SMS:
            sr = RowData;
        break;
/*        case OXGN:
            sr = RowData;//iVoltage;
       case FI3:
            #ifdef DebugMode
            SendDebugMsg("\r\nFI3 ");
            #endif DebugMode
            //new calculation: calculation formula: (RowData x 5) - 1250
            TempResult1 = (float)RowData;
            TempResult1 = TempResult1 * 5;
            TempResult1 = TempResult1 - 1250;
            #ifdef DebugMode
            SendDebugMsg("\r\nvishay value= ");
            PrintNum((long)TempResult1);
            #endif DebugMode

            //check limits:
            if( TempResult1 <  1895)
            {
                #ifdef DebugMode
                SendDebugMsg("\r\n<  1895 ");
                #endif DebugMode
                sr = 2000;
                break;
            }
            if( TempResult1 >  9415)
            {
                #ifdef DebugMode
                SendDebugMsg("\r\n>  9415 ");
                #endif DebugMode
                sr = 8000;
                break;
            }
            //else if measuring is in range
            //formula for calibration calculation:
            //(((vishay mm - Vishay_mm[0]) * Vishay_Tan_Alfa] / 100)) + min_value_1
            TempResult1 = TempResult1 - 1895;
            TempResult1 = TempResult1 * 81;
            TempResult1 = TempResult1 / 100;
            sr = (int)TempResult1 + 2000;
        break;
        case FI_01:
            #ifdef DebugMode
            SendDebugMsg("\r\nFI NEW ");
            #endif DebugMode
            //new calculation: calculation formula: (RowData x 5) - 1250
            TempResult1 = (float)RowData;
            TempResult1 = TempResult1 * 5;
            TempResult1 = TempResult1 - 1250;
            sr = TempResult1;

            //check limits:
            if (sr <  1000)
            {
                #ifdef DebugMode
                SendDebugMsg("\r\n< 1000 ");
                #endif DebugMode
                sr = 1000;
            }
            if (sr >  11000)
            {
                #ifdef DebugMode
                SendDebugMsg("\r\n>  11000 ");
                #endif DebugMode
                sr = 11000;
            }
        break;
        case SD_02:
            #ifdef DebugMode
            SendDebugMsg("\r\nSD Rev2 ");
            #endif DebugMode
            //new calculation: calculation formula: (RowData x 5) - 1250
            TempResult1 = (float)RowData;
            TempResult1 = TempResult1 * 5;
            TempResult1 = TempResult1 - 1250;

            //check limits:
            if (TempResult1 <  2000)
            {
                #ifdef DebugMode
                SendDebugMsg("\r\n< 2000 ");
                #endif DebugMode
                sr = 1000;
            }
            if (TempResult1 >  10000)
            {
                #ifdef DebugMode
                SendDebugMsg("\r\n>  10000 ");
                #endif DebugMode
                sr = 10000;
            }
            TempResult1 -= 2660;
            TempResult1 *= 3.14;
            sr = TempResult1 + 3000;
        break;
        */
        /*
        case HS10:
            //check pota.0: if = 1 it is 10HS type; if = 0 it is Ec5 type
            if(pinB0 == 0)
            {
                #ifdef DebugMode
                SendDebugMsg("\r\nCalc EC");
                #endif DebugMode
                if (RowData < 400)
                    RowData = 400;
                if (RowData > 920)
                    RowData = 920;
                TempResult = RowData * 12; //calibration_param_1
                TempResult = TempResult - 4800;   //calibration_param_2
                TempResult = TempResult / 10;
                sr = TempResult;
            }
            else                    // for 10HS:
            {
                #ifdef DebugMode
                SendDebugMsg("\r\nCalc 10HS");
                #endif DebugMode
                if (RowData < 537)
                    RowData = 537;
                if (RowData > 1160)
                    RowData = 1160;
                TempResult1 = (float)RowData * RowData;
                TempResult2 = TempResult1 * RowData;
                TempResult2 = TempResult2 * 0.00000000297;  //C1;
                TempResult1 = TempResult1 * 0.00000737; //C2;
                TempResult3 = RowData * 0.00669; //C3
                TempResult3 = (TempResult2 - TempResult1) + TempResult3;
                TempResult3 = TempResult3 - 1.92;   //C4;
                TempResult1 = TempResult3 * 1000; //for 0.1% resolution
                sr = TempResult1;
            }
        break;
        case EC:
            sr = calculate_raw_ec();//Pore_EC_calculation();
        break;
        case SM:
            sr = calculate_raw_sm();//calculate_vwc_sm();
        break;
        case TMP:
            sr = calculate_raw_temp();
        break;
        case AT:
        case ST5:
            //if first measuring failed - try again
            if ((RowData > 700) || (RowData < -200) || (RowData == -5))
            {
               RowData = w1_SensRead(); //read out temp. again
            }
            if ((RowData >700) || (RowData <-200) || (RowData == -5))
                sr = 0;
            else
                sr = RowData;
        break;

        case AH: //Air Humidity
            //calculate sensor real value
            RowData = (RowData - 400) / 15;
            sr = RowData;
            if (objToMsr > 0)
                //check min/max value (temp > 50 or < 0) using the temp. sensor results
                if((iLastMsr[objToMsr - 1] > 0) && (iLastMsr[objToMsr-1] < 500))//if 1w read ok
                {
                    //calculate measuring according to out temp.
                    airTmp = iLastMsr[objToMsr-1] / 10;
                    //calculate the const value
                    percentValue = ((long)airTmp * 44) / 10;
                    percentValue = (10305 + percentValue) / 100;
                    //aprocsimated formula
                    sr =  (((long)RowData * 100) / percentValue);
                }
            //check min & max values
            if (sr < 0)
                sr = 0;
            if (sr > 100)
                sr = 100;
        break;
        case TIR:  // radiation
            //if it is out of range, send to eeprom the low or high table value.
            if(RowData < 0)
            {
                sr = 0;
                break;
            }
            if(RowData > 1260)
            {
                sr = 1260;
                break;
            }

            // measuring is in range
            mVolt_differance = (((unsigned int)RowData) * 10);
            mVolt_differance = mVolt_differance / 33;
            sr = (mVolt_differance * 10);
        break;
        case LT:   // leaf temp - liniar
            //calculate sensor real value
            RowData = RowData * 10; //(RowData - LeafTmp_mVolt[0]) * 10;
            sr = RowData / 40;   //LeafTmp_Tan_Alfa;

            //set min/max for error value
            if(sr > 500)
                sr = 500;
            if(sr < 0)
                sr = 0;
        break;
         case BHS:  // bee hive weight
            if(RowData > HIVE_MIN_VOLT)                   //for 100Kg 0 for 200Kg 100
            {
                TempResult = (float)HS_cal_value;
                TempResult1 = TempResult * HIVE_TAN_ALFA;         //for 100Kg 479 for 200Kg 922
                TempResult1 = TempResult1 / 100;
                TempResult = (float)(RowData - HIVE_MIN_VOLT);
                TempResult = TempResult * TempResult1;
                TempResult = TempResult / 100;
                TempResult1 = TempResult + HS_offset_value;
                sr = (int)TempResult1;
            }
            else
                sr = 0;
            // check min & max
            if (sr > HIVE_MAX_VAL)                                  //for 100Kg 11500 for 200Kg 20000
                sr = HIVE_MAX_VAL;
            if (sr < 0)
                sr = 0;
        break;
        case OXGN:
            sr = RowData;
        break;

        case WSPD:
            //max value: 1250 pulses (50.0 m/s) (1250*10)/(2500/100) = 50.0
            if (wndSpdPulseCnt > 6400)
                wndSpdPulseCnt = 6400;
            wndSpdPulseCnt = wndSpdPulseCnt * 10;
            wndSpdPulseCnt = wndSpdPulseCnt / (cpuWakeTimeCnt / 10);
            //get ws result
            sr = wndSpdPulseCnt;
            //---check minimum / maximum limits---
            if(sr > 580)
                sr = 580;
            if(sr < 0)
                sr = 0;
            break;
        break;
        case WM:
        case RAINMTR:
            sr = wtrPulseCnt;
        break;
        case WFS_1:
            sr = WtrCntrPls;
            WtrCntrPls = 0;
        break;
        case WDR:
            //check for deviation from calibration parameters
            //if it is out of range, send to eeprom the low or high table value.
            if(RowData < 0)
                RowData = 0;
            if(RowData > 2500)
                RowData = 2500;
            // measuring is in range
            //calculate sensor real value
            sr = RowData / 7;	// * 10 / 70
        break;
*/
        default:
            sr = RowData;
    }
//    #ifdef DebugMode
//    SendDebugMsg("\r\nCalculated val= ");
//    PrintNum(sr);
//    #endif DebugMode
    return sr;
}

BYTE ParsePhytechData()
{
    char i, pcktSize, cs, msgType, index;//, res;
    long lID;

    fGotData++;
//    #ifdef DebugMode
//    UART1Select(UART_DBG);
//    delay_ms(250);
//    SendDebugMsg("\r\nParse data: \0");
//    putchar1('\r');
//    putchar1('\n');
//    putchar1('*');
//    putchar1('*');
//    for (i = 0; i < buffLen; i++)
//        putchar1(RxUart1Buf[i]);
//    putchar1('*');
//    delay_ms(100);
//    PORTA.6 = 0;
//    #endif DebugMode
    if (flgUart1Error != 0)
    {
        #ifdef DebugMode
        SendDebugMsg("\r\nUart1 Error: \0");
        putchar1(flgUart1Error);
        #endif DebugMode
        flgUart1Error = 0;
    }

    if (buffLen < 15)
    {
//        #ifdef DebugMode
//        SendDebugMsg("\r\n unreasonable size \0");
//        PrintNum(buffLen);
//        delay_ms(100);
//        #endif DebugMode
       i = 0;
        do
        {
            if ((RxUart1Buf[i + 0] == 'N') &&
                (RxUart1Buf[i + 1] == 'O') &&
                (RxUart1Buf[i + 2] == 'D') &&
                (RxUart1Buf[i + 3] == 'A') &&
                (RxUart1Buf[i + 4] == 'T') &&
                (RxUart1Buf[i + 5] == 'A'))
            {
                 #ifdef DebugMode
                SendDebugMsg("\r\nno data\0");
                #endif DebugMode
                return NO_DATA;
            }
            i++;
        }
        while ((i + 5) < buffLen);
        return FALSE;
    }

//    res = 0;
//    while ((res == 0) && (i < buffLen))
//    {
//        if ((RxUart1Buf[i] == 0xAB) && (RxUart1Buf[i+1]))
//            res = 1;
//        else
//            i++;
//    }
   i = 0;
    while ((RxUart1Buf[i] != 0xAB) && (i < buffLen))
        i++;
    if (RxUart1Buf[i] != 0xAB)
    {
        #ifdef DebugMode
        SendDebugMsg("\r\nincorrect header \0");
        #endif DebugMode
        return FALSE;
    }
    pcktSize = bytes2int(&RxUart1Buf[i + SIZE_BYTE_INDEX]);   // [3] on previous version
    #ifdef DebugMode
    SendDebugMsg("\r\ni = \0");
    putchar1(i+0x30);
//    SendDebugMsg("\r\nBuflen size is\0");
//    SendDebugMsg("\r\npacket size is\0");
//    PrintNum(pcktSize);
//    SendDebugMsg("\r\nBuflen size is\0");
//    PrintNum(buffLen);
    #endif DebugMode


    if (buffLen < pcktSize + 1)    // 1 = number of bytes not include in size
    {
        #ifdef DebugMode
        SendDebugMsg("\r\npacket size dont fit\0");
        #endif DebugMode
        return FALSE;
    }
    // verify check sum
    cs = CheckSum(&RxUart1Buf[i], pcktSize  ,1);
    if (cs != RxUart1Buf[pcktSize+i])
    {
        #ifdef DebugMode
        SendDebugMsg("\r\nwrong check sum \0");
        #endif DebugMode
//        #ifdef WLDebug
//        //PORTA.6 = 1;
//        //delay_ms(4000);
//        //PORTA.6 = 0;
//
//        //#ifndef WLOnUart0
//        test[0] = 'W';
//        test[1] = buffLen;
//        test[2] = pcktSize;
//        test[3] = cs;
//        test[4] = RxUart1Buf[buffLen - 2];
//        test[5] = RxUart1Buf[pcktSize + 5];
//        test[6] = 'W';
//        for (i = 0; i < buffLen; i++)
//            test[i+7] = RxUart1Buf[i];
//        i += 7;
//        for (; i < 90; i++)
//            test[i] = 0;
//
//       // #endif WLOnUart0
//
//        #endif WLDebug
        return FALSE;
    }

    index = i + FIRST_DATA_INDEX;
    do
    {
        msgType = RxUart1Buf[index];// == SENSOR_DATA_ID )
        // check msg type
        if (!((msgType == SENSOR_DATA_ID) || (msgType == SENSOR_LOCATION_ID) || (msgType == SENSOR_GPS_ID)))
        {
            #ifdef DebugMode
            SendDebugMsg("\r\n Msg type Wrong\0");
            #endif DebugMode
            return FALSE;
        }
        lID = Bytes2Long(&RxUart1Buf[index+ID]);
        if (lID == 0)
        {
            #ifdef DebugMode
            SendDebugMsg("\r\nID=0 is wrong\0");
            #endif DebugMode
            return FALSE;
        }
        // check if this sensor ID ever connected before
        objToMsr = IsKnownSen(lID);

        // if its first time sensor connected - add it to list of sensors
        if (objToMsr == 0)
        {
//            #ifdef DebugMode
//            SendDebugMsg("\r\nNew Sensor \0");
//            PrintNum(lID);
//            #endif DebugMode
            if (eNumSensors >= MAX_WL_SEN_NUM)
            {
                #ifdef DebugMode
                SendDebugMsg("\r\nToo many sensors. cant add another \0");
                #endif DebugMode
                if (msgType == SENSOR_DATA_ID)
                    index += DATA_MSG_LEN;
                if ((msgType == SENSOR_LOCATION_ID) || (msgType == SENSOR_GPS_ID))
                    index += LOCATION_MSG_LEN;
                continue;
            }
            eNumSensors++;
            objToMsr = eNumSensors;
            // save ID
            WLSenArr[eNumSensors].Id = lID;
            MsrStatus[objToMsr] = MSR_NEEDED;
            // prepare data block
            ResetPointers(objToMsr);
            //init location parameters
            SaveMapRefParams(0,0);
//            WLSenArr[objToMsr].LocationStatus = NO_LOCATION;
        }
//        #ifdef DebugMode
//        else
//            SendDebugMsg("\r\nknown Sensor \0");
//        #endif DebugMode

        //  copy single tag data to tagData
        if (msgType == SENSOR_DATA_ID)
        {
            // if message index is bigger - save it. else - current measure is probably a new one.
            if (WLSenArr[objToMsr].Index <= RxUart1Buf[index + MSG_INDEX])
            {
                WLSenArr[objToMsr].Type = RxUart1Buf[index + TYPE];
                WLSenArr[objToMsr].Volt = bytes2int(&RxUart1Buf[index + VOLTAGE]);  // to get real value of battery should devide by 64
                WLSenArr[objToMsr].Rssi = RxUart1Buf[index + RSSI];
                WLSenArr[objToMsr].Index = RxUart1Buf[index + MSG_INDEX];
                //data = bytes2int(&RxUart1Buf[index + DATA]);
                //data = AnalyzeSensorRealValue(data);
                iLastMsr[objToMsr] = bytes2int(&RxUart1Buf[index + DATA]);//data;
                nDataok++;
            }
//            #ifdef DebugMode
//            else
//            {
//                putchar1('-');
//                putchar1('-');
//                putchar1('-');
//            }
//            #endif DebugMode

            index += DATA_MSG_LEN;
        }
//        if (msgType == SENSOR_GPS_ID)
//        {
//            WLSenArr[objToMsr].Type = TYPE_PIVOT;
//            if (SaveMeasurments(0, &RxUart1Buf[index + LATITUDE],  TYPE_PIVOT) == TRUE)
//                MsrStatus[objToMsr] = MSR_DONE;
//            index += LOCATION_MSG_LEN;
//        }
        if (msgType == SENSOR_LOCATION_ID)
        {
            SaveMapRefParams(Bytes2Float(&RxUart1Buf[index + LATITUDE]), Bytes2Float(&RxUart1Buf[index + LONGITUDE]));
//            WLSenArr[objToMsr].LocationStatus = GOT_LOCATION;
            index += LOCATION_MSG_LEN;
            nDataok++;
            if (WLSenArr[objToMsr].Type == TYPE_PIVOT)
                bConnectNow = TRUE;
        }
    }
    while (index < pcktSize);
    #ifdef DebugMode
    SendDebugMsg("\r\nEnd parsing successfuly");
//    UART1Select(UART_RADIO_UHF);
//    delay_ms(250);
    #endif DebugMode
    return TRUE;
}

void AnswerReceiver(BYTE ans)
{
//    UART1Select(UART_RADIO_UHF);
//    delay_ms(50);
    if (ans == TRUE)
    {
        #ifdef DebugMode
        delay_ms(250);
        #endif DebugMode
        CopyFlashToBuf(ComBuf, ANSWER_ACK);
        BytesToSend = 10;
        TransmitBuf(1);
//        #ifdef DebugMode
//        SendDebugMsg("\r\nACKACK");
//        #endif DebugMode
    }
    else
        if (ans == FALSE)
            nBadAnswer++;
}

void SaveAllData()
{
//    #ifdef DebugMode
//    mainTask = TASK_MONITOR;
//    delay_ms(500);
//    SendDebugMsg("\r\nsave All data\0");
//    #endif DebugMode
    if (fGotData == 0)
        nCntNoDataCycle++;
    else
        nCntNoDataCycle = 0;
//   {
//        #ifdef DebugMode
//        SendDebugMsg("\r\nsave All 2\0");
//        #endif DebugMode

    for (objToMsr = SENSOR1; objToMsr <= eNumSensors; objToMsr++)
    {
       if ((MsrStatus[objToMsr] == MSR_NEEDED) && (iLastMsr[objToMsr] != MinInt))
        {
            #ifdef DebugMode
            SendDebugMsg("\r\nneed to save data for sensor \0");
            PrintNum(WLSenArr[objToMsr].Id);
            SendDebugMsg("\r\nData: \0");
            PrintNum(iLastMsr[objToMsr]);
            SendDebugMsg("\r\nType: \0");
            PrintNum(WLSenArr[objToMsr].Type);
            SendDebugMsg("\r\nrssi \0");
            PrintNum(WLSenArr[objToMsr].Rssi);
            SendDebugMsg("\r\nbatery \0");
            PrintNum(WLSenArr[objToMsr].Volt);
            #endif DebugMode
            if (SaveMeasurments(iLastMsr[objToMsr], WLSenArr[objToMsr].Type) == TRUE)
            {
                MsrStatus[objToMsr] = MSR_DONE;
                WLSenArr[objToMsr].Index = 0;
                #ifdef DebugMode
                SendDebugMsg("\r\nsave OK \0");
//                    PrintNum(WLSenArr[objToMsr].Id);
//                    SendDebugMsg("\r\nData: \0");
//                    PrintNum(iLastMsr[objToMsr]);
//                    SendDebugMsg("\r\nType: \0");
//                    PrintNum(WLSenArr[objToMsr].Type);
//                    SendDebugMsg("\r\nrssi \0");
//                    PrintNum(WLSenArr[objToMsr].Rssi);
//                    SendDebugMsg("\r\nbatery \0");
//                    PrintNum(WLSenArr[objToMsr].Volt);
                #endif DebugMode
            }
            iLastMsr[objToMsr] = MinInt;
        }
    }

    #ifdef DebugMode
    // because read every 5 minutes
       if (nCntNoDataCycle > 10)
    #else
    if (nCntNoDataCycle > 2)
    #endif DebugMode
    {
        #ifdef DebugMode
        SendDebugMsg("\r\nRestart wireless receiver \0");
        #endif DebugMode
        WIRELESS_PWR_DISABLE();
        delay_ms(3000);
        WIRELESS_PWR_ENABLE();
        nCntNoDataCycle = 0;
    }
//    #ifdef DebugMode
////    delay_ms(500);
//    mainTask = TASK_MEASURE;
//    #endif DebugMode
}

BYTE GetNextMsrTask()
{
    switch (msrCurTask)
    {
        case TASK_NONE:
            msrCurTask = TASK_MSR_INIT;
        break;
        case  TASK_MSR_INIT:
            msrCurTask = TASK_MSR_READ;
        break;
        case TASK_MSR_READ:
            if ((nCycles > 5) && ((nDataok == 0) || (nBadAnswer > 3)))
            {
                    msrCurTask = TASK_MSR_CLOSURE;
                    break;
            }
            if (bCheckRxBuf == TRUE)
                msrCurTask = TASK_MSR_SAVE;
            else
                if (nTimeCnt > 0)
//                if (bWait4WLSensor == TRUE)
                    return WAIT;
                else
                    msrCurTask = TASK_MSR_CLOSURE;
        break;
        case TASK_MSR_SAVE:
            if (nParseAns == NO_DATA)
                msrCurTask = TASK_MSR_CLOSURE;
            else
                if ((nParseAns == TRUE) && (buffLen < MAX_RX1_BUF_LEN - 15))
                    msrCurTask = TASK_MSR_CLOSURE;
                else
                    msrCurTask = TASK_MSR_READ;
        break;
        case TASK_MSR_CLOSURE:
            msrCurTask = TASK_NONE;
            bEndOfMeasureTask = TRUE;
            bWait4WLSensor = FALSE;
            #ifdef DebugMode
            SendDebugMsg("\r\nend of read\0");
            #endif DebugMode
        break;
        default:
    }
    return CONTINUE;
}

void CheckMeasurmentsLimits()
{
    int i;
    // if during external reset - do not check limits
    if (bExtReset == TRUE)
        return;
    #ifdef DebugMode
    SendDebugMsg("\r\nCheckMeasurmentsLimits");
    #endif DebugMode
    for (i = SENSOR1; i <= eNumSensors; i++)
    {
        #ifdef DebugMode
        SendDebugMsg("\r\nSensor  ");
        PrintNum(i);
        SendDebugMsg("\r\nStatus before:  ");
        PrintNum(Alerts[i].Status);
        #endif DebugMode
        switch (Alerts[i].Status) //(AlertStatus[i])
        {
            case ALERT_WAIT:
                if ((iLastMsr[i] < MIN_LIMIT[i]) || (iLastMsr[i] > MAX_LIMIT[i]))
                {
                    Alerts[i].Status = TRHRESHOLD_CROSSED;
                    Alerts[i].OutOfLmtCnt = 1;
                    Alerts[i].MsrCnt = 1;
                    #ifdef DebugMode
                    SendDebugMsg("\r\nSensor data out of limit ");
                    #endif DebugMode
                }
            break;
            case TRHRESHOLD_CROSSED:
                Alerts[i].MsrCnt++;
                if ((iLastMsr[i] < MIN_LIMIT[i]) || (iLastMsr[i] > MAX_LIMIT[i]))
                {
                    Alerts[i].OutOfLmtCnt++;
                    #ifdef DebugMode
                    SendDebugMsg("\r\nSensor data out of limit again ");
                    #endif DebugMode
                }
                if ((Alerts[i].MsrCnt >= 3) && (Alerts[i].OutOfLmtCnt < 2))
                {
                    Alerts[i].Status = ALERT_WAIT;
                    Alerts[i].OutOfLmtCnt = 0;
                    Alerts[i].MsrCnt = 0;
                    #ifdef DebugMode
                    SendDebugMsg("\r\nstatus back to wait");
                    #endif DebugMode
                }
            break;
            case ALERT_SHOT:
                if ((iLastMsr[i] >= MIN_STRIP[i]) && (iLastMsr[i] <= MAX_STRIP[i]))
                {
                    Alerts[i].Status = ALERT_BACK_NORMAL;
                    #ifdef DebugMode
                    SendDebugMsg("\r\nSensor data back normal ");
                    #endif DebugMode
                }
            break;
        }
        #ifdef DebugMode
        SendDebugMsg("\r\nStatus After:  ");
        PrintNum(Alerts[i].Status);
        #endif DebugMode
    }
}

void MeasureMain()
{
//    BYTE res;
    if (GetNextMsrTask() == WAIT)
        return;

    switch (msrCurTask)
    {
        case TASK_MSR_INIT:
            Initialization();
        break;
        case TASK_MSR_READ:
            ReadData();
        break;
        case TASK_MSR_SAVE:
            bCheckRxBuf = FALSE;
            nParseAns = ParsePhytechData();
//            #ifdef DebugMode
//            if (res == TRUE)
//                SendDebugMsg("\r\nparse is OK  ");
//            else
//                SendDebugMsg("\r\nparse is not OK");
//            #endif DebugMode
            AnswerReceiver(nParseAns);
        break;
        case TASK_MSR_CLOSURE:
            // close CTS wireless
            WIRELESS_CTS_OFF();
//            DISABLE_UART1();
            SaveAllData();
            break;
//            CheckMeasurmentsLimits(); // todo - put back if needed
        default:
    }
}

BYTE SendAlerts()
{
    int i;//, res = FALSE;

    for (i = SENSOR1; i <= eNumSensors; i++)
    {
        if (Alerts[i].Status == ALERT_BACK_NORMAL)
            return TRUE;
        if ((Alerts[i].Status == TRHRESHOLD_CROSSED) && (Alerts[i].OutOfLmtCnt >= 2))    //(Alerts[i].MsrCnt >= 3))
                return TRUE;
    }
    return FALSE;
}

