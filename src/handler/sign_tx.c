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

/////////////////////////
#include "shared_context.h"
#include "../types.h"
#include "utils2.h"
/////////////////////////

void debug_print_tx(transaction_t* tx) {
    char nonce[21] = {0};
    char address[21] = {0};
    char amount[21] = {0};
    char tx_memo[466] = {0};

    format_u64(nonce, sizeof(nonce), tx->nonce);
    PRINTF("nonce: %s\n", nonce);
    // format_hex(&tx->to, ADDRESS_LEN, address, sizeof(address));
    // PRINTF("address: %s\n", address);
    PRINTF("to: %.*H\n", ADDRESS_LEN, tx->to);
    format_fpu64(amount, sizeof(amount), tx->value, 18);  // exponent of smallest unit is 18
    PRINTF("amount: %s\n", amount);
    transaction_utils_format_memo(tx->memo, tx->memo_len, tx_memo, sizeof(tx_memo));
    PRINTF("memo: %s\n", tx_memo);
}

int handler_sign_tx(buffer_t *cdata, uint8_t chunk, bool more) {
    if (chunk == 0) {  // first APDU, parse BIP32 path
        explicit_bzero(&G_context, sizeof(G_context));
        G_context.req_type = CONFIRM_TRANSACTION;
        G_context.state = STATE_NONE;

        if (!buffer_read_u8(cdata, &G_context.bip32_path_len) ||
            !buffer_read_bip32_path(cdata,
                                    G_context.bip32_path,
                                    (size_t) G_context.bip32_path_len)) {
            return io_send_sw(SW_WRONG_DATA_LENGTH);
        }

        return io_send_sw(SW_OK);
    } else {  // parse transaction
        if (G_context.req_type != CONFIRM_TRANSACTION) {
            return io_send_sw(SW_BAD_STATE);
        }

        if (more) {  // more APDUs with transaction part
            if (G_context.tx_info.raw_tx_len + cdata->size > MAX_TRANSACTION_LEN &&  //
                !buffer_move(cdata,
                             G_context.tx_info.raw_tx + G_context.tx_info.raw_tx_len,
                             cdata->size)) {
                return io_send_sw(SW_WRONG_TX_LENGTH);
            }

            G_context.tx_info.raw_tx_len += cdata->size;

            return io_send_sw(SW_OK);
        } else {  // last APDU, let's parse and sign
            if (G_context.tx_info.raw_tx_len + cdata->size > MAX_TRANSACTION_LEN ||  //
                !buffer_move(cdata,
                             G_context.tx_info.raw_tx + G_context.tx_info.raw_tx_len,
                             cdata->size)) {
                return io_send_sw(SW_WRONG_TX_LENGTH);
            }

            G_context.tx_info.raw_tx_len += cdata->size;

            ///////////////////////////////////////////////

            initTx(&txContext, &global_sha3, &tmpContent.txContent, /*customProcessor*/ NULL, NULL);

            parserStatus_e txResult = processTx(&txContext,
                         &txContext.workBuffer,
                        //  dataLength,
                        0,
                        //  (chainConfig->kind == CHAIN_KIND_WANCHAIN ? TX_FLAG_TYPE : 0));
                        0);

            ///////////////////////////////////////////////

            buffer_t buf = {.ptr = G_context.tx_info.raw_tx,
                            .size = G_context.tx_info.raw_tx_len,
                            .offset = 0};

            parser_status_e status = transaction_deserialize(&buf, &G_context.tx_info.transaction);
            PRINTF("Parsing status: %d.\n", status);
            if (status != PARSING_OK) {
                return io_send_sw(SW_TX_PARSING_FAIL);
            }

            PRINTF("raw: %.*H\n", G_context.tx_info.raw_tx_len, G_context.tx_info.raw_tx);
            debug_print_tx(&G_context.tx_info.transaction);

            G_context.state = STATE_PARSED;

            cx_sha3_t keccak256;
            cx_keccak_init(&keccak256, 256);
            cx_hash((cx_hash_t *) &keccak256,
                    CX_LAST,
                    G_context.tx_info.raw_tx,
                    G_context.tx_info.raw_tx_len,
                    G_context.tx_info.m_hash,
                    sizeof(G_context.tx_info.m_hash));

            PRINTF("Hash: %.*H\n", sizeof(G_context.tx_info.m_hash), G_context.tx_info.m_hash);

            return ui_display_transaction();
        }
    }

    return 0;
}

