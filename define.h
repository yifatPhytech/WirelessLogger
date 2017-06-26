#ifndef DEFINE_H
#define DEFINE_H

#include <mega644p.h>
#include <delay.h>

//optional:
//#define DebugOnUart0
//#define BlueToothOption
#define DebugMode
//#define TestMode
//#define WLDebug
//#define TestMonitor

typedef unsigned char BYTE;

typedef union {
    int ival;
    unsigned char bval[2];
} int_bytes;

typedef union {
    long lVal;
    char bVal[4];
} long_bytes;

typedef struct
{
    unsigned char second; // 0-59
    unsigned char minute; // 0-59
    unsigned char hour;   // 0-23
    unsigned char day;    // 1-31
    unsigned char month;  // 1-12
    unsigned char year;   // 0-99 (representing 2000-2099)
}date_time_t;

typedef union {
    float fVal;
    char bVal[4];
} float_bytes;

struct AlertData
{
    BYTE Status;
    BYTE MsrCnt;
    BYTE OutOfLmtCnt;
};

struct OperatorData
{
    BYTE Status;
    BYTE AccTech;
    long MccMnc;
};

struct WirelessSensor
{
    unsigned long    Id;
    BYTE    Type;
    unsigned int    Volt;
    BYTE    Rssi;
    BYTE    Index;
//    char    LocationStatus;
};

/*struct GPS_Points
{
    BYTE    nSensorIndex;
    float   Lat;
    float   Lon;
};
  */

#define DATA_REGISTER_EMPTY (1<<UDRE0)
#define RX_COMPLETE (1<<RXC0)
#define FRAMING_ERROR (1<<FE0)
#define PARITY_ERROR (1<<UPE0)
#define DATA_OVERRUN (1<<DOR0)

#define MAX_INT         32767
#define MIN_INT         0x8000
//const int MIN_INT = 0xff7f;
 //-32768

#define BTR_CNCT_LIMIT      3300
#define BTR_WIRELESS_LIMIT  3000
#define BTR_CHARGE_STRIP    200

#define BTR_FULL    0
#define BTR_HALF    1
#define BTR_EMPTY    2

#define QUARTER     15
//#define MIN_BTR_4_ALERT 7000

#define HASHTAG     '#'

//map memory: 1 KB memory per sensor
#define SENSOR_MEMORY_START     0x00   //0
#define SENSOR_MEMORY_SIZE      0x286   //MAX_ADDRESS / MAX_WL_SEN_NUM
#define SENSOR_CNTRL_PRM_SIZE   30  //0x14   //20   at the end of the 1024
#define CONTROL_PARAM_LENGTH    18
#define MAX_DATA_PER_PCKT  24
#define PCKT_LNGTH  (8 + 2 * MAX_DATA_PER_PCKT) //56
#define MAX_PCKTS_PER_SENSOR    ((SENSOR_MEMORY_SIZE - SENSOR_CNTRL_PRM_SIZE) / PCKT_LNGTH)//11
#define MAX_ADDRESS  0x9999

#define MAX_PRM_TASKS   19
#define INT_VREF 		2560	//reference voltage [mV] internal
#define ADC_VREF_TYPE ((1<<REFS1) | (1<<REFS0) | (0<<ADLAR))  //reference voltage [mV] internal

#define INTERVAL_PARAM	10	//interval parameter: 10 = 10 min

#define MAX_BAUD_RATE_OPTIONS  7

#define MAX_SBD_BUF_LEN      340       //
#define MAX_TX_BUF_LEN      40       //
#define MAX_RX_BUF_LEN      300 //was 64. change to 100 3/2014 for alerts
#define MAX_RX1_BUF_LEN      200 //was 64. change to 100 3/2014 for alerts
#define MAX_WAIT_MNTR_SEC   2 //10
#define MAX_REG_FAILURE    2
#define MAX_SBD_FAILURE    2
#define MAX_UART0_TX_BUF_LEN    20

