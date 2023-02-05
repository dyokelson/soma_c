/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __SOMA_CLIENT_H
#define __SOMA_CLIENT_H

#include <margo.h>
#include <soma/soma-common.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct soma_client* soma_client_t;
#define SOMA_CLIENT_NULL ((soma_client_t)NULL)

/**
 * @brief Creates a SOMA client.
 *
 * @param[in] mid Margo instance
 * @param[out] client SOMA client
 *
 * @return SOMA_SUCCESS or error code defined in soma-common.h
 */
soma_return_t soma_client_init(margo_instance_id mid, soma_client_t* client);

/**
 * @brief Finalizes a SOMA client.
 *
 * @param[in] client SOMA client to finalize
 *
 * @return SOMA_SUCCESS or error code defined in soma-common.h
 */
soma_return_t soma_client_finalize(soma_client_t client);

#ifdef __cplusplus
}
#endif

#endif
