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

#include <stdbool.h>  // bool
#include <stddef.h>   // size_t
#include <string.h>   // memset, explicit_bzero

#include "get_public_key.h"
#include "globals.h"
#include "sw.h"
#include "crypto.h"
#include "common/buffer.h"
#include "ui/menu.h"
#include "helper/send_response.h"

void handler_get_public_key(buffer_t *cdata, bool display, bool get_chaincode) {
    if (G_context.app_state != APP_STATE_IDLE) {
        reset_app_context();
    }

    G_context.app_state = APP_STATE_GETTING_PUBKEY;

    get_pubkey_ctx_t *ctx = &G_context.get_pubkey;
    ctx->chaincode_requested = get_chaincode;

    // parse BIP32 path
    if (!buffer_read_u8(cdata, &ctx->bip32_path_len) ||
        !buffer_read_bip32_path(cdata, ctx->bip32_path, (size_t) ctx->bip32_path_len)) {
        THROW(SW_WRONG_DATA_LENGTH);
    }

    // parse chain ID
    if (display && !buffer_read_u32(cdata, &ctx->chain_id, BE)) {
        THROW(SW_WRONG_DATA_LENGTH);
    }

    // derive public key and chain code
    crypto_derive_public_key(ctx->bip32_path,
                             ctx->bip32_path_len,
                             ctx->raw_public_key,
                             ctx->chain_code);

    // display and/or return results to caller
    if (display) {
        return ui_get_pubkey();
    }

    helper_send_response_pubkey();
}
