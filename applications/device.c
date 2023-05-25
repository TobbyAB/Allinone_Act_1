/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-21     Tobby       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>
#include "button.h"
#include "device.h"
#include "pin_config.h"
#include "flashwork.h"
//#include "led.h"

#define DBG_TAG "device"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

rt_sem_t KEY2_ON_Sem = RT_NULL;
rt_sem_t KEY2_ON_Long_Sem = RT_NULL;
rt_sem_t KEY1_OFF_Sem = RT_NULL;
rt_sem_t KEY1_OFF_Long_Sem = RT_NULL;
rt_sem_t KEY2_ON_KEY1_OFF_Long_Sem = RT_NULL;

rt_sem_t DC_ON_Sem = RT_NULL;
rt_sem_t DC_OFF_Sem = RT_NULL;

rt_thread_t button_task = RT_NULL;

uint16_t KEY2_ON_Long_Sem_Counter = 0;
uint16_t KEY1_OFF_Long_Sem_Counter = 0;
uint8_t KEY2_ON_OnceFlag = 0;
uint8_t KEY1_OFF_OnceFlag = 0;
uint8_t KEY2_ON_KEY1_OFF_OnceFlag = 0;

uint8_t Key_Pause_Flag;
uint8_t DC_Status, DC_Status_Temp = 0;
uint8_t Hand_Switch_Status, Hand_Switch_Status_Temp = 0;

int8_t ANT_SW_Status = 0;
int32_t ANT_SW_Status_Temp = 0;
extern uint8_t Factory_Flag;

void Key_SemInit(void)
{
    KEY2_ON_Sem = rt_sem_create("KEY2_ON", 0, RT_IPC_FLAG_FIFO);
    KEY2_ON_Long_Sem = rt_sem_create("KEY2_ON_Long", 0, RT_IPC_FLAG_FIFO);
    KEY1_OFF_Sem = rt_sem_create("KEY1_OFF", 0, RT_IPC_FLAG_FIFO);
    KEY1_OFF_Long_Sem = rt_sem_create("KEY1_OFF_Long", 0, RT_IPC_FLAG_FIFO);
    KEY2_ON_KEY1_OFF_Long_Sem = rt_sem_create("KEY2_ON_KEY1_OFF_Long_Sem", 0, RT_IPC_FLAG_FIFO);

    DC_ON_Sem = rt_sem_create("DC_ON", 0, RT_IPC_FLAG_FIFO);
    DC_OFF_Sem = rt_sem_create("DC_OFF", 0, RT_IPC_FLAG_FIFO);
}
void Key_Pin_Init(void)
{
    rt_pin_mode(KEY2_ON, PIN_MODE_INPUT);
    rt_pin_mode(KEY1_OFF, PIN_MODE_INPUT);
    rt_pin_mode(DC_DET, PIN_MODE_INPUT);
    rt_pin_mode(HAND_SWITCH_DET, PIN_MODE_INPUT);

    rt_pin_mode(ANT_INT, PIN_MODE_OUTPUT);
    rt_pin_mode(ANT_EXT, PIN_MODE_OUTPUT);
}
void Key_IO_Init(void)
{
    Key_Pause_Flag = 0;
}
void Key_IO_DeInit(void)
{
    Key_Pause_Flag = 1;
}

void DC_ON_Sem_Release(void)
{
    rt_sem_release(DC_ON_Sem);
    LOG_D("Now is DC_ON\r\n");
}

void DC_OFF_Sem_Release(void)
{
    rt_sem_release(DC_OFF_Sem);
    LOG_D("Now is DC_OFF\r\n");
}

void KEY2_ON_Sem_Release(void *parameter)
{
    rt_sem_release(KEY2_ON_Sem);
    LOG_D("KEY2_ON is Down\r\n");
}
void KEY1_OFF_Sem_Release(void *parameter)
{
    rt_sem_release(KEY1_OFF_Sem);
    LOG_D("KEY1_OFF is Down\r\n");
}
void KEY2_ON_LongSem_Release(void *parameter)
{
    if (KEY2_ON_OnceFlag == 0)
    {
        if (KEY2_ON_Long_Sem_Counter > 6)
        {
            if (KEY1_OFF_Long_Sem_Counter == 0)
            {
                KEY2_ON_OnceFlag = 1;
                rt_sem_release(KEY2_ON_Long_Sem);
                LOG_D("KEY2_ON is Long\r\n");
            }
        }
        else
        {
            LOG_I("KEY2_ON Long Counter is %d", KEY2_ON_Long_Sem_Counter++);
        }
    }
}
void KEY1_OFF_LongSem_Release(void *parameter)
{
    if (KEY1_OFF_OnceFlag == 0)
    {
        if (KEY1_OFF_Long_Sem_Counter > 6)
        {
            KEY1_OFF_OnceFlag = 1;
            if (KEY2_ON_Long_Sem_Counter > 5)
            {
                rt_sem_release(KEY2_ON_KEY1_OFF_Long_Sem);
                LOG_D("KEY2_ON_KEY1_OFF is Long\r\n");
            }
            else
            {
                rt_sem_release(KEY1_OFF_Long_Sem);
                LOG_D("KEY1_OFF is Long\r\n");
            }
        }
        else
        {
            LOG_I("KEY1_OFF Long Counter is %d", KEY1_OFF_Long_Sem_Counter++);
        }
    }
}
void k0_KEY1_OFF_LongSem_Release(void)
{
    if (KEY2_ON_KEY1_OFF_OnceFlag == 0)
    {
        KEY2_ON_KEY1_OFF_OnceFlag = 1;
        rt_sem_release(KEY2_ON_KEY1_OFF_Long_Sem);
        LOG_D("KEY2_ON_KEY1_OFF is Down\r\n");
    }
}
void KEY2_ON_LongFree_Release(void *parameter)
{
    KEY2_ON_OnceFlag = 0;
    KEY2_ON_KEY1_OFF_OnceFlag = 0;
    KEY2_ON_Long_Sem_Counter = 0;
    LOG_D("KEY2_ON is LongFree\r\n");
}
void KEY1_OFF_LongFree_Release(void *parameter)
{
    KEY1_OFF_OnceFlag = 0;
    KEY1_OFF_Long_Sem_Counter = 0;
    LOG_D("KEY1_OFF is LongFree\r\n");
}
uint8_t Read_KEY2_ON_Level(void)
{
    if (Key_Pause_Flag)
    {
        return 1;
    }
    else
    {
        return rt_pin_read(KEY2_ON);
    }
}
uint8_t Read_KEY1_OFF_Level(void)
{
    if (Key_Pause_Flag)
    {
        return 1;
    }
    else
    {
        return rt_pin_read(KEY1_OFF);
    }
}

