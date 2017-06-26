#ifndef VARS_H
#define VARS_H

eeprom char eLoggerID[4] = {0, 0, 0 ,0}; //{0, 0, 0 ,0};

eeprom unsigned char eStartConnectionH = 4;        //first connection hour
eeprom unsigned char eConnectionInDay  = 3;        //number on connectionsin a day
eeprom unsigned char eConnectIntervalH = 8;        //intervalbetween connections (hours)
//eeprom char WakeupInterval = 1;
eeprom unsigned char eNumSensors = 0;      //address 0x7 in eeprom
//eeprom int gMin4UTC = 0;
//eeprom int gHr4UTC = 0;
//extern int iLastMsr[MAX_WL_SEN_NUM];
int iVoltage;
int nTimeCnt = -1;
int measure_time;
int_bytes union_b2i;
unsigned int time_in_minutes;     //time from day start in ninutes
//static unsigned int pSens_ext_e2;	//pointer to current sensor control parameters in ext_e2
unsigned char gUnreadValues;
char e2_writeFlag;
BYTE rssi_val;
volatile unsigned char eepromReadBuf[SENSOR_CNTRL_PRM_SIZE];	//buffer for eeprom read operation
char readClockBuf[7];	         //buffer for data reading from clock
BYTE modemCurTask;
BYTE modemCurSubTask;
BYTE waitingTask;
BYTE msrCurTask;
BYTE monitorCurTask;
volatile BYTE mainTask;
BYTE dataSentOK;
BYTE prmSentOK;
BYTE toDoList;
BYTE timeout4Cnct = 0;
BYTE flagSucOrFail;
BYTE stlRegStatus;
BYTE strRegFailCnt;
int NextByteIndex;
int BytesToSend;
unsigned int rx0_buff_len;
BYTE bStlMdmStatus;
char ComBuf[MAX_SBD_BUF_LEN ]; // buffer for transmit (TX)
char TxUart0Buf[20]; // buffer for transmit (TX)
volatile char RxUart0Buf[MAX_RX_BUF_LEN];
char RxUart1Buf[MAX_RX1_BUF_LEN];
BYTE ModemResponse;
BYTE nFailureCntr;
bit longAnswerExpected = 0;
int nMaxWaitingTime;
int TimeLeftForWaiting;
int gIndex;
unsigned int nextCompare;
bit overFlow = 0;
bit bCheckRxBuf;
bit bExtReset;
bit bReset = 0;
bit bWaitForModemAnswer;
bit bWaitForMonitorCmd;
bit bEndOfModemTask;
bit bEndOfMeasureTask;
bit bEndOfMonitorTask;
bit bWait4WLSensor;
bit bWaitForGPSData;
volatile BYTE bEndOfGPSTask;
#ifdef BlueToothOption
char bEndOfBTTask;
int nMaxWaitBtResponse = 0;
#endif BlueToothOption
bit bNeedToWait4Answer;
bit bConnectNow;
BYTE bConnectOK;
BYTE objToMsr;
//BYTE triggerCnt = 0;
BYTE LedStatus[4];
//BYTE BlinkNum[4];
BYTE UpdatePrmArr[MAX_PRM_TASKS];
char DataBlock[PCKT_LNGTH];
BYTE bMonitorConnected;
BYTE iFirstConnToday;
int nUnreadBlocks;
BYTE bModemType;
BYTE nSbdCntr;
//unsigned int cpuWakeTimeCnt;
BYTE arrSendAlldata[MAX_SEN_NUM];
struct AlertData Alerts[MAX_SEN_NUM];
struct OperatorData OprtTbl[10] ;
char tagSwitched;
char modemOnStartUp;
unsigned int arrReadPointer[MAX_SEN_NUM];
float g_fLat;
float g_fLon;
const int MinInt = 0xff7f;
volatile int iLastMsr[MAX_WL_SEN_NUM];
BYTE flgUart1Error = 0;
BYTE btrStatus;
unsigned char powerOnReset;
#endif VARS_H