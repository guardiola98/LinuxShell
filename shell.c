#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>



#define PROMPT		"\n=> "
#define ESPACIO		' '

#define MAX_BUF		1024
#define	MAX_ARGS	32
#define MAX_COMANDOS    10

struct comando{
  char *argv[MAX_ARGS];
  int nargs;
};


#define DEBUG

int arrange(char *buffer) {

  int i = 0, j = 0;

  while(buffer[i]) {
    while(buffer[i] == ESPACIO) i++;
    while((buffer[i] != ESPACIO) && buffer[i])
          buffer[j++] = buffer[i++];
    if(! buffer[i]) {
       if(buffer[i-2] == ESPACIO)
          buffer[j-2] = '\0';
       else buffer[j-1] = '\0';
    }
    else
       buffer[j++] = ESPACIO;
  }
 return(j);
}

int makeargs(char *buffer, char *args[],  int *haySalida, char **salida, int *hayError, char **error, int *hayEntrada, char **entrada) {

 int i = 0;
 char *p1, *p2, *aux;
 p1 = buffer;
 args[i++] = p1;
 int haCambiado=0;
 


 while(p2 = strchr(p1, ESPACIO)) {
    *p2++ = '\0';
    haCambiado=0;
    //SALIDAS
    if(strcmp(p1,">")==0){//miramos si hay salida
//         printf("Ha entrado");
        *haySalida=1;
        *salida=p2;
//         printf("argnosque=%s", args[i-1]);
//         printf("argnosque2=%s", args[i-1]);
        haCambiado=1;
    }else if (strcmp(p1,">>")==0){//miramos si hay salida trunc
//         printf("Ha entrado");
        *haySalida=2;
        *salida=p2;
        haCambiado=1;
    }
    
    //ERROR
    
    if(strcmp(p1, "2>")==0){
        *hayError=1;
        *error=p2;
        haCambiado=1;
    }else if(strcmp(p1, "2>>")==0){
        *hayError=2;
        *error=p2;
        haCambiado=1;
    }
    
    //ENTRADA
    
    if(strcmp(p1,"<")==0){
        *hayEntrada=1;
        *entrada=p2;
        haCambiado=1;
    }
    
//     printf("haCambiado=%d", haCambiado);
    if( haCambiado==0 && (p2[0]!='>' && (p2[0]!='2' && p2[1]!='>') && p2[0]!='<' )){
        args[i++] = p2;
        
    }
    
    p1=p2;

 }
 args[i] = NULL;
 return(i);
}

int desglosar_tub (char *buffer, struct comando lista_comandos[],int *haySalida, char **salida, int *hayError, char **error, int *hayEntrada, char **entrada ) 
{
/* Desglosa una secuencia de comandos enlazados mediante tuber�as 
   (y almacenada a partir de la posici�n "buffer") en los diferentes
   comandos simples de �sta, alacenando cada uno de ellos en una estructura 
   de tipo "argc-argv" del vector "lista_comandos"  */

  char *p1, *p2;
  int i,j,n;
  int ncomandos=0;

  p1=buffer;
  i=0;

#ifdef DEBUG
//   printf("\nLinea de comandos: #%s#\n", buffer);
#endif

  n=arrange(p1);
  if (n==1) return(0); /* No hay comandos, sino blancos o <CR> */
  while (p2=strchr(p1,'|'))
  {  if (*(p2-1)==ESPACIO) *(p2-1)='\0'; else *p2='\0';
     p2++;
//      printf("Eyyyy");
     lista_comandos[ncomandos].nargs = makeargs(p1,lista_comandos[ncomandos].argv, haySalida, salida, hayError, error, hayEntrada, entrada);
     ncomandos++;
     if (p2!=NULL) { if (*p2!=ESPACIO) p1=p2; else p1=p2+1; }
  } /* fin while */

  lista_comandos[ncomandos].nargs = makeargs(p1,lista_comandos[ncomandos].argv,haySalida, salida, hayError, error, hayEntrada, entrada);

  ncomandos++; 

#ifdef DEBUG
//   for (i=0; i<ncomandos; i++)
//   {  printf("Comando %d (%d argumentos):\n",i,lista_comandos[i].nargs);
//      j=0;
//      while(lista_comandos[i].argv[j])
//      { printf("comando[%d].argv[%d] (dir. %016lX) = #%s#\n", i, j,(unsigned long) (lista_comandos[i].argv[j]), lista_comandos[i].argv[j]);
//        j++;
//      }
//   }
#endif
  return(ncomandos);  /* Numero de componentes del vector lista_comandos 
                          (empezando a contar desde 1) */
}






