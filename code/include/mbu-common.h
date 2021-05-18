#ifndef MBU_COMMON_H
#define MBU_COMMON_H

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "modbus.h"

typedef enum {
    None,
    Tcp,
    Rtu
} ConnType;

int getInt(const char str[], int *ok);

typedef struct {
    ConnType type;

    void (*del)(void *backend);

    //common client/server functions
    int (*setParam)(void *backend, char c, char *value);
    modbus_t *(*createCtxt)(void *backend);

    //server functions
    int (*listenForConnection)(void *backend, modbus_t *ctx);
    void (*closeConnection)(void *backend);
} BackendParams;

typedef struct {
    BackendParams base;
    char devName[32];
    int baud;
    int dataBits;
    int stopBits;
    char parity;
} RtuBackend;

int setRtuParam(void *backend, char c, char *value);

modbus_t *createRtuCtxt(void *backend);

void delRtu(void *backend);

int listenForRtuConnection(void *backend, modbus_t *ctx);
void closeRtuConnection(void *backend);

BackendParams *createRtuBackend(int baud, int dataBits, int stopBits, char parity);

typedef struct {
    BackendParams base;
    char ip[32];
    int port;

    int clientSocket;
} TcpBackend;

int setTcpParam(void* backend, char c, char *value);

modbus_t *createTcpCtxt(void *backend);

void delTcp(void *backend);

int listenForTcpConnection(void *backend, modbus_t *ctx);

void closeTcpConnection(void *backend);

BackendParams *createTcpBackend(const char* ipstr, int port);

#endif //MBU_COMMON_H
