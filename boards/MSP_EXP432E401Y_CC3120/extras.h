#if 0
/* Flag values for getaddrinfo() */
#define AI_PASSIVE     0x00000001
#define AI_NUMERICHOST 0x00000004
#define AI_NUMERICSERV 0x00000400
struct addrinfo {
    int              ai_flags;
    int              ai_family;
    int              ai_socktype;
    int              ai_protocol;
    size_t           ai_addrlen;
    struct sockaddr *ai_addr;
    char            *ai_canonname;
    struct addrinfo *ai_next;
};

#define AF_UNSPEC 0
#endif

#ifndef EAI_ADDRFAMILY
#define EAI_ADDRFAMILY -9
#endif

#ifndef EAI_NONAME
#define EAI_NONAME -11
#endif

#ifndef EAI_SOCKTYPE
#define EAI_SOCKTYPE -12
#endif

#ifndef EAI_FAMILY
#define EAI_FAMILY -13
#endif

#ifndef EAI_SYSTEM
#define EAI_SYSTEM -14
#endif

#ifndef EAI_MEMORY
#define EAI_MEMORY -15
#endif

#if 0
extern int         getaddrinfo(const char *node, const char *service,
                       const struct addrinfo *hints, struct addrinfo **res);
extern void        freeaddrinfo(struct addrinfo *ai);

#define AF_UNIX 99
#define MSG_DONTROUTE 199
#define SO_ERROR 299

#define SO_LINGER SL_SO_LINGER

extern int inet_pton(int af, const char *src, void *dst);
extern char *inet_ntop(int af, const void *src, char *buf, size_t size);

struct sockaddr_storage {
    uint16_t sa_family;
    uint8_t pad[24];
};
#endif
