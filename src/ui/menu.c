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

#include "os.h"
#include "ux.h"

#include "actions.h"
#include "flows.h"
#include "globals.h"
#include "glyphs.h"
#include "menu.h"
#include "render.h"
#include "steps.h"
#include "sw.h"

void ui_menu_main() {
    if (G_ux.stack_count == 0) {
        ux_stack_push();
    }

    return ux_flow_init(0, ux_flow_main_menu, NULL);
}

void ui_menu_about() {
    return ux_flow_init(0, ux_flow_about, NULL);
}

void ui_menu_settings(const ux_flow_step_t* const start_step) {
    render_settings(&strings.settings);
    return ux_flow_init(0, ux_flow_settings, start_step);
}

void ui_get_pubkey() {
    if (G_context.req_type != CONFIRM_ADDRESS || G_context.state != STATE_NONE) {
        G_context.state = STATE_NONE;
        THROW(SW_BAD_STATE);
    }

    render_get_pubkey(&strings.get_pubkey);
    g_validate_callback = &ui_action_validate_pubkey;
    return ux_flow_init(0, ux_flow_get_pubkey, NULL);
}

void ui_sign_tx() {
    // no blind signing
    if (G_context.tx_content.data_present && !N_storage.settings.allow_blind_sign) {
        return ux_flow_init(0, ux_flow_error_blind_sign, NULL);
    }

    // store the hash
    cx_hash((cx_hash_t*) &global_sha3,
            CX_LAST,
            G_context.tx_info.m_hash,
            0,
            G_context.tx_info.m_hash,
            32);

    PRINTF("Hash: %.*H\n", INT256_LENGTH, G_context.tx_info.m_hash);

    // display transaction for review
    render_sign_tx(&strings.sign_tx);
    g_validate_callback = &ui_action_validate_transaction;

    if (N_storage.settings.allow_detailed_display) {
        return ux_flow_init(0, ux_flow_sign_tx_detailed, NULL);
    } else {
        return ux_flow_init(0, ux_flow_sign_tx_simple, NULL);
    }
}
