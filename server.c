#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#define PORT 8080
/************************************FUNCTION DECLARATIONS*********************************/
void Update_queue(char* output, int client_tid);
void Update_Status(char* site, char* tmp_status, int client_tid);
void *ping_site(void *a);
struct node* Read_queue();
void print_status(void);
char ping_command(char* input, int client_tid);
void *connection_handler(void *);
/****************************************************************************************/

/************************************GLOBAL DECLARATIONS*********************************/
//Node Structure Definition to define each node in the queue.
typedef struct node
{
    char site[50];
    int tid;
    int site_number;
    TAILQ_ENTRY(node) nodes;
} node;

//Definition of the Head of Queue Structure.
TAILQ_HEAD(head_s, node) head;

char Status[100][200];

char handles[100][200];

int Site_count = 0;

pthread_mutex_t lock[5];

pthread_mutex_t queue_lock;

pthread_mutex_t client_id_lock;

int lock_count = 0;

int thread_count = 0;

pthread_t worker_threads[5];
 
node * current_node = NULL;

int client_tid = 0;

/****************************************************************************************/

void Update_queue(char* output, int client_tid)
{
	
	char* tmp_status = "IN_QUEUE";
	struct node * node1 = malloc(sizeof(struct node));
	if (node1 == NULL){
		fprintf(stderr, "malloc failed");
		exit(EXIT_FAILURE);
	}
		
	strcpy(node1->site, output);
	node1->tid = client_tid;
	node1->site_number = Site_count;
	TAILQ_INSERT_TAIL(&head, node1, nodes);
	node1 = NULL;
	Update_Status(output, tmp_status, client_tid);
	Site_count++;
}

void Update_Status(char* site, char* tmp_status, int client_tid)
{
	if(strcmp(tmp_status, "IN_QUEUE")==0)
	{
		//static int i = 0;
		char* temp = (char*)malloc(50);
		sprintf(temp,"%d ",client_tid);
		strcat(temp, site);
		strcat(temp, "\t0 0 0 ");
		strcat(temp, tmp_status);
		strcpy(Status[Site_count], temp);
	}
	else if(strcmp(tmp_status, "IN_PROGRESS")==0)
	{
		int i;
		for(i = 0; i <= Site_count; i++)
		{
			char* tmp1 = (char*)malloc(200);
			strcpy(tmp1, Status[i]);
			char* tmp = (char*)malloc(50);
			tmp = strtok(tmp1, " ");
			tmp = strtok(NULL, "\t");
			if(strcmp(tmp,site)==0)
			{
				char dst[200];
				sprintf(dst,"%d ", client_tid);
				strcat(dst, site);
				strcat(dst, "\t0 0 0 ");
				strcat(dst, tmp_status);
				strcpy(Status[i], dst);
				break;
			}

		}
	}
	else
	{
		//puts(tmp_status);
		int i;
		for(i = 0; i <= Site_count; i++)
		{
			char* tmp1 = (char*)malloc(200);
			strcpy(tmp1, Status[i]);
			char* tmp = (char*)malloc(50);
			tmp = strtok(tmp1, " ");
			tmp = strtok(NULL, "\t");
			if(strcmp(tmp,site)==0)
			{
				strcpy(Status[i], tmp_status);
				break;
			}

		}
	}
}
struct node* Read_queue() 
{	
	//struct node * last_node = malloc(sizeof(struct node));
	//last_node = TAILQ_LAST(&head, head_s);
	node * tmp_node = NULL;	
	if(current_node != NULL)
	{
		tmp_node = current_node;
		//puts(tmp_node->site);
		//printf("%s\n", tmp_node->site);
		current_node = TAILQ_NEXT(current_node, nodes);
		/*struct node * tmp_node = current_node;
		current_node = TAILQ_NEXT(current_node, nodes);*/
		return tmp_node;	
	}
	return NULL;
}

void print_status(void)
{
	int i;
	for(i =0; i<Site_count; i++)
	{
		printf("%s\n", Status[i]);
	}
	printf("\n");
}

void *ping_site(void *a)
{
	//printf("%d : %s\n", pthread_self(), (char*)Site);
	//puts("Line137");
	struct node * node1 = malloc(sizeof(struct node)); 
	while(1)
	{
		pthread_mutex_lock(&queue_lock);
		node1 = Read_queue();
		pthread_mutex_unlock(&queue_lock);

		if(node1 != NULL)
		{
			//puts(node1->site);
			while(pthread_mutex_trylock(&lock[lock_count]))
			{
				lock_count++;
				if(lock_count >= 5)
				{
					lock_count = 0;
				}
			}

			char* Site = (char*) malloc(20);
			strcpy(Site,node1->site);
			int tid = node1->tid;
			int site_number = node1->site_number;
			FILE *fp;
			char linux_cmd[] = "/bin/ping ";
		  	char ping_opt[] = " -c 10";

		  	char* ping = (char*)malloc(200);
		  	strcat(linux_cmd, (char*)Site);
		  	strcat(linux_cmd, ping_opt);
			strcpy(ping,linux_cmd);
			/* Open the command for reading. */
			fp = popen(ping, "r");
			if (fp == NULL) {
			printf("Failed to run command\n" );
			exit(1);
			}

			char path[1035];
			/* Read the output a line at a time - output it. */
			while (fgets(path, sizeof(path)-1, fp) != NULL);

			char* curr_Status = (char*)malloc(1000);
			char* output = (char*)malloc(1000);
			strcpy(output,path);
			/**************************************************************************************/
			//Parsing the Output

			output = strtok(output, "=");
			output = strtok(NULL, " ");
			output = strtok(output, "/");
			strcpy(path, output);
			strcat(path, "\t");
			output = strtok(NULL, "/");
			strcat(path,output);
			strcat(path, "\t");
			output = strtok(NULL, "/");
			strcat(path,output);
			strcat(path, "\t");
			strcat(path, ": COMPLETED \0");
			
			strcat(Site, "\t");
			strcat(Site, path);
			sprintf(curr_Status,"%d ", tid);
			strcat(curr_Status,Site);
			//printf("%s\n", curr_Status );
			pthread_mutex_unlock(&lock[lock_count]);
			lock_count = 0;
			thread_count--;
			//printf("%s\n",curr_Status);
			strcpy(Status[site_number],curr_Status);

			print_status();
			pclose(fp);
		}	
	}
	
	/**************************************************************************************/
	/* close */	
}

