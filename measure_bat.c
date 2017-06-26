#include <stdio.h>
#include "define.h"

extern int iVoltage;

//Do battery measuring

// Read the AD conversion result
unsigned int ReadAdc()
{
    ADMUX = 1 | ADC_VREF_TYPE;
    // Delay needed for the stabilization of the ADC input voltage
    delay_us(10);
    // Start the AD conversion
    ADCSRA|=(1<<ADSC);
    // Wait for the AD conversion to complete
    while ((ADCSRA & (1<<ADIF))==0);
    ADCSRA|=(1<<ADIF);
    return ADCW;
}

void MeasureBatt()
{
    char i;
    unsigned int adc_res, hi_value, lo_value;
    unsigned long cur_value;
    // Analog Comparator initialization
    // PortA analog inputs init
    DDRA.1= 0;
    PORTA.1 = 0;
    ENABLE_ADC();
    delay_ms(10);

    // enable ADC conversion
    ADCSRA|=(1<<ADEN);

    hi_value = adc_res = 0;
    lo_value = 0xFFFF;

    for(i = 0; i < 10; i++)
    {
        cur_value = ReadAdc();
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

    iVoltage = adc_res * 2;
    #ifdef DebugMode
    SendDebugMsg("\r\nbattery= ");
    PrintNum(iVoltage);
    #endif DebugMode

}

