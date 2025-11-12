#include "usart.h"
#include "stdio.h"	//fputc函数
#include "string.h" //包含字符串处理函数,strcat()用于拼接两个字符串，以及strlen()用于统计字符串的长度
#include "main.h"
//////////////////////////////////////////////////////////////////////////////////
// 如果使用os,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h" //os 使用
#endif
//////////////////////////////////////////////////////////////////////////////////
// 本程序只供学习使用，未经作者许可，不得用于其它任何用途
// ALIENTEK STM32F7开发板
// 串口1初始化
// 正点原子@ALIENTEK
// 技术论坛:www.openedv.com
// 修改日期:2015/6/23
// 版本：V1.5
// 版权所有，盗版必究。
// Copyright(C) 广州市星翼电子科技有限公司 2009-2019
// All rights reserved
//********************************************************************************
// V1.0修改说明
//////////////////////////////////////////////////////////////////////////////////
// 加入以下代码,支持printf函数,而不需要选择use MicroLIB
// #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#if 1
#pragma import(__use_no_semihosting)
// 标准库需要的支持函数
struct __FILE
{
	int handle;
};

FILE __stdout;
// 定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
	x = x;
}
// 重定义fputc函数
int fputc(int ch, FILE *f)
{
	while ((USART2->ISR & 0X40) == 0)
		; // 循环发送,直到发送完毕
	USART2->TDR = (u8)ch;
	return ch;
}
// int fputc(int ch, FILE *f)
// {
// 	uint8_t temp = ch;
// 	HAL_UART_Transmit(&UART2_Handler, &temp, 1, 1000);
// 	while (__HAL_UART_GET_FLAG(&UART1_Handler, UART_FLAG_TXE) != SET)
// 		;
// 	return ch;
// } // 重定义fputc函数
#endif

// #if EN_USART1_RX // 如果使能了接收
//  串口1中断服务程序
//  注意,读取USARTx->SR能避免莫名其妙的错误
u8 USART_RX_BUF[USART_REC_LEN]; // 接收缓冲,最大USART_REC_LEN个字节.
// 接收状态
// bit15，	接收完成标志
// bit14，	接收到0x0d
// bit13~0，	接收到的有效字节数目
u16 USART_RX_STA = 0; // 接收状态标记

u8 aRxBuffer[RXBUFFERSIZE];		  // HAL库使用的串口接收缓冲
UART_HandleTypeDef UART1_Handler; // UART句柄
UART_HandleTypeDef UART2_Handler; // UART句柄
UART_HandleTypeDef UART3_Handler; // UART句柄

void UART2_Send(uint8_t *Data, uint16_t Size)
{
	uint16_t i = 0;
	for (i = 0; i < Size; i++)
	{
		HAL_UART_Transmit(&UART2_Handler, &Data[i], 1, 1000);
		while (__HAL_UART_GET_FLAG(&UART2_Handler, UART_FLAG_TXE) != SET)
			;
	}
}

void UART3_Send(uint8_t *Data, uint16_t Size)
{
	uint16_t i = 0;
	for (i = 0; i < Size; i++)
	{
		HAL_UART_Transmit(&UART3_Handler, &Data[i], 1, 1000);
		while (__HAL_UART_GET_FLAG(&UART3_Handler, UART_FLAG_TXE) != SET)
			;
	}
}

// 串口1，用于发送调试信息至PC；
// 重定义 fputc 函数，利用HAL库函数发送。
// int fputc(int ch, FILE *f)
// {

