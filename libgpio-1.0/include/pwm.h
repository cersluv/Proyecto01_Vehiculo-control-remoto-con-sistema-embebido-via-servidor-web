#ifndef PWM_H
#define PWM_H

/**
 * Software PWM implementation using GPIO
 * Note: This is a software-based PWM, not hardware PWM
 * Accuracy depends on system load and timing precision
 */

/**
 * Initialize software PWM on a GPIO pin
 * @param pin GPIO pin number (512-538)
 * @param freq_hz Frequency in Hz (1-10000 recommended)
 * @param duty_percent Initial duty cycle percentage (0-100)
 * @return 0 on success, -1 on error
 */
int softPwmInit(int pin, float freq_hz, float duty_percent);

/**
 * Set PWM duty cycle percentage
 * @param pin GPIO pin number
 * @param duty_percent Duty cycle (0-100)
 */
void softPwmSetDutyCycle(int pin, float duty_percent);

/**
 * Set PWM frequency
 * @param pin GPIO pin number
 * @param freq_hz Frequency in Hz
 */
void softPwmSetFrequency(int pin, float freq_hz);

/**
 * Stop PWM on a pin
 * @param pin GPIO pin number
 */
void softPwmStop(int pin);

/**
 * Run PWM for a specific duration
 * This is a blocking call
 * @param pin GPIO pin number
 * @param freq_hz Frequency in Hz
 * @param duty_percent Duty cycle (0-100)
 * @param duration_sec Duration in seconds
 */
void softPwmRun(int pin, float freq_hz, float duty_percent, int duration_sec);

#endif /* PWM_H */
