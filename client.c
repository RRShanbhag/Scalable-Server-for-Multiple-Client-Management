// Client side C/C++ program to demonstrate Socket programming
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#define PORT 8080
  
int main(int argc, char *argv[])
{
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char input[2024];
    
    if(strcmp(argv[2], "pingSites")==0)
    {
        strcpy(input, argv[2]);
        strcat(input, " ");
        strcat(input, argv[3]);
    }

    if(strcmp(argv[2], "showHandles")==0)
    {
        strcpy(input, argv[2]);
        strcat(input, " ");
    }

    if(strcmp(argv[2], "showHandleStatus")==0)
    {
        strcpy(input, argv[2]);
        strcat(input, " ");
        if(argv[3] != NULL)
        {   
            strcat(input, argv[3]);
            strcat(input, " ");
            strcat(input, argv[4]);
        }
        
    }

    if(strcmp(argv[2], "showallStatus")==0)
    {
        strcpy(input, argv[2]);
        strcat(input, " ");
    }

    char buffer[2024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
  
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
      
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    send(sock , input , strlen(input) , 0 );
    printf("\n");

    while((valread = read( sock , buffer, 2024)) > 0)
    {
      printf("%s\n",buffer );
        
    }
    
    return 0;
}