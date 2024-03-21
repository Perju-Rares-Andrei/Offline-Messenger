#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

int main(int argc, char *argv[])
{
    int sd;                    // descriptorul de socket
    struct sockaddr_in server; // structura folosita pentru conectare
                                // mesajul trimis
    int nr = 0;
    char rmsg[3000] ,wmsg[3000];

    /* exista toate argumentele in linia de comanda? */
    if (argc != 3)
    {
        printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }

    /* stabilim portul */
    port = atoi(argv[2]);

    /* cream socketul */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Eroare la socket().\n");
        return errno;
    }

    /* umplem structura folosita pentru realizarea conexiunii cu serverul */
    /* familia socket-ului */
    server.sin_family = AF_INET;
    /* adresa IP a serverului */
    server.sin_addr.s_addr = inet_addr(argv[1]);
    /* portul de conectare */
    server.sin_port = htons(port);

    /* ne conectam la server */
    if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[client]Eroare la connect().\n");
        return errno;
    }


    printf("OFFLINE MESSENGER\nComenzi:\n1.login\n2.new user\n3.quit\n");
    fflush(stdout);
    
    while(1){
        bzero(&rmsg,sizeof(rmsg));
        bzero(&wmsg,sizeof(wmsg));


        read(0, wmsg, sizeof(wmsg));//citesc mesajul de la consola 
        write(sd, wmsg, strlen(wmsg));//scriu mesajul in sd pentru al trimite catre server
        read(sd, rmsg, sizeof(rmsg));//citesc mesajul de la server
        printf("%s", rmsg);// Afisez mesajul in consola 

        if(strncmp(rmsg,"[server]:Program inchis\n",strlen("[server]:Program inchis\n"))==0){
            exit(0);
        }
        fflush(stdout);

    }
close(sd);/* inchidem conexiunea, am terminat */
}