char ping_command(char* input, int client_tid)
{

	//puts("Line251");
	char* temp = (char*)malloc(3000);
	strcpy(temp, input);
	char* command = (char*)malloc(1100);
	char* Sites = (char*)malloc(3000);
	temp = strtok(temp, " ");
	strcpy(command,temp);
	//puts(command);
	temp = strtok(NULL, " ");
	strcpy(Sites, temp);


	char* output = (char*)malloc(3000);

	strcpy(output, Sites);
	int Nsite;
	for (Nsite=0; output[Nsite]; output[Nsite]==',' ? Nsite++ : *output++); //Counts number of Commas
	strcpy(output, Sites);
	
	if(Nsite != 0){
		
		output = strtok(output, ",");
		Update_queue(output,client_tid);
		int count = 0;
		while(count != Nsite){
			output = strtok(NULL, ",");
			//puts(output);
			Update_queue(output, client_tid);
			count++;
		}
	}
	else
	{
		Update_queue(output, client_tid);
	}
}

void *connection_handler(void *socket_desc)
{
	int sock = *(int*)socket_desc;
	char* handle = (char*)malloc(50);

	pthread_mutex_lock(&client_id_lock);
	sprintf(handle, "%d Client", client_tid);
	
	send(sock , handle  , strlen(handle) , 0 );
	strcpy(handles[client_tid], handle);
	client_tid++;
	pthread_mutex_unlock(&client_id_lock);
	return 0;
}

int main(int argc, char* argv[])
{
	int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";

	TAILQ_INIT(&head);
	current_node = TAILQ_FIRST(&head);
	
	for(thread_count =0; thread_count < 5; thread_count++)
	{
		if(pthread_create(&worker_threads[thread_count], NULL, ping_site, NULL))
		{
			
			fprintf(stderr, "Error creating thread\n");
			//return 1;
		}
	}

	// Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
      
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, 
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while(1)
    {

	    if((new_socket = accept(server_fd, (struct sockaddr *)&address, 
	                       (socklen_t*)&addrlen))<0)
	    {
	    	perror("accept");
	        exit(EXIT_FAILURE);
	    }

	    valread = read( new_socket , buffer, 1100);
		//char input[1100] = "pingSites www.google.com,www.espn.com,www.cnn.com,www.facebook.com,\
	www.Linkedin.com,www.youtube.com,www.cricinfo.com,www.quora.com";
		//puts(buffer);
		char* temp = (char*)malloc(3000);
		strcpy(temp, buffer);
		char* command = (char*)malloc(1100);
		temp = strtok(temp, " ");
		strcpy(command,temp);
		//command = strtok(command, " ");
		
		if (strcmp(command,"pingSites")==0)
		{
			//puts(command);
			ping_command(buffer, client_tid);
			pthread_t client_id;
			if(pthread_create(&client_id, NULL, connection_handler, (void*) &new_socket))
			{
				perror("client thread create error");
				return 1;
			}
			printf("Handler Assigned to client %d\n", client_tid);
			//client_tid++;
			//if(client_tid == 0)
			//{
			current_node = TAILQ_FIRST(&head);
			//}
		}
		
		/*struct node * node1 = malloc(sizeof(struct node));
		TAILQ_FOREACH(node1, &head, nodes)
		{
			printf("%d:", node1->tid);
			printf("%d:", node1->site_number);
			printf("%s\n", node1->site);
		}*/

		else if(strcmp(command,"showHandles") == 0)
		{

			int i;
			for(i =0; i<client_tid; i++)
			{
				char* temp = (char*)malloc(50);
				strcpy(temp, handles[i]);
				strcat(temp,"\n");
				send(new_socket, temp, strlen(temp) , 0);
			}
			printf("\n");
		}

		else if(strcmp(command,"showHandleStatus") == 0)
		{
			char* handler_name = strtok(NULL, " ");
			puts(handler_name);
			if(handler_name)
			{
				char* client_number = (char*)malloc(10);
				int i=0;
				for( i = 0; i < Site_count; i++ )
				{
					char* tmp = (char*)malloc(200);
					strcpy(tmp, Status[i]);
					tmp = strtok(tmp, " ");
					if(strcmp(handler_name, tmp) == 0)
					{
						strcpy(tmp, Status[i]);
						strcat(tmp,"\n");
						send(new_socket, tmp, strlen(tmp) , 0);
					}

				}

			}
			/*else
			{
				int i;
				for(i =0; i<Site_count; i++)
				{
					char* temp = (char*)malloc(200);
					strcpy(temp, Status[i]);
					strcat(temp,"\n");	
					send(new_socket, temp, strlen(temp) , 0);
				}
			}*/
		}
		else
		{
			perror("Invalid Command");
			//EXIT_FAILURE;
		}
	}
}

//pingSites www.google.com,www.espn.com,www.cnn.com,www.facebook.com,www.Linkedin.com,www.youtube.com,www.cricinfo.com,www.quora.com