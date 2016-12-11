#define h_addr h_addr_list[0]
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include "mraa/aio.h"
#include <stdlib.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <strings.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>

float adc_value = 0.0;
const int B=4255;
int flag_f = 1;
int start_flag = 1;
int frq = 3;
int id = 605675793;

int sock_fd;

time_t t;
char buffer[26];
struct tm* tm_info;
pthread_t thread_id;

char *massage;
int massage_int;

FILE *file;

float read_temp(int flag,int freq){
	mraa_aio_context adc_a0;
	adc_a0 = mraa_aio_init(0);
	time(&t);
	tm_info = localtime(&t);
	strftime(buffer,26,"%H:%M:%S", tm_info);
	adc_value = mraa_aio_read(adc_a0);
	float R = 1023.0/(adc_value)-1.0;
	R = 100000.0*R;
	float temperature = 1.0/(log(R/100000.0)/B+1/298.15)-273.15;
	if(flag){
		temperature = temperature * (18/10)+32;
		printf("%s %0.1f\n",buffer,temperature);
		
		fprintf(file,"%s %0.1f\n",buffer, temperature);		
		
	}
	else{
	printf("%s %0.1f C\n",buffer,temperature);
	fprintf(file, "%s %0.1f C\n", buffer, temperature);
	}
	sleep(freq);
	return temperature;
}

void error( char *msg){
	perror(msg);
}

void *CDM(void* i){	
	char BYTE[1024];
//	printf("above yolo while 2\n");
	while(recv(sock_fd, &BYTE, 1024,0)>=0) {
		//printf("yolo while 2\n");
		//printf("%s\n",BYTE);
		
		if(strncmp(BYTE,"OFF",3)==0){
			printf("%s/n",BYTE);
			fprintf(file, "%s\n", BYTE);
			exit(0);
		}
		else if (strncmp(BYTE,"START",5)==0){
			printf("%s\n",BYTE);
			fprintf(file, "%s\n", BYTE);
			start_flag =1;		
		}
		else if (strncmp(BYTE,"STOP",4)==0){
			printf("%s\n",BYTE);
			fprintf(file, "%s\n", BYTE);
			start_flag = 0;
		}
		else if (strncmp(BYTE,"SCALE=F",7)==0){	
			printf("%s\n",BYTE);
			fprintf(file, "%s\n", BYTE);
			flag_f =1;
		}
		else if (strncmp(BYTE,"SCALE=C",7)==0){
			printf("%s\n",BYTE);
			fprintf(file, "%s\n", BYTE);
			flag_f =0;
		}
		else if (strncmp(BYTE,"FREQ=",5)==0){
			printf("%s\n",BYTE);
			sscanf(BYTE, "FREQ=%d", &frq);
			fprintf(file, "%s\n", BYTE);
		}
		else{
			printf("%s I\n",BYTE);
			fprintf(file, "%s I\n",BYTE);
		}
		memset(BYTE, 0, sizeof(BYTE));
	}
}


void signal_hand(int sig){
if (sig == SIGINT)
	    printf("received SIGINT\n");
	close(file);
	exit(0);
}

int main(int argc, char *argv[]){
	signal(SIGINT,signal_hand);
	file = fopen("log-part1.txt", "w");

	while(1){		
		read_temp(1,1);
	}
		
	close(file);
	return 0;  
}

