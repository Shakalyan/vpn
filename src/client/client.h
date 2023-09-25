#ifndef CLIENT_GUARD
#define CLIENT_GUARD

int start_client(const char* tun_name, const char* tun_addr, const char* tun_mask, const char* server_addr_str, int server_port, const int MTU);

#endif