/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-25     Tobby       the first version
 */
#include "adc.h"
#include "rtthread.h"
#include "rtdevice.h"
//#include "status.h"
//#include "key.h"
//#include "moto.h"
#include "flashwork.h"
//#include "gateway.h"
#include "board.h"

#define DBG_TAG "adc"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

extern uint8_t ValveStatus;

ADC_HandleTypeDef adc_handle;
DMA_HandleTypeDef dma_handle;

rt_thread_t ntc_work = RT_NULL;

uint8_t NTC_State = 0;
uint32_t adc_value[20];

static void MX_ADC_Init(void)
{

    /* USER CODE BEGIN ADC_Init 0 */

    /* USER CODE END ADC_Init 0 */

    ADC_ChannelConfTypeDef sConfig = { 0 };

    /* USER CODE BEGIN ADC_Init 1 */

    /* USER CODE END ADC_Init 1 */

    /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
     */
    adc_handle.Instance = ADC;
    adc_handle.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    adc_handle.Init.Resolution = ADC_RESOLUTION_12B;
    adc_handle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    adc_handle.Init.ScanConvMode = ADC_SCAN_DISABLE;
    adc_handle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    adc_handle.Init.LowPowerAutoWait = DISABLE;
    adc_handle.Init.LowPowerAutoPowerOff = DISABLE;
    adc_handle.Init.ContinuousConvMode = ENABLE;
    adc_handle.Init.NbrOfConversion = 1;
    adc_handle.Init.DiscontinuousConvMode = DISABLE;
    adc_handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    adc_handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    adc_handle.Init.DMAContinuousRequests = ENABLE;
    adc_handle.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    adc_handle.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_39CYCLES_5;
    adc_handle.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_39CYCLES_5;
    adc_handle.Init.OversamplingMode = DISABLE;
    adc_handle.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
    if (HAL_ADC_Init(&adc_handle) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Regular Channel
     */
    sConfig.Channel = ADC_CHANNEL_8;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
    if (HAL_ADC_ConfigChannel(&adc_handle, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN ADC_Init 2 */
    HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
    HAL_NVIC_DisableIRQ(ADC_IRQn);
    /* USER CODE END ADC_Init 2 */

}


void DMA1_Channel1_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel1_IRQn 0 */

  /* USER CODE END DMA1_Channel1_IRQn 0 */
  HAL_DMA_IRQHandler(&dma_handle);
  /* USER CODE BEGIN DMA1_Channel1_IRQn 1 */

  /* USER CODE END DMA1_Channel1_IRQn 1 */
}


void ADC1_IRQHandler(void)
{
    /* USER CODE BEGIN ADC1_IRQn 0 */

    /* USER CODE END ADC1_IRQn 0 */
    HAL_ADC_IRQHandler(&adc_handle);
    /* USER CODE BEGIN ADC1_IRQn 1 */

    /* USER CODE END ADC1_IRQn 1 */
}

double ADC_Voltage_Calc(void)
{
    uint32_t voltage_temp = 0;
    double real_voltage = 0;

    for (uint8_t i = 0; i < 20; i++)
    {
        voltage_temp += adc_value[i];
    }
    real_voltage = voltage_temp * 0.000040283203125 + 0.018;

    //LOG_D("The value of voltage is %f\n", real_voltage);
    return real_voltage;
}

void NTC_State_Save(uint8_t result)
{
    NTC_State = result;
}
uint8_t NTC_State_read(void)
{
    return NTC_State;
}
void NTC_Work_Callback(void *parameter)
{
    LOG_D("NTC With ADC is Init Success\r\n");
    while (1)
    {
        ADC_Voltage_Calc();

//        if(ADC_Voltage_Calc()<1.153 && GetNowStatus()!=NTCWarning)
//        {
//            NTC_State_Save(ValveStatus);
//            Warning_Enable_Num(8);
//        }
//        if(ADC_Voltage_Calc()>=1.168 && GetNowStatus()==NTCWarning)
//        {
//            WarUpload_GW(1,0,8,0);//NTC报警
//            Warning_Disable();
//            if(NTC_State_read())
//            {
//                Moto_Open(NormalOpen);
//            }
//            else
//            {
//                Moto_Close(NormalOff);
//            }
//        }
        rt_thread_mdelay(1000);
    }
}

void ADC_Init(void)
{
    MX_DMA_Init();
    MX_ADC_Init();
    HAL_ADC_Start_DMA(&adc_handle, (uint32_t*) adc_value, 20);
    ntc_work = rt_thread_create("ntc_work", NTC_Work_Callback, RT_NULL, 2048, 15, 10);
    rt_thread_startup(ntc_work);
}
