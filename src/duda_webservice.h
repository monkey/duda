#include "MKPlugin.h"

#ifndef DUDA_DWEBSERVICE_H
#define DUDA_DWEBSERVICE_H

/* Web service information */
struct web_service {
    mk_pointer name;
    mk_pointer docroot;
    mk_pointer confdir;
    mk_pointer datadir;
    mk_pointer logdir;

    int  enabled;
    int  url_force_redirect;
    int  bind_messages;

    void *handler;

    /* Specifics data when registering the service */
    struct mk_list *map_interfaces;
    struct mk_list *map_urls;

    /* global data */
    struct mk_list *global;

    /* workers list */
    struct mk_list *workers;

    /* loggers list */
    struct mk_list *loggers;

    /* packages loaded by the web service */
    struct mk_list *packages;

    /* node entry associated with services_list */
    struct mk_list _head;

    /* node entry associated with services_loaded */
    struct mk_list _head_loaded;
};

#endif
