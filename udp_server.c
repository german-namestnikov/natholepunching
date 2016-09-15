#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
	printf("UDP NAT Hole Punching PoC Server.\n");
	if(argc != 2) {
		printf("Usage: ./hpserver <port>\n");
		exit(1);
	}

	int server_port = atoi(argv[1]);

	int server_socket = socket(AF_INET, SOCK_DGRAM, 0);

	struct sockaddr_in server_addr, peers_addr[2];

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(server_addr.sin_zero), 8);
	
	bind(server_socket, (struct sockaddr*)&server_addr, sizeof(struct sockaddr));
	

	// Wait for UDP datagrams from peers
	printf("Phase 1. Waiting for peers on port %d.\n", server_port);

	int addr_len = sizeof(struct sockaddr);
	
	int current_peer = 0;
	char recv_data[1024];

	while (current_peer < 2)
	{
		recvfrom(server_socket, &recv_data, 1024, 0, 
			(struct sockaddr*) &peers_addr[current_peer], &addr_len);

		printf("\tNew peer found: (%s, %d)\n", 
			inet_ntoa(peers_addr[current_peer].sin_addr), 
			ntohs(peers_addr[current_peer].sin_port)); 
		current_peer ++;
	}
	printf("End of Phase 1.\n\n");
	
	// Send NATed IP and Ports of peers to each other
	printf("Phase 2. Sending holes to peers.\n");

	printf("\tSending IP and Port of second peer to the first peer.\n");	
	sendto(server_socket, &peers_addr[0], sizeof(struct sockaddr_in), 0,
		(struct sockaddr*) &peers_addr[1], sizeof(struct sockaddr));

	printf("\tSending IP and Port of first peer to the second peer.\n");
	sendto(server_socket, &peers_addr[1], sizeof(struct sockaddr_in), 0,
		(struct sockaddr*)&peers_addr[0], sizeof(struct sockaddr));

	printf("End of Phase 2.\n\n");
	printf("Exiting.\n\n");
	
	return 0;
}
