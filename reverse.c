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



/*
Per Prinzip kann man diese Funktion als gegeben 
betrachten. 
*/
void reverse_string(char *str){
    int len = strlen(str);
    for(int i = 0; i < len/2; i++){
        char temp = str[i];
        str[i] = str[len - i -1];
        str[len -i - 1] = temp;
    }
}

/*
Dieses Programm kann als gegeben angesehen werden. 
Im Programmiertest von 2023 wo fork und exec vorkamen,
war es auch so, dass man ein unbekanntes Programm 
aufrufen musste.
 */
int main(){
    char *message = NULL;
    size_t cap = 0;
    fprintf(stderr, "Child started\n");
    if(getline(&message, &cap, stdin) == -1){
        fprintf(stderr, "ERROR");
        free(message);
        exit(EXIT_FAILURE);
    }
    fprintf(stderr,"%s\n", message);
    if(message[strlen(message)-1] == '\n'){
        message[strlen(message)-1 ] = '\0';
    }

    reverse_string(message);
    fprintf(stdout, "%s\n", message);
    free(message);
    exit(EXIT_SUCCESS);
}