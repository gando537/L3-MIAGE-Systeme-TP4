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
  socklen_t ls = sizeof(struct sockaddr_in); /* Taille des adresses */

  /*---- Caracterisation de la socket d'�mission ----------*/
  int sd_local;           /* Descripteur  */
  int ps_local = 5001;    /* Port         */
  struct sockaddr_in adr_local, *padr_local = &adr_local; /* Adresse  */

  /*---- Caracterisation de la socket client/ distante ------*/
  struct sockaddr_in adr_dist,*padr_dist = &adr_dist;  /* Adresse du destinataire */

  /*---- Buffers pour Messages -------------------------------*/ 
  char msg_in[3] = "0";     /* Message recu de "0" a "99" */
  char msg_out[3] = "0";    /* Message a envoyer "0" a "99" */

  /* 0) Verifications de base : Syntaxe d'appel =====*/
  if (argc != 1){
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
  //getsockname(sd_local,(struct sockaddr *)padr_local,&ls);
  getsockname(sd_local,(struct sockaddr *)padr_dist,&ls);


  /* 3) Boucle emission-reception : A PARTICULARISER selon que l'on
     est le serveur ou le client ... */
  int i = 0;
  printf("\n... Attente de reception ... \n");
  for(;;) {
    struct sockaddr_in adr2, *padr2 = &adr2; /* Inutilise pour l'instant */
	socklen_t len = sizeof(adr2);
    /* a) Emission */
	/* b) Reception */

	printf("------------------\n");
    if (recvfrom(sd_local,msg_in, sizeof(msg_in), 0, (struct sockaddr *)padr2, &len) == -1)
      printf("inachevee : %s !\n",msg_in);
    else  {
      /* c) Traitement : La reception est bonne, on fait evoluer i */
      i = atoi(msg_in); 
      printf("Requete recue %d --- de la machine cliente (%s) / %d\n",i,inet_ntoa(adr2.sin_addr), ntohs(adr2.sin_port));
      i = (i+1)%100; 
      sprintf(msg_out,"%d",i);
    }
	
    if (sendto(sd_local,msg_out, sizeof(msg_out), 0, (struct sockaddr *)padr2, ls) >0)
    	printf("Reponse emise %s --- vers la machine cliente (%s) / %d\n",msg_out,inet_ntoa(adr2.sin_addr), ntohs(adr2.sin_port));
    else
      printf("inacheve : %s !\n",msg_out);
    sleep(1);
  }
}
