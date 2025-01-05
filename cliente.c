#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>
#include <arpa/inet.h>

int main(int argc, char const* argv[]) {
    char comando[255];  // Array para guardar el comando
    int sockD;

    // Crear un socket
    sockD = socket(AF_INET, SOCK_STREAM, 0);
    if (sockD < 0) {
        perror("Error al crear el socket");
        return 1;
    }

    // Definir la dirección del servidor
    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(9001);
    servAddr.sin_addr.s_addr = INADDR_ANY;  // O usa la IP específica

    // Conectar al servidor
    if (inet_pton(AF_INET, "172.20.111.218", &servAddr.sin_addr) <= 0) {
    	perror("Dirección IP inválida o no soportada");
    	close(sockD);
    	return EXIT_FAILURE;
    }


    int connectStatus = connect(sockD, (struct sockaddr*)&servAddr, sizeof(servAddr));
    if (connectStatus == -1) {
        perror("Error conectando con el servidor...");
        return 1;
    }

    printf("Conectado al servidor en %s:%d\n", "192.168.0.125", 9001);
    printf("Si quiere ver la lista de comandos escriba 'comandos'.\n");

    while (1) {
        // Leer el comando del usuario
        printf("\nEscriba el comando que desea enviar al servidor: \n");
        memset(comando, 0, sizeof(comando)); // Limpiar el buffer
        if (fgets(comando, sizeof(comando), stdin) == NULL) {
    		printf("Error al leer el comando. Intentelo nuevamente.\\n");
    		continue;
        }// Leer el comando

        // Eliminar el salto de línea de la entrada
        comando[strcspn(comando, "\n")] = '\0';

        // Enviar el comando al servidor
        send(sockD, comando, strlen(comando), 0);

        // Si el comando es "salida", cerrar la conexión
        if (strcmp(comando, "salida") == 0) {
            printf("Cerrando la conexión.\n");
            break;
        }

        // Recibir la respuesta del servidor
        char strData[2048];
        ssize_t recvSize = recv(sockD, strData, sizeof(strData), 0);
        if (recvSize > 0) {
            strData[recvSize] = '\0';  // Asegurar que la cadena está terminada
            printf("Respuesta del servidor: %s\n", strData);
        } else {
            printf("Error al recibir datos del servidor.\n");
            break;
        }
    }

    close(sockD);  // Cerrar el socket
    return 0;
}
