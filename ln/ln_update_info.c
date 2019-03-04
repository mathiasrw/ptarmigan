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
/** @file   ln_update_info.c
 *  @brief  ln_update_info
 */
#include <inttypes.h>

#include "ln_db_lmdb.h"

#include "ln_update_info.h"


/**************************************************************************
 * public functions
 **************************************************************************/

void ln_update_info_init(ln_update_info_t *pInfo) {
    memset(pInfo, 0x00, sizeof(ln_update_info_t));
    for (uint16_t idx = 0; idx < ARRAY_SIZE(pInfo->htlcs); idx++) {
        utl_buf_init(&pInfo->htlcs[idx].buf_payment_preimage);
        utl_buf_init(&pInfo->htlcs[idx].buf_onion_reason);
        utl_buf_init(&pInfo->htlcs[idx].buf_shared_secret);
    }
}


void ln_update_info_free(ln_update_info_t *pInfo) {
    for (uint16_t idx = 0; idx < ARRAY_SIZE(pInfo->htlcs); idx++) {
        utl_buf_free(&pInfo->htlcs[idx].buf_payment_preimage);
        utl_buf_free(&pInfo->htlcs[idx].buf_onion_reason);
        utl_buf_free(&pInfo->htlcs[idx].buf_shared_secret);
    }
    memset(pInfo, 0x00, sizeof(ln_update_info_t));
}


bool ln_update_info_set_add_htlc_pre_send(ln_update_info_t *pInfo, uint16_t *pUpdateIdx)
{
    uint16_t update_idx;
    ln_update_t *p_update = ln_update_get_empty(pInfo->updates, &update_idx);
    if (!p_update) return false;
    uint16_t htlc_idx;
    ln_htlc_t *p_htlc = ln_htlc_get_empty(pInfo->htlcs, &htlc_idx);
    if (!p_htlc) return false;
    p_update->htlc_idx = htlc_idx;
    p_update->enabled = true;
    p_htlc->enabled = true;
    //p_update->type = LN_UPDATE_TYPE_ADD_HTLC;
        //XXX: Do not set the `type` as it is not currently in a state that can be sent.
        //  (Because the forwarding of `update_add_htlc` requires multiple callbacks)
        //  Eventually, the forwarding is completed immediately and the `type` is set here
        //  so that the sending `update_add_htlc` becomes possible.
    //p_update->flags.up_send = 1; //NOT set the flag, pre send
    *pUpdateIdx = update_idx;
    return true;
}


bool ln_update_info_set_add_htlc_recv(ln_update_info_t *pInfo, uint16_t *pUpdateIdx)
{
    uint16_t update_idx;
    ln_update_t *p_update = ln_update_get_empty(pInfo->updates, &update_idx);
    if (!p_update) return false;
    uint16_t htlc_idx;
    ln_htlc_t *p_htlc = ln_htlc_get_empty(pInfo->htlcs, &htlc_idx);
    if (!p_htlc) return false;
    p_update->htlc_idx = htlc_idx;
    p_update->enabled = true;
    p_htlc->enabled = true;
    p_update->type = LN_UPDATE_TYPE_ADD_HTLC;
    LN_UPDATE_FLAG_SET(p_update, LN_UPDATE_STATE_FLAG_UP_RECV);
    *pUpdateIdx = update_idx;
    return true;
}


bool ln_update_info_clear_htlc(ln_update_info_t *pInfo, uint16_t UpdateIdx)
{
    if (UpdateIdx >= ARRAY_SIZE(pInfo->updates)) {
        assert(0);
        return false;
    }

    ln_update_t *p_update = &pInfo->updates[UpdateIdx];
    if (!(p_update->type & LN_UPDATE_TYPE_MASK_HTLC)) {
        memset(p_update, 0x00, sizeof(ln_update_t));
        return true;
    }

    if (p_update->htlc_idx >= ARRAY_SIZE(pInfo->htlcs)) {
        assert(0);
        return false;
    }

    //clear htlc
    ln_htlc_t *p_htlc = &pInfo->htlcs[p_update->htlc_idx];
    if (p_htlc->buf_payment_preimage.len) {
        /*ignore*/ ln_db_preimage_del(p_htlc->buf_payment_preimage.buf); //XXX: delete outside the function
    }
    utl_buf_free(&p_htlc->buf_payment_preimage);
    utl_buf_free(&p_htlc->buf_onion_reason);
    utl_buf_free(&p_htlc->buf_shared_secret);
    memset(p_htlc, 0x00, sizeof(ln_htlc_t));

    //clear corresponding update (add -> del, del -> add)
    uint16_t corresponding_update_idx;
    if (ln_update_info_get_corresponding_update(pInfo, &corresponding_update_idx, UpdateIdx)) {
        memset(&pInfo->updates[corresponding_update_idx], 0x00, sizeof(ln_update_t));
    }

    //clear update
    memset(p_update, 0x00, sizeof(ln_update_t));
    return true;
}


