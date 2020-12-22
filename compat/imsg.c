/*	$OpenBSD: imsg.c,v 1.16 2017/12/14 09:27:44 kettenis Exp $	*/

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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"
#include "imsg.h"

int	 imsg_fd_overhead = 0;

static int	 imsg_get_fd(struct imsgbuf *);

void
imsg_init(struct imsgbuf *ibuf, int fd)
{
	msgbuf_init(&ibuf->w);
	memset(&ibuf->r, 0, sizeof(ibuf->r));
	ibuf->fd = fd;
	ibuf->w.fd = fd;
	ibuf->pid = getpid();
	TAILQ_INIT(&ibuf->fds);
}

ssize_t
imsg_read(struct imsgbuf *ibuf)
{
#ifdef _WIN32
	WSAMSG msg;
	WSACMSGHDR* cmsg;
	WSABUF iov;
	GUID wsa_recvmsg_guid = WSAID_WSARECVMSG;
	LPFN_WSARECVMSG WSARecvMsg;
	DWORD wsa_num_bytes_returned = 0;
	int wsa_retval;
	int wsa_error;
#else
	struct msghdr msg;
	struct cmsghdr* cmsg;
	struct iovec iov;
#endif
	union {
#ifdef _WIN32
		WSACMSGHDR hdr;
#else
		struct cmsghdr hdr;
#endif
		char	buf[CMSG_SPACE(sizeof(int) * 1)];
	} cmsgbuf;
	ssize_t			 n = -1;
	int			 fd;
	struct imsg_fd		*ifd;
	int received_EINTR = 0;

	memset(&msg, 0, sizeof(msg));
	memset(&cmsgbuf, 0, sizeof(cmsgbuf));

	void* buffer_base = ibuf->r.buf + ibuf->r.wpos;
	size_t buffer_len = sizeof(ibuf->r.buf) - ibuf->r.wpos;
#ifdef _WIN32
	iov.buf = buffer_base;
	iov.len = (ULONG)buffer_len;
	msg.lpBuffers = &iov;
	msg.dwBufferCount = 1;
	msg.Control.buf = (char*)&cmsgbuf.buf;
	msg.Control.len = sizeof(cmsgbuf.buf);
#else
	iov.iov_base = buffer_base;
	iov.iov_len = buffer_len;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = &cmsgbuf.buf;
	msg.msg_controllen = sizeof(cmsgbuf.buf);
#endif

	if ((ifd = calloc(1, sizeof(struct imsg_fd))) == NULL)
		return (-1);

again:
	if (getdtablecount() + imsg_fd_overhead +
	    (int)((CMSG_SPACE(sizeof(int))-CMSG_SPACE(0))/sizeof(int))
	    >= getdtablesize()) {
		errno = EAGAIN;
		free(ifd);
		return (-1);
	}

#ifdef _WIN32
	// WIN32_TODO: Cache the function pointer
	wsa_retval = WSAIoctl(ibuf->fd, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&wsa_recvmsg_guid, sizeof wsa_recvmsg_guid,
		&WSARecvMsg, sizeof WSARecvMsg,
		&wsa_num_bytes_returned, NULL, NULL);
	if (wsa_retval == SOCKET_ERROR)
	{
		wsa_error = WSAGetLastError();
		WSARecvMsg = NULL;
		return 0;
	}
	wsa_retval = WSARecvMsg(ibuf->fd, &msg, &wsa_num_bytes_returned, NULL, NULL);
	n = wsa_num_bytes_returned;
	if (wsa_retval == SOCKET_ERROR)
	{
		wsa_error = WSAGetLastError();
		received_EINTR = wsa_error == WSAEINTR;
	}
#else
	n = recvmsg(ibuf->fd, &msg, 0);
	received_EINTR = errno == EINTR;
#endif

	if (n == -1) {
		if (received_EINTR)
			goto again;
		goto fail;
	}

	ibuf->r.wpos += n;

	for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL;
	    cmsg = CMSG_NXTHDR(&msg, cmsg)) {
		if (cmsg->cmsg_level == SOL_SOCKET
#ifndef _WIN32
			&& cmsg->cmsg_type == SCM_RIGHTS // WIN32_TODO: SCM_RIGHTS doesn't work on Windows
#endif
			) {
			int i;
			int j;

			/*
			 * We only accept one file descriptor.  Due to C
			 * padding rules, our control buffer might contain
			 * more than one fd, and we must close them.
			 */
#ifdef _WIN32
			char* cmsg_data_char_ptr = (char*) WSA_CMSG_DATA(cmsg);
#else
			char* cmsg_data_char_ptr = (char*) CMSG_DATA(cmsg);
#endif
			int* cmsg_data_int_ptr;
			j = (int) ((char *)cmsg + cmsg->cmsg_len - cmsg_data_char_ptr) / sizeof(int);
			for (i = 0; i < j; i++) {
#ifdef _WIN32
				cmsg_data_int_ptr = (int*) WSA_CMSG_DATA(cmsg);
#else
				cmsg_data_int_ptr = (int*) CMSG_DATA(cmsg);
#endif
				fd = cmsg_data_int_ptr[i];
				if (ifd != NULL) {
					ifd->fd = fd;
					TAILQ_INSERT_TAIL(&ibuf->fds, ifd,
					    entry);
					ifd = NULL;
				} else
					close(fd);
			}
		}
		/* we do not handle other ctl data level */
	}

