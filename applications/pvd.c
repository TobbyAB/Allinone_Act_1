/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-28     Tobby       the first version
 */
#include <board.h>
#include <rtthread.h>

#define DBG_TAG "PVD"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

uint8_t Bat_Level;
static uint8_t Detect_Level;

PWR_PVDTypeDef sConfigPVD;

void PVD_PVM_IRQHandler(void)
{
    HAL_PWREx_PVD_PVM_IRQHandler();
    Bat_Level = 1;
//    switch(Detect_Level)
//    {
//    case 0://80
//        sConfigPVD.PVDLevel = PWR_PVDLEVEL_3;
//        Bat_Level = 3;
//        Detect_Level = 1;
//        break;
//    case 1://60
//        sConfigPVD.PVDLevel = PWR_PVDLEVEL_2;
//        Bat_Level = 4;
//        Detect_Level = 2;
//        break;
//    case 2://40
//        sConfigPVD.PVDLevel = PWR_PVDLEVEL_1;
//        Bat_Level = 1;
//        Detect_Level = 3;
//        break;
//    case 3://10
//        Detect_Level = 4;
//        Bat_Level = 2;
//        break;
//    default:break;
//    }
//    HAL_PWR_ConfigPVD(&sConfigPVD);
    LOG_I("Bat_Level is %d now\r\n",Bat_Level);
}

void PVD_Init(void)
{
    HAL_NVIC_SetPriority(PVD_PVM_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(PVD_PVM_IRQn);

    sConfigPVD.PVDLevel = PWR_PVDLEVEL_4;
    sConfigPVD.Mode = PWR_PVD_MODE_IT_RISING_FALLING;
    HAL_PWR_ConfigPVD(&sConfigPVD);
    PVD_Open();
}
void PVD_Open(uint8_t level)
{
    LOG_D("Enable the PVD Output\r\n");
    HAL_PWR_EnablePVD();
}

void PVD_Close(void)
{
    LOG_D("Disable the PVD Output\r\n");
    HAL_PWR_DisablePVD();
}

uint8_t Get_Bat_Level(void)
{
    return Bat_Level;
}

/*******************
PVD_Open
PVD_Close
Get_Bat_Level
*******************/
