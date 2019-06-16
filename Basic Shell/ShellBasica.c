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

void manejador(int senal){
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
				if(estador==EXITED){
				//ELimino de la lista e informo
				printf("\nEl proceso %d, %s ha acabado\n",actual->pgid,actual->command);
				printf("Foreground pid: %d ,command: %s , %s , info: %d\n",actual->pgid,actual->command,status_strings[estador],infor);
				delete_job(listaProcBgs,actual);
				}else if(estador==SUSPENDED){
				printf("\nEl proceso %d, %s suspendido se ha actualizado a STOPPED\n",actual->pgid,actual->command);
				printf("Background pid: %d ,command: %s , %s , info: %d\n",actual->pgid,actual->command,status_strings[estador],infor);
				//Cambio el estado de suspendido por stopped
				actual->state = STOPPED;
				}else if(estador==SIGNALED){
				printf("\nEl proceso %d, %s en signaled se elimina\n",actual->pgid,actual->command);
				printf("Foreground pid: %d ,command: %s , %s , info: %d\n",actual->pgid,actual->command,status_strings[estador],infor);
				delete_job(listaProcBgs,actual);
				}
	}
}
}









int main(void)
{
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
	signal(SIGCHLD,manejador);

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

		
		
		//IMplementacion del comando interno CD"
		if(strcmp(args[0],"cd") == 0){
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
						printf("He suspendido al proceso %d, %s\n",actual->pgid, actual->command);
						printf("Foreground pid: %d ,command: %s , %s , info: %d\n",actual->pgid,actual->command,status_strings[estador],info);
						block_SIGCHLD();
						actual->state = STOPPED;
						unblock_SIGCHLD();
					}else if(strcmp(status_strings[estador],"Exited")==0){
						block_SIGCHLD();
						printf("\nEl proceso %d, %s termina\n",actual->pgid,actual->command);
						printf("Foreground pid: %d ,command: %s , %s , info: %d\n",actual->pgid,actual->command,status_strings[estador],info);
						delete_job(listaProcBgs,actual);
						unblock_SIGCHLD();
					}else if(strcmp(status_strings[estador],"Signaled")==0 ){
						printf("\nHe matado a %d,%s con una señal\n",actual->pgid,actual->command);
						printf("Foreground pid: %d ,command: %s , %s , info: %d\n",actual->pgid,actual->command,status_strings[estador],info);
						block_SIGCHLD();
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
					//Compruebo que la tarea seleccionada este suspendida
					if(actual->state != STOPPED){
						printf(ANSI_COLOR_RED "La tarea seleccionada no esta suspendida\n" ANSI_COLOR_RESET);
					}else{			
						//Cambio en su estructura su estado
						actual->state = BACKGROUND;
						printf("Puesto en background el job %d con la funcion %s que estaba suspendido\n",actual->pgid,actual->command);
						killpg(actual->pgid,SIGCONT);
						continue;
					}
			}
			
			
			
			
			
			
			
			
			
			
		}else{



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
						job * proceso = new_job(pid_fork,args[0],STOPPED);
						block_SIGCHLD();
						//proceso->state = STOPPED;
						add_job(listaProcBgs,proceso);
						unblock_SIGCHLD();
					}
					printf("Foreground pid: %d ,command: %s , %s , info: %d\n",pid_fork,args[0],status_strings[status_res],info);
					fflush(stdout);
			}else{
					job * proceso = new_job(pid_fork,args[0],BACKGROUND);
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
