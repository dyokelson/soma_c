/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __SOMA_PROVIDER_HANDLE_H
#define __SOMA_PROVIDER_HANDLE_H

#include <margo.h>
#include <soma/soma-common.h>

#ifdef __cplusplus
extern "C" {
#endif

struct soma_provider_handle {
    margo_instance_id mid;
    hg_addr_t         addr;
    uint16_t          provider_id;
};

typedef struct soma_provider_handle* soma_provider_handle_t;
#define SOMA_PROVIDER_HANDLE_NULL ((soma_provider_handle_t)NULL)

#ifdef __cplusplus
}
#endif

#endif