void finalizeParsing(bool direct);

int handler_sign_tx2(uint8_t p1,
                uint8_t p2,
                uint8_t *workBuffer,
                uint16_t dataLength){
                // unsigned int *flags,
                // unsigned int *tx) {
    // UNUSED(tx);
    parserStatus_e txResult;
    uint32_t i;
    if (p1 == P1_FIRST) {
        if (dataLength < 1) {
            PRINTF("Invalid data\n");
            THROW(0x6a80);
        }
        if (appState != APP_STATE_IDLE) {
            reset_app_context();
        }
        appState = APP_STATE_SIGNING_TX;
        tmpCtx.transactionContext.pathLength = workBuffer[0];
        if ((tmpCtx.transactionContext.pathLength < 0x01) ||
            (tmpCtx.transactionContext.pathLength > MAX_BIP32_PATH)) {
            PRINTF("Invalid path\n");
            THROW(0x6a80);
        }
        workBuffer++;
        dataLength--;
        for (i = 0; i < tmpCtx.transactionContext.pathLength; i++) {
            if (dataLength < 4) {
                PRINTF("Invalid data\n");
                THROW(0x6a80);
            }
            tmpCtx.transactionContext.bip32Path[i] = U4BE(workBuffer, 0);
            workBuffer += 4;
            dataLength -= 4;
        }
        tmpContent.txContent.dataPresent = false;
        // dataContext.tokenContext.pluginStatus = ETH_PLUGIN_RESULT_UNAVAILABLE;

        initTx(&txContext, &global_sha3, &tmpContent.txContent, /*customProcessor*/ NULL, NULL);

        // EIP 2718: TransactionType might be present before the TransactionPayload.
        uint8_t txType = *workBuffer;
        if (txType >= MIN_TX_TYPE && txType <= MAX_TX_TYPE) {
            // Enumerate through all supported txTypes here...
            if (txType == EIP2930 || txType == EIP1559) {
                cx_hash((cx_hash_t *) &global_sha3, 0, workBuffer, 1, NULL, 0);
                txContext.txType = txType;
                workBuffer++;
                dataLength--;
            } else {
                PRINTF("Transaction type %d not supported\n", txType);
                THROW(0x6501);
            }
        } else {
            txContext.txType = LEGACY;
        }
        PRINTF("TxType: %x\n", txContext.txType);
    } else if (p1 != P1_MORE) {
        THROW(0x6B00);
    }
    if (p2 != 0) {
        THROW(0x6B00);
    }
    if ((p1 == P1_MORE) && (appState != APP_STATE_SIGNING_TX)) {
        PRINTF("Signature not initialized\n");
        THROW(0x6985);
    }
    if (txContext.currentField == RLP_NONE) {
        PRINTF("Parser not initialized\n");
        THROW(0x6985);
    }
    txResult = processTx(&txContext,
                         workBuffer,
                         dataLength,
                        //  (chainConfig->kind == CHAIN_KIND_WANCHAIN ? TX_FLAG_TYPE : 0));
                        0);

    // tmpContent.txContent.gasprice
    // tmpContent.txContent.startgas
    // tmpContent.txContent.value
    // tmpContent.txContent.nonce
    // tmpContent.txContent.chainID
    // tmpContent.txContent.destination

    PRINTF("Gas price %.*H\n", tmpContent.txContent.gasprice.length, tmpContent.txContent.gasprice.value);
    PRINTF("Gas limit %.*H\n", tmpContent.txContent.startgas.length, tmpContent.txContent.startgas.value);
    PRINTF("Value %.*H\n", tmpContent.txContent.value.length, tmpContent.txContent.value.value);
    PRINTF("Nonce %.*H\n", tmpContent.txContent.nonce.length, tmpContent.txContent.nonce.value);
    PRINTF("Chain ID %.*H\n", tmpContent.txContent.chainID.length, tmpContent.txContent.chainID.value);
    PRINTF("Destination: %.*H\n", ADDRESS_LEN, tmpContent.txContent.destination);

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
        finalizeParsing(false);
    }

    // *flags |= IO_ASYNCH_REPLY;
    return 0;
}

