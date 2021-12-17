#pragma once

#include "../common/macros.h"

/**
 * Length of public key.
 */
#define PUBKEY_LEN (MEMBER_SIZE(get_pubkey_ctx_t, raw_public_key))
/**
 * Length of chain code.
 */
#define CHAINCODE_LEN (MEMBER_SIZE(get_pubkey_ctx_t, chain_code))

/**
 * Helper to send APDU response with public key and chain code.
 *
 * response = PUBKEY_LEN (1) ||
 *            G_context.get_pubkey.public_key (PUBKEY_LEN) ||
 *            CHAINCODE_LEN (1) ||
 *            G_context.get_pubkey.chain_code (CHAINCODE_LEN)
 *
 * @return zero or positive integer if success, -1 otherwise.
 *
 */
int helper_send_response_pubkey(void);

/**
 * Helper to send APDU response with signature and v (parity of
 * y-coordinate of R).
 *
 * response = G_context.sign_tx.signature_len (1) ||
 *            G_context.sign_tx.signature (G_context.sign_tx.signature_len) ||
 *            G_context.sign_tx.v (1)
 *
 * @return zero or positive integer if success, -1 otherwise.
 *
 */
void helper_send_response_sig(const uint8_t *signature, uint8_t v);