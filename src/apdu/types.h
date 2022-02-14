#pragma once

/**
 * Enumeration with expected INS of APDU commands.
 */
typedef enum {
    GET_APP_INFO = 0x01,    /// get application flags, name, and version
    GET_PUBLIC_KEY = 0x02,  /// public key of corresponding BIP32 path
    SIGN_TX = 0x03,         /// sign transaction with BIP32 path
    SIGN_PERSONAL = 0x04,   /// sign a personal message
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
