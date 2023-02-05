/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef __SOMA_COLLECTOR_H
#define __SOMA_COLLECTOR_H

#include <margo.h>
#include <soma/soma-common.h>
#include <soma/soma-client.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct soma_collector_handle *soma_collector_handle_t;
#define SOMA_COLLECTOR_HANDLE_NULL ((soma_collector_handle_t)NULL)

/**
 * @brief Creates a SOMA collector handle.
 *
 * @param[in] client SOMA client responsible for the collector handle
 * @param[in] addr Mercury address of the provider
 * @param[in] provider_id id of the provider
 * @param[in] handle collector handle
 *
 * @return SOMA_SUCCESS or error code defined in soma-common.h
 */
soma_return_t soma_collector_handle_create(
        soma_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        soma_collector_id_t collector_id,
        soma_collector_handle_t* handle);

/**
 * @brief Increments the reference counter of a collector handle.
 *
 * @param handle collector handle
 *
 * @return SOMA_SUCCESS or error code defined in soma-common.h
 */
soma_return_t soma_collector_handle_ref_incr(
        soma_collector_handle_t handle);

/**
 * @brief Releases the collector handle. This will decrement the
 * reference counter, and free the collector handle if the reference
 * counter reaches 0.
 *
 * @param[in] handle collector handle to release.
 *
 * @return SOMA_SUCCESS or error code defined in soma-common.h
 */
soma_return_t soma_collector_handle_release(soma_collector_handle_t handle);

/**
 * @brief Makes the target SOMA collector print Hello World.
 *
 * @param[in] handle collector handle.
 * @param[in] x first number.
 * @param[in] y second number.
 * @param[out] result resulting value.
 *
 * @return SOMA_SUCCESS or error code defined in soma-common.h
 */
soma_return_t soma_say_hello(soma_collector_handle_t handle);

/**
 * @brief Makes the target SOMA collector compute the sum of the
 * two numbers and return the result.
 *
 * @param[in] handle collector handle.
 * @param[in] x first number.
 * @param[in] y second number.
 * @param[out] result resulting value.
 *
 * @return SOMA_SUCCESS or error code defined in soma-common.h
 */
soma_return_t soma_compute_sum(
        soma_collector_handle_t handle,
        int32_t x,
        int32_t y,
        int32_t* result);

#ifdef __cplusplus
}
#endif

#endif