void compareOrCopy(char *preapproved_string, size_t size, char *parsed_string, bool silent_mode) {
    // if (silent_mode) {
    //     /* ETH address are not fundamentally case sensitive but might
    //     have some for checksum purpose, so let's get rid of these diffs */
    //     to_uppercase(preapproved_string, strlen(preapproved_string));
    //     to_uppercase(parsed_string, strlen(parsed_string));
    //     if (memcmp(preapproved_string, parsed_string, strlen(preapproved_string))) {
    //         THROW(ERR_SILENT_MODE_CHECK_FAILED);
    //     }
    // } else {
        strlcpy(preapproved_string, parsed_string, size);
    // }
}

void prepareFeeDisplay();
void prepareNetworkDisplay();
void ux_approve_tx(bool fromPlugin);

void finalizeParsing(bool direct) {
    char displayBuffer[50];
//     uint8_t decimals = WEI_TO_ETHER;
//     char *ticker = get_network_ticker();
//     ethPluginFinalize_t pluginFinalize;
//     bool genericUI = true;

//     // Verify the chain
//     if (chainConfig->chainId != ETHEREUM_MAINNET_CHAINID) {
//         uint64_t id = get_chain_id();

//         if (chainConfig->chainId != id) {
//             PRINTF("Invalid chainID %u expected %u\n", id, chainConfig->chainId);
//             reset_app_context();
//             reportFinalizeError(direct);
//             if (!direct) {
//                 return;
//             }
//         }
//     }
    // Store the hash
    cx_hash((cx_hash_t *) &global_sha3,
            CX_LAST,
            tmpCtx.transactionContext.hash,
            0,
            tmpCtx.transactionContext.hash,
            32);

    PRINTF("Hash: %.*H\n", INT256_LENGTH, tmpCtx.transactionContext.hash);

//     // Finalize the plugin handling
//     if (dataContext.tokenContext.pluginStatus >= ETH_PLUGIN_RESULT_SUCCESSFUL) {
//         genericUI = false;
//         eth_plugin_prepare_finalize(&pluginFinalize);

//         uint8_t msg_sender[ADDRESS_LENGTH] = {0};
//         get_public_key(msg_sender, sizeof(msg_sender));
//         pluginFinalize.address = msg_sender;

//         if (!eth_plugin_call(ETH_PLUGIN_FINALIZE, (void *) &pluginFinalize)) {
//             PRINTF("Plugin finalize call failed\n");
//             reportFinalizeError(direct);
//             if (!direct) {
//                 return;
//             }
//         }
//         // Lookup tokens if requested
//         ethPluginProvideInfo_t pluginProvideInfo;
//         eth_plugin_prepare_provide_info(&pluginProvideInfo);
//         if ((pluginFinalize.tokenLookup1 != NULL) || (pluginFinalize.tokenLookup2 != NULL)) {
//             if (pluginFinalize.tokenLookup1 != NULL) {
//                 PRINTF("Lookup1: %.*H\n", ADDRESS_LENGTH, pluginFinalize.tokenLookup1);
//                 pluginProvideInfo.item1 = getKnownToken(pluginFinalize.tokenLookup1);
//                 if (pluginProvideInfo.item1 != NULL) {
//                     PRINTF("Token1 ticker: %s\n", pluginProvideInfo.item1->token.ticker);
//                 }
//             }
//             if (pluginFinalize.tokenLookup2 != NULL) {
//                 PRINTF("Lookup2: %.*H\n", ADDRESS_LENGTH, pluginFinalize.tokenLookup2);
//                 pluginProvideInfo.item2 = getKnownToken(pluginFinalize.tokenLookup2);
//                 if (pluginProvideInfo.item2 != NULL) {
//                     PRINTF("Token2 ticker: %s\n", pluginProvideInfo.item2->token.ticker);
//                 }
//             }
//             if (eth_plugin_call(ETH_PLUGIN_PROVIDE_INFO, (void *) &pluginProvideInfo) <=
//                 ETH_PLUGIN_RESULT_UNSUCCESSFUL) {
//                 PRINTF("Plugin provide token call failed\n");
//                 reportFinalizeError(direct);
//                 if (!direct) {
//                     return;
//                 }
//             }
//             pluginFinalize.result = pluginProvideInfo.result;
//         }
//         if (pluginFinalize.result != ETH_PLUGIN_RESULT_FALLBACK) {
//             // Handle the right interface
//             switch (pluginFinalize.uiType) {
//                 case ETH_UI_TYPE_GENERIC:
//                     tmpContent.txContent.dataPresent = false;
//                     // Add the number of screens + the number of additional screens to get the total
//                     // number of screens needed.
//                     dataContext.tokenContext.pluginUiMaxItems =
//                         pluginFinalize.numScreens + pluginProvideInfo.additionalScreens;
//                     break;
//                 case ETH_UI_TYPE_AMOUNT_ADDRESS:
//                     genericUI = true;
//                     tmpContent.txContent.dataPresent = false;
//                     if ((pluginFinalize.amount == NULL) || (pluginFinalize.address == NULL)) {
//                         PRINTF("Incorrect amount/address set by plugin\n");
//                         reportFinalizeError(direct);
//                         if (!direct) {
//                             return;
//                         }
//                     }
//                     memmove(tmpContent.txContent.value.value, pluginFinalize.amount, 32);
//                     tmpContent.txContent.value.length = 32;
//                     memmove(tmpContent.txContent.destination, pluginFinalize.address, 20);
//                     tmpContent.txContent.destinationLength = 20;
//                     if (pluginProvideInfo.item1 != NULL) {
//                         decimals = pluginProvideInfo.item1->token.decimals;
//                         ticker = pluginProvideInfo.item1->token.ticker;
//                     }
//                     break;
//                 default:
//                     PRINTF("ui type %d not supported\n", pluginFinalize.uiType);
//                     reportFinalizeError(direct);
//                     if (!direct) {
//                         return;
//                     }
//             }
//         } else {
//             genericUI = true;
//         }
//     }

//     if (tmpContent.txContent.dataPresent && !N_storage.dataAllowed) {
//         reportFinalizeError(direct);
//         ui_warning_contract_data();
//         if (!direct) {
//             return;
//         }
//     }

//     // Prepare destination address to display
//     if (genericUI) {
        if (tmpContent.txContent.destinationLength != 0) {
            getEthDisplayableAddress(tmpContent.txContent.destination,
                                     displayBuffer,
                                     sizeof(displayBuffer),
                                     &global_sha3,
                                    //  chainConfig->chainId);
                                    1); // TODO
            compareOrCopy(strings.common.fullAddress,
                          sizeof(strings.common.fullAddress),
                          displayBuffer,
                        //   called_from_swap);
                        false);
        } else {
            strcpy(strings.common.fullAddress, "Contract");
        }
//     }

//     // Prepare amount to display
//     if (genericUI) {
        amountToString(tmpContent.txContent.value.value,
                       tmpContent.txContent.value.length,
                    //    decimals,
                    18,
                    //    ticker,
                      "ETH",
                       displayBuffer,
                       sizeof(displayBuffer));
        compareOrCopy(strings.common.fullAmount,
                      sizeof(strings.common.fullAmount),
                      displayBuffer,
                      //   called_from_swap);
                      false);
//     }

    // Prepare nonce to display
    uint256_t nonce;
    convertUint256BE(tmpContent.txContent.nonce.value, tmpContent.txContent.nonce.length, &nonce);
    tostring256(&nonce, 10, displayBuffer, sizeof(displayBuffer));
    strlcpy(strings.common.nonce, displayBuffer, sizeof(strings.common.nonce));

    // Compute maximum fee
    prepareFeeDisplay();
    PRINTF("Fees displayed: %s\n", strings.common.maxFee);

    // Prepare chainID field
    prepareNetworkDisplay();
    PRINTF("Network: %s\n", strings.common.network_name);

//     bool no_consent;

//     no_consent = called_from_swap;

// #ifdef NO_CONSENT
//     no_consent = true;
// #endif  // NO_CONSENT

//     if (no_consent) {
//         io_seproxyhal_touch_tx_ok(NULL);
//     } else {
//         if (genericUI) {
            ux_approve_tx(false);
//         } else {
//             plugin_ui_start();
//         }
//     }
}

