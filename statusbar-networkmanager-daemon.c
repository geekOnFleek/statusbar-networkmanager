#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <pthread.h>

#define FUNCTION_SUCCESS 1
#define SUCCESS 0
#define ERROR 10
#define ERROR_ON_MEMORY_ALLOCATION 11
#define ERROR_ON_FILE_OPEN 12
#define NULLPOINTER_EXCEPTION 13
#define EXCEPTION_NOT_IMPLEMENTED 15
#define ERROR_ON_NETWORK_READ 20
#define ERROR_ON_SIGNAL_READ 30

/*
	TODO:
		- reacts on mouse-events from bar: 
			* scrolling makes network-name change
			* left-click establishes connection
		- formats output with suiting colors for signal strength
	
*/

struct mesg_buffer {
	long mesg_type;
	char mesg_text[100];
}message;

int active = 1;
int unit = 1;
int connected = 0; //equals 1 if wpa_connection is established
char **networks;
char *active_network = NULL;
int active_network_index = 0;
char *output = NULL;
key_t in_key, out_key = 0;
int in_msgid, out_msgid = 0;


int read_networks()
{
	FILE *config = fopen("/home/jan/.config/i3bar-networkmanagement", "r");
	if(config == NULL) {
		printf("File couldn't be opened.");
		return ERROR_ON_FILE_OPEN;
	}
	char templine[5];
	//scans for 20 lines at max
	for(int i = 0; i<20; i++){
		fscanf(config, "%s", &templine[0]);
		templine[4] = 0b0;
		strncpy(&networks[i][0], &templine[0], 4);
		if(strncmp(&templine[0], "XXX", 3) == 0)
			i = 21; //breaks from loop but still writes the terminating "XXX"-string into array
	}
	fclose(config);
	return FUNCTION_SUCCESS;
}

int read_signal()
{
	if(!connected){
		output[4] = '-';
		output[5] = '0';
		output[6] = 'd';
		output[7] = 'B';
		return FUNCTION_SUCCESS;
	}
	//Asking bash for signal
	FILE *subprocess = NULL;
	if(unit)
		subprocess = popen("grep wlp5s0 /proc/net/wireless | awk '{print int($3 * 100 / 70)}'", "r");
	else
		subprocess = popen("grep wlp5s0 /proc/net/wireless | awk '{print int($3)}'", "r");
	if(subprocess == NULL)
		return ERROR_ON_SIGNAL_READ;
	output[4] = (char) fgetc(subprocess);
	output[5] = (char) fgetc(subprocess);	
	pclose(subprocess);
	//Checking for unit
	if(unit) {
		output[6] = '%';
		output[7] = 0b0;
	} else {
		output[6] = 'd';
		output[7] = 'B';
	}
	return FUNCTION_SUCCESS;
}

int write_active_network()
{
	if(active_network == NULL)
		return NULLPOINTER_EXCEPTION;
	for(int i=0; i<3; i++){
		output[i] = active_network[i];
	} 
	return FUNCTION_SUCCESS;
}

void next_network()
{
	if(connected)
		return;
	char *temp = networks[active_network_index + 1];
	if(temp == NULL || (strncmp(temp, "XXX", 4) == 0)){
		printf("Couldn't find next network.\n");
		return;
	} else {
		active_network_index++;
		active_network = temp;
	}
	printf("Active network is: %s\n", temp);
}

void previous_network()
{
	if(connected)
		return;
	char *temp = networks[active_network_index - 1];
	if(temp == NULL || temp < networks[0] || active_network_index < 0) {
		printf("Couldn't find previous network.");
		return;
	} else {
		active_network_index--;
		active_network = temp;
	}
	printf("Active network is: %s\n", active_network);
}

//inits the outputbuffer with "SGN INIT"
void init_output()
{
	output[0] = 'S'; //Byte for network symbol
	output[1] = 'G'; //Byte for network symbol
	output[2] = 'N'; //Byte for network symbol
	output[3] = ' '; //spacebyte
	output[4] = 'I'; //Byte for signal-strength
	output[5] = 'N'; //Byte for signal-strength
	output[6] = 'I'; //Byte for unit
	output[7] = 'T'; //Byte for unit in case "dB" or 0b0 for terminating after '%'
	output[8] = 0b0; //Byte for terinating in every case
}

int send_output()
{
	read_signal();
	write_active_network();
	printf("Sending Message\n");
	strncpy(message.mesg_text, "CLI", 3);
	printf("Added Header %s.\n", message.mesg_text);
	strncpy((message.mesg_text + sizeof(char)*3), output, 9);
	printf("Added Body %s.\n", message.mesg_text);
	message.mesg_type = 5;
	printf("Out-channel is: %d.\n", out_msgid);
	msgctl(in_msgid, IPC_RMID, NULL);
	msgsnd(out_msgid, &message, sizeof(message), 0);
	printf("Sent Message: %s\n", message.mesg_text);
	return 1;
}

