/************************************************************************
* ipc.h
*
* The only mechanism defined for ipc is through sockets.
************************************************************************/
#ifndef _IPC_H
#define _IPC_H

#ifdef __cplusplus
extern "C" {
#endif


typedef struct sockaddr_in iNetSocketRecType;
typedef struct sockaddr stdSocketRecType;

/************************************************************************
* discardLine...
*
* Read until the next \r\n and throw away what is read.
************************************************************************/
int discardLine(int sockfd);

/************************************************************************
* readResponseCode...
*
* Read one line from a socket and return the number from the first 3 chars.
************************************************************************/
int readResponseCode(int sockfd);

/************************************************************************
* sendEmail...
*
* Connect to a remote email server and send an email.
************************************************************************/
int sendEmail(char *hostname, int port, char *servername, char *sendmailbin, char *fromaddr, 
              char *toaddr, char *subj, char *data);

/************************************************************************
* openPort...
*
* Bind to a port so we can start listening for connections.
************************************************************************/
int openPort(int *sockfd, int portnum);

/************************************************************************
* closePort...
*
* Release the port on a shutdown.
************************************************************************/
int closePort(int sockfd);

/************************************************************************
* acceptConnection...
*
* Does an accept on the socket.
************************************************************************/
int acceptConnection(int sockfd, int *thenewsockfd);

/************************************************************************
* openConnection...
*
* Makes a new connection.
************************************************************************/
int openConnection(int *thesockfd, char *host, int port);

/************************************************************************
* closeConnection...
*
* Closes the connection.
************************************************************************/
int closeConnection(int sockfd);

/************************************************************************
* sendData...
*
* Writes the data down the socket.
************************************************************************/
int sendData(void *data, int datalen, int sockfd);

/************************************************************************
* readData...
*
* Reads data from the socket.
************************************************************************/
int readData(char **thedata, int datalen, int sockfd);

/************************************************************************
* readDataStatic...
*
* Reads data from the socket. (Assuming the memory is already allocated.)
************************************************************************/
int readDataStatic(void *thedata, int datalen, int sockfd);

#ifdef __cplusplus
}
#endif


#endif
