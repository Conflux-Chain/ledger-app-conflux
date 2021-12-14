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

#include "address.h"
#include "cfxaddr.h"
#include "common/bip32.h"
#include "crypto.h"
#include "eth/utils.h"
#include "ethUtils.h"
#include "format.h"
#include "globals.h"
#include "sw.h"

void render_settings(settings_strings_t *strings) {
    strlcpy(strings->blind_signing,
            (N_storage.settings.allow_blind_sign ? "Enabled" : "NOT Enabled"),
            sizeof(strings->blind_signing));

    strlcpy(strings->display_style,
            (N_storage.settings.allow_detailed_display ? "Detailed" : "Simple"),
            sizeof(strings->display_style));
}

void render_get_pubkey_path(char *out, size_t out_len) {
    memset(out, 0, out_len);

    if (!bip32_path_format(G_context.bip32_path, G_context.bip32_path_len, out, out_len)) {
        THROW(SW_DISPLAY_BIP32_PATH_FAIL);
    }
}

void render_get_pubkey_address(char *out, size_t out_len) {
    uint8_t address[ADDRESS_LEN] = {0x00};

    if (!address_from_pubkey(G_context.pk_info.raw_public_key, address, sizeof(address))) {
        THROW(SW_DISPLAY_ADDRESS_FAIL);
    }

    cfxaddr_encode(address, out, out_len, G_context.pk_info.chain_id);
}

void render_get_pubkey(get_pubkey_strings_t *strings) {
    render_get_pubkey_path(strings->bip32_path, sizeof(strings->bip32_path));
    render_get_pubkey_address(strings->address, sizeof(strings->address));
}

void render_sign_tx_sender(char *out, size_t out_len) {
    uint64_t chain_id =
        u64_from_BE(G_context.tx_content.chainid.value, G_context.tx_content.chainid.length);

    if (chain_id > 0xffff) {
        THROW(SW_CHAIN_ID_TOO_LARGE);
    }

    // derive public key
    uint8_t raw_public_key[64];
    uint8_t chain_code[32];

    crypto_derive_public_key(G_context.bip32_path,
                             G_context.bip32_path_len,
                             raw_public_key,
                             chain_code);

    // derive address
    uint8_t address[ADDRESS_LEN] = {0x00};

    if (!address_from_pubkey(raw_public_key, address, sizeof(address))) {
        THROW(SW_DISPLAY_ADDRESS_FAIL);
    }

    // convert to CIP-37 address
    cfxaddr_encode(address, out, out_len, chain_id);
}

void render_sign_tx_receiver(char *out, size_t out_len) {
    if (G_context.tx_content.destinationLength == 0) {
        strlcpy(out, "New contract", out_len);
        return;
    }

    uint64_t chain_id =
        u64_from_BE(G_context.tx_content.chainid.value, G_context.tx_content.chainid.length);

    if (chain_id > 0xffff) {
        THROW(SW_CHAIN_ID_TOO_LARGE);
    }

    cfxaddr_encode(G_context.tx_content.destination, out, out_len, chain_id);
}

void render_sign_tx_amount(char *out, size_t out_len) {
    amountToString(G_context.tx_content.value.value,
                   G_context.tx_content.value.length,
                   EXPONENT_SMALLEST_UNIT,
                   "CFX ",
                   out,
                   out_len);
}

static void computeFees(txInt256_t *BEgasPrice,
                        txInt256_t *BEgasLimit,
                        txInt256_t *BEstorageLimit,
                        uint256_t *output) {
    uint256_t gasPrice = {0};
    uint256_t gasLimit = {0};
    uint256_t storageLimit = {0};
    uint256_t temp1 = {0};
    uint256_t temp2 = {0};

    PRINTF("Gas price %.*H\n", BEgasPrice->length, BEgasPrice->value);
    PRINTF("Gas limit %.*H\n", BEgasLimit->length, BEgasLimit->value);
    PRINTF("Storage limit %.*H\n", BEstorageLimit->length, BEstorageLimit->value);

    convertUint256BE(BEgasPrice->value, BEgasPrice->length, &gasPrice);
    convertUint256BE(BEgasLimit->value, BEgasLimit->length, &gasLimit);
    convertUint256BE(BEstorageLimit->value, BEstorageLimit->length, &storageLimit);

    // 10^18/1024 = 976562500000000 = 0x3782dace9d900
    // initialize multiplier using four uint64_t fields
    uint256_t multiplier = {{{0x00, 0x00}, {0x00, 0x3782dace9d900}}};

    mul256(&gasPrice, &gasLimit, &temp1);
    mul256(&storageLimit, &multiplier, &temp2);
    add256(&temp1, &temp2, output);
}

