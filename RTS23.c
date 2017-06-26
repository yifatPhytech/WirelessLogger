/*******************************************************
This program was created by the
CodeWizardAVR V2.60 Standard
Automatic Program Generator
© Copyright 1998-2012 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : WirelessLogger
Version : W.4.116.1
Date    : 17/04/2016
Author  : Yifat
Company : Phytech LTD
Comments:


Chip type               : ATmega644P
Program type            : Application
AVR Core Clock frequency: 3.685400 MHz
Memory model            : Small
External RAM size       : 0
Data Stack size         : 1024
*******************************************************/

#include <mega644p.h>
#include "define.h"
#include "vars.h"

// Declare your global variables here

void main(void)

{
    // Global enable interrupts
    #asm("sei")

    #asm ("wdr");         //reset the watchdog timer
//    WATCHDOG_ENABLE();     //set the watchdog
    //~~~~~~~~~~~check the startup reason~~~~~~~~~~~~~
    //the MCUSR should be set to 0x00 at the PowerDownSleep() function
    powerOnReset = FALSE;   // save reset source
    bExtReset = FALSE;
    bReset = FALSE;
    mainTask = TASK_NONE;
//    bPwrRst = FALSE;
    tagSwitched = FALSE;
    modemOnStartUp = FALSE;

    // Reset Source checking
    if (MCUSR & (1<<PORF)) //poweron reset
    {
        PORTA.5 = 1;
        delay_ms(500);
        PORTA.5 = 0;
        powerOnReset = TRUE;
    }
    else
        if (MCUSR & (1<<EXTRF))     //ext. reset
        {
            //err_buf[EXT_RESET] = EXT_RESET;
            bExtReset = TRUE;
        }
        else
            if (MCUSR & (1<<WDRF))      //WATCH DOG RESET
            {
                //err_buf[WATCHDOG_RESET] = WATCHDOG_RESET;
                PORTA.7 = 1;
                delay_ms(500);
                PORTA.7 = 0;
                bExtReset = TRUE;
            }

    MCUSR = 0x00; //reset the reset source flags to 0
    // init all IO's and vars
    if (powerOnReset)
        DeepSleep();
    InitProgram();
    if ((eLoggerID[0] == 0) && (eLoggerID[1] == 0) && (eLoggerID[2] == 0) && (eLoggerID[3] == 0))
        if (!SetModemBaudRate())
            return;
        else
        {
            mainTask = TASK_MONITOR;
            monitorCurTask = TASK_MONITOR_CONNECT;
        #ifdef DebugMode
            SendDebugMsg("\r\nSetBaudrate OK\0");
        #endif DebugMode
        }
    while (1)
    {
        switch (mainTask)
        {
            case TASK_MEASURE:
                MeasureMain();
                if (bEndOfMeasureTask == TRUE)
                {
                    bEndOfMeasureTask = FALSE;
//                    #ifdef DebugMode
//                    mainTask = TASK_SLEEP;
//                    #else
                    if ((((bConnectNow == TRUE) && (IsConnectingTimeClose() == FALSE)) || (bExtReset == TRUE))&& (btrStatus == BTR_FULL))// || (tagSwitched == TRUE))
                        InitVarsForConnecting();
                    else
                    {
                        mainTask = TASK_SLEEP;
                        /*
                        if (SendAlerts() == TRUE)      // todo - put back if needed
                        {
                            toDoList = DO_DATA;
                            #ifdef DebugMode
                            SendDebugMsg("\r\nSend Alert");
                            #endif DebugMode
                            if (IsConnectingTimeClose() == FALSE)
                                InitVarsForConnecting();
                            #ifdef DebugMode
                            else
                                SendDebugMsg("\r\nDelay connecting cos usual connection is in few minutes");
                            #endif DebugMode
                        }    */
                    }
//                        #endif DebugMode
                    #ifdef DebugMode
                    SendDebugMsg("\r\nEnd of measure ");
                    #endif DebugMode
                }
                break;
            case TASK_MONITOR:
                MonitorMain();
                if (bEndOfMonitorTask == TRUE)
                {
//                    UART_1_WIRELESS();
                    bEndOfMonitorTask = FALSE;
//                    #ifdef DebugMode
//                    mainTask = TASK_SLEEP;
//                    #else
                    //InitVarsForConnecting();
                    mainTask = TASK_GPS;//TASK_MEASURE;
                    msrCurTask = TASK_NONE;
                    //delay_ms(3000);
                    //#endif DebugMode
                    #ifdef DebugMode
                    SendDebugMsg("\r\nEnd of Monitor\0");
                    #else
//                    DISABLE_UART1();
                    #endif DebugMode
                }
            break;
            #ifdef BlueToothOption
            case TASK_BLUETOOTH:
                BlueToothMain();
                if (bEndOfBTTask == TRUE)
                {
                    PORTD.4 = 0;
                    bEndOfBTTask = FALSE;
                    mainTask = TASK_SLEEP;
                }
                break;
            #endif BlueToothOption
            case TASK_MODEM:
                if (bModemType == MODEM_GSM)
                    ModemMain();
                else
                    StlModemMain();
                if (bEndOfModemTask == TRUE)
                {
                    bEndOfModemTask = FALSE;
                    DISABLE_UART0();
                    if (modemOnStartUp == TRUE)
                    {
                        modemOnStartUp = FALSE;
                        mainTask = TASK_WAKEUP;
                    }
                    else
                    {
                        mainTask = TASK_SLEEP;
                        if (bModemType == MODEM_GSM)
                        {
                            if (bConnectOK == FALSE)
                            {
                                // if connection failed and dont know yet if there is satellite modem - try it
                                if ((bStlMdmStatus == SATELLITE_MODEM_UNCHECKED) && ((bExtReset == TRUE) || (nFailureCntr >= 2)))
                                {
                                    bModemType = MODEM_SATELLITE;
                                    nFailureCntr = 0;
                                    #ifdef DebugMode
                                    SendDebugMsg("\r\nswap to MODEM_SATELLITE");
                                    #endif DebugMode
                                    // try satellite now
                                    InitVarsForConnecting();
                                }
                                else
                                    if (bExtReset == TRUE)
                                        timeout4Cnct = 1;      //@@@ 10
                            }
                        }
                        else //                bModemType == MODEM_SATELLITE
                            if (flagSucOrFail == FALSE)
                            {
                                // if tried to connect satellite modem but it doesnt exist - reset GSM modem, forever
                                if (bStlMdmStatus == SATELLITE_MODEM_NOT_CONNECTED)
                                {
                                    bModemType = MODEM_GSM;
                                    nFailureCntr = 0;
                                    #ifdef DebugMode
                                    SendDebugMsg("\r\nswap to MODEM_GSM");
                                    #endif DebugMode
                                }
                                else
                                    if ((nFailureCntr == 1) && (bExtReset == TRUE))
                                        timeout4Cnct = 10; //@@@ 10
                                    if ((stlRegStatus == REG_FAILED) && (strRegFailCnt <= MAX_REG_FAILURES))
                                        timeout4Cnct = 3;
                            }
                    }
                }
                break;
            case TASK_SLEEP:
                PowerDownSleep();
                break;
            case TASK_WAKEUP:
                WakeUpProcedure();
                break;
            case TASK_GPS:
                GPSMain();
                if (bEndOfGPSTask == TRUE)
                {
                    #ifdef DebugMode
                    SendDebugMsg("\r\nend of gps\0");
                    PrintNum((long)g_fLat);
                    PrintNum((long)g_fLon);
                    #endif DebugMode
                    bEndOfGPSTask = FALSE;
                    mainTask = TASK_MEASURE;
                    msrCurTask = TASK_NONE;
                }
            break;
            default:
                mainTask = TASK_SLEEP;
        }

//        if (bReset == TRUE)
//        {
//            #ifdef DebugMode
//            putchar1('R');
//            #endif DebugMode
//            bReset = FALSE;
//            bExtReset = TRUE;
//            mainTask = TASK_WAKEUP;
//        }

        #asm ("wdr"); 		//reset the watchdog timer
    }
}