//#ifndef DebugMode
//#define MAX_SEC_4_GPS   90
//#else
#define MAX_SEC_4_GPS   120
//#endif  DebugMode
#define SEC_4_GSM_IGNITION  10

#define MINMEA_MAX_LENGTH   200//MAX_RX1_BUF_LEN
#ifdef BlueToothOption
#define MAX_WAIT_BLUETOOTH  120
#endif BlueToothOption

#define SUCCESS 1
#define FAILURE 0

#define CONTINUE    1
#define WAIT        2

#define MODEM_GSM       1
#define MODEM_SATELLITE 2

#define SATELLITE_MODEM_UNCHECKED       0
#define SATELLITE_MODEM_NOT_CONNECTED   1
#define SATELLITE_MODEM_CONNECTED        2

#define ALERT_WAIT          1
#define TRHRESHOLD_CROSSED  2
#define ALERT_SHOT          3
#define ALERT_BACK_NORMAL   4

#define MAX_ICCID_LEN   20

#define TRUE 1
#define FALSE 0

#define NO_DATA 2

#define LED_1        1
#define LED_2        2
#define LED_3        3

#define LED_OFF     0
#define LED_ON      1
#define LED_BLINK   2

#define UART_RADIO_UHF  0
#define UART_GPS        1
#define UART_DBG   2
#define UART_NONE       3

#define COMMAND_INIT_MEMORY     1
#define COMMAND_MAKE_RESET      2
#define COMMAND_INIT_RESET      3
#define COMMAND_RTC_24          4
#define COMMAND_INIT_MEM_SENSORS    5

#define MSR_INIT    0
#define MSR_NEEDED  1
#define MSR_DONE    2

//#define NO_LOCATION 0
//#define GOT_LOCATION    1
//#define SEND_LOCATION   2

#define NO_ANSWER       0
#define TASK_COMPLETE   1
#define TASK_FAILED     2
#define TASK_HOLD       3

#define TASK_NONE       0
#define TASK_MEASURE    1
#define TASK_MODEM      2
#define TASK_SLEEP      4
#define TASK_WAKEUP     5
#define TASK_MONITOR    6
#define TASK_GPS        7
#ifdef BlueToothOption
#define TASK_BLUETOOTH   8
#endif BlueToothOption


#define TASK_MODEM_INIT       1
#define TASK_MODEM_SBD      2
#define TASK_MODEM_CONNECT    3
#define TASK_MODEM_POST       4
#define TASK_MODEM_CLOSE      5
#define TASK_MODEM_NET_REG  4
#define TASK_MODEM_INIT_CSQ     75
#define SUB_TASK_INIT_5V_ON      10
#define SUB_TASK_INIT_MODEM_ON   11
#define SUB_TASK_INIT_MODEM_OK   12
#define SUB_TASK_MODEM_INIT_E     79
#define SUB_TASK_MODEM_INIT_K     71
#define SUB_TASK_MODEM_INIT_D     72
#define SUB_TASK_MODEM_INIT_W     73
#define SUB_TASK_MODEM_INIT_Y     74
#ifdef DebugMode
#define SUB_TASK_MODEM_INIT_CGSN    76
#define SUB_TASK_MODEM_INIT_CGMR    77
#endif DebugMode
#define SUB_TASK_MODEM_INIT_NET_SYNC    78
#define SUB_TASK_REG_CSQ  75
#define SUB_TASK_REG      67
#define SUB_TASK_REG_QES  64

#define SUB_TASK_SBD_MTA      80
#define SUB_TASK_SBD_SND      60
#define SUB_TASK_SBD_SND_DATA 61
#define SUB_TASK_SBD_INIT     62
#define SUB_TASK_SBD_CLR_BUF  63
#define SUB_TASK_SBD_BUILD    66
#define SUB_TASK_SBD_READ     69
#define TASK_MODEM_CLK      68

