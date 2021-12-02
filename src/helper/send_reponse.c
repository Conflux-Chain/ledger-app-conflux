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
#include "../constants.h"
#include "../globals.h"
#include "../sw.h"
#include "common/buffer.h"

int helper_send_response_pubkey() {
    uint8_t resp[1 + 1 + PUBKEY_LEN + 1 + CHAINCODE_LEN] = {0};
    size_t offset = 0;

    resp[offset++] = PUBKEY_LEN + 1;
    resp[offset++] = 0x04;
    memmove(resp + offset, G_context.pk_info.raw_public_key, PUBKEY_LEN);
    offset += PUBKEY_LEN;
    resp[offset++] = CHAINCODE_LEN;
    memmove(resp + offset, G_context.pk_info.chain_code, CHAINCODE_LEN);
    offset += CHAINCODE_LEN;

    return io_send_response(&(const buffer_t){.ptr = resp, .size = offset, .offset = 0}, SW_OK);
}

// int helper_send_response_sig() {
//     uint8_t resp[1 + MAX_DER_SIG_LEN + 1] = {0};
//     size_t offset = 0;

//     resp[offset++] = G_context.tx_info.signature_len;
//     memmove(resp + offset, G_context.tx_info.signature, G_context.tx_info.signature_len);
//     offset += G_context.tx_info.signature_len;
//     resp[offset++] = (uint8_t) G_context.tx_info.v;

//     return io_send_response(&(const buffer_t){.ptr = resp, .size = offset, .offset = 0}, SW_OK);
// }

void helper_send_response_sig(const uint8_t *signature) {
    // if (G_context.tx_content.vLength == 0) {
        // Legacy API
        G_io_apdu_buffer[0] = G_context.tx_info.v;

    // } else {
    //     // New API
    //     // Note that this is wrong for a large v, but ledgerjs will recover.

    //     // Taking only the 4 highest bytes to not introduce breaking changes. In the future,
    //     // this should be updated.
    //     uint32_t v = (uint32_t) u64_from_BE(G_context.tx_content.v, MIN(4, G_context.tx_content.vLength));
    //     G_io_apdu_buffer[0] = (v * 2) + 35;
    // }
    // if (info & CX_ECCINFO_PARITY_ODD) {
    //     G_io_apdu_buffer[0]++;
    // }
    // if (info & CX_ECCINFO_xGTn) {
    //     G_io_apdu_buffer[0] += 2;
    // }

    memset(G_io_apdu_buffer + 1, 0x00, 64);
    uint8_t offset = 1;
    uint8_t xoffset = 4;  // point to r value
    // copy r
    uint8_t xlength = signature[xoffset - 1];
    if (xlength == 33) {
        xlength = 32;
        xoffset++;
    }
    memmove(G_io_apdu_buffer + offset + 32 - xlength, signature + xoffset, xlength);
    offset += 32;
    xoffset += xlength + 2;  // move over rvalue and TagLEn
    // copy s value
    xlength = signature[xoffset - 1];
    if (xlength == 33) {
        xlength = 32;
        xoffset++;
    }
    memmove(G_io_apdu_buffer + offset + 32 - xlength, signature + xoffset, xlength);

    // tx = 65;
    // G_io_apdu_buffer[tx++] = 0x90;
    // G_io_apdu_buffer[tx++] = 0x00;
    // tx = 65;
    G_io_apdu_buffer[65] = 0x90;
    G_io_apdu_buffer[66] = 0x00;

    // Send back the response, do not restart the event loop
    // io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx);
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 67);
}