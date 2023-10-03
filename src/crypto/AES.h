#ifndef AES_GUARD
#define AES_GUARD

const char* AES_get_err();

char AES_init();

void AES_free();

char AES_generate_key(unsigned char* dst);

char AES_generate_ivec(unsigned char* dst);

int AES_encrypt(unsigned char* msg, int msg_size, unsigned char* dst, unsigned char* key, unsigned char* ivec);

int AES_decrypt(unsigned char* encrypted, int encrypted_size, unsigned char* dst, unsigned char* key, unsigned char* ivec);

#endif