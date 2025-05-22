#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //POSIX APIs read(), write(), close()
#include <netinet/in.h> //sockaddr_in
#include <sys/socket.h> //socket(), bind(), listen(), accept()
#include <pthread.h> //for multithreading

#define PORT 8080

//struct to pass multiple args to thread
struct thread_args {
  int socket;
};

void send_response(int socket, const char* body){
  char response[4096];
  sprintf(response,
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Content-Length: %ld\r\n"
    "Connection: close\r\n"
    "\r\n"
    "%s",strlen(body), body);
  write(socket, response, strlen(response));
}

void* handle_client(void* arg){
  struct thread_args* targs = (struct thread_args*) arg;
  int new_socket = targs->socket;
  free(targs); //prevent memory leak
  
  char buffer[3000] = {0};
  read(new_socket, buffer, 3000);
  printf("Request: \n%s\n",buffer);

  char method[8], path[1024];
  sscanf(buffer, "%s %s", method, path);
  printf("Method: %s, Path: %s\n",method,path);

  //respond to path
   if(strcmp(path,"/")==0){
      send_response(new_socket, "<h1>home page</h1>");
    }
   else if(strcmp(path,"/about")==0){
      send_response(new_socket, "<h1>about page</h1>");
    }
    else{
      send_response(new_socket, "<h1>404</h1>");
    }

   //close socket
   close(new_socket);
   return NULL;
}

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

  while(1){
   //accept connections
   new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    if(new_socket<0){
     perror("accept failed");
     continue;
    }

    //allocate thread args
    struct thread_args* targs = malloc(sizeof(struct thread_args));
    targs->socket = new_socket;

    pthread_t thread_id;
    if(pthread_create(&thread_id, NULL, handle_client, targs)!=0){
      perror("pthread_create failed");
      close(new_socket);
      free(targs);
    }
    else {
      pthread_detach(thread_id);
    }
  }
  close(server_fd);
  return 0;
}
