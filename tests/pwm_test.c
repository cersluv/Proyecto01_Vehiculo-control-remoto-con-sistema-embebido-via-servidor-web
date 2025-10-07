#include <stdio.h>
#include <unistd.h>
#include "gpio.h"
#include "pwm.h"

int main() {
    printf("Testing GPIO library\n");
    
    // GPIO test
    int led_pin = 517; // GPIO 5
    blink(led_pin, 2, 5); // Blink at 2Hz for 5 seconds
    
    printf("\nTesting PWM library\n");
    
    // PWM test - PWM0 on Raspberry Pi 4 (GPIO 18)
    int chip = 0;
    int channel = 0;
    
    pwmInit(chip, channel);
    pwmSetFrequency(chip, channel, 1000); // 1kHz
    pwmSetDutyCyclePercent(chip, channel, 50); // 50% duty cycle
    pwmEnable(chip, channel);
    
    printf("PWM running at 50%% duty cycle for 5 seconds\n");
    sleep(5);
    
    // Fade effect
    printf("Fading PWM from 0%% to 100%%\n");
    for (int i = 0; i <= 100; i += 5) {
        pwmSetDutyCyclePercent(chip, channel, i);
        usleep(100000); // 100ms
    }
    
    pwmCleanup(chip, channel);
    
    return 0;
}