#define SUB_TASK_CLOSE_MODEM_OFF    54
#define SUB_TASK_CLOSE_PWR_DOWN     65
#define SUB_TASK_CLOSE_5V_OFF   55
#define SUB_TASK_MODEM_SND_PRM   41
#define SUB_TASK_MODEM_SND_DATA  42
#define SUB_TASK_INIT_MODEM_REG  13
#define SUB_TASK_INIT_MODEM_RSSI 14
#define SUB_TASK_INIT_MODEM_COPS 15
#define SUB_TASK_MODEM_CHK_ICCID 16
//#define SUB_TASK_INIT_MODEM_IGN  17
#define SUB_TASK_INIT_MODEM_GET_COPS    18
#define SUB_TASK_INIT_MODEM_DELAY   19
#define SUB_TASK_INIT_MODEM_MONITOR    20
//#define SUB_TASK_INIT_MODEM_EMAP 21
//#define SUB_TASK_INIT_MODEM_POS 22
//#define SUB_TASK_INIT_MODEM_NITZ    81
//#define SUB_TASK_INIT_MODEM_CCLK    82
//
#define SUB_TASK_INIT_MODEM_COPS_LST    23
#define SUB_TASK_INIT_MODEM_COPS_MAN    24
#define SUB_TASK_INIT_MODEM_REG_STAT    25
#define SUB_TASK_INIT_MODEM_HW_SHDN 26
#define SUB_TASK_DELAY  27
#define SUB_TASK_INIT_MODEM_QSS 28

#define SUB_TASK_MODEM_CONNECT_ATCH     31
#define SUB_TASK_MODEM_CONNECT_SETUP1   32
#define SUB_TASK_MODEM_CONNECT_SETUP2   33
#define SUB_TASK_MODEM_CONNECT_SETUP3   34
#define SUB_TASK_MODEM_CONNECT_PDP_DEF  35
#define SUB_TASK_MODEM_CONNECT_ACTV     36
#define SUB_TASK_MODEM_CONNECT_START_DIAL    37
#define SUB_TASK_MODEM_CONNECT_DELAY    39

#define SUB_TASK_MODEM_POST_PRM   41
#define SUB_TASK_MODEM_POST_DATA  42
#define SUB_TASK_MODEM_POST_UPD   43
#define SUB_TASK_MODEM_POST_CNFRM   44
#define SUB_TASK_MODEM_POST_LOCATION  45

#define SUB_TASK_MODEM_CLOSE_EOD    51
#define SUB_TASK_MODEM_CLOSE_TCP    52
#define SUB_TASK_MODEM_CLOSE_MDM    53
#define SUB_TASK_MODEM_OFF          54
#define SUB_TASK_MODEM_IGN_ON       55
#define SUB_TASK_MODEM_IGN_OFF      56
#define SUB_TASK_MODEM_EXIT         57
#define SUB_TASK_MODEM_CGMM         58

#define DO_DATA 1
#define DO_PARAMS   2
#define DO_DATA_N_PRMS  3

#define INITIAL_STATE   0
#define REG_OK   1
#define REG_FAILED 2
#define MAX_REG_FAILURES 5

#define RATE9600    1
#define RATE38400   2

#define TASK_MSR_READ       1
#define TASK_MSR_INIT       2
#define TASK_MSR_SAVE       3
#define TASK_MSR_CLOSURE    4

#define TASK_GPS_START_READ       1
#define TASK_GPS_INIT       2
#define TASK_GPS_SAVE       3
#define TASK_GPS_CLOSURE    4
#define TASK_GPS_PARSE      5

#define TASK_MONITOR_CONNECT    1
#define TASK_MONITOR_WAIT   2

