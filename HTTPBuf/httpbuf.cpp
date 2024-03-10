#include "httpbuf.h"

void HTTPBuf::AddHeader(string header){
	m_sbuf+=header;
	m_sbuf+="\r\n";
}

void HTTPBuf::AddHeader(char* header_type, int value){

	m_sbuf.append(header_type);
	
	int value_temp = value;
	
	int n = 1;
	char *nch;
	while(value_temp>=10){
		value_temp/=10;
		n++;
	}
	
	nch = new char[n];
	
	sprintf(nch, "%d", value);

	m_sbuf.append(nch);
	m_sbuf+="\r\n";
	
	delete[] nch;
}

void HTTPBuf::AddHeader(char * header_type, string value){
	m_sbuf.append(header_type);
	m_sbuf+=value;
	m_sbuf+="\r\n";
}

char * HTTPBuf::GetBuf(){
	if(!m_binary){
		m_buf = new char[m_sbuf.size()+1];
		sprintf(m_buf, "%s", m_sbuf.c_str());
	}
	return m_buf;	
}

void HTTPBuf::AddContent(char* file){
	fstream fs;
	char *fbuf;
	int flen;

	if(std::ifstream(file)==NULL){
		SetError(404);
		return;
	}

	fs.open(file, ios::in);
	fs.seekg(0, ios::end);
        flen = fs.tellg();
        fs.seekg(0, ios::beg);

	fbuf = new char[flen];
	fs.read(fbuf, flen);
	fs.close();

	AddHeader((char*)HTTP_CONTENT_LENGTH, flen);

	m_sbuf+="\r\n";
	m_sbuf.append(fbuf);
	m_sbuf+="\r\n";
	m_size = m_sbuf.size();

	delete[] fbuf;
	//fbuf = NULL;

	m_binary = false;
}
	
void HTTPBuf::AddContentBinary(char* file){
	fstream fs;
        char *fbuf;
        int flen;
	
	if(std::ifstream(file)==NULL){
                SetError(404);
		m_binary = false;
                return;
        }

        fs.open(file, ios::in | ios::binary);
        fs.seekg(0, ios::end);
        flen = fs.tellg();
        fs.seekg(0, ios::beg);

        fbuf = new char[flen];
        fs.read(fbuf, flen);
        fs.close();
	
	AddHeader((char*)HTTP_CONTENT_LENGTH, flen);

        m_sbuf+="\r\n";
	
	m_buf = new char[m_sbuf.size() + flen + 2];

	sprintf(m_buf, "%s", m_sbuf.c_str());
	
	/*for(int i=m_sbuf.size(); i<m_sbuf.size()+flen; i++){
		m_buf[i] = fbuf[i-m_sbuf.size()];
	}*/

	memcpy(m_buf+m_sbuf.size(), fbuf, flen);
	
	m_buf[m_sbuf.size()+ flen] = '\r';
	m_buf[m_sbuf.size()+ flen + 1] = '\n';
	m_size = m_sbuf.size()+ flen + 2;

	delete[] fbuf;
	//fbuf = NULL;

	m_binary = true;
}

void HTTPBuf::AddContentStr(string content){
	AddHeader((char*)HTTP_CONTENT_LENGTH, content.size());
	m_sbuf+="\r\n";
	m_sbuf+=content;
	m_sbuf+="\r\n";
	m_size = m_sbuf.size();
	
	m_binary = false;
}

HTTPBuf::HTTPBuf(){
	m_boundary = "";
	m_filename = "";
	m_upload = false;
	m_connection = true;
	m_page = 1;
	//m_buf = NULL;
	
	m_sbuf+="HTTP/1.1 200 OK\r\n";
}

HTTPBuf::HTTPBuf(int error){
	int error_tmp;
	
        m_sbuf+="HTTP/1.1 ";
	
	int n = 1;
        char *nch;
        while(error_tmp>=10){
                error_tmp/=10;
                n++;
        }

        nch = new char[n];

        sprintf(nch, "%d", error);
	
	m_sbuf.append(nch);
	m_sbuf+="\r\n";

	delete[] nch;
}

HTTPBuf::HTTPBuf(char reqstm[1024*1024*8], int size, bool is_auth){
	m_is_auth = is_auth;
	m_connection = true;
	m_sbuf+="HTTP/1.1 200 OK\r\n";

	is_auth = HandleReqwest(reqstm, size, is_auth);
}

