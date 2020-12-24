/*	$OpenBSD: imsg-buffer.c,v 1.12 2019/01/20 02:50:03 bcook Exp $	*/

/*
 * Copyright (c) 2003, 2004 Henning Brauer <henning@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>

#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"
#include "imsg.h"

// Not sure why this is needed?
#ifndef IOV_MAX
#define IOV_MAX 1024
#endif

static int	ibuf_realloc(struct ibuf *, size_t);
static void	ibuf_enqueue(struct msgbuf *, struct ibuf *);
static void	ibuf_dequeue(struct msgbuf *, struct ibuf *);

struct ibuf *
ibuf_open(size_t len)
{
	struct ibuf	*buf;

	if ((buf = (struct ibuf *) calloc(1, sizeof(struct ibuf))) == NULL)
		return (NULL);
	if ((buf->buf = (unsigned char *) malloc(len)) == NULL) {
		free(buf);
		return (NULL);
	}
	buf->size = buf->max = len;
	buf->fd = -1;

	return (buf);
}

struct ibuf *
ibuf_dynamic(size_t len, size_t max)
{
	struct ibuf	*buf;

	if (max < len)
		return (NULL);

	if ((buf = ibuf_open(len)) == NULL)
		return (NULL);

	if (max > 0)
		buf->max = max;

	return (buf);
}

static int
ibuf_realloc(struct ibuf *buf, size_t len)
{
	unsigned char	*b;

	/* on static buffers max is eq size and so the following fails */
	if (buf->wpos + len > buf->max) {
		errno = ERANGE;
		return (-1);
	}

	b = (unsigned char *) recallocarray(buf->buf, buf->size, buf->wpos + len, 1);
	if (b == NULL)
		return (-1);
	buf->buf = b;
	buf->size = buf->wpos + len;

	return (0);
}

int
ibuf_add(struct ibuf *buf, const void *data, size_t len)
{
	if (buf->wpos + len > buf->size)
		if (ibuf_realloc(buf, len) == -1)
			return (-1);

	memcpy(buf->buf + buf->wpos, data, len);
	buf->wpos += len;
	return (0);
}

void *
ibuf_reserve(struct ibuf *buf, size_t len)
{
	void	*b;

	if (buf->wpos + len > buf->size)
		if (ibuf_realloc(buf, len) == -1)
			return (NULL);

	b = buf->buf + buf->wpos;
	buf->wpos += len;
	return (b);
}

void *
ibuf_seek(struct ibuf *buf, size_t pos, size_t len)
{
	/* only allowed to seek in already written parts */
	if (pos + len > buf->wpos)
		return (NULL);

	return (buf->buf + pos);
}

size_t
ibuf_size(struct ibuf *buf)
{
	return (buf->wpos);
}

size_t
ibuf_left(struct ibuf *buf)
{
	return (buf->max - buf->wpos);
}

void
ibuf_close(struct msgbuf *msgbuf, struct ibuf *buf)
{
	ibuf_enqueue(msgbuf, buf);
}

int
ibuf_write(struct msgbuf *msgbuf)
{
#ifdef _WIN32
	WSABUF iov[IOV_MAX];
	int wsa_send_retval = 0;
	int wsa_send_error = 0;
	DWORD wsa_num_bytes_sent;
#else
	struct iovec	 iov[IOV_MAX];
#endif
	struct ibuf	*buf;
	unsigned int	 i = 0;
	ssize_t	n;
	int received_EINTR = 0;
	int received_ENOBUFS = 0;

	memset(&iov, 0, sizeof(iov));
	TAILQ_FOREACH(buf, &msgbuf->bufs, entry) {
		if (i >= IOV_MAX)
			break;
		void* buffer_base = buf->buf + buf->rpos;
		size_t buffer_len = buf->wpos - buf->rpos;
#ifdef _WIN32
		iov[i].buf = (char*)buffer_base;
		iov[i].len = (ULONG)buffer_len;
#else
		iov[i].iov_base = buffer_base;
		iov[i].iov_len = buffer_len;
#endif
		i++;
	}

again:
#ifdef _WIN32
	wsa_send_retval = WSASend(msgbuf->fd, iov, i, &wsa_num_bytes_sent, 0, NULL, NULL);
	n = wsa_num_bytes_sent;
	if (wsa_send_retval == SOCKET_ERROR)
	{
		wsa_send_error = WSAGetLastError();
		received_EINTR = wsa_send_error == WSAEINTR;
		received_ENOBUFS = wsa_send_error == WSAENOBUFS;
	}
#else
	n = writev(msgbuf->fd, iov, i);
	if (n == -1)
	{
		received_EINTR = errno == EINTR;
		received_ENOBUFS = errno == ENOBUFS;
	}
#endif
	if (n == -1) {
		if (received_EINTR)
			goto again;
		if (received_ENOBUFS)
			errno = EAGAIN;
		return (-1);
	}

	if (n == 0) {			/* connection closed */
		errno = 0;
		return (0);
	}

	msgbuf_drain(msgbuf, n);

	return (1);
}

void
ibuf_free(struct ibuf *buf)
{
	if (buf == NULL)
		return;
	freezero(buf->buf, buf->size);
	free(buf);
}

void
msgbuf_init(struct msgbuf *msgbuf)
{
	msgbuf->queued = 0;
	msgbuf->fd = -1;
	TAILQ_INIT(&msgbuf->bufs);
}

