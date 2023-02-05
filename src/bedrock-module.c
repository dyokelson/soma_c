/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <bedrock/module.h>
#include "soma/soma-server.h"
#include "soma/soma-client.h"
#include "soma/soma-admin.h"
#include "soma/soma-provider-handle.h"
#include "client.h"
#include <string.h>

static int soma_register_provider(
        bedrock_args_t args,
        bedrock_module_provider_t* provider)
{
    margo_instance_id mid = bedrock_args_get_margo_instance(args);
    uint16_t provider_id  = bedrock_args_get_provider_id(args);

    struct soma_provider_args soma_args = { 0 };
    soma_args.config = bedrock_args_get_config(args);
    soma_args.pool   = bedrock_args_get_pool(args);

    soma_args.abtio = (abt_io_instance_id)
        bedrock_args_get_dependency(args, "abt_io", 0);

    return soma_provider_register(mid, provider_id, &soma_args,
                                   (soma_provider_t*)provider);
}

static int soma_deregister_provider(
        bedrock_module_provider_t provider)
{
    return soma_provider_destroy((soma_provider_t)provider);
}

static char* soma_get_provider_config(
        bedrock_module_provider_t provider) {
    (void)provider;
    // TODO
    return strdup("{}");
}

static int soma_init_client(
        bedrock_args_t args,
        bedrock_module_client_t* client)
{
    margo_instance_id mid = bedrock_args_get_margo_instance(args);
    return soma_client_init(mid, (soma_client_t*)client);
}

static int soma_finalize_client(
        bedrock_module_client_t client)
{
    return soma_client_finalize((soma_client_t)client);
}

static char* soma_get_client_config(
        bedrock_module_client_t client) {
    (void)client;
    // TODO
    return strdup("{}");
}

static int soma_create_provider_handle(
        bedrock_module_client_t client,
        hg_addr_t address,
        uint16_t provider_id,
        bedrock_module_provider_handle_t* ph)
{
    soma_client_t c = (soma_client_t)client;
    soma_provider_handle_t tmp = calloc(1, sizeof(*tmp));
    margo_addr_dup(c->mid, address, &(tmp->addr));
    tmp->provider_id = provider_id;
    *ph = (bedrock_module_provider_handle_t)tmp;
    return BEDROCK_SUCCESS;
}

static int soma_destroy_provider_handle(
        bedrock_module_provider_handle_t ph)
{
    soma_provider_handle_t tmp = (soma_provider_handle_t)ph;
    margo_addr_free(tmp->mid, tmp->addr);
    free(tmp);
    return BEDROCK_SUCCESS;
}

static struct bedrock_module soma = {
    .register_provider       = soma_register_provider,
    .deregister_provider     = soma_deregister_provider,
    .get_provider_config     = soma_get_provider_config,
    .init_client             = soma_init_client,
    .finalize_client         = soma_finalize_client,
    .get_client_config       = soma_get_client_config,
    .create_provider_handle  = soma_create_provider_handle,
    .destroy_provider_handle = soma_destroy_provider_handle,
    .provider_dependencies   = NULL,
    .client_dependencies     = NULL
};

BEDROCK_REGISTER_MODULE(soma, soma)
