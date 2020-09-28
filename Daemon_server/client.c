/**
* Programa que env�a a trav�s de un socket de Flujos de datos un mensaje a un servidor.
* Redes de Computadoras I
* Universidad Industrial de Santander
* ENTRADA: IP o nombre del servidor
* SALIDA: NA
* PROCESO: Este programa crea un socket y recibe un mensaje de bienvenida del servidor,
* env�a un mensaje de texto a un servidor y finalmente recibe un mensaje de recibido
**/


#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// PUERTO DEL SERVIDOR
#define PORT 2048

// EL TAMA�O M�XIMO DE DATOS
#define MAXDATASIZE 100


int main(int argc, char *argv[]){

	// DESCRIPTOR PARA LA GESTI�N DEL SOCKET
	int fd;
	int numbytes;

	// BUFFER PARA ALMACENAR LOS DATOS
	char buf[MAXDATASIZE];

	// ESTRUCTURA PARA ALMACENAR LA INFORMACI�N
	// DEL SERVIDOR
	struct hostent *he;

	// DECLARACI�N DEL SOCKET DEL SERVIDOR
	struct sockaddr_in server;

	char band;

	// EL MENSAJE QUE SE ENVIAR� AL SERVIDOR
	char message[1024]="Este es el mensaje enviado";

	// VALIDACI�N DE LOS PAR�METROS DE ENTRADA
	if (argc !=2) {
		printf("Uso: %s <Direcci�n IP o Nombre>\n",argv[0]);
		exit(-1);
	}


	// OBTENER EL NOMBRE DEL SERVIDOR O LA IP
	if ((he=gethostbyname(argv[1]))==NULL){
		printf("gethostbyname() error\n");
		exit(-1);
	}

	// CREACI�N DE UN EXTREMO DE LA COMUNICACI�N
	if ((fd=socket(AF_INET, SOCK_STREAM, 0))==-1){
		printf("socket() error\n");
		exit(-1);
	}

// ESPECIFICACI�N DE VALORES AL SOCKET DEL SERVIDOR
//
	// SE ESPECIFICA QUE SE UTILIZAR� IPv4
	server.sin_family = AF_INET;

	// SE ESPECIFICA EL PUERTO DEL SOCKET
	server.sin_port = htons(PORT);

	// SE ESPECIFICA LA IP O NOMBRE DEL SERVIDOR QUE VIENE
	// DE LA FUNCI�N gethostbyname
	server.sin_addr = *((struct in_addr *)he->h_addr);

	// SE LLENA CON CEROS EL RESTO DE LOS CAMPOS DEL SOCKET
	bzero(&(server.sin_zero),8);

	// SE REALIZA LA CONEXI�N AL SERVIDOR
	if(connect(fd, (struct sockaddr *)&server, sizeof(struct sockaddr))==-1){
		printf("connect() error\n");
		exit(-1);
	}

	// SE HACE LA RECEPCI�N DEL MENSAJE DE BIENVENIDA DEL SERVIDOR
	if ((numbytes=recv(fd,buf,MAXDATASIZE,0)) == -1){
		printf("Error en recv() \n");
		exit(-1);
	}

	// SE AGREGA UN FIN DE CADENA AL BUFFER DE RECEPCI�N
	buf[numbytes]='\0';

	// SE MUESTRA EL MENSAJE DE BIENVENIDA
	printf("Mensaje del Servidor: %s\n",buf);

	// SE ENV�A EL MENSAJE
	send(fd,message,strlen(message)+1,0);

	// SE CIERRA LA CONEXI�N CON EL SERVIDOR
	close(fd);

}
