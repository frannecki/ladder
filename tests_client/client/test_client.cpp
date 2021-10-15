#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

const int kBufferLen = 1024;

int main(int argc, char** argv) {
  char buffer[kBufferLen];
  int sockfd;
  struct sockaddr_in addr;
  socklen_t addr_len;
  int count = 0;
  int ret;
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    return -1;
  }
  fprintf(stdout, "Created socket: %d\n", sockfd);
  if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) < 0) {
    perror("inet_pton");
    return -1;
  }
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(8070);
  addr_len = sizeof(addr);
  if (connect(sockfd, (struct sockaddr*)&addr, addr_len) < 0) {
    perror("connect");
    return -1;
  }
  fprintf(stdout, "Connected to server\n");

  while (count < 5) {
    ret = read(sockfd, buffer, sizeof(buffer));
    if (ret < 0) {
      perror("read");
      break;
    } else if (ret == 0) {
      break;
    } else {
      buffer[ret] = 0;
      fprintf(stdout, "Received buffer of %d bytes from server: %s\n", ret,
              buffer);
      ret = write(sockfd, buffer, ret);
      if (ret < 0) {
        perror("write");
        break;
      } else {
        buffer[ret] = 0;
        fprintf(stdout, "%d bytes written.\n", ret);
      }
    }
    count += 1;
  }

  if (shutdown(sockfd, SHUT_WR) < 0) {
    perror("shutdown write");
    return -1;
  }

  while ((ret = read(sockfd, buffer, sizeof(buffer))) != 0) {
    if (ret < 0) {
      perror("read");
    } else {
      buffer[ret] = 0;
      fprintf(stdout,
              "Received buffer of %d bytes from server after shutdown: %s\n",
              ret, buffer);
    }
  }

  fprintf(stdout, "connection closed\n");

  if (close(sockfd) < 0) {
    perror("close");
    return -1;
  }

  return 0;
}
