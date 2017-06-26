//smart_data_manager.c file (update 03-01-01)
///////// start of data manager module /////////
#include "define.h"

//define global variables
unsigned int pBwrite;		//pointer to current write sensor data block in ext_e2
unsigned int pBread;		//pointer to last read sensor data block in ext_e2
//unsigned int pBfirst;		//pointer to first write sensor data block in ext_e2
unsigned int pWrite;		//pointer to last sensor data write in ext_e2
unsigned int pRead;		//pointer to sensor's last read data in ext_e2
unsigned int pStart_space;	//pointer to start sensor data space in ext_e2
unsigned int pEnd_space;	//pointer to end sensor data space in ext_e2
static unsigned int pSens_ext_e2;	//pointer to current sensor control parameters in e
unsigned int last_cycle_min;//last cycle data store time in minutes from day start
unsigned int pOriginalReadBlock;
//extern eeprom char cpue2_interval_1;
extern eeprom BYTE eNumSensors;
extern eeprom struct WirelessSensor WLSenArr[];
//extern char cuurent_interval;  	//varible to hold the the current measuring interval
extern char e2_writeFlag;
extern int nUnreadBlocks;
extern char DataBlock[PCKT_LNGTH];
extern unsigned int arrReadPointer[MAX_SEN_NUM];
extern volatile unsigned char eepromReadBuf[SENSOR_CNTRL_PRM_SIZE];	//buffer for eeprom read operation
extern char readClockBuf[7];	         //buffer for data reading from clock
extern char clockBuf[7]; 		 //buffer for all clock operation need
extern unsigned char gUnreadValues;
extern BYTE objToMsr;
extern unsigned int time_in_minutes;     //time from day start in ninutes
extern int measure_time;
//extern int SensorResult;       //save the measuring result into variable
extern int gTrnsID;

/*
//write 2 bytes data into ext_e2
int WriteGPSIntoExte2(char *Data, int iAddress)
{
	//write data onto ext_e2
	if(!(e2_writePage(iAddress, 8, Data)))
		return 1;//if writing into ext_e2 faild exit faild
	//else
    iAddress += 8;
	return iAddress;
} */

//write 2 bytes data into ext_e2
int WriteIntoExte2(int intData, int intAddress, char sensorMeasur)
{
	char helpBuf[2];
	int address;
	address = intAddress;

	//check if next writing point will cross e2 block (256 bytes) limit
	int2bytes(address, helpBuf); //address drvide into 2 bytes
	if(helpBuf[1] == 0xFF)//if lo byte is at the e2 block limit
	{
		helpBuf[0] += 1;	//add 1 to hi byte
		helpBuf[1] = 0x00;//set 0 to lo byte
		//set write pointer
		address = bytes2int(helpBuf);
	}
	else
	{
		if(sensorMeasur) //if it is sensor measur data to be writen into e2
			//move pWrite to next write position
			address += 2;
    }
	//set data into 2 bytes
   	int2bytes(intData, helpBuf);

	//write data onto ext_e2
	if(!(e2_writePage(address, 2, helpBuf)))
		return 1;//if writing into ext_e2 faild exit faild
	//else
	return address;

}

//reset data block in ext_e2 (set FF7F into the block)
char ResetDataBlock(unsigned int block_address)
{
	char i;
	int tempAddress, resetData, pArr;

	pArr = block_address;
	resetData = 0xff7f;

	for(i = 0; i < PCKT_LNGTH / 2; i++) //write into data block (2 bytes x 24 = 48 bytes)
	{
		//tempAddress = e2_writePage(pArr, 8, resetArr);
		tempAddress = WriteIntoExte2(resetData, pArr, 0);
		//if writing into ext_e2 faild exit faild
		if(!(tempAddress == 1))
			pArr = tempAddress;
		else
			return FALSE;
		pArr += 2; //move pointer 2 bytes
	}
	return TRUE;
}

//the function will set address into 'pSens_ext_e2' and 'pSens_cpu_e2'
// (int global variables define in data manager module)
void SetCotrolParamAddress()
{
    //pSens_ext_e2 = sens1_control_param;
    // POINT TO 16 BYTES BEFORE END OF DATA BLOCK
    pSens_ext_e2 = SENSOR_MEMORY_START + ((objToMsr + 1) * SENSOR_MEMORY_SIZE) - SENSOR_CNTRL_PRM_SIZE;
}

