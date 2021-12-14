#pragma once

#include <stdbool.h>  // bool

#include "common/buffer.h"

/**
 * Handler for GET_PUBLIC_KEY command. If successfully parse BIP32 path,
 * derive public key/chain code and send APDU response.
 *
 * @see G_context.get_pubkey.bip32_path, G_context.get_pubkey.raw_public_key and
 *      G_context.get_pubkey.chain_code.
 *
 * @param[in,out] cdata
 *   Command data with BIP32 path.
 * @param[in]     display
 *   Whether to display address on screen or not.
 */
void handler_get_public_key(buffer_t *cdata, bool display, bool get_chaincode);
