/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef __SOMA_BACKEND_H
#define __SOMA_BACKEND_H

#include <soma/soma-server.h>
#include <soma/soma-common.h>

typedef soma_return_t (*soma_backend_create_fn)(soma_provider_t, const char*, void**);
typedef soma_return_t (*soma_backend_open_fn)(soma_provider_t, const char*, void**);
typedef soma_return_t (*soma_backend_close_fn)(void*);
typedef soma_return_t (*soma_backend_destroy_fn)(void*);

/**
 * @brief Implementation of an SOMA backend.
 */
typedef struct soma_backend_impl {
    // backend name
    const char* name;
    // backend management functions
    soma_backend_create_fn   create_collector;
    soma_backend_open_fn     open_collector;
    soma_backend_close_fn    close_collector;
    soma_backend_destroy_fn  destroy_collector;
    // RPC functions
    void (*hello)(void*);
    int32_t (*sum)(void*, int32_t, int32_t);
    // ... add other functions here
} soma_backend_impl;

/**
 * @brief Registers a backend implementation to be used by the
 * specified provider.
 *
 * Note: the backend implementation will not be copied; it is
 * therefore important that it stays valid in memory until the
 * provider is destroyed.
 *
 * @param provider provider.
 * @param backend_impl backend implementation.
 *
 * @return SOMA_SUCCESS or error code defined in soma-common.h 
 */
soma_return_t soma_provider_register_backend(
        soma_provider_t provider,
        soma_backend_impl* backend_impl);

#endif
