#include <stdlib.h>
#include <pthread.h>
#include <sys/stat.h>

#include "simpleServer.h"
#include "globals.h"

//gcc simpleServer.c connectionQueue.c solvedSpellList.c logQueue.c spellQueue.c open_listenfd.c -o server -lpthread

//undefined reference to pthread_*
//you didn't include the pthread library with the linker parameter -lpthread

//An extremely simple server that connects to a given port.
//Once the server is connected to the port, it will listen on that port
//for a user connection.
//A user will connect through telnet, the user also needs to know the port number.
//If the user connects successfully, a socket descriptor will be created.
//The socket descriptor works exactly like a file descriptor, allowing us to read/write on it.
//The server will then use the socket descriptor to communicate with the user, sending and
//receiving messages.

int main(int argc, char** argv)
{
    int i = 0;
    
    if(argc == 1){
        printf("No port number entered.\n");
        return -1;
    }
    
    //sockaddr_in holds information about the user connection.
    //We don't need it, but it needs to be passed into accept().
    struct sockaddr_in client;
    unsigned int clientLen = sizeof(client);
    
    int connectionPort = DEFAULT_PORT;
    char dictionary[MAX_PATH];
    
    int connectionSocket, clientSocket;
    
    strcpy(dictionary, DEFAULT_DICTIONARY);
    
    for (i=1;i<argc;++i)
    {
        if (strstr(argv[i],"-") == argv[i])
        {
            if (strcmp(argv[i],"-d") == 0 && strstr(argv[i+1], "-") != argv[i+1])
            {
                if (argc >= (i+2))
                {
                    strcpy(dictionary,argv[i+1]);
                }
                else
                {
                    printf("No dictionary supplied but argument (-d) supplied.");
                    return -1;
                }
            }
            else if (strstr(argv[i+1], "-") == argv[i+1])
            {
                printf("No dictionary supplied. Other argument passed");
                return -1;
            }
            
            if (strcmp(argv[i], "-p") == 0 && strstr(argv[i+1], "-") != argv[i+1])
            {
                if (argc >= (i+2))
                {
                    //We can't use ports below 1024 and ports above 65535 don't exist.
                    if (atoi(argv[i+1]) > 1024 && atoi(argv[i+1]) <= 65535)
                    {
                        connectionPort = atoi(argv[i+1]);
                    }
                    else
                    {
                        printf("No valid port could be found");
                        return -1;
                    }
                }
                else
                {
                    printf("No port supplied but argument (-p) supplied.");
                    return -1;
                }
            }
            else if (strstr(argv[i+1], "-") == argv[i+1])
            {
                printf("No port supplied. Other argument passed");
                return -1;
            }
        }
    }
    
    //Load the dictionary
    struct stat dictFileInfo;
    char** lpDictionary = &(Queues.lpDictionary);
    stat(dictionary,&dictFileInfo);
    *lpDictionary = malloc(sizeof(char)*dictFileInfo.st_size+1); //use the size of the file to determine the size of our dictionary char pointer
    FILE* dictFile = fopen(dictionary,"r");
    fread(*lpDictionary,dictFileInfo.st_size,1,dictFile);
    fclose(dictFile);
    
    //Does all the hard work for us.
    connectionSocket = open_listenfd(connectionPort);
    if(connectionSocket == -1){
        printf("Could not connect to %s, maybe try another port number?\n", argv[1]);
        return -1;
    }
    
    //set up our logging Queue
    //...//
    initLogQueue(&Queues.logQueueInst, "./log.txt");
    
    //set up our spell checker Queue
    //...//
    initSpellQueue(&Queues.spellCheckQueue);
    
    //set up our connection Queue
    //...//
    initConnectionQueue(&Queues.connectionQueueInst);
    
    //set up our solved word List
    //...//
    initSolvedList(&Queues.solvedSpellCheckList);
    
    //make our worker threads for spell checking and logging
    //pthreads for the first time in 7 years. should be fun.
    //...//
    pthread_t spellQueueWorkerThread;
    pthread_create(&spellQueueWorkerThread, NULL, processSpellQueue, &Queues);
    
    pthread_t logQueueWorkerThread;
    pthread_create(&logQueueWorkerThread, NULL, processLogQueue, &Queues);
    
    pthread_t connQueueWorkerThread[NUM_THREADS];
    for (i=0;i<NUM_THREADS;++i)
        pthread_create(&connQueueWorkerThread[i],NULL,processConnectionQueue, &Queues);
    
    //check our data
    
    while (1)
    {
        //accept() waits until a user connects to the server, writing information about that server
        //into the sockaddr_in client.
        //If the connection is successful, we obtain A SECOND socket descriptor.
        //There are two socket descriptors being used now:
        //One by the server to listen for incoming connections.
        //The second that was just created that will be used to communicate with
        //the connected user.
        if((clientSocket = accept(connectionSocket, (struct sockaddr*)&client, &clientLen)) == -1){
            printf("Error connecting to client.\n");
            return -1;
        }
        
        //put clientSocket in connectionQueue
        //...//
        connAddToQueue(&Queues.connectionQueueInst, clientSocket);
        
        printf("Connection success!\n");
    }
    
    free(lpDictionary);
    
    return 0;
}
