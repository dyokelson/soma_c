/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <stdio.h>
#include <margo.h>
#include <soma/soma-server.h>
#include <soma/soma-admin.h>
#include "munit/munit.h"

struct test_context {
    margo_instance_id mid;
    hg_addr_t         addr;
};

static const char* valid_token = "ABCDEFGH";
static const char* wrong_token = "HGFEDCBA";
static const uint16_t provider_id = 42;
static const char* backend_config = "{ \"foo\" : \"bar\" }";


static void* test_context_setup(const MunitParameter params[], void* user_data)
{
    (void) params;
    (void) user_data;
    margo_instance_id mid;
    hg_addr_t         addr;
    // create margo instance
    mid = margo_init("na+sm", MARGO_SERVER_MODE, 0, 0);
    munit_assert_not_null(mid);
    // get address of current process
    hg_return_t hret = margo_addr_self(mid, &addr);
    munit_assert_int(hret, ==, HG_SUCCESS);
    // register soma provider
    struct soma_provider_args args = SOMA_PROVIDER_ARGS_INIT;
    args.token = valid_token;
    soma_return_t ret = soma_provider_register(
            mid, provider_id, &args,
            SOMA_PROVIDER_IGNORE);
    munit_assert_int(ret, ==, SOMA_SUCCESS);
    // create test context
    struct test_context* context = (struct test_context*)calloc(1, sizeof(*context));
    munit_assert_not_null(context);
    context->mid  = mid;
    context->addr = addr;
    return context;
}

static void test_context_tear_down(void* fixture)
{
    struct test_context* context = (struct test_context*)fixture;
    // free address
    margo_addr_free(context->mid, context->addr);
    // we are not checking the return value of the above function with
    // munit because we need margo_finalize to be called no matter what.
    margo_finalize(context->mid);
}

static MunitResult test_admin(const MunitParameter params[], void* data)
{
    (void)params;
    (void)data;
    struct test_context* context = (struct test_context*)data;
    soma_admin_t admin;
    soma_return_t ret;
    // test that we can create an admin object
    ret = soma_admin_init(context->mid, &admin);
    munit_assert_int(ret, ==, SOMA_SUCCESS);
    // test that we can free the admin object
    ret = soma_admin_finalize(admin);
    munit_assert_int(ret, ==, SOMA_SUCCESS);

    return MUNIT_OK;
}

static MunitResult test_collector(const MunitParameter params[], void* data)
{
    (void)params;
    (void)data;
    struct test_context* context = (struct test_context*)data;
    soma_admin_t admin;
    soma_return_t ret;
    soma_collector_id_t id;
    // test that we can create an admin object
    ret = soma_admin_init(context->mid, &admin);
    munit_assert_int(ret, ==, SOMA_SUCCESS);
    
    // test that we can create a collector with type "dummy"
    ret = soma_create_collector(admin, context->addr,
            provider_id, valid_token, "dummy", backend_config, &id);
    munit_assert_int(ret, ==, SOMA_SUCCESS);
   
    // test that we can list the collectors
    soma_collector_id_t ids[4];
    size_t count = 4;
    ret = soma_list_collectors(admin, context->addr,
            provider_id, valid_token, ids, &count);
    munit_assert_int(ret, ==, SOMA_SUCCESS);
    munit_assert_ulong(count, ==, 1);
    munit_assert_memory_equal(sizeof(id), ids, &id);

    // test that we can destroy the collector we just created
    ret = soma_destroy_collector(admin, context->addr,
            provider_id, valid_token, id);
    munit_assert_int(ret, ==, SOMA_SUCCESS);
    // note: open and close are essentially the same as create and
    // destroy in this code so we won't be testing them.

    // test that we can free the admin object
    ret = soma_admin_finalize(admin);
    munit_assert_int(ret, ==, SOMA_SUCCESS);

    return MUNIT_OK;
}

static MunitResult test_invalid(const MunitParameter params[], void* data)
{
    (void)params;
    (void)data;
    struct test_context* context = (struct test_context*)data;
    soma_admin_t admin;
    soma_return_t ret;
    soma_collector_id_t id;
    // test that we can create an admin object
    ret = soma_admin_init(context->mid, &admin);
    munit_assert_int(ret, ==, SOMA_SUCCESS);

    // test that calling the wrong provider id leads to an error
    ret = soma_create_collector(admin, context->addr,
            provider_id + 1, valid_token, "dummy", backend_config, &id);
    munit_assert_int(ret, ==, SOMA_ERR_FROM_MERCURY);

    // test that calling with the wrong token leads to an error
    ret = soma_create_collector(admin, context->addr,
            provider_id, wrong_token, "dummy", backend_config, &id);
    munit_assert_int(ret, ==, SOMA_ERR_INVALID_TOKEN);

    // test that calling with the wrong config leads to an error
    ret = soma_create_collector(admin, context->addr,
            provider_id, valid_token, "dummy", "{ashqw{", &id);
    munit_assert_int(ret, ==, SOMA_ERR_INVALID_CONFIG);

    // test that calling with an unknown backend leads to an error
    ret = soma_create_collector(admin, context->addr,
            provider_id, valid_token, "blah", backend_config, &id);
    munit_assert_int(ret, ==, SOMA_ERR_INVALID_BACKEND);

    // this creation should be successful
    ret = soma_create_collector(admin, context->addr,
            provider_id, valid_token, "dummy", backend_config, &id);
    munit_assert_int(ret, ==, SOMA_SUCCESS);

    // test that destroying an invalid id leads to an error
    soma_collector_id_t wrong_id;
    memset((void*) &wrong_id, 0, sizeof(wrong_id));
    ret = soma_destroy_collector(admin, context->addr, provider_id, valid_token, wrong_id);
    munit_assert_int(ret, ==, SOMA_ERR_INVALID_COLLECTOR);

    // correctly destroy the created collector
    ret = soma_destroy_collector(admin, context->addr, provider_id, valid_token, id);
    munit_assert_int(ret, ==, SOMA_SUCCESS);

    // test that we can free the admin object
    ret = soma_admin_finalize(admin);
    munit_assert_int(ret, ==, SOMA_SUCCESS);

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    { (char*) "/admin",    test_admin,    test_context_setup, test_context_tear_down, MUNIT_TEST_OPTION_NONE, NULL },
    { (char*) "/collector", test_collector, test_context_setup, test_context_tear_down, MUNIT_TEST_OPTION_NONE, NULL },
    { (char*) "/invalid",  test_invalid,  test_context_setup, test_context_tear_down, MUNIT_TEST_OPTION_NONE, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = { 
    (char*) "/soma/admin", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, (void*) "soma", argc, argv);
}
