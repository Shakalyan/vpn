#include "RSA.h"
#include <openssl/pem.h>
#include <openssl/err.h>
#include <stddef.h>
#include <errno.h>

#define TRUE 1
#define FALSE 0
typedef char bool;

static char RSA_err[256];

const char* RSA_get_err() 
{
    return RSA_err;
}

#define FREE goto Error

#define WERR_FREE(TEMPLATE, ...) {          \
    sprintf(RSA_err, TEMPLATE, __VA_ARGS__);\
    FREE;                                   \
}

#define WERR_RET(RET, TEMPLATE, ...) {      \
    sprintf(RSA_err, TEMPLATE, __VA_ARGS__);\
    return RET;                             \
}

#define WERRS_FREE(MSG) {       \
    sprintf(RSA_err, "%s", MSG);\
    FREE;                       \
}

#define GET_OSSL_ERROR ERR_error_string(ERR_peek_error(), NULL)

#define RSA_BITS 4096
#define ENC_MAX_SIZE 512
#define MAX_FILE_PATH_LEN 4096

static const char pub_key_path[] = "public.key";
static const char pri_key_path[] = "private.key";

bool RSA_generate_keys(char* dir_path) 
{
    FILE *pub_f = NULL, *pri_f = NULL;
    EVP_PKEY* key_pair = NULL;
    char* path = NULL;

    size_t dir_path_len = strlen(dir_path);
    if (dir_path_len == 0)
        WERR_FREE("Dir path is empty%s", "");
    
    path = malloc(MAX_FILE_PATH_LEN);
    memcpy(path, dir_path, dir_path_len);
    if (dir_path[dir_path_len-1] != '/') {
        path[dir_path_len] = '/';
        ++dir_path_len;
    }

    key_pair = EVP_RSA_gen(RSA_BITS);
    if (!key_pair)
        WERR_FREE("%s", ERR_error_string(ERR_peek_error(), NULL));

    memcpy(path + dir_path_len, pub_key_path, sizeof(pub_key_path));
    pub_f = fopen(path, "w");
    if (!pub_f)
        WERR_FREE("Cannot write to file '%s'", path);

    if (!PEM_write_PUBKEY(pub_f, key_pair))
        WERR_FREE("%s", ERR_error_string(ERR_peek_error(), NULL));

    memcpy(path + dir_path_len, pri_key_path, sizeof(pri_key_path));
    pri_f = fopen(path, "w");
    if (!pri_f)
        WERR_FREE("Cannot write to file '%s'", path);

    if (!PEM_write_PrivateKey(pri_f, key_pair, NULL, NULL, 0, NULL, NULL))
        WERR_FREE("%s", ERR_error_string(ERR_peek_error(), NULL));

    EVP_PKEY_free(key_pair);
    fclose(pub_f);
    fclose(pri_f);
    free(path); 
    return TRUE;

Error:
    if (key_pair) EVP_PKEY_free(key_pair);
    if (pub_f) fclose(pub_f);
    if (pri_f) fclose(pri_f);
    if (path) free(path);
    return FALSE;
}

bool RSA_encrypt(unsigned char* msg, size_t msg_size, unsigned char* dst, EVP_PKEY_CTX* pubkey)
{
    size_t encrypted_size = ENC_MAX_SIZE;
    if (EVP_PKEY_encrypt(pubkey, dst, &encrypted_size, msg, msg_size) != 1)
        WERR_RET(FALSE, "Failed to encrypt: %s", GET_OSSL_ERROR);
    return TRUE;
}

int RSA_decrypt(unsigned char* encrypted, size_t encrypted_size, unsigned char* dst, EVP_PKEY_CTX* prikey)
{
    size_t decrypted_size = ENC_MAX_SIZE;
    if (EVP_PKEY_decrypt(prikey, dst, &decrypted_size, encrypted, ENC_MAX_SIZE) != 1)
        WERR_RET(-1, "%s\n", ERR_error_string(ERR_peek_error(), NULL));
    return decrypted_size;
}

EVP_PKEY_CTX* RSA_read_pubkey(char* path) 
{
    FILE* file = NULL;
    EVP_PKEY* key = NULL;
    EVP_PKEY_CTX* ctx = NULL;
    
    file = fopen(path, "r");
    if (!file)
        WERR_FREE("Cannot read file '%s': %s\n", path, strerror(errno));
    
    key = PEM_read_PUBKEY(file, NULL, NULL, NULL);
    if (!key)
        WERR_FREE("Failed to read public key from '%s': %s\n", path, ERR_error_string(ERR_peek_error(), NULL));
    
    ctx = EVP_PKEY_CTX_new(key, NULL);
    if (!ctx)
        WERR_FREE("Failed to create ctx from public key: %s\n", ERR_error_string(ERR_peek_error(), NULL));
    
    if (EVP_PKEY_encrypt_init(ctx) != 1)
        WERR_FREE("Failed to init ecnryption: %s\n", ERR_error_string(ERR_peek_error(), NULL));
    
    fclose(file);
    EVP_PKEY_free(key);
    return ctx;

Error:
    if (file) fclose(file);
    if (key) EVP_PKEY_free(key);
    if (ctx) EVP_PKEY_CTX_free(ctx);
    return NULL;
}

EVP_PKEY_CTX* RSA_read_prikey(char* path) 
{
    FILE* file = NULL;
    EVP_PKEY* key = NULL;
    EVP_PKEY_CTX* ctx = NULL;
    
    file = fopen(path, "r");
    if (!file)
        WERR_FREE("Cannot read file '%s': %s\n", path, strerror(errno));
    
    key = PEM_read_PrivateKey(file, NULL, NULL, NULL);
    if (!key)
        WERR_FREE("Failed to read private key from '%s': %s\n", path, ERR_error_string(ERR_peek_error(), NULL));
    
    ctx = EVP_PKEY_CTX_new(key, NULL);
    if (!ctx)
        WERR_FREE("Failed to create ctx from private key: %s\n", ERR_error_string(ERR_peek_error(), NULL));
    
    if (EVP_PKEY_decrypt_init(ctx) != 1)
        WERR_FREE("Failed to init ecnryption: %s\n", ERR_error_string(ERR_peek_error(), NULL));
    
    fclose(file);
    EVP_PKEY_free(key);
    return ctx;

Error:
    if (file) fclose(file);
    if (key) EVP_PKEY_free(key);
    if (ctx) EVP_PKEY_CTX_free(ctx);
    return NULL;
}