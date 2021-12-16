#pragma once

/**
 * Instruction class of the Conflux application.
 */
#define CLA 0xE0

/**
 * Offset of instruction class.
 */
#define OFFSET_CLA 0

/**
 * Offset of instruction code.
 */
#define OFFSET_INS 1

/**
 * Offset of instruction parameter 1.
 */
#define OFFSET_P1 2

/**
 * Offset of instruction parameter 2.
 */
#define OFFSET_P2 3

/**
 * Offset of command data length.
 */
#define OFFSET_LC 4

/**
 * Offset of command data.
 */
#define OFFSET_CDATA 5

/**
 * P1 value of first transaction data chunk.
 */
#define P1_SIGN_FIRST 0x00

/**
 * P1 value of subsequent transaction data chunks.
 */
#define P1_SIGN_MORE 0x80