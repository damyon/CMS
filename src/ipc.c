#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#ifdef WIN32
#include <winsock2.h>
#include <io.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#include <string.h>
#include <sys/types.h>



#include <errno.h>

#include "strings.h"
#include "logging.h"
#include "ipc.h"
#include "errors.h"
#include "malloc.h"

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR   -1
#endif
#define QUEUE_SIZE     1
#define MAILRESPONSEREADY 220
#define MAILRESPONSESUCCESS 250
#define MAILRESPONSESENDDATA 354
#ifndef SOCKET_TIMEOUT
#define SOCKET_TIMEOUT 60*12
#endif
/************************************************************************
* discardLine...
*
* Read until the next \r\n and throw away what is read.
************************************************************************/
int discardLine(int sockfd) {
  char mailresp[256], *ptr = NULL;
  int err = E_OK;  

  ptr = mailresp;
  do {
    if ((err = readDataStatic(ptr, sizeof(char), sockfd)) != E_OK) {
      return err;
    }
  } while (*ptr != '\r' && *ptr != '\n');
  
  if (*ptr == '\r') {
    if ((err = readDataStatic(ptr, sizeof(char), sockfd)) != E_OK) {
      return err;
    }
  } 
  return E_OK;
}

/************************************************************************
* readResponseCode...
*
* Read one line from a socket and return the number from the first 3 chars.
************************************************************************/
int readResponseCode(int sockfd) {
  char mailresp[256], *ptr = NULL;
  int err = E_OK;
  
  ptr = mailresp;
  if ((err = readDataStatic(ptr, 3*sizeof(char), sockfd)) != E_OK) {
    return err;
  }
  mailresp[3] = '\0';


  // READ REST OF LINE (UNTIL \r\n)
  if ((err = discardLine(sockfd)) != E_OK) {
    return err;
  }
  
  return strtol(mailresp, NULL, 10);
}



