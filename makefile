receive_packet: receive_packet.cpp receive_packet.hpp
	g++ -g -Wall -o receive_packet receive_packet.cpp -std=c++11 -lpthread

clean:
	rm -rf *.o
