/**
 * @copyright Copyright (c) 2021 ApertureData Inc.
 */

#include "OpenSSLBio.h"

#include <cstring>
#include <sys/socket.h>

static int bio_initialized = 0;
static BIO_METHOD* comm_bio_method{nullptr};

static int comm_openssl_bio_read(BIO* bio, char* buffer, int length);
static int comm_openssl_bio_write(BIO* bio, const char* buffer, int length);

static void comm_openssl_bio_init(void)
{
    comm_bio_method = BIO_meth_new(BIO_TYPE_SOCKET, "libcomm");

    auto default_bio_method = const_cast< BIO_METHOD* >(BIO_s_socket());

    BIO_meth_set_create(comm_bio_method, BIO_meth_get_create(default_bio_method));
    BIO_meth_set_destroy(comm_bio_method, BIO_meth_get_destroy(default_bio_method));
    BIO_meth_set_ctrl(comm_bio_method, BIO_meth_get_ctrl(default_bio_method));
    BIO_meth_set_callback_ctrl(comm_bio_method, BIO_meth_get_callback_ctrl(default_bio_method));
    BIO_meth_set_read(comm_bio_method, comm_openssl_bio_read);
    BIO_meth_set_write(comm_bio_method, comm_openssl_bio_write);
    BIO_meth_set_gets(comm_bio_method, BIO_meth_get_gets(default_bio_method));
    BIO_meth_set_puts(comm_bio_method, BIO_meth_get_puts(default_bio_method));

    bio_initialized = 1;
}

static int comm_openssl_bio_read(BIO* bio, char* buffer, int length)
{
    int fd;
    BIO_get_fd(bio, &fd);

    auto count = ::recv(fd, buffer, length, MSG_NOSIGNAL);

    BIO_clear_retry_flags(bio);

    if (count <= 0 && BIO_sock_should_retry(count)) {
        BIO_set_retry_read(bio);
    }

    return count;
}

static int comm_openssl_bio_write(BIO* bio, const char* buffer, int length)
{
    int fd;
    BIO_get_fd(bio, &fd);

    auto count = ::send(fd, buffer, length, MSG_NOSIGNAL);

    BIO_clear_retry_flags(bio);

    if (count <= 0 && BIO_sock_should_retry(count)) {
        BIO_set_retry_write(bio);
    }

    return count;
}

BIO_METHOD* comm_openssl_bio()
{
    if (!bio_initialized) {
        comm_openssl_bio_init();
    }

    return comm_bio_method;
}
