#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <openssl/hmac.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include "gpio.h"
#include "pwm.h"

#define PORT 5000
#define BUFFER_SIZE 4096

// === CONFIGURACIÓN DE PINES ===
#define IN1  515
#define IN2  516
#define IN3  517
#define IN4  518

#define LED_ADELANTE   520
#define LED_ATRAS      521
#define LED_IZQUIERDA  523
#define LED_DERECHA    522

// === PARÁMETROS DE CONTROL ===
#define PWM_FREQ_MOTOR     1000.0

// === CONFIGURACIÓN DE CÁMARA ===
const char *resolution = "640x480";
const char *device_jardin = "/dev/video0";
const char *device_frontal = "/dev/video1";
const char *imageName = "/home/root/foto_temp.jpg";

// === VARIABLES GLOBALES ===
typedef struct {
    char luces[32];
    char direccion[32];
    float velocidad_actual;
} EstadoCarro;

EstadoCarro estado_carro = {"Apagadas", "Detenido", 60.0};
pthread_mutex_t estado_mutex = PTHREAD_MUTEX_INITIALIZER;

// Variables para blink
pthread_t thread_blink_izq = 0;
pthread_t thread_blink_der = 0;
bool blink_izq_activo = false;
bool blink_der_activo = false;
pthread_mutex_t blink_mutex = PTHREAD_MUTEX_INITIALIZER;

const char *STORED_HASH = "623fd75beab1b12972f837f5577aaf1ab87ca0df094151a8d8ec8b1fe0c94dc0";

// === HEADERS HTTP ===
const char* headers =
"HTTP/1.1 200 OK\r\n"
"Content-Type: application/json\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
"Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
"Connection: close\r\n\r\n";

// === FUNCIONES DE BLINK ===
void* blink_thread_func(void* arg) {
    int pin = *(int*)arg;
    bool* activo = (pin == LED_IZQUIERDA) ? &blink_izq_activo : &blink_der_activo;
    
    while(*activo) {
        digitalWrite(pin, HIGH);
        usleep(500000); // 500ms encendido
        if (!*activo) break;
        digitalWrite(pin, LOW);
        usleep(500000); // 500ms apagado
    }
    
    digitalWrite(pin, LOW);
    free(arg);
    return NULL;
}

void iniciar_blink(int pin) {
    pthread_t* thread = (pin == LED_IZQUIERDA) ? &thread_blink_izq : &thread_blink_der;
    bool* activo = (pin == LED_IZQUIERDA) ? &blink_izq_activo : &blink_der_activo;
    
    pthread_mutex_lock(&blink_mutex);
    
    if (*activo) {
        pthread_mutex_unlock(&blink_mutex);
        return; // Ya está parpadeando
    }
    
    *activo = true;
    int* pin_arg = malloc(sizeof(int));
    *pin_arg = pin;
    
    pthread_create(thread, NULL, blink_thread_func, pin_arg);
    pthread_detach(*thread);
    
    pthread_mutex_unlock(&blink_mutex);
}

void detener_blink(int pin) {
    bool* activo = (pin == LED_IZQUIERDA) ? &blink_izq_activo : &blink_der_activo;
    
    pthread_mutex_lock(&blink_mutex);
    *activo = false;
    pthread_mutex_unlock(&blink_mutex);
    
    usleep(100000); // Esperar un poco para que el hilo termine
    digitalWrite(pin, LOW);
}

// === INICIALIZACIÓN ===
void initialize_system() {
    printf("[INIT] Inicializando sistema...\n");
    
    // LEDs
    pinMode(LED_ADELANTE, OUTPUT);
    pinMode(LED_ATRAS, OUTPUT);
    pinMode(LED_IZQUIERDA, OUTPUT);
    pinMode(LED_DERECHA, OUTPUT);
    digitalWrite(LED_ADELANTE, LOW);
    digitalWrite(LED_ATRAS, LOW);
    digitalWrite(LED_IZQUIERDA, LOW);
    digitalWrite(LED_DERECHA, LOW);
    
    // Motores
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    
    // PWM
    softPwmInit(IN1, PWM_FREQ_MOTOR, 0.0f);
    softPwmInit(IN2, PWM_FREQ_MOTOR, 0.0f);
    softPwmInit(IN3, PWM_FREQ_MOTOR, 0.0f);
    softPwmInit(IN4, PWM_FREQ_MOTOR, 0.0f);
    
    printf("[INIT] Sistema inicializado correctamente\n");
}