HTTPBuf::HTTPBuf(bool is_upload, string boundary){
        m_connection = true;
        m_sbuf+="HTTP/1.1 200 OK\r\n";

	m_upload = is_upload;
	m_boundary = boundary;
}

void HTTPBuf::SetError(int error){
        int error_tmp = error;

        m_sbuf+="HTTP/1.1 ";

        int n = 1;
        char *nch;
        while(error_tmp>=10){
                error_tmp/=10;
                n++;
        }

        nch = new char[n];

        sprintf(nch, "%d", error);

        m_sbuf.append(nch);
        m_sbuf+="\r\n";

	delete[] nch;
}

HTTPBuf::~HTTPBuf(){
	if(m_binary) delete[] m_buf;
	m_buf = NULL;
}

int HTTPBuf::GetSize(){
	return m_size;
}

void HTTPBuf::ClearBuf(){
	delete[] m_buf;
	m_buf = NULL;
	m_sbuf = "";
	m_connection = true;
}

bool HTTPBuf::GetConnection(){
	return m_connection;
}

bool HTTPBuf::GetUpload(){
	return m_upload;
}

void HTTPBuf::SetUpload(bool upload){
	m_upload = upload;
}

string HTTPBuf::GetBoundary(){
	return m_boundary;
}

void HTTPBuf::SetBoundary(string boundary){
	m_boundary = boundary;
}

string HTTPBuf::GetFileName(){
	return m_filename;
}

void HTTPBuf::SetFileName(string filename){
	m_filename = filename;
}

int HTTPBuf::GetPage(){
	return m_page;
}

void HTTPBuf::SetPage(int page){
	m_page = page;
}

int HTTPBuf::FindStrIn(char* buf, string str, int pos1, int pos2){
	char* tempbuf;
	tempbuf = new char[str.size()];

	for(int i=pos1;i<pos2;i++){
		memcpy((char*)tempbuf, (char*)(buf+i), str.size());
		if(memcmp((char*)tempbuf, (char*)str.c_str(), str.size())==0){
			delete[] tempbuf;
			return i;
		}
	}
	delete[] tempbuf;
	return -1;
}

#define PICS_IN_PAGE 24
	
