/*
 *  Copyright (C) 2017, Nayuta, Inc. All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed to the Apache Software Foundation (ASF) under one
 *  or more contributor license agreements.  See the NOTICE file
 *  distributed with this work for additional information
 *  regarding copyright ownership.  The ASF licenses this file
 *  to you under the Apache License, Version 2.0 (the
 *  "License"); you may not use this file except in compliance
 *  with the License.  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing,
 *  software distributed under the License is distributed on an
 *  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 *  KIND, either express or implied.  See the License for the
 *  specific language governing permissions and limitations
 *  under the License.
 */
/** @file   ln_htlc.c
 *  @brief  ln_htlc
 */
#include "utl_int.h"

#include "ln_local.h"
#include "ln_update.h"


/**************************************************************************
 * public functions
 **************************************************************************/

uint32_t ln_update_flags2u32(ln_update_flags_t Flags)
{
    if (sizeof(ln_update_flags_t) == 2) {
        return utl_int_pack_u16be((const uint8_t *)&Flags);
    } else if (sizeof(ln_update_flags_t) == 4) {
        return utl_int_pack_u32be((const uint8_t *)&Flags);
    } else {
        return 0;
    }
}


ln_update_t *ln_update_get_empty( ln_update_t *pUpdates, uint16_t *pUpdateIdx)
{
    for (uint16_t idx = 0; idx < LN_UPDATE_MAX; idx++) {
        ln_update_t *p_update = &pUpdates[idx];
        if (LN_UPDATE_ENABLED(p_update)) continue;
        if (pUpdateIdx) {
            *pUpdateIdx = idx;
        }
        return p_update;
    }
    return NULL;
}


ln_htlc_t *ln_htlc_get_empty(ln_htlc_t *pHtlcs, uint16_t *pHtlcIdx)
{
    for (uint16_t idx = 0; idx < LN_HTLC_RECEIVED_MAX; idx++) {
        ln_htlc_t *p_htlc = &pHtlcs[idx];
        if (p_htlc->enabled) continue;
        if (pHtlcIdx) {
            *pHtlcIdx = idx;
        }
        return p_htlc;
    }
    return NULL;
}


#ifdef LN_DBG_PRINT
void ln_update_print(const ln_update_t *pUpdate)
{
    const ln_update_flags_t *p_flags = &pUpdate->flags;
    LOGD("    type=%s\n", ln_update_type_str(pUpdate->type));
    LOGD("    up_send=%d\n", p_flags->up_send);
    LOGD("    up_recv=%d\n", p_flags->up_recv);
    LOGD("    cs_send=%d\n", p_flags->cs_send);
    LOGD("    cs_recv=%d\n", p_flags->cs_recv);
    LOGD("    ra_send=%d\n", p_flags->ra_send);
    LOGD("    ra_recv=%d\n", p_flags->ra_recv);
    LOGD("    fin_type=%s\n", ln_update_type_str(pUpdate->fin_type));
}

void ln_update_updates_print(const ln_update_t *pUpdates)
{
    LOGD("------------------------------------------\n");
    for (uint16_t idx = 0; idx < LN_UPDATE_MAX; idx++) {
        const ln_update_t *p_update = &pUpdates[idx];
        if (!LN_UPDATE_ENABLED(p_update)) continue;
        //LOGD("UPDATE[%d]: neighbor_short_channel_id=%016" PRIx64 "(%d)\n",
        //    idx, p_update->neighbor_short_channel_id, p_update->neighbor_idx);
        ln_update_print(p_update);
    }
    LOGD("------------------------------------------\n");
}
#endif