// BEA_ChatServer.cpp: Hauptprojektdatei.

#include "stdafx.h"
#include <stdio.h>
#include <sys/types.h>
#include <WinSock2.h>

#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)

using namespace System;

/*Definitionen*/
int sockfd, newsockfd;	//Socket-Filedeskriptoren
struct sockaddr_in server;	//Struktur für den Server
struct sockaddr_in client;	//Struktur für den Client
int sockaddrlen;			//Strukturlänge
char buffer[1024];			//Puffer
char *ptr;					//allgemeiner Pointer
int childpid, n, a;
int flag = 1;

/* Main Funktion */
//int main(array<System::String ^> ^args)
//{
//	Console::WriteLine(L"Hello World");
//	for (int i = 0; i < args->Length; i++)
//	{
//		Console::WriteLine(args[i]);
//	}
//	Console::ReadKey();
//	return 0;
//}
int main(array<System::String ^> ^args)
{
	if( args->Length < 2 )
	{
		perror("\nAufruf mit Portnummer des Servers");
		exit(1);
	}
	
	//Struktur für den Server initialisiern
	bzero((char *)& server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(int::Parse(args[1]));
	server.sin_addr.s_addr = htonl(INADDR_ANY);
}
