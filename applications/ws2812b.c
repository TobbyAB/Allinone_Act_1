/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-24     Tobby       the first version
 */
#include "ws2812b.h"
#include "rtthread.h"
#include "rtdevice.h"
#include "board.h"


#define DBG_TAG "ws2818b"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

rt_timer_t ws2812b_timer = RT_NULL;

///*Some Static Colors------------------------------*/
const RGB_Color_TypeDef RED      = {255,0,0};
const RGB_Color_TypeDef GREEN    = {0,255,0};
const RGB_Color_TypeDef BLUE     = {0,0,255};
const RGB_Color_TypeDef BLACK    = {0,0,0};

/*二维数组存放最终PWM输出数组，每一行24个
数据代表一个LED，最后一行24个0代表RESET码*/

uint32_t Pixel_Buf[Pixel_NUM + 1][24] = {0};
TIM_HandleTypeDef tim2_handle;
DMA_HandleTypeDef dma_tim2_handle;

void DMA1_Channel2_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel2_IRQn 0 */

  /* USER CODE END DMA1_Channel2_IRQn 0 */
  HAL_DMA_IRQHandler(&dma_tim2_handle);
  /* USER CODE BEGIN DMA1_Channel2_IRQn 1 */

  /* USER CODE END DMA1_Channel2_IRQn 1 */
}

void TIM2_IRQHandler(void)
{
  /* USER CODE BEGIN TIM2_IRQn 0 */

  /* USER CODE END TIM2_IRQn 0 */
  HAL_TIM_IRQHandler(&tim2_handle);
  /* USER CODE BEGIN TIM2_IRQn 1 */

  /* USER CODE END TIM2_IRQn 1 */
}

static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  tim2_handle.Instance = TIM2;
  tim2_handle.Init.Prescaler = 0;
  tim2_handle.Init.CounterMode = TIM_COUNTERMODE_UP;
  tim2_handle.Init.Period = 59;
  tim2_handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  tim2_handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&tim2_handle) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&tim2_handle, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&tim2_handle, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&tim2_handle);

}

void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);
//  /* DMA1_Channel2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
  HAL_NVIC_DisableIRQ(DMA1_Channel2_IRQn);

}

void ws2812b_init(void)
{
    MX_TIM2_Init();
    RGB_SetColor(0,BLACK);
    RGB_SetColor(1,BLACK);
    RGB_SetColor(2,BLACK);
    //HAL_TIM_PWM_Start_DMA(&tim2_handle, TIM_CHANNEL_1, (uint32_t *)Pixel_Buf,(Pixel_NUM + 1)*24);
}
MSH_CMD_EXPORT(ws2812b_init,ws2812b_init);


void ws2812b_green(uint8_t id,uint8_t value)
{
    if(value)
    {
        for(uint8_t i=0;i<8;i++) Pixel_Buf[id][i]   = ( (30 & (1 << (7 -i)))? (CODE_1):CODE_0 );//数组某一行0~7转化存放G
    }
    else
    {
        for(uint8_t i=0;i<8;i++) Pixel_Buf[id][i]   = ( (0 & (1 << (7 -i)))? (CODE_1):CODE_0 );//数组某一行0~7转化存放G
    }
}

void ws2812b_red(uint8_t id,uint8_t value)
{
    if(value)
    {
        for(uint8_t i=8;i<16;i++) Pixel_Buf[id][i]   = ( (30 & (1 << (15 -i)))? (CODE_1):CODE_0 );//数组某一行8-15转化存放R
    }
    else
    {
        for(uint8_t i=8;i<16;i++) Pixel_Buf[id][i]   = ( (0 & (1 << (15 -i)))? (CODE_1):CODE_0 );//数组某一行8-15转化存放R
    }
}

void ws2812b_blue(uint8_t id,uint8_t value)
{
    if(value)
    {
        for(uint8_t i=16;i<24;i++) Pixel_Buf[id][i]   = ( (30 & (1 << (23 -i)))? (CODE_1):CODE_0 );//数组某一行8-15转化存放R
    }
    else
    {
        for(uint8_t i=16;i<24;i++) Pixel_Buf[id][i]   = ( (0 & (1 << (23 -i)))? (CODE_1):CODE_0 );//数组某一行8-15转化存放R
    }
}
/*
功能：设定单个RGB LED的颜色，把结构体中RGB的24BIT转换为0码和1码
参数：LedId为LED序号，Color：定义的颜色结构体
*/
void RGB_SetColor(uint8_t LedId,RGB_Color_TypeDef Color)
{
    uint8_t i;

    for(i=0;i<8;i++) Pixel_Buf[LedId][i]   = ( (Color.G & (1 << (7 -i)))? (CODE_1):CODE_0 );//数组某一行0~7转化存放G
    for(i=8;i<16;i++) Pixel_Buf[LedId][i]  = ( (Color.R & (1 << (15-i)))? (CODE_1):CODE_0 );//数组某一行8~15转化存放R
    for(i=16;i<24;i++) Pixel_Buf[LedId][i] = ( (Color.B & (1 << (23-i)))? (CODE_1):CODE_0 );//数组某一行16~23转化存放B
}

void custom_delay_us(uint32_t us)
{
    uint32_t i, j;
    for (i = 0; i < us; i++)
    {
        for (j = 0; j < 7; j++) // 此值可能需要根据你的系统时钟进行调整
        {
            __NOP(); // 一个空指令，用于占用一个时钟周期
        }
    }
}


/*
功能：发送数组
参数：(&htim1)定时器1，(TIM_CHANNEL_1)通道1，((uint32_t *)Pixel_Buf)待发送数组，
            (Pixel_NUM+1)*24)发送个数，数组行列相乘
*/
void RGB_SendArray(void)
{
    HAL_TIM_PWM_Stop_DMA(&tim2_handle, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start_DMA(&tim2_handle, TIM_CHANNEL_1, (uint32_t *)Pixel_Buf,(Pixel_NUM + 1)*24);
}


