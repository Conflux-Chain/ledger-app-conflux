#pragma once

/**
 * Length of APPNAME variable in the Makefile.
 */
#define APPNAME_LEN (sizeof(APPNAME) - 1)

/**
 * Maximum length of MAJOR_VERSION || MINOR_VERSION || PATCH_VERSION.
 */
#define APPVERSION_LEN 3

/**
 * Maximum length of application name.
 */
#define MAX_APPNAME_LEN 64

/**
 * Maximum transaction length (bytes).
 */
#define MAX_TRANSACTION_LEN 510

/**
 * Maximum signature length (bytes).
 */
#define MAX_DER_SIG_LEN 72

/**
 * Exponent used to convert Drip to CFX unit (N CFX = N * 10^18 Drip).
 */
#define EXPONENT_SMALLEST_UNIT 18

/**
 * Well-known Conflux chain IDs
 */
#define CONFLUX_MAINNET_CHAINID 1029
#define CONFLUX_TESTNET_CHAINID 1