/*
 * test_app.c
 *
 *  Created on: Feb 20, 2019
 *      Author: tony
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "producer_consumer_shmfifo.h"

int main(int argc, char **argv)
{
	BYTE buffer[100];
	unsigned int i, data_len = 0;

	shmfifo *p_shmfifo = shmfifo_init(10000, 2000);

	printf("argument is: %s\n", *(argv+1));

	if(strcmp(*(argv+1), "C") == 0)  //consumer
	{
		while(1)
		{
			data_len = shmfifo_get(p_shmfifo, buffer, 30);

			printf("\n get data:");

			for(i=0; i<data_len; i++)
			{
				printf("  %d", *(buffer+i));
			}

			puts("");

			usleep(500000);
		}
	}
	else if(strcmp(*(argv+1), "P") == 0)  //producer
	{
		for(i=0; i<50; i++)
		{
			buffer[i] = i+1;
		}

		while(1)
		{
			data_len = shmfifo_put(p_shmfifo, buffer, 50);
			usleep(1000000);
		}
	}


	sleep(20);
	return 0;
}
