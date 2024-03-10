#include "tcpserver.h"

int TCPServer::StartListen(int port){
	m_listener = socket(AF_INET, SOCK_STREAM, 0);
	if(m_listener<0){
		cout<<"\n\nerror: socket\n\n";
		return -1;
	}
	
	m_addr.sin_family = AF_INET;
    	m_addr.sin_port = htons(port);
    	m_addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(m_listener, (struct sockaddr *)&m_addr, sizeof(m_addr)) < 0){
		cout<<"\n\nerror: bind\n\n";	
		return -2;
	}
	
	listen(m_listener, 1);
	
	return m_listener;
}

bool TCPServer::StopListen(){
	close(m_listener);
}

TCPServer::TCPServer(){
	
}

TCPServer::~TCPServer(){
	close(m_listener);
}

