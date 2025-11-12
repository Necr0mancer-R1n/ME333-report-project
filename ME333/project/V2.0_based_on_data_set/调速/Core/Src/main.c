#include "main.h"
#include "usart.h"
#include "gpio.h"
#include "delay.h"
#include "sys.h"
#include <stdlib.h>
#include <string.h>

#define RXBUFFERSIZE 64

uint8_t aRxBuffer[RXBUFFERSIZE];     // 串口临时接收缓冲
uint8_t USART_RX_BUF[RXBUFFERSIZE];  // 完整接收缓存
uint16_t USART_RX_STA = 0;           // 状态变量

enum state_type {
    SEARCH = 0, RIGHT, LEFT, FORWARD, STOP
};
enum state_type state = SEARCH;

int error_num = 0;
float yaw_angle = 0;
uint8_t k230_active = 0;

void SystemClock_Config(void);
void Cache_Enable(void);

// 重定向 printf 到串口1
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 10);
    return ch;
}

int main(void) {
    Cache_Enable();
    Stm32_Clock_Init(432,25,2,9);  // 216MHz
    delay_init(216);
    HAL_Init();

    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();

    HAL_UART_Receive_IT(&huart2, aRxBuffer, RXBUFFERSIZE);  // 启动串口2接收中断

    while (1) {
        // ===== 接收 K230 发送的 Yaw 数据 =====
        if ((huart2.RxState == HAL_UART_STATE_READY) && ((USART_RX_STA & 0x8000) == 0)) {
            HAL_UART_Receive_IT(&huart2, aRxBuffer, RXBUFFERSIZE);
        }

        if (USART_RX_STA & 0x8000) {
            // 成功接收一帧数据
            error_num = 0;
            k230_active = 1;

            uint8_t len = USART_RX_STA & 0x3FFF;
            USART_RX_STA = 0;
            memcpy(USART_RX_BUF, aRxBuffer, len);
            USART_RX_BUF[len] = '\0';

            yaw_angle = atof((const char *)USART_RX_BUF);

            if (yaw_angle >= 5.0f) {
                state = RIGHT;
            } else if (yaw_angle <= -5.0f) {
                state = LEFT;
            } else {
                state = FORWARD;
            }
        } else {
            error_num++;
            if (error_num > 100) {
                k230_active = 0;
                state = SEARCH;
            }
        }

        // ===== 控制电机行为 =====
        switch (state) {
            case SEARCH:
                // 原地逆时针旋转（类似遥控器VOL+）
                Motor_Rotate(1, 2000, 50);
                Motor_Rotate(2, 2000, 50);
                Motor_Rotate(3, 1000, 50);
                Motor_Rotate(4, 1000, 50);
                break;

            case RIGHT:
                // 向右平移
                Motor_Rotate(1, 2000, 40);
                Motor_Rotate(2, 1950, 40);
                Motor_Rotate(3, 1980, 40);
                Motor_Rotate(4, 2040, 40);
                break;

            case LEFT:
                // 向左平移
                Motor_Rotate(1, 900, 40);
                Motor_Rotate(2, 910, 40);
                Motor_Rotate(3, 870, 40);
                Motor_Rotate(4, 850, 40);
                break;

            case FORWARD:
                // 向前直行
                Motor_Rotate(1, 2380, 50);
                Motor_Rotate(2, 830, 50);
                Motor_Rotate(3, 830, 50);
                Motor_Rotate(4, 2350, 50);
                break;

            case STOP:
                // 停止所有电机
                Motor_Rotate(1, 0, 0);
                Motor_Rotate(2, 0, 0);
                Motor_Rotate(3, 0, 0);
                Motor_Rotate(4, 0, 0);
                break;
        }

        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);  // LED闪烁观察运行状态
        HAL_Delay(4);
    }
}


void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE(); 
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2;
  PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInitStruct.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
