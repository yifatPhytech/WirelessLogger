#ifndef MODEM_MANAGER_C
#define MODEM_MANAGER_C
#include "define.h"

extern bit bCheckRxBuf;
extern bit bExtReset;
extern bit bWaitForModemAnswer;
extern bit bEndOfModemTask;
extern bit bNeedToWait4Answer;
extern BYTE bConnectOK;
extern eeprom BYTE eNumSensors;
extern bit longAnswerExpected;
extern bit overFlow;
extern BYTE modemCurTask;
extern BYTE modemCurSubTask;
extern BYTE waitingTask;
extern BYTE dataSentOK;
extern BYTE prmSentOK;
extern BYTE toDoList;
extern int BytesToSend;
extern BYTE ModemResponse;
extern BYTE objToMsr;
extern bit bReset;
extern int TimeLeftForWaiting;
extern unsigned int rx0_buff_len;
extern BYTE rssi_val;
extern BYTE UpdatePrmArr[MAX_PRM_TASKS];
extern char e2_writeFlag;
extern char readClockBuf[];	         //buffer for data reading from clock
extern char DataBlock[PCKT_LNGTH];
extern char clockBuf[7]; 		 //buffer for all clock operation need
extern char ComBuf[MAX_SBD_BUF_LEN];
extern volatile char RxUart0Buf[MAX_RX_BUF_LEN];
//extern unsigned int nextCompare;
extern int nMaxWaitingTime;
extern unsigned int pBread;		//pointer to last read sensor data block in ext_e2
extern int iVoltage;
extern int nTimeCnt;
extern float g_fLat;
extern float g_fLon;
extern unsigned int nextCompare;
//extern eeprom int gMin4UTC;
//extern eeprom int gHr4UTC;
bit bUpdateAddress;
char newURL[32];
char cEndMark = '\0';
BYTE nMaxFailuresNum;
BYTE prmUpdtIndex;
BYTE bufIndexToUpd;
BYTE failCnt;
BYTE initCnt;
BYTE bPostAnswered;
BYTE bMakeReset;
BYTE PrmLen[MAX_PRM_TASKS] = {1,32,4,32,6,32,32,1,1,1,1,0,8,8,1,1,4,4,4};    //task 11 not exist. skip the number
unsigned char ICCID[] = "********************";
unsigned char CellID[] = "*****";
extern int nUnreadBlocks;
extern BYTE nFailureCntr;
BYTE taskAfterDelay;
extern struct AlertData Alerts[MAX_SEN_NUM];
extern struct OperatorData OprtTbl[10] ;
//float fLtd, fLng, fUncr;
char prevID[4];
extern eeprom struct WirelessSensor WLSenArr[];
BYTE isRoaming = FALSE;
BYTE numOprt = 0;
BYTE curOprt = 0;
BYTE nConnectError = 0;
BYTE nRegDenied = 0;
BYTE fMdmAns;
BYTE modemGSMModel = 0;
flash unsigned char PHYTECH_FILE_END_MARK[] = "phy111\r\n#";  //8  MUST BE AT LEAST 6 CHARS!!!!!!!!!
// update here rom version
flash unsigned char MONTH_T = 6;
flash unsigned char YEAR = 17;
flash unsigned char INDEX_T = 3;

//#ifdef DebugMode
//flash unsigned char MONTH = MONTH_T + 100;
//flash unsigned char VER_INDEX = __BUILD__;
////eeprom char test[40] = {'#',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'#'} ;
//#else
flash unsigned char MONTH = MONTH_T;
flash unsigned char VER_INDEX = INDEX_T;
//#endif DebugMode

//flash unsigned char YEAR = YEAR_T + 100;

flash unsigned char RomVersion[4] = {'W',MONTH,YEAR,VER_INDEX};   //__BUILD__

extern eeprom char eLoggerID[]; //Logger id
extern eeprom unsigned char eStartConnectionH;        //first connection hour
extern eeprom unsigned char eConnectionInDay;        //number on connectionsin a day
extern eeprom unsigned char eConnectIntervalH;        //intervalbetween connections (hours)
extern eeprom int MIN_LIMIT[MAX_WL_SEN_NUM];
extern eeprom int MAX_LIMIT[MAX_WL_SEN_NUM];
extern eeprom int MIN_STRIP[MAX_WL_SEN_NUM];
extern eeprom int MAX_STRIP[MAX_WL_SEN_NUM];

eeprom unsigned char eRoamingDelay = 15;      //eOperatorType
eeprom unsigned char eUseCntrCode = 0;
eeprom unsigned char eMobileNetCode[] = "01#0";
eeprom unsigned char eMobileCntryCode[] = "425#";
// if eMobileCntryCode & eMobileNetCode are integers
//eeprom unsigned int eMobileNetCode = 1;
//eeprom unsigned int eMobileCntryCode = 425;
//eeprom unsigned char eIPorURLval1[] = "phytech1.dyndns.org#000000000000"; //32 bytes
//eeprom unsigned char eIPorURLval1[] = "54.246.85.255#000000000000000000"; //32 bytes  //amazon 2
//eeprom unsigned char eIPorURLval1[] = "54.247.88.8#00000000000000000000"; //32 bytes  //amazon 1
//#ifdef DebugMode
//eeprom unsigned char eIPorURLval1[] = "test-proxy.phytech.com#000000000"; //32 bytes  //
//#else
eeprom unsigned char eIPorURLval1[] = "proxy.phytech.com#00000000000000"; //32 bytes  //
//#endif DebugMode
//eeprom unsigned char eIPorURLval1[] = "62.90.59.100#0000000000000000000"; //32 bytes  //server 16    port 1032
eeprom unsigned char ePORTval1[]    = "1018";                             //4 bytes
//eeprom unsigned char eAPN[]         = "internet#00000000000000000000000";     //"internetm2m.air.com#000000000000";
//eeprom unsigned char eAPN[]         = "JTM2M#00000000000000000000000000";     //"internetm2m.air.com#000000000000";
eeprom unsigned char eAPN[]         = "internetm2m.air.com#000000000000";     //"internetm2m.air.com#000000000000";

//

