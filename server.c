#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

/* portul folosit */
#define PORT 1234

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData
{
    int idThread; // id-ul thread-ului tinut in evidenta de acest program
    int cl;       // descriptorul intors de accept
} thData;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);// funcitia de prelucrare a mesajului

int main()
{
  struct sockaddr_in server; // structura folosita de server
  struct sockaddr_in from;
  int nr; // mesajul primit de trimis la client
  int sd; // descriptorul de socket
  int pid;
  pthread_t th[100]; // Identificatorii thread-urilor care se vor crea
  int i = 0;

  /* crearea unui socket */
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("[server]Eroare la socket().\n");
    return errno;
  }
  /* utilizarea optiunii SO_REUSEADDR */
  int on = 1;
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

  /* pregatirea structurilor de date */
  bzero(&server, sizeof(server));
  bzero(&from, sizeof(from));

  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
  server.sin_family = AF_INET;
  /* acceptam orice adresa */
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  /* utilizam un port utilizator */
  server.sin_port = htons(PORT);

  /* atasam socketul */
  if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[server]Eroare la bind().\n");
    return errno;
  }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen(sd, 2) == -1)
  {
    perror("[server]Eroare la listen().\n");
    return errno;
  }
  /* servim in mod concurent clientii...folosind thread-uri */
  while (1)
  {
    int client;
    thData *td; // parametru functia executata de thread
    socklen_t length = sizeof(from);

    printf("[server]Asteptam la portul %d...\n", PORT);
    fflush(stdout);

    /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
    if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0)
    {
      perror("[server]Eroare la accept().\n");
      continue;
    }

    /* s-a realizat conexiunea, se astepta mesajul */


    td = (struct thData *)malloc(sizeof(struct thData));
    td->idThread = i++;
    td->cl = client;

    pthread_create(&th[i], NULL, &treat, td);

  } // while
};
static void *treat(void *arg)
{
  struct thData tdL;
  tdL = *((struct thData *)arg);
  printf("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
  fflush(stdout);
  pthread_detach(pthread_self());

  raspunde((struct thData *)arg);

  /* am terminat cu acest client, inchidem conexiunea */

  close((intptr_t)arg);
  return (NULL);
};

void raspunde(void *arg)
{
  int nr=1, i = 0 ;

  char wmsg[3000],rmsg[3000];
  struct thData tdL;
  tdL = *((struct thData *)arg);

  char username[20] ,password[20];
  char user[20],pass[20];

  bzero(wmsg,sizeof(wmsg));
  bzero(&rmsg,sizeof(rmsg));

  int logat=0;
  while(1){

    bzero(wmsg,sizeof(wmsg));
    bzero(&rmsg,sizeof(rmsg));

    read(tdL.cl, rmsg, sizeof(rmsg));
    
    if (strncmp(rmsg,"login",5)==0)
    {
      if(logat==1){
        strncpy(wmsg,"[server]:Sunteti deja autentificat\n",strlen("[server]:Sunteti deja autentificat\n"));
        write(tdL.cl, wmsg, sizeof(wmsg));
        bzero(wmsg,sizeof(wmsg));
      }
      else
        {

          int ok=0;

          strncpy(wmsg,"[server]:username:",strlen("[server]:username:"));
          write(tdL.cl, wmsg, sizeof(wmsg));
          bzero(wmsg,sizeof(wmsg));

          bzero(&rmsg,sizeof(rmsg));
          read(tdL.cl, rmsg, sizeof(rmsg));

          memset(username, '\0', sizeof(username));
          memset(password, '\0', sizeof(password));
          memset(user, '\0', sizeof(user));
          memset(pass, '\0', sizeof(pass));
          
          strcpy(user,rmsg);

          int len=strlen(user);
          if(user[len-1]=='\n')
            user[len - 1] = '\0';

          FILE *file_ptr;
          file_ptr = fopen("users.txt","r+");
          if(file_ptr==NULL)
            perror("Error opening file");


          while(fscanf(file_ptr,"%s",username)==1){
            if(strcmp(user,username)==0)
            {
              ok=1;
              fscanf(file_ptr,"%s",password);
              break;
            }
          }
          fclose(file_ptr);
          
          if(ok==0){
            strncpy(wmsg,"[server]:Usernameul nu exista.\n",strlen("[server]:Usernameul nu exista.\n"));
            write(tdL.cl, wmsg, sizeof(wmsg));
            bzero(wmsg,sizeof(wmsg));
          }
          
          else{
            strncpy(wmsg,"[server]:password:",strlen("[server]:password:"));
            write(tdL.cl, wmsg, sizeof(wmsg));
            bzero(wmsg,sizeof(wmsg));

            bzero(&rmsg,sizeof(rmsg));
            read(tdL.cl, rmsg, sizeof(rmsg));

            strcpy(pass,rmsg);
            len=strlen(pass);
            if(pass[len-1]=='\n')
              pass[len - 1] = '\0';

            if(strcmp(pass,password)==0)
            {
              strncpy(wmsg,"[server]:Connectat cu succes\nComenzi:\n1.mesaje noi\n2.conversatii\n3.conversatie noua\n4.logout\n5.quit\n",strlen("[server]:Connectat cu succes\nComenzi:\n1.mesaje noi\n2.conversatii\n3.conversatie noua\n4.logout\n5.quit\n"));
              write(tdL.cl, wmsg, strlen(wmsg));
              bzero(wmsg,sizeof(wmsg));
              logat=1;
            }
            else{
              strncpy(wmsg,"[server]:Parola incorecta\n",strlen("[server]:Parola incorecta\n"));
              write(tdL.cl, wmsg, sizeof(wmsg));
              bzero(wmsg,sizeof(wmsg));
            }

          }

        }
    }
    else if(strncmp(rmsg,"new user",strlen("new user"))==0)
    {
      if(logat==1){
        strncpy(wmsg,"[server]:Sunteti deja autentificat\n",strlen("[server]:Sunteti deja autentificat\n"));
        write(tdL.cl, wmsg, sizeof(wmsg));
        bzero(wmsg,sizeof(wmsg));
      }
      else
      {

        strncpy(wmsg,"[server]:Creati un nou username\n[server]:username:",strlen("[server]:Creati un nou username\n[server]:username:"));
        write(tdL.cl, wmsg, sizeof(wmsg));
        bzero(wmsg,sizeof(wmsg));

        bzero(&rmsg,sizeof(rmsg));
        read(tdL.cl, rmsg, sizeof(rmsg));

        memset(user, '\0', sizeof(user));
        memset(pass, '\0', sizeof(pass));  

        int existauser=0;
        strcpy(user,rmsg);

        int len=strlen(user); 
        if(user[len-1]=='\n')  
          user[len - 1] = '\0';     

        FILE *file_ptr;
        file_ptr = fopen("users.txt","r+");
          if(file_ptr==NULL)
            perror("Error opening file");

          memset(username, '\0', sizeof(username));

          while(fscanf(file_ptr,"%s",username)==1){
            if(strncmp(user,username,strlen(username))==0)
                {
                  existauser=1;
                }
              
            
          }
          fclose(file_ptr);

        if(existauser==1)
        {
          strncpy(wmsg,"[server]:Usernameul exista deja.Introduceti alt username\n",strlen("[server]:Usernameul exista deja.Introduceti alt username\n"));
          write(tdL.cl, wmsg, sizeof(wmsg));
          bzero(wmsg,sizeof(wmsg));
        }

        else {
          file_ptr = fopen("users.txt","a+");
          if(file_ptr==NULL)
          perror("Error opening file");

          strcat(user," ");
          fputs("\n",file_ptr);
          fputs(user,file_ptr);

          strncpy(wmsg,"[server]:password:",strlen("[server]:password:"));
          write(tdL.cl, wmsg, sizeof(wmsg));
          bzero(wmsg,sizeof(wmsg));

          bzero(&rmsg,sizeof(rmsg));
          read(tdL.cl, rmsg, sizeof(rmsg));

          strcpy(pass,rmsg);
          len=strlen(pass);
          if(pass[len-1]=='\n')
            pass[len - 1] = '\0';

          fputs(pass,file_ptr);

          fclose(file_ptr);

          strncpy(wmsg,"[server]:User creat cu succes.Acum sunteti logat in aplicatie \n",strlen("[server]:User creat cu succes.Acum sunteti logat in aplicatie \n"));
          write(tdL.cl, wmsg, sizeof(wmsg));
          bzero(wmsg,sizeof(wmsg));


          // char mesaje_noi_txt[30];
          // strcpy(mesaje_noi_txt,"mesaje-noi-");
          // strcat(mesaje_noi_txt,user);
          // strcat(mesaje_noi_txt,".txt");
          // FILE *mesajenoi;
          // mesajenoi = fopen(mesaje_noi_txt, "a+");

          // fputs("Mesaje noi:",mesajenoi);
          // fclose(mesajenoi);

          logat=1;

        }

      }
    }
    else if(strncmp(rmsg,"conversatii",strlen("conversatii"))==0)
    {
        if(logat==0){
          strncpy(wmsg,"[server]:Nu sunteti logat.Trebuie sa va conectati pentru a utiliza aceasta comanda\n",strlen("[server]:Nu sunteti logat.Trebuie sa va conectati pentru a utiliza aceasta comanda\n"));
          write(tdL.cl, wmsg, sizeof(wmsg));
          bzero(wmsg,sizeof(wmsg));
        }
        else
        {
          strncpy(wmsg,"[server]:\nSelecteaza o conversatie.\nLista conversatii:\n",strlen("[server]:\nSelecteaza o conversatie.\nLista conversatii:\n"));

          FILE *lista_conversatii;
          char line[1000];
          lista_conversatii=fopen("lista_conversatii.txt","r+");
          if (lista_conversatii == NULL) {
              perror("Error opening file");
        
          }
          //printf("%s",user);
          int existaconversatii=0;
          while(fgets(line,sizeof(line),lista_conversatii)!=NULL){
            if(strstr(line,user)){
              strcat(wmsg,line);
              existaconversatii=1;
            }
          }

          fclose(lista_conversatii);

          if(existaconversatii==1)
          {
          strcat(wmsg,"\nNumar conversatie(dupa numarul conversatiei puneti '.'):");
          write(tdL.cl, wmsg, sizeof(wmsg));
          bzero(wmsg,sizeof(wmsg));

          bzero(&rmsg,sizeof(rmsg));
          read(tdL.cl, rmsg, sizeof(rmsg));

          char conversatie[100];
          char numar[10];
          char copie[100];
          char numefisier[100];
          
          strcpy(numar,rmsg);

          int len=strlen(numar); 
          if(numar[len-1]=='\n')  
            numar[len - 1] = '\0'; 

          //FILE *lista_conversatii;
          //char line[100];
          lista_conversatii=fopen("lista_conversatii.txt","r+");

          while(fgets(line,sizeof(line),lista_conversatii)!=NULL){
            if(strstr(line,numar))
              strcpy(conversatie,line);
          }

          fclose(lista_conversatii);

          char *dot = strstr(conversatie, ".");
          strcpy(numefisier, dot + 1);
          if(numefisier[strlen(numefisier)-1]=='\n')  
            numefisier[strlen(numefisier)-1] = '\0'; 
          strcat(numefisier,".txt");

          strncpy(wmsg,"1.Mesaj nou\n2.Istoric\n3.Reply\n4.exit\n",strlen("1.Mesaj nou\n2.Istoric\n3.Reply\n4.exit\n"));
          write(tdL.cl, wmsg, sizeof(wmsg));
          bzero(wmsg,sizeof(wmsg));
          while(1){

            bzero(&rmsg,sizeof(rmsg));
            read(tdL.cl, rmsg, sizeof(rmsg));

            if(strncmp(rmsg,"Mesaj nou",strlen("Mesaj nou"))==0){

              strncpy(wmsg,"[server]:Scrie mesajul:",strlen("[server]:Scrie mesajul:"));
              write(tdL.cl, wmsg, sizeof(wmsg));
              bzero(wmsg,sizeof(wmsg));

              bzero(&rmsg,sizeof(rmsg));
              read(tdL.cl, rmsg, sizeof(rmsg));

              char mesaj[300],ultimulmesaj[300],numarmesaj[10];

              FILE *MESAJ;
              MESAJ=fopen(numefisier,"r+");
              bzero(line,sizeof(line));

              if (MESAJ == NULL) {
                perror("Error opening file");
              }

              while(fgets(line,sizeof(line),MESAJ)!=NULL){
                strcpy(ultimulmesaj,line);
              }

              fclose(MESAJ);

              int index=0;

              for(int i=0;i<strlen(ultimulmesaj);i++){
                if (ultimulmesaj[i] == ' ') 
                    continue;
                if(ultimulmesaj[i]=='.')
                    break;
                numarmesaj[index++]=ultimulmesaj[i];
              }
              //write(tdL.cl, ultimulmesaj, sizeof(ultimulmesaj));
              numarmesaj[index]='\0';
              // char* dot = strchr(ultimulmesaj, '.');
              // int Len=dot-ultimulmesaj;
              // strncpy(numarmesaj,ultimulmesaj,Len);
            
              int numar=atoi(numarmesaj);
              numar++;
              sprintf(numarmesaj,"%d",numar);

              strcpy(mesaj,numarmesaj);
              strcat(mesaj,".[");
              strcat(mesaj,user);
              strcat(mesaj,"]:");
              strcat(mesaj,rmsg);
              mesaj[strlen(mesaj)-1]='\0';
              MESAJ=fopen(numefisier,"a+");
              fputs("\n",MESAJ);
              fputs(mesaj,MESAJ);

              fclose(MESAJ);

              strncpy(wmsg,"[server]:Mesaj trimis cu succes\n",strlen("[server]:Mesaj trimis cu succes\n"));
              write(tdL.cl, wmsg, sizeof(wmsg));
              bzero(wmsg,sizeof(wmsg));
              //mesaje noi
              // int found=0,j;
              // int delogat=0;
              // char useroffline[50],line[300],mesaje_noi_txt[50];

              //  for (int i = 0; i < strlen(copie_numefier); i++) {
              //     if (copie_numefier[i] == '-') {
              //       found = 1;
              //         continue;
              //     }
              //     if(found == 1){
              //     useroffline[j++] = copie_numefier[i];
              //     } 
              //   }

              //   FILE *loggedusers;
              //   loggedusers=fopen("logged-users.txt","r+");
              //   while(fgets(line,sizeof(line),loggedusers)!=NULL){
              //     if(strstr(line,useroffline)!=0)
              //       delogat=1;
              //   }
              //   fclose(loggedusers);
              //   strcpy(mesaje_noi_txt,"mesaje-noi-");
              //   strcat(mesaje_noi_txt,useroffline);
              //   strcat(mesaje_noi_txt,".txt");
              //   if(delogat==0)
              //   {
              //     FILE *mesajenoi;
              //     mesajenoi=fopen(mesaje_noi_txt,"r+");
              //     strcat(mesaj,"\n");
              //     fputs(mesaj,mesajenoi);
              //     fclose(mesajenoi);
              //   }
            }
            else if(strncmp(rmsg,"Istoric",strlen("Istoric"))==0){

              strncpy(wmsg,"[server]:Istoric:\n",strlen("[server]:Istoric:\n"));

              char istoric[3000];
              bzero(istoric,sizeof(istoric));
              FILE *Istoric;
              Istoric=fopen(numefisier,"r+");

              if (Istoric == NULL) {
                perror("Error opening file");
              }

              while(fgets(line,sizeof(line),lista_conversatii)!=NULL){
                strcat(istoric,line);
              }
              fclose(Istoric);
              strcat(istoric,"\n");
              strcat(wmsg,istoric);
              write(tdL.cl, wmsg, sizeof(wmsg));
              bzero(wmsg,sizeof(wmsg));

            }
            else if(strncmp(rmsg,"Reply",strlen("Reply"))==0){
              strncpy(wmsg,"[server]:Numarul mesajului:",strlen("[server]:Numarul mesajului:"));
              write(tdL.cl, wmsg, sizeof(wmsg));
              bzero(wmsg,sizeof(wmsg));

              bzero(&rmsg,sizeof(rmsg));
              read(tdL.cl, rmsg, sizeof(rmsg));

              //citesc numarul mesajului
              char nrmsg[10],mesajreplay[300],replay[300],ultimulmesaj[300],numarmesaj[10];
              memset(mesajreplay, '\0', sizeof(mesajreplay));
              strcpy(nrmsg,rmsg);
              if(nrmsg[strlen(nrmsg)-1]=='\n')
                  nrmsg[strlen(nrmsg)-1]='\0';

              //parcurg fisierul de mesaj

              FILE *REPLAY;
              REPLAY=fopen(numefisier,"r+");
              bzero(line,sizeof(line));

              while(fgets(line,sizeof(line),REPLAY)!=NULL)
              {
                  if(strstr(line,nrmsg)!=NULL)
                    if(strlen(mesajreplay)==0)
                      strcpy(mesajreplay,line);
                  strcpy(ultimulmesaj,line);
                  
              }
              //write(tdL.cl, ultimulmesaj, sizeof(ultimulmesaj));

              fclose(REPLAY);

              int index=0;

              for(int i=0;i<strlen(ultimulmesaj);i++){
                if (ultimulmesaj[i] == ' ') 
                    continue;
                if(ultimulmesaj[i]=='.')
                    break;
                numarmesaj[index++]=ultimulmesaj[i];
              }

              // char* DOT = strchr(ultimulmesaj, '.');
              // int Len=DOT-ultimulmesaj;
              // strncpy(numarmesaj,ultimulmesaj,Len);
            
              int numar=atoi(numarmesaj);
              numar++;
              sprintf(numarmesaj,"%d",numar);
              //write(tdL.cl, numarmesaj, sizeof(numarmesaj));

              strncpy(wmsg,"[server]:[reply]:",strlen("[server]:[reply]:"));
              write(tdL.cl, wmsg, sizeof(wmsg));
              bzero(wmsg,sizeof(wmsg));

              bzero(&rmsg,sizeof(rmsg));
              read(tdL.cl, rmsg, sizeof(rmsg));

              strcpy(replay,numarmesaj);
              strcat(replay,".[");
              strcat(replay,user);
              strcat(replay,"]:");
              strcat(replay,"[mesaj]:");
              if(mesajreplay[strlen(mesajreplay)-1]=='\n')
                mesajreplay[strlen(mesajreplay)-1]='\0';
              strcat(replay,mesajreplay);
              strcat(replay,"[reply]:");
              strcat(replay,rmsg);
              replay[strlen(replay)-1]='\0';

              REPLAY=fopen(numefisier,"a+");
              fputs("\n",REPLAY);
              fputs(replay,REPLAY);
              fclose(REPLAY);
              //citesc mesajul de replay

              //trimite mesajul de replay

              //write(tdL.cl, replay, sizeof(replay));

              strncpy(wmsg,"[server]:Mesaj trimis cu succes\n",strlen("[server]:Mesaj trimis cu succes\n"));
              write(tdL.cl, wmsg, sizeof(wmsg));
              bzero(wmsg,sizeof(wmsg));

              // int found=0,j;
              // int delogat=0;
              // char useroffline[50],line[300],mesaje_noi_txt[50];

              //  for (int i = 0; i < strlen(copie_numefier); i++) {
              //     if (copie_numefier[i] == '-') {
              //       found = 1;
              //         continue;
              //     }
              //     if(found == 1){
              //     useroffline[j++] = copie_numefier[i];
              //     } 
              //   }

              //   FILE *loggedusers;
              //   loggedusers=fopen("logged-users.txt","r+");
              //   while(fgets(line,sizeof(line),loggedusers)!=NULL){
              //     if(strstr(line,useroffline)!=0)
              //       delogat=1;
              //   }
              //   fclose(loggedusers);
              //   strcpy(mesaje_noi_txt,"mesaje-noi-");
              //   strcat(mesaje_noi_txt,useroffline);
              //   strcat(mesaje_noi_txt,".txt");
              //   if(delogat==0)
              //   {
              //     FILE *mesajenoi;
              //     mesajenoi=fopen(mesaje_noi_txt,"r+");
              //     strcat(mesaj,"\n");
              //     fputs(mesaj,mesajenoi);
              //     fclose(mesajenoi);
              //   }

            }
            else if(strncmp(rmsg,"exit",strlen("exit"))==0){
              strncpy(wmsg,"[server]:Ai iesit din conversatie\n",strlen("[server]:Ai iesit din conversatie\n"));
              write(tdL.cl, wmsg, sizeof(wmsg));
              bzero(wmsg,sizeof(wmsg));
              break;
            }
            else {
              strncpy(wmsg,"[server]:Comanda inexistenta\n",strlen("[server]:Comanda inexistenta\n"));
              write(tdL.cl, wmsg, sizeof(wmsg));
              bzero(wmsg,sizeof(wmsg));
            }
          }
          }
          else{
            bzero(wmsg,sizeof(wmsg));
            strncpy(wmsg,"[server]:Nu exista conversatii\n",strlen("[server]:Nu exista conversatii\n"));
            write(tdL.cl, wmsg, sizeof(wmsg));
            bzero(wmsg,sizeof(wmsg));
          }
        }
    }
    else if(strncmp(rmsg,"conversatie noua",strlen("conversatie noua"))==0)
    {
      if(logat==0){
        strncpy(wmsg,"[server]:Nu sunteti logat.Trebuie sa va conectati pentru a utiliza aceasta comanda\n",strlen("[server]:Nu sunteti logat.Trebuie sa va conectati pentru a utiliza aceasta comanda\n"));
        write(tdL.cl, wmsg, sizeof(wmsg));
        bzero(wmsg,sizeof(wmsg));
      }
      else{
        // strncpy(wmsg,"[server]:Creati o conversatie\n",strlen("[server]:Creati o conversatie\n"));
        // write(tdL.cl, wmsg, sizeof(wmsg));
        // bzero(wmsg,sizeof(wmsg));

        FILE *USERS;
        char line[1000],listauseri[300];;
        memset(listauseri,'\0',sizeof(listauseri));
        USERS=fopen("users.txt","r+");
        if (USERS == NULL) {
            perror("Error opening file");
        
        }
        while(fgets(line,sizeof(line),USERS)!=NULL){
          int j=0;
          char nume[20];
          for (int i = 0; i < strlen(line); i++) {
                if (line[i] == ' ') break;
              nume[j++] = line[i];
          }
          if(strcmp(nume,user)!=0){
          strncat(listauseri,nume,strlen(nume));
          strcat(listauseri,"\n");}
          memset(nume,'\0',sizeof(nume));
          memset(line,'\0',sizeof(line));
        }  

        fclose(USERS);

        strncpy(wmsg,"[server]:Creeaza o conversatie.\n[server]:Lista useri:\n",strlen("[server]:Creeaza o conversatie.\n[server]:Lista useri:\n"));
        strcat(wmsg,listauseri);
        strcat(wmsg,"[server]:Alege un user:");
        write(tdL.cl, wmsg, sizeof(wmsg));
        bzero(wmsg,sizeof(wmsg));

        bzero(&rmsg,sizeof(rmsg));
        read(tdL.cl, rmsg, sizeof(rmsg));

        char user2[20],conversatie1[50],conversatie2[50];
        strcpy(user2,rmsg);
        int existaconversatie=0;

        strcpy(user2,rmsg);

        if(user2[strlen(user2)-1]=='\n')
            user2[strlen(user2)-1]='\0';

        strcpy(conversatie1,user);
        strcat(conversatie1,"-");
        strcat(conversatie1,user2);

        strcpy(conversatie2,user2);
        strcat(conversatie2,"-");
        strcat(conversatie2,user);
        
        //parcurg fisierul lista useri
        FILE *lista_conversatii;
        lista_conversatii=fopen("lista_conversatii.txt","r+");

        while(fgets(line,sizeof(line),lista_conversatii)!=NULL){
          if(strstr(line,conversatie1)!=0||strstr(line,conversatie2)!=0)
              {
                existaconversatie=1;
                break;
              }
        }
        fclose(lista_conversatii);
        if(existaconversatie==1){
          strncpy(wmsg,"[server]:Conversatia deja exista \n",strlen("[server]:Conversatia deja exista \n"));
          write(tdL.cl, wmsg, sizeof(wmsg));
          bzero(wmsg,sizeof(wmsg));
        }
        else{
          strncpy(wmsg,"[server]:Conversatie creata cu succes.\n[server]:Introduceti un mesaj:",strlen("[server]:Conversatie creata cu succes.\n[server]:Introduceti un mesaj:"));
          write(tdL.cl, wmsg, sizeof(wmsg));
          bzero(wmsg,sizeof(wmsg));

          bzero(&rmsg,sizeof(rmsg));
          read(tdL.cl, rmsg, sizeof(rmsg));

          char mesaj[300];
          strcpy(mesaj,"1.[");
          strcat(mesaj,user);
          strcat(mesaj,"]:");
          strcat(mesaj,rmsg);
          char copie[50];
          strcpy(copie,conversatie1);
          strcat(copie,".txt");

          FILE *conversatienoua;
          conversatienoua=fopen(copie,"w");
          if(mesaj[strlen(mesaj)-1]=='\n')
            mesaj[strlen(mesaj)-1]='\0';
          fputs(mesaj,conversatienoua);
          fclose(conversatienoua);

          strncpy(wmsg,"[server]:Mesaj trimis cu succes\n",strlen("[server]:Mesaj trimis cu succes\n"));
          write(tdL.cl, wmsg, sizeof(wmsg));
          bzero(wmsg,sizeof(wmsg));


          lista_conversatii=fopen("lista_conversatii.txt","r+");

          char ultimulmesaj[100],numarmesaj[10],clista[100];

          while(fgets(line,sizeof(line),lista_conversatii)!=NULL){
            strcpy(ultimulmesaj,line);
          }
          //write(tdL.cl, ultimulmesaj, sizeof(ultimulmesaj));
          int index=0;

              for(int i=0;i<strlen(ultimulmesaj);i++){
                if (ultimulmesaj[i] == ' ') 
                    continue;
                if(ultimulmesaj[i]=='.')
                    break;
                numarmesaj[index++]=ultimulmesaj[i];
              }
            
              int numar=atoi(numarmesaj);
              numar++;
              sprintf(numarmesaj,"%d",numar);

          strcpy(clista,numarmesaj);
          strcat(clista,".");
          strcat(clista,conversatie1);

          fputs("\n",lista_conversatii);
          fputs(clista,lista_conversatii);

          fclose(lista_conversatii);
        }

        


      }
    }
    else if(strncmp(rmsg,"logout",strlen("logout"))==0){
      if(logat==0){
        strncpy(wmsg,"[server]:Sunteti deja deconectat\n",strlen("[server]:Sunteti deja deconectat\n"));
        write(tdL.cl, wmsg, sizeof(wmsg));
        bzero(wmsg,sizeof(wmsg));
      }
      else
      {
        strncpy(wmsg,"[server]:Deconectat cu succes\n",strlen("[server]:Deconectat cu succes\n"));
        write(tdL.cl, wmsg, sizeof(wmsg));
        bzero(wmsg,sizeof(wmsg));
        logat=0;

      }
    }
    else if(strncmp(rmsg,"quit",strlen("quit"))==0){
      strncpy(wmsg,"[server]:Program inchis\n",strlen("[server]:Program inchis\n"));
      write(tdL.cl, wmsg, sizeof(wmsg));
      bzero(wmsg,sizeof(wmsg));
      break;      

    }
    else {
      strncpy(wmsg,"[server]:Comanda inexistenta\n",strlen("[server]:Comanda inexistenta\n"));
      write(tdL.cl, wmsg, sizeof(wmsg));
      bzero(wmsg,sizeof(wmsg));

    }
    // else if(strncmp(rmsg,"mesaje noi",strlen("mesaje noi"))==0){
    //   if(logat==0){
    //     strncpy(wmsg,"[server]:Nu sunteti logat.Trebuie sa va conectati pentru a utiliza aceasta comanda\n",strlen("[server]:Nu sunteti logat.Trebuie sa va conectati pentru a utiliza aceasta comanda\n"));
    //     write(tdL.cl, wmsg, sizeof(wmsg));
    //     bzero(wmsg,sizeof(wmsg));
    //   }
    //   else{
    //     char mesaje_noi_txt[30],line[300],mesaj[3000];
    //     strcpy(mesaje_noi_txt,"mesaje-noi-");
    //     strcat(mesaje_noi_txt,user);
    //     strcat(mesaje_noi_txt,".txt");

    //     FILE *MESAJE_NOI;
    //     MESAJE_NOI=fopen(mesaje_noi_txt,"r+");

    //     while(fgets(line,sizeof(line),MESAJE_NOI)!=NULL){
    //         //strcat(mesaj,"\n");
    //         strcat(mesaj,line);
    //     }

    //     strncpy(wmsg,line,strlen(line));
    //     write(tdL.cl, wmsg, sizeof(wmsg));
    //     bzero(wmsg,sizeof(wmsg));

    //   }
    // }
  }
}

