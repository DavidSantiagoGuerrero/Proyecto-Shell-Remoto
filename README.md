# PROYECTO FINAL DE SISTEMAS OPERATIVOS (750001C)

## DOCENTE: John Alexander Sanabria
## UNIVERSIDAD DEL VALLE
## INTEGRANTES:
- David Santiago Guerrero Delgado (2324594-3743)
- Jhonier Mendez Bravo (2372226-3743)
- Cristian Daniel Guaza Mejia (2372225-3743)

En este proyecto nosotros desarrollamos una aplicación cliente/servidor la cual permite a un usuario ejecutar comandos en un computador remoto. Esta aplicación cliente/servidor tiene como cliente al usuario que ejecuta los comandos y el servidor es el computador remoto  donde se ejecutan los comandos.

## CÓMO EJECUTAR EN WINDOWS:

## CÓMO EJECUTAR EN LINUX:
Usando "Virtual Box":

En una terminal ejecutar el comando "ifconfig" y buscar la IP de la interfaz de red activa a la que estan conectados los equipos, luego buscar la interfaz de red principal en nuestro caso es "enp0s3", despues buscar la direccion IP de la VM, deberias ver una linea como esta:

```
inet 192.168.0.125
```

En este caso la direccion IP de la VM es 192.168.0.125 que es la que usaremos para conectarnos desde el cliente al servidor. Tenemos que poner la IP en el codigo del cliente:

// conexion al equipo que funciona como servidor
inet_pton(AF_INET, "192.168.0.125", &servAddr.sin_addr);

Podemos verficar desde el equipo que va a ser usado como cliente la conexion al equipo que sera usado como servidor ejecutando el siguiente comando:

```
ping 192.168.0.125
```
# Si la conexion es satisfactoria:

En una terminal desde el equipo servidor ejecuta el comando:

```
gcc servidor.c -o servidor
```

Luego en la terminal que hiciste gcc al servidor.c para iniciar el programa ejecuta el siguiente comando:
```
./servidor
```

En equipo cliente ejecuta los siguientes comandos:
```
gcc cliente.c -o cliente
./cliente
```

¡Y listo! ya tienes un cliente conectado a un servidor para una shell remota
