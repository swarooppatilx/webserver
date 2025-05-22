#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //POSIX APIs read(), write(), close()
#include <netinet/in.h> //sockaddr_in
#include <sys/socket.h> //socket(), bind(), listen(),
#include <pthread.h> //for multithreading
#include <sys/stat.h> //for file size 
#include <fcntl.h> //for open()

#define PORT 8080

struct thread_args {
  int socket;
};

const char* get_mime_type(const char* path) {
  if (strstr(path, ".html")) return "text/html";
  if (strstr(path, ".css")) return "text/css";
  if (strstr(path, ".js")) return "application/javascript";
  if (strstr(path, ".png")) return "image/png";
  if (strstr(path, ".jpg") || strstr(path, ".jpeg")) return "image/jpeg";
  if (strstr(path, ".gif")) return "image/gif";
  return "text/plain";
}

void send_response(int socket, const char* body, const char* status, const char* content_type) {
  char response[4096];
  snprintf(response, sizeof(response),
    "HTTP/1.1 %s\r\n"
    "Content-Type: %s\r\n"
    "Content-Length: %ld\r\n"
    "Connection: close\r\n"
    "\r\n"
    "%s", status, content_type, strlen(body), body);
  write(socket, response, strlen(response));
}

void send_file_response(int socket, const char* filepath) {
  printf("Trying to open: %s\n", filepath);
  int fd = open(filepath, O_RDONLY);
  if (fd < 0) {
    send_response(socket, "<h1>404 Not Found</h1>", "404 Not Found", "text/html");
    return;
  }

  struct stat st;
    if (fstat(fd, &st) < 0) {
    perror("fstat");
    close(fd);
    send_response(socket, "<h1>500 Internal Server Error</h1>", "500 Internal Server Error", "text/html");
    return;
  }
  off_t filesize = st.st_size;
  printf("File size: %ld\n",filesize);

  const char* content_type = get_mime_type(filepath);
  char header[1024];
  snprintf(header, sizeof(header),
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: %s\r\n"
    "Content-Length: %ld\r\n"
    "Connection: close\r\n"
    "\r\n", content_type, filesize);

  write(socket, header, strlen(header));

  char buffer[1024];
  ssize_t bytes;
  while ((bytes = read(fd, buffer, sizeof(buffer))) > 0) {
    write(socket, buffer, bytes);
  }
  close(fd);
}

void* handle_client(void* arg) {
  struct thread_args* targs = (struct thread_args*) arg;
  int new_socket = targs->socket;
  free(targs);

  char buffer[3000] = {0};
  ssize_t bytes_read = read(new_socket, buffer, sizeof(buffer) - 1);
  if (bytes_read <= 0) {
    close(new_socket);
    return NULL;
  }

  printf("Request:\n%s\n", buffer);

  char method[8], path[1024];
  sscanf(buffer, "%7s %1023s", method, path);  // size limits added for safety
  printf("Method: %s, Path: %s\n", method, path);

  if (strlen(path) > 1015) {
    send_response(new_socket, "<h1>414 URI Too Long</h1>", "414 URI Too Long", "text/html");
    close(new_socket);
    return NULL;
  }

  if (strstr(path, "..")) {
    send_response(new_socket, "<h1>403 Forbidden</h1>", "403 Forbidden", "text/html");
    close(new_socket);
    return NULL;
  }

  if (strcmp(path, "/") == 0) {
    send_file_response(new_socket, "./static/index.html");
  } else {
    char safe_path[1016];
    strncpy(safe_path, path, sizeof(safe_path) - 1);
    safe_path[sizeof(safe_path) - 1] = '\0';

    char file_path[1024];
    snprintf(file_path, sizeof(file_path), "./static%s", safe_path);
    send_file_response(new_socket, file_path);
  }

  close(new_socket);
  return NULL;
}

int main() {
  int server_fd;
  struct sockaddr_in address;
  int addrlen = sizeof(address);

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, 10) < 0) {
    perror("listen failed");
    exit(EXIT_FAILURE);
  }

  printf("Server Listening on port %d...\n", PORT);

  while (1) {
    int new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    if (new_socket < 0) {
      perror("accept failed");
      continue;
    }

    struct thread_args* targs = malloc(sizeof(struct thread_args));
    targs->socket = new_socket;

    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, handle_client, targs) != 0) {
      perror("pthread_create failed");
      close(new_socket);
      free(targs);
    } else {
      pthread_detach(thread_id);
    }
  }

  close(server_fd);
  return 0;
}
