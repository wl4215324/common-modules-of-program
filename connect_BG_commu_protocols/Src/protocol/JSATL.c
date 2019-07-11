/*
 * JSATL.c
 *
 *  Created on: Jun 12, 2019
 *      Author: tony
 */

#include "JSATL.h"
#include "../net_connection/net_connection.h"
#include "../net_connection/tcp_client.h"


unsigned short send_msg_SN = 0, recv_msg_SN = 0;

pthread_mutex_t wait_recv_mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t wait_recv_cond = PTHREAD_COND_INITIALIZER;
int BG_reply_val = 0xFF;
char authen_str[32] = "Authen";

Dev_Bsc_Info *dev_bsc_info_list;
Dev_Params_Info *dev_params_info_list;
Dev_Warn_Info *dev_warn_info_list;
Dev_Warn_Msg *dev_warn_msg;

Net_Connection *archive_server_connection = NULL;
unsigned char send_warn_attach_state = 0;

int init_protocol_JSATL(Dev_Bsc_Info **dev_bsc_info_list)
{
	unsigned char i, j = 0;

	*dev_bsc_info_list = (Dev_Bsc_Info*)malloc(sizeof(Dev_Bsc_Info)*DEV_COUNT);

	if(*dev_bsc_info_list == NULL)
	{
		return -1;
	}

	for(i=0; i<DEV_COUNT; i++)
	{
		(*dev_bsc_info_list+i)->dev_ID = ADAS_ID + i;
		(*dev_bsc_info_list+i)->dev_state.working_state = 0x01;
		(*dev_bsc_info_list+i)->dev_state.warnning_state = 0x00;
		(*dev_bsc_info_list+i)->dev_sys_info.company_name_len = 32;
		(*dev_bsc_info_list+i)->dev_sys_info.pdt_model_len = 32;
		(*dev_bsc_info_list+i)->dev_sys_info.hdware_ven_len = 32;
		(*dev_bsc_info_list+i)->dev_sys_info.sfware_ven_len = 32;
		(*dev_bsc_info_list+i)->dev_sys_info.dev_ID_len = 32;
		(*dev_bsc_info_list+i)->dev_sys_info.custm_code_len = 32;
		(*dev_bsc_info_list+i)->dev_sys_info.sys_info_buf = malloc(sizeof(char)*DEV_SYS_INFO_BUF_LEN);

		if((*dev_bsc_info_list+i)->dev_sys_info.sys_info_buf == NULL)
		{
			goto init_protocol_JSATL_error;
		}
	}

	printf("init init_protocol_JSATL successfully!\n");
	return 0;

init_protocol_JSATL_error:
	for(j=0; j<=i; j++)
	{
		free(*dev_bsc_info_list+j);
	}
	return -1;
}


int init_protocol_params(Dev_Params_Info **dev_params_info_list)
{
	unsigned char i, j = 0;

	*dev_params_info_list = (Dev_Params_Info*)malloc(sizeof(Dev_Params_Info)*DEV_COUNT);

	if(*dev_params_info_list == NULL)
	{
		return -1;
	}

	for(i=0; i<DEV_COUNT; i++)
	{
		switch(i)
		{
		case 0:
			(*dev_params_info_list+i)->dev_ID = ADAS_PARAMS_ID;
			(*dev_params_info_list+i)->params_len = ADAS_SET_PARAM_ARRAY_LEN;
			(*dev_params_info_list+i)->params_info_buf = malloc(sizeof(char)*ADAS_SET_PARAM_ARRAY_LEN);

			if((*dev_params_info_list+i)->params_info_buf == NULL)
			{
				goto init_protocol_params_error;
			}

			*((*dev_params_info_list+i)->params_info_buf+ADAS_SPEED_THRESHOLD_INX) = 30;
			*((*dev_params_info_list+i)->params_info_buf+ADAS_TIMING_PHOTO_INTERVAL_INX+1) = 60;
			*((*dev_params_info_list+i)->params_info_buf+ADAS_DISTANCE_PHOTO_INTERVAL_INX+1) = 200;
			*((*dev_params_info_list+i)->params_info_buf+ADAS_PHOTO_COUNT_INX) = 3;
			*((*dev_params_info_list+i)->params_info_buf+ADAS_PHOTO_DISTANCE_INX) = 2;
			*((*dev_params_info_list+i)->params_info_buf+ADAS_PHOTO_RESOLUTION_INX) = 1;
			*((*dev_params_info_list+i)->params_info_buf+ADAS_VIDEO_RESOLUTION_INX) = 1;
			break;

		case 1:
			(*dev_params_info_list+i)->dev_ID = DSM_PARAMS_ID;
			(*dev_params_info_list+i)->params_len = DSM_SET_PARAM_ARRAY_LEN;
			(*dev_params_info_list+i)->params_info_buf = malloc(sizeof(char)*DSM_SET_PARAM_ARRAY_LEN);

			if((*dev_params_info_list+i)->params_info_buf == NULL)
			{
				goto init_protocol_params_error;
			}
			break;

		case 2:
			(*dev_params_info_list+i)->dev_ID = TPMS_PARAMS_ID;
			(*dev_params_info_list+i)->params_len = TPMS_SET_PARAM_ARRAY_LEN;
			(*dev_params_info_list+i)->params_info_buf = malloc(sizeof(char)*TPMS_SET_PARAM_ARRAY_LEN);

			if((*dev_params_info_list+i)->params_info_buf == NULL)
			{
				goto init_protocol_params_error;
			}
			break;

		case 3:
			(*dev_params_info_list+i)->dev_ID = BSD_PARAMS_ID;
			(*dev_params_info_list+i)->params_len = BSD_SET_PARAM_ARRAY_LEN;
			(*dev_params_info_list+i)->params_info_buf = malloc(sizeof(char)*BSD_SET_PARAM_ARRAY_LEN);

			if((*dev_params_info_list+i)->params_info_buf == NULL)
			{
				goto init_protocol_params_error;
			}
			break;
		}
	}

	return 0;

init_protocol_params_error:
	for(j=0; j<=i; j++)
	{

		free(*dev_params_info_list+j);
	}
	return -1;
}


int init_warn_info(Dev_Warn_Info **dev_warn_info_list)
{
	unsigned char i, j = 0;

	*dev_warn_info_list = (Dev_Warn_Info*)malloc(sizeof(Dev_Warn_Info)*DEV_COUNT);

	if(*dev_warn_info_list == NULL)
	{
		return -1;
	}

	for(i=0; i<DEV_COUNT; i++)
	{
		switch(i)
		{
		case 0:
			(*dev_warn_info_list+i)->add_info_ID = ADAS_ID;
			(*dev_warn_info_list+i)->add_info_len = ADAS_ADD_INFO_LEN;
			(*dev_warn_info_list+i)->add_info_buf = malloc(sizeof(char)*ADAS_ADD_INFO_LEN);

			if((*dev_warn_info_list+i)->add_info_buf == NULL)
			{
				goto init_warn_info_error;
			}
			break;

		case 1:
			(*dev_warn_info_list+i)->add_info_ID = DSM_ID;
			(*dev_warn_info_list+i)->add_info_len = DSM_ADD_INFO_LEN;
			(*dev_warn_info_list+i)->add_info_buf = malloc(sizeof(char)*DSM_ADD_INFO_LEN);

			if((*dev_warn_info_list+i)->add_info_buf == NULL)
			{
				goto init_warn_info_error;
			}
			break;

		case 2:
			(*dev_warn_info_list+i)->add_info_ID = TPMS_ID;
			(*dev_warn_info_list+i)->add_info_len = TPMS_ADD_INFO_LEN;
			(*dev_warn_info_list+i)->add_info_buf = malloc(sizeof(char)*TPMS_ADD_INFO_LEN);

			if((*dev_warn_info_list+i)->add_info_buf == NULL)
			{
				goto init_warn_info_error;
			}
			break;

		case 3:
			(*dev_warn_info_list+i)->add_info_ID = BSD_ID;
			(*dev_warn_info_list+i)->add_info_len = BSD_ADD_INFO_LEN;
			(*dev_warn_info_list+i)->add_info_buf = malloc(sizeof(char)*BSD_ADD_INFO_LEN);

			if((*dev_warn_info_list+i)->add_info_buf == NULL)
			{
				goto init_warn_info_error;
			}
			break;
		}
	}

	return 0;

init_warn_info_error:
	for(j=0; j<=i; j++)
	{
		free(*dev_warn_info_list+j);
	}

	return -1;
}


