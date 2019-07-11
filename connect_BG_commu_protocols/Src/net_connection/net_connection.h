/*
 * net_connection.h
 *
 *  Created on: Jun 12, 2019
 *      Author: tony
 */

#ifndef NET_CONNECTION_H_
#define NET_CONNECTION_H_

#define  RECV_BUF_SIZE  1024*10
#define  SEND_BUF_SIZE  1024*64

enum Connection_State {
	DISCONNECTED = 0,
	CONNECTED,
} ;

typedef struct {
	int fd;
	enum Connection_State state;
	unsigned char retry_cnt;
	unsigned char *recv_buf;
	unsigned char *send_buf;
	unsigned short recv_buf_msg_len;
	unsigned short send_buf_msg_len;
} Net_Connection;


#define  NET_CONNECTION_INITIALIZER  {\
	.fd = 0, \
	.retry_cnt = 0, \
	.state = DISCONNECTED, \
	.recv_buf = NULL, \
	.send_buf = NULL, \
}


static inline int init_net_connection(Net_Connection *net_connection)
{
	net_connection->fd = 0;
	net_connection->retry_cnt = 0;
	net_connection->state = DISCONNECTED;

	if(!net_connection->recv_buf)
	{
		net_connection->recv_buf = malloc(RECV_BUF_SIZE*sizeof(unsigned char));
		if(!net_connection->recv_buf)
		{
			return -1;
		}
	}

	if(!net_connection->send_buf)
	{
		net_connection->send_buf = malloc(SEND_BUF_SIZE*sizeof(unsigned char));
		if(!net_connection->send_buf)
		{
			return -1;
		}
	}

	return 0;
}

static inline Net_Connection* apply_net_connection(unsigned int connection_cnt)
{
	if(connection_cnt <= 0)
	{
		return (Net_Connection*)(-1);
	}

	Net_Connection *new_net_connection = (Net_Connection*)malloc(sizeof(Net_Connection)*connection_cnt);

	if(!new_net_connection)  //malloc mem error
	{
		return (Net_Connection*)(-1);
	}

	if(init_net_connection(new_net_connection))  //initialize net connection error
	{
		free(new_net_connection);
		return (Net_Connection*)(-1);
	}

	return new_net_connection;
}

#endif /* NET_CONNECTION_H_ */
