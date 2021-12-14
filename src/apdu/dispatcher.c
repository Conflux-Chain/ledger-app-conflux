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

#include <stdint.h>
#include <stdbool.h>

#include "dispatcher.h"
#include "types.h"
#include "sw.h"
#include "common/buffer.h"
#include "apdu_constants.h"
#include "handler/get_public_key.h"
#include "handler/sign_tx.h"

void apdu_dispatcher(const command_t *cmd) {
    if (cmd->cla != CLA) {
        THROW(SW_CLA_NOT_SUPPORTED);
    }

    buffer_t buf = {0};

    switch (cmd->ins) {
        case GET_PUBLIC_KEY:
            if (cmd->p1 > 1 || cmd->p2 > 1) {
                THROW(SW_WRONG_P1P2);
            }

            if (!cmd->data) {
                THROW(SW_WRONG_DATA_LENGTH);
            }

            buf.ptr = cmd->data;
            buf.size = cmd->lc;
            buf.offset = 0;

            return handler_get_public_key(&buf, (bool) cmd->p1, (bool) cmd->p2);

        case SIGN_TX:
            if (cmd->p1 != P1_SIGN_TX_FIRST && cmd->p1 != P1_SIGN_TX_MORE) {
                THROW(SW_WRONG_P1P2);
            }

            if (cmd->p2 != 0x00) {
                THROW(SW_WRONG_P1P2);
            }

            buf.ptr = cmd->data;
            buf.size = cmd->lc;
            buf.offset = 0;

            return handler_sign_tx(&buf, (bool) cmd->p1 == P1_SIGN_TX_FIRST);

        default:
            THROW(SW_INS_NOT_SUPPORTED);
    }
}
