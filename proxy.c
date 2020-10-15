// Reqt. for parsing the packets:
#include "proxy_parse.h"
#include <string.h>

// Reqt. for general purpose functions:
#include <stdio.h>
#include <stdlib.h>

// Common reqt. for both client and server sockets:
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

// Reqt. for client sockets:
#include <netdb.h>

// The limit on how many clients can connect to the proxy:
#define MAX_CLIENTS 100

// // Function that finds the url's ip address:
// int lookup_host (const char *host, char *ip_addr)
// {
//   struct addrinfo hints, *res;
//   int errcode;
//   char addrstr[100];
//   void *ptr;

//   memset (&hints, 0, sizeof (hints));
//   hints.ai_family = PF_UNSPEC;
//   hints.ai_socktype = SOCK_STREAM;
//   hints.ai_flags |= AI_CANONNAME;

//   errcode = getaddrinfo (host, NULL, &hints, &res);
//   if (errcode != 0)
//     {
//       printf("Not working getaddrinfo...\n");
//       return 0;
//     }

//   while (res)
//     {
//       inet_ntop (res->ai_family, res->ai_addr->sa_data, addrstr, 100);

//       switch (res->ai_family)
//         {
//         case AF_INET:
//           ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
//           break;
//         case AF_INET6:
//           ptr = &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr;
//           break;
//         }
//       inet_ntop (res->ai_family, ptr, addrstr, 100);
//       // printf ("IPv%d address: %s (%s)\n", res->ai_family == PF_INET6 ? 6 : 4,
//               // addrstr, res->ai_canonname);
//       if(res->ai_family != PF_INET6){
//         strcpy(ip_addr, addrstr);
//       }
//       res = res->ai_next;
//     }

//   return 0;
// }

int socket_connect(char *host, in_port_t port){
	struct hostent *hp;
	struct sockaddr_in addr;
	int on = 1, sock;     

	if((hp = gethostbyname(host)) == NULL){
		herror("gethostbyname");
		exit(1);
	}
	bcopy(hp->h_addr, &addr.sin_addr, hp->h_length);
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&on, sizeof(int));

	if(sock == -1){
		perror("setsockopt");
		exit(1);
	}
	
	if(connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1){
		perror("connect");
		exit(1);

	}
	return sock;
}

