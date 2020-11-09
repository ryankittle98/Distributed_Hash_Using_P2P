#include "dataStructures.h"
#define ECHOMAX 255     /* Longest string to echo */


int mode;
char echoString[ECHOMAX+1];                /* String to send to echo server */
char returnString[ECHOMAX+1];
char echoBuffer[ECHOMAX+1];      /* Buffer for receiving echoed string */
registeredClient* thisClient;
struct sockaddr_in fromAddr;     /* Source address of echo */
unsigned int fromSize;           /* In-out of address size for recvfrom() */
int sock;
int sockR;                        /* Socket descriptor */
int sockL;
int sockQ;

void thread_recvfrom()
{
	while (true)
	{
		if (mode == 0)
		{			
			if (recvfrom(sockQ, returnString, ECHOMAX, MSG_DONTWAIT, (struct sockaddr *) &fromAddr, &fromSize) > 0 )
			{
				memset(&echoString, 0, sizeof(echoString));
				strcpy(echoString, returnString);
				memset(&returnString, 0, sizeof(returnString));
				mode = 1;
			}
	
			if (recvfrom(sockL, returnString, ECHOMAX, MSG_DONTWAIT, (struct sockaddr *) &fromAddr, &fromSize) > 0 )
			{
				memset(&echoString, 0, sizeof(echoString));
				strcpy(echoString, returnString);
				memset(&returnString, 0, sizeof(returnString));
				mode = 1;
			}
		}
	}
}
void thread_stdin()
{
	size_t temp2 = 0;
	while (true)
	{
		if (mode == 0)
		{
			char* stdin_input;
			temp2 = 0;
			getline(&stdin_input, &temp2, stdin);
			strcpy(echoString, stdin_input);
			mode = 2;
		}
	}
		
}