// === FUNCIONES AUXILIARES ===
void send_error(int socket, int code, const char* message) {
    char response[512];
    snprintf(response, sizeof(response),
             "HTTP/1.1 %d Error\r\n"
             "Content-Type: application/json\r\n"
             "Access-Control-Allow-Origin: *\r\n\r\n"
             "{\"error\": \"%s\"}",
             code, message);
    write(socket, response, strlen(response));
}

int base64_decode(const char* input, char** output) {
    BIO *bio, *b64;
    size_t len = strlen(input);
    *output = (char*)malloc(len);
    if (!*output) return -1;

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new_mem_buf((void*)input, len);
    bio = BIO_push(b64, bio);

    int decoded_len = BIO_read(bio, *output, len);
    BIO_free_all(bio);

    if (decoded_len <= 0) {
        free(*output);
        *output = NULL;
        return -1;
    }
    (*output)[decoded_len] = '\0';
    return decoded_len;
}

bool validar_autenticacion(char* buffer) {
    char* auth_header = strstr(buffer, "Authorization: Basic ");
    if (!auth_header) return false;

    char* token_start = auth_header + strlen("Authorization: Basic ");
    char* token_end = strstr(token_start, "\r\n");
    if (!token_end) return false;

    size_t token_len = token_end - token_start;
    char* base64_token = malloc(token_len + 1);
    memcpy(base64_token, token_start, token_len);
    base64_token[token_len] = '\0';

    char* decoded_token = NULL;
    int decoded_len = base64_decode(base64_token, &decoded_token);
    free(base64_token);

    if (decoded_len <= 0 || !decoded_token) {
        if (decoded_token) free(decoded_token);
        return false;
    }

    char* colon = strchr(decoded_token, ':');
    if (!colon) {
        free(decoded_token);
        return false;
    }

    *colon = '\0';
    char* username = decoded_token;
    char* password = colon + 1;

    if (strcmp(username, "admin") != 0) {
        free(decoded_token);
        return false;
    }

    unsigned char hmac_result[32];
    HMAC(EVP_sha256(), "diositoayuda", 16,
         (unsigned char*)password, strlen(password),
         hmac_result, NULL);

    char hmac_hex[65];
    for (int i = 0; i < 32; i++) {
        sprintf(hmac_hex + (i*2), "%02x", hmac_result[i]);
    }
    hmac_hex[64] = '\0';

    bool valido = (strcmp(hmac_hex, STORED_HASH) == 0);
    free(decoded_token);
    return valido;
}

char* codificar_imagen(const char* directorio, size_t* encoded_len) {
    FILE* fp = fopen(directorio, "rb");
    if (!fp) return NULL;

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    unsigned char* image_data = malloc(file_size);
    if (!image_data) {
        fclose(fp);
        return NULL;
    }

    fread(image_data, 1, file_size, fp);
    fclose(fp);

    BIO *bio, *b64;
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_write(bio, image_data, file_size);
    BIO_flush(bio);
    
    memset(image_data, 0, file_size);
    free(image_data);

    BUF_MEM *bufferPtr;
    BIO_get_mem_ptr(bio, &bufferPtr);
    
    char* encoded = malloc(bufferPtr->length + 1);
    memcpy(encoded, bufferPtr->data, bufferPtr->length);
    encoded[bufferPtr->length] = '\0';
    *encoded_len = bufferPtr->length;

    BIO_free_all(bio);
    return encoded;
}

// === CONTROL DE MOTORES ===
void detener_motores() {
    printf("[MOTOR] Deteniendo motores\n");
    softPwmSetDutyCycle(IN1, 0.0f);
    softPwmSetDutyCycle(IN2, 0.0f);
    softPwmSetDutyCycle(IN3, 0.0f);
    softPwmSetDutyCycle(IN4, 0.0f);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    
    digitalWrite(LED_ADELANTE, LOW);
    digitalWrite(LED_ATRAS, LOW);
    
    // Detener blinks de luces direccionales
    detener_blink(LED_IZQUIERDA);
    detener_blink(LED_DERECHA);
}

