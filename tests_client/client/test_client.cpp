#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const int kBufferLen = 1024;

int main(int argc, char** argv) {
  for(int idx = 0; idx < 100; ++idx) {
  char buffer[kBufferLen];
  int sockfd;
  struct sockaddr_in addr;
  socklen_t addr_len;
  int count = 0;
  int ret;
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    return -1;
  }
  fprintf(stdout, "Created socket: %d\n", sockfd);
  if(inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) < 0) {
    perror("inet_pton");
    return -1;
  }
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(8070);
  addr_len = sizeof(addr);
  if(connect(sockfd, (struct sockaddr*)&addr, addr_len) < 0) {
    perror("connect");
    return -1;
  }
  fprintf(stdout, "Connected to server\n");

  while(count < 5) {
    ret = read(sockfd, buffer, sizeof(buffer));
    if(ret < 0) {
      perror("read");
      break;
    }
    else if(ret == 0) {
      break;
    }
    else {
      fprintf(stdout, "Received buffer of %d bytes from server: %s\n",
              ret, buffer);
      ret = write(sockfd, buffer, ret);
      if(ret < 0) {
        perror("write");
        break;
      }
      else {
        fprintf(stdout, "%d bytes written.\n", ret);
      }
    }
    count += 1;
  }

  if(close(sockfd) < 0) {
    perror("close");
    return -1;
  }
  }

  return 0;
}