bool ln_update_info_get_corresponding_update(
    const ln_update_info_t *pInfo, uint16_t *pCorrespondingUpdateIdx, uint16_t UpdateIdx)
{
    const ln_update_t *p_update = &pInfo->updates[UpdateIdx];
    if (!LN_UPDATE_USED(p_update)) return false;
    if (!(p_update->type & LN_UPDATE_TYPE_MASK_HTLC)) {
        return false;
    }
    for (uint16_t idx = 0; idx < LN_UPDATE_MAX; idx++) {
        if (idx == UpdateIdx) continue; //skip myself
        const ln_update_t *p_update_2 = &pInfo->updates[idx];
        if (!LN_UPDATE_USED(p_update_2)) continue;
        if (!(p_update_2->type & LN_UPDATE_TYPE_MASK_HTLC)) continue;
        if (p_update_2->htlc_idx != p_update->htlc_idx) continue;
        *pCorrespondingUpdateIdx = idx;
        return true;
    }
    return false;
}


bool ln_update_info_set_del_htlc_pre_send(ln_update_info_t *pInfo, uint16_t *pUpdateIdx, uint16_t HtlcIdx, uint8_t Type)
{
    assert(Type & LN_UPDATE_TYPE_MASK_DEL_HTLC);

    if (ln_update_info_get_update_del_htlc_const(pInfo, HtlcIdx)) {
        //I have already received it
        return false;
    }

    uint16_t update_idx;
    ln_update_t *p_update = ln_update_get_empty(pInfo->updates, &update_idx);
    if (!p_update) {
        return false;
    }

    p_update->enabled = true;
    p_update->type = Type;
    //p_update->flags.up_send = 1; //NOT set the flag, pre send
    p_update->htlc_idx = HtlcIdx;
    *pUpdateIdx = update_idx;
    return true;
}


bool ln_update_info_set_del_htlc_recv(ln_update_info_t *pInfo, uint16_t *pUpdateIdx, uint16_t HtlcIdx, uint8_t Type)
{
    assert(Type & LN_UPDATE_TYPE_MASK_DEL_HTLC);

    if (ln_update_info_get_update_del_htlc_const(pInfo, HtlcIdx)) {
        //I have already received it
        return false;
    }

    uint16_t update_idx;
    ln_update_t *p_update = ln_update_get_empty(pInfo->updates, &update_idx);
    if (!p_update) {
        return false;
    }

    p_update->enabled = true;
    p_update->type = Type;
    LN_UPDATE_FLAG_SET(p_update, LN_UPDATE_STATE_FLAG_UP_RECV);
    p_update->htlc_idx = HtlcIdx;
    *pUpdateIdx = update_idx;
    return true;
}


ln_update_t *ln_update_info_get_update_enabled_but_none(ln_update_info_t *pInfo, uint16_t HtlcIdx)
{
    for (uint16_t idx = 0; idx < LN_UPDATE_MAX; idx++) {
        ln_update_t *p_update = &pInfo->updates[idx];
        if (!p_update->enabled) continue;
        if (p_update->type != LN_UPDATE_TYPE_NONE) continue;
        if (p_update->htlc_idx != HtlcIdx) continue;
        return p_update;
    }
    return NULL;
}


ln_update_t *ln_update_info_get_update_add_htlc(ln_update_info_t *pInfo, uint16_t HtlcIdx)
{
    for (uint16_t idx = 0; idx < LN_UPDATE_MAX; idx++) {
        ln_update_t *p_update = &pInfo->updates[idx];
        if (!LN_UPDATE_USED(p_update)) continue;
        if (p_update->type != LN_UPDATE_TYPE_ADD_HTLC) continue;
        if (p_update->htlc_idx != HtlcIdx) continue;
        return p_update;
    }
    return NULL;
}

ln_update_t *ln_update_info_get_update_del_htlc(ln_update_info_t *pInfo, uint16_t HtlcIdx)
{
    for (uint16_t idx = 0; idx < LN_UPDATE_MAX; idx++) {
        ln_update_t *p_update = &pInfo->updates[idx];
        if (!LN_UPDATE_USED(p_update)) continue;
        if (!(p_update->type & LN_UPDATE_TYPE_MASK_DEL_HTLC)) continue;
        if (p_update->htlc_idx != HtlcIdx) continue;
        return p_update;
    }
    return NULL;
}


const ln_update_t *ln_update_info_get_update_del_htlc_const(const ln_update_info_t *pInfo, uint16_t HtlcIdx)
{
    for (uint16_t idx = 0; idx < LN_UPDATE_MAX; idx++) {
        const ln_update_t *p_update = &pInfo->updates[idx];
        if (!LN_UPDATE_USED(p_update)) continue;
        if (!(p_update->type & LN_UPDATE_TYPE_MASK_DEL_HTLC)) continue;
        if (p_update->htlc_idx != HtlcIdx) continue;
        return p_update;
    }
    return NULL;
}


