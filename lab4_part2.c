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
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <pthread.h>


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
		//printf("%s %0.1f F\n",buffer,temperature);
		fprintf(file, "%s %0.1f F\n", buffer, temperature);
		
	}
	else{
	//printf("%s %0.1f C\n",buffer,temperature);
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
			//printf("%s/n",BYTE);
			fprintf(file, "%s\n", BYTE);
			exit(0);
		}
		else if (strncmp(BYTE,"START",5)==0){
			//printf("%s\n",BYTE);
			fprintf(file, "%s\n", BYTE);
			start_flag =1;		
		}
		else if (strncmp(BYTE,"STOP",4)==0){
			//printf("%s\n",BYTE);
			fprintf(file, "%s\n", BYTE);
			start_flag = 0;
		}
		else if (strncmp(BYTE,"SCALE=F",7)==0){	
			//printf("%s\n",BYTE);
			fprintf(file, "%s\n", BYTE);
			flag_f =1;
		}
		else if (strncmp(BYTE,"SCALE=C",7)==0){
			//printf("%s\n",BYTE);
			fprintf(file, "%s\n", BYTE);
			flag_f =0;
		}
		else if (strncmp(BYTE,"FREQ=",5)==0){
			//printf("%s\n",BYTE);
			sscanf(BYTE, "FREQ=%d", &frq);
			fprintf(file, "%s\n", BYTE);
		}
		else{
			//printf("%s I\n",BYTE);
			fprintf(file, "%s I\n",BYTE);
		}
		memset(BYTE, 0, sizeof(BYTE));
	}
}

int main(int argc, char *argv[]){

	//create socket
	int port_num = 16000;

	sock_fd = socket(PF_INET, SOCK_STREAM, 0);
	if(sock_fd < 0){
		error("error");
	}
	// end socket
	// create connection
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	struct hostent* server;

	bzero((char*)&server_addr,sizeof(server_addr));

	server = gethostbyname("lever.cs.ucla.edu");

	server_addr.sin_family = AF_INET;

	bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr,server->h_length);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	};

	server_addr.sin_port = htons(port_num);
	if (connect(sock_fd, (struct sockaddr *) &server_addr,sizeof(server_addr)) < 0) {
		perror("Error connecting"); 
		exit(0);
	}
	// connection done
	// get new port
	massage = "Port request 604675793";
	char resp[1024];
	//int count = send(sock_fd,massage,strlen(massage),0);
	//count = recv(sock_fd,resp,1024,0);
	write(sock_fd,massage,strlen(massage));
	read(sock_fd,&massage_int,8);

	close(sock_fd);

	sock_fd = socket(PF_INET,SOCK_STREAM,0);
	//int newPort = atoi(massage);

	server_addr.sin_port=htons(massage_int);
	printf("port number %d\n", (int)(massage_int));
	// finish new port
	//
	
	if (connect(sock_fd, (struct sockaddr *) &server_addr,sizeof(server_addr)) < 0) {
		perror("Error connecting new port"); 
		exit(0);
	}

	
	pthread_create(&thread_id, NULL, CDM, NULL);

	file = fopen("log-part2.txt", "w");

	while(1){
		float result;
		if (flag_f && start_flag){
		result = read_temp(1,frq);
		}
		else if (!flag_f && start_flag){
		result = read_temp(0,frq);
		}
		else{
		continue;
		}

		char temp[30];
		sprintf(temp,"604675793 TEMP=%.1f\n",result);
		if(start_flag){		
		if(send(sock_fd,&temp,sizeof(temp),0)<0){
		      printf("error sending result");
		}
		//printf("%s",temp);
		}
	}

	return 0;  
}

