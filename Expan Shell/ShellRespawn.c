/**
UNIX Shell Project

Sistemas Operativos
Grados I. Informatica, Computadores & Software
Dept. Arquitectura de Computadores - UMA

Some code adapted from "Fundamentos de Sistemas Operativos", Silberschatz et al.

To compile and run the program:
   $ gcc Shell_project.c job_control.c -o Shell
   $ ./Shell          
	(then type ^D to exit program)

**/

#include "job_control.h"   // remember to compile with module job_control.c 
#include <stdio.h>
#include <string.h>
#include <unistd.h>


#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[36m"

#define MAX_LINE 256 /* 256 chars per line, per command, should be enough. */

// -----------------------------------------------------------------------
//                            MAIN          
// -----------------------------------------------------------------------

//Declaro la lista de jobs que voy a usar para los procesos bg o suspensos como variable global
job * listaProcBgs;
job * listaAlarmas;
int veces;
char claves[100];
int contrasena;

void revivelo(char ** comando){
	/*
				int i=0;
				for(i=0;i<2;i++){
					printf("%s",comando[i]);
				}
	*/	
	//Creo el nuevo proceso y elimino el antiguo
	int pid_fork;
	//Crea un proceso hijo
			pid_fork = fork();
			//==0 es un proceso hijo si no o es el padre o si es -1 error
			if(pid_fork == 0){
				new_process_group(getpid());
				restore_terminal_signals();
				//Controlo que el comando es correcto
					restore_terminal_signals();
					execvp(comando[0],comando);
					exit(-1);
				}
			else{
				//ES el padre
					
					job * proceso = new_job(pid_fork,veces,contrasena,comando[0],comando,RESPAWN);
					if(strcmp(proceso->command,claves)==0){
						//Pongo su bit a 1
						proceso->espiado=1;
					}
					int i;
					for(int i=0;i<100;i++){
						proceso->commando[i]=comando[i];
					}
					block_SIGCHLD();	
					//Le tengo que poner el pid del hijo porque si pongo getpid() esta introduciendo el de la shell y no ejecuta	
					add_job(listaProcBgs,proceso);	
					unblock_SIGCHLD();
					printf("Respawn job running... pid: %d ,command: %s\n",pid_fork,comando[0]);
					fflush(stdout);
			}
}

void mostrar(int senal){
	printf("Caught");
}




int esRespawn(char ** args){
	int a = 0;
	int devuelve = -1;
	while(args[a+1]!=NULL){
		a++;
	}
	
	if(strcmp(args[a],"+")==0){
		devuelve = a;
	}
	return devuelve;
}





void alarmador(int senal){
	
	printf("hula");
	int estado;
	enum status estador;
	int infor;
	int i;
	//printf("ENtra al manejaddor\n");
	for(i=1;i<=list_size(listaProcBgs);i++){
		// Argumentos del waitpid() pid_fork,&status,WUNTRACED
		job * actual = get_item_bypos(listaProcBgs,i);
		int wait_pid = waitpid(actual->pgid,&estado,WNOHANG|WUNTRACED);
		if(wait_pid == actual->pgid){
				//printf("Entra dentro del if\n");
				estador = analyze_status(estado,&infor);
				kill(actual->pgid, SIGKILL);
				delete_job(listaAlarmas, actual);
		}
	}
	
	
}











void manejador(int senal){
	int estado;
	enum status estador;
	int infor;
	int i;
	printf("Caught\n");
	for(i=1;i<=list_size(listaProcBgs);i++){
		// Argumentos del waitpid() pid_fork,&status,WUNTRACED
		job * actual = get_item_bypos(listaProcBgs,i);
		int wait_pid = waitpid(actual->pgid,&estado,WNOHANG|WUNTRACED);
		if(wait_pid == actual->pgid){
				//printf("Entra dentro del if\n");
				estador = analyze_status(estado,&infor);
				if(estador==EXITED && actual->state == RESPAWN){
				//ELimino de la lista e informo
				printf("El proceso respawn termina y vuelve a comenzar\n");
				//printf("%s",actual->commando[0]);
				revivelo(actual->commando);
				
				
				
				delete_job(listaProcBgs,actual);
				}else if(estador==EXITED){
				//ELimino de la lista e informo
				printf("\nEl proceso %d, %s ha acabado\n",actual->pgid,actual->command);
				delete_job(listaProcBgs,actual);
				}else if(estador==SUSPENDED){
				printf("\nEl proceso %d, %s suspendido se ha actualizado a STOPPED\n",actual->pgid,actual->command);
				//Cambio el estado de suspendido por stopped
				actual->state = STOPPED;
				}else if(estador==SIGNALED){
				printf("\nEl proceso %d, %s en signaled se elimina\n",actual->pgid,actual->command);
				delete_job(listaProcBgs,actual);
				}
				
				
	}
}
}









