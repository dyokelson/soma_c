/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "soma/soma-server.h"
#include "provider.h"
#include "types.h"

// backends that we want to add at compile time
#include "dummy/dummy-backend.h"

static void soma_finalize_provider(void* p);

/* Functions to manipulate the hash of collectors */
static inline soma_collector* find_collector(
        soma_provider_t provider,
        const soma_collector_id_t* id);

static inline soma_return_t add_collector(
        soma_provider_t provider,
        soma_collector* collector);

static inline soma_return_t remove_collector(
        soma_provider_t provider,
        const soma_collector_id_t* id,
        int close_collector);

static inline void remove_all_collectors(
        soma_provider_t provider);

/* Functions to manipulate the list of backend types */
static inline soma_backend_impl* find_backend_impl(
        soma_provider_t provider,
        const char* name);

static inline soma_return_t add_backend_impl(
        soma_provider_t provider,
        soma_backend_impl* backend);

/* Function to check the validity of the token sent by an admin
 * (returns 0 is the token is incorrect) */
static inline int check_token(
        soma_provider_t provider,
        const char* token);

/* Admin RPCs */
static DECLARE_MARGO_RPC_HANDLER(soma_create_collector_ult)
static void soma_create_collector_ult(hg_handle_t h);
static DECLARE_MARGO_RPC_HANDLER(soma_open_collector_ult)
static void soma_open_collector_ult(hg_handle_t h);
static DECLARE_MARGO_RPC_HANDLER(soma_close_collector_ult)
static void soma_close_collector_ult(hg_handle_t h);
static DECLARE_MARGO_RPC_HANDLER(soma_destroy_collector_ult)
static void soma_destroy_collector_ult(hg_handle_t h);
static DECLARE_MARGO_RPC_HANDLER(soma_list_collectors_ult)
static void soma_list_collectors_ult(hg_handle_t h);

/* Client RPCs */
static DECLARE_MARGO_RPC_HANDLER(soma_hello_ult)
static void soma_hello_ult(hg_handle_t h);
static DECLARE_MARGO_RPC_HANDLER(soma_sum_ult)
static void soma_sum_ult(hg_handle_t h);

/* add other RPC declarations here */

