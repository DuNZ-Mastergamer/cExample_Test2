#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

char *progName = "<not set yet>";
void error(char *msg){
    fprintf(stderr, "%s: %s", progName, msg);
    exit(EXIT_FAILURE);
}

//Simuliert wie beim Test das parsing
typedef struct {
    char* port;
    char* host;
    char* message;

} argument_t;

/*
Diese Methode ist als Task 1 zu sehen, sollte ein Client kommen.
!Das Schema zu Fork und Pipe hab ich im Server. Es ist viel
wahrscheinlicher, dass ein Server zum Test kommt, da dieser
viel mehr Optionen bietet. (Keine Garantie drauf)
*/
/*
    Der Syntax zur Eingabe der Parameter lautet:
    ./client -h localhost -p 1200 -m "Nachricht"
    Es muss mindestens die Nachricht vorhanden sein, 
    alle anderen Parameter sind nicht verpflichtend.
    Im Fall, dass nur -m vorhanden ist, soll der port
    8080 und der Host der localhost sein. Dies ist durch
    die Initialisierung bereits geschehen.
*/
void parse_arguments(int argc, char **argv, argument_t *args){ //ist als task 1 zu betrachten.
    progName = argv[0];
    if(argc < 3 || argc > 7){
        error("invalid amount of arguments");
    }
    int hflag = 0;
    int pflag = 0;
    int mflag = 0;

    int opt;
    while((opt = getopt(argc, argv, "h:p:m:")) != -1){
        switch(opt){
            case 'h':
                if(hflag != 0){error("h was already set!");}
                args->host = optarg;
                hflag++;
                break;
            case 'p':
                if(pflag != 0){error("p was already set!");}
                args->port = optarg;
                pflag++;
                break;
            case 'm':
                if(mflag != 0){error("m was already set!");}
                args->message = optarg;
                mflag++;
                break;
            case '?':
                error("invalid argument");
            default:
                error("unknown input");
        }
    }
    if(mflag == 0){error("no message declaired");}



}

/*
Soll als task 2 betrachtet werden. Idee ist den FileDescriptor 
returned, der den Socket abbildet.
*/

int setSocket(argument_t *args){
    //Erstellen der Adresse braucht include netdb.h
    struct addrinfo hints, *ai;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    //Adressinformationen claimen
    int res =getaddrinfo(args->host, args->port, &hints, &ai);
    if(res != 0){error("Failed to get address information");}

    //Den Socket erstellen
    int sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if(sockfd <0){error("failed to create socket");}

   //Connect the socket
    if(connect(sockfd, ai->ai_addr, ai->ai_addrlen) >0){
        freeaddrinfo(ai);
        close(sockfd);
        error("failed to connect");
    }

    //Adressinformation freigeben
    freeaddrinfo(ai);

    return sockfd;
}

/*
Soll den Task 3 darstellen, welcher sich durch
den Socket mit dem Server verbindet und seine 
Message abschickt. Der client wartet danach auf
die Antwort und printet diese aus.
(Die Antwort ist die message nur reverse!)
 */
void sendAndReceive(int sockfd,argument_t *args){
    //Die Datei laden
    FILE *sockFile = fdopen(sockfd, "r+");
    if(sockFile == NULL){
        close(sockfd);
        fclose(sockFile);
        error("failed to open socket file");
    }

    //Senden der Nachricht
    fprintf(stderr, "%s\n", args->message);
    fprintf(sockFile, "%s\n", args->message);
    fflush(sockFile); //notwendig damit abgeschickt wird.

    char *answere = NULL;
    size_t cap = 0;
    if(getline(&answere, &cap, sockFile) == -1){
        error("error fetching answere!");
    }
    fprintf(stderr,"Server: %s", answere);
    free(answere);

    
}


int main(int argc, char **argv){

    argument_t args = {"8080","localhost", ""};
    //handling the arguments (task 1)
    parse_arguments(argc, argv, &args);

    //setup the socket (task 2)
    int sockfd = setSocket(&args);

    //connect and communicate (task 3)
    sendAndReceive(sockfd, &args);


    exit(EXIT_SUCCESS);
}