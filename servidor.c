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

#define PORT 9001
#define ARR_SIZE 256
#define BUFFER_SIZE 1024

int main(int argc, char const* argv[])
{
	inicializar_tiempo_servidor();
	//crear un servidor socket similar a lo que se hizo en el programa cliente
	int servSockD = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in servAddr; 	//definir la dirección del servidor

	//llenar la estructura con la información del servidor
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(PORT);
	servAddr.sin_addr.s_addr = INADDR_ANY;
	
	//bind socket a la IP y puerto especificados 
	bind(servSockD, (struct sockaddr*)&servAddr, sizeof(servAddr));
	listen(servSockD, 1); //escuchar para conexiones

	while(1) { // Aceptar solicitudes al servidor
		int clientSocket = accept(servSockD, NULL, NULL); //numero int para guardar el socket del cliente

		if (clientSocket < 0) {
			perror("Error en accept.\n");
			continue;  // Continuar esperando más conexiones
		} else {
			printf("Cliente conectado exitosamente.\n");
		}

		while(1) { // creacion de un nuevo proceso al realizarse una nueva peticion al servidor
			pid_t pid = fork();
			assert(pid != -1);

			if (pid == 0) {
			
				char strData[ARR_SIZE] = {0}; //definimos un array para guardar el comando del cliente

				char filename[ARR_SIZE] = {0};
				char response[BUFFER_SIZE] = {0};

				ssize_t bytesRead = recv(clientSocket, strData, sizeof(strData) - 1, 0);

				if (bytesRead <= 0) {
					exit(EXIT_FAILURE); // Error o desconexión del cliente
				}

				strData[strcspn(strData, "\n")] = '\0'; // Eliminar saltos de línea o espacios finales

				memset(response, 0, sizeof(response));

				if (strcmp(strData, "salida") == 0){
					strcpy(response, SALIDA);
					send(clientSocket, response, sizeof(response), 0);
					exit(EXIT_FAILURE);
				}

				else if (strcmp(strData, "comandos") == 0){
					strcpy(response, COMANDOS);
				}

				else if (strcmp(strData, "ls") == 0){
					ejecutar_ls(response);
				}
 
				else if (strncmp(strData, "cd ", 3) == 0) {
					char *ruta = strData + 3;
					ejecutar_cd(ruta, response);
				}

				else if (strncmp(strData, "cat ", 4) == 0) {
					sscanf(strData + 4, "%s", filename); // extraer el nombre del archivo
					ejecutar_cat(filename, response);
				}

				else if (strcmp(strData, "pwd") == 0) {
					ejecutar_pwd(response);
				}

				else if (strcmp(strData, "tree") == 0) {
					// Implementar funcionalidad de tree
					char path[BUFFER_SIZE] = {0};
					ejecutar_pwd(path);
					ejecutar_tree(path, response, 1);
				}

				else if (strncmp(strData, "head ", 5) == 0) {
					int num_lines = 10; // Número de líneas por defecto
					sscanf(strData + 5, "%s", filename);

					ejecutar_head(filename, response, num_lines);
				}

				else if (strcmp(strData, "uptime") == 0) {
					ejecutar_uptime(response);
				}

				else if (strcmp(strData, "date") == 0) {
					ejecutar_date(response);
				}

				else if (strncmp(strData, "calc ", 5) == 0) {
					ejecutar_calc(strData + 5, response);
				}

				else {
					strcpy(response, NOT_FOUND);
				}

				if (send(clientSocket, response, sizeof(response), 0) < 0) {
 					perror("Error enviando datos al cliente");
					close(clientSocket);
				}
			} 

			else {
				int status;
				wait(&status); // Espera al hijo y captura su estado

				if (WIFEXITED(status) && WEXITSTATUS(status) == 1) {
					// Si el hijo indica "salida == exit(1) == exit(EXIT_FAILURE)", cerrar el bucle
					close(clientSocket); //cierra el socket del cliente
					printf("cerrando la conexión con el cliente.\n");
					break;
				}
			}

		}
	}

	close(servSockD); // Cierra el socket del servidor
	return 0;
}