int soma_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        const struct soma_provider_args* args,
        soma_provider_t* provider)
{
    struct soma_provider_args a = SOMA_PROVIDER_ARGS_INIT;
    if(args) a = *args;
    soma_provider_t p;
    hg_id_t id;
    hg_bool_t flag;

    margo_info(mid, "Registering SOMA provider with provider id %u", provider_id);

    flag = margo_is_listening(mid);
    if(flag == HG_FALSE) {
        margo_error(mid, "Margo instance is not a server");
        return SOMA_ERR_INVALID_ARGS;
    }

    margo_provider_registered_name(mid, "soma_sum", provider_id, &id, &flag);
    if(flag == HG_TRUE) {
        margo_error(mid, "Provider with the same provider id (%u) already register", provider_id);
        return SOMA_ERR_INVALID_PROVIDER;
    }

    p = (soma_provider_t)calloc(1, sizeof(*p));
    if(p == NULL) {
        margo_error(mid, "Could not allocate memory for provider");
        return SOMA_ERR_ALLOCATION;
    }

    p->mid = mid;
    p->provider_id = provider_id;
    p->pool = a.pool;
    p->abtio = a.abtio;
    p->token = (a.token && strlen(a.token)) ? strdup(a.token) : NULL;

    /* Admin RPCs */
    id = MARGO_REGISTER_PROVIDER(mid, "soma_create_collector",
            create_collector_in_t, create_collector_out_t,
            soma_create_collector_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->create_collector_id = id;

    id = MARGO_REGISTER_PROVIDER(mid, "soma_open_collector",
            open_collector_in_t, open_collector_out_t,
            soma_open_collector_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->open_collector_id = id;

    id = MARGO_REGISTER_PROVIDER(mid, "soma_close_collector",
            close_collector_in_t, close_collector_out_t,
            soma_close_collector_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->close_collector_id = id;

    id = MARGO_REGISTER_PROVIDER(mid, "soma_destroy_collector",
            destroy_collector_in_t, destroy_collector_out_t,
            soma_destroy_collector_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->destroy_collector_id = id;

    id = MARGO_REGISTER_PROVIDER(mid, "soma_list_collectors",
            list_collectors_in_t, list_collectors_out_t,
            soma_list_collectors_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->list_collectors_id = id;

    /* Client RPCs */

    id = MARGO_REGISTER_PROVIDER(mid, "soma_hello",
            hello_in_t, void,
            soma_hello_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->hello_id = id;
    margo_registered_disable_response(mid, id, HG_TRUE);

    id = MARGO_REGISTER_PROVIDER(mid, "soma_sum",
            sum_in_t, sum_out_t,
            soma_sum_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->sum_id = id;

    /* add other RPC registration here */
    /* ... */

    /* add backends available at compiler time (e.g. default/dummy backends) */
    soma_provider_register_dummy_backend(p); // function from "dummy/dummy-backend.h"

    margo_provider_push_finalize_callback(mid, p, &soma_finalize_provider, p);

    if(provider)
        *provider = p;
    margo_info(mid, "SOMA provider registration done");
    return SOMA_SUCCESS;
}

static void soma_finalize_provider(void* p)
{
    soma_provider_t provider = (soma_provider_t)p;
    margo_info(provider->mid, "Finalizing SOMA provider");
    margo_deregister(provider->mid, provider->create_collector_id);
    margo_deregister(provider->mid, provider->open_collector_id);
    margo_deregister(provider->mid, provider->close_collector_id);
    margo_deregister(provider->mid, provider->destroy_collector_id);
    margo_deregister(provider->mid, provider->list_collectors_id);
    margo_deregister(provider->mid, provider->hello_id);
    margo_deregister(provider->mid, provider->sum_id);
    /* deregister other RPC ids ... */
    remove_all_collectors(provider);
    free(provider->backend_types);
    free(provider->token);
    margo_instance_id mid = provider->mid;
    free(provider);
    margo_info(mid, "SOMA provider successfuly finalized");
}

int soma_provider_destroy(
        soma_provider_t provider)
{
    margo_instance_id mid = provider->mid;
    margo_info(mid, "Destroying SOMA provider");
    /* pop the finalize callback */
    margo_provider_pop_finalize_callback(provider->mid, provider);
    /* call the callback */
    soma_finalize_provider(provider);
    margo_info(mid, "SOMA provider successfuly destroyed");
    return SOMA_SUCCESS;
}

soma_return_t soma_provider_register_backend(
        soma_provider_t provider,
        soma_backend_impl* backend_impl)
{
    margo_info(provider->mid, "Adding backend implementation \"%s\" to SOMA provider",
             backend_impl->name);
    return add_backend_impl(provider, backend_impl);
}

static void soma_create_collector_ult(hg_handle_t h)
{
    hg_return_t hret;
    soma_return_t ret;
    create_collector_in_t  in;
    create_collector_out_t out;

    /* find the margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find the provider */
    const struct hg_info* info = margo_get_info(h);
    soma_provider_t provider = (soma_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_info(provider->mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = SOMA_ERR_FROM_MERCURY;
        goto finish;
    }

    /* check the token sent by the admin */
    if(!check_token(provider, in.token)) {
        margo_error(provider->mid, "Invalid token");
        out.ret = SOMA_ERR_INVALID_TOKEN;
        goto finish;
    }

    /* find the backend implementation for the requested type */
    soma_backend_impl* backend = find_backend_impl(provider, in.type);
    if(!backend) {
        margo_error(provider->mid, "Could not find backend of type \"%s\"", in.type);
        out.ret = SOMA_ERR_INVALID_BACKEND;
        goto finish;
    }

    /* create a uuid for the new collector */
    soma_collector_id_t id;
    uuid_generate(id.uuid);

    /* create the new collector's context */
    void* context = NULL;
    ret = backend->create_collector(provider, in.config, &context);
    if(ret != SOMA_SUCCESS) {
        out.ret = ret;
        margo_error(provider->mid, "Could not create collector, backend returned %d", ret);
        goto finish;
    }

    /* allocate a collector, set it up, and add it to the provider */
    soma_collector* collector = (soma_collector*)calloc(1, sizeof(*collector));
    collector->fn  = backend;
    collector->ctx = context;
    collector->id  = id;
    add_collector(provider, collector);

    /* set the response */
    out.ret = SOMA_SUCCESS;
    out.id = id;

    char id_str[37];
    soma_collector_id_to_string(id, id_str);
    margo_debug(provider->mid, "Created collector %s of type \"%s\"", id_str, in.type);

finish:
    hret = margo_respond(h, &out);
    hret = margo_free_input(h, &in);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(soma_create_collector_ult)

static void soma_open_collector_ult(hg_handle_t h)
{
    hg_return_t hret;
    soma_return_t ret;
    open_collector_in_t  in;
    open_collector_out_t out;

    /* find the margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find the provider */
    const struct hg_info* info = margo_get_info(h);
    soma_provider_t provider = (soma_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_error(mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = SOMA_ERR_FROM_MERCURY;
        goto finish;
    }

    /* check the token sent by the admin */
    if(!check_token(provider, in.token)) {
        margo_error(mid, "Invalid token");
        out.ret = SOMA_ERR_INVALID_TOKEN;
        goto finish;
    }

    /* find the backend implementation for the requested type */
    soma_backend_impl* backend = find_backend_impl(provider, in.type);
    if(!backend) {
        margo_error(mid, "Could not find backend of type \"%s\"", in.type);
        out.ret = SOMA_ERR_INVALID_BACKEND;
        goto finish;
    }

    /* create a uuid for the new collector */
    soma_collector_id_t id;
    uuid_generate(id.uuid);

    /* create the new collector's context */
    void* context = NULL;
    ret = backend->open_collector(provider, in.config, &context);
    if(ret != SOMA_SUCCESS) {
        margo_error(mid, "Backend failed to open collector");
        out.ret = ret;
        goto finish;
    }

    /* allocate a collector, set it up, and add it to the provider */
    soma_collector* collector = (soma_collector*)calloc(1, sizeof(*collector));
    collector->fn  = backend;
    collector->ctx = context;
    collector->id  = id;
    add_collector(provider, collector);

    /* set the response */
    out.ret = SOMA_SUCCESS;
    out.id = id;

    char id_str[37];
    soma_collector_id_to_string(id, id_str);
    margo_debug(mid, "Created collector %s of type \"%s\"", id_str, in.type);

finish:
    hret = margo_respond(h, &out);
    hret = margo_free_input(h, &in);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(soma_open_collector_ult)

static void soma_close_collector_ult(hg_handle_t h)
{
    hg_return_t hret;
    soma_return_t ret;
    close_collector_in_t  in;
    close_collector_out_t out;

    /* find the margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find the provider */
    const struct hg_info* info = margo_get_info(h);
    soma_provider_t provider = (soma_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_error(mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = SOMA_ERR_FROM_MERCURY;
        goto finish;
    }

    /* check the token sent by the admin */
    if(!check_token(provider, in.token)) {
        margo_error(mid, "Invalid token");
        out.ret = SOMA_ERR_INVALID_TOKEN;
        goto finish;
    }

    /* remove the collector from the provider 
     * (its close function will be called) */
    ret = remove_collector(provider, &in.id, 1);
    out.ret = ret;

    char id_str[37];
    soma_collector_id_to_string(in.id, id_str);
    margo_debug(mid, "Removed collector with id %s", id_str);

finish:
    hret = margo_respond(h, &out);
    hret = margo_free_input(h, &in);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(soma_close_collector_ult)

static void soma_destroy_collector_ult(hg_handle_t h)
{
    hg_return_t hret;
    destroy_collector_in_t  in;
    destroy_collector_out_t out;

    /* find the margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find the provider */
    const struct hg_info* info = margo_get_info(h);
    soma_provider_t provider = (soma_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_error(mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = SOMA_ERR_FROM_MERCURY;
        goto finish;
    }

    /* check the token sent by the admin */
    if(!check_token(provider, in.token)) {
        margo_error(mid, "Invalid token");
        out.ret = SOMA_ERR_INVALID_TOKEN;
        goto finish;
    }

    /* find the collector */
    soma_collector* collector = find_collector(provider, &in.id);
    if(!collector) {
        margo_error(mid, "Could not find collector");
        out.ret = SOMA_ERR_INVALID_COLLECTOR;
        goto finish;
    }

    /* destroy the collector's context */
    collector->fn->destroy_collector(collector->ctx);

    /* remove the collector from the provider 
     * (its close function will NOT be called) */
    out.ret = remove_collector(provider, &in.id, 0);

    if(out.ret == SOMA_SUCCESS) {
        char id_str[37];
        soma_collector_id_to_string(in.id, id_str);
        margo_debug(mid, "Destroyed collector with id %s", id_str);
    } else {
        margo_error(mid, "Could not destroy collector, collector may be left in an invalid state");
    }


finish:
    hret = margo_respond(h, &out);
    hret = margo_free_input(h, &in);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(soma_destroy_collector_ult)

static void soma_list_collectors_ult(hg_handle_t h)
{
    hg_return_t hret;
    list_collectors_in_t  in;
    list_collectors_out_t out;
    out.ids = NULL;

    /* find margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find provider */
    const struct hg_info* info = margo_get_info(h);
    soma_provider_t provider = (soma_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_error(mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = SOMA_ERR_FROM_MERCURY;
        goto finish;
    }

    /* check the token sent by the admin */
    if(!check_token(provider, in.token)) {
        margo_error(mid, "Invalid token");
        out.ret = SOMA_ERR_INVALID_TOKEN;
        goto finish;
    }

    /* allocate array of collector ids */
    out.ret   = SOMA_SUCCESS;
    out.count = provider->num_collectors < in.max_ids ? provider->num_collectors : in.max_ids;
    out.ids   = (soma_collector_id_t*)calloc(provider->num_collectors, sizeof(*out.ids));

    /* iterate over the hash of collectors to fill the array of collector ids */
    unsigned i = 0;
    soma_collector *r, *tmp;
    HASH_ITER(hh, provider->collectors, r, tmp) {
        out.ids[i++] = r->id;
    }

    margo_debug(mid, "Listed collectors");

finish:
    hret = margo_respond(h, &out);
    hret = margo_free_input(h, &in);
    free(out.ids);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(soma_list_collectors_ult)

static void soma_hello_ult(hg_handle_t h)
{
    hg_return_t hret;
    hello_in_t in;

    /* find margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find provider */
    const struct hg_info* info = margo_get_info(h);
    soma_provider_t provider = (soma_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_error(mid, "Could not deserialize output (mercury error %d)", hret);
        goto finish;
    }

    /* find the collector */
    soma_collector* collector = find_collector(provider, &in.collector_id);
    if(!collector) {
        margo_error(mid, "Could not find requested collector");
        goto finish;
    }

    /* call hello on the collector's context */
    collector->fn->hello(collector->ctx);

    margo_debug(mid, "Called hello RPC");

finish:
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(soma_hello_ult)

static void soma_sum_ult(hg_handle_t h)
{
    hg_return_t hret;
    sum_in_t     in;
    sum_out_t   out;

    /* find the margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find the provider */
    const struct hg_info* info = margo_get_info(h);
    soma_provider_t provider = (soma_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_error(mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = SOMA_ERR_FROM_MERCURY;
        goto finish;
    }

    /* find the collector */
    soma_collector* collector = find_collector(provider, &in.collector_id);
    if(!collector) {
        margo_error(mid, "Could not find requested collector");
        out.ret = SOMA_ERR_INVALID_COLLECTOR;
        goto finish;
    }

    /* call hello on the collector's context */
    out.result = collector->fn->sum(collector->ctx, in.x, in.y);
    out.ret = SOMA_SUCCESS;

    margo_debug(mid, "Called sum RPC");

finish:
    hret = margo_respond(h, &out);
    hret = margo_free_input(h, &in);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(soma_sum_ult)

static inline soma_collector* find_collector(
        soma_provider_t provider,
        const soma_collector_id_t* id)
{
    soma_collector* collector = NULL;
    HASH_FIND(hh, provider->collectors, id, sizeof(soma_collector_id_t), collector);
    return collector;
}

static inline soma_return_t add_collector(
        soma_provider_t provider,
        soma_collector* collector)
{
    soma_collector* existing = find_collector(provider, &(collector->id));
    if(existing) {
        return SOMA_ERR_INVALID_COLLECTOR;
    }
    HASH_ADD(hh, provider->collectors, id, sizeof(soma_collector_id_t), collector);
    provider->num_collectors += 1;
    return SOMA_SUCCESS;
}

static inline soma_return_t remove_collector(
        soma_provider_t provider,
        const soma_collector_id_t* id,
        int close_collector)
{
    soma_collector* collector = find_collector(provider, id);
    if(!collector) {
        return SOMA_ERR_INVALID_COLLECTOR;
    }
    soma_return_t ret = SOMA_SUCCESS;
    if(close_collector) {
        ret = collector->fn->close_collector(collector->ctx);
    }
    HASH_DEL(provider->collectors, collector);
    free(collector);
    provider->num_collectors -= 1;
    return ret;
}

static inline void remove_all_collectors(
        soma_provider_t provider)
{
    soma_collector *r, *tmp;
    HASH_ITER(hh, provider->collectors, r, tmp) {
        HASH_DEL(provider->collectors, r);
        r->fn->close_collector(r->ctx);
        free(r);
    }
    provider->num_collectors = 0;
}

static inline soma_backend_impl* find_backend_impl(
        soma_provider_t provider,
        const char* name)
{
    size_t i;
    for(i = 0; i < provider->num_backend_types; i++) {
        soma_backend_impl* impl = provider->backend_types[i];
        if(strcmp(name, impl->name) == 0)
            return impl;
    }
    return NULL;
}

static inline soma_return_t add_backend_impl(
        soma_provider_t provider,
        soma_backend_impl* backend)
{
    provider->num_backend_types += 1;
    provider->backend_types = realloc(provider->backend_types,
                                      provider->num_backend_types);
    provider->backend_types[provider->num_backend_types-1] = backend;
    return SOMA_SUCCESS;
}

static inline int check_token(
        soma_provider_t provider,
        const char* token)
{
    if(!provider->token) return 1;
    return !strcmp(provider->token, token);
}