void mover_carro(const char* direccion, float velocidad) {
    printf("[MOTOR] Moviendo: %s (velocidad: %.0f%%)\n", direccion, velocidad);
    
    // Guardar dirección anterior
    pthread_mutex_lock(&estado_mutex);
    char direccion_anterior[32];
    strncpy(direccion_anterior, estado_carro.direccion, sizeof(direccion_anterior));
    snprintf(estado_carro.direccion, sizeof(estado_carro.direccion), "%s", direccion);
    estado_carro.velocidad_actual = velocidad;
    pthread_mutex_unlock(&estado_mutex);
    
    // Si la dirección es la misma, solo actualizar velocidad
    bool misma_direccion = (strcmp(direccion, direccion_anterior) == 0);
    
    if (!misma_direccion) {
        detener_motores();
    }
    
    if (strcmp(direccion, "adelante") == 0) {
        digitalWrite(IN2, LOW);
        softPwmSetDutyCycle(IN1, velocidad);
        if (!misma_direccion) digitalWrite(LED_ADELANTE, HIGH);
    }
    else if (strcmp(direccion, "atras") == 0) {
        digitalWrite(IN1, LOW);
        softPwmSetDutyCycle(IN2, velocidad);
        if (!misma_direccion) digitalWrite(LED_ATRAS, HIGH);
    }
    else if (strcmp(direccion, "adelante_izquierda") == 0) {
        digitalWrite(IN2, LOW);
        softPwmSetDutyCycle(IN1, velocidad);
        digitalWrite(IN4, LOW);
        softPwmSetDutyCycle(IN3, 85.0f);
        if (!misma_direccion) {
            digitalWrite(LED_ADELANTE, HIGH);
            iniciar_blink(LED_IZQUIERDA);
        }
    }
    else if (strcmp(direccion, "adelante_derecha") == 0) {
        digitalWrite(IN2, LOW);
        softPwmSetDutyCycle(IN1, velocidad);
        digitalWrite(IN3, LOW);
        softPwmSetDutyCycle(IN4, 85.0f);
        if (!misma_direccion) {
            digitalWrite(LED_ADELANTE, HIGH);
            iniciar_blink(LED_DERECHA);
        }
    }
    else if (strcmp(direccion, "atras_izquierda") == 0) {
        digitalWrite(IN1, LOW);
        softPwmSetDutyCycle(IN2, velocidad);
        digitalWrite(IN4, LOW);
        softPwmSetDutyCycle(IN3, 85.0f);
        if (!misma_direccion) {
            digitalWrite(LED_ATRAS, HIGH);
            iniciar_blink(LED_IZQUIERDA);
        }
    }
    else if (strcmp(direccion, "atras_derecha") == 0) {
        digitalWrite(IN1, LOW);
        softPwmSetDutyCycle(IN2, velocidad);
        digitalWrite(IN3, LOW);
        softPwmSetDutyCycle(IN4, 85.0f);
        if (!misma_direccion) {
            digitalWrite(LED_ATRAS, HIGH);
            iniciar_blink(LED_DERECHA);
        }
    }
    else if (strcmp(direccion, "detener") == 0) {
        detener_motores();
        pthread_mutex_lock(&estado_mutex);
        snprintf(estado_carro.direccion, sizeof(estado_carro.direccion), "Detenido");
        pthread_mutex_unlock(&estado_mutex);
    }
}

