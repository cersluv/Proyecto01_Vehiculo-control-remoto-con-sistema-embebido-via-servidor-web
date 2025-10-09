#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "gpio.h"
#include "pwm.h"

// === CONFIGURACIÓN DE PINES GPIO (RPi4) ===
#define IN1  515
#define IN2  516
#define IN3  517
#define IN4  518

#define LED_ADELANTE   520
#define LED_ATRAS      521
#define LED_IZQUIERDA  522
#define LED_DERECHA    523

// === PARÁMETROS DE CONTROL ===
#define PWM_FREQ_MOTOR     1000.0
#define VELOCIDAD_BASE     60.0
#define POTENCIA_GIRO      85.0
#define DURACION_TEST      2

// === INICIALIZACIÓN DE SISTEMA ===
void inicializar_leds() {
    pinMode(LED_ADELANTE, OUTPUT);
    pinMode(LED_ATRAS, OUTPUT);
    pinMode(LED_IZQUIERDA, OUTPUT);
    pinMode(LED_DERECHA, OUTPUT);
    digitalWrite(LED_ADELANTE, LOW);
    digitalWrite(LED_ATRAS, LOW);
    digitalWrite(LED_IZQUIERDA, LOW);
    digitalWrite(LED_DERECHA, LOW);
}

void apagar_leds() {
    digitalWrite(LED_ADELANTE, LOW);
    digitalWrite(LED_ATRAS, LOW);
    digitalWrite(LED_IZQUIERDA, LOW);
    digitalWrite(LED_DERECHA, LOW);
}

void inicializar_pwms() {
    // Crear hilos PWM solo una vez
    softPwmInit(IN1, PWM_FREQ_MOTOR, 0.0f);
    softPwmInit(IN2, PWM_FREQ_MOTOR, 0.0f);
    softPwmInit(IN3, PWM_FREQ_MOTOR, 0.0f);
    softPwmInit(IN4, PWM_FREQ_MOTOR, 0.0f);
}

void detener_motores_local() {
    softPwmSetDutyCycle(IN1, 0.0f);
    softPwmSetDutyCycle(IN2, 0.0f);
    softPwmSetDutyCycle(IN3, 0.0f);
    softPwmSetDutyCycle(IN4, 0.0f);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
}

// === FUNCIONES AUXILIARES DE MOTORES ===
void set_motor_trasero_adelante(float duty) {
    digitalWrite(IN2, LOW);
    softPwmSetDutyCycle(IN1, duty);
}
void set_motor_trasero_atras(float duty) {
    digitalWrite(IN1, LOW);
    softPwmSetDutyCycle(IN2, duty);
}
void set_motor_delantero_izq(float duty) {
    digitalWrite(IN4, LOW);
    softPwmSetDutyCycle(IN3, duty);
}
void set_motor_delantero_der(float duty) {
    digitalWrite(IN3, LOW);
    softPwmSetDutyCycle(IN4, duty);
}

// === MOVIMIENTOS ===
void adelante(float velocidad) {
    printf("  → Moviendo ADELANTE (velocidad: %.0f%%)\n", velocidad);
    set_motor_trasero_adelante(velocidad);
    softPwmSetDutyCycle(IN3, 0.0f);
    softPwmSetDutyCycle(IN4, 0.0f);
    apagar_leds();
    digitalWrite(LED_ADELANTE, HIGH);
    sleep(DURACION_TEST);
    detener_motores_local();
    apagar_leds();
}

void atras(float velocidad) {
    printf("  → Moviendo ATRÁS (velocidad: %.0f%%)\n", velocidad);
    set_motor_trasero_atras(velocidad);
    softPwmSetDutyCycle(IN3, 0.0f);
    softPwmSetDutyCycle(IN4, 0.0f);
    apagar_leds();
    digitalWrite(LED_ATRAS, HIGH);
    sleep(DURACION_TEST);
    detener_motores_local();
    apagar_leds();
}

void izquierda(float potencia) {
    printf("  → Girando IZQUIERDA (potencia: %.0f%%)\n", potencia);
    set_motor_delantero_izq(potencia);
    softPwmSetDutyCycle(IN1, 0.0f);
    softPwmSetDutyCycle(IN2, 0.0f);
    apagar_leds();
    digitalWrite(LED_IZQUIERDA, HIGH);
    sleep(DURACION_TEST);
    detener_motores_local();
    apagar_leds();
}

void derecha(float potencia) {
    printf("  → Girando DERECHA (potencia: %.0f%%)\n", potencia);
    set_motor_delantero_der(potencia);
    softPwmSetDutyCycle(IN1, 0.0f);
    softPwmSetDutyCycle(IN2, 0.0f);
    apagar_leds();
    digitalWrite(LED_DERECHA, HIGH);
    sleep(DURACION_TEST);
    detener_motores_local();
    apagar_leds();
}

void adelante_izquierda(float velocidad, float potencia) {
    printf("  → ADELANTE + IZQUIERDA (vel: %.0f%%, giro: %.0f%%)\n", velocidad, potencia);
    set_motor_trasero_adelante(velocidad);
    set_motor_delantero_izq(potencia);
    apagar_leds();
    digitalWrite(LED_ADELANTE, HIGH);
    digitalWrite(LED_IZQUIERDA, HIGH);
    sleep(DURACION_TEST);
    detener_motores_local();
    apagar_leds();
}

