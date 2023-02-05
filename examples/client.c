/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <stdio.h>
#include <stdlib.h>
#include <margo.h>
#include <assert.h>
#include <soma/soma-client.h>
#include <soma/soma-collector.h>

#define FATAL(...) \
    do { \
        margo_critical(__VA_ARGS__); \
        exit(-1); \
    } while(0)

int main(int argc, char** argv)
{
    if(argc != 4) {
        fprintf(stderr,"Usage: %s <server address> <provider id> <collector id>\n", argv[0]);
        exit(-1);
    }

    soma_return_t ret;
    hg_return_t hret;
    const char* svr_addr_str = argv[1];
    uint16_t    provider_id  = atoi(argv[2]);
    const char* id_str       = argv[3];
    if(strlen(id_str) != 36) {
        FATAL(MARGO_INSTANCE_NULL,"id should be 36 character long");
    }

    margo_instance_id mid = margo_init("tcp", MARGO_CLIENT_MODE, 0, 0);
    assert(mid);

    hg_addr_t svr_addr;
    hret = margo_addr_lookup(mid, svr_addr_str, &svr_addr);
    if(hret != HG_SUCCESS) {
        FATAL(mid,"margo_addr_lookup failed for address %s", svr_addr_str);
    }

    soma_client_t soma_clt;
    soma_collector_handle_t soma_rh;

    margo_info(mid, "Creating SOMA client");
    ret = soma_client_init(mid, &soma_clt);
    if(ret != SOMA_SUCCESS) {
        FATAL(mid,"soma_client_init failed (ret = %d)", ret);
    }

    soma_collector_id_t collector_id;
    soma_collector_id_from_string(id_str, &collector_id);

    margo_info(mid, "Creating collector handle for collector %s", id_str);
    ret = soma_collector_handle_create(
            soma_clt, svr_addr, provider_id,
            collector_id, &soma_rh);
    if(ret != SOMA_SUCCESS) {
        FATAL(mid,"soma_collector_handle_create failed (ret = %d)", ret);
    }

    margo_info(mid, "Saying Hello to server");
    ret = soma_say_hello(soma_rh);
    if(ret != SOMA_SUCCESS) {
        FATAL(mid,"soma_say_hello failed (ret = %d)", ret);
    }

    margo_info(mid, "Computing sum");
    int32_t result;
    ret = soma_compute_sum(soma_rh, 45, 23, &result);
    if(ret != SOMA_SUCCESS) {
        FATAL(mid,"soma_compute_sum failed (ret = %d)", ret);
    }
    margo_info(mid, "45 + 23 = %d", result);

    margo_info(mid, "Releasing collector handle");
    ret = soma_collector_handle_release(soma_rh);
    if(ret != SOMA_SUCCESS) {
        FATAL(mid,"soma_collector_handle_release failed (ret = %d)", ret);
    }

    margo_info(mid, "Finalizing client");
    ret = soma_client_finalize(soma_clt);
    if(ret != SOMA_SUCCESS) {
        FATAL(mid,"soma_client_finalize failed (ret = %d)", ret);
    }

    hret = margo_addr_free(mid, svr_addr);
    if(hret != HG_SUCCESS) {
        FATAL(mid,"Could not free address (margo_addr_free returned %d)", hret);
    }

    margo_finalize(mid);

    return 0;
}
