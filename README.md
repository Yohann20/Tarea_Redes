# Tarea 1 Comunicacion de datos y redes

## Integrantes Grupo 3
- **Yohann Matus I.**
- **Andres Torres C.**

## Pasos previos  
* Instalacion de "make".
* Instalacion de compilador C++.
  
## Consideraciones 
* Para inicializacion es necesario compilar y ejectar. 
* Para compilar programas es necesario el comando "make" en terminal.
* Es necesario ejecutar el servidor primero y luego el cliente.
## Para ejecutar Servidor:
```
$ ./server XXXX
```
Donde: XXXX es el puerto donde recibira conexiones. (EJ: ./server 8080)  

## Para ejecutar Cliente:
```
$ ./client 127.0.0.1 XXXX
```
Donde: 127.0.0.1 es la direccion IP del servidor y XXXX es el puerto al cual conectar en el servidor. (EJ: ./client 127.0.0.1 8080)

