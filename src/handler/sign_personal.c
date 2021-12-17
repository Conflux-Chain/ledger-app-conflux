/*****************************************************************************
 *   app-conflux: Conlfux Ledger App.
 *   (c) 2021 Conflux Foundation.
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

#include "cx.h"

#include "sign_personal.h"
#include "sw.h"
#include "globals.h"
#include "crypto.h"
#include "ui/menu.h"
#include "common/buffer.h"

static const char SIGN_MAGIC[] =
    "\x19"
    "Conflux Signed Message:\n";

void handler_sign_personal(buffer_t *cdata, bool first) {
    sign_personal_ctx_t *ctx = &G_context.sign_personal;

    if (first) {
        if (cdata->size < 1) {
            PRINTF("Invalid data\n");
            THROW(SW_INVALID_DATA);
        }

        if (G_context.app_state != APP_STATE_IDLE) {
            reset_app_context();
        }

        G_context.app_state = APP_STATE_SIGNING_PERSONAL;

        // parse BIP32 path
        if (!buffer_read_u8(cdata, &ctx->bip32_path_len) ||
            !buffer_read_bip32_path(cdata, ctx->bip32_path, (size_t) ctx->bip32_path_len)) {
            THROW(SW_WRONG_DATA_LENGTH);
        }

        // parse chain ID
        if (!buffer_read_u32(cdata, &ctx->chain_id, BE)) {
            THROW(SW_WRONG_DATA_LENGTH);
        }

        // parse message length
        if (!buffer_read_u32(cdata, &ctx->remaining_length, BE)) {
            THROW(SW_INVALID_DATA);
        }

        // init hash with prefix
        cx_keccak_init(&ctx->sha3, 256);

        cx_hash((cx_hash_t *) &ctx->sha3,
                0,
                (uint8_t *) SIGN_MAGIC,
                sizeof(SIGN_MAGIC) - 1,
                NULL,
                0);

        char len_str[11];  // 4 bytes ~ at most 10 characters in decimal repr
        snprintf(len_str, sizeof(len_str), "%d", ctx->remaining_length);
        size_t l = strnlen(len_str, sizeof(len_str));
        cx_hash((cx_hash_t *) &ctx->sha3, 0, (uint8_t *) len_str, l, NULL, 0);

        // init msg hash used for display
        cx_sha256_init(&ctx->msg_hash);
    }

    if (!first && G_context.app_state != APP_STATE_SIGNING_PERSONAL) {
        PRINTF("Signature not initialized\n");
        THROW(SW_BAD_STATE);
    }

    // process data chunk
    const uint8_t *buffer = cdata->ptr + cdata->offset;
    size_t buffer_len = cdata->size - cdata->offset;

    if (buffer_len > ctx->remaining_length) {
        THROW(SW_WRONG_DATA_LENGTH);
    }

    cx_hash((cx_hash_t *) &ctx->sha3, 0, buffer, buffer_len, NULL, 0);
    cx_hash((cx_hash_t *) &ctx->msg_hash, 0, buffer, buffer_len, NULL, 0);
    ctx->remaining_length -= buffer_len;

    // finish hash
    if (ctx->remaining_length > 0) {
        THROW(SW_OK);
    }

    cx_hash((cx_hash_t *) &ctx->sha3, CX_LAST, NULL, 0, ctx->m_hash, 32);

    // display message before signing
    return ui_sign_personal();
}
