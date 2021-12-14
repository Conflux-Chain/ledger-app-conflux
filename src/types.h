#pragma once

#define NETWORK_STRING_MAX_SIZE    16
#define FUNCTION_SELECTOR_MAX_SIZE 14

#include "constants.h"
#include "common/bip32.h"
#include "eth/rlp_parser.h"

/**
 * Enumeration for the status of IO.
 */
typedef enum {
    READY,     /// ready for new event
    RECEIVED,  /// data received
    WAITING    /// waiting
} io_state_e;

/**
 * Enumeration with expected INS of APDU commands.
 */
typedef enum {
    GET_PUBLIC_KEY = 0x02,  /// public key of corresponding BIP32 path
    SIGN_TX = 0x03,         /// sign transaction with BIP32 path
} command_e;

/**
 * Structure with fields of APDU command.
 */
typedef struct {
    uint8_t cla;    /// Instruction class
    command_e ins;  /// Instruction code
    uint8_t p1;     /// Instruction parameter 1
    uint8_t p2;     /// Instruction parameter 2
    uint8_t lc;     /// Lenght of command data
    uint8_t *data;  /// Command data
} command_t;

typedef enum {
    APP_STATE_IDLE,
    APP_STATE_GETTING_PUBKEY,
    APP_STATE_SIGNING_TX,
} app_state_t;

/**
 * Structure for public key context information.
 */
typedef struct {
    uint32_t bip32_path[MAX_BIP32_PATH];  /// BIP32 path
    uint8_t bip32_path_len;               /// length of BIP32 path
    uint16_t chain_id;
    bool chaincode_requested;
    uint8_t raw_public_key[64];  /// x-coordinate (32), y-coodinate (32)
    uint8_t chain_code[32];      /// for public key derivation
} get_pubkey_ctx_t;

/**
 * Structure for transaction context information.
 */
typedef struct {
    uint32_t bip32_path[MAX_BIP32_PATH];  /// BIP32 path
    uint8_t bip32_path_len;               /// length of BIP32 path
    uint8_t m_hash[32];                   /// message hash digest
    uint8_t signature[MAX_DER_SIG_LEN];   /// transaction signature encoded in DER
    uint8_t signature_len;                /// length of transaction signature
    uint8_t v;                            /// parity of y-coordinate of R in ECDSA signature
    parser_context_t parser_context;
    transaction_t transaction;
} sign_tx_ctx_t;

/**
 * Structure for global context.
 */
typedef struct {
    app_state_t app_state;
    union {
        get_pubkey_ctx_t get_pubkey;
        sign_tx_ctx_t sign_tx;
    };
} global_ctx_t;

typedef struct settings_strings_t {
    char blind_signing[15];
    char display_style[15];
} settings_strings_t;

typedef struct get_pubkey_strings_t {
    char bip32_path[60];
    char address[CONFLUX_ADDRESS_MAX_LEN];
} get_pubkey_strings_t;

typedef struct sign_tx_strings_t {
    char sender_address[CONFLUX_ADDRESS_MAX_LEN];
    char receiver_address[CONFLUX_ADDRESS_MAX_LEN];
    char full_amount[79];  // 2^256 is 78 digits long
    char max_fee[50];
    char nonce[8];  // 10M tx per account ought to be enough for everybody
    char network_name[NETWORK_STRING_MAX_SIZE];
    char data[FUNCTION_SELECTOR_MAX_SIZE];
} sign_tx_strings_t;

typedef union {
    settings_strings_t settings;
    get_pubkey_strings_t get_pubkey;
    sign_tx_strings_t sign_tx;
} strings_t;
