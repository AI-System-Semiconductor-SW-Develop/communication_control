#include "device_driver.h"

#define TIM2_TICK         	(20) 					// usec
#define TIM2_FREQ 	  		(1000000./TIM2_TICK)	// Hz
#define TIME2_PLS_OF_1ms  	(1000./TIM2_TICK)
#define TIM2_MAX	  		(0xffffffffu)
#define set_1tick (double)(1.0/PCLK2)
#define set_pulse (1.25/1000000/set_1tick)

void TIM2_Out_Init(void)
{
	Macro_Set_Bit(RCC->AHB1ENR, 1);
	Macro_Set_Bit(RCC->APB1ENR, 0);
	Macro_Write_Block(TIM2->CCMR2,0xff, 0x68, 0);
	Macro_Write_Block(GPIOB->MODER, 0x03,0x02,20);
	Macro_Write_Block(GPIOB->AFR[1],0xf,0x01,8);
	TIM2->CCER =(1<<8);
	TIM2->CR1 = (0<<7)|(1<<4)|(0<<3);
	// Timer 주파수가 TIM3_FREQ가 되도록 PSC 설정
	TIM2->PSC = 0;
	// 요청한 주파수가 되도록 ARR 설정
	TIM2->ARR = set_pulse - 1;
	// Duty Rate 50%가 되도록 CCR3 설정
	TIM2->CCR3 = 0;
	// Manual Update(UG 발생)
	Macro_Set_Bit(TIM2->EGR,0);
}
void TIM2_Stopwatch_Start(void)
{
	Macro_Set_Bit(RCC->APB1ENR, 0);

	TIM2->CR1 = (1<<4)|(1<<3);
	TIM2->PSC = (unsigned int)(TIMXCLK/50000.0 + 0.5)-1;
	TIM2->ARR = TIM2_MAX;

	Macro_Set_Bit(TIM2->EGR,0);
	Macro_Set_Bit(TIM2->CR1, 0);
}

unsigned int TIM2_Stopwatch_Stop(void)
{
	unsigned int time;

	Macro_Clear_Bit(TIM2->CR1, 0);
	time = (TIM2_MAX - TIM2->CNT) * TIM2_TICK;
	return time;
}

void TIM2_Delay(int time)
{
	Macro_Set_Bit(RCC->APB1ENR, 0);

	TIM2->CR1 = (1<<4)|(1<<3);
	TIM2->PSC = (unsigned int)(TIMXCLK/TIM2_FREQ + 0.5)-1;
	TIM2->ARR = TIME2_PLS_OF_1ms * time;

	Macro_Set_Bit(TIM2->EGR,0);
	Macro_Clear_Bit(TIM2->SR, 0);
	Macro_Set_Bit(TIM2->CR1, 0);

	while(Macro_Check_Bit_Clear(TIM2->SR, 0));
	Macro_Clear_Bit(TIM2->CR1, 0);
}

#define TIM4_TICK	  		(20) 					// usec
#define TIM4_FREQ 	  		(HCLK/1000000UL) 	// Hz
#define TIME4_PLS_OF_1ms  	(1000./TIM4_TICK)
#define TIM4_MAX	  		(0xffffu)

void TIM4_Repeat_Interrupt_Enable(int en, int time)
{
	if(en)
	{
		Macro_Set_Bit(RCC->APB1ENR, 2);

		TIM4->CR1 = (1<<3);
		TIM4->PSC = (unsigned int)(TIMXCLK/TIM4_FREQ + 0.5)-1;
		TIM4->ARR = TIME4_PLS_OF_1ms * time;
		Macro_Set_Bit(TIM4->EGR,0);

		Macro_Clear_Bit(TIM4->SR, 0);
		NVIC_ClearPendingIRQ(30);

		Macro_Set_Bit(TIM4->DIER, 0);
		NVIC_EnableIRQ(30);

		Macro_Set_Bit(TIM4->CR1, 0);
	}

	else
	{
		NVIC_DisableIRQ(30);
		Macro_Clear_Bit(TIM4->CR1, 0);
		Macro_Clear_Bit(TIM4->DIER, 0);
	}
}

void TIM4_oneshot(int time_us)
{
    Macro_Set_Bit(RCC->APB1ENR, 2);

    /* 타이머 완전히 정지 */
    TIM4->CR1  = 0;
    TIM4->DIER = 0;
    TIM4->SR   = 0;

    /* TIM4CLK = 96MHz 기준 */
    TIM4->PSC = 96 - 1;
    TIM4->ARR = time_us - 1;
    TIM4->CNT = 0;

    /*
     * OPM  : 한 번 만료되면 자동 정지
     * URS  : 오버플로만 업데이트 인터럽트 발생
     *        EGR의 UG 때문에 인터럽트가 생기는 것을 방지
     */
    TIM4->CR1 = (1 << 3) | (1 << 2);

    /* PSC와 ARR 적용 */
    TIM4->EGR = (1 << 0);

    /* 모든 타이머 플래그 제거 */
    TIM4->SR = 0;

    NVIC_DisableIRQ(TIM4_IRQn);
    NVIC_ClearPendingIRQ(TIM4_IRQn);

    /* 업데이트 인터럽트만 허용 */
    TIM4->DIER = (1 << 0);

    NVIC_EnableIRQ(TIM4_IRQn);

    /* 타이머 시작 */
    TIM4->CR1 |= (1 << 0);
}

int TIM4_Check_Timeout(void)
{
	if(Macro_Check_Bit_Set(TIM4->SR, 0))
	{
		Macro_Clear_Bit(TIM4->SR, 0);
		return 1;
	}
	else
	{
		return 0;
	}
}

void TIM4_Stop(void)
{
	Macro_Clear_Bit(TIM4->CR1, 0);
}

void TIM4_Change_Value(int time)
{
	TIM4->ARR = TIME4_PLS_OF_1ms * time;
}

#define TIM3_FREQ 	  			(8000000.) 	      		// Hz
#define TIM3_TICK	  			(1000000./TIM3_FREQ)	// usec
#define TIME3_PLS_OF_1ms  		(1000./TIM3_TICK)

void TIM3_Out_Init(void)
{
	Macro_Set_Bit(RCC->AHB1ENR, 1);
	Macro_Set_Bit(RCC->APB1ENR, 1);

	Macro_Write_Block(GPIOB->MODER, 0x3, 0x2, 0);
	Macro_Write_Block(GPIOB->AFR[0], 0xf, 0x2, 0);

	Macro_Write_Block(TIM3->CCMR2,0xff, 0x68, 0);
	TIM3->CCER = (0<<9)|(1<<8);
}

void TIM3_Out_Freq_Generation(unsigned short freq)
{
	TIM3->PSC = (unsigned int)(TIMXCLK/TIM3_FREQ + 0.5)-1;
	TIM3->ARR = TIM3_FREQ/freq-1;
	TIM3->CCR3 = TIM3->ARR/2;

	Macro_Set_Bit(TIM3->EGR,0);
	TIM3->CR1 = (1<<4)|(0<<3)|(0<<1)|(1<<0);
}

void TIM3_Out_Stop(void)
{
	Macro_Clear_Bit(TIM3->CR1, 0);
}
