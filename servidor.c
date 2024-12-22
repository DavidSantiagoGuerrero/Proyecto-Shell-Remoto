#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

int main(int argc, char const* argv[]) {
    int servSockD = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(9001);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    bind(servSockD, (struct sockaddr*)&servAddr, sizeof(servAddr));
    listen(servSockD, 1);

    int clientSocket = accept(servSockD, NULL, NULL);
    char strData[255];

    char currentDir[1024];
    getcwd(currentDir, sizeof(currentDir)); // Inicializar el directorio actual

    while (1) {
        recv(clientSocket, strData, sizeof(strData), 0);

        if (strcmp(strData, "salida") == 0) {
            char salir[255] = "Gracias por usar nuestra shell remota";
            send(clientSocket, salir, sizeof(salir), 0);
            close(clientSocket);
            close(servSockD);
            break;
        } else if (strcmp(strData, "comandos") == 0) {
            char comandos[1024] = "Comandos disponibles:\n"
                                  "1. salida: cerrar la conexi\xF3n\n"
                                  "2. ls: listar archivos\n"
                                  "3. pwd: mostrar el directorio actual\n"
                                  "4. cat <archivo>: mostrar el contenido de un archivo\n"
                                  "5. cd <directorio>: cambiar el directorio actual\n"
                                  "6. tree: mostrar estructura de directorios";
            send(clientSocket, comandos, sizeof(comandos), 0);
        } else if (strcmp(strData, "ls") == 0) {
            char ls[1024] = {0};
            DIR* d = opendir(currentDir);
            struct dirent* de;

            if (d) {
                while ((de = readdir(d))) {
                    strcat(ls, de->d_name);
                    strcat(ls, "\n");
                }
                closedir(d);
            } else {
                snprintf(ls, sizeof(ls), "Error abriendo directorio: %s", strerror(errno));
            }

            send(clientSocket, ls, sizeof(ls), 0);
        } else if (strcmp(strData, "pwd") == 0) {
            send(clientSocket, currentDir, sizeof(currentDir), 0);
        } else if (strncmp(strData, "cat ", 4) == 0) {
            char filename[255];
            sscanf(strData + 4, "%s", filename);

            FILE* file = fopen(filename, "r");
            char content[1024] = {0};

            if (file) {
                fread(content, 1, sizeof(content) - 1, file);
                fclose(file);
            } else {
                snprintf(content, sizeof(content), "Error abriendo archivo: %s", strerror(errno));
            }

            send(clientSocket, content, sizeof(content), 0);
        } else if (strncmp(strData, "cd ", 3) == 0) {
            char newDir[255];
            sscanf(strData + 3, "%s", newDir);

            if (chdir(newDir) == 0) {
                getcwd(currentDir, sizeof(currentDir));
                char success[255];
                snprintf(success, sizeof(success), "Directorio cambiado a: %s", currentDir);
                send(clientSocket, success, sizeof(success), 0);
            } else {
                char error[255];
                snprintf(error, sizeof(error), "Error cambiando directorio: %s", strerror(errno));
                send(clientSocket, error, sizeof(error), 0);
            }
        } else if (strcmp(strData, "tree") == 0) {
            // Implementar funcionalidad de tree
            char treeOutput[1024] = "Funcionalidad de 'tree' no implementada completamente.";
            send(clientSocket, treeOutput, sizeof(treeOutput), 0);
        } else {
            char response[255] = "Comando no reconocido";
            send(clientSocket, response, sizeof(response), 0);
        }
    }
    return 0;
}
