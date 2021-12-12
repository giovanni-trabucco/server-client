
#include <unistd.h>
#include <arpa/inet.h>
#include<errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>
char * method, *path, *ver;
char command[1000];
struct sockaddr_in local,remote;
struct headers{
char * n;
char * v;
}h[100];
char request[2001],response[2000];
int c_l = 0;
// ip(7) socket(2) connect(2) errno(3) perror(3) 
int main()
{
	FILE *fin;
	int size,s,s2,t,k,j,w,i,lunghezza;
	int car;
	int yes=1;
	s= socket(AF_INET, SOCK_STREAM ,0);
	if ( s == -1 ) { printf("Errno=%d ",errno); perror("Socket Fallita"); return 1; }
	local.sin_family=AF_INET;
	local.sin_port = htons(8086);
	local.sin_addr.s_addr=0;
	t= setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)); 
	if ( t == -1) {perror("Setsockopt fallita"); return 1;}
	t = bind(s,(struct sockaddr *) &local, sizeof(struct sockaddr_in));
	if ( t == -1) {perror("Bind fallita"); return 1;}
	t = listen(s,5);
	if ( t == -1) {perror("Listen fallita");return 1;}
	lunghezza = sizeof(remote);
	remote.sin_family=AF_INET;
while(1){
		s2=accept(s,(struct sockaddr *) &remote, &lunghezza);
		if ( s2 == -1) {perror("Accept fallita");return 1;}
		t=read(s2, request, 2000);
		request[t]=0;
		
		printf("%s",request);
		method = request;
		for(i=0;request[i]!=' ';i++); request[i]=0;
		path = request +i+1; 
		for(;request[i]!=' ';i++); request[i]=0;
		ver = request +i+1; 
		for(;request[i]!='\r';i++); request[i]=0;
		printf("method = %s, path = %s, ver = %s\n", method, path, ver);		
		fin=NULL;
		char input[50];
		
		if (!strcmp(method,"GET")){
			if (!strncmp(path,"/cgi-bin/",9)){ 
					int path_index = 9;
					int input_index = 0;
					while(path[path_index]!=0){
						if ((path[path_index]=='%') && (path[path_index+1]=='2') && (path[path_index+2]=='0')) {
							input[input_index] = ' ';
							path_index += 3;
							input_index++;
						}
						else {
							input[input_index] = path[path_index];
							input_index++;
							path_index++;
						}			
	
					}
				input[input_index] = 0;
				sprintf(command,"%s > tmpfile.txt", input); // e qui finisce 
				t = system(command);
				if (( t != -1) && (t != 127))
					strcpy(path+1,"tmpfile.txt");
				}	
			if ((fin=fopen(path+1,"r"))==NULL){
				sprintf(response,"HTTP/1.1 404 Not Found\r\nConnection:close\r\n\r\n");
			}else {
				sprintf(response,"HTTP/1.1 200 OK\r\nConnection:close\r\n\r\n");
	
			}
				
		}else if (!strcmp(method,"POST") || !strcmp(method,"HEAD")){
		sprintf(response,"HTTP/1.1 501 Not Implemented\r\nConnection:close\r\n\r\n");
	
		} else {
		sprintf(response,"HTTP/1.1 400 Bad Request\r\nConnection:close\r\n\r\n");
		}
		write(s2,response,strlen(response));
		if(fin!=NULL){
				while((car=fgetc(fin))!=EOF){
					write(s2,&car,1);
				}	
		}
		shutdown(s2,SHUT_RDWR);	
		close(s2);
	}
}
