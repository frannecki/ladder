#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <thread>
#include <mutex>
#include <vector>

const int kBufferLen = 1024;
// const int kClientNumber = 120;
const int kClientNumber = 20;
static std::mutex mutex;
static int cnt = 0;

class EchoClient {
public:
  EchoClient() : 
    running_(true)
  {
    thread_ = std::thread(&EchoClient::ThreadFunc, this);
  }

  ~EchoClient() {
    Stop();
  }

private:
  
  void Stop() {
    running_ = false;
    thread_.join();
  }
  
  int ThreadFunc() {
    char buffer[kBufferLen] = "areyouok";
    int sockfd;
    struct sockaddr_in addr;
    socklen_t addr_len;
    int count = 0;
    int ret;
    {
      std::lock_guard<std::mutex> lock(mutex);
      sockfd = socket(AF_INET, SOCK_STREAM, 0);
      fprintf(stdout, "Socket created. fd = %d\n", sockfd);
    }
    if(sockfd < 0) {
      perror("socket");
      return -1;
    }
    
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
    else {
      fprintf(stdout, "Connected to server. rank: %d. fd = %d\n", ++cnt, sockfd);
    }

    while(count < 5) {
      ret = read(sockfd, buffer, sizeof(buffer));
      if(ret < 0) {
        if(errno != EWOULDBLOCK && errno != EAGAIN)
        perror("read");
        break;
      }
      else if(ret == 0) {
        break;
      }
      else {
        // fprintf(stdout, "Received buffer of %d bytes from server: %s\n",
        //         ret, buffer);
        ret = write(sockfd, buffer, ret);
        if(ret < 0) {
          perror("write");
          break;
        }
        else {
          // fprintf(stdout, "%d bytes written.\n", ret);
        }
      }

      count += 1;
    }
    
    while(running_);

    fprintf(stdout, "Stopping.\n");

    if(close(sockfd) < 0) {
      perror("close");
      return -1;
    }
    else {
      fprintf(stdout, "Closed socket. fd = %d\n", sockfd);
    }

  }

private:
  std::thread thread_;
  bool running_;
};

int main(int argc, char** argv) {
  std::vector<EchoClient*> clients(kClientNumber, nullptr);
  for(int i = 0; i < kClientNumber; ++i) {
    clients[i] = new EchoClient;
  }
  for(int i = 0; i < kClientNumber; ++i) {
    delete clients[i];
    clients[i] = nullptr;
  }
  return 0;
}
