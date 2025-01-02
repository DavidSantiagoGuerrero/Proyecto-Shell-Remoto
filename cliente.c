#include <netinet/in.h> //estructura para guardar la información de dirección
#include <stdio.h>
#include <string.h> //para la función strcmp()
#include <stdlib.h>
#include <sys/socket.h> //para la APIs de socket
#include <unistd.h> //para la función close()
#include <assert.h>
#include <sys/wait.h> //para usar wait()

int main(int argc, char const* argv[])
{
	//------Conectarse------
	char comando[255]; //Array dónde se va a guardar el comando

	//crear un socket
	int sockD = socket(AF_INET, SOCK_STREAM, 0);

	//definir la dirección del servidor
	struct sockaddr_in servAddr;

	//llenar la estructura con la información del servidor
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(9001); //usar un puerto que no se esté usando
	servAddr.sin_addr.s_addr = INADDR_ANY;

	// conexion al equipo que funciona como servidor
	inet_pton(AF_INET, "192.168.0.125", &servAddr.sin_addr);

	//------Read / Write------
	
	//si connect devuelve -1 no hubo conexión, si devuelve 0 si se conectó exitosamente
	int connectStatus = connect(sockD, (struct sockaddr*)&servAddr,	sizeof(servAddr));

	if(connectStatus == -1){
		printf("Error conectando con el servidor...\n"); 
		return 0;
	}

	printf("Si quiere ver la lista de comandos escriba 'comandos'.\n");

	while(1) {
		
		printf("\nEscriba el comando que le va a mandar al servidor: \n");

		// Usar fgets en lugar de scanf para capturar toda la línea
        memset(comando, 0, sizeof(comando)); // Limpiar el buffer
        fgets(comando, sizeof(comando), stdin); //coger toda la consulta

		comando[strcspn(comando, "\n")] = '\0'; // Eliminar el salto de línea

		// Se crea un proceso hijo
		pid_t pid = fork();
		assert(pid != -1);

		if (pid == 0) {

			// Enviar el comando al servidor
			send(sockD, comando, strlen(comando), 0);

			char strData[2048]; //definimos un array para guardar la respuesta del servidor
			ssize_t recvSize = recv(sockD, strData, sizeof(strData), 0); //recibe la respuesta del servidor

			if (recvSize > 0) {
                printf("\nMessage: %s\n", strData); //imprime la respuesta del servidor
            } else {
                printf("Error al recibir datos del servidor.\n");
				exit(1);
            }

			if (strcmp(comando, "salida") == 0){ //si el comando es "salida" se cierra la conexión con el servidor
				// Cerrar conexión y terminar el proceso hijo
				exit(1); // Indica al padre que el cliente quiere salir
			}

			exit(0);
		}

		else {
			int status;
			wait(&status); // Espera al hijo y captura su estado

			if (WIFEXITED(status) && WEXITSTATUS(status) == 1) { //exit(1);
				// Si el hijo indica "salida", cerrar el bucle
				printf("cerrando la conexión.\n");
				break;
			}
		}
	}

	printf("Cerrando la conexión con el servidor...\n"); 
	close(sockD); //cerrar el socket
	return 0;
}