int init_warn_info_msg(Dev_Warn_Msg **dev_warn_msg)
{
	*dev_warn_msg = (Dev_Warn_Msg*) malloc(sizeof(Dev_Warn_Msg));

	if(!*dev_warn_msg)
	{
		return -1;
	}

	return 0;
}


//数据转码
int escape_msg(unsigned char *source, unsigned char *dst, int length)
{
	unsigned char addNum = 0;
	int index = 0;

	if(!source || !dst)
	{
		return -1;
	}

	for (index = 0; index < length; index++)
	{
		if (source[index] == 0x7d)
		{
			dst[index + addNum] = 0x7d;
			addNum += 1;
			dst[index + addNum] = 0x01;
		}
		else if (source[index] == 0x7e)
		{
			dst[index + addNum] = 0x7d;
			addNum += 1;
			dst[index + addNum] = 0x02;
		}
		else
		{
			dst[index + addNum] = source[index];
		}
	}

	return (length + addNum);
}


// 数据反向转码
int rescape_msg(unsigned char *source, unsigned char *dst, int length)
{
	unsigned char subNum = 0;
	int index = 0;

	for (index = 0; index < length; index++)
	{
		if (source[index] == 0x7d)
		{
			if (source[index + 1] == 0x01)
			{
				dst[index - subNum] = 0x7d;
				subNum++;
				index++;
			}
			else if (source[index + 1] == 0x02)
			{
				dst[index - subNum] = 0x7e;
				subNum++;
				index++;
			}
		}
		else
		{
			dst[index - subNum] = source[index];
		}
	}

	return length-subNum;
}


int is_msg_intact(unsigned char *recv_msg, unsigned short msg_len)
{
	unsigned short i = 0;
	unsigned char check_result = 0;
	unsigned short msg_body_len = 0;
	unsigned char encrypt_opt = 0, subpkg_opt = 0, subpkg_entry_len = 0;

	if(!recv_msg || !msg_len)
	{
		return -1;
	}

	for(i=1; i<msg_len-2; i++)
	{
		check_result ^= recv_msg[i];
	}

	msg_body_len = (unsigned short)((*(recv_msg+MSG_PROPERTY_INX)&0x03)<<8)|*(recv_msg+MSG_PROPERTY_INX+1);
	encrypt_opt = (*(recv_msg+MSG_PROPERTY_INX) & 0x1C) >> 2;
	subpkg_opt = (*(recv_msg+MSG_PROPERTY_INX) & 0x20) >> 5;

	if(!subpkg_opt)  //no sub-package entry
	{
		subpkg_entry_len = 0;
	}
	else  //have sub-package entry
	{
		subpkg_entry_len = 4;
	}

	printf("%s: %d line   check_result: %02x, msg_len: %02x \n", __FUNCTION__, __LINE__, check_result, msg_len);
	printf("%s: %d line   recv check: %02x, recv msg len: %02x \n", __FUNCTION__, __LINE__, \
			*(recv_msg+msg_len-2), msg_body_len+15+subpkg_entry_len);

	if((ID_BYTE == *(recv_msg+HEAD_ID_INX)) && \
	   (check_result == *(recv_msg+msg_len-2)) &&\
	   (ID_BYTE == *(recv_msg+msg_len-1)) && \
	   (msg_len == (msg_body_len+15+subpkg_entry_len)))
	{
		return 0;
	}

	return -1;
}


int recv_net_msg(Net_Connection *net_connection, unsigned int recv_timeout_ms)
{
	fd_set rfds;
	struct timeval tv;
	int retval, maxfd = 0;
	int recv_len = 0;
	int rescaped_len = 0;
	unsigned char temp_recv_buf[2048];
	int i = 0;

	if(net_connection->state == DISCONNECTED)
	{
		return -1;
	}

begin_recv:
	FD_ZERO(&rfds);
	FD_SET(net_connection->fd, &rfds);
	maxfd = net_connection->fd;

	tv.tv_sec = recv_timeout_ms/1000;
	tv.tv_usec = recv_timeout_ms*1000;
	retval = select(maxfd+1, &rfds, NULL, NULL, &tv);

	//select error, should exit from program but couldn't here
	if(retval == -1)
	{
		if (errno == EINTR)  //socket still connected
		{
			usleep(500000);

			if(net_connection->retry_cnt++ >= 3)
			{
				goto recv_error_exit;
			}
			else
			{
				goto begin_recv;
			}
		}
		else  //socket disconnected
		{
			goto recv_error_exit;
		}
	}
	else if(retval == 0)  //timeout period expires but socket still connected
	{
		usleep(500000);

		if(net_connection->retry_cnt++ >= 3)
		{
			goto recv_error_exit;
		}
		else
		{
			goto begin_recv;
		}
	}
	else
	{
		if(FD_ISSET(net_connection->fd, &rfds))
		{
			recv_len = recv(net_connection->fd, temp_recv_buf, sizeof(temp_recv_buf), MSG_DONTWAIT); //MSG_PEEK

			if(recv_len > 0)
			{
				for(i=0; i<recv_len; i++)
				{
					printf("%02X ", *(temp_recv_buf+i));
				}

				printf("\n");

				net_connection->retry_cnt = 0;
				rescaped_len = rescape_msg(temp_recv_buf, net_connection->recv_buf + net_connection->recv_buf_msg_len, \
						recv_len);
				net_connection->recv_buf_msg_len += rescaped_len;
				return is_msg_intact(net_connection->recv_buf, net_connection->recv_buf_msg_len);
			}
			else if(recv_len == 0) //socket disconnected!
			{
				goto recv_error_exit;
			}
			else
			{
				if(errno == EINTR) //socket connected
				{
					usleep(500000);

					if(net_connection->retry_cnt++ >= 3)
					{
						goto recv_error_exit;
					}
					else
					{
						goto begin_recv;
					}
				}
				else  //socket disconnected
				{
					goto recv_error_exit;
				}
			}
		}
	}

	return 0;

recv_error_exit:
    net_connection->state = DISCONNECTED;
    net_connection->retry_cnt = 0;
    return -1;
}


unsigned char calc_check_code(unsigned char *buf, unsigned short buf_len)
{
	unsigned char ret_val = 0;
	unsigned short i = 0;

	for(i=0; i<buf_len; i++)
	{
		ret_val ^= *(buf+i);
	}

	return ret_val;
}


