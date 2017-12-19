#include "Server.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include "mergesort.h"
#include <math.h>
#include "sortedmerge.h"
#include <string.h>
#include <errno.h>

#define MESSAGESIZE 5   //NUMBER OF BYTES DESCRIBING MESSAGE LENGTH
#define BUFFERSIZE 99999 //MESSAGE LENGTH TOTAL

node* finalList[1024];
int counter = 0;
int barrier = 0;
bool isNUM = FALSE;

pthread_mutex_t lock;


typedef struct _info{
	char* port;
}info;

#define BACKLOG 256 //max number of connections

//tokenize on ,
bool tokenize( int counter,/*node* head,*/ int chosen ,char* buffer)
{
		printf("TOKEN addr head: %p\n",(finalList[counter]));
		if((finalList[counter]) != NULL)
		printf("head before new node: %s\n\n",(finalList[counter])->data[0]);
	size_t size;
	int c = 0;
	bool isNumber = TRUE;

	node* newNode = NULL;
	newNode = (node*)malloc(sizeof(node));
	printf("Tom addr head: %p\n",newNode);
	newNode->data = (char**)malloc(sizeof(char*)*28);
	
	
	
	newNode->next = NULL;

	int i = 0;
	char* tok = buffer;
	char* end = buffer;
	char* specialTok = NULL;
	bool specialFound = FALSE;

	for (i=0; i<28; i++)
	{
		//
		tok = strsep(&end, ",");
		if(tok != NULL && tok[0] == '"')
		{
			char* tmp = (char*)malloc(sizeof(char)*strlen(tok)+1);
			strcpy(tmp,tok);
			tok = strsep(&end, "\"");
			specialTok = (char*)malloc(sizeof(char) * (strlen(tmp) + strlen(tok) + 3));
			strcpy(specialTok,tmp);
			strcat(specialTok,",");
			strcat(specialTok,tok);
			strcat(specialTok,"\"");
			specialFound = TRUE;
			tok = strsep(&end,",");
		}


		if(tok == NULL || tok == "")
		{
			newNode->data[i] = (char*)malloc(sizeof(char));
			newNode->data[i] = "\0";
			continue;
		}

		newNode->data[i] = (char*)malloc(sizeof(char) * strlen(buffer)+1);
		memset(newNode->data[i], '\0' , strlen(buffer)+1);
		if ( specialFound == FALSE)
		{
			newNode->data[i] = tok;
		}
		else
		{
			newNode->data[i] = specialTok;
			specialFound = FALSE;
		}
		int l = 0;
		char* ind = newNode->data[i];
		if ( i == chosen )
		{
			if ( isNumber == TRUE )
			{
				for(l=0; l < strlen( newNode->data[i]);l++)
				{
					if( ((*ind) - '0') <= 9 )
					{
						continue;
					}
					else
						isNumber = FALSE;
				}
			}
		}

	}
	
	if( finalList[counter] == NULL )
	{
		printf("head is NULL\n");
		(finalList[counter]) = newNode;
		printf("head[6]: %s\n",(finalList[counter])->data[6]);
	}
	else 
	{
		printf("BEFORE  addr newNode: %p\n",(newNode));
		newNode->next = (finalList[counter]);
		(finalList[counter]) = newNode;
		printf("head[6]: %s\n",(finalList[counter])->data[6]);
		printf("AFTER addr newNode: %p\n",(newNode));
	}	
	
	node* curr = (finalList[counter]);
	
	while(curr != NULL)
	{
		int m = 0;
		for ( m=0; m <28; m++ )
		printf("curr[%d]: %s\n",m, curr->data[m]);
		curr = curr->next;	
	}	
	return isNumber;
}

bool checkColumns(char* buffer)
{
	const char *headers[29];
		headers[0]= "color\0";
		headers[1]= "director_name\0";
		headers[2]= "num_critic_for_reviews\0" ;
		headers[3]= "duration\0";
		headers[4]= "director_facebook_likes\0";
		headers[5]= "actor_3_facebook_likes\0";
		headers[6]= "actor_2_name\0";
		headers[7]="actor_1_facebook_likes\0";
		headers[8]="gross\0";
		headers[9]="genres\0";
		headers[10]="actor_1_name\0";
		headers[11]="movie_title\0";
		headers[12]="num_voted_users\0";
		headers[13]="cast_total_facebook_likes\0";
		headers[14]="actor_3_name\0";
		headers[15]="facenumber_in_poster\0";
		headers[16]="plot_keywords\0";
		headers[17]="movie_imdb_link\0";
		headers[18]="num_user_for_reviews\0";
		headers[19]="language\0";
		headers[20]="country\0";
		headers[21]="content_rating\0";
		headers[22]="budget\0";
		headers[23]="title_year\0";
		headers[24]="actor_2_facebook_likes\0";
		headers[25]="imdb_score\0";
		headers[26]="aspect_ratio\0";
		headers[27]="movie_facebook_likes\0";
		headers[28]="-----\0";

	int i =0;
	for( i =0; i < 29; i++)
	{
		if( strcmp( buffer, headers[i]) == 0)
		{
			return i;
		}
	}
}
//
void readSocket( int socket, int bytestoread, void* buffer)
{
	int bytes = 0;
	int result;
	
	while(bytes < bytestoread)
	{
		result = read(socket, buffer + bytes, bytestoread - bytes);

		if( result < 1)
		{
			printf("did not read\n");
		}

		bytes += result;
	}
	printf("[readSocket]:: buffer: %s\n",buffer);

}

