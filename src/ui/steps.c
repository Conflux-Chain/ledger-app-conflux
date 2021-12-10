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

#include "globals.h"
#include "menu.h"
#include "steps.h"
#include "actions.h"

UX_STEP_NOCB(ux_step_get_pubkey_start,
             pn,
             {
                 &C_icon_eye,
                 "Confirm Address",
             });

UX_STEP_NOCB(ux_step_get_pubkey_display_path,
             bnnn_paging,
             {
                 .title = "Path",
                 .text = strings.get_pubkey.bip32_path,
             });

UX_STEP_NOCB(ux_step_get_pubkey_display_address,
             bnnn_paging,
             {
                 .title = "Address",
                 .text = strings.get_pubkey.address,
             });

UX_STEP_CB(ux_step_get_pubkey_approve,
           pb,
           (*g_validate_callback)(true),
           {
               &C_icon_validate_14,
               "Approve",
           });

UX_STEP_CB(ux_step_get_pubkey_reject,
           pb,
           (*g_validate_callback)(false),
           {
               &C_icon_crossmark,
               "Reject",
           });

UX_STEP_NOCB(ux_step_sign_tx_start,
             pnn,
             {
                 &C_icon_eye,
                 "Review",
                 "transaction",
             });

UX_STEP_NOCB(ux_step_sign_tx_display_sender_address,
             bnnn_paging,
             {
                 .title = "Sender",
                 .text = strings.sign_tx.sender_address,
             });

UX_STEP_NOCB(ux_step_sign_tx_display_receiver_address,
             bnnn_paging,
             {
                 .title = "Receiver",
                 .text = strings.sign_tx.receiver_address,
             });

UX_STEP_NOCB(ux_step_sign_tx_display_amount,
             bnnn_paging,
             {.title = "Amount", .text = strings.sign_tx.full_amount});

UX_STEP_NOCB(ux_step_sign_tx_display_fees,
             bnnn_paging,
             {
                 .title = "Max Fees",
                 .text = strings.sign_tx.max_fee,
             });

UX_STEP_NOCB(ux_step_sign_tx_display_nonce,
             bnnn_paging,
             {.title = "Nonce", .text = strings.sign_tx.nonce});

UX_STEP_NOCB(ux_step_sign_tx_display_network,
             bnnn_paging,
             {
                 .title = "Network",
                 .text = strings.sign_tx.network_name,
             });

UX_STEP_NOCB(ux_step_sign_tx_display_data,
             bnnn_paging,
             {
                 .title = "Data",
                 .text = strings.sign_tx.data,
             });

UX_STEP_CB(ux_step_sign_tx_accept,
           pbb,
           (*g_validate_callback)(true),
           {
               &C_icon_validate_14,
               "Accept",
               "and send",
           });

UX_STEP_CB(ux_step_sign_tx_reject,
           pb,
           (*g_validate_callback)(false),
           {
               &C_icon_crossmark,
               "Reject",
           });

UX_STEP_CB(ux_step_error_blind_sign,
           pb,
           THROW(0x6a80),
           {
               &C_icon_warning,
               "Blind sign disabled",
           });

UX_STEP_NOCB(ux_step_about,
             bn,
             {
                 "Conflux App",
                 "(c) 2021 Conflux",
             });

UX_STEP_CB(ux_step_back,
           pb,
           ui_menu_main(),
           {
               &C_icon_back,
               "Back",
           });

UX_STEP_CB(ux_step_settings_blind_sign,
           bnnn_paging,
           ui_action_toggle_settings_blind_signing(),
           {
               .title = "Blind signing",
               .text = strings.settings.blind_signing,
           });

UX_STEP_CB(ux_step_settings_display_style,
           bnnn_paging,
           ui_action_toggle_settings_detailed_display(),
           {.title = "Display style", .text = strings.settings.display_style});

UX_STEP_NOCB(ux_step_menu_ready,
             pnn,
             {
                 &C_boilerplate_logo,
                 "Conflux",
                 "is ready",
             });

UX_STEP_VALID(ux_step_menu_settings,
              pb,
              ui_menu_settings(NULL),
              {
                  &C_icon_coggle,
                  "Settings",
              });

UX_STEP_NOCB(ux_step_menu_version,
             bn,
             {
                 "Version",
                 APPVERSION,
             });

UX_STEP_CB(ux_step_menu_about,
           pb,
           ui_menu_about(),
           {
               &C_icon_certificate,
               "About",
           });

UX_STEP_VALID(ux_step_menu_exit,
              pb,
              os_sched_exit(-1),
              {
                  &C_icon_dashboard_x,
                  "Quit",
              });