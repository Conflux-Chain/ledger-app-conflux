/*****************************************************************************
 *   app-conflux: Conlfux Ledger App.
 *   (c) 2021 Conflux Foundation.
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

#include "actions.h"
#include "crypto.h"
#include "globals.h"
#include "helper/send_response.h"
#include "io.h"
#include "menu.h"
#include "steps.h"
#include "sw.h"

action_validate_cb_t g_validate_callback;

void ui_action_toggle_settings_blind_signing() {
    uint8_t newValue = (N_storage.settings.allow_blind_sign ? BlindSignDisabled : BlindSignEnabled);
    nvm_write((void*) &N_storage.settings.allow_blind_sign, (void*) &newValue, sizeof(uint8_t));
    return ui_menu_settings(&ux_step_settings_blind_sign);
}

void ui_action_toggle_settings_detailed_display() {
    uint8_t newValue =
        (N_storage.settings.allow_detailed_display ? DisplayStyleSimple : DisplayStyleDetailed);
    nvm_write((void*) &N_storage.settings.allow_detailed_display,
              (void*) &newValue,
              sizeof(uint8_t));
    return ui_menu_settings(&ux_step_settings_display_style);
}

void ui_action_validate_pubkey(bool choice) {
    if (choice) {
        helper_send_response_pubkey();
    } else {
        THROW(SW_DENY);
    }

    reset_app_context();
    ui_menu_main();
}

void ui_action_validate_transaction(bool choice) {
    if (choice) {
        G_context.app_state = APP_STATE_IDLE;

        if (crypto_sign_message() < 0) {
            THROW(SW_SIGNATURE_FAIL);
        } else {
            helper_send_response_sig(G_context.sign_tx.signature);
        }
    } else {
        G_context.app_state = APP_STATE_IDLE;
        THROW(SW_DENY);
    }

    reset_app_context();
    ui_menu_main();
}
