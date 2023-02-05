/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef _CLIENT_H
#define _CLIENT_H

#include "types.h"
#include "soma/soma-client.h"
#include "soma/soma-collector.h"

typedef struct soma_client {
   margo_instance_id mid;
   hg_id_t           hello_id;
   hg_id_t           sum_id;
   uint64_t          num_collector_handles;
} soma_client;

typedef struct soma_collector_handle {
    soma_client_t      client;
    hg_addr_t           addr;
    uint16_t            provider_id;
    uint64_t            refcount;
    soma_collector_id_t collector_id;
} soma_collector_handle;

#endif
