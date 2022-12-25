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

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " ="
#define MAX_DGRAM_SIZE 256

char ** split_line( char *line){
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

char *malloced(size_t size){
	char *mall = (char *)malloc(sizeof(char) * size);
	return (mall);
}

void free_malloc(char *operand, char *operator){
  free(operand);
  free(operator);
}

long double operation(long double op1, long double op2, char op)
{
	switch(op) {
		case '+' :
			return (op1 + op2);
		case '-' :
			return (op1 - op2);
		case '*' :
			return (op1 * op2);
		default :
			return (op1 / op2);
	}
}

int parse_op(char *message, char *operand, char *operator)
{
	int j = 0;
	int k = 0;
	int i = 0;
	while(message[i] && message[i] != '\n'){
		if (message[i] == '+' || message[i] == '-' || message[i] == '*' || message[i] == '/'){
			operator[j++] = message[i++];
      operand[k++] = ' ';
		}
		else if((message[i] >= 48 && message[i] <= 57) || message[i] == ' ' || message[i] == 'x'){
			operand[k++] = message[i++];
		}
    else
      return (0);
	}
	operand[k] = '\0';
	operator[j] = '\0';
  return (1);
}

int check_value(char *operator, char *operand, int x){
  char *str = strdup(operand);
  char **s = split_line(str);
  free(str);
  if(operator[0] == '/'){
    if (!strncmp("0", s[1], 1) || (!strncmp("x", s[1], 1) && x == 0)){
      free(s);
      return (1);
    }
  }
  free(s);
  return(0);
}

long double calcul(char *operand, char *operator, int x){
	char **tokens = split_line(operand);
  char op = operator[0];
	int i = 1;
	long double res = (strncmp(tokens[0], "x",1)) ? (long double)atoi(tokens[0]) : (long double)x;

	int j = 0;
  long double k;
	while (tokens[i]) {
    k = (strncmp(tokens[i], "x",1)) ? (long double)atoi(tokens[i]) : (long double)x;
		res = operation(res, k, op);
    op = (operator[++j]) ? operator[j] : op;
		i++;
	}
	free(tokens);
	return (res);
}

int main(int argc, char *argv[]){
	 (void)argc;
	 (void)argv;

  int x = 0;
  pid_t pid;

  char *operator;
	char *operand;

  socklen_t ls = sizeof(struct sockaddr_in); /* Taille des adresses */

  /*---- Caracterisation de la socket d'�mission ----------*/
  int sd_local;           /* Descripteur  */
  int ps_local = 5001;    /* Port         */
  struct sockaddr_in adr_local, *padr_local = &adr_local; /* Adresse  */

  /*---- Caracterisation de la socket client/ distante ------*/
  struct sockaddr_in adr_dist,*padr_dist = &adr_dist;  /* Adresse du destinataire */

  /*---- Buffers pour Messages -------------------------------*/
  char msg_in[10];
  char msg_out[10];

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
  long double i = 0;
  printf("\n... Attente de reception ... \n");
  for(;;) {
    struct sockaddr_in adr2, *padr2 = &adr2;
	  socklen_t len = sizeof(adr2);
    /* a) Emission */
	/* b) Reception */

    printf("------------------\n");
    if (recvfrom(sd_local,msg_in, sizeof(msg_in), 0, (struct sockaddr *)padr2, &len) == -1)
      printf("inachevee : %s !\n",msg_in);
    else  {
      if (!strncmp(msg_in, "(", 1)){
        pid = fork();
        switch (pid) {
          case -1 : perror("Erreur de création du processus");
            exit(1);
          case 0 : /* Ce code s'exécute chez le fils */
            if ((sd_local=socket(AF_INET, SOCK_DGRAM, 0)) == -1)
              perror("[SOCK_DGRAM, AF_INET, 0]");
            else
              printf("socket [SOCK_DGRAM, AF_INET, 0] creee\n");
            struct sockaddr_in adr_local1, *padr_local1 = &adr_local1; 
            adr_local1.sin_family      = AF_INET;
            adr_local1.sin_addr.s_addr = htonl(INADDR_ANY);  /* Format reseau */
            adr_local1.sin_port        = 0;  /* Format reseau */
            if(bind(sd_local,(struct sockaddr *)(padr_local1),ls) == -1) {
              perror("Attachement de la socket impossible");
              close(sd_local);  /* Fermeture de la socket               */
              exit(2);       /* Le processus se termine anormalement.*/
            }
            /* d) Recuperation de l'adresse effective d'attachement. */
            //getsockname(sd_local,(struct sockaddr *)padr_local,&ls);
            getsockname(sd_local,(struct sockaddr *)padr_local1,&ls);
            printf("- Création du fils : [pid=%d] pour [PORT=%d]\n", getpid(), ntohs(adr2.sin_port));
            if (sendto(sd_local,"#", sizeof("#"), 0, (struct sockaddr *)padr2, ls) >0)
              printf("Reponse emise %s --- vers la machine cliente [(%s):%d]\n","#",inet_ntoa(adr2.sin_addr), ntohs(adr2.sin_port));
            else
              printf("inacheve : %s !\n",msg_out);
            while (1)
            {
              printf("------------------\n");
              if (recvfrom(sd_local,msg_in, sizeof(msg_in), 0, (struct sockaddr *)padr2, &len) > 0){
                msg_in[strlen(msg_in)] = '\0';
                if (strstr(msg_in,"=")){
                  char ** tokens = split_line(msg_in);
                  x = (!strncmp(tokens[0], "x", 1)) ? atoi(tokens[1]) : atoi(tokens[0]);
                  char *out = (!strncmp(tokens[0], "x", 1)) ? tokens[1] : tokens[0];
                  free(tokens);

                  if (sendto(sd_local,out, sizeof(out), 0, (struct sockaddr *)padr2, ls) >0)
                    printf("Reponse emise %s --- vers la machine cliente [(%s):%d]\n",out,inet_ntoa(adr2.sin_addr), ntohs(adr2.sin_port));
                  else
                    printf("inacheve : %s !\n",out);
                }
                else {
                  operator = malloced(strlen(msg_in));
                  operand = malloced(strlen(msg_in));
                  if (!parse_op(msg_in, operand, operator) || (check_value(operator, operand, x))) {
                    sendto(sd_local, "NaN", sizeof("NaN"), 0, (struct sockaddr *)padr2, ls);
                    free_malloc(operator, operand);
                    continue;
                  }
                  else {
                    i = calcul(operand, operator, x);
                    free_malloc(operator, operand);
                    printf("Requete recue de la machine cliente [(%s):%d] operation : %s\n",inet_ntoa(adr2.sin_addr), ntohs(adr2.sin_port),msg_in);
                    sprintf(msg_out,"%.2Lf",i);
                  }

                  if (sendto(sd_local,msg_out, sizeof(msg_out), 0, (struct sockaddr *)padr2, ls) >0)
                    printf("Reponse emise %s --- vers la machine cliente [(%s):%d]\n",msg_out,inet_ntoa(adr2.sin_addr), ntohs(adr2.sin_port));
                  else
                    printf("inacheve : %s !\n",msg_out);
                }
              }
            }
          default : /* Ce code s'exécute chez le père */
            ;
        }
      }
    }
  }
}