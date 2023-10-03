#include "AES.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#define TRUE 1
#define FALSE 0
typedef char bool;

static char AES_err[256];

const char* AES_get_err() 
{
    return AES_err;
}

#define FREE goto Error

#define WERR_FREE(TEMPLATE, ...) {          \
    sprintf(AES_err, TEMPLATE, __VA_ARGS__);\
    FREE;                                   \
}

#define WERR_RET(RET, TEMPLATE, ...) {      \
    sprintf(AES_err, TEMPLATE, __VA_ARGS__);\
    return RET;                             \
}

#define WERRS_FREE(MSG) {       \
    sprintf(AES_err, "%s", MSG);\
    FREE;                       \
}

#define GET_OSSL_ERROR ERR_error_string(ERR_peek_error(), NULL)

#define AES_MODE "AES-256-CBC"
#define AES_KEY_SIZE 32
#define AES_IV_SIZE 16 

static EVP_CIPHER* cipher = NULL;
static EVP_CIPHER_CTX* ctx = NULL;

bool AES_init()
{
    cipher = NULL;
    ctx = NULL;

    cipher = EVP_CIPHER_fetch(NULL, AES_MODE, NULL);
    if (!cipher)
        WERR_FREE("Failed to fetch cipher: %s", GET_OSSL_ERROR);
    
    ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
        WERR_FREE("Failed to allocate context: %s", GET_OSSL_ERROR);
    
    return TRUE;

Error:
    if (cipher) EVP_CIPHER_free(cipher);
    if (ctx) EVP_CIPHER_CTX_free(ctx);
    return FALSE;
}

void AES_free() 
{
    if (cipher) EVP_CIPHER_free(cipher);
    if (ctx) EVP_CIPHER_CTX_free(ctx);
}

bool AES_generate_key(unsigned char* dst) 
{
    if (RAND_priv_bytes(dst, AES_KEY_SIZE) != 1)
        WERR_RET(FALSE, "Failed to generate key: %s", GET_OSSL_ERROR);
    return TRUE;
}

bool AES_generate_ivec(unsigned char* dst) 
{
    if (RAND_bytes(dst, AES_IV_SIZE) != 1)
        WERR_RET(FALSE, "Failed to generate IV: %s", GET_OSSL_ERROR);
    return TRUE;
}

int AES_encrypt(unsigned char* msg, int msg_size, unsigned char* dst, unsigned char* key, unsigned char* ivec)
{
    if (EVP_EncryptInit_ex2(ctx, cipher, key, ivec, NULL) != 1)
        WERR_RET(-1, "Failed to init encryption: %s", GET_OSSL_ERROR);

    int encrypted_size = 0, len;
    if (EVP_EncryptUpdate(ctx, dst, &len, msg, msg_size) != 1)
        WERR_RET(-1, "Failed to encrypt: %s", GET_OSSL_ERROR);
    
    encrypted_size += len;
    if (EVP_EncryptFinal_ex(ctx, dst + encrypted_size, &len) != 1)
        WERR_RET(-1, "Failed to encrypt: %s", GET_OSSL_ERROR);

    return encrypted_size + len;
}

int AES_decrypt(unsigned char* encrypted, int encrypted_size, unsigned char* dst, unsigned char* key, unsigned char* ivec)
{
    if (EVP_DecryptInit_ex2(ctx, cipher, key, ivec, NULL) != 1)
        WERR_RET(-1, "Failed to init decryption: %s", GET_OSSL_ERROR);
    
    int decrypted_size = 0, len;
    if (EVP_DecryptUpdate(ctx, dst, &len, encrypted, encrypted_size) != 1)
        WERR_RET(-1, "Failed to decrypt: %s", GET_OSSL_ERROR);
    
    decrypted_size += len;
    if (EVP_DecryptFinal_ex(ctx, dst + decrypted_size, &len) != 1)
        WERR_RET(-1, "Failed to decrypt: %s", GET_OSSL_ERROR);
    
    return decrypted_size + len;
}