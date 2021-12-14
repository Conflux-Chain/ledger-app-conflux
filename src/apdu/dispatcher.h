#pragma once

#include "types.h"

/**
 * Dispatch APDU command received to the right handler.
 *
 * @param[in] cmd
 *   Structured APDU command (CLA, INS, P1, P2, Lc, Command data).
 *
 */
void apdu_dispatcher(const command_t *cmd);
