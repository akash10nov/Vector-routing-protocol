/**
 * @akashpra_assignment3
 * @author  akashprafulchandradesai <akashpra@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <netdb.h>
#include <string.h>
#include <sys/stat.h>

#include "../include/global.h"
#include "../include/logger.h"
#define STDIN 0
/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
 int listenersetup(fd_set* listenlist, uint16_t portnum) {

	int listener, yes = 1;

	struct sockaddr_in my_address;
	if ((listener = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket()");
		exit(EXIT_FAILURE);
	}
	memset(&my_address, 0, sizeof(struct sockaddr_in));
	my_address.sin_addr.s_addr = INADDR_ANY;
	my_address.sin_family = AF_INET;
	my_address.sin_port = portnum;
	/*
		for port re-usability.  
	*/
	setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  
	
	
	if (bind(listener, (struct sockaddr*)&my_address, sizeof(struct sockaddr_in)) < 0) {
		perror("bind()");
		exit(EXIT_FAILURE);
	}
	if(FD_ISSET(listener,listenlist)){
	}
	else{
	FD_SET(listener,listenlist);
	}
	if(FD_ISSET(STDIN,listenlist)){
	}
	else{
	FD_SET(STDIN, listenlist);
	}

	return listener;
}
 
int main(int argc, char **argv)
{
	/*Init. Logger*/
	cse4589_init_log();

	/*Clear LOGFILE and DUMPFILE*/
	fclose(fopen(LOGFILE, "w"));
	fclose(fopen(DUMPFILE, "wb"));
	FILE *fp=fopen(argv[2],"r");//http://stackoverflow.com/questions/174531/easiest-way-to-get-files-contents-in-c
	char buffer[256];
	size_t len = 0;
    ssize_t readbytes;
	uint16_t myid;
	//uint32_t ip;
	//uint16_t portnumber;
	int num_of_servers;
	int num_of_neighbors;
	uint16_t port_num[10];
	uint32_t ip_addr[10];
	int j=0;
	int disable_check[10];
	uint16_t calculate_route[10][10];
	for(j=0;j<100;j++)
	{
		disable_check[j]=0;
	}
	int counter=0;
	int id=0;
	
	uint16_t neighbor_id[100];
			uint16_t neighbor_cost[100];

while ((fgets(buffer, 256, fp))) {
			if (counter==0)
				num_of_servers=atoi(buffer);
			else if(counter==1) {
				num_of_neighbors=atoi(buffer);
				int k;
				for(k=0;k<num_of_servers;k++)
				{
					neighbor_cost[k]=65535;
				}
				}
			else if(counter>1 && counter<(2+num_of_servers))
			{
				char *token;
				int i=0;
				int x;
				token = strtok(buffer, " ");
				while(token!= NULL) {
					if(i==2)
					{
						port_num[x-1]=atoi(token);
						//printf("port %u\n",port_num[x-1]);
					}
					else if(i==1)
					{
						ip_addr[x-1]=inet_addr(token);
						//printf("%d\n",x);
						//printf("%" PRIu32 "\n",ip_addr[x-1]);
					}
					else if(i==0)
					{
						x=atoi(token);
					}
					i++;
					token = strtok(NULL, " ");
				}
			}
			else
			{
				char *token;
				int i=0;
				token = strtok(buffer, " ");
				while(token!= NULL) {
						if(i==0)
						{
							myid=atoi(token);
							//printf("inside loop:my id: %u\n",myid);
						}
						else if(i==1)
						{
							neighbor_id[id++]=atoi(token);
						}
						else
						{
							neighbor_cost[neighbor_id[id-1]-1]=atoi(token);
							disable_check[neighbor_id[id-1]-1]=1;
						}
						i++;
						token = strtok(NULL, " ");
						}
								
			}
							counter++;
           
					}
	//rewind(fp);
	fclose(fp);
	//printf("my id: %u\n",myid);
	// PA3 SubStructure of Update Packet starts
	struct server_details
	{
		uint32_t server_ip;
		uint16_t server_port_number;
		uint16_t zeropadding;
		uint16_t server_id;
		uint16_t distance;
	};
	// PA3 SubStructure of Update Packet End
	neighbor_cost[myid-1]=0;
	// PA3 Update Packet Starts
	struct update_packet
	{
		uint16_t number_of_update_fields;
		uint16_t portnumber;
		uint32_t ip;
		struct server_details information[];
	};
	//PA3 Update Packet End
	struct update_packet *packet;
	int nexthop[10][10];
	int qq;
	for(j=0;j<10;j++)
	{	
		for(qq=0;qq<10;qq++)
		{
		nexthop[j][qq]=-1;
		}
	}
	int miss_count[10];
	for(j=0;j<10;j++)
	{
		miss_count[j]=-1;
	}
	packet = (struct update_packet *) malloc (sizeof(struct update_packet) + sizeof(struct server_details) * num_of_servers);
	packet->number_of_update_fields=(htons)(num_of_servers);
	packet->portnumber=(htons)(port_num[myid-1]);
	packet->ip=ip_addr[myid-1];
	
	//printf("my ip: %u\n",ip_addr[myid-1]);
	for(j=0;j<num_of_servers;j++)
	{
		packet->information[j].server_ip=ip_addr[j];
		packet->information[j].server_port_number=(htons)(port_num[j]);
		packet->information[j].zeropadding=0;
		packet->information[j].server_id=(htons)(j+1);// for time being keep it this way but you need to change it..get the values from topology don't assume ordered
		packet->information[j].distance=(htons)(neighbor_cost[j]);
	}
	//at this point my sending packet is ready.
	/*Start Here*/
	struct timeval tv;
	tv.tv_sec = atoi(argv[4]);
	tv.tv_usec = 0;
	int listener;
	fd_set listenlist;
	fd_set selectloop;
	FD_ZERO(&listenlist);
	FD_ZERO(&selectloop);
	uint16_t myport=port_num[myid-1];
	int temp=listener = listenersetup(&listenlist,port_num[myid-1]);// get port number from the topology file
	FD_SET(listener,&listenlist);
	int count=0;
	//PA3 Routing Table Start
	
	uint16_t **routing_table = (uint16_t **) malloc(sizeof(uint16_t *)*num_of_servers);
    for(j=0; j<num_of_servers; j++)
		{
			routing_table[j] = (uint16_t *) malloc(sizeof(uint16_t)*num_of_servers); 
		}
	//PA3 Routing Table End
	for(qq=0;qq<num_of_servers;qq++)
	{
		for(j=0;j<num_of_servers;j++)
		{
			calculate_route[qq][j]=65535;
		}
	}
	for(j=0;j<num_of_servers;j++)
	{
	routing_table[myid-1][j]=neighbor_cost[j];
	routing_table[j][myid-1]=neighbor_cost[j];
	routing_table[j][j]=0;
	}
	for(j=0;j<num_of_servers;j++)
	{
		calculate_route[myid-1][j]=(ntohs)(packet->information[j].distance);
		calculate_route[j][myid-1]=(ntohs)(packet->information[j].distance);
	}
	for(j=0;j<num_of_servers;j++)
	{
		calculate_route[j][j]=0;
	}
	for(qq=0;qq<num_of_servers;qq++)
	{
		for(j=0;j<num_of_servers;j++)
		{
			routing_table[qq][j]=calculate_route[qq][j];
			routing_table[j][qq]=calculate_route[qq][j];
		}
	}
	
	for(j=0;j<num_of_servers;j++)
	{
		calculate_route[myid-1][j]=routing_table[myid-1][j];
		routing_table[j][j]=0;
	}
	size_t packetsize=sizeof(struct update_packet) + sizeof(struct server_details) * num_of_servers;
	//printf("packetsize:%u\n",packetsize);
	while(1)
	{
		
		fflush(stdout);
		selectloop=listenlist;
		int l;
		if (select(listener + 1, &selectloop, NULL, NULL, &tv) < 0) {
			perror("select()");
			break;
		}
		if (FD_ISSET(STDIN, &selectloop)) {
				char buff[256];
				char *cmd, *cmd_given;
				char** token_argv;

				int bytesRead, token_argc;
				int peer_sockfd;

				if ((bytesRead = read(STDIN, buff, 256)) < 0) {
					perror("read()");
					return 0;
				}

				buff[bytesRead - 1] = 0;

				cmd_given = (char*) malloc(strlen(buff) + 1);

				if(strlen(buff)==0)
				{
					cmd="onlyEnter";
				}
				else{
				strcpy(cmd_given, buff);
				cmd = strtok(buff, " ");

				}				
				while(1) {
					
					if (strcasecmp(cmd, "STEP") == 0) {
							int suc=0;
							
							for(j=0;j<num_of_servers;j++)
							{
								struct sockaddr_in neighbor;
								neighbor.sin_family = AF_INET;
								neighbor.sin_addr.s_addr = ip_addr[neighbor_id[j]-1];
								neighbor.sin_port = port_num[neighbor_id[j]-1];
								if(disable_check[neighbor_id[j]-1]==1)
								{
									suc=suc+1;
								if (sendto(listener,(void*)packet, packetsize, 0,
									(struct sockaddr*)&neighbor, sizeof(struct sockaddr_in)) < 0) {
									// will have to change in case of link down case;
								}
								}
							}
							if(suc>0)
								cse4589_print_and_log("%s:SUCCESS\n", "STEP");
							else
								cse4589_print_and_log("%s:%s\n","STEP","No neighbors found.");
								
							break;
					}
					if(strcasecmp(cmd, "PACKETS") == 0) {
						cse4589_print_and_log("%s:SUCCESS\n", "PACKETS");
						cse4589_print_and_log("%d\n",count);
						count=0;
						
						break;
					
					}
					if(strcasecmp(cmd, "ACADEMIC_INTEGRITY") == 0) {
						cse4589_print_and_log("%s:SUCCESS\n", "ACADEMIC_INTEGRITY");
						cse4589_print_and_log("I have read and understood the course academic integrity policy located at \"http://www.cse.buffalo.edu/faculty/dimitrio/courses/cse4589_f14/index.html#integrity\"");
						break;
					
					}
					if(strcasecmp(cmd, "CRASH")==0)
					{
						//printf("%s:SUCCESS\n", "CRASH");
						while(1)
						{
						}

					}
					if(strcasecmp(cmd, "DUMP")==0)
					{
						cse4589_print_and_log("%s:SUCCESS\n","DUMP");
						cse4589_dump_packet((void*)packet, packetsize);

						break;
					}
					if(strcasecmp(cmd, "DISPLAY")==0)
					{
						cse4589_print_and_log("%s:SUCCESS\n", "DISPLAY");
						for(j=0;j<num_of_servers;j++)
						{
							if(nexthop[myid-1][j]==-1 && disable_check[j]==1)
							{
								nexthop[myid-1][j]=j+1;
							}
							if(routing_table[myid-1][j]==65535)
							{
								nexthop[myid-1][j]=-1;
							}
							cse4589_print_and_log("%-15d%-15d%-15d\n",(j+1),nexthop[myid-1][j],routing_table[myid-1][j]);
						}
						break;
					}
					if(strcasecmp(cmd, "UPDATE")==0)
					{
						token_argv = (char**) malloc(4 * sizeof(char*));
						int argcc=0;
						while(argcc < 4 && cmd) {
							token_argv[argcc] = (char*) malloc(strlen(cmd) + 1);
							strcpy(token_argv[argcc++],cmd);
							cmd = strtok(NULL, " ");
						}
						if(argcc != 4 || cmd) {
							cse4589_print_and_log("%s:%s\n","UPDATE","USAGE: UPDATE ID1 ID2 COST");
							break;
						}
						uint16_t id1 = atoi(token_argv[1]);
						uint16_t id2 = atoi(token_argv[2]);
						uint16_t updated_cost;
						if(strcasecmp(token_argv[3], "inf")==0)
							updated_cost=65535;
						else
						updated_cost=atoi(token_argv[3]);
						if(disable_check[id2-1]==1 && id1==(myid)){
							cse4589_print_and_log("%s:SUCCESS\n", "UPDATE");
							packet->information[id2-1].distance=(htons)(updated_cost);
							calculate_route[myid-1][id2-1]=updated_cost;
							calculate_route[id2-1][myid-1]=updated_cost;
							for(j=0;j<num_of_servers;j++)
							{
								calculate_route[j][j]=0;
							}
							int i,k;
							
							for(i=0;i<num_of_servers;i++)
							{
								for(j=0;j<num_of_servers;j++)
								{
									routing_table[i][j]=calculate_route[i][j];
									routing_table[j][i]=calculate_route[i][j];
								}
							}
							
							for (i = 0; i < num_of_servers; i++)
							{
								for (j = 0; j < num_of_servers;j++)
								{
									for(k=0;k<num_of_servers;k++)
									{
									if (((routing_table[i][k] + routing_table[k][j]) < routing_table[i][j]) && routing_table[i][k]!=65535 && routing_table[k][j]!=65535)
										{
										routing_table[i][j] = routing_table[i][k] + routing_table[k][j];
										//routing_table[j][i] = routing_table[i][k] + routing_table[k][j];
										nexthop[i][j]=k+1;
										}
									}

								}
							}
							for(j=0;j<num_of_servers;j++)
							{
								packet->information[j].distance=htons(routing_table[myid-1][j]);
								calculate_route[myid-1][j]=routing_table[myid-1][j];
							}
							
						}
						else{
						if(id1!=myid)
							cse4589_print_and_log("%s:%s\n", "UPDATE", "ID1 IS NOT SERVER'S ID");
						else
							cse4589_print_and_log("%s:%s\n", "UPDATE", "ID2 IS NOT A NEIGHBOR.");

						}
						break;
					}
					if(strcasecmp(cmd, "DISABLE")==0)
					{
						token_argv = (char**) malloc(2 * sizeof(char*));
						int argcc=0;
						while(argcc < 2 && cmd) {
							token_argv[argcc] = (char*) malloc(strlen(cmd) + 1);
							strcpy(token_argv[argcc++],cmd);
							cmd = strtok(NULL, " ");
						}
						if(argcc != 2 || cmd) {
							printf("%s:%s\n","DISABLE","USAGE: DISABLE ID1");
							break;
						}
						
						uint16_t del_id=atoi(token_argv[1]);
						if(disable_check[del_id-1]==0){
						
							cse4589_print_and_log("%s:%s\n", "DISABLE", "ID IS NOT A NEIGHBOR.");
						}
						else
						{	
							cse4589_print_and_log("%s:SUCCESS\n", "DISABLE");
							packet->information[del_id-1].distance=(htons)(65535);
							routing_table[myid-1][del_id-1]=65535;
							routing_table[del_id-1][myid-1]=65535;
							for(j=0;j<num_of_servers;j++)
							{
								calculate_route[j][j]=0;
							}
							int i,k;
							for(i=0;i<num_of_servers;i++)
							{
								for(j=0;j<num_of_servers;j++)
								{
									routing_table[i][j]=calculate_route[i][j];
								}
							}
							for (i = 0; i < num_of_servers; i++)
							{
								for (j = 0; j < num_of_servers;j++)
								{
									for(k=0;k<num_of_servers;k++)
									{
									if (((routing_table[i][k] + routing_table[k][j]) < routing_table[i][j]) && routing_table[i][k]!=65535 && routing_table[k][j]!=65535)
										{
										routing_table[i][j] = routing_table[i][k] + routing_table[k][j];
										nexthop[i][j]=k+1;
										}
									}

								}
							}
							for(j=0;j<num_of_servers;j++)
							{
								packet->information[j].distance=(htons)(routing_table[myid-1][j]);
								calculate_route[myid-1][j]=routing_table[myid-1][j];
							
							}
							disable_check[del_id-1]=0;
						}
						
						break;
					}
					else{
					printf("%s:%s\n", "STDIN", "INSERT A RIGHT COMMAND.");
					break;
					}
					
		}
		}
		else if (FD_ISSET(listener, &selectloop)) {
		
	
				//from beej's guide.
			socklen_t fromlen;
			struct sockaddr_storage addr;
			struct update_packet *received_packet;
			received_packet = (struct update_packet *) malloc (sizeof(struct update_packet) + sizeof(struct server_details) * num_of_servers);
			int sizebytes;
			fromlen = sizeof addr;
			uint16_t received_packet_id;
			sizebytes= recvfrom(listener, (void*)received_packet, packetsize, 0, (struct sockaddr*)&addr, &fromlen);
			for(j=0;j<num_of_servers;j++)
			{
				if(received_packet->ip==ip_addr[j]){
					
					break;
					}
			}
			int i,k;
			if(disable_check[j]==1)
			{
			count++;
			for(i=0;i<num_of_servers;i++)
			{
				calculate_route[j][i]=(ntohs)(received_packet->information[i].distance);
				calculate_route[i][j]=(ntohs)(received_packet->information[i].distance);
			}
			received_packet_id=j+1;
			
			cse4589_print_and_log("RECEIVED A MESSAGE FROM SERVER %d\n", received_packet_id);
			miss_count[j]=0;
			
			for(j=0;j<num_of_servers;j++)
			{
				routing_table[received_packet_id-1][j]=(ntohs)(received_packet->information[j].distance);
				routing_table[j][received_packet_id-1]=(ntohs)(received_packet->information[j].distance);
				cse4589_print_and_log("%-15d%-15d\n",(j+1), (ntohs)(received_packet->information[j].distance));

				//printf("distance received: %u\n",received_packet->information[j].distance);
				packet->information[received_packet_id-1].distance=received_packet->information[myid-1].distance;
			}// will work on update too. since you are changing a weight too.
			for(j=0;j<num_of_servers;j++)
			{
					calculate_route[j][j]=0;
			}
			for(i=0;i<num_of_servers;i++)
			{
				for(j=0;j<num_of_servers;j++)
				{
					routing_table[i][j]=calculate_route[i][j];
				}
			}
			routing_table[myid-1][myid-1] = 0;
			for (i = 0; i < num_of_servers; i++)
			{
				for (j = 0; j < num_of_servers;j++)
				{
					for(k=0;k<num_of_servers;k++)
					{
					if (((routing_table[i][k] + routing_table[k][j]) < routing_table[i][j]) && routing_table[i][k]!=65535 && routing_table[k][j]!=65535)
						{
						routing_table[i][j] = routing_table[i][k] + routing_table[k][j];
						nexthop[i][j]=k+1;
						}
					}

				}
			}
			for(j=0;j<num_of_servers;j++)
			{
				packet->information[j].distance=(htons)(routing_table[myid-1][j]);
				calculate_route[myid-1][j]=routing_table[myid-1][j];
			}
			}
			
		}
		else{
			/*
				send packets to all neighbours 
			*/
			int i,k;
			for(j=0;j<num_of_servers;j++)
			{
				calculate_route[j][j]=0;
			}
			for(i=0;i<num_of_servers;i++)
			{
				for(j=0;j<num_of_servers;j++)
				{
					routing_table[i][j]=calculate_route[i][j];
				}
			}
			for(i=0;i<num_of_servers;i++)
			{
				if(miss_count[i]!=-1)
					{
					miss_count[i]=miss_count[i]+1;
					if(miss_count[i]>=3)
						{
							packet->information[i].distance=(htons)(65535);
							routing_table[myid-1][i]=65535;
							routing_table[i][myid-1]=65535;
							for(j=0;j<num_of_servers;j++)
							{
								calculate_route[i][j]=65535;
								calculate_route[j][i]=65535;
							}
							for(j=0;j<num_of_servers;j++)
							{
								calculate_route[j][j]=0;
							}
						}
					}
			}
			for(i=0;i<num_of_servers;i++)
			{
				for(j=0;j<num_of_servers;j++)
				{
					routing_table[i][j]=calculate_route[i][j];
				}
			}
			routing_table[myid-1][myid-1] = 0;
			for (i = 0; i < num_of_servers; i++)
			{
				for (j = 0; j < num_of_servers;j++)
				{
					for(k=0;k<num_of_servers;k++)
					{
					if (((routing_table[i][k] + routing_table[k][j]) < routing_table[i][j]) && routing_table[i][k]!=65535 && routing_table[k][j]!=65535)
						{
						routing_table[i][j] = routing_table[i][k] + routing_table[k][j];
						nexthop[i][j]=k+1;
						}
					}

				}
			}
			for(j=0;j<num_of_servers;j++)
			{
				packet->information[j].distance=(htons)(routing_table[myid-1][j]);
				calculate_route[myid-1][j]=routing_table[myid-1][j];
			}
			for(j=0;j<num_of_neighbors;j++)
			{
				struct sockaddr_in neighbor;
				neighbor.sin_family = AF_INET;
				neighbor.sin_addr.s_addr = ip_addr[neighbor_id[j]-1];
				neighbor.sin_port = port_num[neighbor_id[j]-1];
				size_t packetsize1=sizeof(packet);
				
				if(disable_check[neighbor_id[j]-1]==1){
				if (sendto(listener,(void*)packet, packetsize, 0,
					(struct sockaddr*)&neighbor, sizeof(struct sockaddr_in)) < 0) {
					printf("Sendto() Failure\n");
				}
				else{
				//printf("Message sent.\n");
				}
				}
				else{
				}
				
			}
			
			
			tv.tv_sec = atoi(argv[4]);
			tv.tv_usec = 0;
		}
	}

	return 0;
}




// Make and server size array: if neighbor value=1 else value=0; disable then value=0, before sending check value.