// 	uint8_t temp = ch;
// 	HAL_UART_Transmit(&huart1, &temp, 1, 1000);
// 	while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TXE) != SET)
// 		;
// 	return ch;
// }
// 初始化IO 串口1
// bound:波特率
void uart_init(u32 bound)
{
	// UART 初始化设置
	UART1_Handler.Instance = USART1;
	UART1_Handler.Init.BaudRate = 9600;
	UART1_Handler.Init.WordLength = UART_WORDLENGTH_8B;
	UART1_Handler.Init.StopBits = UART_STOPBITS_1;
	UART1_Handler.Init.Parity = UART_PARITY_NONE;
	UART1_Handler.Init.Mode = UART_MODE_TX_RX;
	UART1_Handler.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	UART1_Handler.Init.OverSampling = UART_OVERSAMPLING_16;
	UART1_Handler.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	UART1_Handler.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	HAL_UART_Init(&UART1_Handler); // HAL_UART_Init()会使能UART1
	// HAL_UART_Receive_IT(&UART1_Handler, (u8 *)aRxBuffer, RXBUFFERSIZE); // 该函数会开启接收中断：标志位UART_IT_RXNE，并且设置接收缓冲以及接收缓冲接收最大数据量

	if (HAL_UART_Init(&UART1_Handler) != HAL_OK)
	{
		Error_Handler();
	}

	UART2_Handler.Instance = USART2;
	UART2_Handler.Init.BaudRate = 115200;
	UART2_Handler.Init.WordLength = UART_WORDLENGTH_8B;
	UART2_Handler.Init.StopBits = UART_STOPBITS_1;
	UART2_Handler.Init.Parity = UART_PARITY_NONE;
	UART2_Handler.Init.Mode = UART_MODE_TX_RX;
	UART2_Handler.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	UART2_Handler.Init.OverSampling = UART_OVERSAMPLING_16;
	UART2_Handler.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	UART2_Handler.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	HAL_UART_Init(&UART2_Handler); // 必须有这句！
	HAL_UART_Receive_IT(&UART2_Handler, (u8 *)aRxBuffer, RXBUFFERSIZE);
	// if (HAL_UART_Init(&UART2_Handler) != HAL_OK)
	// {
	// 	Error_Handler();
	// }
	// HAL_UART_Receive_IT(&UART2_Handler, (u8 *)aRxBuffer, RXBUFFERSIZE);

	UART3_Handler.Instance = USART3;
	UART3_Handler.Init.BaudRate = 9600;
	UART3_Handler.Init.WordLength = UART_WORDLENGTH_8B;
	UART3_Handler.Init.StopBits = UART_STOPBITS_1;
	UART3_Handler.Init.Parity = UART_PARITY_NONE;
	UART3_Handler.Init.Mode = UART_MODE_TX_RX;
	UART3_Handler.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	UART3_Handler.Init.OverSampling = UART_OVERSAMPLING_16;
	UART3_Handler.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	UART3_Handler.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	// HAL_UART_Init(&UART3_Handler);
	// HAL_UART_Receive_IT(&UART3_Handler, (u8 *)aRxBuffer, RXBUFFERSIZE); // 该函数会开启接收中断：标志位UART_IT_RXNE，并且设置接收缓冲以及接收缓冲接收最大数据量

	// if (HAL_UART_Init(&UART3_Handler) != HAL_OK)
	// {
	// 	Error_Handler();
	// }
}

// UART底层初始化，时钟使能，引脚配置，中断配置
// 此函数会被HAL_UART_Init()调用
// huart:串口句柄

