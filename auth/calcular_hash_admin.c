#include <stdio.h>
#include <string.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>

int main() {
    const char* password = "admin";
    const char* secret_key = "diositoayuda";
    
    printf("Calculando hash para contraseña: '%s'\n", password);
    printf("Con secret key: '%s'\n\n", secret_key);
    
    // Calcular HMAC-SHA256
    unsigned char hmac_result[32];
    HMAC(
        EVP_sha256(),
        secret_key, strlen(secret_key),
        (unsigned char*)password, strlen(password),
        hmac_result, NULL
    );
    
    // Convertir a hexadecimal
    char hmac_hex[65];
    for (int i = 0; i < 32; i++) {
        sprintf(hmac_hex + (i*2), "%02x", hmac_result[i]);
    }
    hmac_hex[64] = '\0';
    
    printf("====================================================\n");
    printf("HASH GENERADO (Hexadecimal):\n");
    printf("%s\n", hmac_hex);
    printf("\n====================================================\n");
    printf("Copia esta línea en tu servidor.c:\n\n");
    printf("const char* STORED_HASH = \"%s\";\n", hmac_hex);
    printf("====================================================\n");
    
    // Verificación contra el hash actual
    const char* hash_actual = "623fd75beab1b12972f837f5577aaf1ab87ca0df094151a8d8ec8b1fe0c94dc0";
    
    if (strcmp(hmac_hex, hash_actual) == 0) {
        printf("\n✓ ¡COINCIDE! La contraseña 'admin' es correcta.\n");
    } else {
        printf("\n✗ NO coincide con el hash actual del servidor.\n");
        printf("Hash actual del servidor:\n%s\n", hash_actual);
    }
    
    return 0;
}
