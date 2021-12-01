#ifndef __SHARED_CONTEXT_H__

#define __SHARED_CONTEXT_H__

// #include <stdbool.h>
// #include <stdint.h>
// #include <string.h>

#include "cx.h"
#include "ethUstream.h"
#include "tokens.h"

#define WEI_TO_ETHER 18

typedef enum { APP_STATE_IDLE, APP_STATE_SIGNING_TX, APP_STATE_SIGNING_MESSAGE } app_state_t;

#define NETWORK_STRING_MAX_SIZE 16

typedef struct txStringProperties_t {
    char fullAddress[43];
    char fullAmount[79];  // 2^256 is 78 digits long
    char maxFee[50];
    char nonce[8];  // 10M tx per account ought to be enough for everybody
    char network_name[NETWORK_STRING_MAX_SIZE];
} txStringProperties_t;

#define SHARED_CTX_FIELD_1_SIZE 100
#define SHARED_CTX_FIELD_2_SIZE 40

typedef struct strDataTmp_t {
    char tmp[SHARED_CTX_FIELD_1_SIZE];
    char tmp2[SHARED_CTX_FIELD_2_SIZE];
} strDataTmp_t;

typedef union {
    txStringProperties_t common;
    strDataTmp_t tmp;
} strings_t;

// extern chain_config_t *chainConfig;

// extern tmpContent_t tmpContent;
// extern dataContext_t dataContext;
extern strings_t strings;
extern cx_sha3_t global_sha3;

extern uint8_t appState;

void reset_app_context(void);

#endif  // __SHARED_CONTEXT_H__
