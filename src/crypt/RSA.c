#include "RSA.h"

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <stddef.h>

char crypt_err[256];

#define FREE goto Error

#define WERR_RET(RET, TEMPLATE, ...) {          \
    sprintf(crypt_err, TEMPLATE, __VA_ARGS__);  \
    return RET;                                 \
}

#define WERR_FREE(TEMPLATE, ...) {              \
    sprintf(crypt_err, TEMPLATE, __VA_ARGS__);  \
    FREE;                                       \
}

const char* get_crypt_err() {
    return crypt_err;
}

#define TRUE 1
#define FALSE 0
typedef char bool;

#define BITS 2048
#define MAX_FILE_PATH_LEN 4096
static const char pub_key_path[] = "public.key";
static const char pri_key_path[] = "private.key";

bool generate_keys(char* dir_path) 
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

    key_pair = EVP_RSA_gen(BITS);
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

// char* encrypt(const char* message, size_t message_size, size_t* encrypted_size) 
// {
//     FILE* pkey_f = fopen("public.key", "r");
//     if (!pkey_f) {
//         printf("Read public key error\n");
//         return NULL;
//     }
//     EVP_PKEY* pkey = PEM_read_PUBKEY(pkey_f, NULL, NULL, NULL);
//     fclose(pkey_f);

//     EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, NULL);
//     if (!ctx) {
//         printf("Create ctx error\n");
//         return NULL;
//     }
//     EVP_PKEY_encrypt_init(ctx);

//     if (!EVP_PKEY_encrypt(ctx, NULL, encrypted_size, (unsigned char*)message, message_size)) {
//         printf("Encryption error\n");
//         return NULL;
//     }
//     char* encrypted = malloc(*encrypted_size);
//     EVP_PKEY_encrypt(ctx, (unsigned char*)encrypted, encrypted_size, (unsigned char*)message, message_size);
    
//     EVP_PKEY_free(pkey);
//     EVP_PKEY_CTX_free(ctx);

//     return encrypted;
// }

// char* decrypt(const char* encrypted, size_t encrypted_size, size_t* decrypted_size) 
// {
//     FILE* pkey_f = fopen("private.key", "r");
//     if (!pkey_f) {
//         printf("Read private key error\n");
//         return NULL;
//     }
//     EVP_PKEY* pkey = PEM_read_PrivateKey(pkey_f, NULL, NULL, NULL);
//     fclose(pkey_f);

//     EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, NULL);
//     if (!ctx) {
//         printf("Create ctx error\n");
//         return NULL;
//     }
//     EVP_PKEY_decrypt_init(ctx);

//     if (!EVP_PKEY_decrypt(ctx, NULL, decrypted_size, (unsigned char*)encrypted, encrypted_size)) {
//         printf("Decryption error\n");
//         return NULL;
//     }
//     char* decrypted = malloc(*decrypted_size);
//     EVP_PKEY_decrypt(ctx, (unsigned char*)decrypted, decrypted_size, (unsigned char*)encrypted, encrypted_size);

//     EVP_PKEY_free(pkey);
//     EVP_PKEY_CTX_free(ctx);

//     return decrypted;
// }