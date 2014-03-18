Duda MariaDB package
====================

This package aims to make the popular relational database MariaDB available to
the Duda framework.

## Introduction ##
This package is built on top of the MariaDB non-blocking APIs in client library
that was introduced since the MariaDB 5.5 release. All the operations to the
database work in a non-blocking, asynchronous manner. Thus, it doesn't use a
multiple threads model to simulate non-blocking behavior, which makes it more
suitable for the event driven model of Monkey HTTP Daemon.

## Installation ##
This package has not been included in the official Duda release, so it must be
installed individually.

There are two ways to install it:

### Manually ###
clone the Monkey HTTP Daemon repository:

    git clone git://git.monkey-project.com/monkey

change to Monkey's directory:

    cd monkey/

clone the Duda Web Framework repository into the plugins subdirectory of Monkey:

    git clone git://git.monkey-project.com/duda plugins/duda/

clone this package into the packages subdirectory of Duda:

    git clone https://github.com/swpd/duda_mariadb plugins/duda/packages/mariadb

add mariadb package to documentation index:

    echo mariadb >> plugins/duda/docs/index.doc

edit `plugins/duda/package/Makefile.in` with your favorite text editor, add mariadb
package to variable DIRS, make sure it looks like:

    DIRS    = base64 json sha1 sha256 kv sqlite websocket ssls mariadb

#### Configure & Build ####
configure monkey with duda plugin enabled:

    ./configure --enable-plugins=duda

if you prefer verbose message output of monkey, configure it with `--trace` option
(this option should not be used in a production environment):

    ./configure --enable-plugins=duda --trace

build Monkey(go get yourself a cup of coffee, it might take a while):

    make

Notice: The MariaDB client library is compiled from source because it may be
unavailable on some distribution. And it requires `cmake` and `libaio` to be
installed on your machine:

    Debian/Ubuntu              : apt-get install libaio-dev cmake
    RedHat/Fedora/Oracle Linux : yum install libaio-devel cmake
    SUSE                       : zypper install libaio-devel cmake

### All in One ###
clone the Monkey HTTP Daemon repository from my Github:

    git clone https://github.com/swpd/monkey

change to Monkey's directory:

    cd monkey

check out `mariadb` branch:

    git checkout mariadb

That's it, configure Monkey and build it (refer to [this](#configure--build))

## Usage ##
To start with this package, the header file shall be included in your duda web
service:

    #include "packages/mariadb/mariadb.h"

Also, don't forget to load the package in the `duda_main()` function of your web
service:

    int duda_main()
    {
        duda_load_package(mariadb, "mariadb");
        ...
    }

### Establish Connections ###
To establish a connection to MariaDB server requires two steps: create a new
MariaDB connection instance and use it to establish a connection to the server.

    void some_request_callback(duda_request_t *dr)
    {
        response->http_status(dr, 200);
        response->http_header_n(dr, "Content-Type: text/plain", 24);

        /* we suspend the request before we get results from MariaDB server. */
        response->wait(dr);

        /* create a instance. */
        mariadb_con_t *conn = mariadb->create_conn(dr, "user", "password",
                                                   "localhost", "database", 0,
                                                   "/path/to/mariadb/unix/socket", 0);

        /* if the allocation doesn't success, we just end the request. */
        if (!conn) {
            response->cont(dr);
            response->end(dr, NULL);
        }

        /* try to establish a connection */
        mariadb->connect(conn, on_connect_callback);
        ...
        /* issue some queries or other stuffs to the connection */
    }

### Terminate Connections ###
Terminating a connection is done as follows:

    void some_request_callback(duda_request_t* dr)
    {
        ...
        mariadb->disconnect(conn, on_disconnect_callback);
        ...
    }

Notice: `mariadb->disconnect` doesn't close the connection immediately if the
connection still have some pending queries to process, it just notify the
connection to close when all enqueued queries are finished. This makes the
connection termination process more graceful.