fail:
	free(ifd);
	return (n);
}

ssize_t
imsg_get(struct imsgbuf *ibuf, struct imsg *imsg)
{
	size_t			 av, left, datalen;

	av = ibuf->r.wpos;

	if (IMSG_HEADER_SIZE > av)
		return (0);

	memcpy(&imsg->hdr, ibuf->r.buf, sizeof(imsg->hdr));
	if (imsg->hdr.len < IMSG_HEADER_SIZE ||
	    imsg->hdr.len > MAX_IMSGSIZE) {
		errno = ERANGE;
		return (-1);
	}
	if (imsg->hdr.len > av)
		return (0);
	datalen = imsg->hdr.len - IMSG_HEADER_SIZE;
	ibuf->r.rptr = ibuf->r.buf + IMSG_HEADER_SIZE;
	if (datalen == 0)
		imsg->data = NULL;
	else if ((imsg->data = malloc(datalen)) == NULL)
		return (-1);

	if (imsg->hdr.flags & IMSGF_HASFD)
		imsg->fd = imsg_get_fd(ibuf);
	else
		imsg->fd = -1;

	memcpy(imsg->data, ibuf->r.rptr, datalen);

	if (imsg->hdr.len < av) {
		left = av - imsg->hdr.len;
		memmove(&ibuf->r.buf, ibuf->r.buf + imsg->hdr.len, left);
		ibuf->r.wpos = left;
	} else
		ibuf->r.wpos = 0;

	return (datalen + IMSG_HEADER_SIZE);
}

int
imsg_compose(struct imsgbuf *ibuf, uint32_t type, uint32_t peerid, pid_t pid,
    int fd, const void *data, uint16_t datalen)
{
	struct ibuf	*wbuf;

	if ((wbuf = imsg_create(ibuf, type, peerid, pid, datalen)) == NULL)
		return (-1);

	if (imsg_add(wbuf, data, datalen) == -1)
		return (-1);

	wbuf->fd = fd;

	imsg_close(ibuf, wbuf);

	return (1);
}

// This function is not used by Tmux
#if 0
int
imsg_composev(struct imsgbuf *ibuf, uint32_t type, uint32_t peerid, pid_t pid,
    int fd, const struct iovec *iov, int iovcnt)
{
	struct ibuf	*wbuf;
	int		 i, datalen = 0;

	for (i = 0; i < iovcnt; i++)
		datalen += iov[i].iov_len;

	if ((wbuf = imsg_create(ibuf, type, peerid, pid, datalen)) == NULL)
		return (-1);

	for (i = 0; i < iovcnt; i++)
		if (imsg_add(wbuf, iov[i].iov_base, iov[i].iov_len) == -1)
			return (-1);

	wbuf->fd = fd;

	imsg_close(ibuf, wbuf);

	return (1);
}
#endif

/* ARGSUSED */
struct ibuf *
imsg_create(struct imsgbuf *ibuf, uint32_t type, uint32_t peerid, pid_t pid,
    uint16_t datalen)
{
	struct ibuf	*wbuf;
	struct imsg_hdr	 hdr;

	datalen += IMSG_HEADER_SIZE;
	if (datalen > MAX_IMSGSIZE) {
		errno = ERANGE;
		return (NULL);
	}

	hdr.type = type;
	hdr.flags = 0;
	hdr.peerid = peerid;
	if ((hdr.pid = pid) == 0)
		hdr.pid = ibuf->pid;
	if ((wbuf = ibuf_dynamic(datalen, MAX_IMSGSIZE)) == NULL) {
		return (NULL);
	}
	if (imsg_add(wbuf, &hdr, sizeof(hdr)) == -1)
		return (NULL);

	return (wbuf);
}

int
imsg_add(struct ibuf *msg, const void *data, uint16_t datalen)
{
	if (datalen)
		if (ibuf_add(msg, data, datalen) == -1) {
			ibuf_free(msg);
			return (-1);
		}
	return (datalen);
}

void
imsg_close(struct imsgbuf *ibuf, struct ibuf *msg)
{
	struct imsg_hdr	*hdr;

	hdr = (struct imsg_hdr *)msg->buf;

	hdr->flags &= ~IMSGF_HASFD;
	if (msg->fd != -1)
		hdr->flags |= IMSGF_HASFD;

	hdr->len = (uint16_t)msg->wpos;

	ibuf_close(&ibuf->w, msg);
}

void
imsg_free(struct imsg *imsg)
{
	freezero(imsg->data, imsg->hdr.len - IMSG_HEADER_SIZE);
}

static int
imsg_get_fd(struct imsgbuf *ibuf)
{
	int		 fd;
	struct imsg_fd	*ifd;

	if ((ifd = TAILQ_FIRST(&ibuf->fds)) == NULL)
		return (-1);

	fd = ifd->fd;
	TAILQ_REMOVE(&ibuf->fds, ifd, entry);
	free(ifd);

	return (fd);
}

int
imsg_flush(struct imsgbuf *ibuf)
{
	while (ibuf->w.queued)
		if (msgbuf_write(&ibuf->w) <= 0)
			return (-1);
	return (0);
}

void
imsg_clear(struct imsgbuf *ibuf)
{
	int	fd;

	msgbuf_clear(&ibuf->w);
	while ((fd = imsg_get_fd(ibuf)) != -1)
		close(fd);
}