#ifdef WIN32
/************************************************************************
* sendEmail...
*
* Connect to a remote email server and send an email.
************************************************************************/
int sendEmail(char *hostname, int port, char *servername, char *sendmailbin, char *fromaddr, char *toaddr, char *subj, char *data) {
  int sockfd = 0;
  int err = 0;
  char mailreq[256];
  char *command = NULL;

  if (hostname == NULL || servername == NULL || port < 0 || 
      fromaddr == NULL || toaddr == NULL || subj == NULL || 
      data == NULL || strlen(hostname) > 128 || strlen(servername) > 128 || 
      strlen(fromaddr) > 128 || strlen(toaddr) > 128 || strlen(subj) > 128) {
    return INVALIDARGUMENTS;
  }
      
  // This is the windows method
  if ((err = openConnection(&sockfd, hostname, port)) != E_OK) {
    return err;
  }  


  // READ FIRST 3 CHARS AND CHECK FOR 220.
  if ((err = readResponseCode(sockfd)) != MAILRESPONSEREADY) {
    // MAIL SERVER NOT E_OK
    closeConnection(sockfd);
    return err;
  }

  // SEND HELO
  sprintf(mailreq, "HELO %s\r\n", servername);
  if ((err = sendData(mailreq, strlen(mailreq) * sizeof(char), sockfd)) != E_OK) {
    closeConnection(sockfd);
    return err;
  }
  
  // READ FIRST 3 CHARS AND CHECK FOR 250.
  if ((err = readResponseCode(sockfd)) != MAILRESPONSESUCCESS) {
    closeConnection(sockfd);
    return err;
  }
   
  // SEND MAIL FROM
  sprintf(mailreq, "MAIL FROM:%s\r\n", fromaddr);
  if ((err = sendData(mailreq, strlen(mailreq) * sizeof(char), sockfd)) != E_OK) {
    closeConnection(sockfd);
    return err;
  }
  
  // READ FIRST 3 CHARS AND CHECK FOR 250.
  if ((err = readResponseCode(sockfd)) != MAILRESPONSESUCCESS) {
    closeConnection(sockfd);
    return err;
  }
   
  // SEND RCPT TO
  sprintf(mailreq, "RCPT TO:%s\r\n", toaddr);
  if ((err = sendData(mailreq, strlen(mailreq) * sizeof(char), sockfd)) != E_OK) {
    closeConnection(sockfd);
    return err;
  }
  
  // READ FIRST 3 CHARS AND CHECK FOR 250.
  if ((err = readResponseCode(sockfd)) != MAILRESPONSESUCCESS) {
    closeConnection(sockfd);
    return err;
  }
   
  // SEND DATA
  sprintf(mailreq, "DATA\r\n");
  if ((err = sendData(mailreq, strlen(mailreq) * sizeof(char), sockfd)) != E_OK) {
    closeConnection(sockfd);
    return err;
  }
  
  // READ FIRST 3 CHARS AND CHECK FOR 354.
  if ((err = readResponseCode(sockfd)) != MAILRESPONSESENDDATA) {
    closeConnection(sockfd);
    return err;
  }

  // SEND THE FROM
  sprintf(mailreq, "From:%s\r\n", fromaddr);
  if ((err = sendData(mailreq, strlen(mailreq) * sizeof(char), sockfd)) != E_OK) {
    closeConnection(sockfd);
    return err;
  }
  
  // SEND THE TO
  sprintf(mailreq, "To:%s\r\n", toaddr);
  if ((err = sendData(mailreq, strlen(mailreq) * sizeof(char), sockfd)) != E_OK) {
    closeConnection(sockfd);
    return err;
  }
  
  // SEND THE SUBJECT
  sprintf(mailreq, "Subject:%s\r\n", subj);
  if ((err = sendData(mailreq, strlen(mailreq) * sizeof(char), sockfd)) != E_OK) {
    closeConnection(sockfd);
    return err;
  }
  
  // SEND THE TEXT
  if ((err = sendData(data, strlen(data) * sizeof(char), sockfd)) != E_OK) {
    closeConnection(sockfd);
    return err;
  }
   
  // END THE EMAIL
  sprintf(mailreq, "\r\n.\r\n");
  if ((err = sendData(mailreq, strlen(mailreq) * sizeof(char), sockfd)) != E_OK) {
    closeConnection(sockfd);
    return err;
  }
  
  // READ FIRST 3 CHARS AND CHECK FOR 250.
  if ((err = readResponseCode(sockfd)) != MAILRESPONSESUCCESS) {
    closeConnection(sockfd);
    return err;
  }

  // QUIT
  sprintf(mailreq, "QUIT");
  if ((err = sendData(mailreq, strlen(mailreq) * sizeof(char), sockfd)) != E_OK) {
    closeConnection(sockfd);
    return err;
  }
  
  closeConnection(sockfd);
  return E_OK;
}
#else
/************************************************************************
* sendEmail...
*
* Connect to a remote email server and send an email.
************************************************************************/
int sendEmail(char *hostname, int port, char *servername, char *sendmailbin, char *fromaddr, char *toaddr, char *subj, char *data) {
  FILE *sockfd = NULL;
  char *command = NULL;

  if (sendmailbin == NULL ||
      fromaddr == NULL || toaddr == NULL || subj == NULL || 
      data == NULL || strlen(sendmailbin) > 128 ||
      strlen(fromaddr) > 128 || strlen(toaddr) > 128 || strlen(subj) > 128) {
    return INVALIDARGUMENTS;
  }
      
  vstrdupcat(&command, sendmailbin, " ", toaddr, NULL);
  sockfd = popen(command, "w");
  if (sockfd == NULL) {
    return SOCKET_ERROR;
  }
  dhufree(command);

  // SEND THE FROM
  fprintf(sockfd, "From:%s\r\n", fromaddr);
  
  // SEND THE TO
  fprintf(sockfd, "To:%s\r\n", toaddr);
  
  // SEND THE SUBJECT
  fprintf(sockfd, "Subject:%s\r\n\r\n", subj);
  
  // SEND THE TEXT
  fprintf(sockfd, "%s", data);
   
  // END THE EMAIL
  fprintf(sockfd, "\r\n.\r\n");
  
  pclose(sockfd);
  return E_OK;
}
#endif

/************************************************************************
* openPort...
*
* Bind to a port so we can start listening for connections.
************************************************************************/
int openPort(int *thesockfd, int portnum) {
  int sockfd = -1, on = 1;
  iNetSocketRecType serv_addr;

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
    return INVALID_SOCKET;
  }
  
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
 
  memset((void *) &serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons((unsigned short) portnum);
  

  if (bind(sockfd, (struct sockaddr *) &serv_addr, (int) sizeof(serv_addr)) == SOCKET_ERROR) {
    return SOCKET_ERROR;
  }

  if (listen(sockfd, QUEUE_SIZE) < 0) {
    return SOCKET_ERROR;
  }

  *thesockfd = sockfd;
  return E_OK;
}

/************************************************************************
* closePort...
*
* Release the port on a shutdown.
************************************************************************/
int closePort(int sockfd) {
#ifdef WIN32
  closesocket(sockfd);
#else
  close(sockfd);
#endif
  return E_OK;
}

/************************************************************************
* closeConnection...
*
* Closes the socket
************************************************************************/
int closeConnection(int sockfd) {
#ifdef WIN32
  int code = SOCKET_ERROR;
  char tmp[2];

  shutdown(sockfd, SD_SEND );
  
  while (code != SOCKET_ERROR && code != 0) {
    code = recv(sockfd, tmp, 1, 0);
  }
  
  closesocket(sockfd);
#else
  close(sockfd);
#endif
  return E_OK;
}

