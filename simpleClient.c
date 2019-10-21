#include "simpleClient.h"
#include "globals.h"

//gcc simpleClient.c -o simpleClient

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        printf("Usage: ./simpleClient (port) (number_of_words)\r\nex. ./simpleClient 1337 5");
        return -1;
    }
    
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    char buf[BUFSIZ] = {0};
    int bytes_received;
    int i;
    
    char port[25] = {0};
    char word[MAX_WORD_LEN] = {0};
    strcpy(port,argv[1]);
    int num_words = atoi(argv[2]);
    
    if (atoi(port) <= 1024 || atoi(port) > 65535 || num_words == 0)
    {
        printf("Invalid argument supplied. Please check usage.");
        return -1;
    }
    
    struct addrinfo hints, *pinfo;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; //IPV4
    hints.ai_socktype = SOCK_STREAM; //TCP
    
    if (getaddrinfo("localhost",port,&hints,&pinfo) != 0) //port passed as string
        printf("getaddrinfo() error");
    
    if (connect(s,pinfo->ai_addr,pinfo->ai_addrlen) != 0) //sockaddr needed for connect() supplied by getaddrinfo in the pinfo struct pointer
        printf("connect() error");
    
    //First thing client does - send number of words it's going to send
    if (send(s, &num_words, sizeof(int), 0) != sizeof(int))
    {
        printf("send() failed to send the number of words");
        return -1;
    }
    
    //Receive the go ahead message ("GOAHEAD")
    bytes_received = recv(s, buf, BUFSIZ, 0);
    if (bytes_received <= 0)
    {
        printf("recv() failed or found nothing");
        return 1;
    }
    
    printf("Server: %s\r\n",buf);
    if (strcmp(buf, "GOAHEAD") != 0)
        return -1;
    
    for (i=0;i<num_words;++i)
    {
        while (strlen(word) < 1) //Make sure they're sending SOMETHING
        {
            printf("Word %d: ",i+1);
            fgets(word, MAX_WORD_LEN, stdin);
        }
        
        if (word[0] == (char)27)
        {
            send(s, END, sizeof(char)*(strlen(END)+1), 0);
            recv(s, buf, BUFSIZ, 0);
            break;
        }
        
        if (word[strlen(word)-1] == '\n')
            word[strlen(word)-1] = '\0';
        
        //send the word and receive the answer from the server
        send(s, word, sizeof(char)*(strlen(word)+1), 0);
        buf[recv(s, buf, BUFSIZ, 0)] = '\0';
        
        //Server will send either "OK" if it found the word in the dictionary or "MISSPELLED" if it didn't
        if (strcmp(buf, OK) == 0)
            printf("[+] Word \"%s\" was spelled correctly and is in the dictionary.\r\n", word);
        else if (strcmp(buf, MISSPELLED) == 0)
            printf("[-] Word \"%s\" was spelled incorrectly or is not in the dictionary.\r\n", word);
        else
            printf("[O] Server: %s\r\n",word);
        
        memset(word,0,sizeof(char)*MAX_WORD_LEN);
    }
    
    close(s); //we're done. close socket
    
}