void controlar_luces(const char* tipo) {
    printf("[LUCES] Controlando: %s\n", tipo);
    
    pthread_mutex_lock(&estado_mutex);
    
    if (strcmp(tipo, "delantera") == 0) {
        int estado_actual = digitalRead(LED_ADELANTE);
        digitalWrite(LED_ADELANTE, !estado_actual);
        snprintf(estado_carro.luces, sizeof(estado_carro.luces), 
                 !estado_actual ? "Delantera ON" : "Apagadas");
    }
    else if (strcmp(tipo, "traseras") == 0) {
        int estado_actual = digitalRead(LED_ATRAS);
        digitalWrite(LED_ATRAS, !estado_actual);
        snprintf(estado_carro.luces, sizeof(estado_carro.luces), 
                 !estado_actual ? "Traseras ON" : "Apagadas");
    }
    else if (strcmp(tipo, "izquierda") == 0) {
        if (blink_izq_activo) {
            detener_blink(LED_IZQUIERDA);
            snprintf(estado_carro.luces, sizeof(estado_carro.luces), "Apagadas");
        } else {
            iniciar_blink(LED_IZQUIERDA);
            snprintf(estado_carro.luces, sizeof(estado_carro.luces), "Izquierda BLINK");
        }
    }
    else if (strcmp(tipo, "derecha") == 0) {
        if (blink_der_activo) {
            detener_blink(LED_DERECHA);
            snprintf(estado_carro.luces, sizeof(estado_carro.luces), "Apagadas");
        } else {
            iniciar_blink(LED_DERECHA);
            snprintf(estado_carro.luces, sizeof(estado_carro.luces), "Derecha BLINK");
        }
    }
    
    pthread_mutex_unlock(&estado_mutex);
}

int tomarFoto(const char *camara) {
    const char *device = (strcmp(camara, "frontal") == 0) ? device_frontal : device_jardin;
    
    printf("[FOTO] Capturando desde cámara: %s (dispositivo: %s)\n", camara, device);
    
    char command[1000];
    snprintf(command, sizeof(command), 
             "fswebcam -r %s -d %s --no-banner %s 2>/dev/null", 
             resolution, device, imageName);
    
    int result = system(command);
    
    if (result == 0) {
        printf("[FOTO] Captura exitosa\n");
    } else {
        printf("[FOTO] Error en captura (código: %d)\n", result);
    }
    
    return result;
}