void USART2_IRQHandler(void)
{
	HAL_UART_IRQHandler(&UART2_Handler);
}
void USART1_IRQHandler(void)
{
	HAL_UART_IRQHandler(&UART1_Handler);
}
void USART3_IRQHandler(void)
{
	HAL_UART_IRQHandler(&UART3_Handler);
}
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if (huart->Instance == USART1)
	{
		/* USER CODE BEGIN USART1_MspInit 0 */

		/* USER CODE END USART1_MspInit 0 */
		/* USART1 clock enable */
		__HAL_RCC_USART1_CLK_ENABLE();

		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**USART1 GPIO Configuration
		PA9     ------> USART1_TX
		PA10     ------> USART1_RX
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* USER CODE BEGIN USART1_MspInit 1 */

		/* USER CODE END USART1_MspInit 1 */
	}
	else if (huart->Instance == USART2)
	{
		/* USER CODE BEGIN USART2_MspInit 0 */

		/* USER CODE END USART2_MspInit 0 */
		/* USART2 clock enable */
		__HAL_RCC_USART2_CLK_ENABLE();

		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**USART2 GPIO Configuration
		PA2     ------> USART2_TX
		PA3     ------> USART2_RX
		*/

		GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* USER CODE BEGIN USART2_MspInit 1 */
		HAL_NVIC_SetPriority(USART2_IRQn, 1, 0);
		HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE END USART2_MspInit 1 */}
  else if (huart->Instance == USART3)
  {
	  /* USER CODE BEGIN USART3_MspInit 0 */

	  /* USER CODE END USART3_MspInit 0 */
	  /* USART3 clock enable */
	  __HAL_RCC_USART3_CLK_ENABLE();

	  __HAL_RCC_GPIOB_CLK_ENABLE();
	  /**USART3 GPIO Configuration
	  PB10     ------> USART3_TX
	  PB11     ------> USART3_RX
	  */
	  GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
	  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	  GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
	  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	  /* USER CODE BEGIN USART3_MspInit 1 */
	  HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
	  HAL_NVIC_EnableIRQ(USART3_IRQn);
	  /* USER CODE END USART3_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle)
{

	if (uartHandle->Instance == USART1)
	{
		/* USER CODE BEGIN USART1_MspDeInit 0 */

		/* USER CODE END USART1_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_USART1_CLK_DISABLE();

		/**USART1 GPIO Configuration
		PA9     ------> USART1_TX
		PA10     ------> USART1_RX
		*/
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);

		/* USER CODE BEGIN USART1_MspDeInit 1 */

		/* USER CODE END USART1_MspDeInit 1 */
	}
	else if (uartHandle->Instance == USART2)
	{
		/* USER CODE BEGIN USART2_MspDeInit 0 */

		/* USER CODE END USART2_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_USART2_CLK_DISABLE();

		/**USART2 GPIO Configuration
		PA2     ------> USART2_TX
		PA3     ------> USART2_RX
		*/
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2 | GPIO_PIN_3);

		/* USER CODE BEGIN USART2_MspDeInit 1 */

		/* USER CODE END USART2_MspDeInit 1 */
	}
	else if (uartHandle->Instance == USART3)
	{
		/* USER CODE BEGIN USART3_MspDeInit 0 */

		/* USER CODE END USART3_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_USART3_CLK_DISABLE();

		/**USART3 GPIO Configuration
		PB10     ------> USART3_TX
		PB11     ------> USART3_RX
		*/
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10 | GPIO_PIN_11);

		/* USER CODE BEGIN USART3_MspDeInit 1 */

		/* USER CODE END USART3_MspDeInit 1 */
	}
}

/* USER CODE BEGIN 1 */

// 函数：电机速度控制，至于速度和PWM脉宽对应关系，请自行调试或标定。
//			input：电机ID号(1-999)，请输入正常的阿拉伯数字，函数内部会自动补'0'零字符。
//			input：PWM脉宽，500-2500，请输入正常的阿拉伯数字，函数内部会自动补'0'零字符。
//			input：转动时间，1-9999，请输入正常的阿拉伯数字，函数内部会自动补'0'零字符。
//			output: NULL

void Motor_Rotate(int id, int pwm, int time)
{
	char ID_str[10];
	char PWM_str[10];
	char TIME_str[10];

	char data[50] = {"#"}; // data数组，用于发送控制指令,按照协议，开头是"#"

	// 将整形数的ID号转换为字符串
	myitoa(id, ID_str, 10);

	// 在ID号的字符串的前方补零，如果有必要的话
	if ((3 - strlen(ID_str)) == 2)
	{
		strcat(data, "00");
	}
	else if ((3 - strlen(ID_str)) == 1)
	{
		strcat(data, "0");
	}
	// 字符串拼接
	strcat(data, ID_str);
	// 将整形数的PWM脉宽值转换为字符串
	myitoa(pwm, PWM_str, 10);

	// 在PWM脉宽的字符串的前方补零，如果有必要的话
	if ((4 - strlen(PWM_str)) == 1)
	{
		strcat(data, "P0");
	}
	else if ((4 - strlen(PWM_str)) == 0)
	{
		strcat(data, "P");
	}
	// 字符串拼接
	strcat(data, PWM_str);
	// 将整形数的转动时间转换为字符串
	myitoa(time, TIME_str, 10);

	// 在转动时间的字符串的前方补零，如果有必要的话
	if ((4 - strlen(TIME_str)) == 2)
	{
		strcat(data, "T00");
	}
	else if ((4 - strlen(TIME_str)) == 1)
	{
		strcat(data, "T0");
	}
	else if ((4 - strlen(TIME_str)) == 0)
	{
		strcat(data, "T");
	}

	// 字符串拼接
	strcat(data, TIME_str);
	strcat(data, "!\r\n"); // 末端添加"\r\n"，使得指令输出到PC端时，每一条指令仅占1行，单纯为了方便观察和调试。

	UART2_Send((uint8_t *)data, strlen(data));

	printf("%s", data);

	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_2);
}

