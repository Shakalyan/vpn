#ifndef RSA_GUARD
#define RSA_GUARD

#include <openssl/evp.h>

const char* RSA_get_err();

char RSA_generate_keys(char* dir_path);

char RSA_encrypt(unsigned char* msg, size_t msg_size, unsigned char* dst, EVP_PKEY_CTX* pubkey);

int RSA_decrypt(unsigned char* encrypted, size_t encrypted_size, unsigned char* dst, EVP_PKEY_CTX* prikey);

EVP_PKEY_CTX* RSA_read_pubkey(char* path);

EVP_PKEY_CTX* RSA_read_prikey(char* path);

#endif