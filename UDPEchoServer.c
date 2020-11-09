#include "dataStructures.h"
#include <iostream>

#define ECHOMAX 255     /* Longest string to echo */

std::vector<registeredClient*> Universe;
//
bool registerUser(char* word[], unsigned short serverPort)
{

	registeredClient* newClient = new registeredClient;

	newClient->username = word[1];
	
	memset(&newClient->portR, 0, sizeof(newClient->portR));
	newClient->portR.sin_family = AF_INET;        
	newClient->portR.sin_addr.s_addr = inet_addr(word[2]);
	newClient->portR.sin_port = htons(atoi(word[3]));      

	memset(&newClient->portL, 0, sizeof(newClient->portL));   
	newClient->portL.sin_family = AF_INET;               
	newClient->portL.sin_addr.s_addr = inet_addr(word[2]); 
	newClient->portL.sin_port = htons(atoi(word[4]));    

	memset(&newClient->portQ, 0, sizeof(newClient->portQ)); 
	newClient->portQ.sin_family = AF_INET;            
	newClient->portQ.sin_addr.s_addr = inet_addr(word[2]);
	newClient->portQ.sin_port = htons(atoi(word[5]));    

	newClient->state = "f";

	bool exists = false;

	if (Universe.size() > 0) {
		for(std::vector<registeredClient*>::iterator itr = Universe.begin(); itr != Universe.end(); ++itr) 
		{
			if (strcmp((*itr)->username.c_str(), newClient->username.c_str()) == 0)
					exists = true;

			if(	(*itr)->portL.sin_port == newClient->portL.sin_port || 
				(*itr)->portL.sin_port == newClient->portR.sin_port || 
				(*itr)->portL.sin_port == newClient->portQ.sin_port)
					exists = true;
			
			if(	(*itr)->portR.sin_port == newClient->portL.sin_port || 
				(*itr)->portR.sin_port == newClient->portR.sin_port || 
				(*itr)->portR.sin_port == newClient->portQ.sin_port)
					exists = true;

			if(	(*itr)->portQ.sin_port == newClient->portL.sin_port || 
				(*itr)->portQ.sin_port == newClient->portR.sin_port || 
				(*itr)->portQ.sin_port == newClient->portQ.sin_port)
					exists = true;
		}
	}
	if (	htons(serverPort) == newClient->portR.sin_port ||
		htons(serverPort) == newClient->portL.sin_port ||
		htons(serverPort) == newClient->portQ.sin_port)
			exists = true;

	if (!exists) {
		Universe.push_back(newClient);
		
		printf("\nNEW USER: %s\n", newClient->username.c_str());
		printf("- IP Addr - %s\n", inet_ntoa(newClient->portR.sin_addr));
		printf("- PORT R - %s\n", std::to_string(ntohs(newClient->portR.sin_port)).c_str());
		printf("- PORT L - %s\n", std::to_string(ntohs(newClient->portL.sin_port)).c_str());
		printf("- PORT Q - %s\n\n", std::to_string(ntohs(newClient->portQ.sin_port)).c_str());
		
		return true;
	}
	else 
	{
		printf("\nINVALID REGISTER COMMAND : NO ACTIONS TAKEN\n\n");
		
		return false;
	}

}

