
#include <stdio.h>      // libreria estandar
#include <stdlib.h>     // para usar exit y funciones de la libreria standard
#include <string.h>
#include <pthread.h>    // para usar threads
#include <semaphore.h>  // para usar semaforos
#include <unistd.h>
#include <stdint.h>

#define LIMITE 50
#define MAXCHAR 1000



int  EsGanador = 0;

static pthread_mutex_t mutex_salar = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_cocinar_plancha = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_hornear_pan = PTHREAD_MUTEX_INITIALIZER;


//creo estructura de semaforos 
struct semaforos {
	sem_t sem_cortar;
	sem_t sem_mezclar;
	sem_t sem_salar;
	sem_t sem_armar_medallones;
	sem_t sem_cocinar_plancha;
	sem_t sem_cortar_lechuga_tomate;
	sem_t sem_hornear_pan;
	sem_t sem_aux;
	//sem_t sem_armar_hamburguesa;
    
	//poner demas semaforos aqui
};

//creo los pasos con los ingredientes
struct paso {
   char accion [LIMITE];
   char ingredientes[4][LIMITE];
   
};

//creo los parametros de los hilos 
struct parametro {
int equipo_param;
struct semaforos semaforos_param;
struct paso pasos_param[8];
};

//funcion para imprimir las acciones y los ingredientes de la accion
void* imprimirAccion(void *data, char *accionIn) {
	FILE *open;
	open = fopen("ejecucion.txt","a");
	
		
	struct parametro *mydata = data;
	//calculo la longitud del array de pasos 
	int sizeArray = (int)( sizeof(mydata->pasos_param) / sizeof(mydata->pasos_param[0]));
	//indice para recorrer array de pasos 
	int i;
	for(i = 0; i < sizeArray; i ++){
		//pregunto si la accion del array es igual a la pasada por parametro (si es igual la funcion strcmp devuelve cero)
		if(strcmp(mydata->pasos_param[i].accion, accionIn) == 0){
		fprintf(open,"\tEquipo %d - accion %s \n " , mydata->equipo_param, mydata->pasos_param[i].accion);
		printf("\tEquipo %d - accion %s \n " , mydata->equipo_param, mydata->pasos_param[i].accion);
		//calculo la longitud del array de ingredientes
		int sizeArrayIngredientes = (int)( sizeof(mydata->pasos_param[i].ingredientes) / sizeof(mydata->pasos_param[i].ingredientes[0]) );
		//indice para recorrer array de ingredientes
		int h;
		fprintf(open,"\tEquipo %d-----------ingredientes:-------------- \n ", mydata->equipo_param);
		printf("\tEquipo %d -----------ingredientes : ----------\n",mydata->equipo_param); 
			for(h = 0; h < sizeArrayIngredientes; h++) {
				//consulto si la posicion tiene valor porque no se cuantos ingredientes tengo por accion 
				if(strlen(mydata->pasos_param[i].ingredientes[h]) != 0) {
							fprintf(open,"\tEquipo %d ingrediente  %d : %s \n",mydata->equipo_param,h,mydata->pasos_param[i].ingredientes[h]);
							printf("\tEquipo %d ingrediente  %d : %s \n",mydata->equipo_param,h,mydata->pasos_param[i].ingredientes[h]);
				}
			}
		}
	}
}

//funcion para tomar de ejemplo
void* cortar(void *data) {
	//creo el nombre de la accion de la funcion 
	char *accion = "cortar";
	//creo el puntero para pasarle la referencia de memoria (data) del struct pasado por parametro (la cual es un puntero). 
	struct parametro *mydata = data;
		sem_wait(&mydata->semaforos_param.sem_cortar);
		//llamo a la funcion imprimir le paso el struct y la accion de la funcion
	imprimirAccion(mydata,accion);
	//uso sleep para simular que que pasa tiempo
	usleep( 500000 );
	//doy la señal a la siguiente accion (cortar me habilita mezclar)
    		sem_post(&mydata->semaforos_param.sem_mezclar);
	
    pthread_exit(NULL);
}

