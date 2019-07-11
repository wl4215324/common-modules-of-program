/*
 * app_test.c
 *
 *  Created on: Jun 12, 2019
 *      Author: tony
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "net_connection/net_connection.h"
#include "net_connection/tcp_client.h"
#include "protocol/JSATL.h"


#define  FILE_NAME_LEN  64

int report_warn_attach_files(Dev_Warn_Msg *dev_warn_msg, unsigned char files_cnt, \
		char *files_name[], char file_type[])
{
	unsigned int i = 0, j = 0;
	FILE *fp;
	unsigned char temp_cnt = 0;

	if(!dev_warn_msg || !files_cnt || !files_name)
	{
		return -1;
	}

	dev_warn_msg->files_count = files_cnt;
	dev_warn_msg->upload_files_count = 0;
	dev_warn_msg->attach_file_array = (Attach_File *)malloc(files_cnt*sizeof(Attach_File));

	if(!dev_warn_msg->attach_file_array)
	{
		dev_warn_msg->files_count = 0;
		return -1;
	}

	printf("upload file count: %d\n", dev_warn_msg->files_count);

	for(i=0; i<dev_warn_msg->files_count; i++)
	{
		if(access(files_name[i], 0) < 0)  //file not exist
		{
			(dev_warn_msg->attach_file_array+i)->file_data_len = 0;
			(dev_warn_msg->attach_file_array+i)->file_sent_state = ORIG_STATE;  //not exist
			(dev_warn_msg->attach_file_array+i)->file_type = 4;
			(dev_warn_msg->attach_file_array+i)->file_name = NULL;
			(dev_warn_msg->attach_file_array+i)->local_file_name = NULL;
		}
		else
		{
			fp = fopen(files_name[i], "r");

			if(!fp)
			{
				(dev_warn_msg->attach_file_array+i)->file_data_len = 0;
				(dev_warn_msg->attach_file_array+i)->file_sent_state = ORIG_STATE;  //not opened
				(dev_warn_msg->attach_file_array+i)->file_type = 4;
				(dev_warn_msg->attach_file_array+i)->file_name = NULL;
				(dev_warn_msg->attach_file_array+i)->local_file_name = NULL;
			}
			else
			{
				(dev_warn_msg->attach_file_array+i)->local_file_name = malloc(FILE_NAME_LEN);
				(dev_warn_msg->attach_file_array+i)->file_name = malloc(FILE_NAME_LEN);

				if(!(dev_warn_msg->attach_file_array+i)->local_file_name || \
				   !(dev_warn_msg->attach_file_array+i)->file_name )
				{
					(dev_warn_msg->attach_file_array+i)->file_data_len = 0;
					(dev_warn_msg->attach_file_array+i)->file_sent_state = ORIG_STATE;  //file name error
					(dev_warn_msg->attach_file_array+i)->file_type = 4;
				}

			    fseek(fp, 0L, SEEK_END);
			    (dev_warn_msg->attach_file_array+i)->file_data_len = ftell(fp);
				(dev_warn_msg->attach_file_array+i)->file_sent_state = INIT_STATE;
				(dev_warn_msg->attach_file_array+i)->file_type = file_type[i];

				strcpy((dev_warn_msg->attach_file_array+i)->local_file_name, \
						files_name[i]);

				sprintf((dev_warn_msg->attach_file_array+i)->file_name, "%02d_", \
						(dev_warn_msg->attach_file_array+i)->file_type);

				temp_cnt = 0;
				temp_cnt = strlen((dev_warn_msg->attach_file_array+i)->file_name);

				sprintf((dev_warn_msg->attach_file_array+i)->file_name+temp_cnt, "%02X_", \
						dev_warn_msg->dev_ID);
				temp_cnt = strlen((dev_warn_msg->attach_file_array+i)->file_name);

				sprintf((dev_warn_msg->attach_file_array+i)->file_name+temp_cnt, "%02X", \
						dev_warn_msg->dev_ID);
				temp_cnt = strlen((dev_warn_msg->attach_file_array+i)->file_name);

				if(dev_warn_msg->dev_ID == ADAS_ID)
				{
					j = 0;
				}
				else if(dev_warn_msg->dev_ID == DSM_ID)
				{
					j = 1;
				}
				else if(dev_warn_msg->dev_ID == BSD_ID)
				{
					j = 3;
				}

				sprintf((dev_warn_msg->attach_file_array+i)->file_name+temp_cnt, "%02X_", \
						*((dev_warn_info_list+j)->add_info_buf+5)+1);
				temp_cnt = strlen((dev_warn_msg->attach_file_array+i)->file_name);

				sprintf((dev_warn_msg->attach_file_array+i)->file_name+temp_cnt, "%d_", i);
				temp_cnt = strlen((dev_warn_msg->attach_file_array+i)->file_name);

				memcpy((dev_warn_msg->attach_file_array+i)->file_name+temp_cnt, dev_warn_msg->warn_SN, \
						sizeof(dev_warn_msg->warn_SN));
				temp_cnt += sizeof(dev_warn_msg->warn_SN);

				if((dev_warn_msg->attach_file_array+i)->file_type == 0)
				{
					strcpy((dev_warn_msg->attach_file_array+i)->file_name+temp_cnt, ".jpg");
				}
				else if((dev_warn_msg->attach_file_array+i)->file_type == 1)
				{
					strcpy((dev_warn_msg->attach_file_array+i)->file_name+temp_cnt, ".wav");
				}
				else if((dev_warn_msg->attach_file_array+i)->file_type == 2)
				{
					strcpy((dev_warn_msg->attach_file_array+i)->file_name+temp_cnt, ".h264");
				}
				else if((dev_warn_msg->attach_file_array+i)->file_type == 3)
				{
					strcpy((dev_warn_msg->attach_file_array+i)->file_name+temp_cnt, ".bin");
				}
				else if((dev_warn_msg->attach_file_array+i)->file_type == 4)
				{
					strcpy((dev_warn_msg->attach_file_array+i)->file_name+temp_cnt, ".mp4");
				}

				printf("upload file name: %s\n",
						(dev_warn_msg->attach_file_array+i)->file_name);
			}

			fclose(fp);
		}
	}

	return 0;
}


int send_warn_file(Dev_Warn_Msg *dev_warn_msg, Net_Connection *net_connection)
{
	unsigned char temp_cnt = 0;
	unsigned char sections = 0, i = 0, j = 0;
	unsigned short last_sect_len = 0;

	Attach_File *temp_attach_file = \
			temp_attach_file = dev_warn_msg->attach_file_array + dev_warn_msg->upload_files_count;

	if(dev_warn_msg->file_upload_seq == CONNECT_SERVER)
	{
		net_connection->send_buf_msg_len = 0;
		memcpy(net_connection->send_buf+net_connection->send_buf_msg_len,
				dev_warn_msg->terminal_ID, 7);  //terminal ID

		net_connection->send_buf_msg_len += 7;
		memcpy(net_connection->send_buf+net_connection->send_buf_msg_len,
				dev_warn_msg->warn_tag, 16);  //warn tag

		net_connection->send_buf_msg_len += 16;
		memcpy(net_connection->send_buf+net_connection->send_buf_msg_len,
				dev_warn_msg->warn_SN, 32);  //warn SN

		net_connection->send_buf_msg_len += 32;
		*(net_connection->send_buf+net_connection->send_buf_msg_len) = 0;  //warn type
		net_connection->send_buf_msg_len += 1;

		*(net_connection->send_buf+net_connection->send_buf_msg_len) = \
				dev_warn_msg->files_count;  // file count
		net_connection->send_buf_msg_len += 1;

		if(dev_warn_msg->files_count > 0)
		{
			*(net_connection->send_buf+net_connection->send_buf_msg_len) = \
					strlen(temp_attach_file->file_name);
			net_connection->send_buf_msg_len += 1;
			memcpy(net_connection->send_buf+net_connection->send_buf_msg_len, temp_attach_file->file_name, \
					strlen(temp_attach_file->file_name));
			net_connection->send_buf_msg_len += strlen(temp_attach_file->file_name);

			*(net_connection->send_buf+net_connection->send_buf_msg_len) = \
					temp_attach_file->file_data_len >> 24;
			*(net_connection->send_buf+net_connection->send_buf_msg_len+1) = \
					temp_attach_file->file_data_len >> 16;
			*(net_connection->send_buf+net_connection->send_buf_msg_len+2) = \
					temp_attach_file->file_data_len >> 8;
			*(net_connection->send_buf+net_connection->send_buf_msg_len+3) = \
					temp_attach_file->file_data_len & 0xFF;
			net_connection->send_buf_msg_len += 4;
		}

		send_net_msg(net_connection, TERMINAL_SEND_WARN_ATTACH_INFO);  //0x1210
		dev_warn_msg->file_upload_seq = UPLOAD_ATTACH_INFO;
	}
	else if(dev_warn_msg->file_upload_seq == UPLOAD_FILE_INFO)
	{
		sleep(1);
		net_connection->send_buf_msg_len = 0;

		*(net_connection->send_buf+net_connection->send_buf_msg_len) = \
				strlen(temp_attach_file->file_name);
		net_connection->send_buf_msg_len += 1;

		memcpy(net_connection->send_buf+net_connection->send_buf_msg_len, \
				temp_attach_file->file_name, strlen(temp_attach_file->file_name));
		net_connection->send_buf_msg_len += strlen(temp_attach_file->file_name);

		*(net_connection->send_buf+net_connection->send_buf_msg_len) = \
				temp_attach_file->file_type;
		net_connection->send_buf_msg_len += 1;

		*(net_connection->send_buf+net_connection->send_buf_msg_len) = \
				temp_attach_file->file_data_len  >> 24;
		*(net_connection->send_buf+net_connection->send_buf_msg_len+1) = \
				temp_attach_file->file_data_len  >> 16;
		*(net_connection->send_buf+net_connection->send_buf_msg_len+2) = \
				temp_attach_file->file_data_len  >> 8;
		*(net_connection->send_buf+net_connection->send_buf_msg_len+3) = \
				temp_attach_file->file_data_len & 0xFF;
		net_connection->send_buf_msg_len += 4;

		send_net_msg(net_connection, TERMINAL_SEND_WARN_ATTACH_DATA);
		dev_warn_msg->file_upload_seq = WAIT_SERVER_REPLY;
	}
	else if(dev_warn_msg->file_upload_seq == UPLOAD_FILE_DATA)
	{
		FILE *fp = NULL;
		fp = fopen(temp_attach_file->local_file_name, "r");

		sections = temp_attach_file->file_data_len / 65536;
		last_sect_len = temp_attach_file->file_data_len % 65536;

		for(i=0; i<sections; i++)
		{
			net_connection->send_buf_msg_len = 0;
			*(net_connection->send_buf+net_connection->send_buf_msg_len) = 0x30;
			*(net_connection->send_buf+net_connection->send_buf_msg_len+1) = 0x31;
			*(net_connection->send_buf+net_connection->send_buf_msg_len+2) = 0x63;
			*(net_connection->send_buf+net_connection->send_buf_msg_len+3) = 0x64;
			net_connection->send_buf_msg_len += 4;

			memcpy(net_connection->send_buf+net_connection->send_buf_msg_len, \
					temp_attach_file->file_name, 50);
			net_connection->send_buf_msg_len += 50;

			*(net_connection->send_buf+net_connection->send_buf_msg_len) = \
					i*65536 >> 24;
			*(net_connection->send_buf+net_connection->send_buf_msg_len+1) = \
					i*65536 >> 16;
			*(net_connection->send_buf+net_connection->send_buf_msg_len+2) = \
					i*65536 >> 8;
			*(net_connection->send_buf+net_connection->send_buf_msg_len+3) = \
					i*65536 & 0xFF;
			net_connection->send_buf_msg_len += 4;

			*(net_connection->send_buf+net_connection->send_buf_msg_len) = \
					65536 >> 24;
			*(net_connection->send_buf+net_connection->send_buf_msg_len+1) = \
					65536 >> 16;
			*(net_connection->send_buf+net_connection->send_buf_msg_len+2) = \
					65536 >> 8;
			*(net_connection->send_buf+net_connection->send_buf_msg_len+3) = \
					65536 & 0xFF;
			net_connection->send_buf_msg_len += 4;

			for(j=0; j<65536/1400; j++)
			{
				fread(net_connection->send_buf+net_connection->send_buf_msg_len, 1400, 1, fp);
				net_connection->send_buf_msg_len += 1400;
				send_spec_len_data(net_connection->fd, net_connection->send_buf, net_connection->send_buf_msg_len);
				net_connection->send_buf_msg_len = 0;
			}

			if(65536%1400)
			{
				fread(net_connection->send_buf+net_connection->send_buf_msg_len, 65536%1400, 1, fp);
				net_connection->send_buf_msg_len += 65536%1400;
				send_spec_len_data(net_connection->fd, net_connection->send_buf, net_connection->send_buf_msg_len);
				net_connection->send_buf_msg_len = 0;
			}
		}

		if(last_sect_len > 0)
		{
			net_connection->send_buf_msg_len = 0;
			*(net_connection->send_buf+net_connection->send_buf_msg_len) = 0x30;
			*(net_connection->send_buf+net_connection->send_buf_msg_len+1) = 0x31;
			*(net_connection->send_buf+net_connection->send_buf_msg_len+2) = 0x63;
			*(net_connection->send_buf+net_connection->send_buf_msg_len+3) = 0x64;
			net_connection->send_buf_msg_len += 4;

			memcpy(net_connection->send_buf+net_connection->send_buf_msg_len, \
					temp_attach_file->file_name, 50);
			net_connection->send_buf_msg_len += 50;

			*(net_connection->send_buf+net_connection->send_buf_msg_len) = \
					i*65536 >> 24;
			*(net_connection->send_buf+net_connection->send_buf_msg_len+1) = \
					i*65536 >> 16;
			*(net_connection->send_buf+net_connection->send_buf_msg_len+2) = \
					i*65536 >> 8;
			*(net_connection->send_buf+net_connection->send_buf_msg_len+3) = \
					i*65536 & 0xFF;
			net_connection->send_buf_msg_len += 4;

			*(net_connection->send_buf+net_connection->send_buf_msg_len) = \
					last_sect_len >> 24;
			*(net_connection->send_buf+net_connection->send_buf_msg_len+1) = \
					last_sect_len >> 16;
			*(net_connection->send_buf+net_connection->send_buf_msg_len+2) = \
					last_sect_len >> 8;
			*(net_connection->send_buf+net_connection->send_buf_msg_len+3) = \
					last_sect_len & 0xFF;
			net_connection->send_buf_msg_len += 4;

			for(j=0; j<last_sect_len/1400; j++)
			{
				fread(net_connection->send_buf+net_connection->send_buf_msg_len, 1400, 1, fp);
				net_connection->send_buf_msg_len += 1400;
				send_spec_len_data(net_connection->fd, net_connection->send_buf, net_connection->send_buf_msg_len);
				net_connection->send_buf_msg_len = 0;
			}

			if(last_sect_len%1400)
			{
				fread(net_connection->send_buf+net_connection->send_buf_msg_len, last_sect_len%1400, 1, fp);
				net_connection->send_buf_msg_len += last_sect_len%1400;
				send_spec_len_data(net_connection->fd, net_connection->send_buf, net_connection->send_buf_msg_len);
				net_connection->send_buf_msg_len = 0;
			}
		}

		fclose(fp);

		dev_warn_msg->file_upload_seq = UPLOAD_FINISH_INFO;
	}
	else if(dev_warn_msg->file_upload_seq == UPLOAD_FINISH_INFO)
	{
		sleep(2);
		net_connection->send_buf_msg_len = 0;
		*(net_connection->send_buf+net_connection->send_buf_msg_len) = \
				strlen(temp_attach_file->file_name);
		net_connection->send_buf_msg_len += 1;

		memcpy(net_connection->send_buf+net_connection->send_buf_msg_len, \
				temp_attach_file->file_name, strlen(temp_attach_file->file_name));
		net_connection->send_buf_msg_len += strlen(temp_attach_file->file_name);

		*(net_connection->send_buf+net_connection->send_buf_msg_len) = \
				temp_attach_file->file_type;
		net_connection->send_buf_msg_len += 1;

		*(net_connection->send_buf+net_connection->send_buf_msg_len) = \
				temp_attach_file->file_data_len  >> 24;
		*(net_connection->send_buf+net_connection->send_buf_msg_len+1) = \
				temp_attach_file->file_data_len  >> 16;
		*(net_connection->send_buf+net_connection->send_buf_msg_len+2) = \
				temp_attach_file->file_data_len  >> 8;
		*(net_connection->send_buf+net_connection->send_buf_msg_len+3) = \
				temp_attach_file->file_data_len & 0xFF;
		net_connection->send_buf_msg_len += 4;

		send_net_msg(net_connection, TERMINAL_FINISH_UPLOAD_FILE);
		temp_attach_file->file_sent_state = 4;
		dev_warn_msg->file_upload_seq = WAIT_SERVER_REPLY;
	}
	else if(dev_warn_msg->file_upload_seq == DISCONNECT_SERVER)
	{
		for(i=0; i<dev_warn_msg->files_count; i++)
		{
			if((dev_warn_msg->attach_file_array+i)->file_name)
				free((dev_warn_msg->attach_file_array+i)->file_name);

			if((dev_warn_msg->attach_file_array+i)->local_file_name)
				free((dev_warn_msg->attach_file_array+i)->local_file_name);
		}

		if(dev_warn_msg->attach_file_array)
			free(dev_warn_msg->attach_file_array);

		dev_warn_msg->files_count = 0;
		dev_warn_msg->upload_files_count = 0;
		dev_warn_msg->file_upload_seq = 0;
	}

	return 0;
}


int main(int argc, char *argv[])
{
	int i = 0;
	Net_Connection *net_connection = apply_net_connection(1);

	if(net_connection == (Net_Connection *)(-1))
	{
		return -1;
	}

	printf("new net_connection successfully!\n");

	if(init_protocol_JSATL(&dev_bsc_info_list) < 0)
	{
		perror("init_protocol_JSATL error: ");
		return -1;
	}

	if(init_protocol_params(&dev_params_info_list) < 0)
	{
		perror("init_protocol_JSATL params error: ");
		return -1;
	}

	if(init_warn_info(&dev_warn_info_list) < 0)
	{
		perror("init_warn_info error: ");
		return -1;
	}

	if(init_warn_info_msg(&dev_warn_msg) < 0)
	{
		perror("init_warn_info_msg error: ");
		return -1;
	}

	net_connection->fd = tcp_unblock_connect_server("218.5.10.82", 38922, 5);
	//net_connection->fd = tcp_unblock_connect_server("106.15.80.217", 58000, 5);
	printf("net_connection->fd : %d\n", net_connection->fd);

	pthread_t pthread_send, pthread_recv;
	pthread_create(&pthread_send, NULL, (void*)authen_and_warn_process_task, (void*) net_connection);
	pthread_create(&pthread_recv, NULL, (void*)parse_msg_from_BG_task, (void*) net_connection);

	//init_warn_info_msg
	//parse_protocol_task((void*) parse_protocol_task);


	dev_warn_msg->dev_ID = 0x64;
	char *files_name[4] = {"upload_test.jpg", "upload_test_2.jpg","upload_test_3.jpg", "upload_test_4.jpg"};
	char files_type[4] = {0, 0, 0, 0};

	sleep(7);
	report_warn_attach_files(dev_warn_msg, 4, files_name, files_type);
	while(1)
	{
		send_warn_file(dev_warn_msg, archive_server_connection);
		//sleep(2);
		//pause();
	}


	return 0;
}
