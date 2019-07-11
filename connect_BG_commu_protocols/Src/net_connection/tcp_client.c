/*
 * udp_client.c
 *
 *  Created on: Jun 12, 2019
 *      Author: tony
 */

#ifndef UDP_CLIENT_C_
#define UDP_CLIENT_C_

#include "tcp_client.h"


int setnonblocking(int fd )
{
    int old_option = fcntl(fd, F_GETFL);

    int new_option = old_option | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_option );

    return old_option;
}

int tcp_unblock_connect_server(char *server_ip, unsigned short port, int timeout_s )
{
    int ret = 0;
    struct sockaddr_in address;
    int sockfd = 0;
    struct ifreq interface;

    /****** socket() ******/
    if((sockfd = socket( PF_INET, SOCK_STREAM, 0 )) < 0)
    {
		perror("socket() error \n");
		return -1;
    }

    int fdopt = setnonblocking(sockfd);

    /****** connect() ******/
    bzero( &address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton( AF_INET, server_ip, &address.sin_addr);
    address.sin_port = htons(port);

    ret = connect(sockfd, ( struct sockaddr* )&address, sizeof( address ));
    if( ret == 0 )  //connect with server immediately
    {
        fcntl( sockfd, F_SETFL, fdopt );   //set old optional back
        return sockfd;
    }
    //unblock mode --> connect return immediately! ret = -1 & errno=EINPROGRESS
    else if ( errno != EINPROGRESS )  //unblock connect failed
    {
        printf("ret = %d\n", ret);
        perror( "unblock connect failed:" );
        return -1;
    }
    else if (errno == EINPROGRESS)
    {
        printf( "unblock mode socket is connecting...\n" );
    }

    //use select to check write event, if the socket is writable, then connect is complete successfully!
    fd_set writefds;
    struct timeval timeout;

    FD_ZERO( &writefds );
    FD_SET( sockfd, &writefds );

    timeout.tv_sec = timeout_s; //timeout is seconds
    timeout.tv_usec = 0;

    ret = select( sockfd + 1, NULL, &writefds, NULL, &timeout );
    if(ret <= 0)
    {
        perror( "connection time out: ");
        goto unblock_connect_error;
    }

    if (!FD_ISSET( sockfd, &writefds ))
    {
        perror( "no events on sockfd found: ");
        goto unblock_connect_error;
    }

    int error = 0;
    socklen_t length = sizeof( error );
    if( getsockopt( sockfd, SOL_SOCKET, SO_ERROR, &error, &length ) < 0 )
    {
        perror( "get socket option failed: " );
        goto unblock_connect_error;
    }

    if( error != 0 )
    {
        perror( "connection failed after select with the error: ");
        goto unblock_connect_error;
    }

    //connection successful!
    printf( "connection ready after select with the socket: %d \n", sockfd );
    fcntl( sockfd, F_SETFL, fdopt ); //set old optional back
    //return sockfd;

	/*********set socket opt*********/
	strcpy(interface.ifr_ifrn.ifrn_name, "eth0");

	/* bind net interface eth0 */
    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&interface, sizeof(interface)) < 0)
    {
    	perror("bind interface error");
    	goto unblock_connect_error;
    }

    /* close return immediately if called, but left data will be send by system */
    struct linger m_sLinger = {0, 0};

    if(setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (const char*)&m_sLinger, sizeof(m_sLinger)) < 0)
    {
    	perror("setsockopt SO_LINGER error");
    	goto unblock_connect_error;
    }

    printf("unblock connect server successfully!\n");
	return sockfd;

unblock_connect_error:
    close(sockfd);
    return -1;
}


int tcp_block_connect_server(char *server_ip, unsigned short port)
{
	int fd = 0;
	struct sockaddr_in server;
	struct ifreq interface;

	/*********socket*********/
	if((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket error \n");
		return -1;
	}

	printf("socket() successfully!\n");

	/*********connect*********/
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	inet_pton(AF_INET, server_ip, &server.sin_addr);
	server.sin_port = htons(port);

    if(connect(fd, (struct sockaddr *)&server,sizeof(server))== -1)
    {
        perror("connect() error\n");
        return -1;
    }

    printf("connect() successfully!\n");

	/*********set socket opt*********/
	strcpy(interface.ifr_ifrn.ifrn_name, "eth0");
	/* bind net interface eth0 */
    if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&interface, sizeof(interface)) < 0)
    {
    	perror("bind interface error");
    	goto connect_error;
    }

    /* close return immediately if called, but left data will be send by system */
    struct linger m_sLinger = {0, 0};

    if(setsockopt(fd, SOL_SOCKET, SO_LINGER, (const char*)&m_sLinger, sizeof(m_sLinger)) < 0)
    {
    	perror("setsockopt SO_LINGER error");
    	goto connect_error;
    }

    printf("connect server successfully!\n");
	return fd;

connect_error:
    close(fd);
    return -1;
}



/*
 * read specified length data from serial port
 */
int read_spec_len_data(int fd, unsigned char* recv_buf, int spec_len)
{
	int left_bytes = 0;
	int read_bytes = 0;
	int retry_cnt = 0;
	int i = 0;
	unsigned char *buf_begin = recv_buf;
	left_bytes = spec_len;

	while(left_bytes > 0)
	{
		read_bytes = read(fd, recv_buf, left_bytes);

		if(read_bytes > 0)
		{
			retry_cnt = 0;
		}
		else if(read_bytes < 0)
		{
			if(left_bytes == spec_len)
			{
				return -1;
			}
			else
			{
				break;
			}
		}
		else
		{
			if(left_bytes <= 0)
			{
				break;
			}

			if(retry_cnt++ > 3)
			{
				break;
			}
			else
			{
				usleep(30000);
				continue;
			}
		}

		left_bytes -= read_bytes;
		recv_buf += read_bytes;
	}

	return (spec_len - left_bytes);
}


int send_spec_len_data(int fd, const unsigned char *send_buf, unsigned short buf_len)
{
	int send_ret = 0;
	unsigned left_len = buf_len;

	if(!send_buf || buf_len<= 0)
	{
		return -1;
	}

	while(left_len > 0)
	{
		send_ret = send(fd, send_buf, left_len, MSG_DONTWAIT);

		if(send_ret > 0)
		{
			left_len -= send_ret;
		}
		else if(send_ret == 0)
		{
			break;
		}
		else
		{
			return -1;
		}
	}

	return (buf_len -left_len);
}



#endif /* UDP_CLIENT_C_ */