void* mezclar(void *data){
	char *accion = "mezclar";
	struct parametro *mydata = data;
		sem_wait(&mydata->semaforos_param.sem_mezclar);
	imprimirAccion(mydata,accion);
	usleep( 500000 );
	//doy la señal a la siguiente accion ( Mezclar me habilita salar)
   		sem_post(&mydata->semaforos_param.sem_salar);
pthread_exit(NULL);
}
//creo la funcion salar
void* salar(void *data){
	//pthread_mutex_lock(&mutex_salar); // NO ACA
	char *accion = "salar";
	struct parametro *mydata = data;
		pthread_mutex_lock(&mutex_salar);
		sem_wait(&mydata->semaforos_param.sem_salar);
	imprimirAccion(mydata,accion);
	usleep( 500000 );
	// doy la señal a siguiente accion  ( salar me habilita armar medallones)
   		sem_post(&mydata->semaforos_param.sem_armar_medallones);
		pthread_mutex_unlock(&mutex_salar);
pthread_exit(NULL);
}

//creo la funcion armar_medallones
void* armar_medallones(void *data){
	char *accion = "armar medallones";
	struct parametro *mydata = data;
		sem_wait(&mydata->semaforos_param.sem_armar_medallones);
	imprimirAccion(mydata,accion);
	usleep( 500000 );
		sem_post(&mydata->semaforos_param.sem_cocinar_plancha);
pthread_exit(NULL);
}

//creo la funcion  cocinar en plancha
void* cocinar_plancha(void *data){
	char *accion = "cocinar_plancha";
        struct parametro *mydata = data;
        
		sem_wait(&mydata->semaforos_param.sem_cocinar_plancha);
        	pthread_mutex_lock(&mutex_cocinar_plancha);

	imprimirAccion(mydata,accion);
        usleep( 500000 );

		pthread_mutex_unlock(&mutex_cocinar_plancha);
		sem_post(&mydata->semaforos_param.sem_aux);
pthread_exit(NULL);
}

//creo la funcion hornear panes
void* hornear_pan(void *data){
        	pthread_mutex_lock(&mutex_hornear_pan);
	char *accion = "hornear_pan";
        struct parametro *mydata = data;
        imprimirAccion(mydata,accion);
        usleep( 500000 );
		pthread_mutex_unlock(&mutex_hornear_pan);
  		sem_post(&mydata->semaforos_param.sem_hornear_pan);
pthread_exit(NULL);
}

//creo la funcion cortar lechuga y tomate
void* cortar_lechuga_tomate(void *data){
        char *accion = "cortar_lechuga_tomate";
        struct parametro *mydata = data;
        imprimirAccion(mydata,accion);
        usleep( 500000 );
  		 sem_post(&mydata->semaforos_param.sem_cortar_lechuga_tomate);
pthread_exit(NULL);
}

//creo la funcion armar hamburguesa
void* armar_hamburguesa(void *data){
	char *accion = "armar_hamburguesa";
        struct parametro *mydata = data;
	sem_wait(&mydata->semaforos_param.sem_hornear_pan); //pregunta si tiene señal o no
        sem_wait(&mydata->semaforos_param.sem_aux);
        sem_wait(&mydata->semaforos_param.sem_cortar_lechuga_tomate);


        imprimirAccion(mydata,accion);
        usleep( 500000 );
   	sem_post(&mydata->semaforos_param.sem_hornear_pan);//para darle señal a otro semaforo
	//sem_post(&mydata->semaforos_param.sem_cocinar_plancha);
	sem_post(&mydata->semaforos_param.sem_cortar_lechuga_tomate);


	if(EsGanador ==0){
	//imprimo el ganador
	 EsGanador = mydata->equipo_param;

	}

pthread_exit(NULL);
}

