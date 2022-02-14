#pragma once

#define NETWORK_STRING_MAX_SIZE    16
#define FUNCTION_SELECTOR_MAX_SIZE 14

#include "constants.h"
#include "common/bip32.h"
#include "eth/rlp_parser.h"
#include "libcfxaddr/cfxaddr.h"

/**
 * Global application settings.
 */
typedef struct app_settings_t {
    uint8_t allow_blind_sign;
    uint8_t allow_detailed_display;
} app_settings_t;

enum blind_sign_t {
    BlindSignDisabled = 0,
    BlindSignEnabled = 1,
};

enum display_style_t {
    DisplayStyleSimple = 0,
    DisplayStyleDetailed = 1,
};

/**
 * Persistent storage.
 */
typedef struct internal_storage_t {
    app_settings_t settings;
    uint8_t initialized;
} internal_storage_t;

/**
 * Enumeration for the status of IO.
 */
typedef enum {
    READY,     /// ready for new event
    RECEIVED,  /// data received
    WAITING    /// waiting
} io_state_e;

typedef enum {
    APP_STATE_IDLE,
    APP_STATE_GETTING_PUBKEY,
    APP_STATE_SIGNING_TX,
    APP_STATE_SIGNING_PERSONAL,
} app_state_t;

/**
 * Structure for public key context information.
 */
typedef struct {
    uint32_t bip32_path[MAX_BIP32_PATH];  /// BIP32 path
    uint8_t bip32_path_len;               /// length of BIP32 path
    uint32_t chain_id;
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
    cx_sha3_t sha3;
    parser_context_t parser_context;
    transaction_t transaction;
} sign_tx_ctx_t;

/**
 * Structure for personal sign context information.
 */
typedef struct {
    uint32_t remaining_length;
    uint32_t chain_id;
    uint32_t bip32_path[MAX_BIP32_PATH];  /// BIP32 path
    uint8_t bip32_path_len;               /// length of BIP32 path
    uint8_t m_hash[32];                   /// message hash digest
    uint8_t signature[MAX_DER_SIG_LEN];   /// transaction signature encoded in DER
    uint8_t signature_len;                /// length of transaction signature
    uint8_t v;                            /// parity of y-coordinate of R in ECDSA signature
    cx_sha3_t sha3;
    cx_sha256_t msg_hash;
} sign_personal_ctx_t;

/**
 * Structure for global context.
 */
typedef struct {
    app_state_t app_state;
    union {
        get_pubkey_ctx_t get_pubkey;
        sign_tx_ctx_t sign_tx;
        sign_personal_ctx_t sign_personal;
    };
} global_ctx_t;

typedef struct settings_strings_t {
    char blind_signing[15];
    char display_style[15];
} settings_strings_t;

typedef struct get_pubkey_strings_t {
    char bip32_path[60];
    char address[CFXADDR_MAX_LENGTH + 1];
} get_pubkey_strings_t;

typedef struct sign_tx_strings_t {
    char sender_address[CFXADDR_MAX_LENGTH + 1];
    char receiver_address[CFXADDR_MAX_LENGTH + 1];
    char full_amount[79];  // 2^256 is 78 digits long
    char max_fee[50];
    char nonce[8];  // 10M tx per account ought to be enough for everybody
    char network_name[NETWORK_STRING_MAX_SIZE];
    char data[FUNCTION_SELECTOR_MAX_SIZE];
} sign_tx_strings_t;

typedef struct sign_personal_strings_t {
    char signer_address[CFXADDR_MAX_LENGTH + 1];
    char hash[67];  // 2*32 + 2 (0x) + 1 (\0)
} sign_personal_strings_t;

typedef union {
    settings_strings_t settings;
    get_pubkey_strings_t get_pubkey;
    sign_tx_strings_t sign_tx;
    sign_personal_strings_t sign_personal;
} strings_t;
