// BEA_ChatServer.cpp: Hauptprojektdatei.

#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <sys/types.h>
//#include <WinSock2.h>
#include <process.h>
//#include <netinet/in.h>

#pragma once
//#include <msclr\marshal.h>
//#include <vcclr.h>

#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)

//using namespace System;
//using namespace msclr::interop;
//using namespace System::Runtime::InteropServices;
//using namespace msclr::interop;


/*Definitionen*/
int sockfd, newsockfd;	//Socket-Filedeskriptoren
struct sockaddr_in server;	//Struktur für den Server
struct sockaddr_in client;	//Struktur für den Client
int sockaddrlen;			//Strukturlänge
char buffer[1024];			//Puffer
char *ptr;					//allgemeiner Pointer
HANDLE childpid;
int n, a;
int flag = 1;

HANDLE fork(TCHAR *cmdline)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// Start the child process. 
	if (!CreateProcess(
		NULL,   // No module name (use command line)
		cmdline,        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi
	)           // Pointer to PROCESS_INFORMATION structure
		)
	{
		printf("CreateProcess failed (%d).\n", GetLastError());
		return(0);
	}
	else
		return (pi.hProcess);
}

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
int main(int argc, TCHAR *argv[])
{
	if( argc < 2 )
	{
		perror("\nAufruf mit Portnummer des Servers");
		exit(1);
	}
	
	//Struktur für den Server initialisiern
	bzero((char *)& server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(_wtoi(argv[1]));
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	/*Erzeugung eines TCP-Sockets*/
	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd < 0)
	{
		perror("Fehler beim Erzeugen des Sockets\n");
		exit(1);
	}

	/*Socket wurde ordnungsgemäß erzeugt*/
	printf("Socket Nummer %d wurde erzeugt.\n", sockfd);

	/*die Werte für den Server an den Socket binden*/
	if (bind(sockfd, (struct sockaddr *) & server, sizeof(server)) == -1)
	{
		perror("Beim Binden ist ein Fehler aufgetreten!\n");
		closesocket(sockfd);
		exit(1);
	}

	/* Es wird eine Queue für maximal 5 Verbindungsanforderungen eingerichtet */
	listen(sockfd, 5);
	
	/* Es werden nun die Werte in die Struktur für den Client gestellt, die vorher
	*  auf null gelöscht wird
	*/

	bzero((char *)&client, sizeof(client));
	client.sin_family = AF_INET;
	client.sin_port = htons(0);
	client.sin_addr.s_addr = htonl(INADDR_ANY);
	sockaddrlen = sizeof(client);

	/* Es werden Verbindungsanforderungen von Clients erwartet */
	printf("\nServer ist bereit");
	while (flag) //Parentprozess kann nur durch Kommando beendet werden
	{
		printf("\nParent-Prozess %d wartet auf Clientanfrage\n", _getpid());
		newsockfd = accept(sockfd, (struct sockaddr *) &client, &sockaddrlen);
		if (newsockfd < 0)
		{
			perror("\nFehler beim Empfangen.");
			closesocket(sockfd);
			closesocket(newsockfd);
			exit(1);
		}

		/* Verbindungsanforderung vom Client fehlerfrei empfangen */
		printf("\nNeuer Socket Nummer %d wurde erzeugt.", newsockfd);
		printf("\nAnforderung kam vom Rechner: %s", inet_ntoa(client.sin_addr));

		/* Es wird ein Child-Prozess erzeugt, der die Anforderung behandelt */
		childpid = fork(argv[0]);
		if (childpid <= 0)
		{
			closesocket(sockfd);
			closesocket(newsockfd);
			exit(1);
		}

		/* Aufteilung in Child und Parent-Prozess */
		closesocket(sockfd);
		printf("\nProzess %d behandelt Anforderung von REchner %s", childpid, inet_ntoa(client.sin_addr));
		do
		{
			ptr = *(&buffer);
			n = sizeof(buffer);
			while (n > 0)
			{
				a = recv(newsockfd, ptr, n, 0);
				if (a < 0)
				{
					perror("\nDaten-Anforderung nicht fehlerfrei.");
					closesocket(newsockfd);
					exit(1);
				}
				else
				{
					if (a == 0)
						break;
					n -= a;
					ptr += a;
				}
			}

			/* Datenanforderung wurde empfangen und steht im Buffer */
			printf("\nEs wurde empfangen: %s", buffer);

			/*erstmal nur echoserver*/
			/*Daten solange zurücksenden, bis "Ende" erkannt wurde*/
			printf("\n Daten-Erwiderung erfolgt an Rechner %s", inet_ntoa(client.sin_addr));
			/*Datenanforderung wird zurückgesandt*/
			ptr = *(&buffer);
			n = sizeof(buffer);
			while (n > 0) //schleife bis alle Daten geschrieben
			{
				a = send(newsockfd, ptr, n, 0);
				if (a < 0)
				{
					perror("\nDaten-Erwiderung nicht fehlerfrei.");
					closesocket(newsockfd);
					exit(1);
				}
				else
				{
					n -= a;
					ptr += a;
				}
			}
			printf("\nDatenerwiderung an Client gesendet.");
		} while (strncmp("Ende", buffer, 4));
		while (strncmp("Ende", buffer, 4) != 0)
		{
			//Ende wurde gesendet, wird nun noch zurückgesandt
			ptr = *(&buffer);
			n = sizeof(buffer);
			while (n > 0)
			{
				send(newsockfd, ptr, n, 0);
				n -= a;
				ptr += a;
			}
			printf("\nDatenaustausch mit Rechner %s wurde beendet.", inet_ntoa(client.sin_addr));
			/*der Childprozess hat seine Arbeit getan*/
			printf("\n Child-Prozess %d beendet sich.", childpid);
			closesocket(newsockfd);
			exit(0);
		}
		/*wir sind wieder im parent process*/
		closesocket(newsockfd);
	}
}
