#ifndef SERVER_GUARD
#define SERVER_GUARD

int start_server(const char* tun_name, const char* addr_str, const char* mask_str, int port, const int MTU);

#endif