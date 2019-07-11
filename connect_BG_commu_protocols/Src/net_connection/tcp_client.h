/*
 * udp_client.h
 *
 *  Created on: Jun 12, 2019
 *      Author: tony
 */

#ifndef UDP_CLIENT_H_
#define UDP_CLIENT_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>


extern int tcp_unblock_connect_server(char *server_ip, unsigned short port, int timeout_s);
extern int tcp_block_connect_server(char *server_ip, unsigned short port);

int send_spec_len_data(int fd, const unsigned char *send_buf, unsigned short buf_len);

#endif /* UDP_CLIENT_H_ */