int send_net_msg(Net_Connection *net_connection, unsigned short send_msg_ID)
{
	char temp_buf[4096];
	char *raw_send_buf = temp_buf;
	char *escaped_send_buf = temp_buf+2048;
	unsigned short msg_property = 0;
	unsigned short num_of_packages = 0, index_of_subpackage = 1;
	unsigned short raw_send_buf_len = 0, escaped_send_buf_len = 0;

	if(net_connection->state == CONNECTED)
	{
		if(net_connection->send_buf_msg_len <= MAX_MSG_BODY_LEN_OF_SUBPACKAGE)
		{
			*(raw_send_buf+HEAD_ID_INX) = ID_BYTE;
			*(raw_send_buf+MSG_ID_INX) = send_msg_ID>>8;
			*(raw_send_buf+MSG_ID_INX+1) = send_msg_ID&0xFF;
			*(raw_send_buf+MSG_PROPERTY_INX) = (net_connection->send_buf_msg_len) >> 8;
			*(raw_send_buf+MSG_PROPERTY_INX+1) = (net_connection->send_buf_msg_len)&0xFF;
			*(raw_send_buf+PHONE_NUMBER_INX) = 0x21;
			*(raw_send_buf+PHONE_NUMBER_INX+1) = 0x18;
			*(raw_send_buf+PHONE_NUMBER_INX+2) = 0x07;
			*(raw_send_buf+PHONE_NUMBER_INX+3) = 0x03;
			*(raw_send_buf+PHONE_NUMBER_INX+4) = 0x11;
			*(raw_send_buf+PHONE_NUMBER_INX+5) = 0x41;
			*(raw_send_buf+MSG_SN_INX) = send_msg_SN>>8;
			*(raw_send_buf+MSG_SN_INX+1) = send_msg_SN&0xFF;
			send_msg_SN++;
			raw_send_buf_len = MSG_BODY_BEG_INX_NO_SUBPKG;

			memcpy(raw_send_buf+MSG_SN_INX+2, net_connection->send_buf, net_connection->send_buf_msg_len);
			raw_send_buf_len += net_connection->send_buf_msg_len;
			*(raw_send_buf+raw_send_buf_len) = \
					calc_check_code(raw_send_buf+MSG_HEAD_START_INX, raw_send_buf_len-1);
			raw_send_buf_len += 1;

			*(raw_send_buf+raw_send_buf_len) = ID_BYTE;
			raw_send_buf_len += 1;

			net_connection->send_buf_msg_len  = 0;
			escaped_send_buf_len = escape_msg(raw_send_buf+1, escaped_send_buf+1, raw_send_buf_len-2);
			*escaped_send_buf = ID_BYTE;
			*(escaped_send_buf+escaped_send_buf_len+1) = ID_BYTE;
			//printf("line: %d net_connection->fd: %d\n", __LINE__, net_connection->fd);
			return send_spec_len_data(net_connection->fd, (unsigned char*)escaped_send_buf, escaped_send_buf_len+2);
		}

		num_of_packages = net_connection->send_buf_msg_len/MAX_MSG_BODY_LEN_OF_SUBPACKAGE;

		if((num_of_packages > 0) && \
		   (net_connection->send_buf_msg_len%MAX_MSG_BODY_LEN_OF_SUBPACKAGE) > 0)
		{
			num_of_packages++;
		}

		for(index_of_subpackage=1; index_of_subpackage < num_of_packages; index_of_subpackage++)
		{
			*(raw_send_buf+HEAD_ID_INX) = ID_BYTE;
			*(raw_send_buf+MSG_ID_INX) = send_msg_ID>>8;
			*(raw_send_buf+MSG_ID_INX+1) = send_msg_ID&0xFF;
			*(raw_send_buf+MSG_PROPERTY_INX) = 0x23;
			*(raw_send_buf+MSG_PROPERTY_INX+1) = 0xFF;
			*(raw_send_buf+PHONE_NUMBER_INX) = 0x01;
			*(raw_send_buf+PHONE_NUMBER_INX+1) = 0x89;
			*(raw_send_buf+PHONE_NUMBER_INX+2) = 0x15;
			*(raw_send_buf+PHONE_NUMBER_INX+3) = 0x42;
			*(raw_send_buf+PHONE_NUMBER_INX+4) = 0x22;
			*(raw_send_buf+PHONE_NUMBER_INX+5) = 0x76;
			*(raw_send_buf+MSG_SN_INX) = send_msg_SN>>8;
			*(raw_send_buf+MSG_SN_INX+1) = send_msg_SN&0xFF;
			send_msg_SN++;
			raw_send_buf_len = MSG_BODY_BEG_INX_NO_SUBPKG;

			*(raw_send_buf+MSG_TOTAL_PACKAGES_INX) = num_of_packages>>8;
			*(raw_send_buf+MSG_TOTAL_PACKAGES_INX+1) = num_of_packages&0xFF;
			*(raw_send_buf+SEQUENCE_OF_PACKAGE_INX) = index_of_subpackage>>8;
			*(raw_send_buf+SEQUENCE_OF_PACKAGE_INX+1) = index_of_subpackage&0xFF;
			raw_send_buf_len += 4;

			memcpy(raw_send_buf+SEQUENCE_OF_PACKAGE_INX+2, \
					net_connection->send_buf + (index_of_subpackage-1)*MAX_MSG_BODY_LEN_OF_SUBPACKAGE, \
					MAX_MSG_BODY_LEN_OF_SUBPACKAGE);
			raw_send_buf_len += MAX_MSG_BODY_LEN_OF_SUBPACKAGE;
			net_connection->send_buf_msg_len -= MAX_MSG_BODY_LEN_OF_SUBPACKAGE;

			*(raw_send_buf+raw_send_buf_len) = \
					calc_check_code(raw_send_buf+MSG_HEAD_START_INX, raw_send_buf_len-1);
			raw_send_buf_len += 1;

			*(raw_send_buf+raw_send_buf_len) = ID_BYTE;
			raw_send_buf_len += 1;

			escaped_send_buf_len = escape_msg(raw_send_buf+1, escaped_send_buf+1, raw_send_buf_len-2);
			*escaped_send_buf = ID_BYTE;
			*(escaped_send_buf+escaped_send_buf_len+1) = ID_BYTE;
			send_spec_len_data(net_connection->fd, (unsigned char*)escaped_send_buf, escaped_send_buf_len+2);
		}

		*(raw_send_buf+HEAD_ID_INX) = ID_BYTE;
		*(raw_send_buf+MSG_ID_INX) = send_msg_ID>>8;
		*(raw_send_buf+MSG_ID_INX+1) = send_msg_ID&0xFF;
		*(raw_send_buf+MSG_PROPERTY_INX) = (0x2000&(net_connection->send_buf_msg_len)) >> 8;
		*(raw_send_buf+MSG_PROPERTY_INX+1) = (net_connection->send_buf_msg_len) & 0xFF;
		*(raw_send_buf+PHONE_NUMBER_INX) = 0x01;
		*(raw_send_buf+PHONE_NUMBER_INX+1) = 0x89;
		*(raw_send_buf+PHONE_NUMBER_INX+2) = 0x15;
		*(raw_send_buf+PHONE_NUMBER_INX+3) = 0x42;
		*(raw_send_buf+PHONE_NUMBER_INX+4) = 0x22;
		*(raw_send_buf+PHONE_NUMBER_INX+5) = 0x76;
		*(raw_send_buf+MSG_SN_INX) = send_msg_SN>>8;
		*(raw_send_buf+MSG_SN_INX+1) = send_msg_SN&0xFF;
		send_msg_SN++;
		raw_send_buf_len = MSG_BODY_BEG_INX_NO_SUBPKG;

		*(raw_send_buf+MSG_TOTAL_PACKAGES_INX) = num_of_packages>>8;
		*(raw_send_buf+MSG_TOTAL_PACKAGES_INX+1) = num_of_packages&0xFF;
		*(raw_send_buf+SEQUENCE_OF_PACKAGE_INX) = index_of_subpackage>>8;
		*(raw_send_buf+SEQUENCE_OF_PACKAGE_INX+1) = index_of_subpackage&0xFF;
		raw_send_buf_len += 4;

		memcpy(raw_send_buf+SEQUENCE_OF_PACKAGE_INX+2, \
				net_connection->send_buf + (index_of_subpackage-1)*MAX_MSG_BODY_LEN_OF_SUBPACKAGE, \
				net_connection->send_buf_msg_len);
		raw_send_buf_len += net_connection->send_buf_msg_len;

		*(raw_send_buf+raw_send_buf_len) = \
				calc_check_code(raw_send_buf+MSG_HEAD_START_INX, raw_send_buf_len-1);
		raw_send_buf_len += 1;

		*(raw_send_buf+raw_send_buf_len) = ID_BYTE;
		raw_send_buf_len += 1;

		net_connection->send_buf_msg_len = 0;
		escaped_send_buf_len = escape_msg(raw_send_buf, escaped_send_buf, raw_send_buf_len);
		*escaped_send_buf = ID_BYTE;
		*(escaped_send_buf+escaped_send_buf_len+1) = ID_BYTE;
		send_spec_len_data(net_connection->fd, (unsigned char*)escaped_send_buf, escaped_send_buf_len+2);
		return 1;
	}
	else
	{
		return -1;
	}
}




