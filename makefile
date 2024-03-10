site_server: site_server.cpp TCPServer/tcpserver.h TCPServer/tcpserver.cpp HTTPBuf/httpbuf.h HTTPBuf/httpbuf.cpp
	g++ -g -o site_server TCPServer/tcpserver.h TCPServer/tcpserver.cpp HTTPBuf/httpbuf.h HTTPBuf/httpbuf.cpp site_server.cpp