#define UPDATE_URL  1
#define UPDATE_PORT  2
#define UPDATE_APN  3
#define UPDATE_DL_START  7
#define UPDATE_DL_CYCLE  8
#define UPDATE_DL_INTERVAL  9
#define UPDATE_COMMAND  10
#define UPDATE_GMT  11
#define UPDATE_LIMITS  12
#define UPDATE_ARMED_LIMITS  13
#define UPDATE_USE_CC  14
#define UPDATE_ROAMING_DLY  15
#define UPDATE_MNC  16
#define UPDATE_MCC  17
#define UPDATE_ID  18
#define UPDATE_EPOCH  19

#define GET_UPDATE      1
#define CONFIRM_UPDATE  0

#define SENSOR1        1 //0//
#define MAX_WL_SEN_NUM     101//48
#define MAX_SEN_NUM     MAX_WL_SEN_NUM

#define MUX_CHARGE  0
#define MUX_BATTERY 1
#define H_PROG      988
#define R_PROG      3010

#define SENSOR_DATA_ID      10
#define SENSOR_LOCATION_ID  11
#define SENSOR_GPS_ID       12

#define SIZE_BYTE_INDEX 1//4
#define TYPE_BYTE_INDEX 5
#define FIRST_TAG_INDEX 7
#define FIRST_PAYLOAD_INDEX 5
#define FIRST_DATA_INDEX 3

#define ID      1   //2    //2+3+4
#define TYPE    10  //5   //was 8
#define INDEX   6
#define DATA    5   //9   //10+9
//#define DATA2    7   //7+8
#define CSTMR   7    //7+8
#define VOLTAGE 7   //11
#define RSSI    9   //12
#define MSG_INDEX    11
#define LATITUDE    5
#define LONGITUDE   9

#define DATA_MSG_LEN    12
#define LOCATION_MSG_LEN    13

#define BTR_INDEX     0

#define TYPE_NONE	0
#define TYPE_PIVOT	67
#define TYPE_PLANT	68
#define TYPE_WPS	69
#define TYPE_RAIN	70
#define TYPE_TENS	71
#define TYPE_SMS	72
#define TYPE_SD		73
#define TYPE_DER	74

#define TYPE_BTR    10

#define MODEM_3G_IGNITION_ON() (PORTC.5 = 1);    //@@@
#define MODEM_3G_IGNITION_OFF() (PORTC.5 = 0);

#define MODEM_3G_SHUTDOWN_START() (PORTC.6 = 1);      //@@@
#define MODEM_3G_SHUTDOWN_STOP() (PORTC.6 = 0);

#define GSM_POWER_ON()    (PORTC.7 = 0);       //@@@
#define GSM_POWER_OFF()   (PORTC.7 = 1);

//#define UART_1_MONITOR() (PORTD.5 = 1, PORTD.4 = 0);    //@@@
//#define UART_1_WIRELESS() (PORTD.5 = 0, PORTD.4 = 0);

#define WIRELESS_PWR_ENABLE() (PORTD.6 = 1);        //@@@
#define WIRELESS_PWR_DISABLE() (PORTD.6 = 0);

#define WIRELESS_CTS_ENABLE()   (DDRD.7 = 1);      //@@@
#define WIRELESS_CTS_DISABLE()  (DDRD.7 = 0);

#define WIRELESS_CTS_ON()   (PORTD.7 = 0);      //@@@
#define WIRELESS_CTS_OFF()  (PORTD.7 = 1);

#define SATELITE_PWR_ENABLE() (PORTC.4 = 1);   //@@@
#define SATELITE_PWR_DISABLE() (PORTC.4 = 0);

#define SATELITE_PWR_ON() (PORTC.2 = 1);   //@@@
#define SATELITE_PWR_OFF() (PORTC.2 = 0);

#define GPS_PWR_ON() (PORTB.0 = 1);   //@@@
#define GPS_PWR_OFF() (PORTB.0 = 0);

#define GPS_IGN_ON() (PORTB.1 = 1);   //@@@
#define GPS_IGN_OFF() (PORTB.1 = 0);