int query_basic_info(Net_Connection *net_connection)
{
	unsigned char i = 0;
	unsigned char dev_Id_list_count = 0;
	unsigned short offset = 0;

	*net_connection->send_buf = *(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG);
	*(net_connection->send_buf+1) = *(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG+1);
	dev_Id_list_count = *(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG+1);
	net_connection->send_buf_msg_len = 2;

	if(*(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG) == 0xF7)  //state query
	{
		for(i=0; i<DEV_COUNT; i++)
		{
			if((dev_bsc_info_list+i)->dev_ID == *(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG+2))
			{
				*(net_connection->send_buf + net_connection->send_buf_msg_len) = \
						(dev_bsc_info_list+i)->dev_state.working_state;
				net_connection->send_buf_msg_len += 1;

				*(net_connection->send_buf + net_connection->send_buf_msg_len) = \
						((dev_bsc_info_list+i)->dev_state.warnning_state) >> 24;
				*(net_connection->send_buf + net_connection->send_buf_msg_len+1) = \
						(((dev_bsc_info_list+i)->dev_state.warnning_state) >> 16) & 0xFF;
				*(net_connection->send_buf + net_connection->send_buf_msg_len+2) = \
						(((dev_bsc_info_list+i)->dev_state.warnning_state) >> 8) & 0xFF;
				*(net_connection->send_buf + net_connection->send_buf_msg_len+3) = \
						((dev_bsc_info_list+i)->dev_state.warnning_state) & 0xFF;
				net_connection->send_buf_msg_len += 4;
			}
		}
	}
	else  // information query
	{
		for(i=0; i<DEV_COUNT; i++)
		{
			if((dev_bsc_info_list+i)->dev_ID == *(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG+2))
			{
				offset = 0;
				*(net_connection->send_buf + net_connection->send_buf_msg_len) = \
						(dev_bsc_info_list+i)->dev_sys_info.company_name_len;
				net_connection->send_buf_msg_len += 1;
				memcpy((net_connection->send_buf + net_connection->send_buf_msg_len),\
					   (dev_bsc_info_list+i)->dev_sys_info.sys_info_buf + offset, \
					   (dev_bsc_info_list+i)->dev_sys_info.company_name_len);
				net_connection->send_buf_msg_len += (dev_bsc_info_list+i)->dev_sys_info.company_name_len;

				offset += (dev_bsc_info_list+i)->dev_sys_info.company_name_len;
				*(net_connection->send_buf + net_connection->send_buf_msg_len) = \
						(dev_bsc_info_list+i)->dev_sys_info.pdt_model_len;
				net_connection->send_buf_msg_len += 1;
				memcpy((net_connection->send_buf + net_connection->send_buf_msg_len),\
					   (dev_bsc_info_list+i)->dev_sys_info.sys_info_buf + offset, \
					   (dev_bsc_info_list+i)->dev_sys_info.pdt_model_len);
				net_connection->send_buf_msg_len += (dev_bsc_info_list+i)->dev_sys_info.pdt_model_len;

				offset += (dev_bsc_info_list+i)->dev_sys_info.pdt_model_len;
				*(net_connection->send_buf + net_connection->send_buf_msg_len) = \
						(dev_bsc_info_list+i)->dev_sys_info.hdware_ven_len;
				net_connection->send_buf_msg_len += 1;
				memcpy((net_connection->send_buf + net_connection->send_buf_msg_len),\
					   (dev_bsc_info_list+i)->dev_sys_info.sys_info_buf + offset, \
					   (dev_bsc_info_list+i)->dev_sys_info.hdware_ven_len);
				net_connection->send_buf_msg_len += (dev_bsc_info_list+i)->dev_sys_info.hdware_ven_len;

				offset += (dev_bsc_info_list+i)->dev_sys_info.hdware_ven_len;
				*(net_connection->send_buf + net_connection->send_buf_msg_len) = \
						(dev_bsc_info_list+i)->dev_sys_info.sfware_ven_len;
				net_connection->send_buf_msg_len += 1;
				memcpy((net_connection->send_buf + net_connection->send_buf_msg_len),\
					   (dev_bsc_info_list+i)->dev_sys_info.sys_info_buf + offset, \
					   (dev_bsc_info_list+i)->dev_sys_info.sfware_ven_len);
				net_connection->send_buf_msg_len += (dev_bsc_info_list+i)->dev_sys_info.sfware_ven_len;

				offset += (dev_bsc_info_list+i)->dev_sys_info.sfware_ven_len;
				*(net_connection->send_buf + net_connection->send_buf_msg_len) = \
						(dev_bsc_info_list+i)->dev_sys_info.dev_ID_len;
				net_connection->send_buf_msg_len += 1;
				memcpy((net_connection->send_buf + net_connection->send_buf_msg_len),\
					   (dev_bsc_info_list+i)->dev_sys_info.sys_info_buf + offset, \
					   (dev_bsc_info_list+i)->dev_sys_info.dev_ID_len);
				net_connection->send_buf_msg_len += (dev_bsc_info_list+i)->dev_sys_info.dev_ID_len;

				offset += (dev_bsc_info_list+i)->dev_sys_info.dev_ID_len;
				*(net_connection->send_buf + net_connection->send_buf_msg_len) = \
						(dev_bsc_info_list+i)->dev_sys_info.custm_code_len;
				net_connection->send_buf_msg_len += 1;
				memcpy((net_connection->send_buf + net_connection->send_buf_msg_len),\
					   (dev_bsc_info_list+i)->dev_sys_info.sys_info_buf + offset, \
					   (dev_bsc_info_list+i)->dev_sys_info.custm_code_len);
				net_connection->send_buf_msg_len += (dev_bsc_info_list+i)->dev_sys_info.custm_code_len;
			}
		}
	}

	net_connection->recv_buf_msg_len = 0;
	send_net_msg(net_connection, TERMINAL_BSC_INFO_REPLY);
	return 0;
}


int report_basic_location_info(Net_Connection *net_connection)
{
	unsigned int latitude = 31126493;
	unsigned int longitude = 120679336;
	unsigned short altitude = 599;
	unsigned char temp_calc = 0;

    time_t nSeconds;
    struct tm *pTM;

	net_connection->send_buf_msg_len = 0;
	memset(net_connection->send_buf+BLI_WARN_FLAG_INX, 0, 4);
	memset(net_connection->send_buf+BLI_STATE_INX, 0, 4);

	*(net_connection->send_buf+BLI_LATITUDE_INX) = \
			latitude>>24;
	*(net_connection->send_buf+BLI_LATITUDE_INX+1) = \
			latitude>>16;
	*(net_connection->send_buf+BLI_LATITUDE_INX+2) = \
			latitude>>8;
	*(net_connection->send_buf+BLI_LATITUDE_INX+3) = \
			latitude&0xFF;

	*(net_connection->send_buf+BLI_LONGITUDE_INX) = \
			longitude>>24;
	*(net_connection->send_buf+BLI_LONGITUDE_INX+1) = \
			longitude>>16;
	*(net_connection->send_buf+BLI_LONGITUDE_INX+2) = \
			longitude>>8;
	*(net_connection->send_buf+BLI_LONGITUDE_INX+3) = \
			longitude&0xFF;

	*(net_connection->send_buf+BLI_ALTITUDE_INX) = altitude >> 8;
	*(net_connection->send_buf+BLI_ALTITUDE_INX+1) = altitude & 0xFF;

	*(net_connection->send_buf+BLI_SPEED_INX) = 0;
	*(net_connection->send_buf+BLI_SPEED_INX+1) = 0;

	*(net_connection->send_buf+BLI_DIRECTION_INX) = 0;
	*(net_connection->send_buf+BLI_DIRECTION_INX+1) = 0;

    time(&nSeconds);
    pTM = localtime(&nSeconds);

	temp_calc = (((pTM->tm_year+1900)%100)/10) << 4 | ((pTM->tm_year+1900)%10);
	*(net_connection->send_buf+BLI_DATE_TIME_INX) = temp_calc;

	temp_calc = ((pTM->tm_mon+1)/10) << 4 | ((pTM->tm_mon+1)%10);
	*(net_connection->send_buf+BLI_DATE_TIME_INX+1) = temp_calc;

	temp_calc = (pTM->tm_mday/10) << 4 | (pTM->tm_mday%10);
	*(net_connection->send_buf+BLI_DATE_TIME_INX+2) = temp_calc;

	temp_calc = (pTM->tm_hour/10) << 4 | (pTM->tm_hour%10);
	*(net_connection->send_buf+BLI_DATE_TIME_INX+3) = temp_calc;

	temp_calc = (pTM->tm_min/10) << 4 | (pTM->tm_min%10);
	*(net_connection->send_buf+BLI_DATE_TIME_INX+4) = temp_calc;

	temp_calc = (pTM->tm_sec/10) << 4 | (pTM->tm_sec%10);
	*(net_connection->send_buf+BLI_DATE_TIME_INX+5) = temp_calc;
	net_connection->send_buf_msg_len = BLI_ARRAY_LNE;

	send_net_msg(net_connection, 0x0200);  // basic location report
	return 0;
}


