#include <stdio.h>
#include <string.h>
#ifdef _MSC_VER
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#elif defined(__unix__)
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

const int kBufferLen = 1024;

int main(int argc, char** argv) {
#ifdef _MSC_VER
  WSADATA wsa_data;
  if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != NO_ERROR) {
    return -1;
  }
#endif
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
    ret = recv(sockfd, buffer, sizeof(buffer), 0);
    if (ret < 0) {
      perror("read");
      break;
    } else if (ret == 0) {
      break;
    } else {
      buffer[ret] = 0;
      fprintf(stdout, "Received buffer of %d bytes from server: %s\n", ret,
              buffer);
      ret = send(sockfd, buffer, ret, 0);
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
#ifdef _MSC_VER
  if (shutdown(sockfd, SD_SEND) < 0) {
#elif defined(__unix__)
  if (shutdown(sockfd, SHUT_WR) < 0) {
#endif
    perror("shutdown write");
    return -1;
  }

  while ((ret = recv(sockfd, buffer, sizeof(buffer), 0)) != 0) {
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

#ifdef _MSC_VER
  if (closesocket(sockfd) < 0) {
#else
  if (close(sockfd) < 0) {
#endif
    perror("close");
    return -1;
  }

  return 0;
}
