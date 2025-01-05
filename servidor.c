#include <netinet/in.h> //estructura para guardar la información de dirección
#include <stdio.h>
#include <string.h> //para la función strcmp()
#include <stdlib.h>
#include <sys/socket.h> //para la APIs de socket
#include <unistd.h> //para la función close()
#include <dirent.h> //para el comando ls
#include <errno.h>
#include <assert.h>
#include <sys/wait.h> //para usar wait()
#include "comandos.h"

// comandos disponiebles en la powershell
char comandos[] =			
	"Comandos disponibles:\n"
	"1. salida: cerrar la conexión\n"
	"2. ls: listar archivos\n"
	"3. pwd: mostrar el directorio actual\n"
	"4. cat <archivo>: mostrar el contenido de un archivo\n"
	"5. cd <directorio>: cambiar el directorio actual\n"
	"6. tree: mostrar estructura de directorios\n"
	"7. head <archivo>: mostrar las primeras lineas de un archivo\n"
	"8. tail <archivo>: mostrar las ultimas lineas de un archivo\n"
	"9. uptime : muestra el tiempo que lleva el servidor funcionando\n"
	"10. date : muestra la fecha del sistema\n"
	"11. calc : realiza una operacion sencilla";

int main(int argc, char const* argv[])
{
	inicializar_tiempo_servidor();
	//crear un servidor socket similar a lo que se hizo en el programa cliente
	int servSockD = socket(AF_INET, SOCK_STREAM, 0);
	if (servSockD < 0) {
    		perror("Error al crear el socket del servidor");
    		return EXIT_FAILURE;
	}

	struct sockaddr_in servAddr; 	//definir la dirección del servidor

	//llenar la estructura con la información del servidor
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(9001);
	servAddr.sin_addr.s_addr = INADDR_ANY;

	//bind socket a la IP y puerto especificados
	if (bind(servSockD, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
	    perror("Error al enlazar el socket a la dirección");
	    close(servSockD);
	    return EXIT_FAILURE;
	}
	listen(servSockD, 1); //escuchar para conexiones

	while(1) { // Aceptar solicitudes al servidor
		int clientSocket = accept(servSockD, NULL, NULL); //numero int para guardar el socket del cliente

		if (clientSocket < 0) {
			perror("Error al aceptar la conexión del cliente");
			continue;  // Continuar esperando más conexiones
		}
		printf("Cliente conectado exitosamente.\n");


		while(1) { // creacion de un nuevo proceso al realizarse una nueva peticion al servidor
			pid_t pid = fork();
			assert(pid != -1);

			if (pid == 0) {
				char strData[255]; //definimos un array para guardar el comando del cliente

				char filename[255];
				char response[2048];

				memset(strData, 0, sizeof(strData)); // Limpiar buffer
				ssize_t bytesRead = recv(clientSocket, strData, sizeof(strData) - 1, 0);

				if (bytesRead <= 0) {
				    printf("El cliente ha cerrado la conexión.\n");
   				    close(clientSocket);
    				    exit(1);
				}

				strData[bytesRead] = '\0'; // Aseguramos que el buffer sea una cadena terminada en '\0'

				strData[strcspn(strData, "\n")] = '\0'; // Eliminar saltos de línea o espacios finales

				if (strcmp(strData, "salida") == 0){
					char response[255] = "Gracias por usar nuestra shell remota"; //definimos un array para guardar la respuesta del servidor
					send(clientSocket, response, strlen(response), 0);
					close(clientSocket); //cierra el socket del cliente
					exit(1);
				}
				else if (strcmp(strData, "comandos") == 0){
					send(clientSocket, comandos, strlen(comandos), 0);
				}
				else if (strcmp(strData, "ls") == 0){
					ejecutar_ls(response);
					if (send(clientSocket, response, sizeof(response), 0) < 0) {
 					   perror("Error enviando datos al cliente");
					}
				}
				else if (strncmp(strData, "cd ", 3) == 0) {
					char *ruta = strData + 3;

					ejecutar_cd(ruta, response);
					if (send(clientSocket, response, sizeof(response), 0) < 0) {
 					   perror("Error enviando datos al cliente");
					}
				}

				else if (strncmp(strData, "cat ", 4) == 0) {
					sscanf(strData + 4, "%s", filename); // extraer el nombre del archivo

					ejecutar_cat(filename, response);
					if (send(clientSocket, response, sizeof(response), 0) < 0) {
 					   perror("Error enviando datos al cliente");
					}
				}

				else if (strcmp(strData, "pwd") == 0) {
					ejecutar_pwd(response);
					if (send(clientSocket, response, sizeof(response), 0) < 0) {
 					   perror("Error enviando datos al cliente");
					}
				}

				else if (strcmp(strData, "tree") == 0) {
					// Implementar funcionalidad de tree
					char path[1024];
					ejecutar_pwd(path);
					ejecutar_tree(path, response, 1);
					if (send(clientSocket, response, sizeof(response), 0) < 0) {
 					   perror("Error enviando datos al cliente");
					}
				}

				else if (strncmp(strData, "head ", 5) == 0) {
					int num_lines = 10; // Número de líneas por defecto
					sscanf(strData + 5, "%s", filename);

					ejecutar_head(filename, response, num_lines);
					if (send(clientSocket, response, sizeof(response), 0) < 0) {
 					   perror("Error enviando datos al cliente");
					}
				}

				// else if (strncmp(strData, "tail ", 5) == 0) {
				// 	int num_lines = 10; // Número de líneas por defecto
				// 	sscanf(strData + 5, "%s", filename);

				// 	ejecutar_tail(filename, response, num_lines);
				// 	send(clientSocket, response, sizeof(response), 0);
				// }

				else if (strcmp(strData, "uptime") == 0) {
					ejecutar_uptime(response);
					if (send(clientSocket, response, sizeof(response), 0) < 0) {
 					   perror("Error enviando datos al cliente");
					}
				}

				else if (strcmp(strData, "date") == 0) {
					ejecutar_date(response);
					if (send(clientSocket, response, sizeof(response), 0) < 0) {
 					   perror("Error enviando datos al cliente");
					}
				}

				else if (strncmp(strData, "calc ", 5) == 0) {
					ejecutar_calc(strData + 5, response);
					if (send(clientSocket, response, sizeof(response), 0) < 0) {
 					   perror("Error enviando datos al cliente");
					}
				}

				else {
					char response[] = "Comando no reconocido"; //definimos un array para guardar la respuesta del servidor
					send(clientSocket, response, strlen(response), 0); //manda al cliente el mensaje comando no reconocido
				}
			}

			else {
				int status;
				wait(&status); // Espera al hijo y captura su estado
				if (WIFEXITED(status)) {
 				   printf("Proceso hijo terminado con código: %d\n", WEXITSTATUS(status));
				}
				if (WIFEXITED(status) && WEXITSTATUS(status) == 1) {
					// Si el hijo indica "salida == exit(1)", cerrar el bucle
					printf("cerrando la conexión con el cliente.\n");
					break;
				}
			}
		}
	}

	close(servSockD); // Cierra el socket del servidor
	return 0;
}