int send_warn_info(Net_Connection *net_connection, unsigned char add_info_id)
{
	static unsigned int warn_id = 0;
	unsigned int latitude = 31126493;
	unsigned int longitude = 120679336;
	unsigned short altitude = 599;
	unsigned char temp_calc = 0;

    time_t nSeconds;
    struct tm *pTM;

	net_connection->send_buf_msg_len = 0;
	memset(net_connection->send_buf+BLI_WARN_FLAG_INX, 0, 4);
	memset(net_connection->send_buf+BLI_STATE_INX, 0, 4);

	*(net_connection->send_buf+BLI_LATITUDE_INX) = \
			latitude >> 24;
	*(net_connection->send_buf+BLI_LATITUDE_INX+1) = \
			latitude >> 16;
	*(net_connection->send_buf+BLI_LATITUDE_INX+2) = \
			latitude >> 8;
	*(net_connection->send_buf+BLI_LATITUDE_INX+3) = \
			latitude & 0xFF;

	*(net_connection->send_buf+BLI_LONGITUDE_INX) = \
			longitude>>24;
	*(net_connection->send_buf+BLI_LONGITUDE_INX+1) = \
			longitude>>16;
	*(net_connection->send_buf+BLI_LONGITUDE_INX+2) = \
			longitude>>8;
	*(net_connection->send_buf+BLI_LONGITUDE_INX+3) = \
			longitude&0xFF;

	*(net_connection->send_buf+BLI_ALTITUDE_INX) = altitude >> 8;
	*(net_connection->send_buf+BLI_ALTITUDE_INX+1) = altitude & 0xFF;

	*(net_connection->send_buf+BLI_SPEED_INX) = 0;
	*(net_connection->send_buf+BLI_SPEED_INX+1) = 0;

	*(net_connection->send_buf+BLI_DIRECTION_INX) = 0;
	*(net_connection->send_buf+BLI_DIRECTION_INX+1) = 0;

    time(&nSeconds);
    pTM = localtime(&nSeconds);

	temp_calc = (((pTM->tm_year+1900)%100)/10) << 4 | ((pTM->tm_year+1900)%10);
	*(net_connection->send_buf+BLI_DATE_TIME_INX) = temp_calc;

	temp_calc = ((pTM->tm_mon+1)/10) << 4 | ((pTM->tm_mon+1)%10);
	*(net_connection->send_buf+BLI_DATE_TIME_INX+1) = temp_calc;

	temp_calc = (pTM->tm_mday/10) << 4 | (pTM->tm_mday%10);
	*(net_connection->send_buf+BLI_DATE_TIME_INX+2) = temp_calc;

	temp_calc = (pTM->tm_hour/10) << 4 | (pTM->tm_hour%10);
	*(net_connection->send_buf+BLI_DATE_TIME_INX+3) = temp_calc;

	temp_calc = (pTM->tm_min/10) << 4 | (pTM->tm_min%10);
	*(net_connection->send_buf+BLI_DATE_TIME_INX+4) = temp_calc;

	temp_calc = (pTM->tm_sec/10) << 4 | (pTM->tm_sec%10);
	*(net_connection->send_buf+BLI_DATE_TIME_INX+5) = temp_calc;
	net_connection->send_buf_msg_len = BLI_ARRAY_LNE;

	*(net_connection->send_buf+WARN_ADD_INFO_ID_INX) = add_info_id;
	warn_id += 1;

	switch(add_info_id)
	{
	case ADAS_ID:
		*(net_connection->send_buf+WARN_ADD_INFO_LEN_INX) = dev_warn_info_list->add_info_len;
		net_connection->send_buf_msg_len += 2;
		memcpy(net_connection->send_buf+ADAS_ADD_INFO_WARN_ID_INX, dev_warn_info_list->add_info_buf, \
				ADAS_ADD_INFO_LEN);

		*(net_connection->send_buf+ADAS_ADD_INFO_FLAG_STATE_INX) = 0;
		*(net_connection->send_buf+ADAS_ADD_INFO_WARN_TYPE_INX) = 1;
		*(net_connection->send_buf+ADAS_ADD_INFO_WARN_LEVEL_INX) = 1;
		*(net_connection->send_buf+ADAS_ADD_INFO_FRONT_CAR_SPEED_INX) = 100;
		*(net_connection->send_buf+ADAS_ADD_INFO_FRONT_CAR_DISTANCE_INX) = 5;
		*(net_connection->send_buf+ADAS_ADD_INFO_DEPARTURE_TYPE_INX) = 0;
		*(net_connection->send_buf+ADAS_ADD_INFO_SPEED_INX) = 120;
		*(net_connection->send_buf+ADAS_ADD_INFO_ALTITUDE_INX) = 500 >> 8;
		*(net_connection->send_buf+ADAS_ADD_INFO_ALTITUDE_INX+1) = 500 & 0xFF;
		*(net_connection->send_buf+ADAS_ADD_INFO_LATITUDE_INX) = latitude >> 24;
		*(net_connection->send_buf+ADAS_ADD_INFO_LATITUDE_INX+1) = latitude >> 16;
		*(net_connection->send_buf+ADAS_ADD_INFO_LATITUDE_INX+2) = latitude >> 8;
		*(net_connection->send_buf+ADAS_ADD_INFO_LATITUDE_INX+3) = latitude;
		*(net_connection->send_buf+ADAS_ADD_INFO_LONGITUDE_INX) = longitude >> 24;
		*(net_connection->send_buf+ADAS_ADD_INFO_LONGITUDE_INX+1) = longitude >> 16;
		*(net_connection->send_buf+ADAS_ADD_INFO_LONGITUDE_INX+2) = longitude >> 8;
		*(net_connection->send_buf+ADAS_ADD_INFO_LONGITUDE_INX+3) = longitude;


		temp_calc = (((pTM->tm_year+1900)%100)/10) << 4 | ((pTM->tm_year+1900)%10);
		*(net_connection->send_buf+ADAS_ADD_INFO_DATE_TIME_INX) = temp_calc;

		temp_calc = ((pTM->tm_mon+1)/10) << 4 | ((pTM->tm_mon+1)%10);
		*(net_connection->send_buf+ADAS_ADD_INFO_DATE_TIME_INX+1) = temp_calc;

		temp_calc = (pTM->tm_mday/10) << 4 | (pTM->tm_mday%10);
		*(net_connection->send_buf+ADAS_ADD_INFO_DATE_TIME_INX+2) = temp_calc;

		temp_calc = (pTM->tm_hour/10) << 4 | (pTM->tm_hour%10);
		*(net_connection->send_buf+ADAS_ADD_INFO_DATE_TIME_INX+3) = temp_calc;

		temp_calc = (pTM->tm_min/10) << 4 | (pTM->tm_min%10);
		*(net_connection->send_buf+ADAS_ADD_INFO_DATE_TIME_INX+4) = temp_calc;

		temp_calc = (pTM->tm_sec/10) << 4 | (pTM->tm_sec%10);
		*(net_connection->send_buf+ADAS_ADD_INFO_DATE_TIME_INX+5) = temp_calc;
		memcpy(net_connection->send_buf+ADAS_ADD_INFO_WARN_ID_NUM_INX, "abc1234", 7);
		memcpy(net_connection->send_buf+ADAS_ADD_INFO_WARN_ID_NUM_INX+7, \
				net_connection->send_buf+ADAS_ADD_INFO_DATE_TIME_INX, 6);
		*(net_connection->send_buf+ADAS_ADD_INFO_WARN_ID_NUM_INX+13) = 0;
		*(net_connection->send_buf+ADAS_ADD_INFO_WARN_ID_NUM_INX+14) = 2;

		net_connection->send_buf_msg_len += ADAS_ADD_INFO_LEN;
		break;

	case DSM_ID:
		*(net_connection->send_buf+WARN_ADD_INFO_LEN_INX) = (dev_warn_info_list+1)->add_info_len;
		memcpy(net_connection->send_buf+DSM_ADD_INFO_WARN_ID_INX, (dev_warn_info_list+1)->add_info_buf, \
				DSM_ADD_INFO_LEN);
		net_connection->send_buf_msg_len += 2;
		net_connection->send_buf_msg_len += DSM_ADD_INFO_LEN;
//		temp_calc = (((pTM->tm_year+1900)%100)/10) << 4 | ((pTM->tm_year+1900)%10);
//		*(net_connection->send_buf+DSM_ADD_INFO_DATE_TIME_INX) = temp_calc;
//
//		temp_calc = ((pTM->tm_mon+1)/10) << 4 | ((pTM->tm_mon+1)%10);
//		*(net_connection->send_buf+DSM_ADD_INFO_DATE_TIME_INX+1) = temp_calc;
//
//		temp_calc = (pTM->tm_mday/10) << 4 | (pTM->tm_mday%10);
//		*(net_connection->send_buf+DSM_ADD_INFO_DATE_TIME_INX+2) = temp_calc;
//
//		temp_calc = (pTM->tm_hour/10) << 4 | (pTM->tm_hour%10);
//		*(net_connection->send_buf+DSM_ADD_INFO_DATE_TIME_INX+3) = temp_calc;
//
//		temp_calc = (pTM->tm_min/10) << 4 | (pTM->tm_min%10);
//		*(net_connection->send_buf+DSM_ADD_INFO_DATE_TIME_INX+4) = temp_calc;
//
//		temp_calc = (pTM->tm_sec/10) << 4 | (pTM->tm_sec%10);
//		*(net_connection->send_buf+DSM_ADD_INFO_DATE_TIME_INX+5) = temp_calc;
		break;

	case TPMS_ID:
		*(net_connection->send_buf+WARN_ADD_INFO_LEN_INX) = (dev_warn_info_list+2)->add_info_len;
		memcpy(net_connection->send_buf+TPMS_ADD_INFO_WARN_ID_INX, (dev_warn_info_list+2)->add_info_buf, \
				TPMS_ADD_INFO_LEN);
		net_connection->send_buf_msg_len += 2;
		net_connection->send_buf_msg_len += TPMS_ADD_INFO_LEN;
//		temp_calc = (((pTM->tm_year+1900)%100)/10) << 4 | ((pTM->tm_year+1900)%10);
//		*(net_connection->send_buf+TPMS_ADD_INFO_DATE_TIME_INX) = temp_calc;
//
//		temp_calc = ((pTM->tm_mon+1)/10) << 4 | ((pTM->tm_mon+1)%10);
//		*(net_connection->send_buf+TPMS_ADD_INFO_DATE_TIME_INX+1) = temp_calc;
//
//		temp_calc = (pTM->tm_mday/10) << 4 | (pTM->tm_mday%10);
//		*(net_connection->send_buf+TPMS_ADD_INFO_DATE_TIME_INX+2) = temp_calc;
//
//		temp_calc = (pTM->tm_hour/10) << 4 | (pTM->tm_hour%10);
//		*(net_connection->send_buf+TPMS_ADD_INFO_DATE_TIME_INX+3) = temp_calc;
//
//		temp_calc = (pTM->tm_min/10) << 4 | (pTM->tm_min%10);
//		*(net_connection->send_buf+TPMS_ADD_INFO_DATE_TIME_INX+4) = temp_calc;
//
//		temp_calc = (pTM->tm_sec/10) << 4 | (pTM->tm_sec%10);
//		*(net_connection->send_buf+TPMS_ADD_INFO_DATE_TIME_INX+5) = temp_calc;
		break;

	case BSD_ID:
//		*(net_connection->send_buf+WARN_ADD_INFO_LEN_INX) = (dev_warn_info_list+3)->add_info_len;
//		memcpy(net_connection->send_buf+BSD_ADD_INFO_WARN_ID_INX, (dev_warn_info_list+3)->add_info_buf, \
//				BSD_ADD_INFO_LEN);
//		net_connection->send_buf_msg_len += 2;
//		net_connection->send_buf_msg_len += BSD_ADD_INFO_LEN;
//		temp_calc = (((pTM->tm_year+1900)%100)/10) << 4 | ((pTM->tm_year+1900)%10);
//		*(net_connection->send_buf+BSD_ADD_INFO_DATE_TIME_INX) = temp_calc;
//
//		temp_calc = ((pTM->tm_mon+1)/10) << 4 | ((pTM->tm_mon+1)%10);
//		*(net_connection->send_buf+BSD_ADD_INFO_DATE_TIME_INX+1) = temp_calc;
//
//		temp_calc = (pTM->tm_mday/10) << 4 | (pTM->tm_mday%10);
//		*(net_connection->send_buf+BSD_ADD_INFO_DATE_TIME_INX+2) = temp_calc;
//
//		temp_calc = (pTM->tm_hour/10) << 4 | (pTM->tm_hour%10);
//		*(net_connection->send_buf+BSD_ADD_INFO_DATE_TIME_INX+3) = temp_calc;
//
//		temp_calc = (pTM->tm_min/10) << 4 | (pTM->tm_min%10);
//		*(net_connection->send_buf+BSD_ADD_INFO_DATE_TIME_INX+4) = temp_calc;
//
//		temp_calc = (pTM->tm_sec/10) << 4 | (pTM->tm_sec%10);
//		*(net_connection->send_buf+BSD_ADD_INFO_DATE_TIME_INX+5) = temp_calc;
		break;
	}

	*(net_connection->send_buf+ADAS_ADD_INFO_WARN_ID_INX) = warn_id >> 24;
	*(net_connection->send_buf+ADAS_ADD_INFO_WARN_ID_INX+1) = warn_id >> 16;
	*(net_connection->send_buf+ADAS_ADD_INFO_WARN_ID_INX+2) = warn_id >> 8;
	*(net_connection->send_buf+ADAS_ADD_INFO_WARN_ID_INX+3) = warn_id & 0xFF;
	send_net_msg(net_connection, 0x0200);  //
	return 0;
}


