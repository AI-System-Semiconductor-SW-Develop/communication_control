#include "device_driver.h"
#include "stm32f411xe.h"

extern volatile int is_dma_busy;

void DMA1_Stream1_M2P_Start(void * src_addr, int num)
{
	Macro_Set_Bit(RCC->AHB1ENR, 21);

    DMA1_Stream1->CR = (0x3 << 25)|(0x2 << 13)|(0x2 << 11)|(0x1 << 10)|(0x1 << 6)|(0x1 << 4)|(0x0 << 0);
	DMA1_Stream1->PAR  = (uint32_t)&(TIM2->CCR3);
	DMA1_Stream1->M0AR = (unsigned int)src_addr;
	DMA1_Stream1->NDTR = num;
	
	DMA1->LIFCR = (1 << 6)  |  (1 << 8)  |  (1 << 9)  | (1 << 10) | (1 << 11);  
	
	NVIC_ClearPendingIRQ(12);
	NVIC_EnableIRQ(12);
	
	is_dma_busy = 1;

	Macro_Set_Bit(TIM2->DIER, 11);
	Macro_Set_Bit(DMA1_Stream1->CR, 0);	
}

void DMA1_Stream6_USART2_TX_Satrt(void * src_addr, int num)
{
	Macro_Set_Bit(RCC->AHB1ENR, 21);

    DMA1_Stream6->CR = (0x4 << 25)|(0x0 << 13)|(0x0 << 11)|(0x1 << 10)|(0x0 << 9)|(0x1 << 6)|(0x0 << 0);
	DMA1_Stream6->FCR = 0;
	DMA1_Stream6->PAR  = (unsigned int)&USART2->DR;
	DMA1_Stream6->M0AR = (unsigned int)src_addr;
	DMA1_Stream6->NDTR = num;
	
	DMA1->HIFCR = 0x3F << 16;
	Macro_Set_Bit(DMA1_Stream6->CR, 4);
	NVIC_ClearPendingIRQ(17);
	NVIC_EnableIRQ(17);

	Macro_Set_Bit(DMA1_Stream6->CR, 0);	
}
