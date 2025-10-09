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