int main(int argc, char *argv[])
{
   	
    	struct sockaddr_in echoServAddr; /* Echo server address */
    	
    	unsigned short echoServPort;     /* Echo server port */
    	
    	char *servIP;                    /* IP address of server */
    	
	
	char inputCommand[ECHOMAX];
    	int echoStringLen;               /* Length of string to echo */
    	int respStringLen;               /* Length of received response */

	thisClient = new registeredClient;
	

    	if (argc < 3)    /* Test for correct number of arguments */
    	{
    	    fprintf(stderr,"Usage: %s <Server IP address> <Echo Port>\n", argv[0]);
    	    exit(1);
    	}

    	servIP = argv[1];           /* First arg: server IP address (dotted quad) */
    	echoServPort = atoi(argv[2]);  /* Second arg: Use given port, if any */

	printf( "Arguments passed: server IP %s, port %d\n", servIP, echoServPort );

    	/* Create a datagram/UDP socket */
    	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    	/* Construct the server address structure */
    	memset(&echoServAddr, 0, sizeof(echoServAddr));    /* Zero out structure */
    	echoServAddr.sin_family = AF_INET;                 /* Internet addr family */
    	echoServAddr.sin_addr.s_addr = inet_addr(servIP);  /* Server IP address */
    	echoServAddr.sin_port   = htons(echoServPort);     /* Server port */

	while(strcmp(thisClient->state.c_str(), "u") == 0) {

		printf("\nENTER REGISTER COMMAND : \n");

		size_t temp2 = 0;
		char* stdin_input;
		getline(&stdin_input, &temp2, stdin);
		strcpy(echoBuffer, stdin_input);
		echoBuffer[strlen(echoBuffer) - 1] = '\0';

		if ( strcmp(strtok(stdin_input, " "), "register") == 0) {

			if (sendto(sock, echoBuffer, strlen(echoBuffer), 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) != strlen(echoBuffer))
	       			DieWithError("sendto() sent a different number of bytes than expected");

			fromSize = sizeof(fromAddr);
		    	if ((respStringLen = recvfrom(sock, returnString, ECHOMAX, 0, (struct sockaddr *) &fromAddr, &fromSize)) <= ECHOMAX )
			{
				returnString[respStringLen] = '\0';
			
			   	printf("\n-- SERVER RESPONSE -- \n%s\n", returnString );    /* Print the echoed arg */

				/* Get input command into an array of words */
				strcpy(inputCommand, returnString);
				char* word[64];
				word[0] = strtok(inputCommand, " ");
				for (int i = 1; i < 64; i++)
					word[i] = strtok (NULL, " ");

				/* CLIENT ACTIONS UPON SERVER RESPONSE : REGISTER */
				if (strcmp(word[0], "register") == 0 && strcmp(word[7], "SUCCESS") == 0) {

					thisClient->username = word[1];
					
					if ((sockR = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        					DieWithError("socket() failed");

					memset(&thisClient->portR, 0, sizeof(thisClient->portR));   /* Zero out structure */
					thisClient->portR.sin_family = AF_INET;                /* Internet address family */
					thisClient->portR.sin_addr.s_addr = inet_addr(word[2]); /* Any incoming interface */
					thisClient->portR.sin_port = htons(atoi(word[3]));      /* Local port */

					if (bind(sockR, (struct sockaddr *) &thisClient->portR, sizeof(thisClient->portR)) < 0)
        					DieWithError("bind() failed");


					if ((sockL = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        					DieWithError("socket() failed");

					memset(&thisClient->portL, 0, sizeof(thisClient->portL));   /* Zero out structure */
					thisClient->portL.sin_family = AF_INET;                /* Internet address family */
					thisClient->portL.sin_addr.s_addr = inet_addr(word[2]); /* Any incoming interface */
					thisClient->portL.sin_port = htons(atoi(word[4]));      /* Local port */

					if (bind(sockL, (struct sockaddr *) &thisClient->portL, sizeof(thisClient->portL)) < 0)
        					DieWithError("bind() failed");


					if ((sockQ = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        					DieWithError("socket() failed");

					memset(&thisClient->portQ, 0, sizeof(thisClient->portQ));   /* Zero out structure */
					thisClient->portQ.sin_family = AF_INET;                /* Internet address family */
					thisClient->portQ.sin_addr.s_addr = inet_addr(word[2]); /* Any incoming interface */
					thisClient->portQ.sin_port = htons(atoi(word[5]));      /* Local port */
			
					if (bind(sockQ, (struct sockaddr *) &thisClient->portQ, sizeof(thisClient->portQ)) < 0)
        					DieWithError("bind() failed");


					thisClient->state = "f";

					printf("(CLIENT'S INFO SET)\n");
				}				
			}
		}
		else
			printf("\nINVALID REGISTER COMMAND\n");
	}

	/* START THREADS */
	mode = 0;	
	std::thread threadObj1(thread_recvfrom);
	std::thread threadObj2(thread_stdin);

    	while(true)
    	{
    		char prevCommand[32];
    		if(strcmp(prevCommand, "store") != 0)
			printf( "\nENTER ECHO : \n" );

		//memset(&echoBuffer, 0, sizeof(echoBuffer));
		//memset(&echoString, 0, sizeof(echoString));
		//memset(&returnString, 0, sizeof(returnString));

		while(mode == 0){;}

		memset(&echoBuffer, 0, sizeof(echoBuffer));
		strcpy(echoBuffer, echoString);
		echoBuffer[strlen(echoBuffer) - 1] = '\0';

		// IF THERE IS A COMMAND BEING SENT BETWEEN PEERS, HANDLE IT IN THIS IF STATEMENT
		if (mode == 1) {
		
			strcpy(inputCommand, echoBuffer);
			char* word[64];
			word[0] = strtok(inputCommand, " ");
			for (int i = 1; i < 64; i++) 
				word[i] = strtok (NULL, " ");
			
			if(strcmp(word[0], "set-id") == 0) {

				printf( "P2P COMMAND RECEIVED : %s\n", word[0] );
				
				thisClient->state = "i";
				thisClient->id = atoi(word[1]);
				thisClient->ringSize = atoi(word[2]);
				
				sockaddr_in dhtLeftNeighbor;
								
				memset(&dhtLeftNeighbor, 0, sizeof(dhtLeftNeighbor));   /* Zero out structure */
				dhtLeftNeighbor.sin_family = AF_INET;                /* Internet address family */
				dhtLeftNeighbor.sin_addr.s_addr = inet_addr(word[7]); /* Any incoming interface */
				dhtLeftNeighbor.sin_port = htons(atoi(word[8]));      /* Local port */
				
				thisClient->leftNeighbor = dhtLeftNeighbor;
				
				sockaddr_in dhtRightNeighbor;
				
				memset(&dhtRightNeighbor, 0, sizeof(dhtRightNeighbor));   /* Zero out structure */
				dhtRightNeighbor.sin_family = AF_INET;                /* Internet address family */
				dhtRightNeighbor.sin_addr.s_addr = inet_addr(word[14]); /* Any incoming interface */
				dhtRightNeighbor.sin_port = htons(atoi(word[16]));      /* Local port */
				
				thisClient->rightNeighbor = dhtRightNeighbor;
				
				printf("(CLIENT'S INFO SET)\n");				
			}
			
			else if(strcmp(word[0], "store") == 0) {
				if (atoi(word[1]) == thisClient->id)
				{
					if(strcmp(prevCommand, "store") != 0)
						printf( "P2P COMMAND RECEIVED : %s\n(DATA STORED ON THIS CLIENT)\n", word[0] );
					
					char csv_data[256];
					char command_copy[256];
					strcpy(command_copy, echoBuffer);
					strcpy(csv_data, strtok(command_copy, "<"));
					strcpy(csv_data, strtok(NULL, ">"));
				
					thisClient->localTable.put(atoi(word[2]), csv_data);
				}
				else
				{
					sendto(sockR, echoBuffer, strlen(echoBuffer), 0, (struct sockaddr *) &thisClient->rightNeighbor, sizeof(thisClient->rightNeighbor));
				}
			}
			else if(strcmp(word[0], "finished") == 0) {
			
				if (thisClient->id != thisClient->ringSize - 1)
				{
					printf("\nENTER ECHO :\nP2P COMMAND RECEIVED : %s\n(ALL DATA STORED)\n", word[0]);
					char finished[16];
					strcpy(finished, "finished ");
					sendto(sockR, finished, strlen(finished), 0, (struct sockaddr *) &thisClient->rightNeighbor, sizeof(thisClient->rightNeighbor));
				}
				else
					printf("\nENTER ECHO :\nP2P COMMAND RECEIVED : %s\n(ALL DATA STORED)\n", word[0]);
			}
			else if (strcmp(word[0], "teardown-dht") == 0)
			{
				printf( "P2P COMMAND RECEIVED : %s\n", word[0] );
				
				if (strcmp(thisClient->state.c_str(), "l") != 0)
					sendto(sockR, strcat(word[0], " "), strlen(word[0]) + 1, 0, (struct sockaddr *) &thisClient->rightNeighbor, sizeof(thisClient->rightNeighbor));
				
				if (strcmp(thisClient->state.c_str(), "l") == 0)
					printf("\n-- COMMAND MESSAGE -- \nTEARDOWN FINISHED : ISSUE teardown-complete <user> COMMAND\n");
									

				thisClient->localTable.deleteTable();
				thisClient->state = "f";
				thisClient->id = 9999;
				
				memset(&thisClient->rightNeighbor, 0, sizeof(struct sockaddr_in));
				memset(&thisClient->leftNeighbor, 0, sizeof(struct sockaddr_in));
			}
			else if (strcmp(word[0], "query") == 0)
			{
				printf( "P2P COMMAND RECEIVED : %s\n", word[0] );
				
				char echoBufCopy[256];
				strcpy(echoBufCopy, echoBuffer);
				char echoBufSave[256];
				strcpy(echoBufSave, echoBuffer);
			
				strcpy(echoBufCopy, echoBufCopy+7);
				strtok(echoBufCopy, ">");
				
				char longName[64];
				strcpy(longName, echoBufCopy);
				int pos = 0;
				int id = 0;
				int ascii_sum = 0;
				for (int i = 0; i < strlen(longName); i++)
					ascii_sum += int(longName[i]);
			
				pos = ascii_sum % 353;
				int groupSize = thisClient->ringSize;
				id = pos % groupSize;
				
				if (id == thisClient->id) {
					std::string dht_entry;
					dht_entry = thisClient->localTable.get(pos, echoBufCopy);
					
					if (dht_entry.find(longName) != std::string::npos)
					{
						memset(&echoBuffer, 0, sizeof(echoBuffer));
						strcpy(echoBuffer, "SUCCESS - ");
						strcat(echoBuffer, dht_entry.c_str());
						strcat(echoBuffer, "\n");
					}
					else
					{
						memset(&echoBuffer, 0, sizeof(echoBuffer));
						strcpy(echoBuffer, "FAILURE\n");
					}
					
					//If first receiver didn't find string, send to the attached address
					bool passedAlready = false;
					for (int i = 0; i < 64; i++)
					{
						if (word[i] != NULL)
						{
							if (strcmp(word[i], "\n-") == 0)
							passedAlready = true;
						}
						
					}
					
					if (passedAlready)
					{
						char getIP[512];
						char getPort[512];
						char* IP;
						char* Port;
						
						strcpy(getIP, echoBufSave);
						IP = strtok (getIP, ":");
						IP = strtok (NULL, ":");
						
						strcpy(getPort, echoBufSave);
						Port=strtok(getPort, ":");
						Port=strtok(NULL, ":");
						Port=strtok(NULL, ":");
						Port=strtok(NULL, ":");
						
						sockaddr_in origSender;
						memset(&origSender, 0, sizeof(origSender));   /* Zero out structure */
						origSender.sin_family = AF_INET;                /* Internet address family */
						origSender.sin_addr.s_addr = inet_addr(IP); /* Any incoming interface */
						origSender.sin_port = htons(atoi(Port));      /* Local port */
						
						sendto(sockQ, echoBuffer, strlen(echoBuffer), 0, (struct sockaddr *) &origSender, sizeof(origSender));
					}
					//otherwise just send back
					else
					{
						sendto(sockQ, echoBuffer, strlen(echoBuffer), 0, (struct sockaddr *) &fromAddr, sizeof(fromAddr));
					}	
				}
				else
				{
					strcat(echoBuffer, " \n- ip addr -\t- port - \n :");
					strcat(echoBuffer, inet_ntoa(fromAddr.sin_addr));
					strcat(echoBuffer, ": \t :");
					strcat(echoBuffer, std::to_string(ntohs(fromAddr.sin_port)).c_str());
					strcat(echoBuffer, ": \n\n");
					
					sendto(sockR, echoBuffer, strlen(echoBuffer), 0, (struct sockaddr *) &thisClient->rightNeighbor, sizeof(thisClient->rightNeighbor));
				}
			}
			else if(strcmp(word[0], "reset-id") == 0) {
			
				printf( "P2P COMMAND RECEIVED : %s\n", word[0] );
				
				thisClient->localTable.deleteTable();

				if (atoi(word[1]) == 0)
					thisClient->state = "l";
				else
					thisClient->state = "i";
				thisClient->id = atoi(word[1]);
				thisClient->ringSize = atoi(word[2]);
				
				sockaddr_in dhtLeftNeighbor;
				
				memset(&dhtLeftNeighbor, 0, sizeof(dhtLeftNeighbor));   /* Zero out structure */
				dhtLeftNeighbor.sin_family = AF_INET;                /* Internet address family */
				dhtLeftNeighbor.sin_addr.s_addr = inet_addr(word[7]); /* Any incoming interface */
				dhtLeftNeighbor.sin_port = htons(atoi(word[8]));      /* Local port */
				
				thisClient->leftNeighbor = dhtLeftNeighbor;
				
				sockaddr_in dhtRightNeighbor;
				
				memset(&dhtRightNeighbor, 0, sizeof(dhtRightNeighbor));   /* Zero out structure */
				dhtRightNeighbor.sin_family = AF_INET;                /* Internet address family */
				dhtRightNeighbor.sin_addr.s_addr = inet_addr(word[14]); /* Any incoming interface */
				dhtRightNeighbor.sin_port = htons(atoi(word[16]));      /* Local port */
				
				thisClient->rightNeighbor = dhtRightNeighbor;
				
				if (atoi(word[1]) == 0)
					printf("(CLIENT'S INFO RESET AS NEW LEADER)\n");
				else
					printf("(CLIENT'S INFO RESET)\n");
				
				////////////////
				if (thisClient->id == 0)
				{
					sleep(1);
					// READ IN THE CSV FILE ON LEADER 
					// then send each line to node
					std::ifstream file;
					char line[512];
					file.open ("StatsCountry.csv");
		
					if (file.is_open())
					{
						file.getline(line, 512, '\n'); //get the first line since its data should not be stored
				
						int i = 2;
						while (i < 243)
						{
							file.getline(line, 512, '\n');
						
							char lineCopy[512];
							char* longName;
							char store_command[512];
						
							strcpy(lineCopy, line);
							longName = strtok (lineCopy, ",");
							longName = strtok (NULL, ",");
							longName = strtok (NULL, ",");
							longName = strtok (NULL, ",");
							if (i == 23 || i == 42 || i == 43 || i == 64 || i == 75 || i == 82 || i == 92 || i == 102 || i == 116 || i == 135 || i == 146 || i == 180 || i == 232 || i == 239)
								longName = strtok (NULL, ",");
							if (i == 92 || i == 135)
								longName = strtok (NULL, ",");
						
							int pos = 0;
							int id = 0;
							int ascii_sum = 0;
							
							for (int i = 0; i < strlen(longName); i++)
								ascii_sum += int(longName[i]);
					
							pos = ascii_sum % 353;
							int groupSize = thisClient->ringSize;
							id = pos % groupSize;
						
							strcpy(store_command, "store ");
							strcat(store_command, std::to_string(id).c_str());
							strcat(store_command, " ");
							strcat(store_command, std::to_string(pos).c_str());
							strcat(store_command, " \n<");
							strcat(store_command, line);
							strcat(store_command, ">\n");
						
							if (id == 0)
								thisClient->localTable.put(pos, line);
							else
								sendto(sockR, store_command, strlen(store_command), 0, (struct sockaddr *) &thisClient->rightNeighbor, sizeof(thisClient->rightNeighbor));
							i++;
							
						}
					}
					sleep(1);
					char finished[16];
					strcpy(finished, "finished ");
					sendto(sockR, finished, strlen(finished), 0, (struct sockaddr *) &thisClient->rightNeighbor, sizeof(thisClient->rightNeighbor));
				}	
			}
			strcpy(prevCommand, word[0]);
		}
		// ELSE IF THERE IS A COMMAND BETWEEN A PEER AND THE SERVER, HANDLE IT IN THIS ELSE STATEMENT
		else if (mode == 2) {
		
			printf( "\n-- COMMAND MESSAGE -- \n%s\n", echoBuffer );
		
			if ( strcmp(strtok(echoString, " "), "register") == 0) {
				
				printf("ERROR: CANNOT REGISTER CLIENT MULTIPLE TIMES\n");
			}
			else {

				/* Send the struct to the server */
			    	if (sendto(sock, echoBuffer, strlen(echoBuffer), 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) != strlen(echoBuffer))
			       		DieWithError("sendto() sent a different number of bytes than expected");
			
			    	/* Recv a response */
			    	fromSize = sizeof(fromAddr);
			    	if ((respStringLen = recvfrom(sock, returnString, ECHOMAX, 0, (struct sockaddr *) &fromAddr, &fromSize)) <= ECHOMAX )
				{
					returnString[respStringLen] = '\0';

				
				   	printf("-- SERVER REPONSE -- \n%s\n", returnString );    /* Print the echoed arg */

					/* Get input command into an array of words */
					strcpy(inputCommand, returnString);
					char* word[64];
					word[0] = strtok(inputCommand, " ");
					for (int i = 1; i < 64; i++)
						word[i] = strtok (NULL, " ");

					/* CLIENT ACTIONS UPON SERVER RESPONSE : SETUP-DHT */
					if (strcmp(word[0], "setup-dht") == 0) {

						if (strcmp(word[4], "SUCCESS") == 0) {

							thisClient->state = "l";
							thisClient->id = 0;
							thisClient->ringSize = atoi(word[1]);
							
							sockaddr_in dhtLeftNeighbor;
							
							memset(&dhtLeftNeighbor, 0, sizeof(dhtLeftNeighbor));   /* Zero out structure */
							dhtLeftNeighbor.sin_family = AF_INET;                /* Internet address family */
							dhtLeftNeighbor.sin_addr.s_addr = inet_addr(word[15 + atoi(word[1])*4]); /* Any incoming interface */
							dhtLeftNeighbor.sin_port = htons(atoi(word[15 + atoi(word[1])*4 + 1]));      /* Local port */
							
							thisClient->leftNeighbor = dhtLeftNeighbor;
							
							sockaddr_in dhtRightNeighbor;
							
							memset(&dhtRightNeighbor, 0, sizeof(dhtRightNeighbor));   /* Zero out structure */
							dhtRightNeighbor.sin_family = AF_INET;                /* Internet address family */
							dhtRightNeighbor.sin_addr.s_addr = inet_addr(word[22]); /* Any incoming interface */
							dhtRightNeighbor.sin_port = htons(atoi(word[24]));      /* Local port */
							
							thisClient->rightNeighbor = dhtRightNeighbor;

							int userID = 1;
							for(int i = 22; i < (sizeof(word) - 22) && userID < atoi(word[1]); i+=5, userID++)
							{
								memset(&echoBuffer, 0, sizeof(echoBuffer));
								strcpy(echoBuffer, "set-id ");
								strcat(echoBuffer, std::to_string(userID).c_str());
								strcat(echoBuffer, " ");
								strcat(echoBuffer, word[1]);
								strcat(echoBuffer, " \n");
								
								strcat(echoBuffer, "Left Neighbor : ");
								strcat(echoBuffer, word[i - 6]);
								strcat(echoBuffer, " ");
								strcat(echoBuffer, word[i - 5]);
								strcat(echoBuffer, " ");
								strcat(echoBuffer, word[i - 4]);
								strcat(echoBuffer, " ");
								strcat(echoBuffer, word[i - 3]);
								strcat(echoBuffer, " ");
								strcat(echoBuffer, word[i - 2]);
								strcat(echoBuffer, "\n");
								
								//Make the last user's right neighbor be the leader
								if (userID == atoi(word[1]) - 1)
								{
									strcat(echoBuffer, "Right Neighbor : ");
									strcat(echoBuffer, word[16]);
									strcat(echoBuffer, " ");
									strcat(echoBuffer, word[17]);
									strcat(echoBuffer, " ");
									strcat(echoBuffer, word[18]);
									strcat(echoBuffer, " ");
									strcat(echoBuffer, word[19]);
									strcat(echoBuffer, " ");
									strcat(echoBuffer, word[20]);
									strcat(echoBuffer, "\n");
								}
								else
								{
									strcat(echoBuffer, "Right Neighbor : ");
									strcat(echoBuffer, word[i + 4]);
									strcat(echoBuffer, " ");
									strcat(echoBuffer, word[i + 5]);
									strcat(echoBuffer, " ");
									strcat(echoBuffer, word[i + 6]);
									strcat(echoBuffer, " ");
									strcat(echoBuffer, word[i + 7]);
									strcat(echoBuffer, " ");
									strcat(echoBuffer, word[i + 8]);
									strcat(echoBuffer, "\n");
								}
								
								sockaddr_in dhtFollowerQuery;

								memset(&dhtFollowerQuery, 0, sizeof(dhtFollowerQuery));   /* Zero out structure */
								dhtFollowerQuery.sin_family = AF_INET;                /* Internet address family */
								dhtFollowerQuery.sin_addr.s_addr = inet_addr(word[i]); /* Any incoming interface */
								dhtFollowerQuery.sin_port = htons(atoi(word[i + 3]));      /* Local port */

								sendto(sockQ, echoBuffer, strlen(echoBuffer), 0, (struct sockaddr *) &dhtFollowerQuery, sizeof(dhtFollowerQuery));
							}
							sleep(1);
							// READ IN THE CSV FILE ON LEADER 
							// then send each line to node
							std::ifstream file;
							char line[512];
							file.open ("StatsCountry.csv");
							if (file.is_open())
							{
								file.getline(line, 512, '\n'); //get the first line since its data should not be stored
								
								int i = 2;
								while (i < 243)
								{
									file.getline(line, 512, '\n');
									
									char lineCopy[512];
									char* longName;
									char store_command[512];
									
									strcpy(lineCopy, line);
									longName = strtok (lineCopy, ",");
									longName = strtok (NULL, ",");
									longName = strtok (NULL, ",");
									longName = strtok (NULL, ",");
									if (i == 23 || i == 42 || i == 43 || i == 64 || i == 75 || i == 82 || i == 92 || i == 102 || i == 116 || i == 135 || i == 146 || i == 180 || i == 232 || i == 239)
										longName = strtok (NULL, ",");
									if (i == 92 || i == 135)
										longName = strtok (NULL, ",");
							
									int pos = 0;
									int id = 0;
									int ascii_sum = 0;
									
									for (int i = 0; i < strlen(longName); i++)
										ascii_sum += int(longName[i]);
								
									pos = ascii_sum % 353;
									int groupSize = atoi(word[1]);
									id = pos % groupSize;
								
									strcpy(store_command, "store ");
									strcat(store_command, std::to_string(id).c_str());
									strcat(store_command, " ");
									strcat(store_command, std::to_string(pos).c_str());
									strcat(store_command, " \n<");
									strcat(store_command, line);
									strcat(store_command, ">\n");
									
									if (id == 0)
										thisClient->localTable.put(pos, line);
									else
										sendto(sockR, store_command, strlen(store_command), 0, (struct sockaddr *) &thisClient->rightNeighbor, sizeof(thisClient->rightNeighbor));
									i++;
								}
							}
							sleep(1);
							char finished[16];
							strcpy(finished, "finished ");
							sendto(sockR, finished, strlen(finished), 0, (struct sockaddr *) &thisClient->rightNeighbor, sizeof(thisClient->rightNeighbor));
							printf("-- COMMAND MESSAGE -- \nALL DATA STORED : ISSUE dht-complete <user> COMMAND\n");
						}
					}
					
					else if (strcmp(word[0], "leave-dht") == 0) {

						if (strcmp(word[3], "SUCCESS") == 0) {

							int userID = 0;
							int original_size = thisClient->ringSize;
							for(int i = 16; i < (sizeof(word) - 16) && userID < original_size - 1; i+=5, userID++)
							{
								memset(&echoBuffer, 0, sizeof(echoBuffer));
								strcpy(echoBuffer, "reset-id ");
								strcat(echoBuffer, std::to_string(userID).c_str());
								strcat(echoBuffer, " ");
								strcat(echoBuffer, std::to_string(original_size - 1).c_str());
								strcat(echoBuffer, " \n");
								
								if (userID != 0)
								{								
									strcat(echoBuffer, "Left Neighbor : ");
									strcat(echoBuffer, word[i - 6]);
									strcat(echoBuffer, " ");
									strcat(echoBuffer, word[i - 5]);
									strcat(echoBuffer, " ");
									strcat(echoBuffer, word[i - 4]);
									strcat(echoBuffer, " ");
									strcat(echoBuffer, word[i - 3]);
									strcat(echoBuffer, " ");
									strcat(echoBuffer, word[i - 2]);
									strcat(echoBuffer, "\n");
								}
								else
								{
									strcat(echoBuffer, "Left Neighbor : ");
									strcat(echoBuffer, word[12 + (original_size - 1)*4]);
									strcat(echoBuffer, " ");
									strcat(echoBuffer, word[12 + (original_size - 1)*4 + 1]);
									strcat(echoBuffer, " ");
									strcat(echoBuffer, word[12 + (original_size - 1)*4 + 2]);
									strcat(echoBuffer, " ");
									strcat(echoBuffer, word[12 + (original_size - 1)*4 + 3]);
									strcat(echoBuffer, " ");
									strcat(echoBuffer, word[12 + (original_size - 1)*4 + 4]);
									strcat(echoBuffer, "\n");
								}
								
								//Make the last user's right neighbor be the leader
								if (userID == original_size - 2)
								{					
									strcat(echoBuffer, "Right Neighbor : ");
									strcat(echoBuffer, word[15]);
									strcat(echoBuffer, " ");
									strcat(echoBuffer, word[16]);
									strcat(echoBuffer, " ");
									strcat(echoBuffer, word[17]);
									strcat(echoBuffer, " ");
									strcat(echoBuffer, word[18]);
									strcat(echoBuffer, " ");
									strcat(echoBuffer, word[19]);
									strcat(echoBuffer, "\n");
								}
								else
								{								
									strcat(echoBuffer, "Right Neighbor : ");
									strcat(echoBuffer, word[i + 4]);
									strcat(echoBuffer, " ");
									strcat(echoBuffer, word[i + 5]);
									strcat(echoBuffer, " ");
									strcat(echoBuffer, word[i + 6]);
									strcat(echoBuffer, " ");
									strcat(echoBuffer, word[i + 7]);
									strcat(echoBuffer, " ");
									strcat(echoBuffer, word[i + 8]);
									strcat(echoBuffer, "\n");
								}
								
								sockaddr_in dhtFollowerQuery;

								memset(&dhtFollowerQuery, 0, sizeof(dhtFollowerQuery));   /* Zero out structure */
								dhtFollowerQuery.sin_family = AF_INET;                /* Internet address family */
								dhtFollowerQuery.sin_addr.s_addr = inet_addr(word[i]); /* Any incoming interface */
								dhtFollowerQuery.sin_port = htons(atoi(word[i + 3]));      /* Local port */

								sendto(sockQ, echoBuffer, strlen(echoBuffer), 0, (struct sockaddr *) &dhtFollowerQuery, sizeof(dhtFollowerQuery));
							}
							sleep(3);
							printf("-- COMMAND MESSAGE -- \nALL DATA REDISTRIBUTED : ISSUE dht-rebuilt <user> <new leader> COMMAND\n");
						}
					}
					
					else if(strcmp(word[0], "teardown-dht") == 0 && strcmp(word[3], "SUCCESS") == 0)
					{
						sendto(sockR, strcat(word[0], " "), strlen(word[0]) + 1, 0, (struct sockaddr *) &thisClient->rightNeighbor, sizeof(thisClient->rightNeighbor));
					}

					else if (strcmp(word[0], "deregister") == 0 && strcmp(word[3], "SUCCESS") == 0)
					{
						memset(&thisClient, 0, sizeof(thisClient)); 
						printf("\nCLIENT HAS SUCCESSFULLY DEREGISTERED : EXITING NOW\n");
						exit(0);
					
					}
					else if(strcmp(word[0], "query-dht") == 0 && strcmp(word[3], "SUCCESS") == 0)
					{
						sockaddr_in querySocket;
							
						memset(&querySocket, 0, sizeof(querySocket));   /* Zero out structure */
						querySocket.sin_family = AF_INET;                /* Internet address family */
						querySocket.sin_addr.s_addr = inet_addr(word[16]);
						querySocket.sin_port = htons(atoi(word[19]));      /* Local port */
						
						printf("\nENTER FULL NAME OF COUNTRY FOR QUERY : ");
						
						char* stdin_input;
						size_t temp2 = 0;
						getline(&stdin_input, &temp2, stdin);
						
						strtok(stdin_input, "\n");
						
						memset(&echoBuffer, 0, sizeof(echoBuffer));
						strcpy(echoBuffer, "query <");
						strcat(echoBuffer, stdin_input);
						strcat(echoBuffer, "> ");
						
						
						if (sendto(sockQ, echoBuffer, strlen(echoBuffer), 0, (struct sockaddr *) &querySocket, sizeof(querySocket)) != strlen(echoBuffer))
			       				DieWithError("sendto() sent a different number of bytes than expected");
			       			
			       			if (recv(sockQ, echoBuffer, ECHOMAX, 0) < 0)
            						DieWithError("recvfrom() failed");
            					
            					char temp1[256];
            					strcpy(temp1, echoBuffer);
						char* word[64];
						word[0] = strtok(temp1, " ");
            					
            					if (strcmp(word[0], "SUCCESS") == 0)
            					{
            						printf("\n%s", echoBuffer);
            					}
            					else
            					{
            						printf("\nFAILURE - The record associated with %s is not found in the DHT.\n", stdin_input);
            					}
            				}
					strcpy(prevCommand, word[0]);
			    	}
			}
			
		}
		
		mode = 0;
	}
    
    	close(sockR);
	close(sockL);
	close(sockQ);
    	exit(0);
}
