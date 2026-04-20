/*
 * Copyright (C) 2026 Marcus Hirt
 *
 * This software is free:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#	include <winsock2.h>
#	include <ws2tcpip.h>
#	pragma comment(lib, "ws2_32.lib")
#	define CLOSE(s) closesocket(s)
typedef int socklen_t;
typedef SOCKET sock_t;
#else
#	include <arpa/inet.h>
#	include <netinet/in.h>
#	include <sys/socket.h>
#	include <unistd.h>
#	define CLOSE(s) close(s)
#	define INVALID_SOCKET (-1)
typedef int sock_t;
#endif

#define PACKET_LEN 102

static int parse_mac(const char *mac, unsigned char out[6]) {
	char hex[13];
	int j = 0;
	for (int i = 0; mac[i]; i++) {
		if (mac[i] != ':' && mac[i] != '-') hex[j++] = mac[i];
	}
	hex[j] = '\0';
	if (j != 12) return -1;
	for (int i = 0; i < 6; i++) {
		char b[3] = {hex[i * 2], hex[i * 2 + 1], 0};
		out[i] = (unsigned char) strtol(b, NULL, 16);
	}
	return 0;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Usage: wol <MAC> [broadcast-address] [port]\n");
		fprintf(stderr, "  MAC               AA:BB:CC:DD:EE:FF or AA-BB-CC-DD-EE-FF\n");
		fprintf(stderr, "  broadcast-address IPv4 broadcast address (default: 255.255.255.255)\n");
		fprintf(stderr, "                    Use subnet-directed broadcast (e.g. 192.168.1.255)\n");
		fprintf(stderr, "                    if your router blocks 255.255.255.255 across VLANs.\n");
		fprintf(stderr, "  port              UDP port (default: 9)\n");
		fprintf(stderr, "Example: wol AA:BB:CC:DD:EE:FF 192.168.1.255 9\n");
		return 1;
	}
	const char *mac       = argv[1];
	const char *broadcast = argc > 2 ? argv[2] : "255.255.255.255";
	int         port      = argc > 3 ? atoi(argv[3]) : 9;

	unsigned char macb[6];
	if (parse_mac(mac, macb) < 0) {
		fprintf(stderr, "Invalid MAC address: %s\n", mac);
		return 2;
	}

	unsigned char packet[PACKET_LEN];
	memset(packet, 0xFF, 6);
	for (int i = 6; i < PACKET_LEN; i += 6)
		memcpy(packet + i, macb, 6);

#ifdef _WIN32
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

	sock_t sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) { perror("socket"); return 3; }

	int one = 1;
	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (const char *) &one, sizeof(one));

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port   = htons((unsigned short) port);
	if (inet_pton(AF_INET, broadcast, &addr.sin_addr) != 1) {
		fprintf(stderr, "Invalid broadcast address: %s\n", broadcast);
		CLOSE(sock);
		return 4;
	}

	if (sendto(sock, (const char *) packet, PACKET_LEN, 0,
	           (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("sendto");
		CLOSE(sock);
		return 5;
	}
	CLOSE(sock);

#ifdef _WIN32
	WSACleanup();
#endif

	printf("Magic packet sent to %s via %s:%d\n", mac, broadcast, port);
	return 0;
}