bool ln_update_info_irrevocably_committed_htlcs_exists(ln_update_info_t *pInfo)
{
    for (uint16_t idx = 0; idx < LN_UPDATE_MAX; idx++) {
        ln_update_t *p_update = &pInfo->updates[idx];
        if (!LN_UPDATE_USED(p_update)) continue;
        if (!LN_UPDATE_IRREVOCABLY_COMMITTED(p_update)) continue;
        if (!(p_update->type & LN_UPDATE_TYPE_MASK_DEL_HTLC)) continue;
        return true;
    }
    return false;
}


bool ln_update_info_commitment_signed_send_needs(ln_update_info_t *pInfo)
{
    for (uint16_t idx = 0; idx < LN_UPDATE_MAX; idx++) {
        ln_update_t *p_update = &pInfo->updates[idx];
        if (!LN_UPDATE_USED(p_update)) continue;
        if (!LN_UPDATE_WAIT_SEND_CS(p_update)) continue;
        return true;
    }
    return false;
}


void ln_update_info_clear_irrevocably_committed_htlcs(ln_update_info_t *pInfo)
{
    for (uint16_t idx = 0; idx < LN_UPDATE_MAX; idx++) {
        ln_update_t *p_update = &pInfo->updates[idx];
        if (!LN_UPDATE_USED(p_update)) continue;
        if (!LN_UPDATE_IRREVOCABLY_COMMITTED(p_update)) continue;
        if (!(p_update->type & LN_UPDATE_TYPE_MASK_DEL_HTLC)) continue;
        /*ignore*/ ln_update_info_clear_htlc(pInfo, idx);
    }
}


void ln_update_info_set_state_flag_all(ln_update_info_t *pInfo, uint8_t flag)
{
    switch (flag) {
    case LN_UPDATE_STATE_FLAG_CS_SEND:
        for (uint16_t idx = 0; idx < ARRAY_SIZE(pInfo->updates); idx++) {
            ln_update_t *p_update = &pInfo->updates[idx];
            if (!LN_UPDATE_USED(p_update)) continue;
            switch (p_update->state) {
            case LN_UPDATE_STATE_OFFERED_UP_SEND:
            case LN_UPDATE_STATE_RECEIVED_RA_SEND:
                LN_UPDATE_FLAG_SET(p_update, flag);
                break;
            default:
                ;
            }
        }
        break;
    case LN_UPDATE_STATE_FLAG_CS_RECV:
        for (uint16_t idx = 0; idx < ARRAY_SIZE(pInfo->updates); idx++) {
            ln_update_t *p_update = &pInfo->updates[idx];
            if (!LN_UPDATE_USED(p_update)) continue;
            switch (p_update->state) {
            case LN_UPDATE_STATE_OFFERED_RA_RECV:
            case LN_UPDATE_STATE_RECEIVED_UP_RECV:
                LN_UPDATE_FLAG_SET(p_update, flag);
                break;
            default:
                ;
            }
        }
        break;
    case LN_UPDATE_STATE_FLAG_RA_SEND:
        for (uint16_t idx = 0; idx < ARRAY_SIZE(pInfo->updates); idx++) {
            ln_update_t *p_update = &pInfo->updates[idx];
            if (!LN_UPDATE_USED(p_update)) continue;
            switch (p_update->state) {
            case LN_UPDATE_STATE_OFFERED_CS_RECV:
            case LN_UPDATE_STATE_RECEIVED_CS_RECV:
                LN_UPDATE_FLAG_SET(p_update, flag);
                break;
            default:
                ;
            }
        }
        break;
    case LN_UPDATE_STATE_FLAG_RA_RECV:
        for (uint16_t idx = 0; idx < ARRAY_SIZE(pInfo->updates); idx++) {
            ln_update_t *p_update = &pInfo->updates[idx];
            if (!LN_UPDATE_USED(p_update)) continue;
            switch (p_update->state) {
            case LN_UPDATE_STATE_OFFERED_CS_SEND:
            case LN_UPDATE_STATE_RECEIVED_CS_SEND:
                LN_UPDATE_FLAG_SET(p_update, flag);
                break;
            default:
                ;
            }
        }
        break;
    default:
        assert(0);
    }
}


void ln_update_info_reset_new_update(ln_update_info_t *pInfo) {
    for (uint16_t idx = 0; idx < ARRAY_SIZE(pInfo->updates); idx++) {
        ln_update_t *p_update = &pInfo->updates[idx];
        p_update->new_update = false;
    }
}


uint64_t ln_update_info_htlc_value_in_flight_msat(ln_update_info_t *pInfo, bool bLocal)
{
    uint64_t value = 0;
    for (uint16_t idx; idx < ARRAY_SIZE(pInfo->updates); idx++) {
        ln_update_t *p_update = &pInfo->updates[idx];
        if (!LN_UPDATE_USED(p_update)) continue;
        if (LN_UPDATE_RECV_ENABLED(p_update, LN_UPDATE_TYPE_ADD_HTLC, bLocal)) {
            value += pInfo->htlcs[p_update->htlc_idx].amount_msat;
        }
        if (LN_UPDATE_SEND_ENABLED(p_update, LN_UPDATE_TYPE_MASK_DEL_HTLC, bLocal)) {
            value -= pInfo->htlcs[p_update->htlc_idx].amount_msat;
        }
    }
    return value;
}
