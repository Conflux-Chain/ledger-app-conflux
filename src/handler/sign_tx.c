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

#include "os.h"
#include "cx.h"

#include "sign_tx.h"
#include "sw.h"
#include "globals.h"
#include "crypto.h"
#include "ui/menu.h"
#include "common/buffer.h"

#define DEBUG_PRINT_TX(tx)                                                             \
    PRINTF("Nonce %.*H\n", (tx).nonce.length, (tx).nonce.value);                       \
    PRINTF("Gas price %.*H\n", (tx).gasprice.length, (tx).gasprice.value);             \
    PRINTF("Gas limit %.*H\n", (tx).gaslimit.length, (tx).gaslimit.value);             \
    PRINTF("Destination: %.*H\n", ADDRESS_LEN, (tx).destination);                      \
    PRINTF("Value %.*H\n", (tx).value.length, (tx).value.value);                       \
    PRINTF("Storage limit %.*H\n", (tx).storagelimit.length, (tx).storagelimit.value); \
    PRINTF("Epoch height %.*H\n", (tx).epochheight.length, (tx).epochheight.value);    \
    PRINTF("Chain ID %.*H\n", (tx).chainid.length, (tx).chainid.value);                \
    PRINTF("Data prefix %.*H\n", SELECTOR_LENGTH, (tx).selector);

void handler_sign_tx(buffer_t* cdata, bool first) {
    if (first) {
        if (cdata->size < 1) {
            PRINTF("Invalid data\n");
            THROW(SW_INVALID_DATA);
        }

        if (appState != APP_STATE_IDLE) {
            reset_app_context();
        }

        appState = APP_STATE_SIGNING_TX;

        // parse BIP32 path
        if (!buffer_read_u8(cdata, &G_context.bip32_path_len) ||
            !buffer_read_bip32_path(cdata,
                                    G_context.bip32_path,
                                    (size_t) G_context.bip32_path_len)) {
            THROW(SW_WRONG_DATA_LENGTH);
        }

        // init parser
        init_parser(&G_context.tx_context, &global_sha3, &G_context.tx_content);
    }

    if (!first && appState != APP_STATE_SIGNING_TX) {
        PRINTF("Signature not initialized\n");
        THROW(SW_BAD_STATE);
    }

    if (G_context.tx_context.currentField == RLP_NONE) {
        PRINTF("Parser not initialized\n");
        THROW(SW_BAD_STATE);
    }

    // parse RLP-encoded transaction
    const uint8_t* buffer = cdata->ptr + cdata->offset;
    size_t buffer_len = cdata->size - cdata->offset;
    parserStatus_e status = process_tx_chunk(&G_context.tx_context, buffer, buffer_len);

    switch (status) {
        case USTREAM_FINISHED:
            DEBUG_PRINT_TX(G_context.tx_content);
            return ui_sign_tx();

        case USTREAM_PROCESSING:
            THROW(SW_OK);

        case USTREAM_FAULT:
            THROW(SW_TX_PARSING_FAIL);

        default:
            PRINTF("Unexpected parser status\n");
            THROW(SW_INVALID_DATA);
    }
}