void
msgbuf_drain(struct msgbuf *msgbuf, size_t n)
{
	struct ibuf	*buf, *next;

	for (buf = TAILQ_FIRST(&msgbuf->bufs); buf != NULL && n > 0;
	    buf = next) {
		next = TAILQ_NEXT(buf, entry);
		if (buf->rpos + n >= buf->wpos) {
			n -= buf->wpos - buf->rpos;
			ibuf_dequeue(msgbuf, buf);
		} else {
			buf->rpos += n;
			n = 0;
		}
	}
}

void
msgbuf_clear(struct msgbuf *msgbuf)
{
	struct ibuf	*buf;

	while ((buf = TAILQ_FIRST(&msgbuf->bufs)) != NULL)
		ibuf_dequeue(msgbuf, buf);
}

int
msgbuf_write(struct msgbuf *msgbuf)
{
#ifdef _WIN32
	WSABUF iov[IOV_MAX];
	WSAMSG msg;
	WSACMSGHDR* cmsg;
	int wsa_sendmsg_retval = 0;
	int wsa_sendmsg_error = 0;
	DWORD wsa_num_bytes_sent = 0;
#else
	struct iovec	 iov[IOV_MAX];
	struct msghdr	 msg;
	struct cmsghdr* cmsg;
#endif
	struct ibuf	*buf;
	unsigned int	 i = 0;
	ssize_t		 n;
	int received_EINTR = 0;
	int received_ENOBUFS = 0;
	union {
#ifdef _WIN32
		WSACMSGHDR hdr;
#else
		struct cmsghdr	hdr;
#endif
		char		buf[CMSG_SPACE(sizeof(int))];
	} cmsgbuf;

	memset(&iov, 0, sizeof(iov));
	memset(&msg, 0, sizeof(msg));
	memset(&cmsgbuf, 0, sizeof(cmsgbuf));
	TAILQ_FOREACH(buf, &msgbuf->bufs, entry) {
		if (i >= IOV_MAX)
			break;
		void* buffer_base = buf->buf + buf->rpos;
		size_t buffer_len = buf->wpos - buf->rpos;
#ifdef _WIN32
		iov[i].buf = (char*)buffer_base;
		iov[i].len = (ULONG)buffer_len;
#else
		iov[i].iov_base = buffer_base;
		iov[i].iov_len = buffer_len;
#endif
		i++;
		if (buf->fd != -1)
			break;
	}

#ifdef _WIN32
	msg.lpBuffers = iov;
	msg.dwBufferCount = i;
#else
	msg.msg_iov = iov;
	msg.msg_iovlen = i;
#endif

	if (buf != NULL && buf->fd != -1) {
#ifdef _WIN32
		msg.Control.buf = (char*)&cmsgbuf.buf;
		msg.Control.len = sizeof(cmsgbuf.buf);
#else
		msg.msg_control = (caddr_t) &cmsgbuf.buf;
		msg.msg_controllen = sizeof(cmsgbuf.buf);
#endif
		cmsg = CMSG_FIRSTHDR(&msg);
		cmsg->cmsg_len = CMSG_LEN(sizeof(int));
		cmsg->cmsg_level = SOL_SOCKET;
#ifdef _WIN32
		*(int*) WSA_CMSG_DATA(cmsg) = buf->fd;
#else
		cmsg->cmsg_type = SCM_RIGHTS; // WIN32_TODO: This doesn't work in Windows
		*(int*) CMSG_DATA(cmsg) = buf->fd;
#endif
	}

again:
#ifdef WIN32
	wsa_sendmsg_retval = WSASendMsg(msgbuf->fd, &msg, 0, &wsa_num_bytes_sent, NULL, NULL);
	n = wsa_num_bytes_sent;
	if (wsa_sendmsg_retval == SOCKET_ERROR)
	{
		wsa_sendmsg_error = WSAGetLastError();
		received_EINTR = wsa_sendmsg_error == WSAEINTR;
		received_ENOBUFS = wsa_sendmsg_error == WSAENOBUFS;
	}
#else
	n = sendmsg(msgbuf->fd, &msg, 0);
	received_EINTR = errno == EINTR;
	received_ENOBUFS = errno == ENOBUFS;
#endif
	if (n == -1) {
		if (received_EINTR)
			goto again;
		if (received_ENOBUFS)
			errno = EAGAIN;
		return (-1);
	}

	if (n == 0) {			/* connection closed */
		errno = 0;
		return (0);
	}

	/*
	 * assumption: fd got sent if sendmsg sent anything
	 * this works because fds are passed one at a time
	 */
	if (buf != NULL && buf->fd != -1) {
		close(buf->fd);
		buf->fd = -1;
	}

	msgbuf_drain(msgbuf, n);

	return (1);
}

static void
ibuf_enqueue(struct msgbuf *msgbuf, struct ibuf *buf)
{
	TAILQ_INSERT_TAIL(&msgbuf->bufs, buf, entry);
	msgbuf->queued++;
}

static void
ibuf_dequeue(struct msgbuf *msgbuf, struct ibuf *buf)
{
	TAILQ_REMOVE(&msgbuf->bufs, buf, entry);

	if (buf->fd != -1)
		close(buf->fd);

	msgbuf->queued--;
	ibuf_free(buf);
}