#define ENABLE_CLOCK_INT()   (EIMSK |= (1<<INT2));    // enable  external interrupt (clock int)
#define DISABLE_CLOCK_INT()  (EIMSK &= ~(1<<INT2));   // disable  external interrupt (clock int)

#define ENABLE_TIMER1_COMPA()   ( TIMSK1 |= (1<<OCIE1A));
#define DISABLE_TIMER1_COMPA()  ( TIMSK1 &= ~(1<<OCIE1A));   //

#define ENABLE_TIMER0()    (TCCR0B=(0<<WGM02) | (1<<CS02) | (0<<CS01) | (1<<CS00));
#define DISABLE_TIMER0()   (TCCR0B=(0<<WGM02) | (0<<CS02) | (0<<CS01) | (0<<CS00));

#define ENABLE_TIMER1()     (TCCR1B=(0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (1<<WGM12) | (0<<CS12) | (1<<CS11) | (0<<CS10));
#define DISABLE_TIMER1()   (TCCR1B=(0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (0<<WGM12) | (0<<CS12) | (0<<CS11) | (0<<CS10));

#define ENABLE_TIMER2()    (TCCR2B=(0<<WGM22) | (1<<CS22) | (1<<CS21) | (1<<CS20));
#define DISABLE_TIMER2()   (TCCR2B=(0<<WGM22) | (0<<CS22) | (0<<CS21) | (0<<CS20));

#define ENABLE_UART0()      (UCSR0B=(1<<RXCIE0) | (0<<TXCIE0) | (0<<UDRIE0) | (1<<RXEN0) | (1<<TXEN0) | (0<<UCSZ02) | (0<<RXB80) | (0<<TXB80));
#define DISABLE_UART0()     (UCSR0B=(0<<RXCIE0) | (0<<TXCIE0) | (0<<UDRIE0) | (0<<RXEN0) | (0<<TXEN0) | (0<<UCSZ02) | (0<<RXB80) | (0<<TXB80));

#define ENABLE_UART1()      (UCSR1B=(1<<RXCIE1) | (0<<TXCIE1) | (0<<UDRIE1) | (1<<RXEN1) | (1<<TXEN1) | (0<<UCSZ12) | (0<<RXB81) | (0<<TXB81));
#define DISABLE_UART1()     (UCSR1B=(0<<RXCIE1) | (0<<TXCIE1) | (0<<UDRIE1) | (0<<RXEN1) | (0<<TXEN1) | (0<<UCSZ12) | (0<<RXB81) | (0<<TXB81));

#define ENABLE_RX_INT_UART0()  (UCSR1B=(1<<RXCIE0) | (0<<TXCIE0) | (0<<UDRIE0) | (1<<RXEN0) | (1<<TXEN0) | (0<<UCSZ02) | (0<<RXB80) | (0<<TXB80));
#define DISABLE_RX_INT_UART0() (UCSR1B=(0<<RXCIE0) | (0<<TXCIE0) | (0<<UDRIE0) | (1<<RXEN0) | (1<<TXEN0) | (0<<UCSZ02) | (0<<RXB80) | (0<<TXB80));

#define ENABLE_RX_INT_UART1()  (UCSR1B=(1<<RXCIE1) | (0<<TXCIE1) | (0<<UDRIE1) | (1<<RXEN1) | (1<<TXEN1) | (0<<UCSZ12) | (0<<RXB81) | (0<<TXB81));
#define DISABLE_RX_INT_UART1() (UCSR1B=(0<<RXCIE1) | (0<<TXCIE1) | (0<<UDRIE1) | (1<<RXEN1) | (1<<TXEN1) | (0<<UCSZ12) | (0<<RXB81) | (0<<TXB81));

#define ENABLE_TAG_INT()    (PCICR=(0<<PCIE3) | (0<<PCIE2) | (0<<PCIE1) | (1<<PCIE0));
#define DISABLE_TAG_INT()   (PCICR=(0<<PCIE3) | (0<<PCIE2) | (0<<PCIE1) | (0<<PCIE0));