void prepareNetworkDisplay() {
    // char *name = get_network_name();
    char *name = NULL;
    if (name == NULL) {
        // No network name found so simply copy the chain ID as the network name.
        // uint64_t chain_id = get_chain_id();
        uint64_t chain_id = 1;
        u64_to_string(chain_id, strings.common.network_name, sizeof(strings.common.network_name));
    } else {
        // Network name found, simply copy it.
        strlcpy(strings.common.network_name, name, sizeof(strings.common.network_name));
    }
}

// Convert `BEgasPrice` and `BEgasLimit` to Uint256 and then stores the multiplication of both in
// `output`.
static void computeFees(txInt256_t *BEgasPrice, txInt256_t *BEgasLimit, uint256_t *output) {
    uint256_t gasPrice = {0};
    uint256_t gasLimit = {0};

    PRINTF("Gas price %.*H\n", BEgasPrice->length, BEgasPrice->value);
    PRINTF("Gas limit %.*H\n", BEgasLimit->length, BEgasLimit->value);
    convertUint256BE(BEgasPrice->value, BEgasPrice->length, &gasPrice);
    convertUint256BE(BEgasLimit->value, BEgasLimit->length, &gasLimit);
    mul256(&gasPrice, &gasLimit, output);
}

static void feesToString(uint256_t *rawFee, char *displayBuffer, uint32_t displayBufferSize) {
    // char *feeTicker = get_network_ticker();
    char *feeTicker = "ETH";
    uint8_t tickerOffset = 0;
    uint32_t i;

    tostring256(rawFee, 10, (char *) (G_io_apdu_buffer + 100), 100);
    i = 0;
    while (G_io_apdu_buffer[100 + i]) {
        i++;
    }
    adjustDecimals((char *) (G_io_apdu_buffer + 100),
                   i,
                   (char *) G_io_apdu_buffer,
                   100,
                //    WEI_TO_ETHER);
                   18);
    i = 0;
    tickerOffset = 0;
    memset(displayBuffer, 0, displayBufferSize);
    while (feeTicker[tickerOffset]) {
        displayBuffer[tickerOffset] = feeTicker[tickerOffset];
        tickerOffset++;
    }
    while (G_io_apdu_buffer[i]) {
        displayBuffer[tickerOffset + i] = G_io_apdu_buffer[i];
        i++;
    }
    displayBuffer[tickerOffset + i] = '\0';
}

