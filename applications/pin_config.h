/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-21     Tobby       the first version
 */
#ifndef APPLICATIONS_PIN_CONFIG_H_
#define APPLICATIONS_PIN_CONFIG_H_

/*
 * RF
 */
#define RF_SW1_PIN      6  //PA6
#define RF_SW2_PIN      7  //PA7
#define TCXO_PWR_PIN    16  //PB0

//BASIC
#define KEY1_OFF        3   //PA3
#define KEY2_ON         2   //PA2

#define ON_POS          4   //PA4
#define OFF_POS         5   //PA5

#define ANT_EXT         8   //PA8
#define ANT_INT         9   //PA9

#define HAND_SWITCH     1   //PA1
#define LED_DATAT       0   //PA0

#define BUZZER          24   //PB8
#define DC_DET          45   //PC13

#define MOT_ADC         12   //PA12
#define MOT_IN1         11   //PA11
#define MOT_IN2         10   //PA10

#define Peak_Loss      28   //PB12 DISCONNECT
#define Peak_ON    18   //PB2 WATER_SINGAL



#endif /* APPLICATIONS_PIN_CONFIG_H_ */
