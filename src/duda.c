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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>

#include <duda.h>

#ifdef DUDA_HAVE_MTRACE
#include <mcheck.h>
#endif

static void duda_help(int rc)
{
    printf("Usage: duda -w serv1.duda [OPTIONS] -w serv2.duda [OPTIONS] \n\n");
    printf("%sWeb Service Options%s\n", ANSI_BOLD, ANSI_RESET);
    printf("  -r, --root\t\troot path for logs/, html/ and data/ dirs\n");
    printf("  -l, --logdir\t\tlogs directory\n");
    printf("  -d, --datadir\t\tdata directory\n");
    printf("  -t, --htmldir\t\thtml directory\n");
    printf("  -w, --webservice\tweb service file path (.duda)\n");
    printf("\n");

    printf("%sOther Options%s\n", ANSI_BOLD, ANSI_RESET);
    printf("  -h, --help\t\tprint this help\n");
    printf("  -v, --version\t\tshow version number\n\n");
    exit(rc);
}

static void duda_version()
{
    printf("Duda I/O v%s\n", DUDA_VERSION_STR);
    exit(EXIT_SUCCESS);
}

static void duda_banner()
{
    printf("%sDuda I/O v%s%s\n", ANSI_BOLD, DUDA_VERSION_STR, ANSI_RESET);
    printf("%sCopyright (C) Eduardo Silva <eduardo@monkey.io>%s\n\n",
           ANSI_BOLD ANSI_YELLOW, ANSI_RESET);
}

static void duda_signal_handler(int signal)
{
    write(STDERR_FILENO, "[engine] caught signal\n", 23);

    switch (signal) {
    case SIGINT:
    case SIGQUIT:
    case SIGHUP:
    case SIGTERM:
#ifdef FLB_HAVE_MTRACE
        /* Stop tracing malloc and free */
        muntrace();
#endif
        _exit(EXIT_SUCCESS);
    default:
        break;
    }
}

static void duda_signal_init()
{
    signal(SIGINT,  &duda_signal_handler);
    signal(SIGQUIT, &duda_signal_handler);
    signal(SIGHUP,  &duda_signal_handler);
    signal(SIGTERM, &duda_signal_handler);
}

int main(int argc, char **argv)
{
    int opt;
    int ret;
    char *opt_root = NULL;
    char *opt_logdir = NULL;
    char *opt_datadir = NULL;
    char *opt_htmldir = NULL;
    char *opt_webservice = NULL;
    struct duda *duda_ctx;
    struct duda_service *srv;

    /* Setup long-options */
    static const struct option long_opts[] = {
        { "root",       required_argument, NULL, 'r' },
        { "logdir",     required_argument, NULL, 'l' },
        { "datadir",    required_argument, NULL, 'd' },
        { "htmldir",    required_argument, NULL, 't' },
        { "webservice", required_argument, NULL, 'w' },
        { "port",       required_argument, NULL, 'p' },
        { "version",    no_argument      , NULL, 'v' },
        { "help",       no_argument      , NULL, 'h' },
        { NULL, 0, NULL, 0 }
    };

#ifdef DUDA_HAVE_MTRACE
    /* Start tracing malloc and free */
    mtrace();
#endif

    duda_signal_init();

    /* Always create a Duda context from the beginning */
    duda_ctx = duda_create();
    if (!duda_ctx) {
        exit(EXIT_FAILURE);
    }

    /* Parse the command line options */
    while ((opt = getopt_long(argc, argv, "r:l:d:t:w:p:vh",
                              long_opts, NULL)) != -1) {

        switch (opt) {
        case 'r':
            opt_root = optarg;
            break;
        case 'l':
            opt_logdir = optarg;
            break;
        case 'd':
            opt_datadir = optarg;
            break;
        case 't':
            opt_htmldir = optarg;
            break;
        case 'w':
            if (opt_webservice) {
                /* A new service load request found, process the previous one */
                srv = duda_service_create(duda_ctx, opt_root, opt_logdir,
                                          opt_datadir, opt_htmldir, optarg);
                if (!srv) {
                    fprintf(stderr, "Error loading service %s\n", optarg);
                    duda_destroy(duda_ctx);
                    exit(EXIT_FAILURE);
                }

                /* Reset web service params */
                opt_root       = NULL;
                opt_logdir     = NULL;
                opt_datadir    = NULL;
                opt_htmldir    = NULL;
                opt_webservice = NULL;
                srv            = NULL;
            }
            else {
                opt_webservice = optarg;
            }
            break;
        case 'p':
            duda_ctx->tcp_port = mk_string_dup(optarg);
            break;
        case 'h':
            duda_help(EXIT_SUCCESS);
            break;
        case 'v':
            duda_version();
            exit(EXIT_SUCCESS);
        default:
            duda_help(EXIT_FAILURE);
        }
    }

    /* Load any remaining service */
    if (opt_webservice) {
        srv = duda_service_create(duda_ctx, opt_root, opt_logdir,
                                  opt_datadir, opt_htmldir, opt_webservice);
        if (!srv) {
            fprintf(stderr, "Error loading service %s\n", opt_webservice);
            duda_destroy(duda_ctx);
            exit(EXIT_FAILURE);
        }
    }

    /* Start the service */
    duda_start(duda_ctx);

    /* be a good citizen */
    return 0;
}