int main(int argc, char *argv[])
{
    int sock;                        /* Socket */
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned int cliAddrLen;         /* Length of incoming message */
    char echoBuffer[ECHOMAX];        /* Buffer for echo string */
    char inputCommand[ECHOMAX];
    unsigned short echoServPort;     /* Server port */
    int recvMsgSize;                 /* Size of received message */
	bool dhtExists = false;
	bool waiting = false;
	char leavingUser[64];
	
    if (argc != 2)         /* Test for correct number of parameters */
    {
        fprintf(stderr,"Usage:  %s <UDP SERVER PORT>\n", argv[0]);
        exit(1);
    }

    echoServPort = atoi(argv[1]);  /* First arg:  local port */

    /* Create socket for sending/receiving datagrams */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort);      /* Local port */

    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed");

    while(true) /* Run forever */
    {
        /* Set the size of the in-out parameter */
        cliAddrLen = sizeof(echoClntAddr);

        /* Block until receive message from a client */
        if ((recvMsgSize = recvfrom(sock, echoBuffer, ECHOMAX, 0, (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
            DieWithError("recvfrom() failed");

	echoBuffer[ recvMsgSize ] = '\0';

        printf("----- NEW COMMAND RECEIVED -----\nServer handling client %s\n", inet_ntoa( echoClntAddr.sin_addr ));
        printf("Server receives string: %s\n", echoBuffer );

	/* Get input command into an array of words */
	strcpy(inputCommand, echoBuffer);
	char* word[64];
	word[0] = strtok(inputCommand, " ");
	for (int i = 1; i < sizeof(strtok(NULL, " ")); i++)
		word[i] = strtok (NULL, " ");

	if (!waiting) {
		/* WHEN THE SERVER GETS A REGISTER COMMAND */
		if (strcmp(word[0], "register") == 0) {
			bool added = registerUser(word, echoServPort);

			if (added)
				strcat(echoBuffer," - SUCCESS");
			else
				strcat(echoBuffer," - FAILURE");

			if (sendto(sock, echoBuffer, strlen(echoBuffer), 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != strlen(echoBuffer))
		    		DieWithError("sendto() sent a different number of bytes than expected");
		}

		/* WHEN THE SERVER GETS A SETUP-DHT COMMAND */
		else if (strcmp(word[0], "setup-dht") == 0) {

			bool success = false;

			for(std::vector<registeredClient*>::iterator itr = Universe.begin(); itr != Universe.end(); ++itr) {
				if (strcmp((*itr)->username.c_str(), word[2]) == 0)
					success = true;
			}

			if (atoi(word[1]) < 2)
				success = false;

			if (atoi(word[1]) > Universe.size())
				success = false;

			if (dhtExists)
				success = false;


			if (success) {
				strcat(echoBuffer, " - SUCCESS \n\n- username -\t- ip addr -\t- ports (R, L, Q) - \n");
				
				/* TO PUT LEADER ON TOP */
				for(std::vector<registeredClient*>::iterator itr = Universe.begin(); itr != Universe.end(); ++itr) {
					if (strcmp((*itr)->username.c_str(), word[2]) == 0) {

						(*itr)->state = "l";
						(*itr)->id = 0;
						(*itr)->ringSize = atoi(word[1]);

						strcat(echoBuffer, (*itr)->username.c_str());
						strcat(echoBuffer, "\t\t ");
						strcat(echoBuffer, inet_ntoa((*itr)->portR.sin_addr));
						strcat(echoBuffer, "\t ");
						strcat(echoBuffer, std::to_string(ntohs((*itr)->portR.sin_port)).c_str());
						strcat(echoBuffer, " ");
						strcat(echoBuffer, std::to_string(ntohs((*itr)->portL.sin_port)).c_str());
						strcat(echoBuffer, " ");
						strcat(echoBuffer, std::to_string(ntohs((*itr)->portQ.sin_port)).c_str());
						strcat(echoBuffer, " \n");
					}
				}
			
				/* TO FILL IN REST OF DHT WITH RANDOM USERS FROM UNIVERSE */
				for (int count = 1; count < atoi(word[1]); ) {
					
					srand(time(NULL));
					int loc = rand() % Universe.size();

					if (strcmp(Universe.at(loc)->username.c_str(), word[2]) != 0 && strcmp(Universe.at(loc)->state.c_str(), "f") == 0) {
						
						Universe.at(loc)->ringSize = atoi(word[1]);
						Universe.at(loc)->state = "i";
						Universe.at(loc)->id = count;

						strcat(echoBuffer, Universe.at(loc)->username.c_str());
						strcat(echoBuffer, "\t\t ");
						strcat(echoBuffer, inet_ntoa(Universe.at(loc)->portR.sin_addr));
						strcat(echoBuffer, "\t ");
						strcat(echoBuffer, std::to_string(ntohs(Universe.at(loc)->portR.sin_port)).c_str());
						strcat(echoBuffer, " ");
						strcat(echoBuffer, std::to_string(ntohs(Universe.at(loc)->portL.sin_port)).c_str());
						strcat(echoBuffer, " ");
						strcat(echoBuffer, std::to_string(ntohs(Universe.at(loc)->portQ.sin_port)).c_str());
						strcat(echoBuffer, " \n");

						count++;
					}
				}
				printf("\nCLIENTS CURRENTLY STORING DATA, SERVER WAITING FOR \"dht-complete <user>\" COMMAND\n\n");
				waiting = true;
			}
			else
			{
				printf("\nINVALID SETUP-DHT COMMAND : NO ACTIONS TAKEN\n\n");
				strcat(echoBuffer, " - FAILURE");
			}

			if (sendto(sock, echoBuffer, strlen(echoBuffer), 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != strlen(echoBuffer))
		    		DieWithError("sendto() sent a different number of bytes than expected");
		}
		
		else if (strcmp(word[0], "teardown-dht") == 0)
		{
			bool success = false;
			

			for(std::vector<registeredClient*>::iterator itr = Universe.begin(); itr != Universe.end(); ++itr) 
			{
				if(strcmp((*itr)->username.c_str(), word[1]) == 0)
				{
					if (strcmp((*itr)->state.c_str(), "l") == 0)
						success = true;
				}
			}

		
			if (success)
			{
				strcat(echoBuffer," - SUCCESS");
				printf("\nCLIENTS CURRENTLY REMOVING DATA, SERVER WAITING FOR \"teardown-complete <user>\" COMMAND\n\n");
				waiting = true;
			}
			else
			{
				strcat(echoBuffer," - FAILURE");
				printf("\nINVALID TEARDOWN-DHT COMMAND : NO ACTIONS TAKEN\n\n");
			}

			if (sendto(sock, echoBuffer, strlen(echoBuffer), 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != strlen(echoBuffer))
	    			DieWithError("sendto() sent a different number of bytes than expected");

		}
		
		else if (strcmp(word[0], "leave-dht") == 0)
		{
			bool success = false;
			int original_size = 0;

			for(std::vector<registeredClient*>::iterator itr = Universe.begin(); itr != Universe.end(); ++itr) 
			{
				if(strcmp((*itr)->username.c_str(), word[1]) == 0)
				{
					if (strcmp((*itr)->state.c_str(), "f") != 0)
					{
						success = true;
						original_size = (*itr)->ringSize;
						strcpy(leavingUser, (*itr)->username.c_str());
					}
				}
			}
			
			if (original_size - 1 < 2)
				success = false;

		
			if (success)
			{
				
				strcat(echoBuffer, " - SUCCESS \n\n- username -\t- ip addr -\t- ports (R, L, Q) - \n");
				
				/* TO PUT LEADER ON TOP */
				int count = 0;
				for(std::vector<registeredClient*>::iterator itr = Universe.begin(); itr != Universe.end(); ++itr) {
					if (strcmp((*itr)->username.c_str(), word[1]) != 0 && strcmp((*itr)->state.c_str(), "f") != 0) {

						(*itr)->id = count;
						
						strcat(echoBuffer, (*itr)->username.c_str());
						strcat(echoBuffer, "\t\t ");
						strcat(echoBuffer, inet_ntoa((*itr)->portR.sin_addr));
						strcat(echoBuffer, "\t ");
						strcat(echoBuffer, std::to_string(ntohs((*itr)->portR.sin_port)).c_str());
						strcat(echoBuffer, " ");
						strcat(echoBuffer, std::to_string(ntohs((*itr)->portL.sin_port)).c_str());
						strcat(echoBuffer, " ");
						strcat(echoBuffer, std::to_string(ntohs((*itr)->portQ.sin_port)).c_str());
						strcat(echoBuffer, " \n");
						
						count++;
					}
				}
				printf("\nCLIENTS CURRENTLY REDISTRIBUTING DATA, SERVER WAITING FOR \"dht-rebuilt <user> <new leader>\" COMMAND\n\n");
				waiting = true;
			}
			else
			{
				strcat(echoBuffer," - FAILURE");
				printf("\nINVALID LEAVE-DHT COMMAND : NO ACTIONS TAKEN\n\n");
			}

			if (sendto(sock, echoBuffer, strlen(echoBuffer), 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != strlen(echoBuffer))
	    			DieWithError("sendto() sent a different number of bytes than expected");

		}
		
		else if (strcmp(word[0], "query-dht") == 0)
		{
			bool success = false;
			
			for(std::vector<registeredClient*>::iterator itr = Universe.begin(); itr != Universe.end(); ++itr) 
			{
				if(strcmp((*itr)->username.c_str(), word[1]) == 0)
				{
					if (strcmp((*itr)->state.c_str(), "f") == 0)
						success = true;
				}
			}
			
			if (!dhtExists)
				success = false;
			
			int loc = 0;
			if (success)
			{
				int i = 0;
				strcat(echoBuffer, " - SUCCESS \n\n- username -\t- ip addr -\t- ports (R, L, Q) - \n");
				for(std::vector<registeredClient*>::iterator itr = Universe.begin(); itr != Universe.end(); ++itr) 
				{
					if (strcmp((*itr)->state.c_str(), "f") != 0)
						loc = i;
					i++;
				}
				
				strcat(echoBuffer, Universe.at(loc)->username.c_str());
				strcat(echoBuffer, "\t\t ");
				strcat(echoBuffer, inet_ntoa(Universe.at(loc)->portR.sin_addr));
				strcat(echoBuffer, "\t ");
				strcat(echoBuffer, std::to_string(ntohs(Universe.at(loc)->portR.sin_port)).c_str());
				strcat(echoBuffer, " ");
				strcat(echoBuffer, std::to_string(ntohs(Universe.at(loc)->portL.sin_port)).c_str());
				strcat(echoBuffer, " ");
				strcat(echoBuffer, std::to_string(ntohs(Universe.at(loc)->portQ.sin_port)).c_str());
				
				printf("\nCLIENT REQUESTING TO QUERY-DHT, RESPONDING WITH USER DATA\n\n");
			}
			
			else
			{
				strcat(echoBuffer," - FAILURE");
				printf("\nINVALID QUERY-DHT COMMAND : NO ACTIONS TAKEN\n\n");
			}

			if (sendto(sock, echoBuffer, strlen(echoBuffer), 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != strlen(echoBuffer))
	    			DieWithError("sendto() sent a different number of bytes than expected");
		}

		//deregister stiil need work
		else if (strcmp(word[0], "deregister") == 0)
		{
			
			bool success = false;
			std::vector<registeredClient*>::iterator itr;
			int placeHold;
			int i = 0;

			for(itr = Universe.begin(); itr != Universe.end(); ++itr) 
			{
				if(strcmp((*itr)->username.c_str(), word[1]) == 0)
				{
					if (strcmp((*itr)->state.c_str(), "l") == 0)
						success = false;
					
					else if(strcmp((*itr)->state.c_str(), "i") == 0)
						success = false;
						
					else
					{
						success = true;

						placeHold = i;
					}
				}

				i++;

			}


			if (success)
			{
				strcat(echoBuffer," - SUCCESS");				
				Universe.erase(Universe.begin() + placeHold);
				
				printf("\nSERVER DEREGISTERED USER : %s\n\n", word[1]);
			}
						
			else
			{
				strcat(echoBuffer," - FAILURE");
				printf("\nINVALID DEREGISTER COMMAND : NO ACTIONS TAKEN\n\n");
			}

			if (sendto(sock, echoBuffer, strlen(echoBuffer), 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != strlen(echoBuffer))
	    			DieWithError("sendto() sent a different number of bytes than expected");

		}
		
		/* DEFAULT CASE TO SEND MESSAGE BACK WHEN NOT WAITING*/
		else {
			strcpy(echoBuffer, "\nERROR: INVALID COMMAND\n");
			if (sendto(sock, echoBuffer, strlen(echoBuffer), 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != strlen(echoBuffer))
		    		DieWithError("sendto() sent a different number of bytes than expected");
		}

		
	    }
	    
	    /* IF THE SERVER IS WAITING FOR A COMPLETE COMMAND */
	    else
	    {
	    	if (strcmp(word[0], "dht-complete") == 0) {
	    		bool isLeader = false;
	    		for(std::vector<registeredClient*>::iterator itr = Universe.begin(); itr != Universe.end(); ++itr) {
				if (strcmp((*itr)->username.c_str(), word[1]) == 0) {
					if (strcmp((*itr)->state.c_str(), "l") == 0)
						isLeader = true;
				}
			}
			if (isLeader) {
				strcat(echoBuffer," - SUCCESS");
				waiting = false;
				dhtExists = true;
				printf("\nSERVER RECEIVED \"dht-complete <user>\" COMMAND, ABLE TO RECEIVE NEW COMMANDS\n\n");
			}
			else
			{
				strcat(echoBuffer," - FAILURE");
				printf("\nINVALID DHT-COMPLETE COMMAND : STILL WAITING FOR \"dht-complete <user>\" COMMAND\n\n");
			}

			if (sendto(sock, echoBuffer, strlen(echoBuffer), 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != strlen(echoBuffer))
		    		DieWithError("sendto() sent a different number of bytes than expected");
	    	}
	    	else if (strcmp(word[0], "teardown-complete") == 0) {

			bool success = false;

			for(std::vector<registeredClient*>::iterator itr = Universe.begin(); itr != Universe.end(); ++itr) 
			{
				if(strcmp((*itr)->username.c_str(), word[1]) == 0)
				{
					if (strcmp((*itr)->state.c_str(), "l") == 0)
						success = true;
				}
			}

			if (success)
			{
				for(std::vector<registeredClient*>::iterator itr = Universe.begin(); itr != Universe.end(); ++itr) 
				{
					(*itr)->state = "f";
					(*itr)->id = 9999;
					(*itr)->ringSize = 9999;
				}
				
				strcat(echoBuffer," - SUCCESS");
				
				printf("\nSERVER RECEIVED \"teardown-complete <user>\" COMMAND, ABLE TO RECEIVE NEW COMMANDS\n\n");
			
				dhtExists = false;
				waiting = false;
			}
			else
			{
				strcat(echoBuffer," - FAILURE");
				printf("\nINVALID TEARDOWN-COMPLETE COMMAND : STILL WAITING FOR \"teardown-complete <user>\" COMMAND\n\n");
			}

			if (sendto(sock, echoBuffer, strlen(echoBuffer), 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != strlen(echoBuffer))
	    			DieWithError("sendto() sent a different number of bytes than expected");
		}
		else if (strcmp(word[0], "dht-rebuilt") == 0) {

			bool success = false;
			for(std::vector<registeredClient*>::iterator itr = Universe.begin(); itr != Universe.end(); ++itr)
			{
				if (strcmp((*itr)->username.c_str(), word[2]) == 0)
				{
					if ((*itr)->id == 0)
						success = true;
				}
			}
			if (strcmp(word[1], leavingUser) != 0)
				success = false;

			if (success)
			{
				for(std::vector<registeredClient*>::iterator itr = Universe.begin(); itr != Universe.end(); ++itr) 
				{
					if (strcmp((*itr)->state.c_str(), "f") != 0)
					{
						if (strcmp((*itr)->username.c_str(), word[2]) == 0)
						{
							(*itr)->state = "l";
							(*itr)->ringSize = (*itr)->ringSize - 1;
						}	
							
						else if (strcmp((*itr)->username.c_str(), word[1]) == 0)
						{
							(*itr)->state = "f";
							(*itr)->ringSize = 9999;
							(*itr)->id = 9999;
						}
						
						else
						{
							(*itr)->state = "i";
							(*itr)->ringSize = (*itr)->ringSize - 1;
						}
					}
				}
				
				strcat(echoBuffer," - SUCCESS");
				
				printf("\nSERVER RECEIVED \"dht-rebuilt <user> <new leader>\" COMMAND, ABLE TO RECEIVE NEW COMMANDS\n\n");
			
				dhtExists = true;
				waiting = false;
			}
			else
			{
				strcat(echoBuffer," - FAILURE");
				printf("\nINVALID DHT-REBUILT COMMAND : STILL WAITING FOR \"dht-rebuilt <user> <new leader>\" COMMAND\n\n");
			}

			if (sendto(sock, echoBuffer, strlen(echoBuffer), 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != strlen(echoBuffer))
	    			DieWithError("sendto() sent a different number of bytes than expected");
		}
	    	else {
	    		printf("\nERROR : SERVER WAITING FOR <complete> COMMAND\n\n");
			strcpy(echoBuffer, "ERROR: SERVER IS WAITING\n");
			if (sendto(sock, echoBuffer, strlen(echoBuffer), 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != strlen(echoBuffer))
		    		DieWithError("sendto() sent a different number of bytes than expected");
		}
	    }
	    /* STILL IN LOOP */
    }
    /* NOT REACHED */
}
