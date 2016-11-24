#ifndef SERVER
#define SERVER

// Type Definition Structures
struct ltc2983_packet
{
    unsigned int bytes;
    unsigned char *data;
};

// Functions
void error(const char *msg);
int sock_server(unsigned int port);

#endif