void* ejecutarReceta(void *i) {
	
	//variables semaforos
	sem_t sem_cortar;
	sem_t sem_mezclar;
	//crear variables semaforos aqui
	sem_t sem_salar;			///VER!! ademas la funcion tiene que preguntar por los mutex globales
	sem_t sem_armar_medallones;	
	sem_t sem_cocinar_plancha;		// mismo con planchar
	sem_t sem_cortar_lechuga_tomate;	
	sem_t sem_hornear_pan;
	//sem_t sem_armar_hamburguesa;
	sem_t sem_aux;
	
	//variables hilos
	pthread_t p1;
	pthread_t p2;
	pthread_t p3;
	pthread_t p4;
	pthread_t p5;
	pthread_t p6;
	pthread_t p7;
	pthread_t p8;


	//crear variables hilos aqui
	
	//numero del equipo (casteo el puntero a un int)
	int p = *((int *) i);
	
	printf("Ejecutando equipo %d \n", p);

	//reservo memoria para el struct
	struct parametro *pthread_data = malloc(sizeof(struct parametro));

	//seteo los valores al struct
	
	//seteo numero de grupo
	pthread_data->equipo_param = p;

	//seteo semaforos
	pthread_data->semaforos_param.sem_cortar = sem_cortar;
	pthread_data->semaforos_param.sem_mezclar = sem_mezclar;
	//setear demas semaforos al struct aqui
	pthread_data->semaforos_param.sem_salar = sem_salar;
	pthread_data->semaforos_param.sem_armar_medallones = sem_armar_medallones;
	pthread_data->semaforos_param.sem_cocinar_plancha = sem_cocinar_plancha;
        pthread_data->semaforos_param.sem_cortar_lechuga_tomate = sem_cortar_lechuga_tomate;
	pthread_data->semaforos_param.sem_hornear_pan = sem_hornear_pan;
	pthread_data->semaforos_param.sem_aux = sem_aux;
	//pthread_data->semaforos_param.sem_armar_hamburguesa = sem_armar_hamburguesa;
	

	//seteo las acciones y los ingredientes (Faltan acciones e ingredientes) ¿Se ve hardcodeado no? ¿Les parece bien?
     	//buscar funcion para leer del archivo  la receta. 

	 // strtok  =  if a token is found, a pointer to the beginning of the token.
	// otherwise, a null pointer. 
	//A null pointer is always returned when the end of the string (i.e, a null character)is reached in the string being scanned.
	/* 
	char* pch;
	pch = strtok(str,",.-");
	while (pch != NULL){
		printf( "%s\n" , pch);
		pch = strtok (NULL, " ,.-");
	}
	return 0;*/
	

	/*FILE *fp;
	char* filename = "receta.txt";
	
	char * pch = NULL;
	size_t lenght = 0;
	ssize_t read;

	fp = fopen(filename, "r");
	if (fp == NULL){
		printf("Error opening file");
		exit(EXIT_FAILURE);
	*/
	

	strcpy(pthread_data->pasos_param[0].accion, "cortar");
	strcpy(pthread_data->pasos_param[0].ingredientes[0], "ajo");
        strcpy(pthread_data->pasos_param[0].ingredientes[1], "perejil");
 	strcpy(pthread_data->pasos_param[0].ingredientes[2], "cebolla");

	strcpy(pthread_data->pasos_param[1].accion, "mezclar");
	strcpy(pthread_data->pasos_param[1].ingredientes[0], "ajo");
        strcpy(pthread_data->pasos_param[1].ingredientes[1], "perejil");
 	strcpy(pthread_data->pasos_param[1].ingredientes[2], "cebolla");
	strcpy(pthread_data->pasos_param[1].ingredientes[3], "carne picada");

	strcpy(pthread_data->pasos_param[2].accion, "salar");
        strcpy(pthread_data->pasos_param[2].ingredientes[0], "ajo");
        strcpy(pthread_data->pasos_param[2].ingredientes[1], "perejil");
        strcpy(pthread_data->pasos_param[2].ingredientes[2], "cebolla");
        strcpy(pthread_data->pasos_param[2].ingredientes[3], "carne picada");

	strcpy(pthread_data->pasos_param[3].accion, "armar_medallones");
        strcpy(pthread_data->pasos_param[3].ingredientes[0], "ajo");
        strcpy(pthread_data->pasos_param[3].ingredientes[1], "perejil");
        strcpy(pthread_data->pasos_param[3].ingredientes[2], "cebolla");
        strcpy(pthread_data->pasos_param[3].ingredientes[3], "carne picada");

	strcpy(pthread_data->pasos_param[4].accion, "cocinar_plancha");
        strcpy(pthread_data->pasos_param[4].ingredientes[0], "ajo");
        strcpy(pthread_data->pasos_param[4].ingredientes[1], "perejil");
        strcpy(pthread_data->pasos_param[4].ingredientes[2], "cebolla");
        strcpy(pthread_data->pasos_param[4].ingredientes[3], "carne picada");

	strcpy(pthread_data->pasos_param[5].accion, "hornear_pan");
        //strcpy(pthread_data->pasos_param[5].ingredientes[0], "ajo");
        //strcpy(pthread_data->pasos_param[5].ingredientes[1], "perejil");
        //strcpy(pthread_data->pasos_param[5].ingredientes[2], "cebolla");
        //strcpy(pthread_data->pasos_param[5].ingredientes[3], "carne picada");

	strcpy(pthread_data->pasos_param[6].accion, "cortar_lechuga_tomate");
        strcpy(pthread_data->pasos_param[6].ingredientes[0], "lechuga");
        strcpy(pthread_data->pasos_param[6].ingredientes[1], "tomate");
        //strcpy(pthread_data->pasos_param[6].ingredientes[2], "cebolla");
        //strcpy(pthread_data->pasos_param[6].ingredientes[3], "carne picada");

	strcpy(pthread_data->pasos_param[7].accion, "armar_hamburguesa");
        strcpy(pthread_data->pasos_param[7].ingredientes[0], "ajo");
        strcpy(pthread_data->pasos_param[7].ingredientes[1], "perejil");
        strcpy(pthread_data->pasos_param[7].ingredientes[2], "cebolla");
        strcpy(pthread_data->pasos_param[7].ingredientes[3], "carne picada");


	//inicializo los semaforos
	sem_init(&(pthread_data->semaforos_param.sem_cortar),0,1);
	sem_init(&(pthread_data->semaforos_param.sem_mezclar),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_salar),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_armar_medallones),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_cocinar_plancha),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_cortar_lechuga_tomate),0,0);
        sem_init(&(pthread_data->semaforos_param.sem_aux),0,0);
        //sem_init(&(pthread_data->semaforos_param.sem_armar_hamburguesa),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_hornear_pan),0,0);
	//inicializar demas semaforos aqui


	//creo los hilos a todos les paso el struct creado (el mismo a todos los hilos) ya que todos comparten los semaforos 
	int rc;
	rc = pthread_create(&p1,                           //identificador unico
                            NULL,                          //atributos del thread
                                cortar,             //funcion a ejecutar
                                pthread_data);                     //parametros de la funcion a ejecutar, pasado por referencia
	//crear demas hilos aqui
	pthread_join (p1,NULL);
	//crear join de demas hilos
	//valido que el hilo se alla creado bien 
    	if (rc){
        printf("Error:unable to create thread, %d \n", rc);
        exit(-1);
        }
	
	int rc2;
        rc2 = pthread_create(&p2,                           //identificador unico
                            NULL,                          //atributos del thread
                                mezclar,             //funcion a ejecutar
                                pthread_data);                     //parametros de la funcion a ejecutar, pasado por referencia
        //crear demas hilos aqui
        pthread_join (p2,NULL);
        //crear join de demas hilos
        //valido que el hilo se alla creado bien
        if (rc2){
        printf("Error:unable to create thread, %d \n", rc);
        exit(-1);
        }

	int rc3;
        rc3 = pthread_create(&p3,                           //identificador unico
                            NULL,                          //atributos del thread
                                salar,             //funcion a ejecutar
                                pthread_data);                     //parametros de la funcion a ejecutar, pasado por referencia
        //crear demas hilos aqui
        pthread_join (p3,NULL);
        //crear join de demas hilos
        //valido que el hilo se alla creado bien
        if (rc3){
        printf("Error:unable to create thread, %d \n", rc);
        exit(-1);
        }

	int rc4;
        rc4 = pthread_create(&p4,                           //identificador unico
                            NULL,                          //atributos del thread
                                armar_medallones,             //funcion a ejecutar
                                pthread_data);                     //parametros de la funcion a ejecutar, pasado por referencia
        //crear demas hilos aqui
        pthread_join (p4,NULL);
        //crear join de demas hilos
        //valido que el hilo se alla creado bien
        if (rc4){
        printf("Error:unable to create thread, %d \n", rc);
        exit(-1);
        }

	int rc5;
        rc5 = pthread_create(&p5,                           //identificador unico
                            NULL,                          //atributos del thread
                                cocinar_plancha,             //funcion a ejecutar
                                pthread_data);                     //parametros de la funcion a ejecutar, pasado por referencia
        //crear demas hilos aqui
        pthread_join (p5,NULL);
        //crear join de demas hilos
        //valido que el hilo se alla creado bien
        if (rc5){
        printf("Error:unable to create thread, %d \n", rc);
        exit(-1);
        }

	int rc6;
        rc6 = pthread_create(&p6,                           //identificador unico
                            NULL,                          //atributos del thread
                                hornear_pan,             //funcion a ejecutar
                                pthread_data);                     //parametros de la funcion a ejecutar, pasado por referencia
        //crear demas hilos aqui
        pthread_join (p6,NULL);
        //crear join de demas hilos
        //valido que el hilo se alla creado bien
        if (rc6){
        printf("Error:unable to create thread, %d \n", rc);
        exit(-1);
        }

	int rc7;
        rc7 = pthread_create(&p7,                           //identificador unico
                            NULL,                          //atributos del thread
                                cortar_lechuga_tomate,             //funcion a ejecutar
                                pthread_data);                     //parametros de la funcion a ejecutar, pasado por referencia
        //crear demas hilos aqui
        pthread_join (p7,NULL);
        //crear join de demas hilos
        //valido que el hilo se alla creado bien
        if (rc7){
        printf("Error:unable to create thread, %d \n", rc);
        exit(-1);
        }  
	int rc8;
        rc8 = pthread_create(&p8,                           //identificador unico
                            NULL,                          //atributos del thread
                                armar_hamburguesa,             //funcion a ejecutar
                                pthread_data);                     //parametros de la funcion a ejecutar, pasado por referencia
        //crear demas hilos aqui
        pthread_join (p8,NULL);
        //crear join de demas hilos
        //valido que el hilo se alla creado bien
        if (rc8){
        printf("Error:unable to create thread, %d \n", rc);
        exit(-1);
        }
	
	//destruccion de los semaforos 
	
	sem_destroy(&sem_cortar);
	sem_destroy(&sem_mezclar);
	sem_destroy(&sem_salar);	
	sem_destroy(&sem_armar_medallones);
	sem_destroy(&sem_cocinar_plancha);
        sem_destroy(&sem_cortar_lechuga_tomate);
	sem_destroy(&sem_hornear_pan);
        sem_destroy(&sem_aux);
	//sem_destroy(&sem_armar_hamburguesa); 
	//destruir demas semaforos 
	
	//salida del hilo
	 pthread_exit(NULL);
}


