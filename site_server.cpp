#include "TCPServer/tcpserver.h"
#include "HTTPBuf/httpbuf.h"
#include <arpa/inet.h>
#include <fstream>

using namespace std;
//#include <sys/wait.h>
//#include <signal.h>



/*#include <termios.h>
#include <unistd.h>

int getch( ) {
	struct termios oldt, newt;
	int ch;
	tcgetattr( STDIN_FILENO, &oldt );
	newt = oldt;
	newt.c_lflag &= ~( ICANON | ECHO );
	tcsetattr( STDIN_FILENO, TCSANOW, &newt );
	ch = getchar();
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
	return ch;
}*/





//void child_handler(int i) 
//{
//   int status;
//   wait(&status);
//}

int main(int argc, char *argv[]){
//	struct sigaction sa;
//	sa.sa_handler = child_handler;
//	sigaction(SIGCHLD, &sa, 0);

	TCPServer srv;
	//HTTPBuf hdoc;
	int listener = srv.StartListen(1234);
	int sock;
	int pid;
	int err;
	struct sockaddr_in addr;


	bool is_on = true;
	bool is_connected = true;


	socklen_t* addr_size = new socklen_t;
	*addr_size = sizeof(addr);
	bool is_auth = false;
	bool is_upload = false;
	char buf[1024*1024*8];
	int bytes_read = 0;

	string upload_boundary = "";
	string upload_file = "";
	int page = 1;
	

	while(is_on){
		sock = accept(listener, (sockaddr*)&addr, addr_size);
		if(sock<0){
			cout<<"\nerror: accept\n";
			return 2;
		}

		cout<<"\n\nconnected: "<<inet_ntoa(addr.sin_addr)<<"\n\n";

		/*if(getch()==13){
			cout<<"\n\nStopped!\n\n";
			is_on = false;
		}*/
	
		pid = fork();

		switch(pid){
			case -1:
				cout<<"\nerror: fork\n";
				return 3;
			case 0:
				close(listener);
				while(is_connected){

					memset(buf, 0, 1024*1024*8);
					//size = 0;
					
					bytes_read = recv(sock, buf, 1024*1024*8, 0);

					/*for(int i=0;i<1024*1024*8-1;i++){
						if((char)buf[i]=='\r'&&(char)buf[i+1]=='\n'){
							size = i+2;
						}
					}*/

					if(bytes_read<=0){
						cout<<"\nfailed to recieve\n";
						//is_connected = false;
						//return 4;
						//close(sock);
						//_exit(0);
					}
					else{
						cout<<"\n\n\n\n<<<<<<<<<<<<\n\nrecieved:\n\n"<<buf<<"\n\nsize:"<<bytes_read<<"\n\n<<<<<<<<<<<<\n\n\n\n";
					}


					cout<<"\n\n\n==========================================\n\nupload: "<<is_upload<<"\nboundary: "<<upload_boundary<<"\nfilename: "<<upload_file<<"\n\nis_auth: "<<is_auth<<"\n\npage: "<<page<<"\n\n===========================================\n\n\n";
					
					
					HTTPBuf hdoc;

					hdoc.SetUpload(is_upload);
					
					if(is_upload){
						hdoc.SetBoundary(upload_boundary);
						hdoc.SetFileName(upload_file);
					}

					hdoc.SetPage(page);

					is_auth = hdoc.HandleReqwest((char*)buf, bytes_read, is_auth);

					page = hdoc.GetPage();
					
					is_upload = hdoc.GetUpload();

					if(is_upload){
						upload_boundary = hdoc.GetBoundary();
						upload_file = hdoc.GetFileName();
					}
					
					if(!is_upload){
						err = send(sock, hdoc.GetBuf(), hdoc.GetSize(), 0);
				                if(err>0){
        	                	                cout<<"\n\n\n\n>>>>>>>>>>>>\n\nsent: \n\n"<<hdoc.GetBuf()<<"\n>>>>>>>>>>>>\n\n\n\n";
	                                	}
                	                	else{
                        	                	cout<<"\nerror: failed to send error: "<<err<<"\n";
                                		}
					}
					is_connected = bytes_read>0?hdoc.GetConnection():false;
				}
							
				close(sock);
				_exit(0);
			default:
				cout<<"\n\n\n***************************** "<<pid<<" *****************************\n\n\n";

			close(sock);
		}
	}
	
	close(listener);	
	delete addr_size;
	
	return 0;
}