void insertNode( node** head, char** data)
{ 
	node* newNode = (node*)malloc(sizeof(node));
	newNode->data = data;
	(newNode)->next = (*head);

	(*head) = (newNode);
	printf("inserting::%s\n",(*head)->data[6]);
	return;

}

void* newConnection(void* info)
{

	int j =0;
	int socket = (*(int*)(info));
	int myList = counter;
	counter++;
	barrier++;
	pthread_mutex_unlock(&lock);
	bool first = TRUE;
	int index = 0;
	int written = 0;

	//node* head = NULL;
	//node* newNode = NULL;
	
	//newNode = (node*)malloc(sizeof(node));
	

	char* size;
	char* buffer;

	bool isNUMBER =FALSE;
	

	buffer = (char*)malloc(sizeof(char*)* BUFFERSIZE);
	size   = (char*)malloc(sizeof(char*)* 6);
	char* end = "-----\0";
	//newNode->data = (char**)malloc(sizeof(char*)*28);
	//newNode->next = NULL;
	
	bool dec = FALSE;
	//printf("BEFORE addr head: %p\n",&head);
while( strcmp(size,end) != 0)
{
	memset(size,'\0',6);
	memset(buffer,'\0',BUFFERSIZE);

	readSocket(socket, MESSAGESIZE, size);
		
	if ( strcmp(size,end) == 0 )
	{
		printf("breaking from loop!\n");
		break;	
	}
	
	readSocket(socket, atoi(size), buffer);
	
	if(first == TRUE)
		{
			index = checkColumns(buffer);
			printf("COLUMN: %s : %d\n", buffer,index);
			first= FALSE;
			continue;
		}	
	
	
	isNUMBER = 	tokenize(myList, index, buffer);
	//printf("addr head: %p\n",&head);
	
	if( isNUMBER == TRUE)
	{
			isNUMBER = TRUE;
			isNUM = TRUE;
	
	}
	
	//insertNode( &head, newNode->data);
	
	
	//printf("head\n",head->next->data[6]);
	printf("im here\n");
	if(finalList[myList] != NULL)
		printf("newNode: %s\n",finalList[myList]->data[1]);
}




}
	

int acceptConnections( char* port)
{
	//struct addrinfo* hostinfo;
	//struct addrinfo hints;
	//struct sockaddr clientAddr;
	
	struct sockaddr_in serv_addr, cli_addr;
	int listenSock, clientSock, portno;

	pthread_t tid;

	listenSock = socket(AF_INET, SOCK_STREAM, 0);
	bzero((char *) &serv_addr, sizeof(serv_addr));
     	portno = atoi(port);
	serv_addr.sin_family = AF_INET;
     	serv_addr.sin_addr.s_addr = INADDR_ANY;
     	serv_addr.sin_port = htons(portno);

	//listenSock = socket(hostinfo->ai_family, hostinfo->ai_socktype, hostinfo->ai_protocol);

	if ( bind(listenSock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) !=0)
	{
		printf("could not bind\n");
		return -2;
	}
		
	if ( listen(listenSock, BACKLOG) != 0 )
	{
		printf("could not listen\n");
		return -3;
	}

	while (1)
	{
		socklen_t clientLen = sizeof(cli_addr);
		pthread_mutex_lock(&lock);
		if ( (clientSock = accept(listenSock,(struct sockaddr *)&cli_addr, &clientLen  )) < 0)
		{
		  //		  printf("%s\n",gai_strerror(clientSock));
		  printf("can not accept\n");
		  return -4;
		}
		else
		{
			printf("about to thread\n");
			if( pthread_create( &tid, NULL, newConnection, &clientSock) != 0 )
			{
				printf("could not create thread\n");
				return -5;
			}
			printf("Threaded\n");
		}
	}




	return 0;
}