int main( int argc, char *argv[] ) {

#ifdef DEBUG
 int i;
#endif

 int  haySalida, hayEntrada, hayError;
 char  *salida,*entrada, *error, *directorio, *di, *puntFinal, *puntero, *varEntorno, *p1, *p2;
 char *puntVariable= (char *) malloc(255);
 char *echo= (char *) malloc(255);
 char *aux= (char *) malloc(255);
 int chdir( const char *path);
 haySalida=0;
 hayEntrada=0;
 hayError=0;
 int variable, estado;
 const char *dirCons;
 char buffer[MAX_BUF], *args[MAX_ARGS], bufferCopia[MAX_BUF];
 struct comando lista_comandos[MAX_COMANDOS];

 puts("\nShell creat per Josep Guardiola\n");
 
 while(1) {
   printf(PROMPT);
   if (fgets(buffer,  MAX_BUF, stdin) == NULL) 
       continue;
   //GESTION DEL CD
      directorio=buffer;
      strcpy(echo, directorio);//podem gastar echo per a strcpm


   if (buffer[0]=='c'&& directorio[1]=='d'){
       directorio ++;
       directorio ++;
       directorio ++;
       puntFinal=strchr(directorio, '\n');
       *puntFinal='\0';
       variable=chdir(directorio);
       continue;
   }

   //GESTION DE VARIABLES
   if(strchr(directorio, '=')!=NULL){//hay asignacion
       strcpy(puntVariable, directorio);
       puntero=strchr(puntVariable, '\n');
       *puntero='\0';
       

       putenv(puntVariable);
       continue;
   }
   

   
   if(strstr(echo, "echo")!=NULL){
        p1=echo;
        p2=strchr(p1,ESPACIO);
        *p2++='\0';
        p1=p2;


        if(strstr(p1, "$")==NULL){//si no hay variables
            printf("%s", p1);
        }else{
            while(p2 = strchr(p1,ESPACIO)){
                *p2++='\0';
                 if(p1[0]=='$'){
                
                    p1++;
                    if(strcmp(p1,"?")==0){
                        
                        printf("%d ",estado);
                    }else{
                        varEntorno=getenv(p1);
                        printf("%s ", varEntorno);
                }
            }else{
                printf("%s ", p1); 
            }
            p1=p2;
        }
        if(strstr(p1, "$")!=NULL){
            p1++;
            p2=strstr(p1,"\n");
            *p2='\0';
            varEntorno=getenv(p1);
            
            printf("%s", varEntorno);
        }else{
            printf("%s", p1);
        }
        }

       
       continue;   
       
      

   }
   
   //EXIT
   if(strcmp(echo,"exit\n")==0){
       exit(0);
    }
   
   
   //arrange(buffer);
   
#ifdef DEBUG
//    printf("Entrada: #%s#\n", buffer);
//    i = 0;
#endif
   //makeargs(buffer, args);
#ifdef DEBUG
//    while(args[i]) {
//      printf("\nargs[%d] (dir. %016lX) = #%s#", i, (unsigned long) (args[i]), args[i]);
//      i++;
//    }
#endif
       if(fork()){
      wait(&estado);
      estado = estado & 0x0000ff00;
      estado = estado >> 8;
   }
   else 
    {
      int i, tub[2], n_comandos;
      char bufferCopia[MAX_BUF];
      strcpy(bufferCopia, buffer);
      n_comandos=desglosar_tub(buffer, lista_comandos, &haySalida, &salida, &hayError, &error, &hayEntrada, &entrada);

    //GESTION DE SALIDA DE ERROR
     if(hayError == 1){
        close(2);
        printf("ERROR %s", error);
        open(error, O_WRONLY|O_CREAT|O_TRUNC, 0666);
     }else if (hayError == 2){
        close(2);
        open(error, O_WRONLY|O_CREAT|O_APPEND, 0666);
         
     }
     
     
      for(i=0;i<n_comandos-1;i++){
        pipe(tub);
        if(fork()!=0){//Pare
            close(tub[1]);
            close(0); dup(tub[0]);
            close(tub[0]);
                
            //redirecciones
                
            //salida
            if (haySalida==1){
                close(1);
                open(salida, O_WRONLY|O_CREAT|O_TRUNC, 0666);
            }else if(haySalida == 2){
                close(1);
                open(salida, O_WRONLY|O_CREAT|O_APPEND, 0666);
            }
                
            
                
            execvp(lista_comandos[n_comandos-1-i].argv[0], lista_comandos[n_comandos-1-i].argv);
            perror("ERROR");exit(-1);
            
        }else{
            
            close(1);dup(tub[1]);close(tub[1]);
            close(tub[0]);
            
            if(i==n_comandos-2){
                if(hayEntrada==1){
                    close(0);
                    open(entrada, O_RDONLY, 0666);
                }
            }
            
        }
      }
      
      if(n_comandos == 1 ){
        //salida
        if(haySalida==1){
          close(1);
          open(salida, O_WRONLY|O_CREAT|O_TRUNC, 0666);
        }else if(haySalida==2){
          close(1);
          open(salida, O_WRONLY|O_CREAT|O_APPEND, 0666);
        }else if(hayEntrada==1){
            close(0);
            open(entrada, O_RDONLY, 0666);
        }
        
        
      }
      
      execvp(lista_comandos[0].argv[0], lista_comandos[0].argv);
      perror("ERROR");
      exit(-1);
      
      /*
      execvp(args[0], args);
      perror("Error en exec");
      exit(-1);
      */
      }
  }
}