//set next pBread position
//end of reading space is the first block (including first block)
//return 1 if there is new pBread address or 0 if not
char NextpReadAddress()
{
   //if read block = write block
    if(pBread == pBwrite)
		return FALSE; //exit the function: no more data

	//if read block + 48 > data end space
	if((pBread + PCKT_LNGTH) > pEnd_space)
 	{
		pBread = pStart_space;
		return TRUE;
	}

	//else
	pBread += PCKT_LNGTH;
	return TRUE;
}

//the function will copy one data block from ext_e2 (pBread address)
//into 'sensDataBlock' buffer in ram
//return 1 (succsess) or 0 (failure)
char CopyBlockIntoRam()
{
	char i, j, n; //, more_data;
	unsigned int tmpRead;
    #ifdef DebugMode
        SendDebugMsg("\r\nCopyBlockIntoRam");
        #endif DebugMode
	tmpRead = pBread;

	//disable other operation while run this function
    e2_writeFlag = 1;

	for(i = 0; i < PCKT_LNGTH / 8; i++) 			//read data block (8 bytes x 6 = 48 bytes)
	{
		//read ext e2 (from data block into read buf)
		if(!(e2_readSeqBytes(tmpRead, 8)))
		{
			e2_writeFlag = 0; 	// enable other use
			return FALSE;		//if reading fail, exit the program
		}
		//copy data from read buf into sensDataBlock
		for(j = 0; j < 8; j++)
		{
		    DataBlock[((int)j)+((int)i*8)] = eepromReadBuf[j];
		}
		tmpRead += 8; 			//move pointer 8 bytes
	}
	e2_writeFlag = 0; 				// enable other use
    //calculate num of real data (not dummy 0xff7f) in current read block
    n = 0;
    do
    {
        if ((DataBlock[8 + (n * 2)] == 0x7f) && (DataBlock[9 + (n * 2)] == 0xff))
            break;
        n++;
    }
    while (n < MAX_DATA_PER_PCKT);

    gUnreadValues = n;
//    #ifdef DebugMode
//    SendDebugMsg("\r\ngUnreadValues:");
//    PrintNum(gUnreadValues);
//    #endif DebugMode
	return TRUE;

}

//calculate the time from last measuring cycle
//(in 10 minutes resulution)
unsigned int TimeFromLastCycle()
{
	unsigned int temp;

	//if minute now <= minute last measur sycle
	if(measure_time <= last_cycle_min)
	{
		//add 24(houers)x60 = 1440 to minute now
		temp = measure_time + 1440;
		//calculate minutes from last cycle
		temp = temp - last_cycle_min;
	}
	else
		temp = measure_time - last_cycle_min;

	return temp;
}

