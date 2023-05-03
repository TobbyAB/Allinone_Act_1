/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-26     Tobby       the first version
 */

#include "rtthread.h"
#include "rtdevice.h"
#include "pin_config.h"
#include "led.h"
//#include "key.h"
#include "moto.h"
#include "flashwork.h"
#include "status.h"
//#include "gateway.h"
//#include "radio_encoder.h"
#include "device.h"
//#include "work.h"

#define DBG_TAG "motor"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>



rt_timer_t ON_Pos_Detect_Timer = RT_NULL;
rt_timer_t OFF_Pos_Detect_Timer = RT_NULL;
rt_timer_t In_ON_Pos_Detect_Timer = RT_NULL;
rt_timer_t In_OFF_Pos_Detect_Timer = RT_NULL;

rt_timer_t Actuator_SelfCheck_Timer = RT_NULL;
rt_timer_t Moto_Detect_Timer = RT_NULL;

uint8_t Actuator_Open_Flag;
uint8_t Actuator_Close_Flag;
uint8_t Actuator_SelfCheck_Flag;

uint8_t Moto1_Fail_FLag;
uint8_t Valve_Alarm_Flag;

extern uint8_t ValveStatus;
extern enum Device_Status Now_Status;
extern Device_Info Global_Device;

void ON_Pos_Detect_Timer_Callback(void *parameter)
{
    if (rt_pin_read(ON_POS) == 0)
    {
        LOG_I("The motor in the ON position");
        //rt_thread_mdelay(500);
        Actuator_Stop();
    }
}

void OFF_Pos_Detect_Timer_Callback(void *parameter)
{
    if (rt_pin_read(OFF_POS) == 0)
    {
        LOG_I("The motor in the OFF position");
        //rt_thread_mdelay(500);
        Actuator_Stop();
    }
}

void In_ON_Pos_Detect_Timer_Callback(void *parameter)
{
    if (rt_pin_read(ON_POS) == 1)
    {
        Motor_Stop();
        rt_timer_stop(ON_Pos_Detect_Timer);
        LOG_W("he motor has not reached the ON position");
    }
}

void In_OFF_Pos_Detect_Timer_Callback(void *parameter)
{
    if (rt_pin_read(OFF_POS) == 1)
    {
        Motor_Stop();
        rt_timer_stop(OFF_Pos_Detect_Timer);
        LOG_W("he motor has not reached the OFF position");
    }
}

void Actuator_SelfCheck_Timer_Callback(void *parameter)
{
    if (rt_pin_read(ON_POS) == 1)
    {

        Actuator_Open();

        WarUpload_GW(1, 0, 2, 0); //MOTO1解除报警
        Valve_Alarm_Flag = 0;
        Moto1_Fail_FLag = 0;

        rt_thread_mdelay(5000);
        Key_IO_Init();
        WaterScan_IO_Init();

        LOG_D("Actuator_SelfCheck is success");
    }
    else
    {
        Warning_Enable_Num(6);
        Valve_Alarm_Flag = 1;
        Moto1_Fail_FLag = 1;

        Actuator_Stop();
        Key_IO_Init();
        WaterScan_IO_Init();

        LOG_D("Actuator Self-test is failure");
    }
}

void Moto_Detect_Timer_Callback(void *parameter)
{
    LOG_D("Moto_Detect_Timer_Callback\r\n");
    Moto_Detect();
}



void Moto_Init(void)
{
    rt_pin_mode(MOT_IN1, PIN_MODE_OUTPUT);
    rt_pin_mode(MOT_IN2, PIN_MODE_OUTPUT);

    rt_pin_mode(ON_POS, PIN_MODE_INPUT);
    rt_pin_mode(OFF_POS, PIN_MODE_INPUT);

    ON_Pos_Detect_Timer = rt_timer_create("ON_Pos_Detect_Timer", ON_Pos_Detect_Timer_Callback, RT_NULL, 10,
    RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER);

    OFF_Pos_Detect_Timer = rt_timer_create("OFF_Pos_Detect_Timer", OFF_Pos_Detect_Timer_Callback, RT_NULL, 10,
    RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER);

    Actuator_SelfCheck_Timer = rt_timer_create("Actuator_SelfCheck_Timer", Actuator_SelfCheck_Timer_Callback, RT_NULL,
            6000,
            RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);

    In_ON_Pos_Detect_Timer = rt_timer_create("In_ON_Pos_Detect_Timer", In_ON_Pos_Detect_Timer_Callback, RT_NULL, 30000,
    RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);

    In_OFF_Pos_Detect_Timer = rt_timer_create("In_OFF_Pos_Detect_Timer", In_OFF_Pos_Detect_Timer_Callback, RT_NULL,
            30000,
            RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);

    Moto_Detect_Timer = rt_timer_create("Moto_Detect", Moto_Detect_Timer_Callback, RT_NULL, 60 * 1000 * 5,
    RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);

//    if (Flash_Get_SlaveAlarmFlag())
//    {
//        Warning_Enable_Num(2);
//        LOG_I("Moto is Init Fail,Last is Slaver Alarm\r\n");
//        return;
//    }
//    if (Global_Device.LastFlag != OtherOff)
//    {
//        Moto_InitOpen(NormalOpen);
//    }
//    LOG_D("Moto is Init Success,Flag is %d\r\n", Global_Device.LastFlag);
}

void Moto_Detect(void)
{
    if (ValveStatus == 1)
    {
        LOG_I("Actuator_Detect to start\r\n");
        Moto1_Fail_FLag = 0;

        Key_IO_DeInit();
        WaterScan_IO_DeInit();

        Motor_Close();
        rt_timer_start(Actuator_SelfCheck_Timer);
    }
}