#define ENABLE_ADC()  (ADCSRA=(1<<ADEN) | (0<<ADSC) | (0<<ADATE) | (0<<ADIF) | (0<<ADIE) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0));
#define DISABLE_ADC() (ADCSRA=(0<<ADEN) | (0<<ADSC) | (0<<ADATE) | (0<<ADIF) | (0<<ADIE) | (0<<ADPS2) | (0<<ADPS1) | (0<<ADPS0));

#define WATCHDOG_ENABLE_STEP1() (WDTCSR=(0<<WDIE) | (1<<WDP3) | (1<<WDCE) | (1<<WDE) | (0<<WDP2) | (0<<WDP1) | (1<<WDP0));
#define WATCHDOG_ENABLE_STEP2() (WDTCSR=(0<<WDIE) | (1<<WDP3) | (0<<WDCE) | (1<<WDE) | (0<<WDP2) | (0<<WDP1) | (1<<WDP0));

#define WATCHDOG_DISABLE() 		(WDTCSR = 0x00) //WDE = 0

//#define BATTERY_nTimeCnt       10

#define CONNECTING_TIME     2   // the 5th minutes is the connecting time
#define RETRY_CONNECTIN_TIME    15

#define GET_REQUEST 0
#define SET_REQUEST 1

#define REQ_ID      0           // LOGGER id
#define REQ_MSR     10
#define REQ_INTRVL  20
#define REQ_TYPE    30
#define REQ_TIME    40
#define REQ_IP      41
#define REQ_PORT    42
#define REQ_APN     43
#define REQ_MCC     44
#define REQ_MNC     45
#define REQ_ROAMING 46
#define REQ_SCH     47  // START CONNECT HOUR
#define REQ_CPD     48  //CONNECTS PER DAY
#define REQ_CI      49  //CONNECTS INTERVAL
#define REQ_BTR     55
#define REQ_RSSI    56
#define REQ_TIMEZONE     57
#define REQ_VER     58
#define REQ_NUM_SEN     59
#define REQ_DISCNCT   61
#define REQ_UPPER_CARD  62
#define REQ_WL_SEN_ID   1//64
#define REQ_EPOCH   64

void putchar1(char c);

#ifdef DebugOnUart0
void putchar0(char c);
#endif DebugOnUart0

/////////////////////////////////////////////
// GPS_manager functions
////////////////////////////////////////////

void GPSMain();

/////////////////////////////////////////////
// Data_manager functions
////////////////////////////////////////////

//arrange the saving of sensors measurments results in the external eeprom
//the function return (1 = success) or (0 = failue)
char SaveMeasurments(int, char );

char GetMeasurments(char read_mode);

char InitDataBlocks();

//char InitSensorBlocks(/*char senIndex, char senType*/);

char PointersValidate();

char ResetReadPointer();

char ResetAllReadPointers();

void ResetPointers(BYTE senIndex);

char GetSensorInterval(char senType);

char IsMoreData();

char SaveMapRefParams(float lat, float lon);

int GetMapRefAddress(char * data);

//BYTE GetNextLocation2Send();
/////////////////////////////////////////////
// i2c_bus functions
////////////////////////////////////////////
void SendStartBit(void);

void SendStopBit(void);

void SendByte(unsigned char send_byte);

unsigned char TestAck(void);

void SendAck(void);

unsigned char RecByte(void);

unsigned char SendBuf(unsigned char adress, int length, unsigned char  *buffer);

unsigned char GetBuf(unsigned char adress, int length, unsigned char *buffer) ;

/////////////////////////////////////////////
// general functions
////////////////////////////////////////////

void WakeUpProcedure(void);

void PowerDownSleep ( void );

BYTE CheckSum( BYTE *buff, BYTE length, BYTE param );

int GetCheckSum( BYTE *buff, int length);