//read control parameters from ext_e2 into ram
//arrange the pointers and parameters for manipulation
/*
char ReadExte2ToRam()
{
    BYTE bRes, n = 0;
    #ifdef DebugMode
    SendDebugMsg("\r\nReadExte2ToRam ");
    PrintNum(pSens_ext_e2);
    #endif DebugMode

    do
    {
        bRes = 1;
        //read the sensor control parameters from ext_e2 into the ram read_buf
        if (e2_readSeqBytes(pSens_ext_e2, 2) == FAILURE)
        {
            #ifdef DebugMode
            SendDebugMsg("\r\nReadExte2ToRam error1 ");
            #endif DebugMode
            //return FALSE;
            bRes = 0;
        }
        //sets pointer of current writing block
        pBwrite = bytes2int(eepromReadBuf);

        if (e2_readSeqBytes(pSens_ext_e2 + 2, 2) == FAILURE)
        {
            #ifdef DebugMode
            SendDebugMsg("\r\nReadExte2ToRam error2 ");
            #endif DebugMode
            //return FALSE;
            bRes = 0;
        }
        //set pointer of current writing index
        pWrite = bytes2int(eepromReadBuf);

        if (e2_readSeqBytes(pSens_ext_e2 + 4, 2) == FAILURE)
        {
            #ifdef DebugMode
            SendDebugMsg("\r\nReadExte2ToRam error3 ");
            #endif DebugMode
            //return FALSE;
            bRes = 0;
        }
        //sets pointer of current reading block
        pBread = bytes2int(eepromReadBuf);

        if (e2_readSeqBytes(pSens_ext_e2+6, 2) == FAILURE)
        {
            #ifdef DebugMode
            SendDebugMsg("\r\nReadExte2ToRam error4 ");
            #endif DebugMode
            //return FALSE;
            bRes = 0;
        }
        //sets pointer of current reading index
        pRead = bytes2int(eepromReadBuf);

        if (e2_readSeqBytes(pSens_ext_e2+8, 2) == FAILURE)
        {
            #ifdef DebugMode
            SendDebugMsg("\r\nReadExte2ToRam error5 ");
            #endif DebugMode
            //return FALSE;
            bRes = 0;
        }
        //set pointer to start data space
        pStart_space = bytes2int(eepromReadBuf);

        if (e2_readSeqBytes(pSens_ext_e2+10, 2) == FAILURE)
        {
            #ifdef DebugMode
            SendDebugMsg("\r\nReadExte2ToRam error6 ");
            #endif DebugMode
            //return FALSE;
            bRes = 0;
        }
        //set pointer to end data space
        pEnd_space = bytes2int(eepromReadBuf);

        if (e2_readSeqBytes(pSens_ext_e2+12, 2) == FAILURE)
        {
            #ifdef DebugMode
            SendDebugMsg("\r\nReadExte2ToRam error7 ");
            #endif DebugMode
            //return FALSE;
            bRes = 0;
        }
        //set last cycle time into variable
        last_cycle_min = bytes2int(eepromReadBuf);
        n++;
    }
    while ((bRes == 0) && (n < 2));
    if (bRes == 0)
    {
        #ifdef DebugMode
        SendDebugMsg("\r\nunable 2 read pointers. reset them ");
        #endif DebugMode
        ResetPointers(objToMsr);
    }
	//set interval into variable
//	cuurent_interval = eepromReadBuf[14];
    //set sensor ID into variablee
//    g_lID = Bytes2Long(eepromReadBuf+16);
//    #ifdef DebugMode
//    SendDebugMsg("\r\nReadExte2ToRam ");
//    PrintNum(pBwrite);
//    #endif DebugMode
}
*/

char ReadExte2ToRam()
{
    BYTE bRes, n = 0;
//    #ifdef DebugMode
//    SendDebugMsg("\r\nReadExte2ToRam ");
//    PrintNum(pSens_ext_e2);
//    #endif DebugMode

	//read the sensor control parameters from ext_e2 into the ram read_buf
    do
    {
	    bRes = e2_readSeqBytes(pSens_ext_e2, CONTROL_PARAM_LENGTH);
        n++;
//        #ifdef DebugMode
//        SendDebugMsg("\r\nReadSeqBytes ");
//        #endif DebugMode
        delay_ms(50);
    }
    while ((bRes == FAILURE) && (n < 3));

    if (bRes == FAILURE)
    {
        #ifdef DebugMode
        SendDebugMsg("\r\nFailed e2_readSeqBytes. reset pointers ");
        #endif DebugMode
        ResetPointers(objToMsr);
        return TRUE;
    }

	//sets pointer of current writing block
	pBwrite = bytes2int(eepromReadBuf);
	//set pointer of current writing index
	pWrite = bytes2int(eepromReadBuf+2);
	//sets pointer of current reading block
	pBread = bytes2int(eepromReadBuf+4);
	//sets pointer of current reading index
	pRead = bytes2int(eepromReadBuf+6);
	//set pointer to start data space
	pStart_space = bytes2int(eepromReadBuf+8);
	//set pointer to end data space
	pEnd_space = bytes2int(eepromReadBuf+10);
	//set last cycle time into variable
	last_cycle_min = bytes2int(&eepromReadBuf[12]);
    return TRUE;
}


char GetSensorInterval(char senType)
{
//    return WakeupInterval;

    char inrvl = 0;
    switch(senType)
    {
        case TYPE_NONE:
        case TYPE_PLANT:
        case TYPE_TENS:
        case TYPE_WPS:
        case TYPE_RAIN:
        case TYPE_SD:
        case TYPE_DER:
        case TYPE_SMS:
        case TYPE_PIVOT:
            inrvl = 6;
        break;
        default:
            inrvl = 6;
    }
    return inrvl;
}

