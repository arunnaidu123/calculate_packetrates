#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdexcept>
#include <netinet/in.h>
#include <unistd.h>
#include <chrono>
#include <memory>
#include <fstream>
#include <vector>
#include <sched.h>
#include <pthread.h>
#include <time.h>
#include <thread>
#include "receive_packet.hpp"
struct network_stats ip_link[4];
//struct network_stats link1;
//struct network_stats link2;
//struct network_stats link3;


int receive_packets(char *ip_address, int thread_id)
{
  
  static const int udp_port_number = 1313;
  static const int packet_size = 4264;
  static const int max_events = 100;
  
  int sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);      
  if (sock_fd < 0)
  {
    std::cout << "network thread: socket() failed: " << strerror(errno) << std::endl;
    //throw runtime_error(strerror(errno));
    exit(0);
  }
  
  struct sockaddr_in server_address;
  memset(&server_address, 0, sizeof(server_address));
  
  server_address.sin_family = AF_INET;
  inet_pton(AF_INET, ip_address, &server_address.sin_addr);
  server_address.sin_port = htons(udp_port_number);
  
  if (::bind(sock_fd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
  {
    std::cout << "network thread: bind() failed: " << strerror(errno) << std::endl;
    //throw runtime_error(strerror(errno));
    exit(0);
  }
  
  int n = 16*1024*1024;
  if (setsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, (void *) &n, sizeof(n)) < 0)
  {
    std::cout << "network thread: setsockopt() failed: " << strerror(errno) << std::endl;
    //throw runtime_error(strerror(errno));
    exit(0);
  }
  
  struct epoll_event events[max_events];
  struct epoll_event ev;
  int epoll_fd = epoll_create(10);
  
  if (epoll_fd < 0)
  {
    std::cout << "network thread: epoll_create() failed: " << strerror(errno) << std::endl;
    //throw runtime_error(strerror(errno));
    exit(0);
  }
  
  ev.events = EPOLLIN;
  ev.data.fd = sock_fd;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &ev) < 0)
  {
    std::cout << "network thread: epoll_ctl() failed: " << strerror(errno) << std::endl;
    //throw runtime_error(strerror(errno));
    exit(0);
  }
  //struct timeval tv0 = get_time();
    
  
  uint64_t total_events=0;  
  uint16_t *packet = new uint16_t[packet_size/2];
  
  std::chrono::system_clock::time_point start = std::chrono::high_resolution_clock::now();

  while (1)
  {
    uint32_t num_events = epoll_wait(epoll_fd, events, max_events, -1);

    if (num_events < 0)
    {
      std::cout << "network thread: epoll_wait() failed: " << strerror(errno) << std::endl;
      //throw runtime_error(strerror(errno));
      exit(0);
    }

      
    
    //std::cout<<"total_events: "<<total_events<< "  \n";   

    for (uint32_t i = 0; i < num_events; i++)
    {
      if (events[i].data.fd != sock_fd)
        continue;

      ssize_t bytes_read = read(sock_fd, packet, packet_size);

      if (bytes_read != packet_size)
      {
        std::cout<<"Something is wrong. Please check \n";
        std::cout.flush();
      }
      
    }
    long nsec = std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::high_resolution_clock::now() - start).count();
    total_events += num_events;
    for(int i=0;i<4;i++) ip_link[i].compute_stats(total_events,packet_size,nsec);
    std::cout.flush();
  }

  return 0;
}
        
      
int main(int argc, char *argv[])
{
  std::thread t[4];
  long double *gbps = new long double[4];
  for(int i=0;i<4;i++) t[i] = std::thread(receive_packets,argv[i+1],i);
  
  while(1)
  {
    for(int i=0;i<4;i++) gbps[i] = ip_link[i].read_stats();

    for(int i=0;i<4;i++) std::cout<<"link"<<i<<": "<<gbps[i];
    std::cout<<"\r";
  }
  
  for(int i=0;i<4;i++) t[i].join();
}  
 



