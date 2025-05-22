#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //POSIX APIs read(), write(), close()
#include <netinet/in.h> //sockaddr_in
#include <sys/socket.h> //socket(), bind(), listen(), accept()


#define PORT 8080

int main(){
  int server_fd;
  int new_socket;
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  
  //create socket
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(server_fd==0){
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  //bind address
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY; //0.0.0.0
  address.sin_port = htons(PORT); //host to network

  if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0){
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  //listen
  if(listen(server_fd, 3)<0){
    perror("listen failed");
    exit(EXIT_FAILURE);
  }
  
  printf("Server Listining on port %d....\n",PORT);

  //accept connections
  new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
  if(new_socket<0){
    perror("accept failed");
    exit(EXIT_FAILURE);
  }

  //read client request
  char buffer[3000] = {0};
  read(new_socket,buffer,3000);
  printf("Request: \n%s\n", buffer);

  char* http_response = "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/html\r\n"
                        "Connection: close\r\n"
                        "\r\n"
                        "<!DOCTYPE html><html><body><h1>Hello, World</h1></body></html>";
  write(new_socket, http_response, strlen(http_response));
  printf("Response Sent!\n");

  //close socket
  close(new_socket);
  close(server_fd);
  return 0;
}