### Connection Pooling ###
Establishing a connection for every request may work fine for low concurrency,
but the overhead will become obvious when it got high, considerable time and
resource will be spent on connecting and closing connections.

This is when connection pooling comes into being, we created a pool of connections
and when a request requires one, we pick one connections from the pool, after using
the connection is returned back to the pool. This reduce the overhead by reusing
the connected connections.

To create a pool, you shall define a global variable for the pool in your web
service, and initialize it in `duda_main()`:

    duda_global_t some_pool;

    int duda_main()
    {
        ...
        /* initialize a connection pool */
        duda_global_init(&some_pool, NULL, NULL);
        mariadb->create_pool(&some_pool, 0, 0, "user", "password", "localhost",
                             "database", 0, "/path/to/mariadb/unix/socket", 0);
        ...
    }

When a duda request claims for a connection, we can borrow one from the pool:

    void some_request_callback(duda_request_t* dr)
    {
        ...
        mariadb_conn_t *conn = mariadb->pool_get_conn(&demo_pool, dr, on_connect_callback);
        ...
    }

And when the connection is no longer needed, we return it back to the pool by using
the same call when we terminate a connection, only this time it doesn't close the
connection, it just mark it as usable again:

    void some_request_callback(duda_request_t* dr)
    {
        ...
        mariadb->disconnect(conn, on_disconnect_callback);
        ...
    }

### Secure Connections ###
Sometimes we need to establish secure connections to MariaDB server using SSL for
data proctection. It does nothing unless SSL support is enabled in the client
library.

To establish a single secure connection, just set up some proper file paths before
calling `mariadb->connect()`:

    void some_request_callback(duda_request_t* dr)
    {
        ...
        mariadb->set_ssl(conn, "/path/to/key/file", "/path/to/cert/file",
                         "/path/to/ca/file/", "/path/to/capth/directory",
                         "permissible_ciphers");
        mariadb->connect(...);
        ...
    }

To create a pool of secure connections, just set up some proper file paths right
after `mariadb->create_pool()`:
    
    int duda_main()
    {
        ...
        mariadb->create_pool(...);
        mariadb->pool_set_ssl(conn, "/path/to/key/file", "/path/to/cert/file",
                              "/path/to/ca/file/", "/path/to/capth/directory",
                              "permissible_ciphers");
    }

### Issue Queries ###
We can issue queries once we get a MariaDB connection from the package, the queries
will be enqueued into that connection and be processed one by one:

    mariadb->query(conn, "SHOW databases", on_result_available_callback,
                   on_row_callback, on_finish_processing_callback, NULL);

### Escape Query String ###
We may need to escape a query string to make sure that all the special characters
in that string are encoded. You shall allocate a buffer for the escaped string,
the buffer size must be at least `2 * length(origin_string) + 1`, because in the
worse case every character in that string gotta be encoded, and we need one byte
for the null byte.

    char *origin_string = "SELECT something FROM need_escape";
    char escaped_string[1024];
    mariadb->escape(conn, escaped_string, origin_string, strlen(origin_string));
    mariadb->query(conn, escaped_string, ...);

### Abort Query ###
A query can be aborted while it is being processed, if abort takes actions before
the query has been passed to the server, it is simply dropped, otherwise the
result set of this query will be freed and the query comes to the end of its life:
    
    mariadb->abort(query);

### Mutiple Statements Query ###
A query can contain severl SQL statements, you can enable this feature by passing
the `CLIENT_MULTI_STATEMENTS` client flags to `mariadb->create_conn()` or
`mariadb->create_pool()`.

Notice: This feature may lead to potential risk of SQL injection, so use it wisely
and carefully.

### API Documentation ###
For full API reference of this package, please consult `plugins/duda/docs/html/packages/mariadb.html`.

### FAQ ###
1. Why do I get error like this: Can't connect to local MySQL server through
socket '/tmp/mysql.sock'?

    It may be caused by the following reasons:

    You don't have MariaDB server set up properly;

    You are using the default unix socket path compiled with the client library,
    which may differ from your MariaDB server;

    The path you provide is wrong.
