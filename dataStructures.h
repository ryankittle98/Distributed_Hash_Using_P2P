#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <string>
#include <vector>
#include <thread>
#include <iostream>
#include <fstream>


typedef struct registeredClient registeredClient;

class HashEntry {
private:
      int key;
      std::string line;
public:
      HashEntry(int new_key, char new_line[256]) {
            key = new_key;
            line = new_line;
      }
      int getKey() {
            return key;
      }
      std::string getValue() {
            return line;
      }
};

const int TABLE_SIZE = 353;
const int TABLE_SIZE2 = 32;
class localHashMap {
private:
      HashEntry* table[TABLE_SIZE][TABLE_SIZE2];
public:
      localHashMap() {
            for (int i = 0; i < TABLE_SIZE; i++)
            {
            	for (int j = 0; j < TABLE_SIZE2; j++)
			table[i][j] = NULL;
		}
      }

      std::string get(int key, char long_name[]) {
            int hash = 0;
            int loc = 9999;
            bool success = false;
            for (int i = 0; i < TABLE_SIZE2; i++)
            {
            	if (table[key][hash] != NULL)
            	{
		    	if (table[key][hash]->getValue().find(long_name) != std::string::npos)
			{
				    success = true;
				    loc = hash; // found
			}
			else
				hash = (hash + 1) % TABLE_SIZE2;
		}
		else
			hash = (hash + 1) % TABLE_SIZE2;
	    }
            
            if (!success)
            	return "hellyeah, damnbro, goodshit, ahfuck, whatthedicks, thatsassman";
            else
	    	return table[key][loc]->getValue();
      } 

      void put(int key, char line[]) {
            int hash = 0;
            while (table[key][hash] != NULL)
                  hash++;

            table[key][hash] = new HashEntry(key, line);
      }   

	void deleteTable()
	{
	    for (int i = 0; i < TABLE_SIZE; i++)
            	for (int j = 0; j < TABLE_SIZE2; j++)
			table[i][j] = NULL;
	}
};

struct registeredClient {
	std::string username;

	struct sockaddr_in portR;
	struct sockaddr_in portL;
	struct sockaddr_in portQ;
	
	struct sockaddr_in leftNeighbor;
	struct sockaddr_in rightNeighbor;
	
	localHashMap localTable;

	std::string state = "u";
	int id = 9999;
	int ringSize = 9999;
};

void DieWithError(const char *errorMessage) /* External error handling function */
{
    perror(errorMessage);
    exit(1);
}