int connect()
{
	FILE *sub = NULL;
	if(strncmp(active_network, "RGB", 3) == 0){
		sub = popen("wpa_supplicant -B -iwlp5s0 -c/root/rgb.conf", "r");
		char temp[30] = "TEMPSTRING";
		sleep(2);
		fscanf(sub, "%s", &temp[0]);
		printf("%s\n", temp);
		if(strncmp(&temp[0], "Successfully", 12) != 0){
			pclose(sub);
			sub = popen("i3-nagbar -t warning -m \"Couldn't initialize wpa_supplicant.\"", "r");
			sleep(5);
			pclose(sub);
			return -1;
		}
		pclose(sub);
		sub = popen("dhclient", "r");
		sleep(10);
		pclose(sub);
		connected = 1;
	}
	return 1;
}

int parse_command(char *text)
{
	printf("%s\n", text);
	if(strncmp(text, "GETF", 4) == 0)
		return send_output();
	if(strncmp(text, "NEXT", 4) == 0)
		next_network();
	if(strncmp(text, "PREV", 4) == 0)
		previous_network();
	if(strncmp(text, "CONN", 4) == 0)
		return connect();
	if(strncmp(text, "UNIT", 4) == 0)
		unit = (unit + 1) % 2;
	return 1;
}

int communication(){
	in_key = ftok("/home/jan/.uscripts/wifiservice/network-handler-daemon.c", 67);
	in_msgid = msgget(in_key, 0666 | IPC_CREAT);
	out_key = ftok("/home/jan/.uscripts/wifiservice/connection-info.c", 68);
	out_msgid = msgget(out_key, 0666 | IPC_CREAT);
	printf("%d, %d\n", in_msgid, out_msgid);
	active = 1;	
	while(active) {
		in_msgid = msgget(in_key, 0666 | IPC_CREAT);
		msgrcv(in_msgid, &message, sizeof(message), 2, 0);
		printf("Message received: %s \t|%d \n", message.mesg_text, message.mesg_type);
		if(strncmp(message.mesg_text, "CLI", 3) == 0)
			strncpy(message.mesg_text, "---", 3);
		if(strncmp(message.mesg_text, "DAE", 3) == 0)
			parse_command(&(message.mesg_text[3]));
		if(strncmp(message.mesg_text, "EXT", 3) == 0)
			active = 0;
	}
	msgctl(in_msgid, IPC_RMID, NULL);
	msgctl(out_msgid, IPC_RMID, NULL);
	return FUNCTION_SUCCESS;
}

int refresh() {
	while(active) {
		read_signal();
		write_active_network();
		sleep(1);
	}
	return FUNCTION_SUCCESS;
}

int main(void)
{
	//allocate memory for at max 20 network symbols with 3 letters each
	printf("Allocating memory for network-list.\n");
	networks = malloc(sizeof(char *) * 20);
	for(int i = 0; i < 20; i++){
		networks[i] = malloc(sizeof(char) * 4);
	}
	if(networks == NULL) {
		free(networks);
		return ERROR_ON_MEMORY_ALLOCATION;
	}
	printf("Allocated.\n\n");
	
	active_network = networks[active_network_index];

	//reading networks from file
	printf("Reading network-names from config.\n");
	int success = read_networks(networks);
	if(success != FUNCTION_SUCCESS){
		free(networks);
		printf("Reading failed - Error Code: %d\n", success);
		return success;
	}
	printf("Following Symbols could be parsed:\n");
	for(int i = 0; i<20; i++){
		printf("\t%s\n", networks[i]);
		if(networks[i][0] == 0b0)
			break;
	}
	printf("Active Network is: %s\n\n", active_network);
	//allocate memory for outputs which is consumed by print-thread
	//9 bytes with usage: "NNN-LLUU0b0" - See init_output()
	output = malloc(sizeof(char) * 9);
	if(output == NULL) {
		free(networks);
		free(output);
		return ERROR_ON_MEMORY_ALLOCATION;
	}
	//init ouput-array
	init_output(output);
	printf("%s\n", output);
	read_signal();
	printf("%s\n", output);
	write_active_network();
	printf("%s\n", output);	
	
	pthread_t comm_thread;
	pthread_create(&comm_thread, NULL, (void*) communication, NULL);		

	pthread_join(comm_thread, NULL);
	
	printf("Ending programm.");

	free(output);
	for(int i=0; i<20; i++) {
		free(networks[i]);
	}
	free(networks);
	return SUCCESS;
}
