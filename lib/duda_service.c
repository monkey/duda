/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Duda I/O
 *  --------
 *  Copyright (C) 2012-2016, Eduardo Silva P. <eduardo@monkey.io>.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <duda.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dlfcn.h>

static void *load_symbol(void *handle, const char *symbol)
{
    void *s;
    char *err;

    dlerror();
    s = dlsym(handle, symbol);
    if ((err = dlerror()) != NULL) {
        return NULL;
    }

    return s;
}

static int validate_dir(char *path)
{
    int ret;
    struct stat st;

    ret = stat(path, &st);
    if (ret == -1) {
        return -1;
    }

    /* Validate entry is a directory */
    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "path '%s' is not a directory\n", path);
        return -1;
    }

    /* Check read/exec permissions on directory */
    ret = access(path, R_OK | X_OK);
    if (ret == -1) {
        fprintf(stderr, "permission problems on '%s' directory\n", path);
        return -1;
    }

    return 0;
}

static char *service_path(char *root, char *path)
{
    int len_root;
    int len_path;
    size_t size;
    char *new_path;

    /* If no root/prefix exists */
    if (!root) {
        return mk_string_dup(path);
    }

    /* Root prefix is set, check if 'path' is absolute or not */
    if (*path == '/') {
        /* Absolute path never uses the prefix */
        return mk_string_dup(path);
    }

    /* Concatenare root and path */
    len_root = strlen(root);
    len_path = strlen(path);
    size = (len_root + len_path + 2);

    new_path = mk_mem_alloc(size);
    if (!new_path) {
        return NULL;
    }

    snprintf(new_path, size, "%s/%s", root, path);
    return new_path;
}

/* Lookup and load web service symbols */
static int map_internals(struct duda_service *ds, struct duda_api_objects *api)
{
    int (*cb_main) (struct duda_service *, struct duda_api_objects *);

    /* Lookup and invoke duda_main() */
    cb_main = (int (*)()) load_symbol(ds->dl_handle, "_duda_bootstrap");
    cb_main(ds, api);


}

/* Creates a web service instance */
struct duda_service *duda_service_create(struct duda *d, char *root, char *log,
                                         char *data, char *html, char *service)
{
    int ret;
    void *handle;
    struct duda_service *ds;
    struct duda_api_objects *api;

    if (!d) {
        return NULL;
    }

    /* Check access to root, log, data and html directories */
    if (root) {
        ret = validate_dir(root);
        if (ret != 0) {
            return NULL;
        }
    }
    if (log) {
        ret = validate_dir(log);
        if (ret != 0) {
            return NULL;
        }
    }
    if (data) {
        ret = validate_dir(log);
        if (ret != 0) {
            return NULL;
        }
    }
    if (html) {
        ret = validate_dir(html);
        if (ret != 0) {
            return NULL;
        }
    }

    /* Validate the web service file */
    handle = dlopen(service, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Error opening web service file '%s'\n", service);
        return NULL;
    }

    /* Create web service context */
    ds = mk_mem_alloc_z(sizeof(struct duda_service));
    if (!ds) {
        dlclose(handle);
        return NULL;
    }

    /* Root prefix path */
    if (root) {
        ds->path_root = mk_string_dup(root);
    }

    /*
     * If root prefix path is set, for all incoming paths that are not
     * absolute, we prefix the path
     */
    if (log) {
        ds->path_log  = service_path(root, log);
    }
    if (data) {
        ds->path_data = service_path(root, data);
    }
    if (html) {
        ds->path_html = service_path(root, html);
    }
    if (service) {
        ds->path_service = service_path(root, service);
    }
    ds->dl_handle = handle;
    mk_list_add(&ds->_head, &d->services);

    /* Initialize references for API objects */
    mk_list_init(&ds->router_list);

    api = duda_api_create();
    map_internals(ds, api);

    return ds;
}

int duda_service_destroy(struct duda_service *ds)
{
    /* Free paths */
    mk_mem_free(ds->path_root);
    mk_mem_free(ds->path_log);
    mk_mem_free(ds->path_data);
    mk_mem_free(ds->path_html);
    mk_mem_free(ds->path_service);

    /* Close handle */
    dlclose(ds->dl_handle);
    mk_list_del(&ds->_head);
    mk_mem_free(ds);

    return 0;
}