//set time, interval, error flag into the block header
char SetBlockHeader(unsigned int block_address)
{
	char headerArr[8], i, arrByte;
//    unsigned char pTime[5];
	int tempAddress;
	int tempData;
	tempAddress = block_address;

	headerArr[0] = 'd'; 			//start of data block
	headerArr[1] = GetSensorInterval(GetSensorType());
	headerArr[2] = 0x00;
	headerArr[3] = readClockBuf[0]; //year
	headerArr[4] = readClockBuf[1]; //month
	headerArr[5] = readClockBuf[2]; //day
	headerArr[6] = readClockBuf[4]; //hour
	headerArr[7] = readClockBuf[5]; //minute
/*    // save measurment time in UTC
    ConvertCurTimeToUTC(pTime);
    MemCopy(&headerArr[3], pTime, 5);
#ifdef DebugMode
    SendDebugMsg("\r\nUTC now2:\0");
    putchar1(headerArr[3]);
    putchar1(headerArr[4]);
    putchar1(headerArr[5]);
    putchar1(headerArr[6]);
    putchar1(headerArr[7]);
#endif DebugMode
    */

	arrByte=0;

    // write header to memory
	for(i = 0; i < 4; i++)
	{
		//set data 2 bytes into int data
		tempData = bytes2int(&headerArr[arrByte]);
        //write into ext_e2
		tempAddress = WriteIntoExte2(tempData, tempAddress, 0);
		if(!(tempAddress == 1))
		{
            tempAddress += 2;
            arrByte += 2;
  		}
  		else
  			return FALSE;
	}

	return TRUE;
}

//open new data block
//set the write block pointer to the new place
char CreateNewDataBlock()
{
//    char new_pWrite_flag = 0;
//    #ifdef DebugMode
//    SendDebugMsg("CreateNewDataBlock");
//    #endif DebugMode
	//if write block + 48 = first block
/*	if((pBwrite + 48) == pBfirst)
	{
		new_pWrite_flag = 1;
		pBwrite = pBfirst;
		//if first block + 48 = the data space end
		if((pBfirst + 48) > (pEnd_space))
		{
			//if read block = first block move it with the new first block
			if(pBread == pBfirst)
		   		pBread = pStart_space;
		 	pBfirst = pStart_space;
		}
		else //first block not in data space end
		{
		 	pBread_vers_pBfirst();
		}
	}

	//if write block + 48 = data space end
	if(((pBwrite + 48) > pEnd_space)&&(new_pWrite_flag == 0))
	{
		new_pWrite_flag = 1;
		pBwrite = pBfirst;
		pBread_vers_pBfirst();
	}

	//if pBwrite not in one of the previos situation
    if(new_pWrite_flag == 0)
		pBwrite += 48;
*/
    	//if write block + 48 = data space end
	if ((pBwrite + PCKT_LNGTH) > pEnd_space)
		pBwrite = pStart_space;
    else
        pBwrite += PCKT_LNGTH;

    // if after increasing pBwrite it reaches pBread - means the pBwrite completed a whole writing cycle to the buffer without reading.
    //  increase the pBread pointer by one packet
    if (pBwrite == pBread)
        NextpReadAddress();

	if(ResetDataBlock(pBwrite) == FALSE)
		return FALSE;
	//set time, interval, error flag into the block header
	if(SetBlockHeader(pBwrite) == FALSE)
		return FALSE;
	//set write pointer to block first data
	//pWrite points 2 step before the next writing point
	pWrite = pBwrite + 6;

	return TRUE;
}

// Save the longitude and latitude of sensor in specific address in control param area in memory
char SaveMapRefParams(float lat, float lon)
{
    char l[4];
    unsigned int p;

	//set address for selected sensor into 'pSens_ext_e2'
	SetCotrolParamAddress();
    p = pSens_ext_e2 + CONTROL_PARAM_LENGTH;
     //sensor  Latitude
    Float2Bytes(lat, l);
	//write control parameters onto ext_e2
	if(!(e2_writePage(p, 4, l)))
		return FALSE;
    p += 4;
   //sensor longitude
     Float2Bytes(lon, l);
    //write control parameters onto ext_e2
    if(!(e2_writePage(p, 4, l)))
	return FALSE;
//    #ifdef DebugMode
//    if (lon != 0)
//    {
//        SendDebugMsg("\r\nSave Location: \0");
//        SendDebugMsg("\r\nobject: \0");
//        putchar1(objToMsr);
//        PrintNum((long)lat);
//        PrintNum((long)lon);
//        SendDebugMsg("\r\nat: \0");
//        PrintNum(p-4);
////        putchar1('\r');
//        delay_ms(200);
//    }
//    #endif DebugMode

    return TRUE;
}

