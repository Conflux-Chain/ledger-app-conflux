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

#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t
#include <string.h>  // memmove

#include "send_response.h"
#include "constants.h"
#include "globals.h"
#include "sw.h"
#include "common/buffer.h"

int helper_send_response_pubkey() {
    uint8_t resp[PUBKEY_LEN + CHAINCODE_LEN + 3] = {0};
    size_t offset = 0;

    resp[offset++] = PUBKEY_LEN + 1;
    resp[offset++] = 0x04;

    memmove(resp + offset, G_context.get_pubkey.raw_public_key, PUBKEY_LEN);
    offset += PUBKEY_LEN;

    if (G_context.get_pubkey.chaincode_requested) {
        resp[offset++] = CHAINCODE_LEN;
        memmove(resp + offset, G_context.get_pubkey.chain_code, CHAINCODE_LEN);
        offset += CHAINCODE_LEN;
    }

    return io_send_response(&(const buffer_t){.ptr = resp, .size = offset, .offset = 0}, SW_OK);
}

void helper_send_response_sig(const uint8_t *signature, uint8_t v) {
    uint8_t resp[65] = {0};
    size_t offset = 0;

    // v
    resp[offset++] = v;

    // r
    uint8_t xoffset = 4;  // point to r value
    uint8_t xlength = signature[xoffset - 1];

    if (xlength == 33) {
        xoffset++;
    }

    memmove(resp + offset, signature + xoffset, 32);
    offset += 32;
    xoffset += 32 + 2;  // move over rvalue and TagLEn

    // s
    xlength = signature[xoffset - 1];

    if (xlength == 33) {
        xoffset++;
    }

    memmove(resp + offset, signature + xoffset, 32);
    offset += 32;

    io_send_response(&(const buffer_t){.ptr = resp, .size = offset, .offset = 0}, SW_OK);
}