void prepareAndCopyFees(txInt256_t *BEGasPrice,
                        txInt256_t *BEGasLimit,
                        char *displayBuffer,
                        uint32_t displayBufferSize) {
    uint256_t rawFee = {0};
    computeFees(BEGasPrice, BEGasLimit, &rawFee);
    feesToString(&rawFee, displayBuffer, displayBufferSize);
}

void prepareFeeDisplay() {
    prepareAndCopyFees(&tmpContent.txContent.gasprice,
                       &tmpContent.txContent.startgas,
                       strings.common.maxFee,
                       sizeof(strings.common.maxFee));
}


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////


void format_signature_out(const uint8_t *signature) {
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
}

unsigned int io_seproxyhal_touch_tx_ok(__attribute__((unused)) const bagl_element_t *e) {
    uint8_t privateKeyData[INT256_LENGTH];
    uint8_t signature[100];
    cx_ecfp_private_key_t privateKey;
    uint32_t tx = 0;
    io_seproxyhal_io_heartbeat();
    os_perso_derive_node_bip32(CX_CURVE_256K1,
                               tmpCtx.transactionContext.bip32Path,
                               tmpCtx.transactionContext.pathLength,
                               privateKeyData,
                               NULL);
    cx_ecfp_init_private_key(CX_CURVE_256K1, privateKeyData, 32, &privateKey);

    ////////////////////////
    // PRINTF("privateKey: %.*H\n", sizeof(privateKey), privateKey);
    // PRINTF("privateKeyData: %.*H\n", sizeof(privateKeyData), privateKeyData);
    ////////////////////////

    explicit_bzero(privateKeyData, sizeof(privateKeyData));
    unsigned int info = 0;
    io_seproxyhal_io_heartbeat();
    cx_ecdsa_sign(&privateKey,
                  CX_RND_RFC6979 | CX_LAST,
                  CX_SHA256,
                  tmpCtx.transactionContext.hash,
                  sizeof(tmpCtx.transactionContext.hash),
                  signature,
                  sizeof(signature),
                  &info);

    ////////////////////////
    PRINTF("signature: %.*H\n", sizeof(signature), signature);
    ////////////////////////

    explicit_bzero(&privateKey, sizeof(privateKey));
    // if (txContext.txType == EIP1559 || txContext.txType == EIP2930) {
    //     if (info & CX_ECCINFO_PARITY_ODD) {
    //         G_io_apdu_buffer[0] = 1;
    //     } else {
    //         G_io_apdu_buffer[0] = 0;
    //     }
    // } else {
        // Parity is present in the sequence tag in the legacy API
        if (tmpContent.txContent.vLength == 0) {
            // Legacy API
            G_io_apdu_buffer[0] = 27;
        } else {
            // New API
            // Note that this is wrong for a large v, but ledgerjs will recover.

            // Taking only the 4 highest bytes to not introduce breaking changes. In the future,
            // this should be updated.
            uint32_t v = (uint32_t) u64_from_BE(tmpContent.txContent.v,
                                                MIN(4, tmpContent.txContent.vLength));
            G_io_apdu_buffer[0] = (v * 2) + 35;
        }
        if (info & CX_ECCINFO_PARITY_ODD) {
            G_io_apdu_buffer[0]++;
        }
        if (info & CX_ECCINFO_xGTn) {
            G_io_apdu_buffer[0] += 2;
        }
    // }
    format_signature_out(signature);
    tx = 65;
    G_io_apdu_buffer[tx++] = 0x90;
    G_io_apdu_buffer[tx++] = 0x00;
    // Send back the response, do not restart the event loop
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx);
    // if (called_from_swap) {
    //     os_sched_exit(0);
    // }
    reset_app_context();
    // Display back the original UX
    ui_menu_main();
    return 0;  // do not redraw the widget
}

