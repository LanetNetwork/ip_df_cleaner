/* vim: set tabstop=4:softtabstop=4:shiftwidth=4:noexpandtab */

/*
 * ip_df_cleaner - NFQUEUE userspace helper to clean IP DF bit
 * Copyright (C) Oleksandr Natalenko <oleksandr@natalenko.name>
 *
 * References:
 *
 *   http://netfilter.org/projects/libnetfilter_queue/doxygen/nfqnl__test_8c_source.html
 *   http://backreference.org/2013/07/23/gre-bridging-ipsec-and-nfqueue/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <arpa/inet.h>
#include <getopt.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <linux/netfilter.h>
#include <linux/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

/* standard IPv4 header checksum calculation, as per RFC 791 */
static u_int16_t ipv4_header_checksum(unsigned char* _hdr, size_t _hdrlen)
{
	unsigned long sum = 0;
	const u_int16_t* bbp;
	int count = 0;

	bbp = (u_int16_t*)_hdr;
	while (_hdrlen > 1)
	{
		/* the checksum field itself should be considered to be 0 (ie, excluded) when calculating the checksum */
		if (count != 10)
			sum += *bbp;
		bbp++;
		_hdrlen -= 2;
		count += 2;
	}

	/* in case hdrlen was an odd number, there will be one byte left to sum */
	if (_hdrlen > 0)
		sum += *(const unsigned char*)bbp;

	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	return (~sum);
}

/* callback function; this is called for every matched packet. */
static int cb(struct nfq_q_handle* _qh, struct nfgenmsg* _nfmsg, struct nfq_data* _nfa, void* _data)
{
	(void)_nfmsg;
	(void)_data;

	u_int32_t queue_id;
	struct nfqnl_msg_packet_hdr* ph;
	int pkt_len;
	unsigned char* buf;
	size_t hdr_len;

	/* determine the id of the packet in the queue */
	ph = nfq_get_msg_packet_hdr(_nfa);
	if (ph)
		queue_id = ntohl(ph->packet_id);
	else
		return -1;

	/* try to get at the actual packet */
	pkt_len = nfq_get_payload(_nfa, &buf);

	if (pkt_len >= 0)
	{
		hdr_len = ((buf[0] & 0x0f) * 4);

		/* clear DF bit */
		buf[6] &= 0xbf;

		/* set new packet ID */
		*((u_int16_t*)(buf + 4)) = htons((rand() % 65535) + 1);

		/* recalculate checksum */
		*((u_int16_t*)(buf + 10)) = ipv4_header_checksum(buf, hdr_len);
	}

	/* "accept" mangled packet */
	return nfq_set_verdict(_qh, queue_id, NF_ACCEPT, pkt_len, buf);
}

__attribute__((noreturn)) static void __usage(void)
{
	fprintf(stderr, "Usage: ip_df_cleaner [--queue=N]\n");
	exit(EX_USAGE);
}

int main(int _argc, char** _argv)
{
	struct nfq_handle *h;
	struct nfq_q_handle *qh;
	int fd;
	int rv;
	u_int16_t queue = 0;
	int opts = 0;
	char* invalid = NULL;
	char buf[4096] __attribute__ ((aligned));

	struct option cli_options[] =
	{
		{ "queue",	required_argument,	NULL,	'q' },
		{ 0, 0, 0, 0}
	};

	while ((opts = getopt_long(_argc, _argv, "q:", cli_options, NULL)) != -1)
	{
		switch (opts)
		{
			case 'q':
				queue = strtol(optarg, &invalid, 10);
				if (*invalid != '\0')
					__usage();
				break;
			default:
				__usage();
				break;
		}
	}

	/* open library handle */
	h = nfq_open();
	if (!h)
	{
		perror("nfq_open");
		exit(EX_OSERR);
	}

	/* unbind existing nf_queue handler for AF_INET (if any) */
	if (nfq_unbind_pf(h, AF_INET) < 0)
	{
		perror("nfq_unbind_pf");
		exit(EX_OSERR);
	}

	/* bind nfnetlink_queue as nf_queue handler for AF_INET */
	if (nfq_bind_pf(h, AF_INET) < 0)
	{
		perror("nfq_bind_pf");
		exit(EX_OSERR);
	}

	/* bind this socket to specified queue */
	qh = nfq_create_queue(h, queue, &cb, NULL);
	if (!qh)
	{
		perror("nfq_create_queue");
		exit(EX_OSERR);
	}

	/* set copy_packet mode */
	if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0)
	{
		perror("nfq_set_mode");
		exit(EX_OSERR);
	}

	fd = nfq_fd(h);

	/* initialize random number generator */
	srand(time(NULL));

	while ((rv = recv(fd, buf, sizeof(buf), 0)) && rv >= 0)
		nfq_handle_packet(h, buf, rv);

	/* unbind from specified queue */
	nfq_destroy_queue(qh);

	/* close library handle */
	nfq_close(h);

	exit(EX_OK);
}