// Get the longitude and latitude of sensor from specific address in control param area in memory
int GetMapRefAddress(unsigned char * data)
{
    unsigned int p, i;
	//set address for selected sensor into 'pSens_ext_e2'
	SetCotrolParamAddress();
    p = pSens_ext_e2 + CONTROL_PARAM_LENGTH;
    	//read the sensor control parameters from ext_e2 into the ram read_buf
	if(!(e2_readSeqBytes(p, 8)))
		return FALSE;
    for (i = 0; i < 8; i++)
        data[i] = eepromReadBuf[i];
//    #ifdef DebugMode
//    SendDebugMsg("\r\nGetMapRefAddress: \0");
//    //PrintNum((long)lon);
//    //PrintNum((long)lat);
//    for (i = 0; i < 8; i++)
//    {
//        putchar1(data[i]);
//        putchar1(' ');
//    }
//    SendDebugMsg("\r\nat: \0");
//    PrintNum(p);
//    #endif DebugMode
    return TRUE;
}


//save control parameters into sensor block in ext_e2
char SaveControlParam()
{
    char temp[2];
    char l[4];
    int p = pSens_ext_e2;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~
	int2bytes(pBwrite, temp);
    if(!(e2_writePage(p, 2, temp)))
		return FALSE;
    p += 2;

	int2bytes(pWrite, temp);
    if(!(e2_writePage(p, 2, temp)))
		return FALSE;
    p += 2;

    int2bytes(pBread, temp);
    if(!(e2_writePage(p, 2, temp)))
		return FALSE;
    p += 2;

	int2bytes(pRead, temp);
    if(!(e2_writePage(p, 2, temp)))
		return FALSE;
    p += 2;

	int2bytes(pStart_space, temp);
    if(!(e2_writePage(p, 2, temp)))
		return FALSE;
    p += 2;

	int2bytes(pEnd_space, temp);
    if(!(e2_writePage(p, 2, temp)))
		return FALSE;
    p += 2;
	int2bytes(measure_time, temp);
    if(!(e2_writePage(p, 2, temp)))
		return FALSE;
    p += 2;
//    temp[0] = GetSensorInterval(GetSensorType());
//    temp[1] = 0;
//    if (!(e2_writePage(p, 2,temp)))
//		return FALSE;
//    p += 2;
     //sensor ID
     Long2Bytes(WLSenArr[objToMsr].Id, l);
	//write control parameters onto ext_e2
	if(!(e2_writePage(p, 4, l)))
		return FALSE;
    // UP TO HERE ITS 18 bytes. CONTROL_PARAM_LENGTH
	return TRUE;
}

//save sensors measurments results in the external eeprom
//the function return (1 = success) or (0 = failue)
//char SaveMeasurments(int data, char *fData, char cType)
char SaveMeasurments(int data, char cType)
{
	int temp_address;
    unsigned int reqIntrvl, timeIntrvl;
    BYTE isNextSeqMsr = TRUE;
//    BYTE nDataLngth;
	//set address for selected sensor into 'pSens_ext_e2'
	SetCotrolParamAddress();
    #ifdef DebugMode
    SendDebugMsg("\r\nSaveMeasurments objToMsr ");
    putchar1(objToMsr+0x30);
    #endif DebugMode

//    nDataLngth = 2;
//    if (cType == TYPE_PIVOT)
//        nDataLngth = 8;

	//read control parameters from cpu_e2 into ram
	//arrange the pointers and parameters for manipulation
	if(ReadExte2ToRam() == FALSE)
    {
        #ifdef DebugMode
        SendDebugMsg("\r\nSaveMeasurme failed1 ");
        #endif DebugMode
		return FALSE;
    }
    //check if it is the first writing data cycle
    //if((pBfirst == pBwrite)&&(pWrite == pBwrite + 6)) //for the 24-04-01 version
    if(pWrite == pBwrite + 6)
    {
        if(ResetDataBlock(pBwrite) == FALSE)
        {
            #ifdef DebugMode
            SendDebugMsg("\r\nSaveMeasurme failed2 ");
            #endif DebugMode
            return FALSE;
        }
        //set time, interval, error flag into the block header
        if(SetBlockHeader(pBwrite) == FALSE)
        {
            #ifdef DebugMode
            SendDebugMsg("\r\nSaveMeasurme failed3 ");
            #endif DebugMode
            return FALSE;
        }
    }
	else //it is not the first data writing cycle
	{
        timeIntrvl = TimeFromLastCycle();
        #ifdef DebugMode
        SendDebugMsg("\r\nTimeFromLastCycle: \0");
        PrintNum(timeIntrvl);
        #endif DebugMode
        //if temp =, or +1, or -1, from current interval it is o.k.
        reqIntrvl = GetSensorInterval(cType) * INTERVAL_PARAM;
        if ((reqIntrvl == timeIntrvl)||\
            ((reqIntrvl+1) == timeIntrvl)||\
            ((reqIntrvl-1) == timeIntrvl))
            isNextSeqMsr = TRUE;
        else
            isNextSeqMsr = FALSE;
        //check if its not next sequence measure or write block is full
        if((isNextSeqMsr == FALSE) || ((pWrite + 2) >= (pBwrite + 48)))
		{
			if(CreateNewDataBlock() == FALSE)
            {
                #ifdef DebugMode
                SendDebugMsg("\r\nSaveMeasurme failed4 ");
                #endif DebugMode
                return FALSE;
            }
		}
	}
//    if (cType == TYPE_PIVOT)
//        temp_address = WriteGPSIntoExte2(fData, pWrite);
//    else
        temp_address = WriteIntoExte2(data, pWrite, 1);

	if (temp_address != 1)
		pWrite = temp_address;
	else
	{
        #ifdef DebugMode
        SendDebugMsg("\r\nSaveMeasurme failed5 ");
        #endif DebugMode
        return FALSE;
    }

    if (SaveControlParam() == FALSE)
    {
        #ifdef DebugMode
        SendDebugMsg("\r\nSaveMeasurme failed6 ");
        #endif DebugMode
        return FALSE;
    }

	return TRUE;
}

