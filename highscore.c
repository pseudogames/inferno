#include "highscore.h"

#define MAXLEN 1024

void ping_pseudogames() {

    IPaddress ip;
    TCPsocket tcpsock;

    if(SDLNet_ResolveHost(&ip,"localhost",9999)==-1) {
        printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        return;
    }

    tcpsock=SDLNet_TCP_Open(&ip);
    if(!tcpsock) {
        printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
        return; 
    }

    int len,result;
    char *msg=PSEUDO_PING_MESSAGE;

    len=strlen(msg)+1; // add one for the terminating NULL
    result=SDLNet_TCP_Send(tcpsock,msg,len);
    if(result<len) {
        printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
    }
    
    if(tcpsock){
        SDLNet_TCP_Close(tcpsock);
    }

}

void send_highscore(Uint32 elapsed, Uint32 score, char* initials) {

    char msg[1024];

    IPaddress ip;
    TCPsocket tcpsock;

    if(SDLNet_ResolveHost(&ip,"localhost",9999)==-1) {
        printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        return;
    }

    tcpsock=SDLNet_TCP_Open(&ip);
    if(!tcpsock) {
        printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
        return; 
    }

    sprintf(msg, "scr:%u:%u:%s", elapsed, score, initials);
    

    int len,result;
    len=strlen(msg)+1; // add one for the terminating NULL
    result=SDLNet_TCP_Send(tcpsock,msg,len);
    if(result<len) {
        printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
    }
    
    if(tcpsock){
        SDLNet_TCP_Close(tcpsock);
    }
}

void get_highscore(char *msg){

    IPaddress ip;
    TCPsocket tcpsock;

    if(SDLNet_ResolveHost(&ip,"localhost",9999)==-1) {
        printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        return;
    }

    tcpsock=SDLNet_TCP_Open(&ip);
    if(!tcpsock) {
        printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
        return; 
    }

    char cmd[4] = "top";

    int len,result;
    len=strlen(cmd)+1; // add one for the terminating NULL
    result=SDLNet_TCP_Send(tcpsock,cmd,len);
    if(result<len) {
        printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
    }


    result=SDLNet_TCP_Recv(tcpsock,msg,MAXLEN);
    if(result<=0) {
        // An error may have occured, but sometimes you can just ignore it
        // It may be good to disconnect sock because it is likely invalid now.
        return NULL;
    }
    printf("Received: \"%s\"\n",msg);


    if(tcpsock){
        SDLNet_TCP_Close(tcpsock);
    }
}

