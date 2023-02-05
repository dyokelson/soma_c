/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __SOMA_COMMON_H
#define __SOMA_COMMON_H

#include <uuid.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Error codes that can be returned by SOMA functions.
 */
typedef enum soma_return_t {
    SOMA_SUCCESS,
    SOMA_ERR_ALLOCATION,        /* Allocation error */
    SOMA_ERR_INVALID_ARGS,      /* Invalid argument */
    SOMA_ERR_INVALID_PROVIDER,  /* Invalid provider id */
    SOMA_ERR_INVALID_COLLECTOR,  /* Invalid collector id */
    SOMA_ERR_INVALID_BACKEND,   /* Invalid backend type */
    SOMA_ERR_INVALID_CONFIG,    /* Invalid configuration */
    SOMA_ERR_INVALID_TOKEN,     /* Invalid token */
    SOMA_ERR_FROM_MERCURY,      /* Mercurt error */
    SOMA_ERR_FROM_ARGOBOTS,     /* Argobots error */
    SOMA_ERR_OP_UNSUPPORTED,    /* Unsupported operation */
    SOMA_ERR_OP_FORBIDDEN,      /* Forbidden operation */
    /* ... TODO add more error codes here if needed */
    SOMA_ERR_OTHER              /* Other error */
} soma_return_t;

/**
 * @brief Identifier for a collector.
 */
typedef struct soma_collector_id_t {
    uuid_t uuid;
} soma_collector_id_t;

/**
 * @brief Converts a soma_collector_id_t into a string.
 *
 * @param id Id to convert
 * @param out[37] Resulting null-terminated string
 */
static inline void soma_collector_id_to_string(
        soma_collector_id_t id,
        char out[37]) {
    uuid_unparse(id.uuid, out);
}

/**
 * @brief Converts a string into a soma_collector_id_t. The string
 * should be a 36-characters string + null terminator.
 *
 * @param in input string
 * @param id resulting id
 */
static inline void soma_collector_id_from_string(
        const char* in,
        soma_collector_id_t* id) {
    uuid_parse(in, id->uuid);
}

#ifdef __cplusplus
}
#endif

#endif
