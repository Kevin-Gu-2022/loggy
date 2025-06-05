/*
 * LM335.c
 *
 *  Created on: Apr 01, 2025
 *      Author: Lucy 2
 */

#include "LM335.h"

extern ADC_HandleTypeDef hadc1;

// read from temp sensor
float Read_LM335_ADC() {
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK) {  // wait max 100ms
        uint32_t adc_value = HAL_ADC_GetValue(&hadc1);
        float vout = Convert_ADC_To_Voltage(adc_value);
        float temperature = Convert_Voltage_To_Temperature(vout);
        HAL_ADC_Stop(&hadc1);

        return temperature;
    } else {
        SEGGER_RTT_printf(0, "ADC conversion timeout!\n");
        HAL_ADC_Stop(&hadc1);
        return 0;
    }
}

float Convert_ADC_To_Voltage(uint32_t adc_value) {
    return (adc_value / 4095.0) * VREF;  // Convert ADC value to voltage
}

float Convert_Voltage_To_Temperature(float vout) {
    return (vout / 0.01) - 273.15;  // Convert voltage to temperature in °C
}
