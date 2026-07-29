#include "device_driver.h"
#include <stdio.h>

void _Invalid_ISR(void)
{
	unsigned int r = Macro_Extract_Area(SCB->ICSR, 0x1ff, 0);
	printf("\nInvalid_Exception: %d!\n", r);
	printf("Invalid_ISR: %d!\n", r - 16);
	for(;;);
}

volatile int TIM4_Expired = 0;

void TIM4_IRQHandler(void)
{
	Macro_Clear_Bit(TIM4->SR, 0);
	NVIC_ClearPendingIRQ(30);
	TIM4_Expired = 1;
}

volatile int DMA2_STREAM0_DONE = 0;

void DMA2_Stream0_IRQHandler(void)
{
	DMA2->LIFCR = 0x3F << 0;
	NVIC_ClearPendingIRQ(56);
    DMA2_STREAM0_DONE = 1;
}

volatile int DMA1_STREAM6_DONE = 0;

void DMA1_Stream6_IRQHandler(void)
{
	DMA1->HIFCR = 0x3F << 16;
	NVIC_ClearPendingIRQ(17);
    DMA1_STREAM6_DONE = 1;
}

extern volatile int is_dma_busy;

void DMA1_Stream1_IRQHandler(void)
{
    if (DMA1->LISR & (1 << 11)) 
    {
        DMA1->LIFCR = (1 << 11); 
		Macro_Clear_Bit(TIM2->DIER, 11);
        is_dma_busy = 0;      
    }
}