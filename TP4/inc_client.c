/** Fichier : inc.c (Communication Sockets/UDP)
 *   Les deux processus distants s'envoient un nombre qu'ils 
 *   incrementent successivement : L'un compte en pair, l'autre en impair ...  */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]){ 
	(void)argc;
	(void)argv;
  int looptime = 0; /* Numero de la boucle */
  socklen_t ls = sizeof(struct sockaddr_in); /* Taille des adresses */

  /*---- Caracterisation de la socket d'�mission ----------*/
  int sd_local;           /* Descripteur  */
  int ps_local = 0;    /* Port         */
  struct sockaddr_in adr_local, *padr_local = &adr_local; /* Adresse  */

  /*---- Caracterisation de la socket distante ------*/
  struct sockaddr_in adr_dist,*padr_dist = &adr_dist;  /* Adresse du destinataire */
  struct hostent *hp_dist;       /* Adresse IP de la machine distante */
	int ps_dist = 5001;

  /*---- Buffers pour Messages -------------------------------*/ 
  char msg_in[3] = "0";     /* Message recu de "0" a "99" */
  char msg_out[3] = "0";    /* Message a envoyer "0" a "99" */

  /* 0) Verifications de base : Syntaxe d'appel =====*/
  if (argc != 2){
    fprintf(stderr,"Syntaxe d'appel : a.out nom_du_host_peer \n");
    exit(2);
  }
  /* 1) Preparation de  la socket d'�mission ================*/
  /* a) Creation : Domaine AF_INET, type DGRAM, proto. par defaut*/
  if ((sd_local=socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    perror("[SOCK_DGRAM, AF_INET, 0]");
  else
    printf("socket [SOCK_DGRAM, AF_INET, 0] creee\n");
  /* b) Preparation de l'adresse d'attachement */
  adr_local.sin_family      = AF_INET;
  adr_local.sin_addr.s_addr = htonl(INADDR_ANY);  /* Format reseau */
  adr_local.sin_port        = htons(ps_local);  /* Format reseau */

  printf("Local Adress avant bind() : %s\n", inet_ntoa(adr_local.sin_addr));
  printf("Local Port avant bind() : %d\n", ntohs(adr_local.sin_port));
  /* c) Demande d'attachement de la socket */
  if(bind(sd_local,(struct sockaddr *)(padr_local),ls) == -1) {
    perror("Attachement de la socket impossible");
    close(sd_local);  /* Fermeture de la socket               */
    exit(2);       /* Le processus se termine anormalement.*/
  }
  /* d) Recuperation de l'adresse effective d'attachement. */
  getsockname(sd_local,(struct sockaddr *)padr_local,&ls);

  /* 2) Concernant l'adresse de la  socket de destination ======*/
  /* a) A partir du nom du destinataire */
  hp_dist =gethostbyname(argv[1]);
  if(hp_dist == NULL){ 
    fprintf(stderr,"machine %s inconnue\n",argv[1]); 
    exit(2); 
  }
  else{ /* Recuperation de l'adresse IP depuis la struct hostent */
    memcpy(&adr_dist.sin_addr.s_addr, hp_dist->h_addr, hp_dist->h_length);
    adr_dist.sin_family = AF_INET;
    adr_dist.sin_port   = htons(ps_dist); /* Meme port que sd0 : why not ? */
    fprintf(stdout,"machine %s --> %s \n", hp_dist->h_name, inet_ntoa(adr_dist.sin_addr)); 
  }

  /* 3) Boucle emission-reception : A PARTICULARISER selon que l'on 
     est le serveur ou le client ... */
	int i = 0;
  for(;;) { 
    struct sockaddr_in adr2, *padr2 = &adr2; /* Inutilise pour l'instant */
    socklen_t len = sizeof(adr2);
    /* a) Emission */
    printf("------------------\n");
    if (sendto(sd_local,msg_out, sizeof(msg_out), 0, (struct sockaddr *)padr_dist, ls) >0)
    	printf("Requete (#%d) ... envoyee : valeur = %d ! [HOST(SERVER):%s] [PORT:%d]\n", looptime, i, inet_ntoa(adr2.sin_addr), ntohs(adr2.sin_port));
    else
      printf("inacheve : %s !\n",msg_out);
    /* b) Reception */
    if (recvfrom(sd_local,msg_in, sizeof(msg_in), 0, (struct sockaddr *)padr2, &len) == -1)  
      printf("inachevee : %s !\n",msg_in);
    else  {
      printf("Reponse (#%d) ... recue   : valeur = %s ! [PORT:%d]\n",looptime, msg_in, ntohs(adr_local.sin_port));
      /* c) Traitement : La reception est bonne, on fait evoluer i */
      i = atoi(msg_in); 
      i = (i+1)%100; 
      sprintf(msg_out,"%d",i);
    }
    sleep(1); looptime++;
  }
}