MSH_CMD_EXPORT(Moto_Detect, Moto_Detect);

void Motor_Open(void)
{
    rt_pin_write(MOT_IN1, 0);
    rt_pin_write(MOT_IN2, 1);
//LOG_D("Now Motor start to open");
}

void Motor_Close(void)
{
    rt_pin_write(MOT_IN1, 1);
    rt_pin_write(MOT_IN2, 0);
//LOG_D("Now Motor start to close");
}

void Motor_Stop(void)
{
    rt_pin_write(MOT_IN1, 0);
    rt_pin_write(MOT_IN2, 0);
//LOG_D("Now Motor is stop");
}

void Clean_Actuator_Timer(void)
{
    rt_timer_stop(OFF_Pos_Detect_Timer);
    rt_timer_stop(In_OFF_Pos_Detect_Timer);
    rt_timer_stop(ON_Pos_Detect_Timer);
    rt_timer_stop(In_ON_Pos_Detect_Timer);
//LOG_D("Clean Actuator Timer");
}

void Actuator_Open(void)
{
    Clean_Actuator_Timer();
    Motor_Open();
    rt_timer_start(ON_Pos_Detect_Timer);
    rt_timer_start(In_ON_Pos_Detect_Timer);
    LOG_I("Now Actuator command to open");
}
MSH_CMD_EXPORT(Actuator_Open, Actuator_Open);

void Actuator_Close(void)
{
    Clean_Actuator_Timer();
    Motor_Close();
    rt_timer_start(OFF_Pos_Detect_Timer);
    rt_timer_start(In_OFF_Pos_Detect_Timer);
    LOG_I("Now Actuator command to close");
}
MSH_CMD_EXPORT(Actuator_Close, Actuator_Close);

void Actuator_Stop(void)
{
    Clean_Actuator_Timer();
    Motor_Stop();
    LOG_I("Now Actuator is stop");
}
MSH_CMD_EXPORT(Actuator_Stop, Actuator_Stop);

void Actuator_Aging(void)
{
    Actuator_Open();
    rt_thread_mdelay(15000);
    Actuator_Close();
    rt_thread_mdelay(14000);
}
MSH_CMD_EXPORT(Actuator_Aging, Actuator_Aging);

/*******************
 Actuator_Open
 Actuator_Close
 Actuator_Stop
 Actuator_Detect
 *******************/

void Moto_InitOpen(uint8_t ActFlag)
{
    LOG_I("Moto Open Init Now is is %d , act is %d\r\n", Global_Device.LastFlag, ActFlag);
    if ((Global_Device.LastFlag == OtherOff && ActFlag == OtherOpen) || (Global_Device.LastFlag != OtherOff))
    {
        LOG_D("Moto is Open\r\n");
        Now_Status = Open;
        led_valve_on();
        ValveStatus = 1;
        Global_Device.LastFlag = ActFlag;
        Flash_Moto_Change(ActFlag);
        Actuator_Open();
    }
    else if (Global_Device.LastFlag == OtherOff && ActFlag == NormalOpen)
    {
        led_valve_fail();
        LOG_D("No permissions to Open\r\n");
    }
    else
    {
        led_notice_once();
        LOG_D("No permissions to Open\r\n");
    }
}

void Moto_Open(uint8_t ActFlag)
{
    LOG_I("Moto Open Now is is %d , act is %d\r\n", Global_Device.LastFlag, ActFlag);
    if ((Global_Device.LastFlag == OtherOff && ActFlag == OtherOpen) || (Global_Device.LastFlag != OtherOff))
    {
        LOG_D("Moto is Open\r\n");
        Now_Status = Open;
        led_valve_on();
        ValveStatus = 1;
        Global_Device.LastFlag = ActFlag;
        Flash_Moto_Change(ActFlag);

        Actuator_Open();

        if (ActFlag == NormalOpen)
        {
            ControlUpload_GW(1, 0, 1, 1);
            rt_timer_start(Moto_Detect_Timer);
        }
        Delay_Timer_Stop();
    }
    else if (Global_Device.LastFlag == OtherOff && ActFlag == NormalOpen)
    {
        led_valve_fail();
        LOG_D("No permissions to Open\r\n");
    }
}
MSH_CMD_EXPORT(Moto_Open, Moto_Open);

void Moto_Close(uint8_t ActFlag)
{
    LOG_I("Moto Close Now is is %d , act is %d\r\n", Global_Device.LastFlag, ActFlag);
    if (Global_Device.LastFlag != OtherOff)
    {
        LOG_D("Moto is Close\r\n");
        Now_Status = Close;
        led_valve_off();
        ValveStatus = 0;
        Global_Device.LastFlag = ActFlag;
        Flash_Moto_Change(ActFlag);
        if (ActFlag == NormalOff)
        {
            ControlUpload_GW(1, 0, 1, 0);
        }
        Actuator_Close();
        Delay_Timer_Stop();

        Key_IO_Init();
        WaterScan_IO_Init();
    }
    else if (Global_Device.LastFlag == OtherOff && ActFlag == OtherOff)
    {
        Now_Status = Close;
        ValveStatus = 0;
        beep_once();
        Delay_Timer_Stop();
        LOG_D("Moto is alreay otheroff\r\n");
    }
    else
    {
        led_valve_fail();
        LOG_D("No permissions to Off\r\n");
    }
}
MSH_CMD_EXPORT(Moto_Close, Moto_Close);

uint8_t Get_Moto1_Fail_FLag(void)
{
    return Moto1_Fail_FLag;
}