char ResetReadPointer()
{
    if (objToMsr > eNumSensors)
        return FALSE;

    #ifdef DebugMode
    SendDebugMsg("\r\nreset read pointer ");
    #endif DebugMode	//set address for selected sensor into 'pSens_ext_e2'
	SetCotrolParamAddress();

	//read control parameters from cpu_e2 into ram
	//arrange the pointers and parameters for manipulation
	if(ReadExte2ToRam() == FALSE)
		return FALSE;
    pBread = pOriginalReadBlock;
    pRead = pBread;

    if (SaveControlParam() == FALSE)
        return FALSE;
    #ifdef DebugMode
    SendDebugMsg(" OK\r\n");
    #endif DebugMode	//set address for selected sensor into 'pSens_ext_e2'

	return TRUE;
}

char ResetAllReadPointers()    //
{
    // save original objToMsr
    BYTE n = objToMsr;

    for (objToMsr = SENSOR1; objToMsr <= eNumSensors; objToMsr++)
    {
        if (arrReadPointer[objToMsr] != MAX_ADDRESS) // if send sensor data
        {
            #ifdef DebugMode
            SendDebugMsg("\r\nreset read pointer of sensor ");
            putchar1(objToMsr+0x30);
            #endif DebugMode
            //set address for selected sensor into 'pSens_ext_e2'
            SetCotrolParamAddress();

            //read control parameters from cpu_e2 into ram
            //arrange the pointers and parameters for manipulation
            if(ReadExte2ToRam() == FALSE)
                return FALSE;
            pBread = arrReadPointer[objToMsr];  // retrieve the read pointer
            pRead = pBread;
            if (SaveControlParam() == FALSE)
                return FALSE;
        }
    }
    // retrieve  objToMsr to original value
    objToMsr = n;
	return TRUE;
}