void TransmitBuf(char iPortNum);

void InitProgram( void );

void InitVarsForConnecting();

void cpu_e2_to_MemCopy( BYTE* to, char eeprom* from, BYTE length);

void cpu_flash_to_MemCopy( BYTE* to, char flash* from, BYTE length);

BYTE CopyFlashToBuf( BYTE* to, char flash* from);

void MemCopy( BYTE* to, BYTE* from, BYTE length);

int bytes2int(char* buf);

//int bytes2intBigEnd(char* buf);

void int2bytes(int int_pointer, char* buf);

long Bytes2Long(char* buf);

void Long2Bytes(long l, char* buf);

void Float2Bytes(float f, char* buf);

float Bytes2Float(char* buf);

BYTE IsConnectingTimeClose();

//copy from ram buf into cpu e2
void MemCopy_to_cpu_e2( char eeprom* to, BYTE* from, BYTE length);

void TurnOnLed(BYTE led, BYTE type);

void TurnOffLed();

void UART1Select(BYTE uartTarget);

void SetUART1BaudRate(BYTE rate);

void DeepSleep( void );

//void ConvertCurTimeToUTC(unsigned char* time);

//void SetTimer0ForReceiver();

//void ResetTimer0();

#ifdef DebugMode
void SendDebugMsg(flash unsigned char *bufToSend);

void PrintNum(long val);

//void PrintFltNum(float val);

#endif DebugMode

/////////////////////////////////////////////
// RTC functions
////////////////////////////////////////////
//power on initialization ruthin
//check if power flag is on, if yes you should preform reset to r.t.cclock
unsigned char IsPowerFlagOn(void);

//unsigned char ResetCommand(void);
void ResetCommand(void);

//unsigned char DisableClockIntr(void);
void DisableClockIntr(void);

unsigned char SetRealTime(void);

//read the clock data into buffer
unsigned char ReadTime(void);

//initiate the rtc at program startup
void InitRTC();

//unsigned char ResetClockIntr(unsigned char);
void ResetClockIntr();

void GetRealTime(void);

void SetRtc24Hour();

/////////////////////////////////////////////
// eeprom functions
////////////////////////////////////////////

char e2_readSeqBytes(unsigned int address, char read_length);

char e2_writePage(unsigned int address, char write_length, char* string_1);

/////////////////////////////////////////////
// modem_manager functions
////////////////////////////////////////////

void ModemMain();

char IsModemOn();

void PutString(unsigned char* dest, unsigned char* src, int len);

BYTE SetModemBaudRate();

/////////////////////////////////////////////
//
////////////////////////////////////////////
void StlModemMain();

void MeasureBatt();

unsigned int MeasureCharger(void);

void ConvertEpoch2SysTime(date_time_t* local_date_time, unsigned long epoch_counter);

char UpdateEpoch(char* newEpoch);
/////////////////////////////////////////////
// Wireless sensor functions
////////////////////////////////////////////

void MeasureMain();

BYTE GetMinInterval();

BYTE GetSensorType();

void InitSensorArray();

BYTE SendAlerts();

/////////////////////////////////////////////
// Monitor_manager functions
////////////////////////////////////////////

void MonitorMain();

#ifdef BlueToothOption
/////////////////////////////////////////////
// Bluetuth_manager functions
////////////////////////////////////////////

void BlueToothMain();

char IsBlueToothConnect();

#endif BlueToothOption

/////////////////////////////////////////////
// Each sensors type general functions
////////////////////////////////////////////
/*

/////////////////////////////////////////////
// w1_ds1820 functions:
/////////////////////////////////////////////
int w1_SensRead();

/////////////////////////////////////////////
// adc12bit functions:
/////////////////////////////////////////////
int MeasureADC12bit();

/////////////////////////////////////////////
// EC_Sm_Te function:
/////////////////////////////////////////////
int ech2o_te_read();


*/
#endif DEFINE_H