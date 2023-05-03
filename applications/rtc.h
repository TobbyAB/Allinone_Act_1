/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-25     Tobby       the first version
 */
#ifndef APPLICATIONS_RTC_H_
#define APPLICATIONS_RTC_H_

void RTC_Init(void);
void RTC_Clear(void);
uint8_t Detect_RTC_Wakeup(void);
void RTC_Check(void);
void LowPowerTimerStart(void);

#endif /* APPLICATIONS_RTC_H_ */