//get sensoer measurments results from the external eeprom
//read_mode = 0 - send all data (from 'first' block)
//read_mode = 1 - send un read data (from 'read' block)
//read_mode = 2 - next data block
//read_mode = 3 - last data block again
//arrange in 'sensDataBlock': more data|records; status; time; data;
//return 1 (succsess) or 0 (failure)
char GetMeasurments(char read_mode)
{
    int n;
    gUnreadValues = 0;
    #ifdef DebugMode
    SendDebugMsg("\r\nGetMeasurments objToMsr ");
    putchar1(objToMsr+0x30);
    #endif DebugMode
    if (objToMsr > eNumSensors)
        return FALSE;
	//set address for selected sensor into 'pSens_ext_e2'
	SetCotrolParamAddress();

	//read control parameters from cpu_e2 ('pSens_ext_e2' address) into ram
	//arrange the pointers and parameters for manipulation
	if(ReadExte2ToRam() == FALSE)
    {
        #ifdef DebugMode
        SendDebugMsg("\r\nGetMeasurments error 1 ");
        #endif DebugMode
		return FALSE;
    }
    //check if there is any data block in eeprom
	if(pWrite == pBwrite + 6) //there is no data records yet
	{
		return NO_DATA;
	}
//    #ifdef DebugMode
//    SendDebugMsg("\r\nGetMeasurments2 ");
//    #endif DebugMode

   	//set the proper address for data block to be read
	switch(read_mode)
	{
		case 1:
            if ((pBwrite == pBread) && (pWrite == pRead))
            {
//                #ifdef DebugMode
//                SendDebugMsg("\r\nno new data2 ");
//                #endif DebugMode
                return NO_DATA;
            }
            if (pBwrite >= pBread)
                n = pBwrite - pBread;
            else
            {
                n = pEnd_space - pBread + 1;
                n += pBwrite - pStart_space;
            }
            n /= PCKT_LNGTH;

            nUnreadBlocks = n + 1;
            if (nUnreadBlocks > MAX_PCKTS_PER_SENSOR)
                nUnreadBlocks = 1;
            // for satellite modem
            arrReadPointer[objToMsr] = pBread;
            // for GSM modem:
            pOriginalReadBlock = pBread;
//    #ifdef DebugMode
//    SendDebugMsg("\r\nnUnreadBlocks= ");
//    PrintNum(nUnreadBlocks);
//    #endif DebugMode
        break;
		case 2:
            NextpReadAddress();
            break;
        //dont chang pBread, start read from last block send
		case 3:
           break;
		default:
            return FALSE; //exit the function (failure)
	}
//    #ifdef DebugMode
//    SendDebugMsg("\r\nGetMeasurments3 ");
//    #endif DebugMode

	if(CopyBlockIntoRam() == FALSE) //copy data block at the pBread address
    {
        #ifdef DebugMode
        SendDebugMsg("\r\nGetMeasurments error2 ");
        #endif DebugMode
		return FALSE;
    }

    // if the block is the last one to send - keep the inside address
    if (pBwrite == pBread)
        pRead = pWrite;

	//save control parameters into ext_e2
	if (SaveControlParam() == FALSE)
    {
        #ifdef DebugMode
        SendDebugMsg("\r\nGetMeasurments error3 ");
        #endif DebugMode
		return FALSE;
    }

    #ifdef DebugMode
    SendDebugMsg("OK \r\n");
    #endif DebugMode

	//else
	return TRUE;
}

// check if there are more packets with data to this sensor and save the transmition ID in the header of sent block
char IsMoreData()
{
//    #ifdef DebugMode
//    SendDebugMsg("\r\IsMoreData");
//    #endif DebugMode

    if (objToMsr > eNumSensors)
        return FALSE;
	//set address for selected sensor into 'pSens_ext_e2'
	SetCotrolParamAddress();

	//read control parameters from cpu_e2 ('pSens_ext_e2' address) into ram
	//arrange the pointers and parameters for manipulation
	if(ReadExte2ToRam() == FALSE)
		return FALSE;

    // save transmisionID of buffer in header of buffer in 1st 2 bytes
//    WriteIntoExte2(gTrnsID, pBread, 0);

    //if read block = write block
    if(pBread == pBwrite)
        return FALSE;

    return TRUE;
}

//set new interval
//char SetNewInterval(char new_interval)
//{
//    //if interval in cpu_e2 == new interval do nothing
//    if((WakeupInterval == new_interval))
//        return TRUE;
//    if ((new_interval != 1) &&
//        (new_interval != 2) &&
//        (new_interval != 3) &&
//        (new_interval != 6) &&
//        (new_interval != 12)&&
//        (new_interval != 24))
//        return FALSE;
//    //if it is a new interval
//    //initiate the data blocks in ext_e2 for sensor 1   //15/10/13 - remove next line
////    if(InitDataBlocks(new_interval) == FALSE)
////        return FALSE;
//    //update interval in cpu_e2
//   WakeupInterval = new_interval;
//	//else if all o.k.
//	return TRUE;
//}