int main ()
{
	//declarar mutex globales
	//creo los nombres de los equipos 
	int rc;
	int *equipoNombre1 =malloc(sizeof(*equipoNombre1));
	int *equipoNombre2 =malloc(sizeof(*equipoNombre2));
	int *equipoNombre3 =malloc(sizeof(*equipoNombre3));
	*equipoNombre1 = 1;
	*equipoNombre2 = 2;
	*equipoNombre3 = 3;

	//creo las variables los hilos de los equipos
	pthread_t equipo1; 
	pthread_t equipo2;
	pthread_t equipo3;
 
	//inicializo los hilos de los equipos
    rc = pthread_create(&equipo1,                           //identificador unico
                            NULL,                          //atributos del thread
                                ejecutarReceta,             //funcion a ejecutar
                                equipoNombre1); 

    rc = pthread_create(&equipo2,                           //identificador unico
                            NULL,                          //atributos del thread
                                ejecutarReceta,             //funcion a ejecutar
                                equipoNombre2);

    rc = pthread_create(&equipo3,                           //identificador unico
                            NULL,                          //atributos del thread
                                ejecutarReceta,             //funcion a ejecutar
                                equipoNombre3);

   if (rc){
       printf("Error:unable to create thread, %d \n", rc);
       exit(-1);
     } 

	//join de todos los hilos
	pthread_join (equipo1,NULL);
	pthread_join (equipo2,NULL);
	pthread_join (equipo3,NULL);
//destruyo todo los mutex globales
pthread_mutex_destroy(&mutex_salar);
pthread_mutex_destroy(&mutex_cocinar_plancha);
pthread_mutex_destroy(&mutex_hornear_pan);
	

//
	printf("El equipo ganador es:%d \n" ,EsGanador);






    pthread_exit(NULL);
}


//Para compilar:   gcc HellsBurgers.c -o ejecutable -lpthread
//Para ejecutar:   ./ejecutable
