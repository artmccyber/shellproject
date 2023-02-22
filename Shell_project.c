/**


Sistemas Operativos
Grados I. Informatica, Computadores & Software
Dept. Arquitectura de Computadores - UMA

Some code adapted from "Fundamentos de Sistemas Operativos", Silberschatz et al.

To compile and run the program:
   $ gcc Shell_project.c job_control.c -o Shell
   $ ./Shell
	(then type ^D to exit program)

**/
#include <string.h>
#include "job_control.h" // remember to compile with module job_control.c

#define MAX_LINE 256 /* 256 chars per line, per command, should be enough. */

job *tareas; // Definicmos la lista de tareas

void manejador(int sen)
{
	job *item;
	int status, info;
	int pid_wait = 0; // Para ver si hemos recogido un proceso
	enum status status_res;

	for (int i = 1; i <= list_size(tareas); i++)
	{																  // Iteramos sobre la lista de tareas
		item = get_item_bypos(tareas, i);							  // Extraemos un item por iteración
		pid_wait = waitpid(item->pgid, &status, WUNTRACED | WNOHANG); // Mira por tareas que han cambiado de estado y recoge el pid
		if (pid_wait == item->pgid)
		{												// Si coincide el pid recogido
			status_res = analyze_status(status, &info); // Analizo su estado
			if (status_res == SUSPENDED)				// Si es suspendido
			{
				printf("\nBackground pid: %d, command: %s, suspend exec\n", item->pgid, item->command); // Muestro el pid asociado al SIGCHLD
				item -> state = STOPPED;																// Cambio su estado a detenido
			}
			else if (status_res = EXITED) // Si es finalizado
			{
				printf("\nBackground command: %d, command: %s. Finish\n", item->pgid, item->command); // Muestro el pid asociado al SIGCHLD
				delete_job(tareas, item);														  // Lo elimino de las tareas
			}
		}
	}
}

// -----------------------------------------------------------------------
//                            MAIN
// -----------------------------------------------------------------------

int main(void)
{
	char inputBuffer[MAX_LINE]; /* buffer to hold the command entered */
	int background;				/* equals 1 if a command is followed by '&' */
	char *args[MAX_LINE / 2];	/* command line (of 256) has max of 128 arguments */
	// probably useful variables:
	int pid_fork, pid_wait; /* pid for created and waited process */
	int status;				/* status returned by wait */
	enum status status_res; /* status processed by analyze_status() */
	int info;				/* info processed by analyze_status() */
	job *item;				/* job to insert in list of jobs to manage*/

	ignore_terminal_signals();
	signal(SIGCHLD, manejador); // Manejamos SIGCHLD

	tareas = new_list("JOBS");

	while (1) /* Program terminates normally inside get_command() after ^D is typed*/
	{
		printf("COMMAND->");
		fflush(stdout);
		get_command(inputBuffer, MAX_LINE, args, &background); /* get next command */

		if (args[0] == NULL)
			continue;
		 // if empty command

			/* the steps are:
				 (1) fork a child process using fork()
				 (2) the child process will invoke execvp()
				 (3) if background == 0, the parent will wait, otherwise continue
				 (4) Shell shows a status message for processed command
				 (5) loop returns to get_commnad() function
			*/

			if (!strcmp(args[0], "cd"))
		{ // Tratamiento commando interno <cd>
			chdir(args[1]);
			continue;
		}
		else if (!strcmp(args[0], "logout"))
		{ // Tratamiento comando interno <logout>
			exit(0);
		}
		else
		{
			pid_fork = fork(); // Generamos proceso hijo identico
			if (pid_fork > 0)
			{ // Proceso padre
				if (background == 0)
				{												// Ejecutado en primer plano
					waitpid(pid_fork, &status, WUNTRACED);		// Esperar cambio de estado del hijo
					set_terminal(getpid());						// Devolvemos la terminal al proceso padre
					status_res = analyze_status(status, &info); // Analizamos estado
					if (status_res == SUSPENDED)
					{												// Si el proceso ha sido suspendido
						item = new_job(pid_fork, args[0], STOPPED); // Creamos un nuevo job
						add_job(tareas, item);						// Y lo añadimos a la lista de tareas
						printf("\nForeground pid: %d, command: %s, %s, info: %d\n",
							   pid_fork, args[0], status_strings[status_res], info); // Foregorund command
					}
					else if (status_res = EXITED)
						if (info != 255)
						{
							printf("\nForeground pid: %d, command: %s, %s, info: %d\n",
								   pid_fork, args[0], status_strings[status_res], info); // Foregorund command
						}
				}
				else
				{
					item = new_job(pid_fork, args[0], BACKGROUND);
					add_job(tareas, item);
					printf("\nBackground job running... pid: %d, command: %s\n",
						   pid_fork, args[0]); // Background command
				}
			}
			else
			{								 // Proceso hijo
				new_process_group(getpid()); // Set gpid para que el terminal pueda asiganrse a una sola tarea por momento
				if (background == 0)
				{ // Si estamos en primer plano
					set_terminal(getpid());
				}
				restore_terminal_signals();
				execvp(args[0], args);								 // Ejecutamos comando recibido
				printf("\nError, command not found: %s\n", args[0]); // Error command
				exit(-1);											 // Salimos si execvp() no es ejecutado. Es decir error command
			}
		}
	} // end while
}