//check validity of sensors data pointers and interval
char PointersValidate()
{
//    #ifdef DebugMode
//    SendDebugMsg("PointersValidate\r\n");
//    #endif DebugMode
	//check sensor pointers validate:
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//read control parameters from cpu_e2 into ram
    SetCotrolParamAddress();

	//arrange the pointers and parameters for manipulation
	if(ReadExte2ToRam() == FALSE)
    {
        #ifdef DebugMode
        SendDebugMsg("\r\nPointersValidateError1\r\n");
        #endif DebugMode
		return FALSE;
    }
	//if pointer to write data block not in range exit failure
	if(!((pBwrite >= pStart_space) && (pBwrite < pEnd_space)))
    {
        #ifdef DebugMode
        SendDebugMsg("\r\nPointersValidateError2\r\n");
        SendDebugMsg("pStart_space: \r\n");
        PrintNum(pStart_space);
        SendDebugMsg("pEnd_space\r\n");
        PrintNum(pEnd_space);
        SendDebugMsg("pBwrite\r\n");
        PrintNum(pBwrite);
        SendDebugMsg("pBread\r\n");
        PrintNum(pBread);
        #endif DebugMode
        return FALSE;
    }
	//if pointer to read data block not in range exit failure
	if(!((pBread >= pStart_space) && (pBread < pEnd_space)))
    {
        #ifdef DebugMode
        SendDebugMsg("\r\nPointersValidateError3\r\n");
        #endif DebugMode
		return FALSE;
    }
//    if (cpue2_interval_1 != cuurent_interval)
//        return FALSE;
	//if all test is ok
	return TRUE;
}

void ResetPointers(BYTE senIndex)
{
    //set pointer to first data block
    pStart_space = SENSOR_MEMORY_START + (senIndex * SENSOR_MEMORY_SIZE);
    //set pointer of write data block
    pBwrite = pStart_space;    //sens1_data_start;
    //set write pointer
    pWrite = pStart_space; //sens1_data_start;
    pWrite += 6;
    //set read block pointer
    pBread = pStart_space; //sens1_data_start;
    pRead =  pStart_space;
    //set pointer to start data space
//    pStart_space = pStart_space; //sens1_data_start;
    //set pointer to end data space
    pEnd_space = pStart_space + SENSOR_MEMORY_SIZE - SENSOR_CNTRL_PRM_SIZE - 1;//SENSOR_MEMORY_START + (senIndex * SENSOR_MEMORY_SIZE) - SENSOR_CNTRL_PRM_SIZE - 1; //sens1_data_ends;
    //set parameters for sensor
    pSens_ext_e2 = pEnd_space + 1; //SENSOR_MEMORY_START + (senIndex * SENSOR_MEMORY_SIZE) - SENSOR_CNTRL_PRM_SIZE; //sens1_control_param;
    //set last cycle time into variable
    last_cycle_min = time_in_minutes;
    //set interval into variable
//    cuurent_interval = cpue2_interval_1;
    //save control parameters into ext_e2
    SaveControlParam();
//        return FALSE;
	//else if ok
//	return TRUE;
}

//initiate the data blocks in ext_e2 for all sensors
char InitDataBlocks(/*char interval*/)
{
    BYTE senIndex;
    for (senIndex = SENSOR1; senIndex < MAX_SEN_NUM; senIndex++)
        ResetPointers(senIndex);
	//else if ok
	return TRUE;
}


/*char InitSensorBlocks()
{
    //set pointer to first data block
    pStart_space = SENSOR_MEMORY_START + ((objToMsr - 1) * SENSOR_MEMORY_SIZE);    // sens1_data_start;
    //set pointer of write data block
    pBwrite = pStart_space;    //sens1_data_start;
    //set write pointer
    pWrite = pStart_space; //sens1_data_start;
    pWrite += 6;
    //set read block pointer
    pBread = pStart_space; //sens1_data_start;
    pRead =  pStart_space;
    //set pointer to start data space
//    pStart_space = pStart_space; //sens1_data_start;
    //set pointer to end data space
    pEnd_space = pStart_space + SENSOR_MEMORY_SIZE - SENSOR_CNTRL_PRM_SIZE - 1;//SENSOR_MEMORY_START + (senIndex * SENSOR_MEMORY_SIZE) - SENSOR_CNTRL_PRM_SIZE - 1; //sens1_data_ends;
    //set parameters for sensor
    pSens_ext_e2 = pEnd_space + 1; //SENSOR_MEMORY_START + (senIndex * SENSOR_MEMORY_SIZE) - SENSOR_CNTRL_PRM_SIZE; //sens1_control_param;
    //set last cycle time into variable
    last_cycle_min = time_in_minutes;
    //set interval into variable
//    cuurent_interval = GetSensorInterval(GetSensorType());
    //save control parameters into ext_e2
    if (SaveControlParam() == FALSE)
        return FALSE;
	//else if ok
	return TRUE;
}
*/
///////// end of data manager module ///////////
////////////////////////////////////////////////

