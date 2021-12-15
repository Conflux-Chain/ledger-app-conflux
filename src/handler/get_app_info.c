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

#include <assert.h>  // _Static_assert

#include "get_app_info.h"
#include "constants.h"
#include "globals.h"
#include "sw.h"

void handler_get_app_info() {
    _Static_assert(APPVERSION_LEN == 3, "Length of (MAJOR || MINOR || PATCH) must be 3");
    _Static_assert(MAJOR_VERSION >= 0 && MAJOR_VERSION <= UINT8_MAX, "MAJOR version out of range");
    _Static_assert(MINOR_VERSION >= 0 && MINOR_VERSION <= UINT8_MAX, "MINOR version out of range");
    _Static_assert(PATCH_VERSION >= 0 && PATCH_VERSION <= UINT8_MAX, "PATCH version out of range");

    if (G_context.app_state != APP_STATE_IDLE) {
        reset_app_context();
    }

    // construct flags
    uint8_t flags = 0x00;
    if (N_storage.settings.allow_blind_sign) flags |= APP_FLAG_BLIND_SIGNING_ENABLED;
    if (N_storage.settings.allow_detailed_display) flags |= APP_FLAG_DETAILED_DISPLAY_ENABLED;

    uint8_t resp[1 + APPVERSION_LEN] = {flags,
                                        (uint8_t) MAJOR_VERSION,
                                        (uint8_t) MINOR_VERSION,
                                        (uint8_t) PATCH_VERSION};

    io_send_response(&(const buffer_t){.ptr = resp, .size = sizeof(resp), .offset = 0}, SW_OK);
}