unsigned int io_seproxyhal_touch_tx_cancel(__attribute__((unused)) const bagl_element_t *e) {
    reset_app_context();
    G_io_apdu_buffer[0] = 0x69;
    G_io_apdu_buffer[1] = 0x85;
    // Send back the response, do not restart the event loop
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
    // Display back the original UX
    ui_menu_main();
    return 0;  // do not redraw the widget
}

UX_STEP_NOCB(ux_approval_review_step,
    pnn,
    {
      &C_icon_eye,
      "Review",
      "transaction",
    });

UX_STEP_NOCB(
    ux_approval_amount_step,
    bnnn_paging,
    {
      .title = "Amount",
      .text = strings.common.fullAmount
    });
UX_STEP_NOCB(
    ux_approval_address_step,
    bnnn_paging,
    {
      .title = "Address",
      .text = strings.common.fullAddress,
    });

UX_STEP_NOCB(
    ux_approval_fees_step,
    bnnn_paging,
    {
      .title = "Max Fees",
      .text = strings.common.maxFee,
    });

UX_STEP_CB(
    ux_approval_accept_step,
    pbb,
    io_seproxyhal_touch_tx_ok(NULL),
    {
      &C_icon_validate_14,
      "Accept",
      "and send",
    });
UX_STEP_CB(
    ux_approval_reject_step,
    pb,
    io_seproxyhal_touch_tx_cancel(NULL),
    {
      &C_icon_crossmark,
      "Reject",
    });

const ux_flow_step_t *ux_approval_tx_flow[15];

void ux_approve_tx(bool fromPlugin) {
    int step = 0;
    ux_approval_tx_flow[step++] = &ux_approval_review_step;

    // if (!fromPlugin && tmpContent.txContent.dataPresent && !N_storage.contractDetails) {
    //     ux_approval_tx_flow[step++] = &ux_approval_blind_signing_warning_step;
    // }

    // if (fromPlugin) {
    //     // Add the special dynamic display logic
    //     ux_approval_tx_flow[step++] = &ux_plugin_approval_id_step;
    //     ux_approval_tx_flow[step++] = &ux_plugin_approval_before_step;
    //     ux_approval_tx_flow[step++] = &ux_plugin_approval_display_step;
    //     ux_approval_tx_flow[step++] = &ux_plugin_approval_after_step;
    // } else {
        // We're in a regular transaction, just show the amount and the address
        ux_approval_tx_flow[step++] = &ux_approval_amount_step;
        ux_approval_tx_flow[step++] = &ux_approval_address_step;
    // }

    // if (N_storage.displayNonce) {
    //     ux_approval_tx_flow[step++] = &ux_approval_nonce_step;
    // }

    // uint64_t chain_id = get_chain_id();
    // if (chainConfig->chainId == ETHEREUM_MAINNET_CHAINID && chain_id != chainConfig->chainId) {
    //     ux_approval_tx_flow[step++] = &ux_approval_network_step;
    // }

    ux_approval_tx_flow[step++] = &ux_approval_fees_step;
    ux_approval_tx_flow[step++] = &ux_approval_accept_step;
    ux_approval_tx_flow[step++] = &ux_approval_reject_step;
    ux_approval_tx_flow[step++] = FLOW_END_STEP;

    ux_flow_init(0, ux_approval_tx_flow, NULL);
}