#include <unistd.h>
#include <arpa/inet.h>
#include<errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>
struct sockaddr_in server;
struct headers{
char * n;
char * v;
}h[100];
int c_l = 0;
// ip(7) socket(2) connect(2) errno(3) perror(3) 
int main()
{
	int size,s,t,k,j,w;
	char request[100];
	char response[10000];
	char * entity_body;
	unsigned char * p;
	unsigned char a[4]={88,80,187,84};	
	s= socket(AF_INET, SOCK_STREAM ,0);
	if ( s == -1 ) { printf("Errno=%d ",errno); perror("Socket Fallita"); return 1; }
	printf("%d\n",s);
	server.sin_family=AF_INET;

	server.sin_port = htons(3459);

	// Modalita 1 (solo se little endian)
	server.sin_addr.s_addr = 0xA3D33AD8;      // 216.58.211.163	
	// Modalita 2 (qualunque endianness)
	p = (unsigned char *)&server.sin_addr.s_addr;
	p[0]=216;p[1]=58;p[2]=211;p[3]=163;
	// Modalita 3 (qualunque endianness)
	server.sin_addr.s_addr = *(unsigned int *)a;
	
	t = connect(s,(struct sockaddr *)&server,sizeof(struct sockaddr_in));		
	if ( t == -1 ) { printf("Errno=%d ",errno); perror("Connect Fallita"); return 1; }
	sprintf(request,"GET / HTTP/1.1\r\nHost:www.google.it\r\n\r\n");//request[0]='G';request[1]='E';...;request[8]=0;
	for(size=0;request[size];size++){}// size = strlen(request);	
	write(s,request,size);	
	j=0;k=0;
        h[k].n = response;
        while(read(s,response+j,1)){
        if((response[j]==':') && (!h[k].v )){
                response[j]=0;
                h[k].v=response+j+1;
                }
        if((response[j]=='\n') && (response[j-1]=='\r')){
                response[j-1]=0;
                if(h[k].n[0]==0) break;
		if(k>0) while((*h[k].v)==' ') h[k].v++;
                h[++k].n=response+j+1;
                }
        j++;
        }
	printf("%s\n",h[0].n);
	 c_l=0;
	 for(k=1;h[k].n[0];k++){
		printf("Header:%s--->%s\n",h[k].n,h[k].v);	
		if(!strcmp(h[k].n,"Content-Length"))
			for(w=0;h[k].v[w];w++)
				c_l=c_l*10+h[k].v[w]-'0';
	}
	if (c_l){	
		entity_body = malloc(c_l+1);
		for(size=0;k = read(s, entity_body + size,c_l-size);size+=k);
		}
	else{
		entity_body = malloc (1000000);
		for(size=0;k = read( s, entity_body + size,999999-size);size+=k);
		}
	entity_body[size]=0;
	printf("%s",entity_body);
	free(entity_body);

}