const BYTE BAUD_RATE_HIGH[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
const BYTE BAUD_RATE_LOW[]  = {0x01, 0x03, 0x05, 0x0B, 0x17, 0x2F, 0xBF};
const BYTE BAUD_RATE_LONG[] = {115200, 57600,  38400 , 19200 , 9600, 4800, 1200};


flash unsigned char  AT_DELL_ECHO[] = "ATE0\r\n\0";
flash unsigned char  AT_GPIO[] = "AT#GPIO=1,0,2\r\n\0";
flash unsigned char  AT_SLED[] = "AT#SLED=2,10\r\n\0";
flash unsigned char  AT_SLEDSAV[] = "AT#SLEDSAV\r\n\0";
flash unsigned char  AT_K[] = "AT&K0;&D0;+ICF=3\r\n\0";
flash unsigned char  AT_IPR[] = "AT+IPR=19200;&P0;&W\r\n\0";

flash unsigned char  AT_IsModemOK[] = "AT\r\n\0";             //5 bytes
flash unsigned char  AT_IsModemReg[] = "AT+CREG?\r\n\0";     //10 bytes
flash unsigned char  AT_REG_STATUS[] = "AT+CGREG?\r\n\0";
flash unsigned char  AT_COPS_AUTO_0[] = "AT+COPS=0\r\n\0";      //11 bytes
flash unsigned char  AT_COPS_AUTO_2[] = "AT+COPS=2\r\n\0";      //11 bytes
flash unsigned char  AT_COPS_LST[] = "AT+COPS=?\r\n\0";
flash unsigned char  AT_COPS_MAN[] = "AT+COPS=1,2,@";       //12 bytes. must have # at the end
flash unsigned char  AT_COPS_ASK[] = "AT+COPS?\r\n\0";
flash unsigned char  AT_CELL_MONITOR[] = "AT#MONI\r\n\0";
flash unsigned char  AT_CSQ[] = "AT+CSQ\r\n\0";               //8 bytes  AT+GMR-returns the software revision identification
flash unsigned char  AT_EOD[] = "+++\0";               //8 bytes  AT+GMR-returns the software revision identification
flash unsigned char  AT_QSS[] = "AT#QSS?\r\n\0";             //
flash unsigned char AT_CCID[] = "AT#CCID\r\n\0";
flash unsigned char AT_CGMM[] = "AT+CGMM\r\n\0";
//GPRS connecting commands:
flash unsigned char GPRS_ATTACH[] = "AT+CGATT=1\r\n\0";                               //12
flash unsigned char DEF_PDP_CNTXT[] = "AT+CGDCONT=1,\"IP\",@";    //39
flash unsigned char DEF_QULT_MIN_PROF[] = "AT+CGQMIN=1,0,0,0,0,0\r\n\0";                     //24
flash unsigned char DEF_QULT_REQ_PROF[] = "AT+CGQREQ=1,0,0,0,0,0\r\n\0";                     //24
flash unsigned char DEF_SCKT_CNFG[] = "AT#SCFG=1,1,1500,30,150,3\r\n\0";                      //24
flash unsigned char ACTIVATE_CNTXT[] = "AT#SGACT=1,1\r\n\0";                                //14
//flash unsigned char ACTIVATE_CNTXT_QUS[] = "AT#SGACT?\r\n\0";                             //14
//flash unsigned char ENABLE_MAPPING[] = "AT#CACHEDNS=1\r\n\0";                                //14
//flash unsigned char GET_POS[] = "AT#AGPSSND\r\n\0";                                //14
//flash unsigned char GET_TIME[] = "AT$GPSACP\r\n\0";                                //14
//flash unsigned char NETWORTK_TIMEZONE[] = "AT#NITZ=8,1\r\n\0";                                //14
//flash unsigned char GET_CLOCK[] = "AT#CCLK?\r\n\0";

flash unsigned char AT_TCP_OPN[] = "AT#SD=1,0,@";           //"AT#SD=1,0,1020,\"phytech1.dyndns.org\"\r\n";   //40
flash unsigned char AT_TCP_CLS[] = "AT#SH=1\r\n\0";                               //9
flash unsigned char MODEM_CLS[] = "AT#SHDN\r\n\0";
// post commands
flash unsigned char AT_POST_TITLE_PRM[] = "POST /api/sensor/sensorparamsext HTTP/1.1\r\n#";  //                 api/file/sensorparams
//flash unsigned char AT_POST_TITLE_LOCATION[] = "POST /api/sensor/SensorLocation HTTP/1.1\r\n#";  //                 api/file/sensorparams
//flash unsigned char AT_POST_TITLE_PRM[] = "POST /api/sensor/sensorparams HTTP/1.1\r\n#";  //                 api/file/sensorparams
//flash unsigned char AT_POST_TITLE_DATA[] = "POST /api/sensor/sensordata HTTP/1.1\r\n#";   //36               api/file/sensordata
//flash unsigned char AT_POST_TITLE_DATA_LOGGER[] = "POST /api/sensor/loggersensordata HTTP/1.1\r\n#";   //               api/file/loggersensordata
//flash unsigned char AT_POST_TITLE_DATA_LOGGER1[] = "POST /api/sensor/loggersensordatath HTTP/1.1\r\n#";   //               api/file/loggersensordata
//flash unsigned char AT_POST_TITLE_DATA_LOGGER1[] = "POST /api/sensor/loggersensordata24 HTTP/1.1\r\n#";     //same as loggersensordatath just with 24 values each msg
flash unsigned char AT_POST_TITLE_DATA_LOGGER1[] = "POST /api/sensor/LoggerWirelessSensorData HTTP/1.1\r\n#";
flash unsigned char AT_POST_TITLE_GETPRM[] = "POST /api/sensor/sensorupdate HTTP/1.1\r\n#";  //  41          api/file/postparamupdate
flash unsigned char AT_POST_TITLE_CNFRMPRM[] = "POST /api/sensor/sensorupdateconfirmation HTTP/1.1\r\n#";  //api/file/postparamupdateconfirmation
flash unsigned char AT_POST_CONN[] = "Connection: keep-alive\r\n#";                     //24
flash unsigned char AT_POST_TYPE[] = "Content-Type: multipart/form-data; boundary=#";   //52
flash unsigned char AT_POST_HOST[] = "Host: @";         //Host: phytech1.dyndns.org:1011\r\n#";             //32
flash unsigned char AT_POST_LENGTH1[] = "Content-Length: 253\r\n\r\n#";                     //BuildExtParamBuff  253
//flash unsigned char AT_POST_LENGTH1[] = "Content-Length: 226\r\n\r\n#";                     //BuildParamBuf  226
flash unsigned char AT_POST_LENGTH4[] = "Content-Length: @";                     //23
flash unsigned char AT_POST_LENGTH3[] = "Content-Length: 130\r\n\r\n#";                     //23
//flash unsigned char AT_POST_LENGTH5[] = "Content-Length: 144\r\n\r\n#";                     //23
flash unsigned char AT_POST_FILE_HDR1[] = "Content-Disposition: form-data; name=\"file\"; #";             //45
flash unsigned char AT_POST_FILE_HDR_PRM[] = "filename=\"PARAMS.txt\"\r\n#";    //23
flash unsigned char AT_POST_FILE_HDR_DATA[] = "filename=\"DATA.txt\"\r\n#";    //21
flash unsigned char AT_POST_FILE_HDR_GETDATA[] = "filename=\"GETPARAMPOST.txt\"\r\n#";    //29
//flash unsigned char AT_POST_FILE_HDR_LCTN[] = "filename=\"SENSOR_LOCATION.txt\"\r\n#";    //24
flash unsigned char AT_POST_FILE_HDR2[] = "Content-Type: text/plain\r\n\r\n#";          //28
flash unsigned char BACKUP_URL[] = "proxy.backup.phytech.com#0000000";
flash unsigned char MODEM_4D_MODEL[] = "LE910-SVL";

//send flash init string to modem
void SendATCmd(flash unsigned char *bufToSend)
{
    BYTE i;

    i = 0;
    //copy flash string to buff
    while ((bufToSend[i] != cEndMark) && (i < MAX_SBD_BUF_LEN))
    {
         ComBuf[i] = bufToSend[i];
         i++;
    }
    BytesToSend = i ;
    if (modemGSMModel == TRUE)
    {
         if ((modemCurSubTask ==  SUB_TASK_MODEM_CONNECT_PDP_DEF) || (modemCurSubTask == SUB_TASK_MODEM_CONNECT_SETUP1) ||
            (modemCurSubTask ==  SUB_TASK_MODEM_CONNECT_SETUP2) || (modemCurSubTask == SUB_TASK_MODEM_CONNECT_SETUP3) ||
            (modemCurSubTask ==  SUB_TASK_MODEM_CONNECT_ACTV) || (modemCurSubTask == SUB_TASK_MODEM_CONNECT_START_DIAL) ||
            (modemCurSubTask ==  SUB_TASK_MODEM_CLOSE_TCP))
            for (i = 0; i < BytesToSend; i++)
                if ((ComBuf[i] == '=') && (ComBuf[i+1] == '1'))
                {
                    ComBuf[i+1] = '3';
                    break;
                }
    }
    //transmitt to local modem port
    TransmitBuf(0);
}
/*
BYTE GetContextStatus()
{
    BYTE i, b = 0;

    do            //if return "SGACT:"
    {
        if(RxUart0Buf[i++] == 0x2C)
            b = i;
    }
    while ((i < rx0_buff_len) && (b == 0));

    return RxUart0Buf[b];
}
*/
// translate rssi val from string to single byte
char ConvertRssiVal(void)
{
        BYTE  b = 0;
        int i;

        rssi_val = 100;
        for( i = 0; i < rx0_buff_len - 4 ; i++)
        {
            //if return "+CSQ:"
            if((RxUart0Buf[i] == 0x2B) &&
               (RxUart0Buf[i+1] == 0x43) &&
               (RxUart0Buf[i+2] == 0x53) &&
               (RxUart0Buf[i+3] == 0x51) &&
               (RxUart0Buf[i+4] == 0x3A))
            {
                b = 1;
                //serch for ',':
                // if rssi val is only 1 digit - take it as is
                if(RxUart0Buf[i+7] == 0x2C)
                    //rssi val is 1 byte
                    rssi_val = RxUart0Buf[i+6]-0x30;
                   // rssi_val = 0x30 + ComBuf[i+6];
                else
                    // if its 2 digits:
                    if(RxUart0Buf[i+8] == 0x2C)
                    {
                        rssi_val = RxUart0Buf[i+6]-0x30;
                        rssi_val *= 10;
                        rssi_val += RxUart0Buf[i+7]-0x30;
                    }
                break;
            }
        }
        // no answer found (+CSQ:)
        if (b == 0)
            return FALSE;

        // if rssi is very low:
        if (rssi_val < 10)
        {
            // if should have done only data - send also params
            if (toDoList == DO_DATA)
                toDoList = DO_DATA_N_PRMS;
        }

        return TRUE;
}

void TurnOnIgnition()
{
    #ifdef DebugMode
    SendDebugMsg("\r\nTurnOnIgnition");
    #endif DebugMode
    MODEM_3G_IGNITION_ON();
    initCnt++;
    ModemResponse = TASK_COMPLETE;
}

void TurnOffIgnition()
{
    #ifdef DebugMode
    SendDebugMsg("\r\nTurnOffIgnition");
    #endif DebugMode
    MODEM_3G_IGNITION_OFF();
    ModemResponse = TASK_COMPLETE;
}

void ModemHwShdn()
{
    MODEM_3G_SHUTDOWN_START();
    delay_ms(200);
    MODEM_3G_SHUTDOWN_STOP();
    ModemResponse = TASK_COMPLETE;
    #ifdef DebugMode
    SendDebugMsg("\r\nModemHwShdn");
    #endif DebugMode
}

char IsModemOn()     //MODEM_3G_PWR_MON
{
//return TRUE;
    if (PINC.7 == 0)
    {
    #ifdef DebugMode
    SendDebugMsg("\r\nModemIsOn\r\n");
    #endif DebugMode
        return TRUE;
    }
    #ifdef DebugMode
    SendDebugMsg("\r\nModemIsOff");
    #endif DebugMode
    return FALSE;
}

/*void SetModemOff()
{
//    BYTE stat = TRUE;
//    int n =  0;
    #ifdef DebugMode
    SendDebugMsg("\r\nSetModemOff");
    #endif DebugMode
//    do
//    {
//        delay_ms(500);
//        stat = IsModemOn();
//        n++;
//    }
//    while ((stat != FALSE) && (n < 30)) ;
//    if (stat == TRUE)
//        ModemHwShdn();
    GSM_POWER_OFF();
     ModemResponse = TASK_COMPLETE;
}
*/

//send init string AT+WIPCREATE=2,1,"109.253.23.173",1007 to mode
void SendString(unsigned char *bufToSend, BYTE bufLen)
{
    BYTE i;

    for (i = 0; i < bufLen; i++)
        ComBuf[i] = bufToSend[i];
        //transmitt to local modem port
    BytesToSend = bufLen;
    TransmitBuf(0);
}

void SendPostHost()
{
    BYTE i, n;
    //build the host address from url+port
    n = CopyFlashToBuf(ComBuf, AT_POST_HOST);
    i = 0;
    while (eIPorURLval1[i] != HASHTAG)
    {
        ComBuf[i+n] = eIPorURLval1[i];
        i++;
    }

    n += i;
    ComBuf[n++] = ':';
    cpu_e2_to_MemCopy(&ComBuf[n], ePORTval1, 4);
    n += 4;
    ComBuf[n++] = '\r';
    ComBuf[n++] = '\n';
    BytesToSend = n;
    TransmitBuf(0);
}

void PutString(unsigned char* dest, unsigned char* src, int len)
 {
    int i;
    for (i = 0; i <len; i++)
        dest[i] = src[i];
 }

// build the string of parameters to send to web service
void BuildExtParamsBuff()
{
    int index;
    BYTE cs, i;
    char buf[4];

    index = 0;
    // ID
    cpu_e2_to_MemCopy( &ComBuf[index], &eLoggerID[0], 4);
    index += 4;
    // URL
    cpu_e2_to_MemCopy( &ComBuf[index], eIPorURLval1, 32);
    index += 32;
    // APN
    cpu_e2_to_MemCopy( &ComBuf[index], eAPN, 32);
    index += 32;
    //Port
    cpu_e2_to_MemCopy( &ComBuf[index], ePORTval1, 4);
    index += 4;

// ICCID num.
    for (i = 0; i < MAX_ICCID_LEN; i++)
        ComBuf[index++] = ICCID[i];

    //MemCopy(&GPRSBuf[index],ICCID, MAX_ICCID_LEN);
    //index += MAX_ICCID_LEN;

    // Wakeup timing
    ComBuf[index++] = eStartConnectionH;
    ComBuf[index++] = eConnectionInDay;
    ComBuf[index++] = eConnectIntervalH;
    // RSSI
    ComBuf[index++] = rssi_val;
    //Battery
    ComBuf[index++] = (unsigned char)((iVoltage >> 8) & 0xFF);     //address high
    ComBuf[index++] = (unsigned char)(iVoltage) ;                 //address low
//    int2bytes(iVoltage, &GPRSBuf[index++]);
//    index++;
    //location :
    Float2Bytes(g_fLat, buf);
    MemCopy(&ComBuf[index], buf  ,4);
    index+= 4;

    Float2Bytes(g_fLon, buf);
    MemCopy(&ComBuf[index], buf  ,4);
    index+= 4;

    Float2Bytes(0, buf);
    MemCopy(&ComBuf[index], buf  ,4);
    index+= 4;

    //roaming
    ComBuf[index++] = eUseCntrCode;
    ComBuf[index++] = eRoamingDelay;

    // mobile net code
    cpu_e2_to_MemCopy( &ComBuf[index], eMobileNetCode, 4);
    index += 4;

    // mobile country code
    cpu_e2_to_MemCopy( &ComBuf[index], eMobileCntryCode, 4);
    index += 4;

    // Cell ID
    for (i = 0; i < 5; i++)
        ComBuf[index++] = CellID[i];

    // Clock
    e2_writeFlag = 0; //enable reading the rtc
    GetRealTime();

    ComBuf[index++] = readClockBuf[0]; //year
    ComBuf[index++] = readClockBuf[1]; //month
    ComBuf[index++] = readClockBuf[2]; //day
    ComBuf[index++] = readClockBuf[4]; //hour
    ComBuf[index++] = readClockBuf[5]; //minute

    // Version
    cpu_flash_to_MemCopy(&ComBuf[index], RomVersion, 4);
    index += 4;

    //num of sensors connected to this logger       // todo: insert when service is ready
//    ComBuf[index++] = eNumSensors;

    //check sum
    cs = CheckSum(ComBuf, index, 1);
    ComBuf[index++] = cs;

    ComBuf[index++] = '\r';
    ComBuf[index++] = '\n';
    BytesToSend = index;
}

void SendPostUpdate(BYTE bGetOrConfirm)
{
    char endFileStr[10];
    BYTE cs;

    // preare end file mark string
    endFileStr[0] = '-';
    endFileStr[1] = '-';
    cpu_flash_to_MemCopy(&endFileStr[2], PHYTECH_FILE_END_MARK, 8);

    bNeedToWait4Answer = FALSE;
    if (bGetOrConfirm)
        SendATCmd(AT_POST_TITLE_GETPRM);
    else
        SendATCmd(AT_POST_TITLE_CNFRMPRM);
    SendATCmd(AT_POST_CONN);
    SendATCmd(AT_POST_TYPE);
    SendATCmd(PHYTECH_FILE_END_MARK);
    //GPRS_send_init_flash_string(AT_POST_HOST);
    SendPostHost();
    SendATCmd(AT_POST_LENGTH3);

    // send file header:
    //GPRS_send_init_flash_string(PHYTECH_FILE_END_MARK);
    SendString(endFileStr, 10);            //10
    SendATCmd(AT_POST_FILE_HDR1);       //45
    SendATCmd(AT_POST_FILE_HDR_GETDATA);    //29
    SendATCmd(AT_POST_FILE_HDR2);       //28

    // build post body
    // if confirming update logger ID
    if ((bGetOrConfirm == CONFIRM_UPDATE) && (prmUpdtIndex == 18))
        MemCopy(ComBuf, prevID, 4);        //send previous logger ID
    // all other cases - send ID
    else
        cpu_e2_to_MemCopy(ComBuf, &eLoggerID[0], 4);        //
    ComBuf[4] = prmUpdtIndex;                           //index+checksum+\r+\n = 4
    //check sum
    cs = CheckSum(ComBuf, 5, 1);
    //GPRSBuf[5] = 0;
    ComBuf[5] = cs;
    ComBuf[6] = '\r';
    ComBuf[7] = '\n';
    // send post
    BytesToSend = 8;
    TransmitBuf(0);                //8
    bNeedToWait4Answer = TRUE;
    SendString(endFileStr, 10);            //10
//    lastByteIndex = 0;
    longAnswerExpected = 1;
    overFlow = 0;
}

// build the string of data to send to web service
// for 1st service, func "sensordata": length was 63
// for extended service (Nir), func "loggersensordata": length was  58
// for service with limites: func "loggersensordatath": length is: 66  (limits+strips)
// to be: for service of wireless sensors: "LoggerWirelessSensorData". length is 70
void BuildDataStr()
{
    int index;
    BYTE cs;
    char buf[2];
    char b[4];
    unsigned char cLatLon[8];

//    #ifdef DebugMode
//    SendDebugMsg("\r\nBuildDataStr \0");
//    SendDebugMsg("\r\nobjToMsr= \0");
//    PrintNum(objToMsr);
//    SendDebugMsg("\r\nID= \0");
//    PrintNum((WLSenArr[objToMsr]).Id);
//    #endif DebugMode
    index = 0;
    // ID
    Long2Bytes(WLSenArr[objToMsr].Id, b);
    MemCopy( &ComBuf[index], b, 4);
    index += 4;

    //Type
    ComBuf[index++] = GetSensorType();     // set id of type

    // interval
    ComBuf[index++] = GetSensorInterval(GetSensorType());  //DataBlock[1]; //interval of block

    // send logger ID
    cpu_e2_to_MemCopy( &ComBuf[index], &eLoggerID[0], 4);
    index += 4;

    // min & max values
    int2bytes(MIN_LIMIT[objToMsr], buf);
    MemCopy( &ComBuf[index], buf, 2);
    index += 2;

    int2bytes(MAX_LIMIT[objToMsr], buf);
    MemCopy( &ComBuf[index], buf, 2);
    index += 2;

    // min & max strip values
    int2bytes(MIN_STRIP[objToMsr], buf);
    MemCopy( &ComBuf[index], buf, 2);
    index += 2;

    int2bytes(MAX_STRIP[objToMsr], buf);
    MemCopy( &ComBuf[index], buf, 2);
    index += 2;

        // send longitude & latitude
    if (GetMapRefAddress(cLatLon) == TRUE)
        MemCopy(&ComBuf[index], cLatLon, 8);
    index += 8;

    // customer id
//    int2bytes(WLSenArr[objToMsr].Client, buf);
    MemCopy( &ComBuf[index], buf, 2);
    index += 2;

    // RSSI
    ComBuf[index++] = WLSenArr[objToMsr].Rssi;

    // Battery
    //ComBuf[index++] = WLSenArr[objToMsr].Volt;
    int2bytes(WLSenArr[objToMsr].Volt, buf);
    MemCopy( &ComBuf[index], buf, 2);
    index += 2;

    // Timestamp
    PutString(&ComBuf[index],&DataBlock[3], 5);         //DataBlock[10] - used to be cos offset of 7 from unknown reasen
    index += 5;
    // values:
    PutString(&ComBuf[index],&DataBlock[8], MAX_DATA_PER_PCKT*2);    //48
    index += MAX_DATA_PER_PCKT*2;

    //check sum
    cs = CheckSum(ComBuf, index, 1);
    ComBuf[index++] = cs;

    ComBuf[index++] = '\r';
    ComBuf[index++] = '\n';
    BytesToSend = index;
//    return DataBlock[8];
}

/*void BuildLocationStr()
{
    int index;
    BYTE cs;
    unsigned char cLonLat[8];
    char b[4];

//    #ifdef DebugMode
//    SendDebugMsg("\r\nBuildLocationStr \0");
//    SendDebugMsg("\r\nobjToMsr= \0");
//    PrintNum(objToMsr);
//    SendDebugMsg("\r\nID= \0");
//    PrintNum((WLSenArr[objToMsr]).Id);
//    #endif DebugMode
    index = 0;
    // ID
    Long2Bytes(WLSenArr[objToMsr].Id, b);
    MemCopy( &ComBuf[index], b, 4);
    index += 4;

    // send logger ID
    cpu_e2_to_MemCopy( &ComBuf[index], &eLoggerID[0], 4);
    index += 4;

    // send longitude & latitude
    if (GetMapRefAddress(cLonLat) == TRUE)
        MemCopy(&ComBuf[index], cLonLat, 8);
    index += 8;

        //check sum
    cs = CheckSum(ComBuf, index, 1);
    ComBuf[index++] = cs;

    ComBuf[index++] = '\r';
    ComBuf[index++] = '\n';
    BytesToSend = index;
}
*/
void SendPostParam()
{
     char endFileStr[10];

     // sign that for all next transmits - no need to wait for ana answer from modem
     bNeedToWait4Answer = FALSE;
//     bSendData = 1;
     // preare end file mark string
     endFileStr[0] = '-';
     endFileStr[1] = '-';
     cpu_flash_to_MemCopy(&endFileStr[2], PHYTECH_FILE_END_MARK, 8);

     // send post header
     SendATCmd(AT_POST_TITLE_PRM);
     SendATCmd(AT_POST_CONN);
     SendATCmd(AT_POST_TYPE);
     SendATCmd(PHYTECH_FILE_END_MARK);
     //GPRS_send_init_flash_string(AT_POST_HOST);
     SendPostHost();
     SendATCmd(AT_POST_LENGTH1);

     // send file header:
     SendString(endFileStr, 10);         //10
     SendATCmd(AT_POST_FILE_HDR1);       //45
     SendATCmd(AT_POST_FILE_HDR_PRM);    //23
     SendATCmd(AT_POST_FILE_HDR2);       //28

     // build post body                                    //110
//     BuildParamsBuff();
     BuildExtParamsBuff();  //137       change MAX_TX_BUF_LEN to > 137  & AT_POST_LENGTH1 to fit size
     // send post
     //GPRS_send_init_ram_string(GPRSBuf, 110);

     TransmitBuf(0);
     bNeedToWait4Answer = TRUE;
     SendString(endFileStr, 10);            //10

//     lastByteIndex = 0;
     longAnswerExpected = 1;
     overFlow = 0;
}

/*void SendPostLocation()
{
     char endFileStr[10];

     // sign that for all next transmits - no need to wait for ana answer from modem
     bNeedToWait4Answer = FALSE;
//     bSendData = 1;
     // preare end file mark string
     endFileStr[0] = '-';
     endFileStr[1] = '-';
     cpu_flash_to_MemCopy(&endFileStr[2], PHYTECH_FILE_END_MARK, 8);

     // send post header
     SendATCmd(AT_POST_TITLE_LOCATION);
     SendATCmd(AT_POST_CONN);
     SendATCmd(AT_POST_TYPE);
     SendATCmd(PHYTECH_FILE_END_MARK);
     //GPRS_send_init_flash_string(AT_POST_HOST);
     SendPostHost();
     SendATCmd(AT_POST_LENGTH5);

     // send file header:
     SendString(endFileStr, 10);         //10
     SendATCmd(AT_POST_FILE_HDR1);       //45
     SendATCmd(AT_POST_FILE_HDR_LCTN);    //32
     SendATCmd(AT_POST_FILE_HDR2);       //28

     // build post body
     BuildLocationStr();               //4+4+8+3=19
     TransmitBuf(0);
     bNeedToWait4Answer = TRUE;
     SendString(endFileStr, 10);            //10

//     lastByteIndex = 0;
     longAnswerExpected = 1;
     overFlow = 0;
}
*/
void SendPostLength(int len)
{
    BYTE i, n;
    char s[4];
    //build the host address from url+port
    n = CopyFlashToBuf(ComBuf, AT_POST_LENGTH4);
    i = 0;
    do
    {
        s[i++] = (char)(len % 10);
        len = len / 10;
    }
    while (len > 0);

    for (; i > 0; i--)
       ComBuf[n++] = s[i-1] + 48;

    ComBuf[n++] = '\r';
    ComBuf[n++] = '\n';
    ComBuf[n++] = '\r';
    ComBuf[n++] = '\n';
    BytesToSend = n;
    TransmitBuf(0);
}
//changed to 170 on 22/04/2014 for tresh hold limits (before it was 162)
//change to 178 on 15/12/2015 - number of values each packet change from 20 to 24 (before it was 170)
void SendPostData(/*BYTE readMode*/)
{
    char endFileStr[10];
    char res, n;
    int num;

    res = GetMeasurments(1);
    if (res == FALSE)
    {
        ModemResponse = TASK_FAILED;
        #ifdef DebugMode
        SendDebugMsg("\r\nGetMeasurments failed \0");
        #endif DebugMode
        return;
    }

    if (res == NO_DATA)
    {
        ModemResponse = TASK_COMPLETE;
        #ifdef DebugMode
        SendDebugMsg("\r\nGetMeasurments returned no data \0");
        #endif DebugMode
        return;
    }
    // preare end file mark string
    endFileStr[0] = '-';
    endFileStr[1] = '-';
    cpu_flash_to_MemCopy(&endFileStr[2], PHYTECH_FILE_END_MARK, 8);
    // sign that for all next transmits - no need to wait for ana answer from modem
    bNeedToWait4Answer = FALSE;
    // send post header
    SendATCmd(AT_POST_TITLE_DATA_LOGGER1);
    SendATCmd(AT_POST_CONN);
    SendATCmd(AT_POST_TYPE);
    SendATCmd(PHYTECH_FILE_END_MARK);
    //GPRS_send_init_flash_string(AT_POST_HOST);
    SendPostHost();
    //calc the length
    // todo: change to 191 for location data
    num = nUnreadBlocks * 191;  //change to 191 at 15/2/17 for lang & lon.  change to 183 at 16/5/16 for 24 values per packet.. change to 174 at 25/1/16 for wireless. before was 170 changed 22/04/2014 for tresh hold limits (before it was 162)
    num += 10;
    SendPostLength(num);
    do
    {
        SendString(endFileStr, 10);             // 10 bytes
        SendATCmd(AT_POST_FILE_HDR1);           // 45
        SendATCmd(AT_POST_FILE_HDR_DATA);       // 21
        SendATCmd(AT_POST_FILE_HDR2);           // 28
         // build post body
        BuildDataStr();  //87 with location
        //send post
        TransmitBuf(0);
        nUnreadBlocks--;
        n = 0;
        do
        {
            if (n == 0)
                res = GetMeasurments(2);
            else
                res = GetMeasurments(3);
            n++;
        }
        while ((res == FALSE) && (n < 3));

    }
    while (nUnreadBlocks);

    //GPRS_send_init_ram_string(GPRSBuf, 63);
    bNeedToWait4Answer = TRUE;
    SendString(endFileStr, 10);                    //10
//    lastByteIndex = 0;
    longAnswerExpected = 1;
    overFlow = 0;
}

BYTE IsPrmToUpdate()
{
    BYTE i;
    for (i = 0; i < MAX_PRM_TASKS; i++)
        if (UpdatePrmArr[i] == '1')
        {
            prmUpdtIndex = i;
            return TRUE;
        }
    return FALSE;
}

BYTE GetBufferIndex(int i)
{
    BYTE b;

    if (i >= MAX_RX_BUF_LEN)
        b = (BYTE)i - MAX_RX_BUF_LEN;
    else
        b = i;
    return b;
}

BYTE IsOK()
{
    int index = 0;
    while (index < rx0_buff_len-1)
    {
        if ((RxUart0Buf[index] == 'O') && (RxUart0Buf[index+1] == 'K'))
        {
            return TRUE;
        }
        else
            index++;
    }
    return FALSE;
}

//+COPS:
BYTE IsCOPS()
{
    int index = 0;
    while (index < rx0_buff_len-1)
    {
        if ((RxUart0Buf[index] == '+') && (RxUart0Buf[index+1] == 'C') &&
        (RxUart0Buf[index+2] == 'O') && (RxUart0Buf[index+3] == 'P') &&
        (RxUart0Buf[index+4] == 'S') && (RxUart0Buf[index+5] == ':'))
        {
            return TRUE;
        }
        else
            index++;
    }
    return FALSE;
}

BYTE IsConnect()
{
    int index = 0;
    while (index < rx0_buff_len-4)
    {
        if ((RxUart0Buf[index] == 'C') && (RxUart0Buf[index+1] == 'O') && (RxUart0Buf[index+2] == 'N') && (RxUart0Buf[index+3] == 'N'))
            return TRUE;
        else
            index++;
    }
    return FALSE;
}

BYTE IsSimDetected()
{
    BYTE index = 0;

    while (index < rx0_buff_len)
    {
        if (RxUart0Buf[index++] == ',')
            break;
    }
    // if , was found - lookon the digitt after:
    if (index < rx0_buff_len)
        if (RxUart0Buf[index] == '1')
            return TRUE;
    return FALSE;
}

BYTE IsRegistOK()
{
    int index = 0;
    //find the , (comma \ psik)
    while (index < rx0_buff_len)
    {
        if (RxUart0Buf[index++] == ',')
            break;
    }
    // if , was found - lookon the digitt after:
    if (index < rx0_buff_len)
    {
        if ((RxUart0Buf[index] == '1') || (RxUart0Buf[index] == '5'))
        {
            //TurnOnLed(LED_2, SUCCESS);
            return TRUE;
        }
        if (RxUart0Buf[index] == '3')
            nRegDenied++;
        else
            nRegDenied = 0;
        if (nRegDenied >= (nMaxFailuresNum / 2))
        {
            failCnt = nMaxFailuresNum;
            nRegDenied = 0;
        }
    }
    return FALSE;
}

void GetCellID()
{
    int n, i = 0;
    if (!IsOK())
        return;
    //find begining of SIM num
    while (!((RxUart0Buf[i] == 'I') && (RxUart0Buf[i+1] == 'd') && (RxUart0Buf[i+2] == ':')) && (i < rx0_buff_len))
        i++;
    i += 3;
    n = 0;
    do
    {
        CellID[n++] = RxUart0Buf[i++];
    }
    while ((RxUart0Buf[i] != ' ') && (n < 5));

    for (; n < 5; n++)
        CellID[n] = '*';
}

void GetICCID()
{
    int n, i = 0;
    if (!IsOK())
        return;
    //find begining of SIM num
    while ((RxUart0Buf[i] < '0') || (RxUart0Buf[i] > '9'))
        i++;
     n = 0;
    do
    {
        ICCID[n++] = RxUart0Buf[i++];
    }
    while ((RxUart0Buf[i] >= '0') && (RxUart0Buf[i] <= '9') && (n < 20));
    for (; n < 20; n++)
        ICCID[n] = '*';
    if (eUseCntrCode == 0)
        if (((ICCID[2] == '3') && (ICCID[3] == '4') && (ICCID[4] == '0')) || ((ICCID[2] == '4') && (ICCID[3] == '4') && (ICCID[4] == '5')))     // if sim is movustar and user hasnt defined specific operator to use
        {
            isRoaming = TRUE;
            #ifdef DebugMode
            SendDebugMsg("\r\nneed roaming \0");
            #endif DebugMode
        }
}

int ParseSnsrMinMax(char* sID, BYTE lmt)
{
    int i = 0, j = 0;
    long l = 0, lID = 0;
    int min,max;
    BYTE  nSenNum, res = 0;;

    nSenNum = sID[0];
    if (nSenNum > MAX_SEN_NUM)
        return -1;
    #ifdef DebugMode
    SendDebugMsg("\r\nNum sensor to set limits: ");
    PrintNum(nSenNum);
    #endif DebugMode
    for (j = 0; j < nSenNum; j++)
    {
        lID = Bytes2Long(&sID[j*8 + 1]);

        #ifdef DebugMode
        SendDebugMsg("\r\nSensor to change limits: ");
        PrintNum(lID);
        #endif DebugMode
        for (i = SENSOR1; i <= eNumSensors; i++)
        {
            //MemCopy(s, &WirelessSenID[i*4], 4);
            l = WLSenArr[i].Id; //Bytes2Long(s);
//            #ifdef DebugMode
//            SendDebugMsg("\r\nSensor  ");
//            PrintNum(l);
//            #endif DebugMode
            if (l == lID)
            {
                #ifdef DebugMode
                SendDebugMsg("\r\nfound sensor: ");
                PrintNum(i);
                #endif DebugMode
                min = bytes2int(&sID[j*8 + 5]);  //
                max = bytes2int(&sID[j*8 + 7]);
                #ifdef DebugMode
                SendDebugMsg("\r\nMin value: ");
                PrintNum(min);
                SendDebugMsg("\r\nMax value: ");
                PrintNum(max);
                #endif DebugMode
                // if function was called to update limits
                if (lmt == 0)
                {
                    if (min < max)
                    {
                        MIN_LIMIT[i] = min;
                        MAX_LIMIT[i] = max;
                        //SetDefaultStrip(i)
                        if (MIN_LIMIT[i] == MIN_INT)
                            MIN_STRIP[i] = MIN_INT;
                        else
                            MIN_STRIP[i] = min + 1;
                        if (MAX_LIMIT[i] == MAX_INT)
                            MAX_STRIP[i] = MAX_INT;
                        else
                            MAX_STRIP[i] = max - 1;
                        // init all alerts flags of this sensor
                        Alerts[i].Status = ALERT_WAIT;
                        Alerts[i].OutOfLmtCnt = 0;
                        Alerts[i].MsrCnt = 0;
                        res++;
                    }
                }
                else  //if function was called to update strip
                {
                    // make sure that strip is in between limits
                    if ((min >= MIN_LIMIT[i]) && (max <= MAX_LIMIT[i]))
                    {
                        MIN_STRIP[i] = min;
                        MAX_STRIP[i] = max;
                        res++;
                    }
                }
                continue;
            }
        }
    }
    if (res != nSenNum)   // if num of sensor was defined is different from num of sensor was sent - something wrong
        return -1;
    return 1;
}

char UpdateParam()
{
    char new_data_buf[32];
    BYTE n, dataLen;
    char res = TRUE;
    int nID;
    signed char x;

    if ((prmUpdtIndex != 12) && (prmUpdtIndex != 13))
        dataLen = PrmLen[prmUpdtIndex];
    else
    {
        dataLen = 1 + RxUart0Buf[bufIndexToUpd] * 8;
    }
    // if result buffer overFlow - meanning can be split at end of buffer and start of it
    if (bufIndexToUpd + dataLen/*PrmLen[prmUpdtIndex]*/ > MAX_RX_BUF_LEN)
    {
        for ( n = 0; n < dataLen/*PrmLen[prmUpdtIndex]*/; n++)
            new_data_buf[n] = RxUart0Buf[GetBufferIndex(bufIndexToUpd++)];
    }
    else
    {
        for ( n = 0; n < dataLen /*PrmLen[prmUpdtIndex]*/; n++)
            new_data_buf[n] = RxUart0Buf[bufIndexToUpd++];
    }

    switch (prmUpdtIndex)
    {
        case UPDATE_URL:     // Server_IP_URL  1
//            if (res == TRUE)
            {
                if (bUpdateAddress)                   // if updating url & port - save new url and wait till the new port will come.
                    MemCopy(newURL, new_data_buf, 32);
                else
                    MemCopy_to_cpu_e2(eIPorURLval1, new_data_buf, 32);  //len might varied
            }
            break;
        case UPDATE_PORT:      // Server_Port  2
            for (n = 0; n < 4; n++)
                if ((new_data_buf[n] < '0') || (new_data_buf[n] > '9'))  //if port is not a number
                    res = FALSE;
            if (res == TRUE)
            {                                          // if all 4 digits are numbers - replace port
                if (bUpdateAddress)                                       // if updating url & port
                    MemCopy_to_cpu_e2(eIPorURLval1, newURL, 32);    // now replace the url - after make sure port comes ok
                MemCopy_to_cpu_e2(ePORTval1, new_data_buf, 4);
            }
            break;
        case UPDATE_APN:     // Access_Point_Name  3
            MemCopy_to_cpu_e2(eAPN, new_data_buf, 32);
        break;
        case UPDATE_DL_START:        //Start_DL  7
            if ((new_data_buf[0] >= 0) &&(new_data_buf[0] <= 23))
                eStartConnectionH = new_data_buf[0];
            else
                res = FALSE;
        break;
        case UPDATE_DL_CYCLE:        //DL_Cycles 8
            if ((new_data_buf[0] > 0) && (new_data_buf[0] <= 24))
                eConnectionInDay = new_data_buf[0];
            else
                res = FALSE;
        break;
        case UPDATE_DL_INTERVAL:        //DL_Interval 9
            if ((new_data_buf[0] >= 0) && (new_data_buf[0] < 24))
                eConnectIntervalH = new_data_buf[0];
            else
                res = FALSE;
        break;
        case UPDATE_COMMAND:       //reset memory (used to be GMT) 10
            if (new_data_buf[0] == COMMAND_INIT_MEMORY)
                res = InitDataBlocks();
            else
                if (new_data_buf[0] == COMMAND_MAKE_RESET)
                    bMakeReset = TRUE;
                else
                    if (new_data_buf[0] == COMMAND_INIT_RESET)
                    {
                        res = InitDataBlocks();
                        bMakeReset = TRUE;
                    }
                    else
                        if (new_data_buf[0] == COMMAND_RTC_24)
                            SetRtc24Hour();
                        else
                            if (new_data_buf[0] == COMMAND_INIT_MEM_SENSORS)
                            {
                                InitDataBlocks();
                                InitSensorArray();
                            }
                            else
                                res = FALSE;
            break;
            case UPDATE_LIMITS:       // setup min & max limits    12
                nID = ParseSnsrMinMax(new_data_buf, 0);
                if (nID == -1)
                    res = FALSE;
            break;
            case UPDATE_ARMED_LIMITS:       // setup min & max armed limits      13
                nID = ParseSnsrMinMax(new_data_buf, 1);
                if (nID == -1)
                    res = FALSE;
            break;
            case UPDATE_USE_CC:        //USE_COUNTRY_CODE    14
                if ((new_data_buf[0] == 0) || (new_data_buf[0] == 1))
                    eUseCntrCode = new_data_buf[0];
                else
                    res = FALSE;
            break;
            case UPDATE_ROAMING_DLY:        //ROAMING_DELAY  15
                x = (signed char)(new_data_buf[0]);
                if ((x >= 0) && (x <= 60))
                    eRoamingDelay = new_data_buf[0];
                else
                    res = FALSE;
            break;
            case UPDATE_MNC:         //MOBILE_NET_CODE   16
                MemCopy_to_cpu_e2(eMobileNetCode, new_data_buf, 4);
            break;
            case UPDATE_MCC:         //MOBILE_COUNTRY_CODE  17
                MemCopy_to_cpu_e2(eMobileCntryCode, new_data_buf, 4);
            break;
            case UPDATE_ID:          // sensors ID    18
                // save previous Logger ID for confirm
                cpu_e2_to_MemCopy( prevID, &eLoggerID[0], 4);
                MemCopy_to_cpu_e2(&eLoggerID[0], new_data_buf  ,4);
                // also make reset - so the new logger will register in db.
                bMakeReset = TRUE;
            break;
        default:
            res = FALSE;
    }
    return res;
}

/*
float Str2Float(char* s)
{
    float d = 0;
    BYTE isPstv = TRUE;
    int i = 0;
    unsigned long l = 0, right2DecPoint = 0;

//    #ifdef DebugMode
//    SendDebugMsg("\r\nStr2Float");
//    #endif DebugMode
    if (s[0] == '-')
    {
        isPstv = FALSE;
        i++;
    }
    while (s[i] != HASHTAG)
    {
        if (s[i] == '.')
            right2DecPoint = 1;
        else
        {
            l*=10;
            l = l + (s[i] - 0x30);
            right2DecPoint *= 10;
        }
        i++;
    }
//    #ifdef DebugMode
//    SendDebugMsg("\r\nnumber is ");
//    PrintNum(l);
//    PrintNum(right2DecPoint);
//    #endif DebugMode
    d = ((float)l / right2DecPoint);
    if (!isPstv)
        d *= -1;
    return d;
}
*/

long Str2Long(char* s)
{
    long l = 0;
    int i = 0;
    BYTE isPos = TRUE;

    i = 0;

    if (s[0] == '-')
    {
        isPos = FALSE;
        i++;
    }
    while ((s[i] != HASHTAG) && (s[i] >= '0') && (s[i] <= '9'))
    {
        l*=10;
        l = l + (s[i] - 0x30);
        i++;
    }
    if (!isPos)
        l *= -1;
    return l;
}

/*
BYTE GetPosition()
{
    int n, i = 0, c = 0;
    char tmp[12];

    if (!IsOK())
        return FALSE;
    //find begining of SIM num
    while ((RxUart0Buf[i] != ',') && (i < rx0_buff_len))
        i++;
    i++;
    n = 0;
    do
    {
//        #ifdef DebugMode
//        putchar1(RxUart0Buf[i]);
//        #endif DebugMode
        tmp[n++] = RxUart0Buf[i++];
        if (RxUart0Buf[i] == ',')
        {
            c++;
            tmp[n] = HASHTAG;
            if (c == 1)
            {
                fLtd = Str2Float(tmp);
                // check number is legal:
                if ((fLtd > 90) || (fLtd < -90))
                    fLtd = 0;
            }
            if (c == 2)
            {
                fLng = Str2Float(tmp);
                if ((fLng > 180) || (fLng < -180))
                    fLng = 0;
            }
            if (c == 4)
                fUncr = Str2Long(tmp);
            n = 0;
            i++;
        }
    }
    while ((c < 4) && (i < rx0_buff_len) && (n < 35));
    return TRUE;
}
*/
BYTE IsOperatorExist(long lNewOp, char nTotalNum )
{
    char i;
    for (i = 1; i <= nTotalNum; i++)
        if (lNewOp == OprtTbl[i].MccMnc)
            return TRUE;
    return FALSE;
}
/*
AT+COPS=?
.           op name    op num
.+COPS: (2,"Cellcom",,"42502",2),(2,"Cellcom",,"42502",0),(3,"Orange IL",,"42501",2),(3,"IL Pelephone",,"42503",2),(3,"Orange IL",,"42501",0),(3,"PS, Wataniya Mobile",,"42506",0),,(0-4),(0,2)
.
.OK
*/
void ParseCopsLst()
{
    int n, i = 0;//, c;
    char tmp1[12];
    char tmp2[12];
    BYTE stat, acT;//, nExist;
    char index = 0;
    long l;

    n = 0;
    do
    {
        while ((RxUart0Buf[i] != '(') && (i < rx0_buff_len))
            i++;

        n = 0;
        i++;
        stat = RxUart0Buf[i];
//        #ifdef DebugMode
//        SendDebugMsg("\r\nstatus is \0 ");
//        putchar1(stat);
//        SendDebugMsg("\r\noperator name \0");
//        #endif DebugMode
        i += 3;
        // get operator name
        while ((RxUart0Buf[i] != '"') && (i < rx0_buff_len))
        {
            tmp1[n++]  = RxUart0Buf[i++];
        }
        if (i == rx0_buff_len)
            break;
        tmp1[n] = HASHTAG;
        // get operator number
        n = 0;
        //i += 4;
        i++;
        while ((RxUart0Buf[i] != '"') && (i < rx0_buff_len))
            i++;
        i++;
        while ((RxUart0Buf[i] != '"') && (i < rx0_buff_len))
        {
            tmp2[n++]  = RxUart0Buf[i++];
        }
        if (i == rx0_buff_len)
            break;
        tmp2[n] = HASHTAG;
        i += 2;
        acT = RxUart0Buf[i];
        l = Str2Long(tmp2);
        if ((stat != 3) && (IsOperatorExist(l, index) == FALSE) && (l > 0))
        {
            index++;
            OprtTbl[index].Status = stat;
            OprtTbl[index].AccTech = acT;
            OprtTbl[index].MccMnc = l;
        }
    }
    while ((i < rx0_buff_len) && (index < 10));

    numOprt = index;
    #ifdef DebugMode
    SendDebugMsg("\r\nnum operators \0");
    putchar1(numOprt);
    #endif DebugMode
}

// check  if "phy111" is in buffer:
int CheckResult()
{
    int i = 0, n;
    char tmpClock[5];
    int index;
    int nMaxBytesToCheck;

    if (overFlow == 0)                       //if result was less than 64 bytes
        nMaxBytesToCheck = /*lastByteIndex*/rx0_buff_len;    // max bytes to check is num of bytes got in
    else
        nMaxBytesToCheck = MAX_RX_BUF_LEN;
    //look for the string "phy111" to sign its begining of results:
    index = -1;
    do
    {
        if (overFlow == 0)                       //if result was less than 64 bytes
            n = i;
        else
           n = /*lastByteIndex*/rx0_buff_len + i;
        if((RxUart0Buf[GetBufferIndex(n)] == PHYTECH_FILE_END_MARK[0]) &&       // P
        (RxUart0Buf[GetBufferIndex(n+1)] == PHYTECH_FILE_END_MARK[1]) &&        // h
        (RxUart0Buf[GetBufferIndex(n+2)] == PHYTECH_FILE_END_MARK[2]) &&        // y
        (RxUart0Buf[GetBufferIndex(n+3)] == PHYTECH_FILE_END_MARK[3]) &&        // 1
        (RxUart0Buf[GetBufferIndex(n+4)] == PHYTECH_FILE_END_MARK[4]) &&        // 1
        (RxUart0Buf[GetBufferIndex(n+5)] == PHYTECH_FILE_END_MARK[5]))          // 1
            index = GetBufferIndex(n+6);
        i++;
    }
    while ((i < nMaxBytesToCheck - 5) && (index == -1));
//    while ((i < MAX_RX_BUF_LEN - 5) && (index == -1));
    // if hasnt find - return -1, means no data got back

    if (index == -1)
    {
        #ifdef DebugMode
        SendDebugMsg("\r\nPOST not OK\0");
        #endif DebugMode
        return -1;
    }
    bPostAnswered = TRUE;    // sign that post answered - even if failed - its an answer. no need to open different socket. connection is OK.
    // if found the sign of "phy111"- look on next bytes
    // if it "FAILED" string - then it means webservice return failed getting the parameters
    // eelse it is the begining of parameters sent from service, return the index

    i = index;
    if ((RxUart0Buf[GetBufferIndex(i)] == 0x46) &&          // F
        (RxUart0Buf[GetBufferIndex(i+1)] == 0x41) &&        // A
        (RxUart0Buf[GetBufferIndex(i+2)] == 0x49) &&        // I
        (RxUart0Buf[GetBufferIndex(i+3)] == 0x4c) &&        // L
        (RxUart0Buf[GetBufferIndex(i+4)] == 0x45) &&        // E
        (RxUart0Buf[GetBufferIndex(i+5)] == 0x44))          // D
        {
//            if (toDoList == DO_DATA)                // if got failed for data msg - do also params
//                toDoList = DO_DATA_N_PRMS;

            #ifdef DebugMode
            SendDebugMsg("\r\nPOST not OK2\0");
            #endif DebugMode

            return -1;
        }

    // if its parameters post - save the amswer
    if (modemCurSubTask == SUB_TASK_MODEM_POST_PRM)
    {
        for (n = 0; n < 5; n++)
            tmpClock[n] = RxUart0Buf[GetBufferIndex(i++)];
        if (!((tmpClock[0] == 0) && (tmpClock[1] == 0) && (tmpClock[2] == 0) && (tmpClock[3] == 0) && (tmpClock[4] == 0)))
        {
            MemCopy( clockBuf, &tmpClock[0], 3 ); //copy year month day
            MemCopy( &clockBuf[4], &tmpClock[3], 2 );  //copy hour minute
            if(SetRealTime() == FAILURE)
            {
                //
                #ifdef DebugMode
                SendDebugMsg("\r\nSetRealTimeFailed\0");//set real time
                #endif DebugMode
            }
        }
        #ifdef DebugMode
        else
            SendDebugMsg("\r\nskip update clock\0");//set real time
        #endif DebugMode

        #ifdef DebugMode
        GetRealTime();
        SendDebugMsg("\r\nnow:\0");
        putchar1(readClockBuf[0]);
        putchar1(readClockBuf[1]);
        putchar1(readClockBuf[2]);
        putchar1(readClockBuf[4]);
        putchar1(readClockBuf[5]);
        SendDebugMsg("\r\n\0");
        #endif DebugMode

        //i += 5;
        for ( n = 0; n < MAX_PRM_TASKS; n++)
            UpdatePrmArr[n] = RxUart0Buf[GetBufferIndex(i++)];

        if ((UpdatePrmArr[1] == '1') && (UpdatePrmArr[2] == '1'))
            bUpdateAddress = TRUE;
        else
            bUpdateAddress = FALSE;
        bMakeReset = FALSE; // init flag of make reset
    }
    if (modemCurSubTask == SUB_TASK_MODEM_POST_UPD)
    {
        bufIndexToUpd = index;
        if (UpdateParam() != TRUE)
            index = -1;
    }
    // check if there is ok after phy111
    if (modemCurSubTask == SUB_TASK_MODEM_POST_DATA)
    {
        i = index;
        // first - save clock
        for (n = 0; n < 5; n++)
            tmpClock[n] = RxUart0Buf[GetBufferIndex(i++)];
        if (!((tmpClock[0] == -1) && (tmpClock[1] == -1) && (tmpClock[2] == -1) && (tmpClock[3] == -1) && (tmpClock[4] == -1)))
        {
            MemCopy( clockBuf, &tmpClock[0], 3 );       //update year month day
            MemCopy( &clockBuf[4], &tmpClock[3], 2 );   //update hour minute
            if ((toDoList == DO_DATA) && (objToMsr == SENSOR1)) // if its first sensor and no params this time:
                if(SetRealTime() == FAILURE)
                {
                }
        }
        // check if there is PENDING
        if ((RxUart0Buf[GetBufferIndex(i)] == 0x50) &&      //P
        (RxUart0Buf[GetBufferIndex(i+1)] == 0x45) &&        //E
        (RxUart0Buf[GetBufferIndex(i+2)] == 0x4e) &&        //N
        (RxUart0Buf[GetBufferIndex(i+3)] == 0x44) &&        //D
        (RxUart0Buf[GetBufferIndex(i+4)] == 0x49) &&        //I
        (RxUart0Buf[GetBufferIndex(i+5)] == 0x4E) &&        //N
        (RxUart0Buf[GetBufferIndex(i+6)] == 0x47) )         //G
        {
            toDoList = DO_DATA_N_PRMS;
            i += 7;
        }
        // now check for OK
        if (!((RxUart0Buf[GetBufferIndex(i)] == 0x4f) && (RxUart0Buf[GetBufferIndex(i+1)] == 0x4b)))         // OK
        {
        #ifdef DebugMode
        SendDebugMsg("\r\nPOST not OK1\0");
        #endif DebugMode
        return -1;
        }
    }
//    if (modemCurSubTask == SUB_TASK_MODEM_POST_LOCATION)
//        WLSenArr[objToMsr].LocationStatus = SEND_LOCATION;
    #ifdef DebugMode
    SendDebugMsg("\r\nPOSTOK\0");
    #endif DebugMode

    return index;
}

/*
BYTE GetTime()
{
    int i = 0;
    char sign = 0, tz;
    signed char utcH, utcM, index = 0;
    char dst = 0;//Daylight Saving Time
//yy/MM/dd,hh:mm:ss±zz,d
//#CCLK: "17/02/01,11:56:59+08,0"
//.
//.OK

    if (!IsOK())
        return FALSE;
    //find begining of SIM num
    while ((RxUart0Buf[i] != '"') && (i < rx0_buff_len))
        i++;
    i++;
//    n = 0;
    do
    {
        #ifdef DebugMode
        putchar1(RxUart0Buf[i]);
        #endif DebugMode
        clockBuf[index++] = (10 * (RxUart0Buf[i] - 0x30)) + (RxUart0Buf[i+1] - 0x30);
        i += 3;
//        #ifdef DebugMode
//        SendDebugMsg("\r\n number is ");
//        putchar1(clockBuf[index-1]);
//        #endif DebugMode
        if (index == 2)
            index++;
    }
    while ((index < 6) && (i < rx0_buff_len));

    while (((RxUart0Buf[i] != '-') && (RxUart0Buf[i] != '+')) && (i < rx0_buff_len))
        i++;
    sign = RxUart0Buf[i++];
//    #ifdef DebugMode
//    SendDebugMsg("\r\n sign is ");
//    putchar1(sign);
//    #endif DebugMode
    tz = 10 * (RxUart0Buf[i] - 0x30) + (RxUart0Buf[i+1] - 0x30);
    i += 2;
    if (RxUart0Buf[i] == ',')
        dst = RxUart0Buf[i+1] - 0x30;
//    #ifdef DebugMode
//    SendDebugMsg("\r\n time zone is ");
//    putchar1(tz);
//    SendDebugMsg("\r\n Daylight Saving Time  is ");
//    putchar1(dst);
//    #endif DebugMode
    if (sign == '+')
    {
        gMin4UTC = -QUARTER * (tz % 4);
        gHr4UTC = -1 *(tz / 4);
    }
    else
    {
        gMin4UTC = QUARTER * (tz % 4);
        gHr4UTC = tz / 4;
    }
    gHr4UTC -= dst;

    utcH = clockBuf[4];
    utcM = clockBuf[5];
    if (sign == '+')
        utcM += (tz % 4) * -QUARTER;
    else
        utcM += (tz % 4) * QUARTER;
    if (utcM > 60)
    {
        utcH++;
        utcM -= 60;
    }
    if (utcM < 0)
    {
        utcH--;
        utcM += 60;
    }
    if (sign == '+')
        utcH += -1 *(tz / 4);
    else
        utcH += (tz / 4);
    utcH -= dst;
    if (utcH < 0)
        utcH += 24;
    if (utcH > 23)
        utcH -= 24;
    #ifdef DebugMode
    SendDebugMsg("\r\n UTC time is ");
    PrintNum(utcH);
    PrintNum(utcM);
    putchar1('\r');
    #endif DebugMode
    if(SetRealTime() == FAILURE)
    {
    }
    return TRUE;
}
*/

BYTE Is4dModem()
{
    int index = 0;
    while (index < rx0_buff_len-8)
    {
        if ((RxUart0Buf[index] == MODEM_4D_MODEL[0]) && (RxUart0Buf[index+1] == MODEM_4D_MODEL[1]) &&
        (RxUart0Buf[index+2] == MODEM_4D_MODEL[2]) && (RxUart0Buf[index+3] == MODEM_4D_MODEL[3]) &&
        (RxUart0Buf[index+4] == MODEM_4D_MODEL[4]) && (RxUart0Buf[index+5] == MODEM_4D_MODEL[5]) &&
        (RxUart0Buf[index+6] == MODEM_4D_MODEL[6]) && (RxUart0Buf[index+7] == MODEM_4D_MODEL[7]) &&
        (RxUart0Buf[index+8] == MODEM_4D_MODEL[8]))
        {
            return TRUE;
        }
        else
            index++;
    }
    return FALSE;
}

void ParseModemResponse()
{
    switch (modemCurSubTask)
    {
        case SUB_TASK_INIT_MODEM_OK:
        case SUB_TASK_INIT_MODEM_COPS:
        case SUB_TASK_INIT_MODEM_COPS_MAN:
        case SUB_TASK_INIT_MODEM_GET_COPS:
        case SUB_TASK_INIT_MODEM_RSSI:
        case SUB_TASK_MODEM_CONNECT_ATCH:
        case SUB_TASK_MODEM_CONNECT_SETUP1:
        case SUB_TASK_MODEM_CONNECT_SETUP2:
        case SUB_TASK_MODEM_CONNECT_SETUP3:
        case SUB_TASK_MODEM_CONNECT_PDP_DEF:
//        case SUB_TASK_MODEM_CONNECT_IS_ACTV:
        case SUB_TASK_MODEM_CONNECT_ACTV:
        case SUB_TASK_MODEM_CLOSE_EOD:
        case SUB_TASK_MODEM_CLOSE_TCP:
        case SUB_TASK_MODEM_CLOSE_MDM:
//        case SUB_TASK_INIT_MODEM_EMAP:
//        case SUB_TASK_INIT_MODEM_NITZ:
            if (IsOK() == TRUE)
            {
                ModemResponse = TASK_COMPLETE;
                if (modemCurSubTask == SUB_TASK_INIT_MODEM_RSSI)
                    ConvertRssiVal();
            }
            else
                ModemResponse = TASK_FAILED;
        break;
        case SUB_TASK_INIT_MODEM_COPS_LST:
            if (IsCOPS() == TRUE)
            {
                ModemResponse = TASK_COMPLETE;
                ParseCopsLst();
            }
            else
            {
                ModemResponse = TASK_FAILED;
                fMdmAns = TRUE;
            }
        break;
        case SUB_TASK_INIT_MODEM_REG:
        case SUB_TASK_INIT_MODEM_REG_STAT:
            if (IsRegistOK() == TRUE)
                ModemResponse = TASK_COMPLETE;
            else
                ModemResponse = TASK_FAILED;
        break;
        case SUB_TASK_INIT_MODEM_QSS:
            if ((IsOK() == TRUE) && (IsSimDetected() == TRUE))
                ModemResponse = TASK_COMPLETE;
            else
                ModemResponse = TASK_FAILED;

        break;
        case SUB_TASK_MODEM_CGMM:
            if (IsOK() == TRUE)
            {
                modemGSMModel = Is4dModem();
                ModemResponse = TASK_COMPLETE;
            }
            else
                ModemResponse = TASK_FAILED;

        break;

        case SUB_TASK_INIT_MODEM_MONITOR:
            GetCellID();
            ModemResponse = TASK_COMPLETE;
        break;
        case SUB_TASK_MODEM_CHK_ICCID:
            GetICCID();
            //ConvertIccID();
            ModemResponse = TASK_COMPLETE;
        break;
//        case SUB_TASK_INIT_MODEM_POS:
//            if (rx0_buff_len > 10)
//            {
//                if (GetPosition() == TRUE)
//                    ModemResponse = TASK_COMPLETE;
//                else
//                    ModemResponse = TASK_FAILED;
//            }
//            else
//            {
//                if (IsOK() == TRUE)
//                {
//                    ModemResponse = NO_ANSWER;
//                    TimeLeftForWaiting = 300;
//                    bWaitForModemAnswer = TRUE;
//                    bCheckRxBuf = FALSE;
//                }
//                else
//                    ModemResponse = TASK_FAILED;
//            }
//        break;
//        case SUB_TASK_INIT_MODEM_CCLK:
//            if (GetTime())
//                ModemResponse = TASK_COMPLETE;
//            else
//                ModemResponse = TASK_FAILED;
//        break;
        case SUB_TASK_MODEM_CONNECT_START_DIAL:
            if (IsConnect() == TRUE)
            {
                ModemResponse = TASK_COMPLETE;
                nConnectError = 0;
            }
            else
                ModemResponse = TASK_FAILED;
        break;
        case SUB_TASK_MODEM_POST_PRM:
        case SUB_TASK_MODEM_POST_UPD:
        case SUB_TASK_MODEM_POST_CNFRM:
        case SUB_TASK_MODEM_POST_DATA:
//        case SUB_TASK_MODEM_POST_LOCATION:
            if (CheckResult() == -1)
                ModemResponse = TASK_FAILED;
            else
                ModemResponse = TASK_COMPLETE;
        break;
    }
}

//convert int less than 1000000 to string and send string to uart1
int Long2Str(long val, char* s)
{
    char tmp[10];
    BYTE i = 0, n = 0;
//    #ifdef DebugMode
//    SendDebugMsg("\r\nLong2Str. long is \0");
////    PrintNum(val);
//    #endif DebugMode
    do
    {
        tmp[i++] = (char)(val % 10);
        val = val / 10;
    }
    while (val > 0);
    for (; i > 0; i--)
        s[n++] = tmp[i-1] + 48;
    return n;
}

void SendOperatorSelection()
{
    BYTE i, n;
    //build the cops command  from country+network codes
    n = CopyFlashToBuf(ComBuf, AT_COPS_MAN);
    ComBuf[n++] = '"';
    if (eUseCntrCode == 1)
    {
        i = 0;
        while (eMobileCntryCode[i] != HASHTAG)
        {
            ComBuf[i+n] = eMobileCntryCode[i];
            i++;
        }
        n += i;
        i = 0;
        while (eMobileNetCode[i] != HASHTAG)
        {
            ComBuf[i+n] = eMobileNetCode[i];
            i++;
        }
    }
    else
    {
        curOprt++;
        i = Long2Str(OprtTbl[curOprt].MccMnc, &ComBuf[n]);
    }
    n += i;
    ComBuf[n++] = '"';
    ComBuf[n++] = '\r';
    ComBuf[n++] = '\n';
    BytesToSend = n;
    TransmitBuf(0);
}

void SendStartDial()
{
    BYTE i, n;
    // if connect server failed few times, maybe the address is wrong, - change IP/url address
    //build the cops command  from country+network codes
    n = CopyFlashToBuf(ComBuf, AT_TCP_OPN);
    cpu_e2_to_MemCopy(&ComBuf[n], ePORTval1, 4);
    n += 4;
    ComBuf[n++] = ',';
    ComBuf[n++] = '"';
    i = 0;
    if (nConnectError >= 3)
    {
        for (i = 0; i < 32; i++)
            eIPorURLval1[i] = BACKUP_URL[i];
        i = 0;
        nConnectError = 0;
    }
    while ((eIPorURLval1[i] != HASHTAG) && (i < 32))
    {
        ComBuf[i+n] = eIPorURLval1[i];
        i++;
    }
    n += i;
    ComBuf[n++] = '"';
    ComBuf[n++] = '\r';
    ComBuf[n++] = '\n';
    BytesToSend = n;
    TransmitBuf(0);
}

void SendPDPCntDef()
{
    BYTE i, n;
    //build the cops command  from country+network codes
    n = CopyFlashToBuf(ComBuf, DEF_PDP_CNTXT);
    ComBuf[n++] = '"';
    i = 0;
    while ((eAPN[i] != HASHTAG) && (i < 32))
    {
        ComBuf[i+n] = eAPN[i];
        i++;
    }
    n += i;
    ComBuf[n++] = '"';
    ComBuf[n++] = '\r';
    ComBuf[n++] = '\n';
    BytesToSend = n;
    TransmitBuf(0);
}

//void SetModemOn()
//{
//    #ifdef DebugMode
//    SendDebugMsg("\r\nSetModemOn");
//    #endif DebugMode
//    GSM_POWER_ON();
//    delay_ms(1000);
//    ModemResponse = TASK_COMPLETE;
//}

BYTE GetNextTask()
{
    if (nTimeCnt > 0)
        return WAIT;

    // first task-
    if (modemCurTask == TASK_NONE)
    {
        modemCurTask = TASK_MODEM_INIT;
        if (IsModemOn())
            modemCurSubTask = SUB_TASK_INIT_MODEM_OK;
        else
            modemCurSubTask = SUB_TASK_MODEM_IGN_ON;
        initCnt = 0;
        return  CONTINUE;
    }
    if ((bWaitForModemAnswer == TRUE) && (TimeLeftForWaiting == 0))
    {
        bWaitForModemAnswer = FALSE;
        ModemResponse = TASK_FAILED;
    }
    // if flag of end of rx received is on
    if (bCheckRxBuf == TRUE)
    {
        bCheckRxBuf = FALSE;
        ParseModemResponse();
        delay_ms(500);
    }

    if (nTimeCnt > 0)
        return WAIT;

    switch (ModemResponse)
    {
        case NO_ANSWER:
            return WAIT;
        case TASK_COMPLETE:
        {
            switch (modemCurTask)
            {
                case TASK_MODEM_INIT:
                    switch (modemCurSubTask)
                    {
                        case SUB_TASK_MODEM_IGN_ON:
                            taskAfterDelay = SUB_TASK_MODEM_IGN_OFF;
                            modemCurSubTask = SUB_TASK_DELAY;
                            nTimeCnt = SEC_4_GSM_IGNITION * 10;
                        break;
                        case SUB_TASK_MODEM_IGN_OFF:
                            if (IsModemOn())
                                modemCurSubTask = SUB_TASK_INIT_MODEM_OK;
                            else
                                modemCurSubTask = SUB_TASK_INIT_MODEM_HW_SHDN;
                        break;
                        case SUB_TASK_INIT_MODEM_HW_SHDN:
                            if (IsModemOn())
                                modemCurSubTask = SUB_TASK_INIT_MODEM_OK;
                            else
                                if (initCnt < 2)
                                    modemCurSubTask = SUB_TASK_MODEM_IGN_ON;
                                else
                                {
                                    modemCurTask = TASK_MODEM_CLOSE;
                                    modemCurSubTask = SUB_TASK_MODEM_EXIT;
                                }

                        break;
                        case SUB_TASK_INIT_MODEM_OK:
                            if (bExtReset == TRUE)
                            {
                                modemCurSubTask = SUB_TASK_INIT_MODEM_QSS;
                                TurnOnLed(LED_2, LED_BLINK);
                            }
                            else
                            {
                                if ((isRoaming == TRUE) && (numOprt == 0))  // if need to find operator but hasnt on reset - try again
                                    modemCurSubTask = SUB_TASK_INIT_MODEM_COPS_LST;
                                else
                                {
                                    taskAfterDelay = SUB_TASK_INIT_MODEM_REG;
                                    modemCurSubTask = SUB_TASK_DELAY;
                                    nTimeCnt = 100; //delay before creg
                                }
                            }
                        break;
                        case SUB_TASK_INIT_MODEM_QSS:
                            modemCurSubTask = SUB_TASK_MODEM_CGMM;//SUB_TASK_MODEM_CHK_ICCID;
                        break;
                        case  SUB_TASK_MODEM_CGMM:
                            modemCurSubTask = SUB_TASK_MODEM_CHK_ICCID;
                        break;
                        case  SUB_TASK_MODEM_CHK_ICCID: //SUB_TASK_INIT_MODEM_COPS:
                            modemCurSubTask = SUB_TASK_INIT_MODEM_COPS;
                        break;
                        case  SUB_TASK_INIT_MODEM_COPS:
                            if (eUseCntrCode == 1)
                                modemCurSubTask = SUB_TASK_INIT_MODEM_COPS_MAN;
                            else
                                if (isRoaming == TRUE)
                                    modemCurSubTask = SUB_TASK_INIT_MODEM_COPS_LST;
                                else
                                {
                                    //modemCurSubTask = SUB_TASK_INIT_MODEM_REG;
                                    taskAfterDelay = SUB_TASK_INIT_MODEM_REG;
                                    modemCurSubTask = SUB_TASK_DELAY;
                                    nTimeCnt = 100; //delay before creg
                                }
                        break;
                        case SUB_TASK_INIT_MODEM_COPS_LST:
                        if (numOprt > 0)              // if found at list 1 operator
                            modemCurSubTask = SUB_TASK_INIT_MODEM_COPS_MAN;
                        else
                            {
                                modemCurTask = TASK_MODEM_CLOSE;
                                modemCurSubTask = SUB_TASK_MODEM_CLOSE_MDM;
                            }
                        break;
                        case SUB_TASK_INIT_MODEM_COPS_MAN:
                            modemCurSubTask = SUB_TASK_DELAY;
                            nTimeCnt = 100;
                            taskAfterDelay = SUB_TASK_INIT_MODEM_REG;
                        break;
                        case SUB_TASK_INIT_MODEM_REG:
                            modemCurSubTask = SUB_TASK_INIT_MODEM_REG_STAT;
                        break;
                        case SUB_TASK_INIT_MODEM_REG_STAT:
                            modemCurSubTask = SUB_TASK_INIT_MODEM_GET_COPS;
                        break;
                        case SUB_TASK_INIT_MODEM_GET_COPS:
                            modemCurSubTask = SUB_TASK_INIT_MODEM_MONITOR;//SUB_TASK_INIT_MODEM_NITZ
                        break;
//                        case SUB_TASK_INIT_MODEM_NITZ:
//                            modemCurSubTask = SUB_TASK_INIT_MODEM_CCLK;
//                        break;
//                        case SUB_TASK_INIT_MODEM_CCLK:
//                            modemCurSubTask = SUB_TASK_INIT_MODEM_MONITOR;
//                        break;
                        case SUB_TASK_INIT_MODEM_MONITOR:
                            modemCurSubTask = SUB_TASK_INIT_MODEM_RSSI;
                        break;
                        case SUB_TASK_INIT_MODEM_RSSI: //todo: check rssi and continue only if it over min
                            {
                                modemCurTask = TASK_MODEM_CONNECT;
                                modemCurSubTask = SUB_TASK_MODEM_CONNECT_ATCH;//  SUB_TASK_MODEM_CONNECT_DELAY
                                //nTimeCnt = 150; //delay before attach
                            }
                        break;
                        case SUB_TASK_DELAY:
//                        #ifdef DebugMode
//                        SendDebugMsg("\r\nSUB_TASK_DELAY");
//                        putchar1(nTimeCnt);
//                        #endif DebugMode
//                            delay_ms(500);

                            if (nTimeCnt <= 0)
                                modemCurSubTask = taskAfterDelay;
                        break;
                        default:
                            modemCurTask = TASK_MODEM_CLOSE;
                            modemCurSubTask = SUB_TASK_MODEM_CLOSE_MDM;
                        break;
                    }
                break;
                case TASK_MODEM_CONNECT:
                    switch (modemCurSubTask)
                    {
//                        case SUB_TASK_MODEM_CONNECT_DELAY:
//                            modemCurSubTask = SUB_TASK_MODEM_CONNECT_ATCH;
//                            break;
                        case SUB_TASK_MODEM_CONNECT_ATCH:
                            if (bExtReset == FALSE)
                                modemCurSubTask = SUB_TASK_MODEM_CONNECT_ACTV;//SUB_TASK_MODEM_CONNECT_PDP_DEF;
                            else
                                modemCurSubTask = SUB_TASK_MODEM_CONNECT_PDP_DEF;   //SUB_TASK_MODEM_CONNECT_SETUP1;
                        break;
                        case SUB_TASK_MODEM_CONNECT_SETUP1:
                            modemCurSubTask = SUB_TASK_MODEM_CONNECT_SETUP2;
                        break;
                        case SUB_TASK_MODEM_CONNECT_SETUP2:
                            modemCurSubTask = SUB_TASK_MODEM_CONNECT_SETUP3;
                        break;
                        case SUB_TASK_MODEM_CONNECT_SETUP3:
                            modemCurSubTask = SUB_TASK_MODEM_CONNECT_ACTV;//SUB_TASK_MODEM_CHK_ICCID;
                        break;
                        case SUB_TASK_MODEM_CONNECT_PDP_DEF:
                            modemCurSubTask = SUB_TASK_MODEM_CONNECT_SETUP1;// SUB_TASK_MODEM_CONNECT_ACTV;   //   SUB_TASK_MODEM_CONNECT_IS_ACTV
                        break;
//                        case SUB_TASK_MODEM_CONNECT_IS_ACTV:
//                            if (GetContextStatus() == '1')
//                                modemCurSubTask = SUB_TASK_MODEM_CONNECT_START_DIAL;
//                            else
//                                modemCurSubTask = SUB_TASK_MODEM_CONNECT_ACTV;
//                        break;
                        case SUB_TASK_MODEM_CONNECT_ACTV:
//                            if ((toDoList == DO_PARAMS) || (toDoList == DO_DATA_N_PRMS))
//                                modemCurSubTask = SUB_TASK_INIT_MODEM_EMAP;
//                            else
                                modemCurSubTask = SUB_TASK_MODEM_CONNECT_START_DIAL;
                        break;
//                        case SUB_TASK_INIT_MODEM_EMAP:
//                            modemCurSubTask = SUB_TASK_INIT_MODEM_POS;
//                        break;
//                        case SUB_TASK_INIT_MODEM_POS:
//                            modemCurSubTask = SUB_TASK_MODEM_CONNECT_START_DIAL;
//                        break;
                        case SUB_TASK_MODEM_CONNECT_START_DIAL:
                            modemCurTask = TASK_MODEM_POST;
                            modemCurSubTask = waitingTask;
                        break;
                        default:
                            modemCurTask = TASK_MODEM_CLOSE;
                            modemCurSubTask = SUB_TASK_MODEM_CLOSE_MDM;
                        break;
                    }
                break;
                case TASK_MODEM_POST:
                    switch (modemCurSubTask)
                    {
                        case SUB_TASK_MODEM_POST_DATA:
                            objToMsr++;
                            // if finish last sensor
                            if (objToMsr > eNumSensors)
                            {
                                dataSentOK = TRUE;
//                                objToMsr = GetNextLocation2Send();
//                                if (objToMsr > 0)
//                                    modemCurSubTask = SUB_TASK_MODEM_POST_LOCATION;
//                                //bDialAgain4Data = FALSE;
//                                else
                                {
                                    modemCurTask = TASK_MODEM_CLOSE;
                                    modemCurSubTask = SUB_TASK_MODEM_CLOSE_EOD;
                                }
                            }
                        break;
//                        case SUB_TASK_MODEM_POST_LOCATION:
//                            objToMsr = GetNextLocation2Send();
//                            if (objToMsr == 0)
//                            {
//                                modemCurTask = TASK_MODEM_CLOSE;
//                                modemCurSubTask = SUB_TASK_MODEM_CLOSE_EOD;
//                            }
//                        break;
                        case SUB_TASK_MODEM_POST_PRM:
                        case SUB_TASK_MODEM_POST_CNFRM:
                            if ((modemCurSubTask == SUB_TASK_MODEM_POST_CNFRM) && (bUpdateAddress))  // if updating url & port
                            {
                                if (prmUpdtIndex == 1)      // if finish to confirm new url,
                                {
                                    prmUpdtIndex = 2;       // now confirm port
                                    break;
                                }
                                else
                                    if (prmUpdtIndex == 2)          // if finish to confirm new port
                                        bUpdateAddress = FALSE;     //reset flag
                            }
                            // if there is more update to do
                            if (IsPrmToUpdate() == TRUE)
                                modemCurSubTask = SUB_TASK_MODEM_POST_UPD;
                            else  // if no updates to do
                            {
                                prmSentOK = TRUE;
                                //bDialAgain4Prm = FALSE;
                                //waitingTask = TASK_NONE;
                                modemCurTask = TASK_MODEM_CLOSE;
                                modemCurSubTask = SUB_TASK_MODEM_CLOSE_EOD;
                            }
                        break;
                        case SUB_TASK_MODEM_POST_UPD:
                            if ((bUpdateAddress) && (prmUpdtIndex == 1))   // if updating url & port: and finish to get new url,
                                prmUpdtIndex = 2;                          // right after get the new url - send post to get port (before confirm)
                            else
                            {
                                modemCurSubTask = SUB_TASK_MODEM_POST_CNFRM;
                                if ((bUpdateAddress) && (prmUpdtIndex == 2))   // if updating url & port: and finish to get new port,
                                    prmUpdtIndex = 1;                          // now send the confirm to url (and later to port)
                            }
                        break;
                        default:
                            modemCurTask = TASK_MODEM_CLOSE;
                            modemCurSubTask = SUB_TASK_MODEM_CLOSE_EOD;
                        break;
                    }
                break;
                case TASK_MODEM_CLOSE:
                    switch (modemCurSubTask)
                    {
                        case  SUB_TASK_MODEM_CLOSE_EOD:
                            modemCurSubTask = SUB_TASK_MODEM_CLOSE_TCP;
                        break;
                        case SUB_TASK_MODEM_CLOSE_TCP:
                            if (waitingTask == SUB_TASK_MODEM_POST_DATA)
                            {
                                if ((dataSentOK == TRUE) || (nFailureCntr >= 2))
                                {
                                    if (toDoList == DO_DATA_N_PRMS)
                                    {
                                        waitingTask = SUB_TASK_MODEM_POST_PRM;
//                                        nFailureCntr = 0;
                                        modemCurTask = TASK_MODEM_CONNECT;
                                        modemCurSubTask = SUB_TASK_MODEM_CONNECT_START_DIAL;
                                    }
                                    else
                                        modemCurSubTask = SUB_TASK_MODEM_CLOSE_MDM;
                                }
                                else
                                {
                                    if ((dataSentOK == FALSE) && (nFailureCntr < 2))
                                    {
                                        modemCurTask = TASK_MODEM_CONNECT;
                                        modemCurSubTask = SUB_TASK_MODEM_CONNECT_START_DIAL;
                                    }
                                }
                            }
                            else
                                if (waitingTask == SUB_TASK_MODEM_POST_PRM)
                                {
                                    if ((prmSentOK == TRUE) || (nFailureCntr >= 2))
                                        modemCurSubTask = SUB_TASK_MODEM_CLOSE_MDM;
                                    else
                                    {
                                        modemCurTask = TASK_MODEM_CONNECT;
                                        modemCurSubTask = SUB_TASK_MODEM_CONNECT_START_DIAL;
                                    }
                                }
                        break;
                        case SUB_TASK_MODEM_CLOSE_MDM:
                            delay_ms(3000);
                            if (IsModemOn())
                            {
                                taskAfterDelay = SUB_TASK_MODEM_OFF;
                                modemCurSubTask = SUB_TASK_DELAY;
                                nTimeCnt = 150;
                            }
                            else
                                modemCurSubTask = SUB_TASK_MODEM_EXIT;
                        break;
                        case SUB_TASK_MODEM_OFF:
//                            if (IsModemOn())
//                                modemCurSubTask = SUB_TASK_MODEM_IGN_ON;
//                            else
                                modemCurSubTask = SUB_TASK_MODEM_EXIT;
                        break;
//                        case SUB_TASK_MODEM_IGN_ON:
//                            taskAfterDelay = SUB_TASK_MODEM_IGN_OFF;
//                            modemCurSubTask = SUB_TASK_DELAY;
//                            nTimeCnt = 40;
//                        break;
//                        case SUB_TASK_MODEM_IGN_OFF:
//                            modemCurSubTask = SUB_TASK_MODEM_OFF;
//                        break;
                        case SUB_TASK_DELAY:
                            delay_ms(500);
                            if (nTimeCnt <= 0)
                                modemCurSubTask = taskAfterDelay;
                        break;
                    }
                break;
            }
            failCnt = 0;
            return CONTINUE;
        }
        case TASK_FAILED:
        {
            failCnt++;    // count num of failures
            #ifdef DebugMode
            putchar1(failCnt + 0x30);
            #endif DebugMode
            //  if failure is cos service hasn't answered- no reason to try again.
            if ((modemCurTask == TASK_MODEM_POST) && (bPostAnswered == FALSE))
            {
                failCnt = nMaxFailuresNum;
//                nFailureCntr++;
            }

            // if failed more than 2 times - quit
            if (failCnt >= nMaxFailuresNum)
            {
                switch (modemCurTask)
                {
                    case TASK_MODEM_INIT:
                        switch (modemCurSubTask)
                        {
                            case SUB_TASK_INIT_MODEM_OK:
                                // if try only once to jig the iggnition pulse - try again, else- switch off.
                                if (initCnt < 2)
                                    modemCurSubTask = SUB_TASK_MODEM_IGN_ON;
                                else
                                {
//                                    if (!bExtReset)
//                                        TurnOnLed(LED_1, FAILURE);
                                    modemCurTask = TASK_MODEM_CLOSE;
                                    modemCurSubTask = SUB_TASK_MODEM_OFF;
                                }
                            break;
                            case SUB_TASK_INIT_MODEM_QSS:
                                modemCurTask = TASK_MODEM_CLOSE;
                                modemCurSubTask = SUB_TASK_MODEM_CLOSE_MDM;
                            break;
                            case SUB_TASK_MODEM_CGMM:
                                modemCurTask = TASK_MODEM_CLOSE;
                                modemCurSubTask = SUB_TASK_MODEM_CLOSE_MDM;
                            break;
                            case  SUB_TASK_INIT_MODEM_COPS:
                                modemCurSubTask = SUB_TASK_INIT_MODEM_REG;
                            break;
                            case SUB_TASK_INIT_MODEM_REG:
                            case SUB_TASK_INIT_MODEM_REG_STAT:
                                if ((curOprt < numOprt) && (isRoaming == TRUE))
                                    modemCurSubTask = SUB_TASK_INIT_MODEM_COPS_MAN;
                                else
                                {
                                    if (isRoaming == TRUE)      // if failed to connect with all operators reset flags for next connection
                                    {
                                        curOprt = 0;
                                        numOprt = 0;
                                    }
                                    //TurnOnLed(LED_2, FAILURE);
                                    modemCurTask = TASK_MODEM_CLOSE;
                                    modemCurSubTask = SUB_TASK_MODEM_CLOSE_MDM;
                                }
                                break;
                            break;
                            case SUB_TASK_INIT_MODEM_GET_COPS:
                                modemCurSubTask = SUB_TASK_INIT_MODEM_MONITOR;//SUB_TASK_INIT_MODEM_NITZ
                            break;
//                            case SUB_TASK_INIT_MODEM_NITZ:
//                                modemCurSubTask = SUB_TASK_INIT_MODEM_CCLK;
//                            break;
//                            case SUB_TASK_INIT_MODEM_CCLK:
//                                modemCurSubTask = SUB_TASK_INIT_MODEM_MONITOR;
//                            break;
                            case SUB_TASK_INIT_MODEM_COPS_LST:
                                modemCurTask = TASK_MODEM_CLOSE;
                                modemCurSubTask = SUB_TASK_MODEM_CLOSE_MDM;
                            break;
                            case SUB_TASK_INIT_MODEM_COPS_MAN:
                                if ((curOprt >= numOprt) || (isRoaming == FALSE))
                                {
                                    //TurnOnLed(LED_2, FAILURE);
                                    modemCurTask = TASK_MODEM_CLOSE;
                                    modemCurSubTask = SUB_TASK_MODEM_CLOSE_MDM;
                                }
                            break;
                            case SUB_TASK_INIT_MODEM_MONITOR:
                                modemCurSubTask = SUB_TASK_INIT_MODEM_RSSI;
                            break;
                            case SUB_TASK_INIT_MODEM_RSSI: //todo: check rssi and continue only if it over min
                                    modemCurTask = TASK_MODEM_CONNECT;
                                    modemCurSubTask = SUB_TASK_MODEM_CONNECT_ATCH;
                            break;
                            case SUB_TASK_MODEM_CHK_ICCID:
                                modemCurSubTask = SUB_TASK_INIT_MODEM_REG;
                            break;
                            default:
                                modemCurTask = TASK_MODEM_CLOSE;
                                modemCurSubTask = SUB_TASK_MODEM_CLOSE_MDM;
                            break;
                        }
                    break;
                    case TASK_MODEM_CONNECT:
                        switch (modemCurSubTask)
                        {
                            case SUB_TASK_MODEM_CONNECT_ATCH:
                                 if (bExtReset == FALSE)
                                    modemCurSubTask = SUB_TASK_MODEM_CONNECT_ACTV;  //
                                else
                                    modemCurSubTask = SUB_TASK_MODEM_CONNECT_PDP_DEF;  //SUB_TASK_MODEM_CONNECT_SETUP1;
                            break;
                            case SUB_TASK_MODEM_CONNECT_SETUP1:
                                modemCurSubTask = SUB_TASK_MODEM_CONNECT_SETUP2;
                            break;
                            case SUB_TASK_MODEM_CONNECT_SETUP2:
                                modemCurSubTask = SUB_TASK_MODEM_CONNECT_SETUP3;
                            break;
                            case SUB_TASK_MODEM_CONNECT_SETUP3:
                                modemCurSubTask = SUB_TASK_MODEM_CONNECT_ACTV;//SUB_TASK_MODEM_CHK_ICCID;
                            break;
                            case SUB_TASK_MODEM_CONNECT_PDP_DEF:
                                modemCurSubTask = SUB_TASK_MODEM_CONNECT_SETUP1;//SUB_TASK_MODEM_CONNECT_ACTV;
                            break;
//                            case SUB_TASK_MODEM_CONNECT_IS_ACTV:
//                                modemCurTask = TASK_MODEM_CLOSE;
//                                modemCurSubTask = SUB_TASK_MODEM_CLOSE_MDM;   //SUB_TASK_MODEM_CLOSE_TCP
//                            break;
                            case SUB_TASK_MODEM_CONNECT_ACTV:
                                if ((curOprt < numOprt) && (isRoaming == TRUE))
                                {
                                    modemCurTask = TASK_MODEM_INIT;
                                    modemCurSubTask = SUB_TASK_INIT_MODEM_COPS_MAN;
                                }
                                else
                                {
                                    modemCurTask = TASK_MODEM_CLOSE;
                                    modemCurSubTask = SUB_TASK_MODEM_CLOSE_MDM;   //SUB_TASK_MODEM_CLOSE_TCP
                                }
                            break;
//                            case SUB_TASK_INIT_MODEM_EMAP:
//                                modemCurSubTask = SUB_TASK_MODEM_CONNECT_START_DIAL;
//                            break;
//                            case SUB_TASK_INIT_MODEM_POS:
//                                modemCurSubTask = SUB_TASK_MODEM_CONNECT_START_DIAL;
//                            break;
                            case SUB_TASK_MODEM_CONNECT_START_DIAL:
                                nConnectError++;
                                modemCurTask = TASK_MODEM_CLOSE;
                                modemCurSubTask = SUB_TASK_MODEM_CLOSE_TCP;
//                                nFailureCntr++;
                            break;
                            default:
                                modemCurTask = TASK_MODEM_CLOSE;
                                modemCurSubTask = SUB_TASK_MODEM_CLOSE_MDM;
                            break;
                        }
                    break;
                    case TASK_MODEM_POST:
                        switch (modemCurSubTask)
                        {
                            case SUB_TASK_MODEM_POST_DATA:
                                ResetReadPointer();          // reset pointer of next read block to the first one
                                if (bPostAnswered == TRUE)
                                    objToMsr++;
                            // if finish last sensor
                                if ((objToMsr > eNumSensors) || (bPostAnswered == FALSE))
                                {
                                    modemCurTask = TASK_MODEM_CLOSE;
                                    modemCurSubTask = SUB_TASK_MODEM_CLOSE_EOD;
                                    if (bPostAnswered == TRUE)
                                        dataSentOK = TRUE;
                                }
                            break;
                            case SUB_TASK_MODEM_POST_PRM:
//                            case SUB_TASK_MODEM_POST_LOCATION:
                                modemCurTask = TASK_MODEM_CLOSE;
                                modemCurSubTask = SUB_TASK_MODEM_CLOSE_EOD;
                                if (bPostAnswered == TRUE)
                                    prmSentOK = TRUE;
                                break;
                            case SUB_TASK_MODEM_POST_UPD:
                            case SUB_TASK_MODEM_POST_CNFRM:
                                //if trying to update address but failed because url is incorrect - skip over update port
                                if ((bUpdateAddress) && (prmUpdtIndex == 1))
                                    UpdatePrmArr[2] = '0';
                                // if sensor gets answers just not expected once, and there is more update to do
                                if ((IsPrmToUpdate() == TRUE) && (bPostAnswered == TRUE))
                                    modemCurSubTask = SUB_TASK_MODEM_POST_UPD;
                                else
                                {
                                    modemCurTask = TASK_MODEM_CLOSE;
                                    modemCurSubTask = SUB_TASK_MODEM_CLOSE_EOD;
                                    // if post answered but wrong answer - mark mission as done
                                    if (bPostAnswered == TRUE)
                                        prmSentOK = TRUE;
                                }
                                break;
                            default:
                                modemCurTask = TASK_MODEM_CLOSE;
                                modemCurSubTask = SUB_TASK_MODEM_CLOSE_EOD;
                            break;
                        }
                    break;
                    case TASK_MODEM_CLOSE:
                        switch (modemCurSubTask)
                        {
                            case  SUB_TASK_MODEM_CLOSE_EOD:
                                modemCurSubTask = SUB_TASK_MODEM_CLOSE_TCP;
                            break;
                            case SUB_TASK_MODEM_CLOSE_TCP:
                                modemCurSubTask = SUB_TASK_MODEM_CLOSE_MDM;
                            break;
                            case SUB_TASK_MODEM_CLOSE_MDM:
                                modemCurSubTask = SUB_TASK_MODEM_OFF;
                            break;
                            default:
                                modemCurSubTask = SUB_TASK_MODEM_CLOSE_MDM;
                            break;
                        }
                    break;
                }
                failCnt = 0;
                if (modemCurTask == TASK_MODEM_CLOSE)
                {
                    if (toDoList == DO_PARAMS)
                    {
                        if (prmSentOK == FALSE)
                        {
                            nFailureCntr++;
                            #ifdef DebugMode
                            SendDebugMsg("\r\nnFailureCntr++");
                            #endif DebugMode
                        }
                    }
                    else
                        if (dataSentOK == FALSE)
                        {
                            nFailureCntr++;
                            #ifdef DebugMode
                            SendDebugMsg("\r\nnFailureCntr++");
                            #endif DebugMode
                        }
                }
            }
            else
            {
                if (modemCurSubTask == SUB_TASK_INIT_MODEM_COPS_LST)
                {
                    if (fMdmAns == TRUE) //if modem answer 'ERROR' - delay 10 sec, else no delay
                        delay_ms(10000);
                    else
                        return CONTINUE;
                }
                delay_ms(1000);
                // if fail on send data - change the read mod to 3 (send again the buffer)
                if ((modemCurSubTask == SUB_TASK_MODEM_POST_DATA) && (failCnt == 1))
                    ResetReadPointer();          // reset pointer of next read block to the first one
                if ((modemCurSubTask == SUB_TASK_INIT_MODEM_REG) || (modemCurSubTask == SUB_TASK_INIT_MODEM_REG_STAT))
                    delay_ms(4000);
                //if ((modemCurSubTask == SUB_TASK_MODEM_CONNECT_IS_ACTV) ||
                if (modemCurSubTask == SUB_TASK_MODEM_CONNECT_START_DIAL)        //                   (modemCurSubTask == SUB_TASK_MODEM_CONNECT_ACTV) ||
                {
                    delay_ms(3000);
//                    nFailureCntr++;
                    nConnectError++;
                }
                if (modemCurSubTask == SUB_TASK_MODEM_CONNECT_ACTV)
                    delay_ms(1000);
            }
        }
        return CONTINUE;
        break;
    }
}

void ModemMain()
{
    BYTE res, i;
    res = GetNextTask();
    if (res == WAIT)
        return;

    switch (modemCurTask)
    {
        case TASK_MODEM_INIT:
            switch (modemCurSubTask)
            {
                case SUB_TASK_MODEM_IGN_ON:
                    TurnOnIgnition();
                break;
                case SUB_TASK_MODEM_IGN_OFF:
                    TurnOffIgnition();
                break;
                case SUB_TASK_INIT_MODEM_HW_SHDN:
                    ModemHwShdn();
                break;
                case SUB_TASK_INIT_MODEM_OK:
                    cEndMark = '\0';
                    nMaxWaitingTime = 25;   // wait max 2.5 sec for answer
                    nMaxFailuresNum = 2;
                    bNeedToWait4Answer = TRUE;
                    //ask modem if OK
                    SendATCmd(AT_IsModemOK);
                break;
                case SUB_TASK_INIT_MODEM_COPS:
                    //treat leds:
//                    if (!bExtReset)
//                        TurnOnLed(LED_1, SUCCESS);
                    //select the  operator
//                    if (isRoaming)
//                        SendATCmd(AT_COPS_AUTO_2);
//                    else
                        SendATCmd(AT_COPS_AUTO_0);
                break;
                case SUB_TASK_MODEM_CHK_ICCID:
                    SendATCmd(AT_CCID);
                break;
                case SUB_TASK_INIT_MODEM_COPS_LST:
                    nMaxFailuresNum = 10;
                    nMaxWaitingTime = 900;//450;//350;  //wait 90 sec for timeout
                    fMdmAns = FALSE;
                    curOprt = 0;
                    numOprt = 0;
                    SendATCmd(AT_COPS_LST);
                break;
                case SUB_TASK_INIT_MODEM_QSS:
                    nMaxWaitingTime = 20;   // wait max 2.5 sec for answer
                    nMaxFailuresNum = 1;
                    SendATCmd(AT_QSS);
                break;
                case SUB_TASK_MODEM_CGMM:
                    nMaxWaitingTime = 20;   // wait max 2 sec for answer
                    nMaxFailuresNum = 1;
                    SendATCmd(AT_CGMM);
                break;
                case SUB_TASK_INIT_MODEM_COPS_MAN:
                    nMaxWaitingTime = 25;   // wait max 2.5 sec for answer
                    nMaxFailuresNum = 2;
                    SendOperatorSelection();
                break;
                case SUB_TASK_INIT_MODEM_REG:
                    nMaxFailuresNum = 30;
//                    #ifdef DebugMode
//                    nMaxFailuresNum = 10;
//                    #endif DebugMode
                    //ask modem if sync to network
                    SendATCmd(AT_IsModemReg);
                break;
                case SUB_TASK_INIT_MODEM_REG_STAT:
                    nMaxFailuresNum = 15;
                    SendATCmd(AT_REG_STATUS);
                break;
                case SUB_TASK_INIT_MODEM_GET_COPS:
                    TurnOnLed(LED_2, LED_ON);
                    TurnOnLed(LED_3, LED_BLINK);
                    nMaxFailuresNum = 1;
                    SendATCmd(AT_COPS_ASK);
                break;
//                case SUB_TASK_INIT_MODEM_NITZ:
//                    SendATCmd(NETWORTK_TIMEZONE);
//                break;
//                case SUB_TASK_INIT_MODEM_CCLK:
//                    SendATCmd(GET_CLOCK);
//                break;
                case SUB_TASK_INIT_MODEM_MONITOR:
                    longAnswerExpected = 1;
                    SendATCmd(AT_CELL_MONITOR);
                break;
                case SUB_TASK_INIT_MODEM_RSSI:
                    longAnswerExpected = 0;
//                    nMaxFailuresNum = 1;
                    //ask modem RSSI with network host
                    SendATCmd(AT_CSQ);
                break;
            }
        break;
        case TASK_MODEM_CONNECT:
            switch (modemCurSubTask)
            {
                case SUB_TASK_MODEM_CONNECT_ATCH:
                    nMaxFailuresNum = 2;
                    SendATCmd(GPRS_ATTACH);
                break;
                case SUB_TASK_MODEM_CONNECT_SETUP1:
                    SendATCmd(DEF_QULT_MIN_PROF);
                break;
                case SUB_TASK_MODEM_CONNECT_SETUP2:
                    SendATCmd(DEF_QULT_REQ_PROF);
                break;
                case SUB_TASK_MODEM_CONNECT_SETUP3:
                    SendATCmd(DEF_SCKT_CNFG);
                break;
                case SUB_TASK_MODEM_CONNECT_PDP_DEF:
                    SendPDPCntDef();
                break;
                case SUB_TASK_MODEM_CONNECT_ACTV:
                    nMaxWaitingTime = 100;  //wait max 10 sec
                    nMaxFailuresNum = 10;
                    SendATCmd(ACTIVATE_CNTXT);
                break;
//                case SUB_TASK_INIT_MODEM_EMAP:
//                    nMaxWaitingTime = 20;  //wait max 2 sec
//                    nMaxFailuresNum = 2;
//                    SendATCmd(ENABLE_MAPPING);
//                break;
//                case SUB_TASK_INIT_MODEM_POS:
//                    nMaxWaitingTime = 1800;  //wait max 3 min
//                    SendATCmd(GET_POS);
//                break;
                case SUB_TASK_MODEM_CONNECT_START_DIAL:
                    nMaxWaitingTime = 200;  //wait max 20 sec
                    nMaxFailuresNum = 2;
                    SendStartDial();
                break;
            }
        break;
        case TASK_MODEM_POST:
            cEndMark = HASHTAG;
            nMaxWaitingTime = 300;  // wait max 30 sec fot response
            bPostAnswered = FALSE;
            switch (modemCurSubTask)
            {
                case SUB_TASK_MODEM_POST_PRM:
//                    cEndMark = '#';
//                    nMaxWaitingTime = 300;  // wait max 30 sec fot response
                    nMaxFailuresNum = 1;
                    SendPostParam();
                    break;
                case SUB_TASK_MODEM_POST_UPD:
                    nMaxFailuresNum = 3;         //max 3 times try to get update & confirm
                    SendPostUpdate(GET_UPDATE);
                    UpdatePrmArr[prmUpdtIndex] = '0';
                    break;
                case SUB_TASK_MODEM_POST_CNFRM:
                    //UpdateParam();
                    SendPostUpdate(CONFIRM_UPDATE);
                    break;
                case SUB_TASK_MODEM_POST_DATA:
                    nMaxFailuresNum = 2;
                    SendPostData();
                    break;
//                case SUB_TASK_MODEM_POST_LOCATION:
//                    SendPostLocation();
//                    break;
            }
        break;
        case TASK_MODEM_CLOSE:
            nMaxFailuresNum = 2;
            nMaxWaitingTime = 40;  // wait max 4 sec for response
            cEndMark = '\0';

            switch (modemCurSubTask)
            {
                case  SUB_TASK_MODEM_CLOSE_EOD:
                    SendATCmd(AT_EOD);
                break;
                case SUB_TASK_MODEM_CLOSE_TCP:
                    if (dataSentOK == TRUE)// && (AlertConnecting))
                    {
                        // init  vars of alerts
                        for (i = SENSOR1; i <= eNumSensors; i++)
                            if (Alerts[i].Status == TRHRESHOLD_CROSSED)
                            {
                                Alerts[i].Status = ALERT_SHOT;
                                Alerts[i].OutOfLmtCnt = 0;
                                #ifdef DebugMode
                                SendDebugMsg("\r\nstatus changed to alert shot");
                                #endif DebugMode
                            }
                            else
                                if (Alerts[i].Status == ALERT_BACK_NORMAL)
                                {
                                    Alerts[i].Status = ALERT_WAIT;
                                    Alerts[i].OutOfLmtCnt = 0;
                                    Alerts[i].MsrCnt = 0;
                                    #ifdef DebugMode
                                    SendDebugMsg("\r\nstatus changed to alert wait");
                                    #endif DebugMode
                                }
                    }
                    SendATCmd(AT_TCP_CLS);
                    if ((toDoList == DO_DATA_N_PRMS) && (dataSentOK == TRUE) && (prmSentOK == TRUE))
                        bConnectOK = TRUE;
                    if ((toDoList == DO_DATA) && (dataSentOK == TRUE))
                        bConnectOK = TRUE;
                    if ((toDoList == DO_PARAMS) && (prmSentOK == TRUE))
                        bConnectOK = TRUE;
                break;
                case SUB_TASK_MODEM_CLOSE_MDM:
                    SendATCmd(MODEM_CLS);
                    // delay 15 sec after shdn modem
                break;
                case SUB_TASK_MODEM_OFF:
                    ModemHwShdn();
//                    SetModemOff();
                break;
//                case SUB_TASK_MODEM_IGN_ON:
//                    TurnOnIgnition();
//                break;
//                case SUB_TASK_MODEM_IGN_OFF:
//                    TurnOffIgnition();
//                break;
                case SUB_TASK_MODEM_EXIT:
                    bEndOfModemTask = TRUE;
                    //if sensor got task of reset sensor - do it now
                    if (bMakeReset == TRUE)
                    {
                        bReset = TRUE;
                        bMakeReset = FALSE;
                    }
                    if (bConnectOK == TRUE)                // sign that all tasks of connection complete ok
                    {
                        #ifdef DebugMode
                        SendDebugMsg("\r\nconnect finished OK");
                        #endif DebugMode
                        TurnOnLed(LED_3, LED_ON);
                    }

                break;
            }
        break;
        default:
    }
}

BYTE SendRecATCmd(flash unsigned char *bufToSend, BYTE tOut)
{
    BYTE bRes = FALSE;
    SendATCmd(bufToSend);
    nTimeCnt = tOut;
    while ((nTimeCnt > 0) && (bCheckRxBuf != TRUE));
    if (bCheckRxBuf == TRUE)
        bRes =  IsOK();
    #ifdef DebugMode
    if (!bRes)
        SendDebugMsg("\r\nfailed to send at cmd ");
    #endif DebugMode
    return bRes;
}

BYTE SetModemBaudRate()
{
    BYTE n = 0, brOK = FALSE;
    unsigned char MyBaudRateH, MyBaudRateL;
    ENABLE_UART0();
    #ifdef DebugMode
    ENABLE_UART1();
    #endif DebugMode

    MyBaudRateH = UBRR0H;
    MyBaudRateL = UBRR0L;

    bNeedToWait4Answer = TRUE;
    if (!IsModemOn())
    {
        MODEM_3G_IGNITION_ON();
        delay_ms(SEC_4_GSM_IGNITION * 1000);
        MODEM_3G_IGNITION_OFF();
    }
    if (!IsModemOn())
        return FALSE;

    // find current baud rate
    do
    {
        #ifdef DebugMode
        SendDebugMsg("\r\ntry to connect modem on baud rate ");
        PrintNum(BAUD_RATE_LONG[n]);
        putchar1(n);
        #endif DebugMode
        UBRR0H = BAUD_RATE_HIGH[n];
        UBRR0L = BAUD_RATE_LOW[n];
        delay_ms(250);
        brOK = SendRecATCmd(AT_IsModemOK, 20);
        if (!brOK)
            n++;
    }
    while ((brOK == FALSE) && (n < MAX_BAUD_RATE_OPTIONS));

    if (brOK == FALSE)
    {
        #ifdef DebugMode
        SendDebugMsg("\r\nfailed to find baudrate  ");
        PrintNum(BAUD_RATE_LONG[n]);
        #endif DebugMode

        return FALSE;
    }
    // change baud rate
    if (! SendRecATCmd(AT_DELL_ECHO, 20))
        return FALSE;
    if (! SendRecATCmd(AT_GPIO, 20))
        return FALSE;
    if (! SendRecATCmd(AT_SLED, 20))
        return FALSE;
    if (! SendRecATCmd(AT_SLEDSAV, 20))
        return FALSE;
    if (! SendRecATCmd(AT_K, 50))
        return FALSE;
    if (! SendRecATCmd(AT_IPR, 50))
        return FALSE;

    delay_ms(1000);
    // back to original baudrate
    UBRR0H = MyBaudRateH;
    UBRR0L = MyBaudRateL;
    delay_ms(250);
    SendRecATCmd(MODEM_CLS, 20);

    MODEM_3G_SHUTDOWN_START();
    delay_ms(200);
    MODEM_3G_SHUTDOWN_STOP();
    #ifdef DebugMode
    SendDebugMsg("\r\nModemHwShdn");
    #endif DebugMode
    return TRUE;
}

#endif MODEM_MANAGER_C

