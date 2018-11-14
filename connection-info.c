#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

struct mesg_buffer {
	long mesg_type;
	char mesg_text[100];
}message;

int main()
{
	key_t key;
	int msgid;
	//ftok to generate unique key
	key_t in_key = ftok("/home/jan/.uscripts/wifiservice/network-handler-daemon.c", 67);
	key_t out_key = ftok("/home/jan/.uscripts/wifiservice/connection-info.c", 68);
	if(in_key == out_key)
		printf("Same KEY.\n");
	int out_msgid = msgget(in_key, 0666 | IPC_CREAT);
	int in_msgid = msgget(out_key, 0666 | IPC_CREAT);
	if(in_msgid == out_msgid)
		printf("Same ID.\n");
	message.mesg_type = 2;
	strncpy(message.mesg_text, "DAEGETF", 7);
	msgsnd(out_msgid, &message, sizeof(message), 0);
	//printf("Sent data in %d.\n", out_msgid);
	//msgsnd(msgid, &message, sizeof(message), 0);
	//printf("Waiting for data in %d. \n", in_msgid);
	msgrcv(in_msgid, &message, sizeof(message), 5, 0);
	printf("");
	//printf("Data received is: %s \n", message.mesg_text);
	//msgctl(in_msgid, IPC_RMID, NULL);
	//msgctl(out_MSGID, IPC_RMID, NULL);
	if(strncmp(message.mesg_text, "CLI", 3) == 0)
		printf("%s\n", &message.mesg_text[3]);
	return 0;
}
