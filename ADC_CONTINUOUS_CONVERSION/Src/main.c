
#include "adc.h"

int main(void) {
    ADC_Init();
    while (1) {
        uint16_t value = ADC_Read();
        // value is 0–4095
    }
}


