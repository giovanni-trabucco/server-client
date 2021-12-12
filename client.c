#include <unistd.h>
#include <arpa/inet.h>
#include<errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>
#define TRUE 1
#define FALSE 0
struct sockaddr_in server;
struct headers{
	char * n;
	char * v;
}h[100];

struct chunk { //to parse chunks
	char* chunk_size;
	char* data;
}c[100];

int c_l = 0;
// ip(7) socket(2) connect(2) errno(3) perror(3) 
int main()
{
	int size,s,t,k,j,w,q,z,n;
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

	// Mode 1 (only if little endian)
	server.sin_addr.s_addr = 0xA3D33AD8;      // 216.58.211.163	
	// Mode 2 (whatever endianness)
	p = (unsigned char *)&server.sin_addr.s_addr;
	p[0]=216;p[1]=58;p[2]=211;p[3]=163;
	// Mode 3 (whatever endianness)
	server.sin_addr.s_addr = *(unsigned int *)a;
	
	t = connect(s,(struct sockaddr *)&server,sizeof(struct sockaddr_in));		
	if ( t == -1 ) { printf("Errno=%d ",errno); perror("Connect Fallita"); return 1; }
	sprintf(request,"GET /prova2.html HTTP/1.1\r\nHost:\r\n\r\n");//request[0]='G';request[1]='E';...;request[8]=0;
	for(size=0;request[size];size++){}// size = strlen(request);	
	write(s,request,size);	
	j=0;k=0;
        h[k].n = response;
        while(read(s,response+j,1)){ //start parsing headers
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
	 for(k=1;h[k].n[0];k++){//looking for errors in the request,
		printf("Header:%s--->%s\n",h[k].n,h[k].v);//that is, if i find 'Content-Length'
		if(!strcmp(h[k].n,"Content-Length"))
			for(w=0;h[k].v[w];w++)
				c_l=c_l*10+h[k].v[w]-'0';
	}
	if (c_l){//I found 'Content-Length'
		entity_body = malloc(c_l+1);
		for(size=0;(k = read(s, entity_body + size,c_l-size));size+=k);
		entity_body[size]=0;
		printf("%s",entity_body);
		free(entity_body); //redundant here
		}

	else{ // didnt' find it, receiving chunks
		entity_body = malloc (1000000);
		
		n = 1;
		z = 0;
		int sono_in_size = TRUE;
		read (s, entity_body,1);  //reading only one char: first digit of size of first chunk
		c[z].chunk_size = entity_body; //c[0].chunk_size points to that char 

		while (read(s,entity_body+n,1)) {
			if (sono_in_size) { //reading size
				if (entity_body[n] == '\n' && entity_body[n-1] == '\r') {
					entity_body[n-1] = 0;
					if (c[z].chunk_size[0] == '0') break; //stopping because I am at last chunk
					c[z].data = entity_body+n+1;
					sono_in_size = FALSE; // next field will be data
				}
				//otherwise reading next char
			}

			else if (!sono_in_size) { //reading data field
				if (entity_body[n] == '\n' && entity_body[n-1] == '\r') {
					entity_body[n-1] = 0;
					c[++z].chunk_size = entity_body+n+1;
					sono_in_size = TRUE; // next field will be size
				}
				//otherwise reading next char
			}
			n++;

		}
		printf("\nENTITY BODY:\n\n");
		for (q = 0; q<z;q++) { //printing only data, I don't need size
			printf("%s",c[q].data);
		}

	free(entity_body); //redundant
	} 

} //end of main
