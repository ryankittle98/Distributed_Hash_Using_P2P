
-- TO COMPILE BOTH FILES, JUST TYPE "make" AND THE MAKEFILE WILL DO IT FOR YOU --

-- TO START SERVER, TYPE ./Server <port> --
-- TO START CLIENT, TYPE ./Client <server ip> <server port> --

Here is a list of commands and how to execute them on a client:
- Register : register <username> <port> <port> <port>
- Setup DHT : setup-dht <ring size> <username>
- Complete DHT Setup : dht-complete <username>
- Query DHT : query-dht <username>
- Leave DHT : leave-dht <username>
- Rebuild DHT : dht-rebuilt <username> <new leader>
- Deregister : deregister <username>
- Teardown DHT : teardown-dht <username>
- Complete Teardown : teardown-complete <username>