int main(int argc, char * argv[]) {
  // Take command line input of the port number:
  if(argc != 2){
    printf("Provide a single port number to listen on...\n");
    return 0;
  }
  int proxsockfd, proxnewsockfd, proxportno, clilen, n, ret, rlen;
  
  /*
    struct sockaddr_in {
          short   sin_family;
          u_short sin_port;
          struct  in_addr sin_addr;
          char    sin_zero[8];
  };
  */
  struct sockaddr_in prox_addr, cli_addr;

  // Server Behaviour:
  // Set up socket to listen to clients:
  // 1. Creating a socket:
  proxsockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(proxsockfd < 0){
    printf("Error in opening the proxy socket for client...\n");
    return 0;
  }

  // 2. Setting the address of the socket:
  bzero((char *) &prox_addr, sizeof(prox_addr));
  proxportno = atoi(argv[1]);
  prox_addr.sin_family = AF_INET;
  prox_addr.sin_addr.s_addr = INADDR_ANY;
  prox_addr.sin_port = htons(proxportno);

  // 3. Binding the socket:
  if (bind(proxsockfd, (struct sockaddr *) &prox_addr, sizeof(prox_addr)) < 0){
    printf("Error in binding the proxy socket for client to the address...\n");
    return 0;
  }

  // 4. Listening to the clients:
  listen(proxsockfd, MAX_CLIENTS);

  // 5. Accepting a client:
  while(1){
    clilen = sizeof(cli_addr);
    proxnewsockfd = accept(proxsockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (proxnewsockfd < 0){
      printf("Error in accepting the client...\n");
      return 0;
    }
    ret = fork();
    if(ret == 0){
      break;
    }
    close(proxnewsockfd);
  }

  // The newly spawned child that 
  // handles the client that just got accepted:
  close(proxsockfd);

  // 1. Read the request sent by the client:
  // char buf[8192], serv_host[2048];
  // n = read(proxnewsockfd, buf, 8191);
  char buf[8192], serv_host[2048];
  int ttllen = 8191;
  // printf("The cmp array:\n%s\nIt ends here.", cmp);
  while(1){
    n = read(proxnewsockfd, buf+strlen(buf), ttllen);
    // printf("So Far buf:\n%s\n\n", buf);
    char* index = strstr(buf, "\r\n\r\n");
    if (index) {
      break;
    }
  }
  if(n < 0){
    printf("Error in reading request from client...\n");
    return 0;
  }
  // printf("The request is:\n%s", buf);

  // 2. Check if the request formatting is correct:
  rlen = strlen(buf);
  struct ParsedRequest *req = ParsedRequest_create();

  // 2.1 Gives invalid request output due to wrong format:
  if (ParsedRequest_parse(req, buf, rlen) < 0) {
    printf("Bad Request (400)\n");
    return 0;
  }

  // 2.2 Gives invalid request due to non get http call:
  if(strcmp(req->method, "GET")){
    printf("Not Implemented(501)\n");
  }

  // 3. Re-format the request to relative:
  ParsedHeader_set(req, "Host", req->host);
  ParsedHeader_set(req, "Connection", "close");
  strcpy(serv_host, req->host);
  req->host = "";
  req->protocol = "";
  bzero(buf, 8192);
  ParsedRequest_unparse(req, buf, 8192);
  // printf("The buffer is:\n%s", buf);

  // 4. Forward the request to the server:
  // int spsockfd, spportno;
  // struct sockaddr_in sp_addr;
  // char ip_addr[1024];
  
  // // opening the socket to the server:
  // spsockfd = socket(AF_INET, SOCK_STREAM, 0);
  // if(spsockfd < 0){
  //   printf("Error in opening proxy socket to the server...\n");
  //   return 0;
  // }
  // spportno = 80;
  
  // // get the url address using the hostname:
  // lookup_host(serv_host, ip_addr);
  // printf("The host address is:\n%s\n", ip_addr);

  // // initialize the proxy server address:
  // bzero((char *) &sp_addr, sizeof(sp_addr));
  // sp_addr.sin_family = AF_INET;
  // sp_addr.sin_addr.s_addr = inet_addr(ip_addr);
  // sp_addr.sin_port = htons(spportno);


  // printf("The buffer length is: %ld\n", strlen(buf)+1);
  // n = write(spsockfd, buf, strlen(buf)+1);
  int spportno = 80, spfd = socket_connect(serv_host, spportno);


  write(spfd, buf, strlen(buf)); // write(fd, char[]*, len);  
  if (n < 0){
    printf("Error in sending message to url server...\n");
    return 0;
  }
  else{
    // printf("Sent request to the server from proxy...\n");
  }

  // 5. Read the reply from the server:
  bzero(buf, 8192);
  
  while(read(spfd, buf, 8191) != 0){
    // fprintf(stdout, "%s", buf);
    write(proxnewsockfd, buf, strlen(buf));
    bzero(buf, 8192);
  }
  // bzero(buf, 8192);
  // n = read(spsockfd, buf, 8191);
  // if (n < 0){
  //   printf("Error in reading message from url server...\n");
  //   return 0;
  // }

  // printf("The reply from url server is:\n%s\n", buf);

  // // 6. Forward the reply to the client:
  // n = write(proxnewsockfd, buf, strlen(buf));
  // if (n < 0){
  //   printf("Error in writing message to client...\n");
  //   return 0;
  // }

  // printf("Successfully dealt with client!\n");
  close(spfd);

  return 0;
}
