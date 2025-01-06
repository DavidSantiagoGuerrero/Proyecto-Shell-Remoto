#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h> //para el comando ls
#include <errno.h>
#include <sys/stat.h>
#include <time.h>

#define SALIDA "Gracias por usar nuestra shell remota"

#define COMANDOS \
    "Comandos disponibles:\n" \
	"1. salida: cerrar la conexión\n" \
	"2. ls: listar archivos\n" \
	"3. pwd: mostrar el directorio actual\n" \
	"4. cat <archivo>: mostrar el contenido de un archivo\n" \
	"5. cd <directorio>: cambiar el directorio actual\n" \
	"6. tree: mostrar estructura de directorios\n" \
	"7. head <archivo>: mostrar las primeras lineas de un archivo\n" \
	"8. uptime : muestra el tiempo que lleva el servidor funcionando\n" \
	"9. date : muestra la fecha del sistema\n" \
	"10. calc : realiza una operacion sencilla"

#define NOT_FOUND "Comando no reconocido"
#define BUFFER_SIZE 1024

// Variable global para guardar el tiempo de inicio del servidor
time_t tiempo_inicio;

// muestra el contenido de un directorio
void ejecutar_ls(char *response) {
    DIR *d = opendir(".");
    if (d) {
        struct dirent *de;
        while ((de = readdir(d)) != NULL) {
            strcat(response, de->d_name);
            strcat(response, "\n");
        }
        closedir(d);
    } else {
        strcpy(response, "Error: No se pudo abrir el directorio actual\n");
    }
}

// permite desplazarse entre archivos
void ejecutar_cd(const char *ruta, char *response) {
    while (*ruta == ' ') ruta++;
        if (strlen(ruta) == 0) {
            strcpy(response, "Error: No se especificó una ruta válida\n");
        } else if (chdir(ruta) == 0) {
            snprintf(response, BUFFER_SIZE, "Directorio cambiado a: %s\n", ruta); //cambie sizeof(response) por el tamaño del array por un warning
        } else {
            snprintf(response, BUFFER_SIZE, "Error: No se pudo cambiar al directorio: %s\n", ruta);
        }
}

// permite visualizar el contenido de un archivo
void ejecutar_cat(const char *filename, char *response) {
    FILE* file = fopen(filename, "r");

    if (file) {
        fread(response, 1, BUFFER_SIZE - 1, file);
        fclose(file);
    } else {
        snprintf(response, BUFFER_SIZE, "Error abriendo archivo: %s", strerror(errno));
    }
}

// retorna la direccion de un directorio
void ejecutar_pwd(char *response) {
    getcwd(response, BUFFER_SIZE);
}

// muestra la estrucutura de un directorio
void ejecutar_tree(const char *path, char *response, int level) {
    DIR *dir = opendir(path);
    if (dir == NULL) {
        snprintf(response, BUFFER_SIZE, "Error: No se pudo abrir el directorio %s\n", path);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Formatear espacios según el nivel de profundidad
        for (int i = 0; i < level; i++) {
            strcat(response, "  ");
        }
        strcat(response, "|- ");
        strcat(response, entry->d_name);
        strcat(response, "\n");

        // Construir la ruta completa
        char subPath[BUFFER_SIZE];
        snprintf(subPath, sizeof(subPath), "%s/%s", path, entry->d_name);

        // Verificar si es un directorio
        struct stat st;
        if (stat(subPath, &st) == 0 && S_ISDIR(st.st_mode)) {
            ejecutar_tree(subPath, response, level + 1);
        }
    }
    closedir(dir);
}

//muestra las primeras lineas de un archivo
void ejecutar_head(const char *filename, char *response, int num_lines) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        snprintf(response, BUFFER_SIZE, "Error abriendo archivo: %s\n", strerror(errno));
        return;
    }

    char line[BUFFER_SIZE];
    int count = 0;
    response[0] = '\0'; // limpiar la respuesta

    while (fgets(line, sizeof(line), file) != NULL) {
        strcat(response, line);
        count++;
        if (count >= num_lines) break; // Mostrar solo las primeras `num_lines` líneas
    }

    fclose(file);
}

// Inicializa el tiempo de inicio del servidor
void inicializar_tiempo_servidor() {
    tiempo_inicio = time(NULL);
}

// tiempo que lleva el servidor en funcionamiento
void ejecutar_uptime(char *response) {
    time_t tiempo_actual = time(NULL);
    double segundos_transcurridos = difftime(tiempo_actual, tiempo_inicio);

    int horas = segundos_transcurridos / 3600;
    int minutos = (segundos_transcurridos - (horas * 3600)) / 60;
    int segundos = (int)segundos_transcurridos % 60;

    snprintf(response, BUFFER_SIZE, "El servidor ha estado en funcionamiento por: %d horas, %d minutos y %d segundos.\n",
             horas, minutos, segundos);
}

// fecha y hora
void ejecutar_date(char *response) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(response, BUFFER_SIZE, "Fecha y hora actual: %Y-%m-%d %H:%M:%S\n", t);
}

// hace un calculo sencillo
void ejecutar_calc(const char *input, char *response) {
    char operator;
    double num1, num2, result;

    if (sscanf(input, "%lf %c %lf", &num1, &operator, &num2) != 3) {
        strcpy(response, "Error: Formato incorrecto. Usa: calc <num1> <operador> <num2>\n");
        return;
    }

    switch (operator) {
        case '+':
            result = num1 + num2;
            break;
        case '-':
            result = num1 - num2;
            break;
        case '*':
            result = num1 * num2;
            break;
        case '/':
            if (num2 == 0) {
                strcpy(response, "Error: División por cero.\n");
                return;
            }
            result = num1 / num2;
            break;
        default:
            strcpy(response, "Error: Operador no válido. Usa +, -, *, /.\n");
            return;
    }

    snprintf(response, BUFFER_SIZE, "Resultado: %.2lf\n", result);
}
