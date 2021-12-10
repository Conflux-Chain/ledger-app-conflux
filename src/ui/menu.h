#pragma once

#include "ux.h"

/**
 * Show main menu.
 */
void ui_menu_main(void);

/**
 * Show about submenu.
 */
void ui_menu_about(void);

/**
 * Show settings submenu.
 */
void ui_menu_settings(const ux_flow_step_t* const start_step);

/**
 * Display address on the device and ask confirmation to export.
 */
void ui_get_pubkey(void);

/**
 * Display transaction on the device and ask confirmation to sign.
 */
void ui_sign_tx(void);