/************************************************************************
* acceptConnection...
*
* Does an accept on the socket.
************************************************************************/
int acceptConnection(int sockfd, int *thenewsockfd) {
  int newsockfd = -1;
  socklen_t c_len = 0;
  iNetSocketRecType cli_addr;

  errno = 0;

  c_len = sizeof(cli_addr);

  newsockfd = accept(sockfd, (stdSocketRecType *) &cli_addr, &c_len);

  while ((newsockfd < 0) && (errno == EINTR)) {
    newsockfd = accept(sockfd, (stdSocketRecType *) & cli_addr, &c_len);
  }

  if (newsockfd < 0) {
    return errno;
  }
  *thenewsockfd = newsockfd;
  return E_OK;
}

/************************************************************************
* socketWritable...
*
* is there data to write on the socket?...
************************************************************************/
int socketWritable(int sockfd) {
  fd_set set;
  struct timeval timeout;
  int retval = 0;

  /* Initialise the file descriptor */
  FD_ZERO(&set);
  FD_SET(sockfd, &set);

  timeout.tv_sec = SOCKET_TIMEOUT;
  timeout.tv_usec = 0;

  errno = 0;

  do {
    /* select returns 0 if timeout, 1 if input available and -1 on error */
    if ((retval = select(FD_SETSIZE, NULL, &set, NULL, &timeout)) > 0) {
      return 1;
    } else if (retval == 0) {
      // socket timeout
      closeConnection(sockfd);
    }
  } while (errno == EINTR);

  return E_OK;
}

/************************************************************************
* openConnection...
*
* Makes a new connection.
************************************************************************/
int openConnection(int *thesockfd, char *host, int port) {
  iNetSocketRecType serv_addr;
  struct hostent    *host_name;
  int               sockfd  = 0;
  //char option = 1;

  if (!(host_name = gethostbyname(host))) {
    return 1;
  }

  /* flush the structure */
  memset((char *) &serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family      = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *) host_name->h_addr)));
  serv_addr.sin_port        = htons((unsigned short)port);


  /* set up socket and make a connection to host */
  

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    return 1;

  //option = 1;
  //setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &option, sizeof(option));

  if (connect(sockfd, (stdSocketRecType *) &serv_addr, sizeof(serv_addr)) != 0) {
#ifdef WIN32
    closesocket(sockfd);
#else
    close(sockfd);
#endif
    return 1;
  }

  if (!socketWritable(sockfd)) {
#ifdef WIN32
    closesocket(sockfd);
#else
    close(sockfd);
#endif
    return 1;
  }

  *thesockfd = sockfd;
  return E_OK;
}

/************************************************************************
* sendData...
*
* Writes the data down the socket.
************************************************************************/
int sendData(void *data, int datalen, int sockfd) {
  int len = 0, total = 0;
  errno = 0;
  
  while (total < datalen) {
    len = send(sockfd, (char *) data + total, datalen - total, 0);
    if (len > 0) {
      total += len;
    } else {
      if (errno != EINTR) {
        return errno;
      }
    }
  }
  return E_OK;
}


/************************************************************************
* socketOK...
*
* is there data to read from the socket?...
************************************************************************/
int socketOK(int sockfd) {
  fd_set set;
  struct timeval timeout;
  int retval = 0;

  /* Initialise the file descriptor */
  FD_ZERO(&set);
  FD_SET(sockfd, &set);

  timeout.tv_sec = SOCKET_TIMEOUT;
  timeout.tv_usec = 0;

  errno = 0;

  do {
    /* select returns 0 if timeout, 1 if input available and -1 on error */
    if ((retval = select(FD_SETSIZE, &set, NULL, NULL, &timeout)) > 0) {
      return 1;
    } else if (retval == 0) {
      // socket timeout
      closeConnection(sockfd);
    }
  } while (errno == EINTR);

  return E_OK;
}



#ifdef WIN32
/************************************************************************
* readDataStatic...
*
* Reads data from the socket.
************************************************************************/
int readDataStatic(void *thedata, int datalen, int sockfd) {
  int bytesRecv = SOCKET_ERROR, total = 0;
  char *data = thedata;
  
  while( total < datalen) {
    bytesRecv = recv( sockfd, data + total, datalen - total, 0 );
    
    if ( bytesRecv <= 0) {
      // Connection Closed
      break;
    }
   
	total += bytesRecv;
  }

  if (total == datalen)
    return E_OK;
  return -1;
}


#else
/************************************************************************
* readDataStatic...
*
* Reads data from the socket.
************************************************************************/
int readDataStatic(void *thedata, int datalen, int sockfd) {
  int len = 0, total = 0;
  char *data = (char *) thedata;

  while (total < datalen) {
    errno = 0;
    if (socketOK(sockfd)) {
      len = recv(sockfd, data + total, datalen - total, 0);
      if (len > 0) {
        total += len;
      } else {
		if (errno != EINTR) {
          return -1;
        }
	  }
    }
  }
  return E_OK;
} 
#endif

/************************************************************************
* readData...
*
* Reads data from the socket.
************************************************************************/
int readData(char **thedata, int datalen, int sockfd) {
  char *data = NULL;

  data = (char *) dhumalloc(sizeof(char) * (datalen + 1));
  memset(data, 0, datalen + 1);
  *thedata = data;
  return readDataStatic(data, datalen, sockfd);
} 