void authen_and_warn_process_task(void *connection)
{
	Net_Connection *net_connection = (Net_Connection *)connection;

	unsigned short province_ID = 0, city_ID = 0;
	char vendor_Id[5] = { 0x02, 0x00, 0x01, 0x03, 0xFF };
	char model[8] = {'T', '7', 'J', 'S', 'A', 'T', 'L', '1'};
	char terminal_ID[20] = {'J', 'S', 'A', 'T', 'L', '0', '1'};
	unsigned char plate_color;
	char plate_number[16] = "苏E654321";
	unsigned char i = 0;
	unsigned char temp_len = 0;
	char test_file_name[64] = "01_64_6401_01_";

	unsigned char bin_file[64];
	unsigned int latitude = 31126493;
	unsigned int longitude = 120679336;
	unsigned short altitude = 599;
	unsigned int bin_file_check = 0;

authen:
	//1. send authen information
	strcpy(net_connection->send_buf, authen_str);
	net_connection->send_buf_msg_len = strlen(authen_str);
	net_connection->state = CONNECTED;
	send_net_msg(net_connection, 0x0102);
	BG_reply_val == 0xFF;

	pthread_mutex_lock(&wait_recv_mut);

	while(BG_reply_val == 0xFF)
	{
		pthread_cond_wait(&wait_recv_cond, &wait_recv_mut);
	}

	pthread_mutex_unlock(&wait_recv_mut);

	if((BG_reply_val == 0) || (BG_reply_val == 3))  //authen successfully nor not support
	{
		goto cycle_send;
	}
	else if((BG_reply_val == 1) || (BG_reply_val == 2))  //authen failed or message error
	{
terminal_register:
		// terminal register
		*net_connection->send_buf = province_ID>>8;
		*(net_connection->send_buf+1) = province_ID&0xFF;
		*(net_connection->send_buf+2) = city_ID>>8;
		*(net_connection->send_buf+3) = city_ID&0xFF;
		net_connection->send_buf_msg_len = 4;
		memcpy(net_connection->send_buf+net_connection->send_buf_msg_len, vendor_Id, sizeof(vendor_Id));
		net_connection->send_buf_msg_len += sizeof(vendor_Id);
		memcpy(net_connection->send_buf+net_connection->send_buf_msg_len, model, sizeof(model));
		net_connection->send_buf_msg_len += sizeof(model);
		memcpy(net_connection->send_buf+net_connection->send_buf_msg_len, terminal_ID, sizeof(terminal_ID));
		net_connection->send_buf_msg_len += sizeof(terminal_ID);
		*(net_connection->send_buf+net_connection->send_buf_msg_len) = 1; //1: blue, 2: yellow, 3: black, 4: white, 9: others
		net_connection->send_buf_msg_len += 1;
		memcpy(net_connection->send_buf+net_connection->send_buf_msg_len, plate_number, strlen(plate_number));
		net_connection->send_buf_msg_len += strlen(plate_number);
		send_net_msg(net_connection, 0x0100);
		BG_reply_val = 0xFF;

		pthread_mutex_lock(&wait_recv_mut);

		while(BG_reply_val == 0xFF)
		{
			pthread_cond_wait(&wait_recv_cond, &wait_recv_mut);
		}

		pthread_mutex_unlock(&wait_recv_mut);

		if(BG_reply_val == 0)  //register successfully
		{
			goto authen;
		}
		else
		{
			goto terminal_register;
		}
	}

cycle_send:
//sleep(2);
send_warn_info(net_connection, 0x64);
	while(1)
	{
		if(archive_server_connection && !recv_net_msg(archive_server_connection, 3000))
		{
			printf("recv data: ");

			for(i=0; i<net_connection->recv_buf_msg_len; i++)
			{
				printf("%02x ", *(archive_server_connection->recv_buf+i));
			}

			printf("\n");
			parse_recv_msg(archive_server_connection);
		}
		else
		{
			sleep(3);
		}
	}
}


