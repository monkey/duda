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
    printf("Usage: duda [OPTION]\n\n");
    printf("%sAvailable Options%s\n", ANSI_BOLD, ANSI_RESET);
    printf("  -v, --version\t\tshow version number\n");
    printf("  -h, --help\t\tprint this help\n\n");
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

    /* Setup long-options */
    static const struct option long_opts[] = {
        { "version", no_argument      , NULL, 'v' },
        { "help",    no_argument      , NULL, 'h' },
        { NULL, 0, NULL, 0 }
    };

#ifdef DUDA_HAVE_MTRACE
    /* Start tracing malloc and free */
    mtrace();
#endif

    duda_signal_init();

    /* Parse the command line options */
    while ((opt = getopt_long(argc, argv, "vh",
                              long_opts, NULL)) != -1) {

        switch (opt) {
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

    /* be a good citizen */
    return 0;
}
