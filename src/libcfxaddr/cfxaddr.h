/*******************************************************************************
 *   cfxaddr
 *   (c) 2021 Conflux Foundation
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
 ********************************************************************************/

// Source: https://github.com/Conflux-Chain/libcfxaddr/releases/tag/v1.0.1
// See https://github.com/Conflux-Chain/libcfxaddr for more info.

#pragma once

#include <stdint.h>    // uint8_t, uint32_t

// common constants
#define CFXADDR_MAINNET_ID 1029
#define CFXADDR_TESTNET_ID 1
#define CFXADDR_HEX_ADDRESS_LEN 20
#define CFXADDR_MAX_LENGTH 56

// error codes
#define CFXADDR_SUCCESS 0
#define CFXADDR_ERROR_WRONG_LENGTH 1

int cfxaddr_encode(uint8_t *in, char *out, size_t out_len, uint32_t network_id);