int BG_genernal_reply(Net_Connection *net_connection)
{
	int fun_ret = 0;

	unsigned short reply_SN = 0;
	unsigned short reply_ID = 0;
	unsigned char reply_ret = 0;
	static unsigned char use_once = 0;

	reply_SN = (*(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG)<<8)|\
			*(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG+1);
	reply_ID = (*(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG+2)<<8)|\
			*(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG+3);
	reply_ret = *(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG+4);

	switch(reply_ID)
	{
	case TERMINAL_AUTHEN:  //authen
		BG_reply_val = reply_ret;
		pthread_mutex_lock(&wait_recv_mut);
		pthread_cond_signal(&wait_recv_cond);
		pthread_mutex_unlock(&wait_recv_mut);
		fun_ret = reply_ret;
		break;

	case TERMINAL_SEND_WARN_ATTACH_INFO:  //0x1210
		dev_warn_msg->file_upload_seq = UPLOAD_FILE_INFO;
		break;

	case TERMINAL_SEND_WARN_ATTACH_DATA:  //0x1211
		dev_warn_msg->file_upload_seq = UPLOAD_FILE_DATA;
		break;

	default:
		break;
	}

	return fun_ret;
}

int BG_register_reply(Net_Connection *net_connection)
{
	int fun_ret = 0;
	unsigned short reply_SN = 0;
	unsigned char reply_ret = 0;

	reply_SN = (*(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG)<<8)|\
			*(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG+1);
	reply_ret = *(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG+2);

	if(reply_ret == 0)  //register successfully
	{
		fun_ret = 0;
		memset(authen_str, '\0', sizeof(authen_str));
		memcpy(authen_str, net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG+3, \
				net_connection->recv_buf_msg_len-15-3);
	}
	else if(reply_ret == 1 || reply_ret == 3)  //vehicle or terminal was already registered
	{
		fun_ret = 1;
	}
	else if(reply_ret == 2 || reply_ret == 4)  //vehicle or terminal was not added into DB
	{
		fun_ret = -1;
	}
	else //
	{
		fun_ret = -1;
	}

	pthread_mutex_lock(&wait_recv_mut);
	BG_reply_val = fun_ret;
	pthread_cond_signal(&wait_recv_cond);
	pthread_mutex_unlock(&wait_recv_mut);

	net_connection->recv_buf_msg_len = 0;
	return fun_ret;
}

int set_dev_param_reply(Net_Connection *net_connection)
{
	unsigned char i = 0, j = 0, k = 0;
	unsigned off_set = 0, dev_cnt = 0;
	unsigned char recv_param_len = 0;
	unsigned int param_ID = 0;

	net_connection->send_buf_msg_len = 0;
	memcpy(net_connection->send_buf+net_connection->send_buf_msg_len, \
			net_connection->recv_buf+MSG_SN_INX, 2);
	net_connection->send_buf_msg_len += 2;

	memcpy(net_connection->send_buf+net_connection->send_buf_msg_len, \
			net_connection->recv_buf+MSG_ID_INX, 2);
	net_connection->send_buf_msg_len += 2;

	dev_cnt = *(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG);
	off_set = MSG_BODY_BEG_INX_NO_SUBPKG+1;

	for(i=0; i<dev_cnt; i++)
	{
		param_ID = *(net_connection->recv_buf+off_set) << 24 | \
				   *(net_connection->recv_buf+off_set+1) << 16 | \
				   *(net_connection->recv_buf+off_set+2) << 8 | \
				   *(net_connection->recv_buf+off_set+3);
		recv_param_len = *(net_connection->recv_buf+off_set+4);

		printf("param_ID: %04x, param_len: %02x\n", param_ID, recv_param_len);

		for(j=0; j<DEV_COUNT; j++)
		{
			if(((dev_params_info_list+j)->dev_ID == param_ID) && \
			   ((dev_params_info_list+j)->params_len == recv_param_len))
			{
				for(k=0; k<recv_param_len; k++)
				{
					if(*(net_connection->recv_buf+off_set+5+k) != 0xFF)
					{
						*((dev_params_info_list+j)->params_info_buf+k) = \
								*(net_connection->recv_buf+off_set+5+k);
					}
				}

				off_set += recv_param_len;
				break;
			}
		}

		if(j >= DEV_COUNT)
		{
			net_connection->recv_buf_msg_len = 0;
			*(net_connection->send_buf+net_connection->send_buf_msg_len) = 2;  //msg error
			net_connection->send_buf_msg_len += 1;
			send_net_msg(net_connection, TERMINAL_GENERAL_REPLY);
			return -1;
		}
	}

	*(net_connection->send_buf+net_connection->send_buf_msg_len) = 0;  //successfully
	net_connection->send_buf_msg_len += 1;
	send_net_msg(net_connection, TERMINAL_GENERAL_REPLY);
	return 0;
}


int query_specify_dev_param_reply(Net_Connection *net_connection)
{
	unsigned char dev_cnt = *(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG);
	unsigned char i = 0, j = 0;
	unsigned int param_ID = 0;

	if(dev_cnt <= 0)
	{
		return -1;
	}

	net_connection->send_buf_msg_len = 0;
	memcpy(net_connection->send_buf+net_connection->send_buf_msg_len,\
		   net_connection->recv_buf+MSG_SN_INX, 2);
	net_connection->send_buf_msg_len += 2;
	*(net_connection->send_buf+net_connection->send_buf_msg_len) = dev_cnt;
	net_connection->send_buf_msg_len += 1;

	for(i=0; i<dev_cnt; i++)
	{
		param_ID = *(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG+1+4*i)<<24 | \
				   *(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG+1+4*i+1)<<16 | \
				   *(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG+1+4*i+2)<<8 | \
				   *(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG+1+4*i+3);

		for(j=0; j<DEV_COUNT; j++)
		{
			if(param_ID == (dev_params_info_list+j)->dev_ID)
			{
				*(net_connection->send_buf+net_connection->send_buf_msg_len) = \
						*(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG+1+4*i);
				*(net_connection->send_buf+net_connection->send_buf_msg_len+1) = \
						*(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG+1+4*i+1);
				*(net_connection->send_buf+net_connection->send_buf_msg_len+2) = \
						*(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG+1+4*i+2);
				*(net_connection->send_buf+net_connection->send_buf_msg_len+3) = \
						*(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG+1+4*i+3);
				net_connection->send_buf_msg_len += 4;

				*(net_connection->send_buf+net_connection->send_buf_msg_len) = \
						(dev_params_info_list+j)->params_len;
				net_connection->send_buf_msg_len += 1;

				memcpy(net_connection->send_buf+net_connection->send_buf_msg_len, \
					   (dev_params_info_list+j)->params_info_buf, (dev_params_info_list+j)->params_len);
				net_connection->send_buf_msg_len += (dev_params_info_list+j)->params_len;
			}
		}
	}

	net_connection->recv_buf_msg_len = 0;
	send_net_msg(net_connection, TERMINAL_QUERY_PARAM_REPLY);

	return 0;
}


