#include <netinet/in.h> //estructura para guardar la información de dirección
#include <stdio.h>
#include <string.h> //para la función strcmp()
#include <stdlib.h>
#include <sys/socket.h> //para la APIs de socket
#include <unistd.h> //para la función close()
#include <assert.h>
#include <arpa/inet.h>
#include <sys/wait.h> //para usar wait()

#define PORT 9001
#define IP_SERVIDOR "192.168.0.7"
#define COM_SIZE 256
#define BUFFER_SIZE 1024

int main(int argc, char const* argv[])
{
	//------Conectarse------

	//crear un socket
	int sockD = socket(AF_INET, SOCK_STREAM, 0);
    if (sockD < 0) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

	//definir la dirección del servidor
	struct sockaddr_in servAddr;

	//llenar la estructura con la información del servidor
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(PORT); //usar un puerto que no se esté usando
	servAddr.sin_addr.s_addr = INADDR_ANY;

	// conexion al equipo que funciona como servidor
	
	if (inet_pton(AF_INET, IP_SERVIDOR, &servAddr.sin_addr) <= 0) {
    	perror("Dirección IP inválida o no soportada");
    	close(sockD);
    	exit(EXIT_FAILURE);
    }

	//------Read / Write------
	
	//si connect devuelve -1 no hubo conexión, si devuelve 0 si se conectó exitosamente
	int connectStatus = connect(sockD, (struct sockaddr*)&servAddr,	sizeof(servAddr));

	if(connectStatus == -1){
		printf("Error conectando con el servidor...\n"); 
		close(sockD);
		exit(EXIT_FAILURE);
	}

	printf("Conectado al servidor en %s:%d\n", IP_SERVIDOR, PORT);
	printf("Si quiere ver la lista de comandos escriba 'comandos'.\n");

	while(1) {
		printf("\nEscriba el comando que le va a mandar al servidor: \n");

		// Usar fgets en lugar de scanf para capturar toda la línea
		char comando[COM_SIZE]; //Array dónde se va a guardar el comando
		if (fgets(comando, sizeof(comando), stdin) == NULL) {
    		printf("Error al leer el comando. Intentelo nuevamente.\\n");
    		continue;
        } // Leer el comando

		comando[strcspn(comando, "\n")] = '\0'; // Eliminar el salto de línea

		// Se crea un proceso hijo
		pid_t pid = fork();
		assert(pid != -1);

		if (pid == 0) {

			// Enviar el comando al servidor
			send(sockD, comando, strlen(comando), 0);

			char strData[BUFFER_SIZE]; //definimos un array para guardar la respuesta del servidor
			ssize_t recvSize = recv(sockD, strData, sizeof(strData), 0); //recibe la respuesta del servidor

			if (recvSize > 0) {
                printf("\nMessage: %s\n", strData); //imprime la respuesta del servidor
            } else {
                printf("Error al recibir datos del servidor.\n");
				exit(EXIT_FAILURE);
            }

			if (strcmp(comando, "salida") == 0) { //si el comando es "salida" se cierra la conexión con el servidor
				// Cerrar conexión y terminar el proceso hijo
				close(sockD);
				exit(EXIT_FAILURE);
			}

			exit(EXIT_SUCCESS);
		}

		else {
			int status;
			wait(&status); // Espera al hijo y captura su estado

			if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_FAILURE) { //exit(1);
				// Si el hijo indica "salida", cerrar el bucle
				printf("cerrando la conexión con el servidor.\n");
				break;
			}
		}
	}

	printf("Conexión con el servidor cerrada\n"); 
	close(sockD); //cerrar el socket
}