// TODO: review this
static void feesToString(uint256_t *rawFee, char *displayBuffer, uint32_t displayBufferSize) {
    char *feeTicker = "CFX ";
    uint8_t tickerOffset = 0;
    uint32_t i;

    // TODO
    char buffer1[100];
    char buffer2[100];

    tostring256(rawFee, 10, (char *) buffer1, 100);

    i = 0;
    while (buffer1[i]) {
        i++;
    }

    adjustDecimals(buffer1, i, buffer2, 100, EXPONENT_SMALLEST_UNIT);
    i = 0;
    tickerOffset = 0;
    memset(displayBuffer, 0, displayBufferSize);

    while (feeTicker[tickerOffset]) {
        displayBuffer[tickerOffset] = feeTicker[tickerOffset];
        tickerOffset++;
    }

    while (buffer2[i]) {
        displayBuffer[tickerOffset + i] = buffer2[i];
        i++;
    }

    displayBuffer[tickerOffset + i] = '\0';
}

void prepareAndCopyFees(txInt256_t *BEGasPrice,
                        txInt256_t *BEGasLimit,
                        txInt256_t *BEstorageLimit,
                        char *displayBuffer,
                        uint32_t displayBufferSize) {
    uint256_t rawFee = {0};
    computeFees(BEGasPrice, BEGasLimit, BEstorageLimit, &rawFee);
    feesToString(&rawFee, displayBuffer, displayBufferSize);
}

void render_sign_tx_fee(char *out, size_t out_len) {
    prepareAndCopyFees(&G_context.tx_content.gasprice,
                       &G_context.tx_content.gaslimit,
                       &G_context.tx_content.storagelimit,
                       out,
                       out_len);
}

void render_sign_tx_nonce(char *out, size_t out_len) {
    uint256_t nonce;
    convertUint256BE(G_context.tx_content.nonce.value, G_context.tx_content.nonce.length, &nonce);
    tostring256(&nonce, 10, out, out_len);
}

void render_sign_tx_network(char *out, size_t out_len) {
    uint64_t chain_id =
        u64_from_BE(G_context.tx_content.chainid.value, G_context.tx_content.chainid.length);

    switch (chain_id) {
        case CONFLUX_MAINNET_CHAINID:
            strlcpy(out, "mainnet", out_len);
            break;

        case CONFLUX_TESTNET_CHAINID:
            strlcpy(out, "testnet", out_len);
            break;

        default:
            u64_to_string(chain_id, out, out_len);
    }
}

void render_sign_tx_data(char *out, size_t out_len) {
    memset(out, 0, out_len);

    strlcpy(out, "0x", out_len);
    out += 2;
    out_len -= 2;

    if (G_context.tx_content.data_length > 0) {
        uint8_t len = MIN(SELECTOR_LENGTH, G_context.tx_content.data_length);
        format_hex(G_context.tx_content.selector, len, out, out_len);
        out += 2 * len;
        out_len -= 2 * len;
    }

    if (G_context.tx_content.data_length > SELECTOR_LENGTH) {
        strlcpy(out, "...", out_len);
    }
}

void render_sign_tx(sign_tx_strings_t *strings) {
    render_sign_tx_sender(strings->sender_address, sizeof(strings->sender_address));
    render_sign_tx_receiver(strings->receiver_address, sizeof(strings->receiver_address));
    render_sign_tx_amount(strings->full_amount, sizeof(strings->full_amount));
    render_sign_tx_fee(strings->max_fee, sizeof(strings->max_fee));
    render_sign_tx_nonce(strings->nonce, sizeof(strings->nonce));
    render_sign_tx_network(strings->network_name, sizeof(strings->network_name));
    render_sign_tx_data(strings->data, sizeof(strings->data));
}