int main(void)
{
	veces = 0;
	contrasena = 0;
	int tamInterno = 0;
	char interno[10][10];//Numero de string y tamaño de cada uno de ellos
	strcpy(interno[tamInterno],"cd");
	tamInterno++;
	strcpy(interno[tamInterno],"jobs");
	tamInterno++;
	strcpy(interno[tamInterno],"fg");
	tamInterno++;
	strcpy(interno[tamInterno],"bg");
	tamInterno++;
	strcpy(interno[tamInterno],"respawn");
	tamInterno++;
	
	
	char inputBuffer[MAX_LINE]; /* buffer to hold the command entered */
	int background;             /* equals 1 if a command is followed by '&' */
	char *args[MAX_LINE/2];     /* command line (of 256) has max of 128 arguments */
	// probably useful variables:
	int pid_fork, pid_wait; /* pid for created and waited process */
	int status;             /* status returned by wait */
	enum status status_res; /* status processed by analyze_status() */
	int info;				/* info processed by analyze_status() */
	char localizacion[250];  /*Indica la localizacion en la que me encuentro*/

	//Ignoro señales
	ignore_terminal_signals();
	//Inicializo la lista para los procesos bg o suspensos
	listaProcBgs = new_list("Lista de procesos en segundo plano o suspendidos\n");
	listaAlarmas = new_list("Lista de procesos programados con alarmas\n");
	signal(SIGCHLD,manejador);
	signal(SIGALRM,alarmador);

	while (1)   /* Program terminates normally inside get_command() after ^D is typed*/
	{   	
		
		getcwd(localizacion, sizeof(localizacion));	
		printf(ANSI_COLOR_GREEN "ShellIvan:" ANSI_COLOR_RESET);
		printf(ANSI_COLOR_MAGENTA "%s$ " ANSI_COLOR_RESET ,localizacion);	
		fflush(stdout);
		get_command(inputBuffer, MAX_LINE, args, &background);  /* get next command */
		
		if(args[0]==NULL) {
			continue;   // if empty command
		}
		
		if(strcmp(args[0],"^d")==0 || strcmp(args[0], "exit")==0 ){
				printf("Bye\n");
				return 0;
		}
		
		/*EN CONSTRUCCION 
		if(strcmp(args[0],"drop+")==0){
			
			
			if(claves == NULL){
				printf("Error inserte una clave antes");
			}else{
					//Recorro la lista de jobs y voy eliminando
					int a=1;
					for(a=1;a<=list_size(listaProcBgs);a++){
					
						job * actual = get_item_bypos(listaProcBgs,a);
						
						
						//Tengo que recorrer su comando
						int b = 0;
						int siguiente = -1;
						while(actual->commando[b]!=NULL && siguiente == -1){
						
							if(strcmp(actual->commando[b],claves)==0){
						
								//ELimina el proceso
								killpg(actual->pgid,SIGKILL);
								siguiente = 0;
							}
							b++;
						}
						siguiente = -1;
					}
					
					
				
			}
		
		
		*/
		if(strcmp(args[0],"drop")==0){
		
		
			if(claves == NULL){
				printf("Error inserte una clave antes");
			}else{
					//Recorro la lista de jobs y voy eliminando
					int a=1;
					for(a=1;a<=list_size(listaProcBgs);a++){
					
						job * actual = get_item_bypos(listaProcBgs,a);
					
						if(strcmp(actual->command,claves)==0){
						
							//ELimina el proceso
							killpg(actual->pgid,SIGKILL);
							
						}
					
						
					}
					
					
				
			}
		
		
		
		}else if(strcmp(args[0],"unspy")==0){
			
			int as= 1;
				while(as<=list_size(listaProcBgs)){
					job * actual = get_item_bypos(listaProcBgs,as);
					if(strcmp(actual->command,claves)==0){
						//Pongo su bit a 0
						actual->espiado=0;
					}
					as++;
			}
			
			strcpy(claves,"");
		
		
		}else if(strcmp(args[0],"spy")==0){
			if(args[1]==NULL){
				printf("%s\n",claves);
			}else{
				//Recorro la lista para cambiar el bit del de antes
				
				int as= 1;
				while(as<=list_size(listaProcBgs)){
					job * actual = get_item_bypos(listaProcBgs,as);
					if(strcmp(actual->command,claves)==0){
						//Pongo su bit a 0
						actual->espiado=0;
					}
					as++;
				}
				//Implementacion
				//Modifico la cadena por el argumento spia
				strcpy(claves,args[1]);
				//Recorro la lista de procesos y cambio el bit en la que sea
				
				int a= 1;
				while(a<=list_size(listaProcBgs)){
					job * actual = get_item_bypos(listaProcBgs,a);
					if(strcmp(actual->command,claves)==0){
						//Pongo su bit a 1
						actual->espiado=1;
					}
					a++;
				}
			}
		
		
		
		}else if(esRespawn(args)!=-1){
			printf("Ejecutando el comando en respawn\n");
			char ** reserva = malloc(1000);
			int i;
			for(i=0;i<10;i++){
				reserva[i]=malloc(100);
			}
			int a=0;
			while(args[a]!= NULL){
				reserva[a] = strdup(args[a]);
				a++;	
			}
			reserva[a-1]=NULL;
			//Pasos a seguir:
			//modificar job_control para meterle los argumentos
			//--Prueba de la modificacion--
			args[esRespawn(args)]=NULL;
			//Crea un proceso hijo
			pid_fork = fork();
			//==0 es un proceso hijo si no o es el padre o si es -1 error
			if(pid_fork == 0){
				new_process_group(getpid());
				restore_terminal_signals();
				//Controlo que el comando es correcto
				if(execvp(reserva[0],reserva)<0){
					printf(ANSI_COLOR_RED "El comando %s no existe\n" ANSI_COLOR_RESET ,reserva[0]);
					exit(-1);
				}else{
					restore_terminal_signals();
					execvp(reserva[0],reserva);
					exit(-1);
				}
			}else{
				//ES el padre
					job * proceso = new_job(pid_fork,veces,contrasena,reserva[0],reserva,RESPAWN);
					if(strcmp(proceso->command,claves)==0){
						//Pongo su bit a 1
						proceso->espiado=1;
					}
					int i;
					for(int i=0;i<100;i++){
						proceso->commando[i]=reserva[i];
					}
					block_SIGCHLD();	
					//Le tengo que poner el pid del hijo porque si pongo getpid() esta introduciendo el de la shell y no ejecuta	
					add_job(listaProcBgs,proceso);	
					unblock_SIGCHLD();
					printf("Respawn job running... pid: %d ,command: %s\n",pid_fork,reserva[0]);
					fflush(stdout);
			}
			
			


		}else if(strcmp(args[0],"time-out")==0){
			
			if(args[1]==NULL){
				printf("Error en la introduccion del comando");
			}else{
				
				int alarma = atoi(args[1]);
		
				//printf("Es cronometrado\n");
				//alarm(10);
				
				int a = 0;
				int b = 2;
				
				while(args[b]!=NULL){
					args[a]=args[b];
					a++;
					b++;
				}
				
				args[a]=args[b];
				//Creo el comando y aviso a la alarma
				//Crea un proceso hijo
				pid_fork = fork();
				//==0 es un proceso hijo si no o es el padre o si es -1 error
				if(pid_fork == 0){
					new_process_group(getpid());
					restore_terminal_signals();
					//Controlo que el comando es correcto
					if(execvp(args[0],args)<0){
						printf(ANSI_COLOR_RED "El comando %s no existe\n" ANSI_COLOR_RESET ,args[0]);
						exit(-1);
					}else{
						if(!background){
							set_terminal(getpid()); //Tomo el control si es foreground
						}else{
							//ESta en background
							//bloqueo las señales al crear el job porque modifico las estructura de datos lista
							//luego desbloqueo
						}
						//El comando es correcto
						//Exec primero el argumento y luego el resto de la cadena
						restore_terminal_signals();
						execvp(args[0],args);
						exit(-1);
					}
				}else{
					//ES el padre
					alarm(alarma);
					if(!background){
						new_process_group(pid_fork);
						set_terminal(pid_fork);
						waitpid(pid_fork,&status,WUNTRACED); //ESpera al hijo
						set_terminal(getpid());
						status_res = analyze_status(status,&info);
						//Compruebo si se suspende
						if(strcmp(status_strings[status_res],"Suspended")==0){
							job * proceso = new_job(pid_fork,veces,contrasena,args[0],args,STOPPED);
							if(strcmp(proceso->command,claves)==0){
						//Pongo su bit a 1
						proceso->espiado=1;
					}
							block_SIGCHLD();
							//proceso->state = STOPPED;
							add_job(listaAlarmas,proceso);
							unblock_SIGCHLD();
						}
						printf("Foreground pid: %d ,command: %s , %s , info: %d\n",pid_fork,args[0],status_strings[status_res],info);
						fflush(stdout);
				}else{
						job * proceso = new_job(pid_fork,veces,contrasena,args[0],args,BACKGROUND);
						if(strcmp(proceso->command,claves)==0){
						//Pongo su bit a 1
						proceso->espiado=1;
					}
						block_SIGCHLD();	
						//Le tengo que poner el pid del hijo porque si pongo getpid() esta introduciendo el de la shell y no ejecuta	
						add_job(listaAlarmas,proceso);	
						unblock_SIGCHLD();
						printf("Background job running... pid: %d ,command: %s\n",pid_fork,args[0]);
						fflush(stdout);
				}
			  }
				
			}
			
	
			
			
			
			
			
			
		
		
		
		
		
		}else if(strcmp(args[0],"type")==0){
			if(args[1]==NULL){
				printf("Inserte un comando valido\n");
			}else{
				int i;
				int devuelve = 0;
				for(int i=0;i<tamInterno;i++){
					if(strcmp(args[1],interno[i])==0){
						devuelve++;
					}	
				}
				if(devuelve!=0){
					printf("%s en un comando interno\n",args[1]);
				}else{
					printf("El comando es externo o no existe\n");
				}
			}
		
		
		
		
		}else if(strcmp(args[0],"cd") == 0){
			//IMplementacion del comando interno CD"
			//Como el comando cd se encuentra en args[0] la ruta esta en args[1]
			int resultado = chdir(args[1]);
			
			if(args[1]==NULL){
				chdir("/");
			}else if(resultado != 0){
				printf(ANSI_COLOR_RED "Error la ruta indicada no existe\n" ANSI_COLOR_RESET);
			}else{
				chdir(args[1]);
			}
			
			
			
			
		}else if(strcmp(args[0],"jobs")==0){
			//Mostrar las tareas de la lista o si on hay indicarlo
			//LA funcion es vacia retorna 1 si lo es
			if(empty_list(listaProcBgs)==1){
				printf("No hay procesos en segundo plano o suspendidos\n");
			}else{
				//Recorro la lista
				print_job_list(listaProcBgs);				
			}
			
			
			
			
		}else if(strcmp(args[0],"fg")==0){
			//En args[1] identificador lugar ocupa en la lista
			//si args[1] cojer la primera de la lista
			enum status estador;
			//Combierto a un en entero ese char de args[1] y luego trabajo sobre el
			//Cuando existen errores en los char por ejemplo el de args[1] te da una violacion de segmento
			int posicion;
			if(args[1]==NULL){
				posicion =1; //Primera posicion de la lista
			}else{
				posicion = atoi(args[1]);
			}
			//AHora compruebo que el valor sea valido
			if(list_size(listaProcBgs)==0){
				printf(ANSI_COLOR_RED "Error la lista de procesos esta vacia\n" ANSI_COLOR_RESET);
			}else if(list_size(listaProcBgs) < posicion){
				//Error la posicion no es valida
				printf(ANSI_COLOR_RED "Error el numero de proceso indicado no es valido\n" ANSI_COLOR_RESET);
			}else{
					//en posicion ya tengo la posicion valida que tengo que modificar
					//cojo el job que hay en esa posicion
					job * actual = get_item_bypos(listaProcBgs,posicion);
					//Cambio en su estructura su estado
					actual->state = FOREGROUND;
					set_terminal(actual->pgid); //Le cedo la señal a ese proceso
					printf("Puesto en foreground el job %d con la funcion %s que estaba suspendido\n",actual->pgid,actual->command);
					killpg(actual->pgid,SIGCONT);
					waitpid(actual->pgid,&status,WUNTRACED);
					set_terminal(getpid());
					estador = analyze_status(status,&info);
					if(strcmp(status_strings[estador],"Suspended")==0){	
						actual->state = STOPPED;
					}else{
						block_SIGCHLD();
						printf("\nEl proceso %d, %s termina\n",actual->pgid,actual->command);
						delete_job(listaProcBgs,actual);
						unblock_SIGCHLD();
					}
			}
			
			
		
			
			
			
		}else if(strcmp(args[0],"bg")==0){
			enum status estador;
			int posicion;
			if(args[1]==NULL){
				posicion =1; //Primera posicion de la lista
			}else{
				posicion = atoi(args[1]);
			}
			//AHora compruebo que el valor sea valido
			if(list_size(listaProcBgs)==0){
				printf(ANSI_COLOR_RED "Error la lista de procesos esta vacia\n" ANSI_COLOR_RESET);
			}else if(list_size(listaProcBgs) < posicion){
				//Error la posicion no es valida
				printf(ANSI_COLOR_RED "Error el numero de proceso indicado no es valido\n" ANSI_COLOR_RESET);
			}else{
					//en posicion ya tengo la posicion valida que tengo que modificar
					//cojo el job que hay en esa posicion
					job * actual = get_item_bypos(listaProcBgs,posicion);
					actual->veces = actual->veces+1;
					//printf("%d",actual->veces);
					//Compruebo que la tarea seleccionada este suspendida
					
					
					//if(actual->state != STOPPED || actual->state != RESPAWN){
					//	printf(ANSI_COLOR_RED "La tarea seleccionada no esta suspendida\n" ANSI_COLOR_RESET);
					//}else{			
						//Cambio en su estructura su estado
						actual->state = BACKGROUND;
						printf("Puesto en background el job %d veces; %d con la funcion %s que estaba suspendido\n",actual->pgid,actual->veces,actual->command);
						killpg(actual->pgid,SIGCONT);
						continue;
					//}
			}
			
			
			
			
			
			
			
			
			
			
		}else{

			if(strcmp(claves,args[0])==0){
				printf("Este comando esta siendo espiado\n");
			}

			//Crea un proceso hijo
			pid_fork = fork();
			//==0 es un proceso hijo si no o es el padre o si es -1 error
			if(pid_fork == 0){
				new_process_group(getpid());
				restore_terminal_signals();
				//Controlo que el comando es correcto
				if(execvp(args[0],args)<0){
					printf(ANSI_COLOR_RED "El comando %s no existe\n" ANSI_COLOR_RESET ,args[0]);
					exit(-1);
				}else{
					if(!background){
						set_terminal(getpid()); //Tomo el control si es foreground
					}else{
						//ESta en background
						//bloqueo las señales al crear el job porque modifico las estructura de datos lista
						//luego desbloqueo
					}
					//El comando es correcto
					//Exec primero el argumento y luego el resto de la cadena
					restore_terminal_signals();
					execvp(args[0],args);
					exit(-1);
				}
			}else{
				//ES el padre
				if(!background){
					new_process_group(pid_fork);
					set_terminal(pid_fork);
					waitpid(pid_fork,&status,WUNTRACED); //ESpera al hijo
					set_terminal(getpid());
					status_res = analyze_status(status,&info);
					//Compruebo si se suspende
					if(strcmp(status_strings[status_res],"Suspended")==0){
						job * proceso = new_job(pid_fork,veces,contrasena,args[0],args,STOPPED);
						if(strcmp(proceso->command,claves)==0){
						//Pongo su bit a 1
						proceso->espiado=1;
					}
						block_SIGCHLD();
						//proceso->state = STOPPED;
						add_job(listaProcBgs,proceso);
						unblock_SIGCHLD();
					}
					printf("Foreground pid: %d ,command: %s , %s , info: %d\n",pid_fork,args[0],status_strings[status_res],info);
					fflush(stdout);
			}else{
					job * proceso = new_job(pid_fork,veces,contrasena,args[0],args,BACKGROUND);
					if(strcmp(proceso->command,claves)==0){
						//Pongo su bit a 1
						proceso->espiado=1;
					}
					block_SIGCHLD();	
					//Le tengo que poner el pid del hijo porque si pongo getpid() esta introduciendo el de la shell y no ejecuta	
					add_job(listaProcBgs,proceso);	
					unblock_SIGCHLD();
					printf("Background job running... pid: %d ,command: %s\n",pid_fork,args[0]);
					fflush(stdout);
			}
			
		}
		
	}

	} // end while
}
