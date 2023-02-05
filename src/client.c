/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#include "types.h"
#include "client.h"
#include "soma/soma-client.h"

soma_return_t soma_client_init(margo_instance_id mid, soma_client_t* client)
{
    soma_client_t c = (soma_client_t)calloc(1, sizeof(*c));
    if(!c) return SOMA_ERR_ALLOCATION;

    c->mid = mid;

    hg_bool_t flag;
    hg_id_t id;
    margo_registered_name(mid, "soma_sum", &id, &flag);

    if(flag == HG_TRUE) {
        margo_registered_name(mid, "soma_sum", &c->sum_id, &flag);
        margo_registered_name(mid, "soma_hello", &c->hello_id, &flag);
    } else {
        c->sum_id = MARGO_REGISTER(mid, "soma_sum", sum_in_t, sum_out_t, NULL);
        c->hello_id = MARGO_REGISTER(mid, "soma_hello", hello_in_t, void, NULL);
        margo_registered_disable_response(mid, c->hello_id, HG_TRUE);
    }

    *client = c;
    return SOMA_SUCCESS;
}

soma_return_t soma_client_finalize(soma_client_t client)
{
    if(client->num_collector_handles != 0) {
        fprintf(stderr,  
                "Warning: %ld collector handles not released when soma_client_finalize was called\n",
                client->num_collector_handles);
    }
    free(client);
    return SOMA_SUCCESS;
}

soma_return_t soma_collector_handle_create(
        soma_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        soma_collector_id_t collector_id,
        soma_collector_handle_t* handle)
{
    if(client == SOMA_CLIENT_NULL)
        return SOMA_ERR_INVALID_ARGS;

    soma_collector_handle_t rh =
        (soma_collector_handle_t)calloc(1, sizeof(*rh));

    if(!rh) return SOMA_ERR_ALLOCATION;

    hg_return_t ret = margo_addr_dup(client->mid, addr, &(rh->addr));
    if(ret != HG_SUCCESS) {
        free(rh);
        return SOMA_ERR_FROM_MERCURY;
    }

    rh->client      = client;
    rh->provider_id = provider_id;
    rh->collector_id = collector_id;
    rh->refcount    = 1;

    client->num_collector_handles += 1;

    *handle = rh;
    return SOMA_SUCCESS;
}

soma_return_t soma_collector_handle_ref_incr(
        soma_collector_handle_t handle)
{
    if(handle == SOMA_COLLECTOR_HANDLE_NULL)
        return SOMA_ERR_INVALID_ARGS;
    handle->refcount += 1;
    return SOMA_SUCCESS;
}

soma_return_t soma_collector_handle_release(soma_collector_handle_t handle)
{
    if(handle == SOMA_COLLECTOR_HANDLE_NULL)
        return SOMA_ERR_INVALID_ARGS;
    handle->refcount -= 1;
    if(handle->refcount == 0) {
        margo_addr_free(handle->client->mid, handle->addr);
        handle->client->num_collector_handles -= 1;
        free(handle);
    }
    return SOMA_SUCCESS;
}

soma_return_t soma_say_hello(soma_collector_handle_t handle)
{
    hg_handle_t   h;
    hello_in_t     in;
    hg_return_t ret;

    memcpy(&in.collector_id, &(handle->collector_id), sizeof(in.collector_id));

    ret = margo_create(handle->client->mid, handle->addr, handle->client->hello_id, &h);
    if(ret != HG_SUCCESS)
        return SOMA_ERR_FROM_MERCURY;

    ret = margo_provider_forward(handle->provider_id, h, &in);
    if(ret != HG_SUCCESS) {
        margo_destroy(h);
        return SOMA_ERR_FROM_MERCURY;
    }

    margo_destroy(h);
    return SOMA_SUCCESS;
}

soma_return_t soma_compute_sum(
        soma_collector_handle_t handle,
        int32_t x,
        int32_t y,
        int32_t* result)
{
    hg_handle_t   h;
    sum_in_t     in;
    sum_out_t   out;
    hg_return_t hret;
    soma_return_t ret;

    memcpy(&in.collector_id, &(handle->collector_id), sizeof(in.collector_id));
    in.x = x;
    in.y = y;

    hret = margo_create(handle->client->mid, handle->addr, handle->client->sum_id, &h);
    if(hret != HG_SUCCESS)
        return SOMA_ERR_FROM_MERCURY;

    hret = margo_provider_forward(handle->provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        ret = SOMA_ERR_FROM_MERCURY;
        goto finish;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        ret = SOMA_ERR_FROM_MERCURY;
        goto finish;
    }

    ret = out.ret;
    if(ret == SOMA_SUCCESS)
        *result = out.result;

finish:
    margo_free_output(h, &out);
    margo_destroy(h);
    return ret;
}