int query_all_dev_param_reply(Net_Connection *net_connection)
{
	unsigned char i = 0;

	net_connection->send_buf_msg_len = 0;
	memcpy(net_connection->send_buf+net_connection->send_buf_msg_len,\
		   net_connection->recv_buf+MSG_SN_INX, 2);
	net_connection->send_buf_msg_len += 2;
	*(net_connection->send_buf+net_connection->send_buf_msg_len) = DEV_COUNT;
	net_connection->send_buf_msg_len += 1;

	for(i=0; i<DEV_COUNT; i++)
	{
		*(net_connection->send_buf+net_connection->send_buf_msg_len) = \
				((dev_params_info_list+i)->dev_ID) >> 24;
		*(net_connection->send_buf+net_connection->send_buf_msg_len+1) = \
				((dev_params_info_list+i)->dev_ID) >> 16;
		*(net_connection->send_buf+net_connection->send_buf_msg_len+2) = \
				((dev_params_info_list+i)->dev_ID) >> 8;
		*(net_connection->send_buf+net_connection->send_buf_msg_len+3) = \
				((dev_params_info_list+i)->dev_ID) & 0xFF;
		net_connection->send_buf_msg_len += 4;

		*(net_connection->send_buf+net_connection->send_buf_msg_len) = \
				(dev_params_info_list+i)->params_len;
		net_connection->send_buf_msg_len += 1;

		memcpy(net_connection->send_buf+net_connection->send_buf_msg_len, \
			   (dev_params_info_list+i)->params_info_buf, (dev_params_info_list+i)->params_len);
		net_connection->send_buf_msg_len += (dev_params_info_list+i)->params_len;
	}

	net_connection->recv_buf_msg_len = 0;
	send_net_msg(net_connection, TERMINAL_QUERY_PARAM_REPLY);

	return 0;
}


int take_photo_immediately_reply(Net_Connection *net_connection)
{
	net_connection->send_buf_msg_len = 0;
	memcpy(net_connection->send_buf+net_connection->send_buf_msg_len,\
		   net_connection->recv_buf+MSG_SN_INX, 2);
	net_connection->send_buf_msg_len += 2;
	*(net_connection->send_buf+net_connection->send_buf_msg_len) = 0;
	net_connection->send_buf_msg_len += 1;

	*(net_connection->send_buf+net_connection->send_buf_msg_len) = 0;
	*(net_connection->send_buf+net_connection->send_buf_msg_len+1) = 1;
	net_connection->send_buf_msg_len +=2;


	*(net_connection->send_buf+net_connection->send_buf_msg_len) = 0;
	*(net_connection->send_buf+net_connection->send_buf_msg_len+1) = 0;
	*(net_connection->send_buf+net_connection->send_buf_msg_len+2) = 0;
	*(net_connection->send_buf+net_connection->send_buf_msg_len+3) = \
			*(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG);
	net_connection->send_buf_msg_len += 4;
	net_connection->recv_buf_msg_len = 0;
	send_net_msg(net_connection, TERMINAL_TAKE_PHOTO_REPLY);
	return 0;
}


int warning_attached_upload_process(Net_Connection *net_connection)
{
	char server_ip[16] = "";
	unsigned short port;
	unsigned char server_ip_len = *(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG);

	net_connection->send_buf_msg_len = 0;
	memcpy(net_connection->send_buf+net_connection->send_buf_msg_len, \
			net_connection->recv_buf+MSG_SN_INX, 2);
	net_connection->send_buf_msg_len += 2;

	memcpy(net_connection->send_buf+net_connection->send_buf_msg_len, \
			net_connection->recv_buf+MSG_ID_INX, 2);
	net_connection->send_buf_msg_len += 2;

	if((server_ip_len >= 7) && (server_ip_len <= 15))
	{
		archive_server_connection = apply_net_connection(1);

		if(archive_server_connection == (Net_Connection *)(-1))
		{
			goto connect_archive_server_error;
		}

		strncpy(server_ip, (net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG+1), server_ip_len);

		port = (*(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG+1+server_ip_len) << 8) |\
				*(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG+2+server_ip_len);
		archive_server_connection->fd = tcp_unblock_connect_server(server_ip, port, 5);

		if(archive_server_connection->fd <= 0)
		{
			free(archive_server_connection);
			goto connect_archive_server_error;
		}
	}
	else
	{
		goto connect_archive_server_error;
	}

	memcpy(dev_warn_msg->warn_tag, net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG+5+server_ip_len, \
			sizeof(dev_warn_msg->warn_tag));
	memcpy(dev_warn_msg->warn_SN, net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG+21+server_ip_len, \
			sizeof(dev_warn_msg->warn_SN));

	dev_warn_msg->file_upload_seq = CONNECT_SERVER;

	*(net_connection->send_buf+net_connection->send_buf_msg_len) = 0;
	net_connection->send_buf_msg_len += 1;
	archive_server_connection->state = CONNECTED;
	send_net_msg(net_connection, TERMINAL_GENERAL_REPLY);
	return 0;

connect_archive_server_error:
	*(net_connection->send_buf+net_connection->send_buf_msg_len) = 1;
	net_connection->send_buf_msg_len += 1;
	send_net_msg(net_connection, TERMINAL_GENERAL_REPLY);
	return -1;
}


int upload_finish_reply(Net_Connection *net_connection)
{
	static unsigned char retry_cnt = 0;

	unsigned char file_name_len = *(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG);

	//upload result is 00: upload finish
	if(!*(net_connection->recv_buf+MSG_BODY_BEG_INX_NO_SUBPKG+file_name_len+2))
	{
		retry_cnt = 0;
		(dev_warn_msg->attach_file_array+dev_warn_msg->upload_files_count)->file_sent_state = SENT_OK;
		goto upload_finish_reply_handle;
	}
	else //upload result is 01: file is not intact, left part need to be upload
	{
		(dev_warn_msg->attach_file_array+dev_warn_msg->upload_files_count)->file_sent_state = SENT_INCOMPLETE;

		if(retry_cnt++ > 2)  // already try twice
		{
			retry_cnt = 0;
			goto upload_finish_reply_handle;
		}
		else
		{
			dev_warn_msg->file_upload_seq = UPLOAD_FILE_DATA;
			return -1;
		}
	}

upload_finish_reply_handle:
	dev_warn_msg->upload_files_count += 1;

	// not all of files are sent already, left need to be upload continually
	if(dev_warn_msg->upload_files_count < dev_warn_msg->files_count)
	{
		dev_warn_msg->file_upload_seq = CONNECT_SERVER;  //upload next file
	}
	else  //all files are sent already, so close connection
	{
		close(net_connection->fd);
		net_connection->retry_cnt = 0;
		net_connection->state = DISCONNECTED;
		net_connection->recv_buf_msg_len = 0;
		net_connection->send_buf_msg_len = 0;
		dev_warn_msg->file_upload_seq = DISCONNECT_SERVER;
	}

	return 0;
}


int parse_recv_msg(Net_Connection *net_connection)
{
	unsigned short msg_ID_from_BK = 0;

	msg_ID_from_BK = (*(net_connection->recv_buf+MSG_ID_INX)<<8)|\
			*(net_connection->recv_buf+MSG_ID_INX+1);
	printf("BG msg ID %04X\n", msg_ID_from_BK);

	switch(msg_ID_from_BK)
	{
	case BG_GENERAL_REPLY:  //0x8001
		BG_genernal_reply(net_connection);
		break;

	case CMD_QUERY_INFO:  //0x8900
		query_basic_info(net_connection);
		break;

	case BG_REGISTER_REPLY:  //0x8100
		BG_register_reply(net_connection);
		break;

	case CMD_PARAM_SET:  //0x8103
		set_dev_param_reply(net_connection);
		break;

	case CMD_SPECIFY_PARAM_QUERY:  //0x8106
		query_specify_dev_param_reply(net_connection);
		break;

	case CMD_ALL_PARAM_QUERY:  //0x8104
		query_all_dev_param_reply(net_connection);
		break;

	case CMD_TAKE_PHOTO:  //0x8801
		take_photo_immediately_reply(net_connection);
		break;

	case WARNNING_ATTACHED_CALL:  //0x9208
		warning_attached_upload_process(net_connection);
		break;

	case FINISH_UPLOAD_FILE_REPLY:  //0x9212
		upload_finish_reply(net_connection);
		break;

	default:
		net_connection->recv_buf_msg_len = 0;
		break;
	}

	net_connection->recv_buf_msg_len = 0;
	return 0;
}


void parse_msg_from_BG_task(void *connection)
{
	Net_Connection *net_connection = (Net_Connection *)connection;
	int i = 0;

	while(1)
	{
		if(net_connection && !recv_net_msg(net_connection, 3000))
		{
			printf("recv data: ");

			for(i=0; i<net_connection->recv_buf_msg_len; i++)
			{
				printf("%02x ", *(net_connection->recv_buf+i));
			}

			printf("\n");
			parse_recv_msg(net_connection);
			net_connection->recv_buf_msg_len = 0;
		}
		else
		{
			sleep(2);
		}
	}
}






