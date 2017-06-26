#include <stdio.h>
#include "define.h"

extern int iVoltage;

//Do battery measuring

// Read the AD conversion result
unsigned int ReadAdc(BYTE mux)
{
    ADMUX = mux | ADC_VREF_TYPE;
    // Delay needed for the stabilization of the ADC input voltage
    delay_us(10);
    // Start the AD conversion
    ADCSRA|=(1<<ADSC);
    // Wait for the AD conversion to complete
    while ((ADCSRA & (1<<ADIF))==0);
    ADCSRA|=(1<<ADIF);
    return ADCW;
}

unsigned int MeasureADC(BYTE mux)
{
    char i;
    unsigned int adc_res, hi_value, lo_value;
    unsigned long cur_value;
    ENABLE_ADC();
    delay_ms(10);

    // enable ADC conversion
    ADCSRA|=(1<<ADEN);

    hi_value = adc_res = 0;
    lo_value = 0xFFFF;

    for(i = 0; i < 10; i++)
    {
        cur_value = ReadAdc(mux);
//    #ifdef DebugMode
//    SendDebugMsg("\r\ncur_value= ");
//    PrintNum(cur_value);
//    #endif DebugMode
        cur_value = (cur_value * INT_VREF)/1024L;
//        #ifdef DebugMode
//        SendDebugMsg("\r\ncur_value(mV)= ");
//        PrintNum(cur_value);
//        #endif DebugMode
        delay_ms(10);
        if (hi_value < cur_value)
            hi_value = (int)cur_value;
        if (lo_value > cur_value)
            lo_value = (int)cur_value;
        adc_res += cur_value;
    }
    adc_res = adc_res - hi_value;
    adc_res = adc_res - lo_value;
    adc_res = adc_res / 8;
    DISABLE_ADC();
    return adc_res;
}

void MeasureBatt()
{
    // Analog Comparator initialization
    // PortA analog inputs init
    DDRA.1= 0;
    PORTA.1 = 0;

    iVoltage = MeasureADC(MUX_BATTERY);
    iVoltage *= 2;
    #ifdef DebugMode
    SendDebugMsg("\r\nbattery= ");
    PrintNum(iVoltage);
    #endif DebugMode
}

unsigned int MeasureCharger(void)
{
	char tmpBuf[32];
	unsigned int a2dValue, u32Calc;
//	uint32_t u32Calc;

	// adc value converted to milivolts.
	a2dValue = MeasureADC(MUX_CHARGE);
//	u32Calc = a2dValue*2560UL;	// reference voltage = 2560mV.
//	u32Calc = u32Calc/1024UL;	// a2d full scale = 1024 = 10bits
	//if( sprintf_P(tmpBuf, PSTR("ADC measure: %dmV."), (uint16_t)u32Calc )) UART_DBG(tmpBuf);
	/*
	 * Ichg = (Hprog*Vprog)/Rprog
	 * Hprog = 988
	 * Rprog = 3010
	 */
	u32Calc = (a2dValue*H_PROG)/R_PROG;	// Charge Current Monitor
    #ifdef DebugMode
    SendDebugMsg("\r\nCharge Current= ");
    PrintNum(u32Calc);
    #endif DebugMode
    return u32Calc;
//	if(sprintf_P(tmpBuf, PSTR("Charge Current Monitor: %dmA."), u32Calc)) UART_DBG(tmpBuf);
/*
	u32Calc = (u32Calc*100)/MAX_CHRG_CURRENT_200;	// Battery Charge Monitor Percent
	if(sprintf_P(tmpBuf, PSTR("Battery Charge: %d%%."), u32Calc)) UART_DBG(tmpBuf);*/

}
