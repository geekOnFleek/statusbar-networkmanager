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
	key = ftok("/home/jan/.uscripts/wifiservice/network-handler-daemon.c", 67);
	
	msgid = msgget(key, 0666 | IPC_CREAT);
	message.mesg_type = 2;

	strncpy(message.mesg_text, "DAECONN", 7);

	msgsnd(msgid, &message, sizeof(message), 0);
	//printf("Data sent is: %s \n", message.mesg_text);
	return 0;
}
