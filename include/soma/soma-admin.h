/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef __SOMA_ADMIN_H
#define __SOMA_ADMIN_H

#include <margo.h>
#include <soma/soma-common.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct soma_admin* soma_admin_t;
#define SOMA_ADMIN_NULL ((soma_admin_t)NULL)

#define SOMA_COLLECTOR_ID_IGNORE ((soma_collector_id_t*)NULL)

/**
 * @brief Creates a SOMA admin.
 *
 * @param[in] mid Margo instance
 * @param[out] admin SOMA admin
 *
 * @return SOMA_SUCCESS or error code defined in soma-common.h
 */
soma_return_t soma_admin_init(margo_instance_id mid, soma_admin_t* admin);

/**
 * @brief Finalizes a SOMA admin.
 *
 * @param[in] admin SOMA admin to finalize
 *
 * @return SOMA_SUCCESS or error code defined in soma-common.h
 */
soma_return_t soma_admin_finalize(soma_admin_t admin);

/**
 * @brief Requests the provider to create a collector of the
 * specified type and configuration and return a collector id.
 *
 * @param[in] admin SOMA admin object.
 * @param[in] address address of the provider.
 * @param[in] provider_id provider id.
 * @param[in] type type of collector to create.
 * @param[in] config Configuration.
 * @param[out] id resulting collector id.
 *
 * @return SOMA_SUCCESS or error code defined in soma-common.h
 */
soma_return_t soma_create_collector(
        soma_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        const char* type,
        const char* config,
        soma_collector_id_t* id);

/**
 * @brief Requests the provider to open an existing collector of the
 * specified type and configuration and return a collector id.
 *
 * @param[in] admin SOMA admin object.
 * @param[in] address address of the provider.
 * @param[in] provider_id provider id.
 * @param[in] token security token.
 * @param[in] type type of collector to open.
 * @param[in] config Configuration.
 * @param[out] id resulting collector id.
 *
 * @return SOMA_SUCCESS or error code defined in soma-common.h
 */
soma_return_t soma_open_collector(
        soma_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        const char* type,
        const char* config,
        soma_collector_id_t* id);

/**
 * @brief Requests the provider to close a collector it is managing.
 *
 * @param[in] admin SOMA admin object.
 * @param[in] address address of the provider.
 * @param[in] provider_id provider id.
 * @param[in] token security token.
 * @param[in] id resulting collector id.
 *
 * @return SOMA_SUCCESS or error code defined in soma-common.h
 */
soma_return_t soma_close_collector(
        soma_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        soma_collector_id_t id);

/**
 * @brief Requests the provider to destroy a collector it is managing.
 *
 * @param[in] admin SOMA admin object.
 * @param[in] address address of the provider.
 * @param[in] provider_id provider id.
 * @param[in] token security token.
 * @param[in] id resulting collector id.
 *
 * @return SOMA_SUCCESS or error code defined in soma-common.h
 */
soma_return_t soma_destroy_collector(
        soma_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        soma_collector_id_t id);

/**
 * @brief Lists the ids of collectors available on the provider.
 *
 * @param[in] admin SOMA admin object.
 * @param[in] address address of the provider.
 * @param[in] provider_id provider id.
 * @param[in] token security token.
 * @param[out] ids array of collector ids.
 * @param[inout] count size of the array (in), number of ids returned (out).
 *
 * @return SOMA_SUCCESS or error code defined in soma-common.h
 */
soma_return_t soma_list_collectors(
        soma_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        soma_collector_id_t* ids,
        size_t* count);

#endif
