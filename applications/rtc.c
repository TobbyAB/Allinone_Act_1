/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-25     Tobby       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>
#include "pin_config.h"
#include "rtc.h"
#include "Flashwork.h"
//#include "moto.h"
#include "board.h"

#define DBG_TAG "RTC"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

uint8_t RTC_Counter=0;
uint32_t RTC_Hours = 0;

rt_sem_t RTC_IRQ_Sem;
rt_thread_t RTC_Scan = RT_NULL;
RTC_HandleTypeDef rtc_handle;

void RTC_Timer_Entry(void *parameter)
{
    while(1)
    {
        static rt_err_t result;
        result = rt_sem_take(RTC_IRQ_Sem, RT_WAITING_FOREVER);
        if (result == RT_EOK)
        {
//            if(RTC_Hours%120==0)
//            {
//                //Moto_Detect();
//            }
            if(RTC_Counter<24)
            {
//                Update_All_Time();//24小时更新全部时间
                RTC_Counter++;
            }
            else
            {
//                Update_All_Time();//24小时更新全部时间
//                Detect_All_Time();//25个小时检测计数器
                RTC_Counter=0;
            }
            LOG_D("Device RTC Detect,Hour is %d\r\n",RTC_Counter);
        }
    }
}

static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};
  RTC_AlarmTypeDef sAlarm = {0};

  /* USER CODE BEGIN RTC_Init 1 */
  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  rtc_handle.Instance = RTC;
  rtc_handle.Init.HourFormat = RTC_HOURFORMAT_24;
  rtc_handle.Init.AsynchPrediv = 127;
  rtc_handle.Init.SynchPrediv = 255;
  rtc_handle.Init.OutPut = RTC_OUTPUT_DISABLE;
  rtc_handle.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  rtc_handle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  rtc_handle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  rtc_handle.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  rtc_handle.Init.BinMode = RTC_BINARY_NONE;
  if (HAL_RTC_Init(&rtc_handle) != HAL_OK)
  {
      Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&rtc_handle, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&rtc_handle, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
      Error_Handler();
  }

  /** Enable the Alarm A
  */
  sAlarm.AlarmTime.Hours = 0x1;
  sAlarm.AlarmTime.Minutes = 0x0;
  sAlarm.AlarmTime.Seconds = 0x0;
  sAlarm.AlarmTime.SubSeconds = 0x0;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay = 0x1;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm_IT(&rtc_handle, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
  {
      Error_Handler();

  }
  /* USER CODE BEGIN RTC_Init 2 */
  LOG_D("RTC Alarm Set Ok\r\n");
  /* USER CODE END RTC_Init 2 */

}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *RtcHandle)
{
    RTC_TimeTypeDef sTime = {0};
    sTime.Hours = 0x0;
    sTime.Minutes = 0x0;
    sTime.Seconds = 0x0;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    if (HAL_RTC_SetTime(&rtc_handle, &sTime, RTC_FORMAT_BCD) != HAL_OK)
    {
      Error_Handler();
    }
    rt_sem_release(RTC_IRQ_Sem);
    RTC_Hours++;
}



void RTC_Init(void)
{
    __HAL_RCC_RTC_ENABLE();
    RTC_IRQ_Sem = rt_sem_create("RTC_IRQ", 0, RT_IPC_FLAG_FIFO);
    RTC_Scan = rt_thread_create("RTC_Scan", RTC_Timer_Entry, RT_NULL, 2048, 5, 5);
    if(RTC_Scan!=RT_NULL)
    {
        rt_thread_startup(RTC_Scan);
    }
    else
    {
        LOG_W("RTC Init Fail\r\n");
    }

    MX_RTC_Init();

    HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
}

void RTC_Alarm_IRQHandler(void)
{
  HAL_RTC_AlarmIRQHandler(&rtc_handle);
}
