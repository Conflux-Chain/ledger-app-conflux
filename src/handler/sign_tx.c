/*****************************************************************************
 *   Ledger App Boilerplate.
 *   (c) 2020 Ledger SAS.
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
 *****************************************************************************/

#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool
#include <stddef.h>   // size_t
#include <string.h>   // memset, explicit_bzero

#include "os.h"
#include "cx.h"

#include "sign_tx.h"
#include "../sw.h"
#include "../globals.h"
#include "../crypto.h"
#include "../ui/display.h"
#include "../common/buffer.h"
#include "../transaction/types.h"
#include "../transaction/deserialize.h"
#include "../types.h"
#include "apdu_constants.h"
#include "eth/utils.h"

void debug_print_tx(txContent_t* tx) {
    PRINTF("Nonce %.*H\n", tx->nonce.length, tx->nonce.value);
    PRINTF("Gas price %.*H\n", tx->gasprice.length, tx->gasprice.value);
    PRINTF("Gas limit %.*H\n", tx->gaslimit.length, tx->gaslimit.value);
    PRINTF("Destination: %.*H\n", ADDRESS_LEN, tx->destination);
    PRINTF("Value %.*H\n", tx->value.length, tx->value.value);
    PRINTF("Storage limit %.*H\n", tx->storagelimit.length, tx->storagelimit.value);
    PRINTF("Epoch height %.*H\n", tx->epochheight.length, tx->epochheight.value);
    PRINTF("Chain ID %.*H\n", tx->chainid.length, tx->chainid.value);
    // TODO: data
}

// int handler_sign_tx(buffer_t *cdata, uint8_t chunk, bool more) {
//     if (chunk == 0) {  // first APDU, parse BIP32 path
//         explicit_bzero(&G_context, sizeof(G_context));
//         G_context.req_type = CONFIRM_TRANSACTION;
//         G_context.state = STATE_NONE;

//         if (!buffer_read_u8(cdata, &G_context.bip32_path_len) ||
//             !buffer_read_bip32_path(cdata,
//                                     G_context.bip32_path,
//                                     (size_t) G_context.bip32_path_len)) {
//             return io_send_sw(SW_WRONG_DATA_LENGTH);
//         }

//         return io_send_sw(SW_OK);
//     } else {  // parse transaction
//         if (G_context.req_type != CONFIRM_TRANSACTION) {
//             return io_send_sw(SW_BAD_STATE);
//         }

//         if (more) {  // more APDUs with transaction part
//             if (G_context.tx_info.raw_tx_len + cdata->size > MAX_TRANSACTION_LEN &&  //
//                 !buffer_move(cdata,
//                              G_context.tx_info.raw_tx + G_context.tx_info.raw_tx_len,
//                              cdata->size)) {
//                 return io_send_sw(SW_WRONG_TX_LENGTH);
//             }

//             G_context.tx_info.raw_tx_len += cdata->size;

//             return io_send_sw(SW_OK);
//         } else {  // last APDU, let's parse and sign
//             if (G_context.tx_info.raw_tx_len + cdata->size > MAX_TRANSACTION_LEN ||  //
//                 !buffer_move(cdata,
//                              G_context.tx_info.raw_tx + G_context.tx_info.raw_tx_len,
//                              cdata->size)) {
//                 return io_send_sw(SW_WRONG_TX_LENGTH);
//             }

//             G_context.tx_info.raw_tx_len += cdata->size;

//             buffer_t buf = {.ptr = G_context.tx_info.raw_tx,
//                             .size = G_context.tx_info.raw_tx_len,
//                             .offset = 0};

//             parser_status_e status = transaction_deserialize(&buf, &G_context.tx_info.transaction);
//             PRINTF("Parsing status: %d.\n", status);
//             if (status != PARSING_OK) {
//                 return io_send_sw(SW_TX_PARSING_FAIL);
//             }

//             PRINTF("raw: %.*H\n", G_context.tx_info.raw_tx_len, G_context.tx_info.raw_tx);
//             // debug_print_tx(&G_context.tx_info.transaction);

//             G_context.state = STATE_PARSED;

//             cx_sha3_t keccak256;
//             cx_keccak_init(&keccak256, 256);
//             cx_hash((cx_hash_t *) &keccak256,
//                     CX_LAST,
//                     G_context.tx_info.raw_tx,
//                     G_context.tx_info.raw_tx_len,
//                     G_context.tx_info.m_hash,
//                     sizeof(G_context.tx_info.m_hash));

//             PRINTF("Hash: %.*H\n", sizeof(G_context.tx_info.m_hash), G_context.tx_info.m_hash);

//             return ui_display_transaction();
//         }
//     }

//     return 0;
// }

int handler_sign_tx(uint8_t p1, uint8_t p2, uint8_t *workBuffer, uint16_t dataLength) {
    if (p1 == P1_FIRST) {
        if (dataLength < 1) {
            PRINTF("Invalid data\n");
            THROW(0x6a80);
        }

        if (appState != APP_STATE_IDLE) {
            reset_app_context();
        }

        appState = APP_STATE_SIGNING_TX;

        // parse BIP32 path
        G_context.bip32_path_len = workBuffer[0];

        if ((G_context.bip32_path_len < 0x01) ||
            (G_context.bip32_path_len > MAX_BIP32_PATH)) {
            PRINTF("Invalid path\n");
            THROW(0x6a80);
        }

        workBuffer++;
        dataLength--;

        for (uint32_t ii = 0; ii < G_context.bip32_path_len; ii++) {
            if (dataLength < 4) {
                PRINTF("Invalid data\n");
                THROW(0x6a80);
            }

            G_context.bip32_path[ii] = U4BE(workBuffer, 0);
            workBuffer += 4;
            dataLength -= 4;
        }

        initTx(&G_context.tx_context, &global_sha3, &G_context.tx_content, NULL);
    }

    else if (p1 != P1_MORE) {
        THROW(0x6B00);
    }

    if (p2 != 0) {
        THROW(0x6B00);
    }

    if ((p1 == P1_MORE) && (appState != APP_STATE_SIGNING_TX)) {
        PRINTF("Signature not initialized\n");
        THROW(0x6985);
    }

    if (G_context.tx_context.currentField == RLP_NONE) {
        PRINTF("Parser not initialized\n");
        THROW(0x6985);
    }

    // parse RLP-encoded transaction
    parserStatus_e txResult = processTx(&G_context.tx_context, workBuffer, dataLength);
    debug_print_tx(&G_context.tx_content);

    switch (txResult) {
        case USTREAM_SUSPENDED:
            break;
        case USTREAM_FINISHED:
            break;
        case USTREAM_PROCESSING:
            THROW(0x9000);
        case USTREAM_FAULT:
            THROW(0x6A80);
        default:
            PRINTF("Unexpected parser status\n");
            THROW(0x6A80);
    }

    if (txResult == USTREAM_FINISHED) {
        prepareDisplayTransaction();
    }

    return 0;
}
