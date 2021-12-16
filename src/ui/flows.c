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

#include "flows.h"
#include "globals.h"
#include "steps.h"

UX_FLOW(ux_flow_main_menu,
        &ux_step_menu_ready,
        &ux_step_menu_settings,
        &ux_step_menu_about,
        &ux_step_menu_exit,
        FLOW_LOOP);

UX_FLOW(ux_flow_settings,
        &ux_step_settings_blind_sign,
        &ux_step_settings_display_style,
        &ux_step_back,
        FLOW_LOOP);

UX_FLOW(ux_flow_about, &ux_step_about, &ux_step_menu_version, &ux_step_back, FLOW_LOOP);

UX_FLOW(ux_flow_get_pubkey,
        &ux_step_get_pubkey_start,
        &ux_step_get_pubkey_display_path,
        &ux_step_get_pubkey_display_address,
        &ux_step_get_pubkey_approve,
        &ux_step_get_pubkey_reject);

UX_FLOW(ux_flow_sign_tx_simple,
        &ux_step_sign_tx_start,
        &ux_step_sign_tx_display_receiver_address,
        &ux_step_sign_tx_display_amount,
        &ux_step_sign_tx_display_fees,
        &ux_step_sign_tx_accept,
        &ux_step_sign_tx_reject);

UX_FLOW(ux_flow_sign_tx_detailed,
        &ux_step_sign_tx_start,
        &ux_step_sign_tx_display_sender_address,
        &ux_step_sign_tx_display_receiver_address,
        &ux_step_sign_tx_display_amount,
        &ux_step_sign_tx_display_fees,
        &ux_step_sign_tx_display_nonce,
        &ux_step_sign_tx_display_network,
        &ux_step_sign_tx_display_data,
        &ux_step_sign_tx_accept,
        &ux_step_sign_tx_reject);

UX_FLOW(ux_flow_sign_personal_simple,
        &ux_step_sign_personal_start,
        &ux_step_sign_personal_display_hash,
        &ux_step_sign_personal_accept,
        &ux_step_sign_personal_reject);

UX_FLOW(ux_flow_sign_personal_detailed,
        &ux_step_sign_personal_start,
        &ux_step_sign_personal_display_signer_address,
        &ux_step_sign_personal_display_hash,
        &ux_step_sign_personal_accept,
        &ux_step_sign_personal_reject);

UX_FLOW(ux_flow_error_blind_sign, &ux_step_error_blind_sign);
