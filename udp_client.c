#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
	printf("UDP NAT Hole Punching PoC Client.\n");
	
	if(argc != 4) {
		printf("Usage: ./hpclient <client_port> <rendezvous_server_ip> <rendezvous_server_port>\n");
		exit(1);
	}

	int client_port = atoi(argv[1]);
	char* rendezvous_server_ip = argv[2];
	int rendezvous_server_port = atoi(argv[3]);

	int sock = socket(AF_INET, SOCK_DGRAM, 0);

	struct sockaddr_in client_addr, rendezvous_server_addr;

	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(client_port);
	client_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(client_addr.sin_zero), 8);

	bind(sock, (struct sockaddr *)&client_addr, sizeof(struct sockaddr));

	rendezvous_server_addr.sin_family = AF_INET;
	rendezvous_server_addr.sin_port = htons(rendezvous_server_port);
	inet_aton(rendezvous_server_ip, &rendezvous_server_addr.sin_addr.s_addr);
	bzero(&(rendezvous_server_addr.sin_zero), 8);
		

	// Send an empty UDP datagram to Rendezvous Server
	printf("Phase 1. Sending empty UDP to rendezvous server.\n");

	char* data = "";
	sendto(sock, data, strlen(data), 0,
		(struct sockaddr *)&rendezvous_server_addr, sizeof(struct sockaddr));
	printf("End of Phase 1.\n\n");


	// Receiving peer address from server
	printf("Phase 2. Receiving peer address from server.\n");
	
	int sockaddr_in_len = sizeof(struct sockaddr_in);
	char peer_bytes[sockaddr_in_len];

	int sockaddr_len = sizeof(struct sockaddr);
	recvfrom(sock, peer_bytes, sockaddr_in_len, 0,
		(struct sockaddr *)&rendezvous_server_addr, &sockaddr_len);

	struct sockaddr_in peer_addr;
	memcpy(&peer_addr, &peer_bytes, 16);

	printf("\tReceived peer address: (%s, %d)\n", inet_ntoa(peer_addr.sin_addr),
		ntohs(peer_addr.sin_port));
	printf("End of Phase 2.\n\n");


	// Simple UDP chat between peers
	printf("Phase 3. Establishing communication with peer");

	while(1) {
		printf("\tType a message to send to another peer: ");
		char buffer[1024];
		fgets(buffer, 1024, stdin);
		
		sendto(sock, buffer, strlen(buffer) + 1, 0,
			(struct sockaddr* )&peer_addr, sizeof(struct sockaddr));

		recvfrom(sock, buffer, 1024, 0,
			(struct sockaddr* )&peer_addr, &sockaddr_len);

		printf("\tReceived from peer: %s\n", buffer);
	}

	return 0;
}