bool HTTPBuf::HandleReqwest(char reqstm[1024*1024*8], int size, bool is_auth){

//////////////////////////////////////////////////////////////Here you can handle reqwests from browser/////////////////////////////////////
	string reqst = "";
	bool reqst_mid = false;
	bool reqst_end = false;
	bool reqst_beg = false;
	string file;
        string content_type;
        int strn, strn2, strn3, len;
	
	if(!m_upload){
		if(size>1000){
			reqst.append((char*)reqstm);
		}
		else{
			reqst.append((char*)reqstm, size);
		}
	}
	else{
		string boundary_temp;
                boundary_temp = m_boundary + "--";

		char temp_str[4];
		memcpy((char*)temp_str, reqstm, 4);

		char* temp_str2;
		temp_str2 = new char[boundary_temp.size()];
		memcpy(temp_str2, (char*)reqstm + (size-boundary_temp.size()) - 2, boundary_temp.size());

		char* temp_str3;
                temp_str3 = new char[m_boundary.size()];
                memcpy(temp_str3, (char*)reqstm, m_boundary.size());

		if(memcmp(temp_str, (char*)"GET ", 4)!=0&&memcmp(temp_str, (char*)"POST", 4)!=0&&memcmp(temp_str2, boundary_temp.c_str(), boundary_temp.size())!=0&&memcmp(temp_str3, m_boundary.c_str(), m_boundary.size())!=0){
			reqst_mid = true;
		}
		if(memcmp(temp_str, (char*)"GET ", 4)!=0&&memcmp(temp_str, (char*)"POST", 4)!=0&&memcmp(temp_str2, boundary_temp.c_str(), boundary_temp.size())==0){
                       	reqst_end = true;
                }
		if(memcmp(temp_str3, m_boundary.c_str(), m_boundary.size())==0){
			reqst_beg = true;
		}


		cout<<"\n\n\n--------------------\n\n";
		cout<<"GET: "<<memcmp(temp_str, (char*)"GET ", 4)<<"\n\n";
		cout<<"POST: "<<memcmp(temp_str, (char*)"POST", 4)<<"\n\n";
		cout<<"boundary_beg: "<<memcmp(temp_str3, m_boundary.c_str(), m_boundary.size())<<"\n\n";
		cout<<"boundary_end: "<<memcmp(temp_str2, boundary_temp.c_str(), boundary_temp.size())<<"\n\n";
		cout<<"\n\n--------------------\n\n\n";
		
		delete[] temp_str2;
		delete[] temp_str3;
		
	}

	if(!m_upload&&reqst.substr(0, 3)=="GET"){
        	strn = reqst.find("HTTP");
        	file = reqst.substr(5, strn - 6);
                if(file=="") file = "index.html";

              	strn = file.find(".");

        	if(strn<0){
			AddHeader((char*)HTTP_CONNECTION, "keep-alive");
			
		      	if(file=="main"){
				AddHeader((char*)HTTP_CONTENT_TYPE, "text/html; charset=UTF-8");
				AddContent((char*)"main.html");
				return false;
			}
                	if(file=="portfolio"){
				AddHeader((char*)HTTP_CONTENT_TYPE, "text/html; charset=UTF-8");
				AddContent((char*)"index.html");
				return false;
			}
			if(file=="contacts"){
				AddHeader((char*)HTTP_CONTENT_TYPE, "text/html; charset=UTF-8");
				AddContent((char*)"contacts.html");
				return false;
			}
			if(file=="nextpage"){
				int n=0;
				DIR *dir = opendir("./img");
    				if(dir){
        				struct dirent *ent;
        				while((ent = readdir(dir)) != NULL){
            					n++;
        				}
					n-=2;
					if((m_page + 1)*PICS_IN_PAGE<=n){
						m_page++;
					}
    				}
    				else{
					fprintf(stderr, "Error opening directory\n");
				}
				file = "scrollpreview";

				//return is_auth;			
			}
			if(file=="prevpage"){
				m_page-=m_page==1?0:1;

				file = "scrollpreview";
				
				//return is_auth;			
			}
			if(file=="scrollpreview"){
				cout<<"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^SCROLLPREVIEW^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^";
				string frame;
								
				int i=1;
				int n=1;

				DIR *dir = opendir("./img");
                                if(dir){
					frame = "<html><body>";

                                        struct dirent *ent;

                                        while((ent = readdir(dir)) != NULL){
                                                if(i>(m_page*PICS_IN_PAGE-PICS_IN_PAGE)&&i<=m_page*PICS_IN_PAGE&&strcmp(ent->d_name, (char*)".")!=0&&strcmp(ent->d_name, (char*)"..")!=0){
							frame+="<a href=\"img";


							int number = n;

        						int nn = 1;
							char *nch;
        						while(number>=10){
                						number/=10;
                						nn++;
        						}

        						nch = new char[nn];

        						sprintf(nch, "%d", n);

							frame.append(nch);

							//char nch[3];
							//sprintf(nch, "%d", n);
							//frame.append(nch);
							frame+="\" onClick=\"onload:ChangeLnk(this);return false;\"> <img src=\"img/";
							frame.append(ent->d_name);
							frame+="\" width=\"128\" height=\"96\" hspace=\"10\" vspace=\"15\"> </a>";
							n++;
							delete[] nch;
						}
						i++;							
                                        }
					frame+="</body>";
					frame+="<script language=\"javascript\">function ChangeLnk(a){window.top.frames[1].location=a.href;}</script>";
					frame+="</html>";
					AddHeader((char*)HTTP_CONTENT_TYPE, "text/html; charset=UTF-8");
					AddContentStr(frame);
				}
                                else{
                                        fprintf(stderr, "Error opening directory\n");
					SetError(404);
					return is_auth;
                                }
				
				return is_auth;
			}
			if(file.substr(0, 3)=="img"||file=="body"){
				fstream fs;
				char *fbuf;
				string body;
        			int flen;
				int number;

        			if(std::ifstream("body.html")==NULL){
                			SetError(404);
                			return false;
        			}

        			fs.open("body.html", ios::in);
        			fs.seekg(0, ios::end);
        			flen = fs.tellg();
        			fs.seekg(0, ios::beg);

       				fbuf = new char[flen];
       				fs.read(fbuf, flen);
       				fs.close();

       				body.append(fbuf);

				strn2 = body.rfind("img");
				body.erase(strn2, 7);

				number = (file=="body")?1:atoi((file.substr(3, file.size()-3)).c_str());

				cout<<"\n\n################################### number:"<<number<<"########################################\n\n";

				int i=0;

				DIR *dir = opendir("./img");
                                if(dir){
                                        struct dirent *ent;

                                        while(i!=number){
						if((ent = readdir(dir)) == NULL){
							SetError(404);
							return is_auth;							
						}
						cout<<ent->d_name<<"\n";
                                                if(strcmp(ent->d_name, (char*)".")!=0&&strcmp(ent->d_name, (char*)"..")!=0){
							i++;
                                               	}							
                                        }
                                        AddHeader((char*)HTTP_CONTENT_TYPE, "text/html; charset=UTF-8");
					body.insert(strn2, ent->d_name);
                                }
                                else{
                                        fprintf(stderr, "Error opening directory\n");
                                        SetError(404);
                                        return is_auth;
                                }

                                //len = file.size()-3;
                                //body.insert(strn2 + 3, file.substr(3, len));

       				delete[] fbuf;

				AddContentStr(body);
				
				return false;
			}
			if(file=="admin"){
				if(!is_auth){
					AddHeader((char*)HTTP_CONTENT_TYPE, "text/html; charset=UTF-8");
					AddContent((char*)"admin.html");
					return false;
				}
				else{
					AddHeader((char*)HTTP_CONTENT_TYPE, "text/html; charset=UTF-8");
					AddContent((char*)"upload.html");
					return true;
				}
			}
		}			
		else{
			strn++;

        		if(file.substr(strn, file.size()-strn)=="html"||file.substr(strn, file.size()-strn)=="txt"){
				AddHeader((char*)HTTP_CONTENT_TYPE, "text/html; charset=UTF-8");
				AddHeader((char*)HTTP_CONNECTION, "keep-alive");
				AddContent((char*)file.c_str());
              		}
                	else{
				AddHeader((char*)HTTP_CONNECTION, "keep-alive");
				if(file.substr(strn, file.size()-strn)=="ico"||file.substr(strn, file.size()-strn)=="jpg"){
                        		content_type = "image/";
                                	content_type+=file.substr(strn, file.size()-strn)=="ico"?"x-icon":file.substr(strn, file.size()-strn);
					AddHeader((char*)HTTP_CACHE_CONTROL, "public");
                                	AddHeader((char*)HTTP_CONTENT_TYPE, content_type);
                                	AddContentBinary((char*)file.c_str());
                		}
                	}
			return is_auth;
		}
	}
	if(!m_upload&&reqst.substr(0, 4)=="POST"){
		string log, pass, cont;
		strn = reqst.find("HTTP");
                file = reqst.substr(6, strn - 7);

		if(file=="admin"&&!is_auth){
			strn2 = reqst.find("login=") + 6;
			strn3 = reqst.find("&")-strn2;

			log = reqst.substr(strn2, strn3);
			
			strn2 = reqst.find("passwd=") + 7;
			strn3 = reqst.rfind("\r\n");
			pass = reqst.substr(strn2, strn3);
	
			if(log=="1"&&pass=="2"){
				AddHeader((char*)HTTP_CONTENT_TYPE, "text/html; charset=UTF-8");
				AddHeader((char*)HTTP_CONNECTION, "keep-alive");
				AddContent((char*)"upload.html");
				return true;
			}
			else{
				AddHeader((char*)HTTP_CONTENT_TYPE, "text/html; charset=UTF-8");
                                AddHeader((char*)HTTP_CONNECTION, "keep-alive");
                                AddContentStr("<html><body><h1>Access denied! Invalid login or password.<h1></body><html>");
                                return false;
			}
		}
		if(file=="upload"&&is_auth){
			/*strn2 = reqst.find("boundary=");
			strn3 = reqst.find("\r\n", strn2)-strn2-9;
			m_boundary = "--" + reqst.substr(strn2 + 9, strn3);*/

			strn2 = FindStrIn((char*)reqstm, "boundary=", 0, size) + 9;
			strn3 = FindStrIn((char*)reqstm, "\r\n", strn2, size);

			m_boundary.append((char*)(reqstm + strn2), strn3 - strn2);
			m_boundary = "--" + m_boundary;

			cout<<"\n\n\nboundary: "<<m_boundary<<"\n\n\n";

			m_upload = true;

			//strn2 = reqst.find(m_boundary);
			strn2 = FindStrIn((char*)reqstm, m_boundary, 0, size);
			if(strn2>=0){
				strn2 = FindStrIn((char*)reqstm, "filename=", 0, size-1) + 10;
                		strn3 = FindStrIn((char*)reqstm, "\"", strn2 + 1, strn2 + 32);

                		m_filename.append((char*)(reqstm+strn2), strn3-strn2);

				m_filename = "img/" + m_filename;

				strn3 = FindStrIn((char*)reqstm, "Content-Type", strn3, strn3 + 64);
				strn2 = FindStrIn((char*)reqstm, "\r\n", strn3, strn3 + 32) + 4;
				
				ofstream of((char*)m_filename.c_str(), ofstream::binary);
                		char* fbuf;
	        	        fbuf = new char[size - strn2];

				memcpy(fbuf, reqstm+strn2, size-strn2);

	                	of.write(fbuf, size - strn2);
	                	of.close();

        	        	delete[] fbuf;
			}

			//m_sbuf = "HTTP/1.1 303 See Other\r\nLocation: /admin\r\n";
			

			/*AddHeader((char*)HTTP_CONTENT_TYPE, "text/html; charset=UTF-8");
			AddHeader((char*)HTTP_CONNECTION, "keep-alive");
			AddContent((char*)"upload.html");*/


			return true;						
		}
	}
	if(m_upload&&reqst_mid){
		cout<<"\n\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^REQSTMID^^^^^^^^^^^^^^^^^^^^^^^^^^^\n\n";
		ofstream of((char*)m_filename.c_str(), ofstream::binary | ofstream::app);

		of.write(reqstm, size);
		of.close();
		m_upload = true;
		return true;
	}
	if(m_upload&&reqst_beg){
		cout<<"\n\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^REQSTBEG^^^^^^^^^^^^^^^^^^^^^^^^^^^\n\n";
		strn2 = FindStrIn((char*)reqstm, "filename=", 0, size-1) + 10;
		strn3 = FindStrIn((char*)reqstm, "\"", strn2 + 1, strn2 + 32);

		m_filename.append((char*)(reqstm+strn2), strn3-strn2);

		m_filename = "img/" + m_filename;

		cout<<"\n\n\nfilename: "<<m_filename<<"\n\n\n";

		strn3 = FindStrIn((char*)reqstm, "Content-Type", strn3, strn3 + 64);
                strn2 = FindStrIn((char*)reqstm, "\r\n", strn3, strn3 + 32) + 4;
		
		
		ofstream of((char*)m_filename.c_str(), ofstream::binary);
		char* fbuf;
                fbuf = new char[size - strn2];

		memcpy(fbuf, reqstm+strn2, size-strn2);

		
		of.write(fbuf, size - strn2);
                of.close();
		
		delete[] fbuf;
		
		m_upload = true;
		return true;
	}
	if(m_upload&&reqst_end){
		cout<<"\n\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^REQSTEND^^^^^^^^^^^^^^^^^^^^^^^^^^^\n\n";
		bool found = false;
		char* boundary_find;
		boundary_find = new char[m_boundary.size()];
		int i = size-m_boundary.size()*2 - 3;

		while(!found){
			i--;
			memcpy(boundary_find, reqstm + i, m_boundary.size());
			if(memcmp((char*)boundary_find, (char*)m_boundary.c_str(), m_boundary.size())==0){
				found = true;
			}
		}
		
		delete[] boundary_find;
			
		strn2 = i;

		cout<<"\n\n\nstrn2: "<<strn2<<"\n\n\n";
		               
                ofstream of((char*)m_filename.c_str(), ofstream::binary | ofstream::app);
                char* fbuf;
                fbuf = new char[strn2];

		memcpy(fbuf, reqstm, strn2);

                of.write(fbuf, strn2);
                of.close();

                delete[] fbuf;
		m_upload = false;
		
		AddHeader((char*)HTTP_CONTENT_TYPE, "text/html; charset=UTF-8");
                AddHeader((char*)HTTP_CONNECTION, "keep-alive");
                AddContent((char*)"upload.html");
                
                //m_sbuf = "HTTP/1.1 301 Moved Permanently\r\nLocation: http://andre.sknt.ru/admin\r\n\r\n";

		m_boundary = "";
		m_filename = "";
                return true;
        }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}



