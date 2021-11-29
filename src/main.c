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

#include <stdint.h>  // uint*_t
#include <string.h>  // memset, explicit_bzero

#include "os.h"
#include "ux.h"

#include "types.h"
#include "globals.h"
#include "io.h"
#include "sw.h"
#include "ui/menu.h"
#include "apdu/parser.h"
#include "apdu/dispatcher.h"

///////////////////////
#include "shared_context.h";
///////////////////////

uint8_t G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];
io_state_e G_io_state;
ux_state_t G_ux;
bolos_ux_params_t G_ux_params;
global_ctx_t G_context;
const internalStorage_t N_storage_real;

///////////////////////
tmpCtx_t tmpCtx;
txContext_t txContext;
tmpContent_t tmpContent;
cx_sha3_t global_sha3;
uint8_t appState;
strings_t strings;
///////////////////////

void init_storage() {
    if (N_storage.initialized != 0x01) {
        internalStorage_t storage;
        storage.settings.allow_blind_sign = BlindSignDisabled;
        storage.initialized = 0x01;

        nvm_write(
            (internalStorage_t*)&N_storage,
            (void*)&storage,
            sizeof(internalStorage_t)
        );
    }
}

// TODO!!!!
// call in loop
void reset_app_context() {
    appState = APP_STATE_IDLE;
//     called_from_swap = false;
//     pluginType = OLD_INTERNAL;
// #ifdef HAVE_STARKWARE
//     quantumSet = false;
// #endif
// #ifdef HAVE_ETH2
//     eth2WithdrawalIndex = 0;
// #endif
    memset((uint8_t *) &tmpCtx, 0, sizeof(tmpCtx));
    memset((uint8_t *) &txContext, 0, sizeof(txContext));
    memset((uint8_t *) &tmpContent, 0, sizeof(tmpContent));
}

// // TODO: use ui_menu_main instead
// void ui_idle(void) {
//     // reserve a display stack slot if none yet
//     if (G_ux.stack_count == 0) {
//         ux_stack_push();
//     }
//     // ux_flow_init(0, ux_idle_flow, NULL);
//     ux_flow_init(0, ux_menu_main_flow, NULL);
// }


/**
 * Handle APDU command received and send back APDU response using handlers.
 */
void app_main() {
    // Length of APDU command received in G_io_apdu_buffer
    int input_len = 0;
    // Structured APDU command
    command_t cmd;

    // Reset length of APDU response
    G_output_len = 0;
    G_io_state = READY;

    // Reset context
    explicit_bzero(&G_context, sizeof(G_context));

    for (;;) {
        BEGIN_TRY {
            TRY {
                // Reset structured APDU command
                memset(&cmd, 0, sizeof(cmd));

                // Receive command bytes in G_io_apdu_buffer
                if ((input_len = io_recv_command()) < 0) {
                    return;
                }

                // Parse APDU command from G_io_apdu_buffer
                if (!apdu_parser(&cmd, G_io_apdu_buffer, input_len)) {
                    PRINTF("=> /!\\ BAD LENGTH: %.*H\n", input_len, G_io_apdu_buffer);
                    io_send_sw(SW_WRONG_DATA_LENGTH);
                    continue;
                }

                PRINTF("=> CLA=%02X | INS=%02X | P1=%02X | P2=%02X | Lc=%02X | CData=%.*H\n",
                       cmd.cla,
                       cmd.ins,
                       cmd.p1,
                       cmd.p2,
                       cmd.lc,
                       cmd.lc,
                       cmd.data);

                // Dispatch structured APDU command to handler
                if (apdu_dispatcher(&cmd) < 0) {
                    return;
                }
            }
            CATCH(EXCEPTION_IO_RESET) {
                THROW(EXCEPTION_IO_RESET);
            }
            CATCH_OTHER(e) {
                io_send_sw(e);
            }
            FINALLY {
            }
            END_TRY;
        }
    }
}

/**
 * Exit the application and go back to the dashboard.
 */
void app_exit() {
    BEGIN_TRY_L(exit) {
        TRY_L(exit) {
            os_sched_exit(-1);
        }
        FINALLY_L(exit) {
        }
    }
    END_TRY_L(exit);
}

/**
 * Main loop to setup USB, Bluetooth, UI and launch app_main().
 */
__attribute__((section(".boot"))) int main() {
    __asm volatile("cpsie i");

    os_boot();

    for (;;) {
        // Reset UI
        memset(&G_ux, 0, sizeof(G_ux));

        BEGIN_TRY {
            TRY {
                io_seproxyhal_init();

#ifdef TARGET_NANOX
                G_io_app.plane_mode = os_setting_get(OS_SETTING_PLANEMODE, NULL, 0);
#endif  // TARGET_NANOX

                USB_power(0);
                USB_power(1);

                init_storage();

                ui_menu_main();

#ifdef HAVE_BLE
                BLE_power(0, NULL);
                BLE_power(1, "Nano X");
#endif  // HAVE_BLE
                app_main();
            }
            CATCH(EXCEPTION_IO_RESET) {
                CLOSE_TRY;
                continue;
            }
            CATCH_ALL {
                CLOSE_TRY;
                break;
            }
            FINALLY {
            }
        }
        END_TRY;
    }

    app_exit();

    return 0;
}
