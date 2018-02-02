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
/** @file   ln_db_lmdb.h
 *  @brief  showdb用
 *  @author ueno@nayuta.co
 */
#ifndef LN_DB_LMDB_H__
#define LN_DB_LMDB_H__

#include "lmdb.h"

#include "ln.h"


#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/********************************************************************
 * LMDB
 ********************************************************************/


/**************************************************************************
 * typedefs
 **************************************************************************/

typedef enum {
    LN_LMDB_DBTYPE_UNKNOWN,
    LN_LMDB_DBTYPE_SELF,
    LN_LMDB_DBTYPE_SHARED_SECRET,
    LN_LMDB_DBTYPE_CHANNEL_ANNO,
    LN_LMDB_DBTYPE_NODE_ANNO,
    LN_LMDB_DBTYPE_PREIMAGE,
    LN_LMDB_DBTYPE_PAYHASH,
    LN_LMDB_DBTYPE_VERSION,
} ln_lmdb_dbtype_t;


typedef struct {
    MDB_txn     *txn;
    MDB_dbi     dbi;
} ln_lmdb_db_t;


/**************************************************************************
 * public functions
 **************************************************************************/

/** channel情報読込み
 *
 * @param[out]      self
 * @param[in]       txn
 * @param[in]       pdbi
 * @retval      true    成功
 * @attention
 *      -
 *      - 新規 self に読込を行う場合は、事前に #ln_self_ini()を行っておくこと(seedはNULLでよい)
 */
int ln_lmdb_load_channel(ln_self_t *self, MDB_txn *txn, MDB_dbi *pdbi);


/**
 *
 */
int ln_lmdb_load_anno_channel_cursor(MDB_cursor *cur, uint64_t *pShortChannelId, char *pType, uint32_t *pTimeStamp, ucoin_buf_t *pBuf);


/**
 *
 *
 */
int ln_lmdb_load_anno_node_cursor(MDB_cursor *cur, ucoin_buf_t *pBuf, uint32_t *pTimeStamp, uint8_t *pNodeId);


/**
 *
 *
 */
int ln_lmdb_check_version(ln_lmdb_db_t *pDb, uint8_t *pMyNodeId);


ln_lmdb_dbtype_t ln_lmdb_get_dbtype(const char *pDbName);


#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /* LN_DB_LMDB_H__ */
