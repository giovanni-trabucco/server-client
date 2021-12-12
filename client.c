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

struct chunk {  		//per fare il parsing dei chunk
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

	// Modalita 1 (solo se little endian)
	server.sin_addr.s_addr = 0xA3D33AD8;      // 216.58.211.163	
	// Modalita 2 (qualunque endianness)
	p = (unsigned char *)&server.sin_addr.s_addr;
	p[0]=216;p[1]=58;p[2]=211;p[3]=163;
	// Modalita 3 (qualunque endianness)
	server.sin_addr.s_addr = *(unsigned int *)a;
	
	t = connect(s,(struct sockaddr *)&server,sizeof(struct sockaddr_in));		
	if ( t == -1 ) { printf("Errno=%d ",errno); perror("Connect Fallita"); return 1; }
	sprintf(request,"GET /prova2.html HTTP/1.1\r\nHost:\r\n\r\n");//request[0]='G';request[1]='E';...;request[8]=0;
	for(size=0;request[size];size++){}// size = strlen(request);	
	write(s,request,size);	
	j=0;k=0;
        h[k].n = response;
        while(read(s,response+j,1)){			//inizio il parsing degli header 
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
	 for(k=1;h[k].n[0];k++){						//cerco se ho fatto un errore nella richiesta,
		printf("Header:%s--->%s\n",h[k].n,h[k].v);	//cioe' se trovo content-length
		if(!strcmp(h[k].n,"Content-Length"))
			for(w=0;h[k].v[w];w++)
				c_l=c_l*10+h[k].v[w]-'0';
	}
	if (c_l){										//ho trovato content-length
		entity_body = malloc(c_l+1);
		for(size=0;(k = read(s, entity_body + size,c_l-size));size+=k);
		entity_body[size]=0;
		printf("%s",entity_body);
		free(entity_body); //ridondante ma vabbe'
		}

	else{											//non l' ho trovato:mi arrivano i chunk
		entity_body = malloc (1000000);
		
		n = 1;
		z = 0;
		int sono_in_size = TRUE;
		read (s, entity_body,1);  //leggo un solo carattere: la prima cifra del size del primo chunk
		c[z].chunk_size = entity_body; //e poi faccio puntare c[0].chunk_size a quel carattere

		while (read(s,entity_body+n,1)) {
			if (sono_in_size) { 	//sto leggendo un campo size
				if (entity_body[n] == '\n' && entity_body[n-1] == '\r') {
					entity_body[n-1] = 0;
					if (c[z].chunk_size[0] == '0') break; //mi fermo perche sono all' ultimo chunk
					c[z].data = entity_body+n+1;
					sono_in_size = FALSE; // il prossimo campo sara' data
				}
				//altrimenti leggi il prossimo carattere
			}

			else if (!sono_in_size) { 	//sto leggendo un campo data
				if (entity_body[n] == '\n' && entity_body[n-1] == '\r') {
					entity_body[n-1] = 0;
					c[++z].chunk_size = entity_body+n+1;
					sono_in_size = TRUE; // il prossimo campo sara' size
				}
				//altrimenti leggi il prossimo carattere
			}
			n++;

		}
		printf("\nENTITY BODY:\n\n");
		for (q = 0; q<z;q++) { 					//stampo solo i campi data; non mi interessa size
			printf("%s",c[q].data);
		}

	free(entity_body); //ridondante ma vabbe'
	} 

} //fine main
