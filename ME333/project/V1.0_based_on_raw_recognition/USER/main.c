#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "sdram.h"
#include "remote.h"
/************************************************
 ALIENTEK 阿波罗STM32F7开发板 实验32
 红外遥控器实验-HAL库函数版
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司
 作者：正点原子 @ALIENTEK
************************************************/
int time_tick = 0;
u8 isTargetFound = 0;
int error_num = 0;
int control_flag = 1;
float yaw_angle;

enum state_type
{
	UP = 1,
	RIGHT,
	LEFT,
	DOWN,
	GYRO_L,
	GYRO_R,
	NONE,
};
enum state_type state = NONE;

void Remote_Ctrl(void)
{
	if (time_tick % 1 == 0)
	{
		if (state == UP)
		{
			if (control_flag == 1)
			{
				control_flag = 2;
				Motor_Rotate(1, 2380, 1000);
				printf("114,514\r\n");
			}
			else if (control_flag == 2)
			{
				control_flag = 3;
				Motor_Rotate(2, 830, 1000);
			}
			else if (control_flag == 3)
			{
				control_flag = 4;
				Motor_Rotate(3, 830, 1000);
			}
			else if (control_flag == 4)
			{
				control_flag = 1;
				Motor_Rotate(4, 2350, 1000);
			}
		}
		else if (state == NONE)
		{
			if (control_flag == 1)
			{
				control_flag = 2;
				Motor_Rotate(1, 1500, 1000);
				Motor_Rotate(2, 1500, 1000);
				Motor_Rotate(3, 1500, 1000);
				Motor_Rotate(4, 1500, 1000);
			}
			else if (control_flag == 2)
			{
				control_flag = 3;
			}
			else if (control_flag == 3)
			{
				control_flag = 4;
			}
			else if (control_flag == 4)
			{
				control_flag = 1;
			}
		}
		else if (state == DOWN)
		{
			if (control_flag == 1)
			{
				control_flag = 2;
				Motor_Rotate(1, 800, 5000);
			}
			else if (control_flag == 2)
			{
				control_flag = 3;
				Motor_Rotate(2, 2350, 5000);
			}
			else if (control_flag == 3)
			{
				control_flag = 4;
				Motor_Rotate(3, 2350, 5000);
			}
			else if (control_flag == 4)
			{
				control_flag = 1;
				Motor_Rotate(4, 800, 5000);
			}
		}
		else if (state == LEFT)
		{
			if (control_flag == 1)
			{
				control_flag = 2;
				Motor_Rotate(1, 900, 5000);
			}
			else if (control_flag == 2)
			{
				control_flag = 3;
				Motor_Rotate(2, 910, 5000);
			}
			else if (control_flag == 3)
			{
				control_flag = 4;
				Motor_Rotate(3, 870, 5000);
			}
			else if (control_flag == 4)
			{
				control_flag = 1;
				Motor_Rotate(4, 850, 5000);
			}
		}
		else if (state == RIGHT)
		{
			if (control_flag == 1)
			{
				control_flag = 2;
				Motor_Rotate(1, 2000, 5000);
			}
			else if (control_flag == 2)
			{
				control_flag = 3;
				Motor_Rotate(2, 1950, 5000);
			}
			else if (control_flag == 3)
			{
				control_flag = 4;
				Motor_Rotate(3, 1980, 5000);
			}
			else if (control_flag == 4)
			{
				control_flag = 1;
				Motor_Rotate(4, 20400, 5000);
			}
		}
		else if (state == GYRO_L)
		{
			if (control_flag == 1)
			{
				control_flag = 2;
				Motor_Rotate(1, 1700, 5000);
			}
			else if (control_flag == 2)
			{
				control_flag = 3;
				Motor_Rotate(2, 1700, 5000);
			}
			else if (control_flag == 3)
			{
				control_flag = 4;
				Motor_Rotate(3, 1300, 5000);
			}
			else if (control_flag == 4)
			{
				control_flag = 1;
				Motor_Rotate(4, 1300, 5000);
			}
		}
		else if (state == GYRO_R)
		{
			if (control_flag == 1)
			{
				control_flag = 2;
				Motor_Rotate(1, 1300, 5000);
			}
			else if (control_flag == 2)
			{
				control_flag = 3;
				Motor_Rotate(2, 1300, 5000);
			}
			else if (control_flag == 3)
			{
				control_flag = 4;
				Motor_Rotate(3, 1700, 5000);
			}
			else if (control_flag == 4)
			{
				control_flag = 1;
				Motor_Rotate(4, 1700, 5000);
			}
		}
		time_tick = 0;
	}
} /* Remote_Ctrl */

int main(void)
{
	u8 t = 0;
	u8 *str = 0;
	u8 len;

	uint8_t dirty_flag = 1;
	uint8_t timess = 0;
	uint8_t key = 0;

	Cache_Enable();					 // 打开L1-Cache
	HAL_Init();						 // 初始化HAL库
	Stm32_Clock_Init(432, 25, 2, 9); // 设置时钟,216Mhz
	delay_init(216);				 // 延时初始化
	MX_GPIO_Init();
	uart_init(115200); // 串口初始化
	LED_Init();		   // 初始化LED
	KEY_Init();		   // 初始化按键
	SDRAM_Init();	   // 初始化SDRAM
	// LCD_Init();						 // 初始化LCD
	Remote_Init(); // 初始化	红外接收
	POINT_COLOR = RED;
	// LCD_ShowString(30, 50, 200, 16, 16, "Apollo STM32F4/F7");
	// LCD_ShowString(30, 70, 200, 16, 16, "REMOTE TEST");
	// LCD_ShowString(30, 90, 200, 16, 16, "ATOM@ALIENTEK");
	// LCD_ShowString(30, 110, 200, 16, 16, "2016/7/12");
	// LCD_ShowString(30, 130, 200, 16, 16, "KEYVAL:");
	// LCD_ShowString(30, 150, 200, 16, 16, "KEYCNT:");
	// LCD_ShowString(30, 170, 200, 16, 16, "SYMBOL:");
	POINT_COLOR = BLUE; // 设置字体为蓝色
	while (1)
	{
		key = Remote_Scan();
		
		time_tick++;
		Remote_Ctrl();
		delay_ms(20);
		if ((UART2_Handler.RxState == HAL_UART_STATE_READY) && ((USART_RX_STA & 0x8000) == 0))
		{
			HAL_UART_Receive_IT(&UART2_Handler, (u8 *)aRxBuffer, RXBUFFERSIZE);
		}

		if (USART_RX_STA & 0x8000)
		{
			isTargetFound = 1;
			error_num = 0;
			len = USART_RX_STA & 0x3fff; // 得到此次接收到的数据长度
			USART_RX_STA = 0;
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); // LED1对应引脚PB0拉高，灭，等同于LED1(1)
			USART_RX_BUF[len] = '\0';							// 添加字符串结束符
			yaw_angle = atof((const char *)USART_RX_BUF);		// 转为浮点数
			if (yaw_angle >= 5.0f)
			{
				state = RIGHT;
			}
			else if (yaw_angle <= -5.0f)
			{
				state = LEFT;
			}
			else if (yaw_angle > -5.0f && yaw_angle < 5.0f)
			{
				state = UP;
			}
		}
		else
		{
			error_num++;
			if (error_num > 25)
				state = NONE;
			if (error_num > 100)
			{
				isTargetFound = 0;
				state = GYRO_L;
			}
		}
	}
}

void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

	/* USER CODE END Error_Handler_Debug */
}
