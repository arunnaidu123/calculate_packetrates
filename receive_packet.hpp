#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>



struct network_stats
{
  public:
  std::condition_variable cv;
  std::mutex m;
  long double gbps = 0;
  bool readFlag= false;
  bool writeFlag = true;
  
  int compute_stats(long int total_events,int packet_size, long int nsec) 
  {
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk, [this]{return readFlag;});  
    
    gbps = ((long double)(total_events*packet_size*8)/((long double)nsec));

    writeFlag = true;
    readFlag = false;
    cv.notify_one();
    return 0;
  }
  
  long double read_stats()
  {
    long double temp_gbps;

    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk, [this]{return writeFlag;});
    
    temp_gbps = gbps*1;
    
    writeFlag = false;
    readFlag = true;
    cv.notify_one();   
    return temp_gbps; 
  }  
};

//extern class network_stats link0;
//extern class network_stats link1;
//extern class network_stats link2;
//extern class network_stats link3;

