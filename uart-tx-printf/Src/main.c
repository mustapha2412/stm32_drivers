
#include<stdint.h>
#include<stdio.h>
#include "stm32f407xx.h"
// LED BLUE PD15
#define pin5          (1U<<15)
#define led_pin       pin5
#define pin_0         (1U<<0)
#define button_pin    pin_0
#define gpioa_EN     (1U<<0)
#define gpiod_EN     (1U<<3)
#define UART2EN		  (1U<<17)
#define sys_FREQ	  16000000
#define APB1_clk      sys_FREQ
#define CR1_TE        (1U<<3)
#define CR1_UE         (1U<<0)
#define SR_TXE         (1U<<7)
static uint16_t compute_uart_div(uint32_t periphclk , uint32_t baudrate) ;
static void uart_set_baudrate(USART_TypeDef *USARTX,uint32_t pariphclk ,uint32_t baudrate);
static void uart2_write(USART_TypeDef *USARTX,uint8_t ch);
void uart_tx_init (void);
int _io_putchar(int ch){
	uart2_write(USART2,ch);
	return(ch);
}

int main(void )
{
	uart_tx_init();


	while(1){
		printf("hello from stm32 F407vG \n");
	}




}
void uart_tx_init (void){
	//***********configure uart**********//
	//enable clock access to gpioa for pa2 as tx line
	RCC->AHB1ENR |= gpioa_EN;
	// SET PA2 as alternate function mode
	GPIOA->MODER &=~ (1U<<4);
	GPIOA->MODER |= (1U<<5);
	//set the alternate function set to uart
	GPIOA->AFR[0] |= (1U<<8);
	GPIOA->AFR[0] |= (1U<<9);
	GPIOA->AFR[0] |= (1U<<10);
	GPIOA->AFR[0] &=~ (1U<<11);



	//*********configure the uart module ************/
	//enable clock access to uart2
	RCC->APB1ENR |= UART2EN;
	// configure baude rate
	uart_set_baudrate(USART2,APB1_clk,115200);
	//configure transfer direction
	USART2->CR1 = CR1_TE;
	//enable uart module
	USART2->CR1 |= CR1_UE;



}
static void uart2_write(USART_TypeDef *USARTX,uint8_t ch){
	// wait for transmit data register to be empty
	while(! (USARTX->SR & SR_TXE)){}
	//transmit value
	USARTX->DR = ch & 0xFF;
}
static void uart_set_baudrate(USART_TypeDef *USARTX,uint32_t pariphclk ,uint32_t baudrate){
	USARTX->BRR = compute_uart_div(pariphclk,baudrate);
}
static uint16_t compute_uart_div(uint32_t periphclk , uint32_t baudrate){
return ((periphclk+(baudrate/2))/baudrate);
}

