#include "device_driver.h"
#include <stdio.h>
#include <string.h>

uint32_t buff[96];
uint32_t i = 0;
volatile int is_dma_busy = 0;

static void Sys_Init(int baud) 
{
	SCB->CPACR |= (0x3 << 10*2)|(0x3 << 11*2); 
	Clock_Init();
	Uart2_Init(baud);
	setvbuf(stdout, NULL, _IONBF, 0);
	LED_Init();
}

extern volatile int TIM4_Expired;

void Main(void)
{
    int timer_started = 0;
    
    Sys_Init(115200);
    printf("\nTest\n");
    
    for(;;)
    {
        if ((is_dma_busy == 0) && (timer_started == 0))
        {
            Macro_Clear_Bit(TIM2->CR1, 0);
            Macro_Write_Block(GPIOB->MODER, 0x03, 0x01, 20);
            Macro_Clear_Bit(GPIOB->ODR, 10);

            TIM4_Expired = 0;
            TIM4_oneshot(50);

            timer_started = 1;
        }
        
        if (TIM4_Expired)
        {
            TIM4_Expired = 0;
            timer_started = 0;

            i = 0;
            make_RGB(0,0,0);
            make_RGB(0,0,0);
            make_RGB(0,0,0);
            make_RGB(0,0,0);
            
            TIM2_Out_Init();
            DMA1_Stream1_M2P_Start(buff,96);
            // Timer Start
	        Macro_Set_Bit(TIM2->CR1,0);
        }
    }
}

void make_RGB (uint8_t g,uint8_t r, uint8_t b)
{

    for (int bit= 7; bit>= 0; bit--)
    {
        if ((g>>bit) & 0x01)
        {
            buff[i++]=82;
        }
        else 
        {
            buff[i++]=32;
        }
    }

    for (int bit= 7; bit>= 0; bit--)
    {
        if ((r>>bit) & 0x01)
        {
            buff[i++]=82;
        }
        else 
        {
            buff[i++]=32;
        }
    }

    for (int bit= 7; bit>= 0; bit--)
    {
        if ((b>>bit) & 0x01)
        {
            buff[i++]=82;
        }
        else 
        {
            buff[i++]=32;
        }
    }
}

