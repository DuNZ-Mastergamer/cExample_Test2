#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include <signal.h>

#define READ_END 0
#define WRITE_END 1


typedef struct {
    pid_t pid;
    int parent_to_child[2];
    int child_to_parent[2];
} child_t;

char *message = NULL;
char *progName = "<not set yet>";
void error(char *msg){
    fprintf(stderr, "%s: %s\r\n", progName, msg);
    exit(EXIT_FAILURE);
}

/*
Methode stellt den ersten Task dar, wo auf der
Server-Seite der Socket aufgemacht werden soll.
Dabei ist es wichtig, dass maximal 1 gleichzeitige
Verbindungen angenommen wird. Weiters soll auch
die Adresse wiederverwendet werden. Ein erneuter
Server Aufruf darf keine Probleme bereiten.
*/

int setSocket(char *port){
    //Erstellen der Adresse braucht include netdb.h
    struct addrinfo hints, *ai;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; //Braucht man nur am Server

    //Adressinformationen claimen
    int res =getaddrinfo(NULL, port, &hints, &ai);
    if(res != 0){error("Failed to get address information");}

    //Den Socket erstellen
    int sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if(sockfd <0){error("failed to create socket");}

    //Adresse wiederverwendbar machen
    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    //Den Socket binden
    if(bind(sockfd, ai->ai_addr,ai->ai_addrlen) < 0){
        freeaddrinfo(ai);
        close(sockfd);
        error("failed to bind socket");
    }

    //Am Socket lauschen
    if(listen(sockfd, 3) < 0){
        freeaddrinfo(ai);
        close(sockfd);
        error("failed to listen");
    }


    //Adressinformation freigeben
    freeaddrinfo(ai);

    return sockfd;
}

/*
Diese Methode soll als Task 2 angesehen werden.
Sie soll die Pipes zur Kommunikation mit den
Kindprozessen erstellen und den Prozess 
reverse.c aufrufen. Diesem werden keine 
Argumente mitgegeben. Er bekommst seinen
Input via stdin und schreibt auf stdout 
in die Pipe, das Resultat.
*/

void creatingChild(child_t *child){
   
    //Pipes erstellen
    if(pipe(child->parent_to_child) == -1){
        error("pipe failure");
    }
    if(pipe(child->child_to_parent) == -1){
        close(child->parent_to_child[READ_END]);
        close(child->parent_to_child[WRITE_END]);
        error("second pipe failure");
    }

    //Kindprozess erstellen
    child->pid = fork();
    if(child->pid == -1){
        error("fork failure");
    }

    switch(child->pid){
        case 0:
            close(child->parent_to_child[WRITE_END]);//Schreib-Ende vom Elternprozess zumachen
            close(child->child_to_parent[READ_END]);//Lese-Ende des Elternprozesses zumachen
            
            //Umleitungen vornehmen
            if(dup2(child->parent_to_child[READ_END], STDIN_FILENO) == -1){
                error("Redirecting of pipe failed");
            }
            if(dup2(child->child_to_parent[WRITE_END], STDOUT_FILENO) == -1){
                error("Redirecting of second pipe failed");
            }

            //Den Prozess executen
            if(execlp("./reverse", "reverse", NULL) == -1){
                error("Failed to execute reverse.c");
            }
            break;
        default:
            //Die Enden der andern Pipes zumachen
            close(child->parent_to_child[READ_END]);
            close(child->child_to_parent[WRITE_END]);
    }
}

/*
Diese Methode soll als Task 3 gesehen werden.
Sie soll die Nachricht via Socket empfangen,
durch die Pipe an den Kindprozess weitergeben.
Vom Kindprozess bekommt der server die erhaltene
Nachricht reverse wieder und schickt diese dann
letztendlich noch an den client.
*/
void processMessage(int connfd, child_t *child){
     FILE *connFile = fdopen(connfd, "r+");
    //Nachricht des Client lesen
    char *line = NULL;
    size_t cap = 0;
    if(getline(&line, &cap, connFile) == -1){
        fprintf(stderr, "failed to get from socket\n");
    }

    //Nachricht an den Kindprozess schicken.
    if(write(child->parent_to_child[WRITE_END], line, strlen(line)) < 0){
        error("error parsing to child");
    }

    int status = 0;
    waitpid(child->pid, &status, 0);
    if(WEXITSTATUS(status) == EXIT_FAILURE){
        close(child->child_to_parent[0]);
        close(child->child_to_parent[1]);
        fprintf(stderr, "Child error");
    }

    
    //Nachricht von Kindprozess annehmen
    char response[256];
    ssize_t bytes_read = read(child->child_to_parent[0], response, 255);
    if(bytes_read < 0){
        error("failed to get reversed");
    }
    response[bytes_read]='\0';


    //Ergebnis an Client schicken
     //Senden der Nachricht
    
    fprintf(connFile, "%s\n", response);
    fflush(connFile); //notwendig damit abgeschickt wird.

}


int main(int argc, char **argv){

    child_t child;
    //Basisprüfung;
    progName = argv[0];
    if(argc < 3 || argc > 3){
        error("Invalid amount of arguments");
    }

    //Socket erstellen
    int sockfd = setSocket(argv[2]);
    int connfd = accept(sockfd, NULL, NULL);
    if(connfd < 0){error("failed to accept");}
    creatingChild(&child);

    processMessage(connfd, &child);

    exit(EXIT_SUCCESS);
}