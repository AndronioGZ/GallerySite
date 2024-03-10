#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>

using namespace std;

class TCPServer{
        private:
                int m_listener;
		struct sockaddr_in m_addr;
        public:	
                int StartListen(int port);
		bool StopListen();
                TCPServer();
                ~TCPServer();
};

