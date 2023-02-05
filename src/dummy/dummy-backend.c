/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <string.h>
#include <json-c/json.h>
#include "soma/soma-backend.h"
#include "../provider.h"
#include "dummy-backend.h"

typedef struct dummy_context {
    struct json_object* config;
    /* ... */
} dummy_context;

static soma_return_t dummy_create_collector(
        soma_provider_t provider,
        const char* config_str,
        void** context)
{
    (void)provider;
    struct json_object* config = NULL;

    // read JSON config from provided string argument
    if (config_str) {
        struct json_tokener*    tokener = json_tokener_new();
        enum json_tokener_error jerr;
        config = json_tokener_parse_ex(
                tokener, config_str,
                strlen(config_str));
        if (!config) {
            jerr = json_tokener_get_error(tokener);
            margo_error(provider->mid, "JSON parse error: %s",
                      json_tokener_error_desc(jerr));
            json_tokener_free(tokener);
            return SOMA_ERR_INVALID_CONFIG;
        }
        json_tokener_free(tokener);
    } else {
        // create default JSON config
        config = json_object_new_object();
    }

    dummy_context* ctx = (dummy_context*)calloc(1, sizeof(*ctx));
    ctx->config = config;
    *context = (void*)ctx;
    return SOMA_SUCCESS;
}

static soma_return_t dummy_open_collector(
        soma_provider_t provider,
        const char* config_str,
        void** context)
{
    (void)provider;

    struct json_object* config = NULL;

    // read JSON config from provided string argument
    if (config_str) {
        struct json_tokener*    tokener = json_tokener_new();
        enum json_tokener_error jerr;
        config = json_tokener_parse_ex(
                tokener, config_str,
                strlen(config_str));
        if (!config) {
            jerr = json_tokener_get_error(tokener);
            margo_error(provider->mid, "JSON parse error: %s",
                      json_tokener_error_desc(jerr));
            json_tokener_free(tokener);
            return SOMA_ERR_INVALID_CONFIG;
        }
        json_tokener_free(tokener);
    } else {
        // create default JSON config
        config = json_object_new_object();
    }

    dummy_context* ctx = (dummy_context*)calloc(1, sizeof(*ctx));
    ctx->config = config;
    *context = (void*)ctx;
    return SOMA_SUCCESS;
}

static soma_return_t dummy_close_collector(void* ctx)
{
    dummy_context* context = (dummy_context*)ctx;
    json_object_put(context->config);
    free(context);
    return SOMA_SUCCESS;
}

static soma_return_t dummy_destroy_collector(void* ctx)
{
    dummy_context* context = (dummy_context*)ctx;
    json_object_put(context->config);
    free(context);
    return SOMA_SUCCESS;
}

static void dummy_say_hello(void* ctx)
{
    dummy_context* context = (dummy_context*)ctx;
    (void)context;
    printf("Hello World from Dummy collector\n");
}

static int32_t dummy_compute_sum(void* ctx, int32_t x, int32_t y)
{
    (void)ctx;
    return x+y;
}

static soma_backend_impl dummy_backend = {
    .name             = "dummy",

    .create_collector  = dummy_create_collector,
    .open_collector    = dummy_open_collector,
    .close_collector   = dummy_close_collector,
    .destroy_collector = dummy_destroy_collector,

    .hello            = dummy_say_hello,
    .sum              = dummy_compute_sum
};

soma_return_t soma_provider_register_dummy_backend(soma_provider_t provider)
{
    return soma_provider_register_backend(provider, &dummy_backend);
}
