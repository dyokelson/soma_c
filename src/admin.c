/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#include "types.h"
#include "admin.h"
#include "soma/soma-admin.h"

soma_return_t soma_admin_init(margo_instance_id mid, soma_admin_t* admin)
{
    soma_admin_t a = (soma_admin_t)calloc(1, sizeof(*a));
    if(!a) return SOMA_ERR_ALLOCATION;

    a->mid = mid;

    hg_bool_t flag;
    hg_id_t id;
    margo_registered_name(mid, "soma_create_collector", &id, &flag);

    if(flag == HG_TRUE) {
        margo_registered_name(mid, "soma_create_collector", &a->create_collector_id, &flag);
        margo_registered_name(mid, "soma_open_collector", &a->open_collector_id, &flag);
        margo_registered_name(mid, "soma_close_collector", &a->close_collector_id, &flag);
        margo_registered_name(mid, "soma_destroy_collector", &a->destroy_collector_id, &flag);
        margo_registered_name(mid, "soma_list_collectors", &a->list_collectors_id, &flag);
        /* Get more existing RPCs... */
    } else {
        a->create_collector_id =
            MARGO_REGISTER(mid, "soma_create_collector",
            create_collector_in_t, create_collector_out_t, NULL);
        a->open_collector_id =
            MARGO_REGISTER(mid, "soma_open_collector",
            open_collector_in_t, open_collector_out_t, NULL);
        a->close_collector_id =
            MARGO_REGISTER(mid, "soma_close_collector",
            close_collector_in_t, close_collector_out_t, NULL);
        a->destroy_collector_id =
            MARGO_REGISTER(mid, "soma_destroy_collector",
            destroy_collector_in_t, destroy_collector_out_t, NULL);
        a->list_collectors_id =
            MARGO_REGISTER(mid, "soma_list_collectors",
            list_collectors_in_t, list_collectors_out_t, NULL);
        /* Register more RPCs ... */
    }

    *admin = a;
    return SOMA_SUCCESS;
}

soma_return_t soma_admin_finalize(soma_admin_t admin)
{
    free(admin);
    return SOMA_SUCCESS;
}

soma_return_t soma_create_collector(
        soma_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        const char* type,
        const char* config,
        soma_collector_id_t* id)
{
    hg_handle_t h;
    create_collector_in_t  in;
    create_collector_out_t out;
    hg_return_t hret;
    soma_return_t ret;

    in.type   = (char*)type;
    in.config = (char*)config;
    in.token  = (char*)token;

    hret = margo_create(admin->mid, address, admin->create_collector_id, &h);
    if(hret != HG_SUCCESS)
        return SOMA_ERR_FROM_MERCURY;

    hret = margo_provider_forward(provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return SOMA_ERR_FROM_MERCURY;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return SOMA_ERR_FROM_MERCURY;
    }

    ret = out.ret;
    
    if(ret != SOMA_SUCCESS) {
        margo_free_output(h, &out);
        margo_destroy(h);
        return ret;
    }

    memcpy(id, &out.id, sizeof(*id));

    margo_free_output(h, &out);
    margo_destroy(h);
    return SOMA_SUCCESS;
}

soma_return_t soma_open_collector(
        soma_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        const char* type,
        const char* config,
        soma_collector_id_t* id)
{
    hg_handle_t h;
    open_collector_in_t  in;
    open_collector_out_t out;
    hg_return_t hret;
    soma_return_t ret;

    in.type   = (char*)type;
    in.config = (char*)config;
    in.token  = (char*)token;

    hret = margo_create(admin->mid, address, admin->open_collector_id, &h);
    if(hret != HG_SUCCESS)
        return SOMA_ERR_FROM_MERCURY;

    hret = margo_provider_forward(provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return SOMA_ERR_FROM_MERCURY;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return SOMA_ERR_FROM_MERCURY;
    }

    ret = out.ret;
    
    if(ret != SOMA_SUCCESS) {
        margo_free_output(h, &out);
        margo_destroy(h);
        return ret;
    }

    memcpy(id, &out.id, sizeof(*id));

    margo_free_output(h, &out);
    margo_destroy(h);
    return SOMA_SUCCESS;
}

soma_return_t soma_close_collector(
        soma_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        soma_collector_id_t id)
{
    hg_handle_t h;
    close_collector_in_t  in;
    close_collector_out_t out;
    hg_return_t hret;
    int ret;

    memcpy(&in.id, &id, sizeof(id));
    in.token  = (char*)token;

    hret = margo_create(admin->mid, address, admin->close_collector_id, &h);
    if(hret != HG_SUCCESS)
        return SOMA_ERR_FROM_MERCURY;

    hret = margo_provider_forward(provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return SOMA_ERR_FROM_MERCURY;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return SOMA_ERR_FROM_MERCURY;
    }

    ret = out.ret;

    margo_free_output(h, &out);
    margo_destroy(h);
    return ret;
}

soma_return_t soma_destroy_collector(
        soma_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        soma_collector_id_t id)
{
    hg_handle_t h;
    destroy_collector_in_t  in;
    destroy_collector_out_t out;
    hg_return_t hret;
    int ret;

    memcpy(&in.id, &id, sizeof(id));
    in.token  = (char*)token;

    hret = margo_create(admin->mid, address, admin->destroy_collector_id, &h);
    if(hret != HG_SUCCESS)
        return SOMA_ERR_FROM_MERCURY;

    hret = margo_provider_forward(provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return SOMA_ERR_FROM_MERCURY;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return SOMA_ERR_FROM_MERCURY;
    }

    ret = out.ret;

    margo_free_output(h, &out);
    margo_destroy(h);
    return ret;
}

soma_return_t soma_list_collectors(
        soma_admin_t admin,
        hg_addr_t address,
        uint16_t provider_id,
        const char* token,
        soma_collector_id_t* ids,
        size_t* count)
{
    hg_handle_t h;
    list_collectors_in_t  in;
    list_collectors_out_t out;
    soma_return_t ret;
    hg_return_t hret;

    in.token  = (char*)token;
    in.max_ids = *count;

    hret = margo_create(admin->mid, address, admin->list_collectors_id, &h);
    if(hret != HG_SUCCESS)
        return SOMA_ERR_FROM_MERCURY;

    hret = margo_provider_forward(provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return SOMA_ERR_FROM_MERCURY;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return SOMA_ERR_FROM_MERCURY;
    }

    ret = out.ret;
    if(ret == SOMA_SUCCESS) {
        *count = out.count;
        memcpy(ids, out.ids, out.count*sizeof(*ids));
    }
    
    margo_free_output(h, &out);
    margo_destroy(h);
    return ret;
}
