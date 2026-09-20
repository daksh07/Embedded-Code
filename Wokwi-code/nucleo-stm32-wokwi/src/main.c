#include "stm32c0xx_hal.h"
#include <string.h>

//RCC_BASE 0x4002000
#define RCC_IOPENR (*(volatile uint32_t *)(RCC_BASE + 0x34))
//GPIOA_BASE 0x50000000
#define GPIOA_MODER (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_OTYPER (*(volatile uint32_t *)(GPIOA_BASE + 0x04))
#define GPIOA_OSPEEDR (*(volatile uint32_t *)(GPIOA_BASE + 0x08))
#define GPIOA_PUPDR (*(volatile uint32_t *)(GPIOA_BASE + 0x0C))
#define GPIOA_ODR (*(volatile uint32_t *)(GPIOA_BASE + 0x14))

int main(void){
 
    RCC_IOPENR |= (1 << 0); // Enable GPIOA clock
    GPIOA_MODER &= ~(3 << (5 * 2)); // Clear mode bits for PA5
    GPIOA_MODER |= (1 << (5 * 2)); // Set PA5 as output
    GPIOA_OTYPER &= ~(1 << 5); // Set PA5 as push-pull
    GPIOA_OSPEEDR &= ~(3 << (5 * 2)); // Clear speed bits for PA5
    GPIOA_OSPEEDR |= (1 << (5 * 2)); // Set PA5 as medium speed
    GPIOA_PUPDR &= ~(3 << (5 * 2)); // Clear pull-up/pull-down bits for PA5

    while(1){
        GPIOA_ODR ^= (1 << 5); // Toggle PA5
        for(volatile int i = 0; i < 100000; i++); // Delay
    }
    
}