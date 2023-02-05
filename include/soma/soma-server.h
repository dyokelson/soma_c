/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __SOMA_SERVER_H
#define __SOMA_SERVER_H

#include <soma/soma-common.h>
#include <margo.h>
#include <abt-io.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SOMA_ABT_POOL_DEFAULT ABT_POOL_NULL

typedef struct soma_provider* soma_provider_t;
#define SOMA_PROVIDER_NULL ((soma_provider_t)NULL)
#define SOMA_PROVIDER_IGNORE ((soma_provider_t*)NULL)

struct soma_provider_args {
    const char*        token;  // Security token
    const char*        config; // JSON configuration
    ABT_pool           pool;   // Pool used to run RPCs
    abt_io_instance_id abtio;  // ABT-IO instance
    // ...
};

#define SOMA_PROVIDER_ARGS_INIT { \
    .token = NULL, \
    .config = NULL, \
    .pool = ABT_POOL_NULL, \
    .abtio = ABT_IO_INSTANCE_NULL \
}

/**
 * @brief Creates a new SOMA provider. If SOMA_PROVIDER_IGNORE
 * is passed as last argument, the provider will be automatically
 * destroyed when calling margo_finalize.
 *
 * @param[in] mid Margo instance
 * @param[in] provider_id provider id
 * @param[in] args argument structure
 * @param[out] provider provider
 *
 * @return SOMA_SUCCESS or error code defined in soma-common.h
 */
int soma_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        const struct soma_provider_args* args,
        soma_provider_t* provider);

/**
 * @brief Destroys the Alpha provider and deregisters its RPC.
 *
 * @param[in] provider Alpha provider
 *
 * @return SOMA_SUCCESS or error code defined in soma-common.h
 */
int soma_provider_destroy(
        soma_provider_t provider);

#ifdef __cplusplus
}
#endif

#endif
