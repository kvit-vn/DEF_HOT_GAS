/*При появлении сигнала оттайки (низкий уровень на входе) выключаем компрессор,
 *ожидаем 3 минуты, включаем клапан оттайки(реверс), опять ожидаем 3 мин.
 *При выключении оттайки все в обратном порядке
 */
#include "stm8s.h"
#include "stm8s_gpio.h"
#include "stm8s_clk.h"
#include "stm8s_tim4.h"

#define LED_PORT GPIOB
#define LED GPIO_PIN_5


#define IN_PORT		GPIOA //Порт входных сигналов компрессора и оттайки
#define OUT_PORT	GPIOC //Порт выходных сигналов компрессора и оттайки
#define COMP_IN		GPIO_PIN_1 //Входной пин компрессора
#define DEF_IN		GPIO_PIN_2 //Входной пин оттайки
#define COMP_OUT	GPIO_PIN_4 //Выход на компрессор
#define DEF_OUT		GPIO_PIN_3 //Выход на оттайку
#define ON	1
#define OFF	0

#define TIM4_PERIOD       124


volatile uint16_t count;

INTERRUPT_HANDLER(IRQ_Handler_TIM4, 23)
{
    if (count)
        count--;

    TIM4_ClearITPendingBit(TIM4_IT_UPDATE);
}

void delay_ms(uint16_t ms)
{
        TIM4_Cmd(DISABLE);       // stop
        TIM4_TimeBaseInit(TIM4_PRESCALER_128, TIM4_PERIOD);
        TIM4_ClearFlag(TIM4_FLAG_UPDATE);
        TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);

        count = ms;

        TIM4_Cmd(ENABLE);       // let's go

    while(count);
}


void delay_sec(uint16_t time)
{
    while (time)
    {
        delay_ms(1000);
        GPIO_WriteReverse(LED_PORT, LED);
        time--;
    }
    GPIO_WriteHigh(LED_PORT, LED);//Выключаем светодиод
}

ErrorStatus status = FALSE;

int main( void )
{
    	unsigned char DefPrevState=OFF,DefCurState=OFF;
	// ----------- GPIO CONFIG -------------------
	GPIO_DeInit(LED_PORT);
	GPIO_Init(LED_PORT, LED, GPIO_MODE_OUT_PP_LOW_FAST);
	GPIO_DeInit(IN_PORT);
	GPIO_Init(IN_PORT,COMP_IN,GPIO_MODE_IN_FL_NO_IT);	
	GPIO_Init(IN_PORT,DEF_IN,GPIO_MODE_IN_FL_NO_IT);	
	GPIO_DeInit(OUT_PORT);
	GPIO_Init(OUT_PORT, COMP_OUT, GPIO_MODE_OUT_PP_LOW_FAST);
	GPIO_Init(OUT_PORT, DEF_OUT, GPIO_MODE_OUT_PP_LOW_FAST);
    // ---------- CLK CONFIG -----------------------
	CLK_DeInit();

	CLK_SYSCLKConfig(CLK_PRESCALER_CPUDIV1);
	CLK_SYSCLKConfig(CLK_PRESCALER_HSIDIV1); // set 16 MHz for CPU

    // uncomment if use HSE on Quartz
    //status = CLK_ClockSwitchConfig(CLK_SWITCHMODE_AUTO, CLK_SOURCE_HSE, DISABLE,
    //                CLK_CURRENTCLOCKSTATE_DISABLE);

    // ---------- TIM4 CONFIG -----------------------
    TIM4_DeInit();

    enableInterrupts();

    GPIO_WriteLow(OUT_PORT, COMP_OUT); //Выключаем компрессор
    GPIO_WriteLow(OUT_PORT,DEF_OUT);// Выключаем клапан оттайки
    delay_sec(180);//Выжидаем 3 мин
    for(;;)
    {

	 if(GPIO_ReadInputPin(IN_PORT,DEF_IN)) DefCurState=OFF; else DefCurState=ON;

	 if(DefPrevState == DefCurState)
	 {
		if(GPIO_ReadInputPin(IN_PORT,COMP_IN)){
			GPIO_WriteLow(OUT_PORT,COMP_OUT);
		}else{
			GPIO_WriteHigh(OUT_PORT,COMP_OUT);
		}	
	 }else{
		if(DefCurState==ON)
		{
		
			GPIO_WriteLow(OUT_PORT, COMP_OUT); //Выключаем компрессор
			delay_sec(180);//Выжидаем 3 мин
			GPIO_WriteHigh(OUT_PORT,DEF_OUT);// Включаем клапан оттайки
			delay_sec(180);//Выжидаем 3 мин
			if(!GPIO_ReadInputPin(IN_PORT,COMP_IN)) GPIO_WriteHigh(OUT_PORT, COMP_OUT);//Включаем компрессор, если он включенконтроллером	
			DefPrevState=DefCurState;	
		}else{

			GPIO_WriteLow(OUT_PORT, COMP_OUT); //Выключаем компрессор
			delay_sec(180);//Выжидаем 3 мин
			GPIO_WriteLow(OUT_PORT,DEF_OUT);// Выключаем клапан оттайки
			delay_sec(180);//Выжидаем 3 мин
			if(!GPIO_ReadInputPin(IN_PORT,COMP_IN)) GPIO_WriteHigh(OUT_PORT, COMP_OUT);//Включаем компрессор, если он включенконтроллером	
			DefPrevState=DefCurState;	

		}

	 }
   	 
	 delay_ms(100);
        
    }
}

#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif
