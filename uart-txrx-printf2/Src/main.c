/**
 * @file    main.c
 * @brief   UART2 TX/RX with printf on STM32F407 Discovery
 *
 * Hardware setup (requires USB-TTL adapter):
 *   PA2 (TX) --> Adapter RX
 *   PA3 (RX) --> Adapter TX
 *   GND      --> Adapter GND
 *
 * UART settings: 115200 baud, 8N1
 * LED: PD15 (blue LED on STM32F407 Discovery)
 *
 * Note: SB13 and SB14 solder bridges must be closed on the Discovery board
 *       to use ST-Link as virtual COM port. Otherwise use a USB-TTL adapter.
 */

#include <stdint.h>
#include <stdio.h>
#include "stm32f407xx.h"

/* ─── Peripheral clock enables ─────────────────────────────────────────── */
#define GPIOA_EN        (1U << 0)
#define GPIOD_EN        (1U << 3)
#define UART2EN         (1U << 17)

/* ─── Clock frequencies ─────────────────────────────────────────────────── */
#define SYS_FREQ        16000000U
#define APB1_CLK        SYS_FREQ

/* ─── USART CR1 bits ────────────────────────────────────────────────────── */
#define CR1_TE          (1U << 3)   /* Transmitter enable  */
#define CR1_RE          (1U << 2)   /* Receiver enable     */
#define CR1_UE          (1U << 0)   /* USART enable        */

/* ─── USART SR bits ─────────────────────────────────────────────────────── */
#define SR_TXE          (1U << 7)   /* Transmit data register empty */
#define SR_RXNE         (1U << 5)   /* Read data register not empty */

/* ─── GPIO ──────────────────────────────────────────────────────────────── */
#define LED_PIN         (1U << 15)  /* PD15 blue LED on Discovery board */

/* ─── Function prototypes ───────────────────────────────────────────────── */
void uart_rxtx_init(void);
void led_init(void);

static void          uart_set_baudrate(USART_TypeDef *USARTx, uint32_t periphclk, uint32_t baudrate);
static uint16_t      compute_uart_div (uint32_t periphclk, uint32_t baudrate);
static void          uart_write       (USART_TypeDef *USARTx, uint8_t ch);
static uint8_t       uart_read        (USART_TypeDef *USARTx);

/* ─── Redirect printf to UART2 ──────────────────────────────────────────── */
int _write(int file, char *ptr, int len)
{
    for (int i = 0; i < len; i++) {
        uart_write(USART2, (uint8_t)ptr[i]);
    }
    return len;
}

/* ─── Main ──────────────────────────────────────────────────────────────── */
int main(void)
{
    uart_rxtx_init();
    led_init();

    printf("READY\r\n");

    while (1) {
        uint8_t received = uart_read(USART2);

        if (received == '1') {
            GPIOD->ODR |= LED_PIN;
            printf("LED ON\r\n");
        } else {
            GPIOD->ODR &= ~LED_PIN;
            printf("LED OFF\r\n");
        }
    }
}

/* ─── LED init ──────────────────────────────────────────────────────────── */
void led_init(void)
{
    /* Enable clock access to GPIOD */
    RCC->AHB1ENR |= GPIOD_EN;

    /* Set PD15 as general-purpose output (MODER = 01) */
    GPIOD->MODER |=  (1U << 30);
    GPIOD->MODER &= ~(1U << 31);
}

/* ─── UART2 RX/TX init ──────────────────────────────────────────────────── */
void uart_rxtx_init(void)
{
    /* 1. Enable clock access to GPIOA */
    RCC->AHB1ENR |= GPIOA_EN;

    /* 2. Set PA2 (TX) as alternate function (MODER = 10) */
    GPIOA->MODER &= ~(1U << 4);
    GPIOA->MODER |=  (1U << 5);

    /* 3. Set PA2 alternate function to AF7 (USART2_TX) */
    GPIOA->AFR[0] |=  (1U << 8);
    GPIOA->AFR[0] |=  (1U << 9);
    GPIOA->AFR[0] |=  (1U << 10);
    GPIOA->AFR[0] &= ~(1U << 11);

    /* 4. Set PA3 (RX) as alternate function (MODER = 10) */
    GPIOA->MODER &= ~(1U << 6);
    GPIOA->MODER |=  (1U << 7);

    /* 5. Set PA3 alternate function to AF7 (USART2_RX) */
    GPIOA->AFR[0] |=  (1U << 12);
    GPIOA->AFR[0] |=  (1U << 13);
    GPIOA->AFR[0] |=  (1U << 14);
    GPIOA->AFR[0] &= ~(1U << 15);

    /* 6. Enable clock access to USART2 */
    RCC->APB1ENR |= UART2EN;

    /* 7. Set baud rate to 115200 */
    uart_set_baudrate(USART2, APB1_CLK, 115200);

    /* 8. Enable TX, RX, then enable USART */
    USART2->CR1 = CR1_TE | CR1_RE;
    USART2->CR1 |= CR1_UE;
}

/* ─── UART helpers ──────────────────────────────────────────────────────── */
static uint8_t uart_read(USART_TypeDef *USARTx)
{
    while (!(USARTx->SR & SR_RXNE)) {}
    return (uint8_t)(USARTx->DR & 0xFF);
}

static void uart_write(USART_TypeDef *USARTx, uint8_t ch)
{
    while (!(USARTx->SR & SR_TXE)) {}
    USARTx->DR = (ch & 0xFF);
}

static void uart_set_baudrate(USART_TypeDef *USARTx, uint32_t periphclk, uint32_t baudrate)
{
    USARTx->BRR = compute_uart_div(periphclk, baudrate);
}

static uint16_t compute_uart_div(uint32_t periphclk, uint32_t baudrate)
{
    /* Simple integer division — gives BRR = 138 for 16MHz / 115200 */
    return (uint16_t)(periphclk / baudrate);
}