void adelante_derecha(float velocidad, float potencia) {
    printf("  → ADELANTE + DERECHA (vel: %.0f%%, giro: %.0f%%)\n", velocidad, potencia);
    set_motor_trasero_adelante(velocidad);
    set_motor_delantero_der(potencia);
    apagar_leds();
    digitalWrite(LED_ADELANTE, HIGH);
    digitalWrite(LED_DERECHA, HIGH);
    sleep(DURACION_TEST);
    detener_motores_local();
    apagar_leds();
}

void atras_izquierda(float velocidad, float potencia) {
    printf("  → ATRÁS + IZQUIERDA (vel: %.0f%%, giro: %.0f%%)\n", velocidad, potencia);
    set_motor_trasero_atras(velocidad);
    set_motor_delantero_izq(potencia);
    apagar_leds();
    digitalWrite(LED_ATRAS, HIGH);
    digitalWrite(LED_IZQUIERDA, HIGH);
    sleep(DURACION_TEST);
    detener_motores_local();
    apagar_leds();
}

void atras_derecha(float velocidad, float potencia) {
    printf("  → ATRÁS + DERECHA (vel: %.0f%%, giro: %.0f%%)\n", velocidad, potencia);
    set_motor_trasero_atras(velocidad);
    set_motor_delantero_der(potencia);
    apagar_leds();
    digitalWrite(LED_ATRAS, HIGH);
    digitalWrite(LED_DERECHA, HIGH);
    sleep(DURACION_TEST);
    detener_motores_local();
    apagar_leds();
}

// === TESTS ===
void test_leds() {
    printf("\n=== TEST 1: LEDs Indicadores ===\n");
    int leds[] = {LED_ADELANTE, LED_ATRAS, LED_IZQUIERDA, LED_DERECHA};
    const char* nombres[] = {"Adelante", "Atrás", "Izquierda", "Derecha"};
    for (int i = 0; i < 4; i++) {
        printf("LED %s...\n", nombres[i]);
        digitalWrite(leds[i], HIGH);
        sleep(1);
        digitalWrite(leds[i], LOW);
    }
    printf("Todos los LEDs...\n");
    for (int i = 0; i < 4; i++) digitalWrite(leds[i], HIGH);
    sleep(1);
    apagar_leds();
    printf("✓ Test LEDs completado\n");
}

void test_movimientos_basicos() {
    printf("\n=== TEST 2: Movimientos Básicos ===\n");
    adelante(VELOCIDAD_BASE); sleep(1);
    atras(VELOCIDAD_BASE); sleep(1);
    derecha(POTENCIA_GIRO); sleep(1);
    izquierda(POTENCIA_GIRO); sleep(1);
    printf("✓ Test movimientos básicos completado\n");
}

void test_movimientos_combinados() {
    printf("\n=== TEST 3: Movimientos Combinados ===\n");
    adelante_izquierda(VELOCIDAD_BASE, POTENCIA_GIRO); sleep(1);
    adelante_derecha(VELOCIDAD_BASE, POTENCIA_GIRO); sleep(1);
    atras_izquierda(VELOCIDAD_BASE, POTENCIA_GIRO); sleep(1);
    atras_derecha(VELOCIDAD_BASE, POTENCIA_GIRO); sleep(1);
    printf("✓ Test movimientos combinados completado\n");
}

void test_velocidades() {
    printf("\n=== TEST 4: Diferentes Velocidades ===\n");
    printf("Velocidad 30%%\n"); adelante(30.0); sleep(1);
    printf("Velocidad 60%%\n"); adelante(60.0); sleep(1);
    printf("Velocidad 90%%\n"); adelante(90.0); sleep(1);
    printf("✓ Test velocidades completado\n");
}

void secuencia_demostracion() {
    printf("\n=== SECUENCIA DE DEMOSTRACIÓN ===\n");
    printf("Ejecutando recorrido completo...\n\n");
    adelante(VELOCIDAD_BASE); sleep(1);
    adelante_derecha(VELOCIDAD_BASE, POTENCIA_GIRO); sleep(1);
    adelante(VELOCIDAD_BASE); sleep(1);
    adelante_izquierda(VELOCIDAD_BASE, POTENCIA_GIRO); sleep(1);
    atras(VELOCIDAD_BASE); sleep(1);
    printf("  → DETENIDO\n");
    detener_motores_local();
    apagar_leds();
    printf("\n✓ Secuencia de demostración completada\n");
}

// === MAIN ===
int main() {
    printf("╔════════════════════════════════════════════╗\n");
    printf("║  TEST COMPLETO - CARRITO RPi4 + L293D     ║\n");
    printf("╚════════════════════════════════════════════╝\n\n");

    printf("Inicializando GPIOs...\n");
    inicializar_leds();
    pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
    inicializar_pwms();
    printf("✓ Sistema inicializado\n");

    sleep(2);
    test_leds(); sleep(2);
    test_movimientos_basicos(); sleep(2);
    test_movimientos_combinados(); sleep(2);
    test_velocidades(); sleep(2);
    secuencia_demostracion();

    printf("\n=== FINALIZANDO ===\n");
    detener_motores_local();
    softPwmStop(IN1); softPwmStop(IN2);
    softPwmStop(IN3); softPwmStop(IN4);
    apagar_leds();
    printf("✓ Todos los sistemas apagados\n\n");
    printf("¡Test completado exitosamente! ✓\n");
    return 0;
}