// === MANEJADOR DE CLIENTES ===
void* manejar_cliente(void* socket_ptr) {
    int socket = *(int*)socket_ptr;
    char buffer[BUFFER_SIZE];
    read(socket, buffer, BUFFER_SIZE);

    char metodo[16], ruta[256];
    sscanf(buffer, "%s %s", metodo, ruta);
    
    char* query_start = strchr(ruta, '?');
    char query_params[256] = "";
    if (query_start) {
        strncpy(query_params, query_start + 1, sizeof(query_params) - 1);
        *query_start = '\0';
    }

    printf("\n[REQUEST] %s %s%s%s\n", metodo, ruta, 
           query_params[0] ? "?" : "", query_params);

    if (strcmp(metodo, "OPTIONS") == 0) {
        printf("[CORS] Respondiendo preflight\n");
        write(socket, headers, strlen(headers));
        close(socket);
        free(socket_ptr);
        return NULL;
    }

    // AUTENTICACIÓN
    if (!validar_autenticacion(buffer)) {
        printf("[AUTH] Acceso denegado\n");
        send_error(socket, 401, "Acceso no autorizado");
        close(socket);
        free(socket_ptr);
        return NULL;
    }

    // === ENDPOINT: /mover ===
    if(strcmp(ruta, "/mover") == 0) {
        char* cuerpo = strstr(buffer, "\r\n\r\n");
        if (!cuerpo) {
            send_error(socket, 400, "Solicitud sin cuerpo");
            close(socket);
            free(socket_ptr);
            return NULL;
        }
        cuerpo += 4;

        char direccion[32] = {0};
        float velocidad = 60.0;
        
        sscanf(cuerpo, "{\"direccion\":\"%31[^\"]\",\"velocidad\":%f", direccion, &velocidad);
        
        printf("[MOVER] Dirección: %s, Velocidad: %.0f%%\n", direccion, velocidad);
        
        mover_carro(direccion, velocidad);
        
        pthread_mutex_lock(&estado_mutex);
        char respuesta[512];
        snprintf(respuesta, sizeof(respuesta),
                 "%s{\"mensaje\":\"Movimiento ejecutado\",\"direccion\":\"%s\",\"velocidad\":%.0f}",
                 headers, estado_carro.direccion, estado_carro.velocidad_actual);
        pthread_mutex_unlock(&estado_mutex);
        
        write(socket, respuesta, strlen(respuesta));
    }
    // === ENDPOINT: /luz ===
    else if(strcmp(ruta, "/luz") == 0) {
        char* cuerpo = strstr(buffer, "\r\n\r\n");
        if (!cuerpo) {
            send_error(socket, 400, "Solicitud sin cuerpo");
            close(socket);
            free(socket_ptr);
            return NULL;
        }
        cuerpo += 4;

        char tipo[32] = {0};
        sscanf(cuerpo, "{\"tipo\":\"%31[^\"]", tipo);
        
        printf("[LUZ] Tipo: %s\n", tipo);
        
        controlar_luces(tipo);
        
        pthread_mutex_lock(&estado_mutex);
        char respuesta[512];
        snprintf(respuesta, sizeof(respuesta),
                 "%s{\"mensaje\":\"Luz controlada\",\"luces\":\"%s\"}",
                 headers, estado_carro.luces);
        pthread_mutex_unlock(&estado_mutex);
        
        write(socket, respuesta, strlen(respuesta));
    }
    // === ENDPOINT: /tomar_foto ===
    else if(strcmp(ruta, "/tomar_foto") == 0) {
        char camara[32] = "jardin";
        
        if (query_params[0]) {
            sscanf(query_params, "camara=%31s", camara);
        }
        
        printf("[FOTO] Solicitada cámara: %s\n", camara);
        
        int resultado = tomarFoto(camara);
        
        if (resultado != 0) {
            printf("[FOTO] Error al capturar\n");
            send_error(socket, 500, "Error al capturar imagen");
            close(socket);
            free(socket_ptr);
            return NULL;
        }
        
        size_t encoded_len;
        char* encoded_data = codificar_imagen(imageName, &encoded_len);
        
        if (!encoded_data) {
            printf("[FOTO] Error al codificar\n");
            send_error(socket, 500, "Error al codificar imagen");
            close(socket);
            free(socket_ptr);
            return NULL;
        }
        
        printf("[FOTO] Enviando imagen (%zu bytes en base64)\n", encoded_len);

        char header[512];
        snprintf(header, sizeof(header),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Length: %zu\r\n\r\n",
            encoded_len);

        write(socket, header, strlen(header));
        write(socket, encoded_data, encoded_len);

        memset(encoded_data, 0, encoded_len);
        free(encoded_data);
    }
    // === ENDPOINT: /detener ===
    else if(strcmp(ruta, "/detener") == 0) {
        printf("[DETENER] Deteniendo carro\n");
        
        detener_motores();
        
        pthread_mutex_lock(&estado_mutex);
        snprintf(estado_carro.direccion, sizeof(estado_carro.direccion), "Detenido");
        estado_carro.velocidad_actual = 0.0;
        pthread_mutex_unlock(&estado_mutex);
        
        char respuesta[512];
        snprintf(respuesta, sizeof(respuesta),
                 "%s{\"mensaje\":\"Carro detenido\",\"direccion\":\"Detenido\",\"velocidad\":0}",
                 headers);
        
        write(socket, respuesta, strlen(respuesta));
    }
    // === RUTA NO ENCONTRADA ===
    else {
        printf("[404] Ruta no encontrada: %s\n", ruta);
        send_error(socket, 404, "Ruta no encontrada");
    }

    close(socket);
    free(socket_ptr);
    return NULL;
}

// === MAIN ===
int main() {
    printf("╔════════════════════════════════════════╗\n");
    printf("║  SERVIDOR CONTROL DE CARRITO RC       ║\n");
    printf("╚════════════════════════════════════════╝\n\n");
    
    initialize_system();
    
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if(bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if(listen(server_fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("[SERVER] Escuchando en puerto %d...\n\n", PORT);

    while(1) {
        int* new_socket = malloc(sizeof(int));
        *new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);

        if (*new_socket < 0) {
            perror("accept");
            free(new_socket);
            continue;
        }

        pthread_t hilo;
        pthread_create(&hilo, NULL, manejar_cliente, new_socket);
        pthread_detach(hilo);
    }

    return 0;
}