// 函数功能：整形数转字符串
// stm32不支持标准的itoa,这里自己实现
// 如果对指针很熟悉，可以自己研读下这一段代码。

char *myitoa(int value, char *string, int radix)
{
	int i, d;
	int flag = 0;
	char *ptr = string;

	/* This implementation only works for decimal numbers. */
	if (radix != 10)
	{
		*ptr = 0;
		return string;
	}

	if (!value)
	{
		*ptr++ = 0x30;
		*ptr = 0;
		return string;
	}

	/* if this is a negative value insert the minus sign. */
	if (value < 0)
	{
		*ptr++ = '-';

		/* Make the value positive. */
		value *= -1;
	}

	for (i = 10000; i > 0; i /= 10)
	{
		d = value / i;

		if (d || flag)
		{
			*ptr++ = (char)(d + 0x30);
			value -= (d * i);
			flag = 1;
		}
	}

	/* Null terminate the string. */
	*ptr = 0;

	return string;

} /* NCL_Itoa */

void MX_GPIO_Init(void)
{

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_RESET);

	/*Configure GPIO pins : PB0 PB1 */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/****************************************************************************************/
/****************************************************************************************/
/*************************下面程序通过在回调函数中编写中断控制逻辑*********************/
/****************************************************************************************
***************************************************************************************************/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2)
	{
		if ((USART_RX_STA & 0x8000) == 0)
		{
			if (USART_RX_STA & 0x4000)
			{
				if (aRxBuffer[0] == 0x0a)
					USART_RX_STA |= 0x8000; // 收到\r后收到\n，帧结束
				else
					USART_RX_STA = 0; // 不是\n，清零
			}
			else
			{
				if (aRxBuffer[0] == 0x0d)
					USART_RX_STA |= 0x4000; // 收到\r，等待\n
				else
				{
					if (USART_RX_STA > (USART_REC_LEN - 1))
						USART_RX_STA = 0;
					else
					{
						USART_RX_BUF[USART_RX_STA & 0X3FFF] = aRxBuffer[0];
						USART_RX_STA++;
					}
				}
			}
		}
		HAL_UART_Receive_IT(&UART2_Handler, (u8 *)aRxBuffer, RXBUFFERSIZE);
	}
}
// 串口1中断服务程序
//  void USART1_IRQHandler(void)
//  {
//  	u32 timeout=0;
//  	u32 maxDelay=0x1FFFF;
//  #if SYSTEM_SUPPORT_OS	 	//使用OS
//  	OSIntEnter();
//  #endif

// 	HAL_UART_IRQHandler(&UART1_Handler);	//调用HAL库中断处理公用函数

// 	timeout=0;
// 	while (HAL_UART_GetState(&UART1_Handler)!=HAL_UART_STATE_READY)//等待就绪
// 	{
// 		timeout++;////超时处理
// 		if(timeout>maxDelay) break;
// 	}

// 	timeout=0;
// 	while(HAL_UART_Receive_IT(&UART1_Handler,(u8 *)aRxBuffer, RXBUFFERSIZE)!=HAL_OK)//一次处理完成之后，重新开启中断并设置RxXferCount为1
// 	{
// 		timeout++; //超时处理
// 		if(timeout>maxDelay) break;
// 	}
// #if SYSTEM_SUPPORT_OS	 	//使用OS
// 	OSIntExit();
// #endif
// }
