/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef _ADMIN_H
#define _ADMIN_H

#include "types.h"
#include "soma/soma-admin.h"

typedef struct soma_admin {
   margo_instance_id mid;
   hg_id_t           create_collector_id;
   hg_id_t           open_collector_id;
   hg_id_t           close_collector_id;
   hg_id_t           destroy_collector_id;
   hg_id_t           list_collectors_id;
} soma_admin;

#endif