void RF_Switch_Inside_Pin_Init(void)
{
    led_ExAnt_off();
    Flash_Key_Change(55556666, 0);
    rt_pin_write(ANT_EXT, ANT_SW_Status);
    rt_pin_write(ANT_INT, !ANT_SW_Status);
    LOG_I("RF Switch Inside Ant  \r\n");
}

void RF_Switch_Outside_Pin_Init(void)
{
    led_ExAnt_on();
    Flash_Key_Change(55556666, 1);
    rt_pin_write(ANT_EXT, !ANT_SW_Status);
    rt_pin_write(ANT_INT, ANT_SW_Status);
    LOG_I("RF Switch Outside Ant  \r\n");
}

uint32_t Get_RF_Exant(void)
{
    uint32_t read_value = 0;
    char *keybuf = "55556666";
    read_value = Flash_Get_Key_Value(55556666);
    return read_value;
    LOG_D("Reading Key %s value %ld \r\n", keybuf, read_value);
}

void RF_Switch_Pin_Init(void)
{
    ANT_SW_Status_Temp = Get_RF_Exant();
    if (!ANT_SW_Status_Temp)
    {
        RF_Switch_Inside_Pin_Init();
    }
    else
    {
        RF_Switch_Outside_Pin_Init();
    }
}

void DC_Detect(void)
{
    DC_Status_Temp = rt_pin_read(DC_DET);
    if (DC_Status_Temp != DC_Status)
    {
        DC_Status = DC_Status_Temp;
        LOG_I("Now DC_Status is %d \r\n", DC_Status_Temp);
        if (DC_Status_Temp == 1)
        {
            DC_ON_Sem_Release();
        }
        else
        {
            DC_OFF_Sem_Release();
        }
    }
}

void Hand_Switch_Detect(void)
{
    Hand_Switch_Status_Temp = rt_pin_read(HAND_SWITCH_DET);
    if (Hand_Switch_Status_Temp != Hand_Switch_Status)
    {
        Hand_Switch_Status = Hand_Switch_Status_Temp;
        LOG_I("Now Hand_Switch_Status is %d \r\n", Hand_Switch_Status_Temp);
        if (Hand_Switch_Status_Temp == 0)
        {
            Moto_Open(NormalOpen);
        }
        else
        {
            Motor_Stop();
        }

    }
}

void button_task_entry(void *parameter)
{
    Key_SemInit();
    Key_Pin_Init();
    RF_Switch_Pin_Init();
    Button_t Key0;
    Button_t Key1;
    Button_Create("Key0", &Key0, Read_KEY2_ON_Level, 0);
    Button_Create("Key1", &Key1, Read_KEY1_OFF_Level, 0);
    Button_Attach(&Key0, BUTTON_DOWM, KEY2_ON_Sem_Release);

    Button_Attach(&Key1, BUTTON_DOWM, KEY1_OFF_Sem_Release);
    Button_Attach(&Key0, BUTTON_LONG, KEY2_ON_LongSem_Release);
    Button_Attach(&Key1, BUTTON_LONG, KEY1_OFF_LongSem_Release);
    Button_Attach(&Key0, BUTTON_LONG_FREE, KEY2_ON_LongFree_Release);
    Button_Attach(&Key1, BUTTON_LONG_FREE, KEY1_OFF_LongFree_Release);
    LOG_D("Button Init Success\r\n");
    while (1)
    {
        DC_Detect();
        Hand_Switch_Detect();
        Button_Process();
        rt_thread_mdelay(10);
    }
}
void Button_Init(void)
{
    button_task = rt_thread_create("button_task", button_task_entry, RT_NULL, 2048, 5, 10);
    rt_thread_startup(button_task);
}
