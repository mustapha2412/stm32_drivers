#include "adc.h"

void ADC_Init(void) {
    // 1. Clocks
    RCC->AHB1ENR |= (1 << 0);   // GPIOA
    RCC->APB2ENR |= (1 << 8);   // ADC1

    // 2. PA0 analog
    GPIOA->MODER |= (3 << 0);

    // 3. Prescaler
    ADC123_COMMON->CCR = (1 << 16);

    // 4. Reset CR1, CR2
    ADC1->CR1 = 0;
    ADC1->CR2 = 0;

    // 5. Sample time channel 0
    ADC1->SMPR2 = (7 << 0);

    // 6. Sequence
    ADC1->SQR1 = 0;
    ADC1->SQR3 = 0;

    // 7. Power on
    ADC1->CR2 |= (1 << 0);
    for (volatile int i = 0; i < 1000; i++);
}

uint16_t ADC_Read(void) {
    ADC1->CR2 |= (1 << 30);             // SWSTART
    while (!(ADC1->SR & (1 << 1)));     // wait EOC
    return (uint16_t)(ADC1->DR);        // read result
}
