/*******************************************************************************
 *   Ledger Ethereum App
 *   (c) 2016-2019 Ledger
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 ********************************************************************************/

#ifndef _ETHUSTREAM_H_
#define _ETHUSTREAM_H_

#include <stdbool.h>
#include <stdint.h>

#include "os.h"
#include "cx.h"

struct txContext_t;

typedef enum customStatus_e {
    CUSTOM_NOT_HANDLED,
    CUSTOM_HANDLED,
    CUSTOM_SUSPENDED,
    CUSTOM_FAULT
} customStatus_e;

typedef customStatus_e (*ustreamProcess_t)(struct txContext_t *context);

#define TX_FLAG_TYPE   0x01
#define ADDRESS_LENGTH 20
#define SELECTOR_LENGTH 4
#define INT256_LENGTH  32

// First variant of every Tx enum.
#define RLP_NONE 0

typedef enum rlpConfluxTxField_e {
    CONFLUX_RLP_NONE = RLP_NONE,
    CONFLUX_RLP_CONTENT,
    CONFLUX_RLP_NONCE,
    CONFLUX_RLP_GASPRICE,
    CONFLUX_RLP_GASLIMIT,
    CONFLUX_RLP_TO,
    CONFLUX_RLP_VALUE,
    CONFLUX_RLP_STORAGE_LIMIT,
    CONFLUX_RLP_EPOCH_HEIGHT,
    CONFLUX_RLP_CHAIN_ID,
    CONFLUX_RLP_DATA,
    CONFLUX_RLP_V,
    CONFLUX_RLP_R,
    CONFLUX_RLP_S,
    CONFLUX_RLP_DONE
} rlpConfluxTxField_e;

typedef enum parserStatus_e {
    USTREAM_PROCESSING,  // Parsing is in progress
    USTREAM_FINISHED,    // Parsing is done
    USTREAM_FAULT,       // An error was encountered while parsing
    USTREAM_CONTINUE     // Used internally to signify we can keep on parsing
} parserStatus_e;

typedef struct txInt256_t {
    uint8_t value[INT256_LENGTH];
    uint8_t length;
} txInt256_t;

typedef struct txContent_t {
    txInt256_t gasprice;
    txInt256_t gaslimit;
    txInt256_t value;
    txInt256_t nonce;
    txInt256_t chainid;
    txInt256_t storagelimit;
    txInt256_t epochheight;
    uint8_t selector[SELECTOR_LENGTH];
    uint8_t data_length;
    uint8_t destination[ADDRESS_LENGTH];
    uint8_t destinationLength;
    uint8_t v[8];
    uint8_t vLength;
    bool data_present;
} txContent_t;

typedef struct txContext_t {
    uint8_t currentField;
    cx_sha3_t *sha3;
    uint32_t currentFieldLength;
    uint32_t currentFieldPos;
    bool currentFieldIsList;
    bool processingField;
    bool fieldSingleByte;
    uint32_t dataLength;
    uint8_t rlpBuffer[5];
    uint32_t rlpBufferPos;
    const uint8_t *workBuffer;
    uint32_t commandLength;
    txContent_t *content;
} txContext_t;

void init_parser(txContext_t *context,
            cx_sha3_t *sha3,
            txContent_t *content);
parserStatus_e process_tx_chunk(txContext_t *context,
                         const uint8_t *buffer,
                         uint32_t length);
parserStatus_e continueTx(txContext_t *context);
void copyTxData(txContext_t *context, uint8_t *out, uint32_t length);
uint8_t readTxByte(txContext_t *context);

#endif /* _ETHUSTREAM_H_ */