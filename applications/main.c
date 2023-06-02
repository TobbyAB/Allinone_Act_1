/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-18     RT-Thread    first version
 */

#include <rtthread.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define AGING
extern uint8_t Bat_Level;
int main(void)
{

    Flash_Init();
    ADC_Init();
    led_Init();
    Key_Reponse();
    WarningInit();
    RTC_Init();
    RF_Init();
    Moto_Init();
    Button_Init();
    WaterScan_Init();
    Gateway_Init();

#ifdef AGING1
    int count = 1;
    int aging_count = 1000;
#endif
    while (1)
    {
        rt_thread_mdelay(1000);

#ifdef AGING1

        if (count <= aging_count)
        {
            Actuator_Aging();
            LOG_D("Number of actuator aging times: %d", count);
        }
        else if (count <= (aging_count + 1))
        {
            LOG_W("actuator aging stop");
        }
#endif
    }

    return RT_EOK;
}
