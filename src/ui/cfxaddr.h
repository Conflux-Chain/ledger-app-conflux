/*******************************************************************************
 *   cfxaddr
 *   (c) 2021 Conflux Foundation
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

#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CFXADDR_MAINNET_ID 1029
#define CFXADDR_TESTNET_ID 1
#define CFXADDR_HEX_ADDRESS_LEN 20
#define CFXADDR_VERSION_BYTE 0x00
#define CFXADDR_MASK5BIT 0x1f

#define CFXADDR_SUCCESS 0
#define CFXADDR_ERROR_WRONG_LENGTH 1

// source: https://github.com/bitcoincashorg/bitcoincash.org/blob/master/spec/cashaddr.md
uint64_t cfxaddr_polymod(uint8_t *data, size_t data_len) {
    uint64_t c = 1;

    for (size_t ii = 0; ii < data_len; ++ii) {
        uint8_t d = data[ii];

        uint8_t c0 = c >> 35;
        c = ((c & 0x07ffffffff) << 5) ^ d;

        if (c0 & 0x01) c ^= 0x98f2bc8e61;
        if (c0 & 0x02) c ^= 0x79b76d99e2;
        if (c0 & 0x04) c ^= 0xf33e5fb3c4;
        if (c0 & 0x08) c ^= 0xae2eabe2a8;
        if (c0 & 0x10) c ^= 0x1e4f43e470;
    }

    return c ^ 1;
}

int cfxaddr_encode(uint8_t *in, char *out, size_t out_len, uint16_t network_id) {
    static char CHARSET[] = "abcdefghjkmnprstuvwxyz0123456789";

    char prefix[10]; // the longest prefix is "net" + "65535" + ':' + '\0'
    uint8_t prefix_len = 0;

    // initialize prefix based on network_id
    switch (network_id) {
        case CFXADDR_MAINNET_ID: {
            if (out_len < 4 + 34 + 8 + 1) { return CFXADDR_ERROR_WRONG_LENGTH; };
            memcpy(prefix, "cfx:", 4);
            prefix_len = 4;
            break;
        }

        case CFXADDR_TESTNET_ID: {
            if (out_len < 8 + 34 + 8 + 1) { return CFXADDR_ERROR_WRONG_LENGTH; };
            memcpy(prefix, "cfxtest:", 8);
            prefix_len = 8;
            break;
        }

        default: {
            uint8_t length = 1;
            uint16_t netid = network_id / 10;
            while (netid > 0) { length += 1; netid /= 10; }

            if (out_len < length + 4 + 34 + 8 + 1) { return CFXADDR_ERROR_WRONG_LENGTH; };
            snprintf(prefix, 10, "net%d:", network_id);
            prefix_len = length + 4;
        }
    }

    // store "extended prefix"
    for (size_t ii = 0; ii < prefix_len - 1; ++ii) {
        out[ii] = (uint8_t)prefix[ii] & CFXADDR_MASK5BIT;
    }

    out[prefix_len - 1] = 0x00;

    // convert 21 x 8bits (version byte + payload) to 34 x 5bits
    // treat as if version byte 0x00 is already in acc
    uint16_t acc = CFXADDR_VERSION_BYTE;
    uint8_t num = 8;
    uint8_t buf_len = prefix_len;

    for (size_t ii = 0; ii < CFXADDR_HEX_ADDRESS_LEN; ++ii) {
        acc = (acc << 8 | (uint8_t)(in[ii]));
        num += 8;

        while (num >= 5) {
            out[buf_len++] = (uint8_t)(acc >> (num - 5));
            acc &= ~(CFXADDR_MASK5BIT << (num - 5));
            num -= 5;
        }
    }

    out[buf_len++] = (uint8_t)(acc << (5 - num));

    // calculate checksum
    memset((uint8_t *) out + buf_len, 0x00, 8);
    uint64_t checksum = cfxaddr_polymod((uint8_t *) out, buf_len + 8);

    // convert to characters
    for (size_t ii = 0; ii < prefix_len; ++ii) {
        out[ii] = prefix[ii];
    }

    for (size_t ii = 0; ii < 34; ++ii) {
        out[prefix_len + ii] = CHARSET[(uint8_t) out[prefix_len + ii]];
    }

    for (size_t ii = 0; ii < 8; ++ii) {
        out[prefix_len + 34 + ii] = CHARSET[(uint8_t)(checksum >> ((7 - ii) * 5) & CFXADDR_MASK5BIT)];
    }

    out[prefix_len + 34 + 8] = '\0';

    return CFXADDR_SUCCESS;
}
