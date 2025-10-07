#include "pwm.h"
#include "gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define MAX_PWM_PINS 10

typedef struct {
    int pin;
    float freq_hz;
    float duty_percent;
    int running;
    pthread_t thread;
} PwmPin;

static PwmPin pwm_pins[MAX_PWM_PINS];
static int pwm_pins_count = 0;
static pthread_mutex_t pwm_mutex = PTHREAD_MUTEX_INITIALIZER;

static void* pwm_thread(void* arg) {
    PwmPin* pwm = (PwmPin*)arg;
    
    pinMode(pwm->pin, OUTPUT);
    
    while (pwm->running) {
        if (pwm->duty_percent <= 0) {
            digitalWrite(pwm->pin, LOW);
            usleep(1000000 / pwm->freq_hz);
            continue;
        }
        
        if (pwm->duty_percent >= 100) {
            digitalWrite(pwm->pin, HIGH);
            usleep(1000000 / pwm->freq_hz);
            continue;
        }
        
        unsigned long period_us = 1000000 / pwm->freq_hz;
        unsigned long high_time_us = (period_us * pwm->duty_percent) / 100;
        unsigned long low_time_us = period_us - high_time_us;
        
        digitalWrite(pwm->pin, HIGH);
        usleep(high_time_us);
        
        if (pwm->running) {
            digitalWrite(pwm->pin, LOW);
            usleep(low_time_us);
        }
    }
    
    digitalWrite(pwm->pin, LOW);
    return NULL;
}

static PwmPin* find_pwm_pin(int pin) {
    for (int i = 0; i < pwm_pins_count; i++) {
        if (pwm_pins[i].pin == pin) {
            return &pwm_pins[i];
        }
    }
    return NULL;
}

int softPwmInit(int pin, float freq_hz, float duty_percent) {
    pthread_mutex_lock(&pwm_mutex);
    
    if (pwm_pins_count >= MAX_PWM_PINS) {
        fprintf(stderr, "Error: Maximum number of PWM pins reached\n");
        pthread_mutex_unlock(&pwm_mutex);
        return -1;
    }
    
    PwmPin* existing = find_pwm_pin(pin);
    if (existing != NULL) {
        fprintf(stderr, "Error: PWM already initialized on pin %d\n", pin);
        pthread_mutex_unlock(&pwm_mutex);
        return -1;
    }
    
    if (duty_percent < 0) duty_percent = 0;
    if (duty_percent > 100) duty_percent = 100;
    if (freq_hz <= 0) freq_hz = 1000;
    
    PwmPin* pwm = &pwm_pins[pwm_pins_count++];
    pwm->pin = pin;
    pwm->freq_hz = freq_hz;
    pwm->duty_percent = duty_percent;
    pwm->running = 1;
    
    if (pthread_create(&pwm->thread, NULL, pwm_thread, pwm) != 0) {
        fprintf(stderr, "Error: Failed to create PWM thread\n");
        pwm_pins_count--;
        pthread_mutex_unlock(&pwm_mutex);
        return -1;
    }
    
    pthread_mutex_unlock(&pwm_mutex);
    return 0;
}

void softPwmSetDutyCycle(int pin, float duty_percent) {
    pthread_mutex_lock(&pwm_mutex);
    
    PwmPin* pwm = find_pwm_pin(pin);
    if (pwm == NULL) {
        fprintf(stderr, "Error: PWM not initialized on pin %d\n", pin);
        pthread_mutex_unlock(&pwm_mutex);
        return;
    }
    
    if (duty_percent < 0) duty_percent = 0;
    if (duty_percent > 100) duty_percent = 100;
    
    pwm->duty_percent = duty_percent;
    
    pthread_mutex_unlock(&pwm_mutex);
}

void softPwmSetFrequency(int pin, float freq_hz) {
    pthread_mutex_lock(&pwm_mutex);
    
    PwmPin* pwm = find_pwm_pin(pin);
    if (pwm == NULL) {
        fprintf(stderr, "Error: PWM not initialized on pin %d\n", pin);
        pthread_mutex_unlock(&pwm_mutex);
        return;
    }
    
    if (freq_hz <= 0) freq_hz = 1000;
    pwm->freq_hz = freq_hz;
    
    pthread_mutex_unlock(&pwm_mutex);
}

void softPwmStop(int pin) {
    pthread_mutex_lock(&pwm_mutex);
    
    PwmPin* pwm = find_pwm_pin(pin);
    if (pwm == NULL) {
        pthread_mutex_unlock(&pwm_mutex);
        return;
    }
    
    pwm->running = 0;
    pthread_mutex_unlock(&pwm_mutex);
    
    pthread_join(pwm->thread, NULL);
    
    pthread_mutex_lock(&pwm_mutex);
    
    // Remove from array
    int index = pwm - pwm_pins;
    for (int i = index; i < pwm_pins_count - 1; i++) {
        pwm_pins[i] = pwm_pins[i + 1];
    }
    pwm_pins_count--;
    
    pthread_mutex_unlock(&pwm_mutex);
}

void softPwmRun(int pin, float freq_hz, float duty_percent, int duration_sec) {
    pinMode(pin, OUTPUT);
    
    if (duty_percent < 0) duty_percent = 0;
    if (duty_percent > 100) duty_percent = 100;
    if (freq_hz <= 0) freq_hz = 1000;
    
    unsigned long period_us = 1000000 / freq_hz;
    unsigned long high_time_us = (period_us * duty_percent) / 100;
    unsigned long low_time_us = period_us - high_time_us;
    
    unsigned long cycles = freq_hz * duration_sec;
    
    for (unsigned long i = 0; i < cycles; i++) {
        if (duty_percent > 0) {
            digitalWrite(pin, HIGH);
            usleep(high_time_us);
        }
        
        if (duty_percent < 100) {
            digitalWrite(pin, LOW);
            usleep(low_time_us);
        }
    }
    
    digitalWrite(pin, LOW);
}
