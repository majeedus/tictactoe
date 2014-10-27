#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>

#include <iostream>
#include <sstream>
#include <cstring>

#include <netdb.h>
#include <unistd.h>

#include "packet.h"      // defined by us
#include "lab1_client.h" // some supporting functions.
#include "Tic_Tac_Toe.h"

using namespace std;

// Usman Majeed
// 9/25/2014
// Tic Tac Toe Client Side

int main(int argc, char *argv[])
{
	sockaddr_in          server_addr;
	sockaddr_in          tcp_server_addr;
	sockaddr_in          tcp_connection_addr;
    socklen_t            tcp_server_addr_len = sizeof(tcp_server_addr);
	
	hostent			     *hp; 	// Address of remote host
	
    int                  tcp_connection_fd;
    int                  udp_socket_fd;

	int       		     tcp_server_fd;
	
    int                  bytes_received;
    int                  bytes_sent;

	int					 sk, sk2;	// socket descriptors
	sockaddr_in			 remote; 	// socket address for remote
	sockaddr_in			 local;		// socket address for us
	int  				 len=sizeof(local); // Length of local address
	
	
    char                 *server_name_str = 0;
    unsigned short int   tcp_server_port;
    unsigned short int   udp_server_port;

    My_Packet            incoming_pkt;
    My_Packet            outgoing_pkt;

    char                 type_name[type_name_len];
    char                 my_mark;

	// parse the argvs, obtain server_name and tcp_server_port
    parse_argv(argc, argv, &server_name_str, tcp_server_port);
		
	cout << "[TCP] Tic Tac Toe client started..." << endl;
    cout << "[TCP] Connecting to server: " << server_name_str
		<< ":" << tcp_server_port << endl;
		
	// details are covered in class/slides
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(tcp_server_port); // ask the OS to assign the port
    server_addr.sin_addr.s_addr = INADDR_ANY;
	
    // Create the socket
	// create a TCP listening socket for clients to connect
    tcp_connection_fd = socket(AF_INET, SOCK_STREAM, 0);

	// Get the address of the remote host and store
	hp = gethostbyname(server_name_str);
	
	// store
    memcpy(&server_addr.sin_addr, hp->h_addr, hp->h_length);

	// Send error if you cannot connect
    if( connect(tcp_connection_fd, (struct sockaddr * )&server_addr, sizeof(server_addr)) < 0 ) {
        cout << "connection error!" << endl;
        close(tcp_connection_fd);
        exit(1);
    }
	
	// Send JOIN message
	memset(&outgoing_pkt, 0, sizeof(outgoing_pkt));
	outgoing_pkt.type = JOIN;

	bytes_sent = send(tcp_connection_fd, 
					  &outgoing_pkt, 
					  sizeof(outgoing_pkt), 
					  0);
	
    // Check
    if(bytes_sent < 0) {
        cerr << "[ERR] Error sending message to client." << endl;
        exit(1);
    } else {
        get_type_name(outgoing_pkt.type, type_name);
        cout << "[TCP] Sent: " << type_name << " "
             << outgoing_pkt.buffer << endl;
    }
	

	// Clear the incoming packet buffer
    memset(&incoming_pkt, 0, sizeof(incoming_pkt));
    // Receive
    bytes_received = recv(tcp_connection_fd, 
                          &incoming_pkt, 
                          sizeof(incoming_pkt), 
                          0);

    // Check
    if(bytes_received < 0) {
        cerr << "[ERR] Error receiving message from client" << endl;
        return false;
    } else {
        get_type_name(incoming_pkt.type, type_name);
        cout << "[TCP] Recv: " << type_name << " "
             << incoming_pkt.buffer << endl;
    }
	
	// Set the first character mark
	my_mark = incoming_pkt.buffer[0];
	
	//--------//-------------//------------//--------------//-------//
						//Let's begin with UDP now//
	//--------//-------------//------------//--------------//-------//
	
	memset(&outgoing_pkt, 0, sizeof(outgoing_pkt));
	outgoing_pkt.type = GET_UDP_PORT;
	
	bytes_sent = send(tcp_connection_fd, 
					  &outgoing_pkt, 
					  sizeof(outgoing_pkt), 
					  0);
	// check
	if(bytes_sent < 0) {
		cerr << "[ERR] Error sending message to client." << endl;
		exit(1);
	} else {
		get_type_name(outgoing_pkt.type, type_name);
		cout << "[TCP] Sent: " << type_name << " "
			 << outgoing_pkt.buffer << endl;
	}
	
	// Clear the incoming packet buffer
    memset(&incoming_pkt, 0, sizeof(incoming_pkt));
    // Receive
    bytes_received = recv(tcp_connection_fd, 
                          &incoming_pkt, 
                          sizeof(incoming_pkt), 
                          0);
						  
	// Check
    if(bytes_received < 0) {
        cerr << "[ERR] Error receiving message from client" << endl;
        return false;
    } else {
        get_type_name(incoming_pkt.type, type_name);
        cout << "[TCP] Recv: " << type_name << " "
             << incoming_pkt.buffer << endl;
    }

	// Setup the board
	udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    server_addr.sin_port = htons( atoi( incoming_pkt.buffer ) );
	
	//--------//-------------//------------//--------------//-------//
						//Let's start the game//
	//--------//-------------//------------//--------------//-------//
	bool user_quit = false;
	Tic_Tac_Toe game;
	
	while(true) {
		memset(&outgoing_pkt, 0, sizeof(outgoing_pkt));
		outgoing_pkt.type = GET_BOARD;
		outgoing_pkt.buffer[0] = my_mark;

		// Get the board
		bytes_sent = sendto(udp_socket_fd, 
			&outgoing_pkt, 
			sizeof(outgoing_pkt),
			0,
			(sockaddr*)&server_addr, 
			sizeof(server_addr) );
		
		// Check
		if(bytes_sent < 0) {
			cerr << "[ERR] Error sending message to client." << endl;
			exit(1);
		} else {
			get_type_name(outgoing_pkt.type, type_name);
			cout << "[UDP] Sent: " << type_name << " "
				 << outgoing_pkt.buffer << endl;
		}
		
		// Sent message, now look for response
		memset(&incoming_pkt, 0, sizeof(incoming_pkt));
		unsigned int server_addr_len = sizeof(server_addr);  //Weird error if I dont make this unsigned      
        bytes_received = recvfrom(udp_socket_fd, 
            &incoming_pkt, 
            sizeof(incoming_pkt), 
            0,  
            (sockaddr*)&server_addr, 
            &server_addr_len );

        // Check
        if(bytes_received < 0) {
            cerr << "[ERR] Error receiving message from client (UDP)" << endl;
            exit(1);
        } else {
            get_type_name(incoming_pkt.type, type_name);

            cout << "[UDP] Recv: " << type_name << " " << incoming_pkt.buffer << endl;
        }
	
			//--------//-------------//------------//--------------//-------//
						//Let's analyze the responses from clients//
			//--------//-------------//------------//--------------//-------//
		
		 if (incoming_pkt.type == YOUR_TURN) 
		{
            Tic_Tac_Toe game(incoming_pkt.buffer); 
            game.print_board();
            cout << "[SYS] Your turn." << endl;
			
			// my_mark is empty, the client program needs to get it from the server.
			while(get_command(outgoing_pkt, game, my_mark) == false){}
			
			if (outgoing_pkt.type == EXIT)
				{ user_quit = true; }
				
	            bytes_sent = sendto(udp_socket_fd, 
                &outgoing_pkt, 
                sizeof(outgoing_pkt),
                0,
                (sockaddr*)&server_addr, 
                sizeof(server_addr) );

				// Check
				if(bytes_sent < 0) {
					cerr << "[ERR] Error sending message to client." << endl;
					exit(1);
				} 
				else {
					get_type_name(outgoing_pkt.type, type_name);
					cout << "[UDP] Sent: " << type_name << " "
						 << outgoing_pkt.buffer << endl;
				}
				
				if (outgoing_pkt.type == EXIT)
				{ user_quit = true; }
				
			} else if (incoming_pkt.type == UPDATE_BOARD){
				Tic_Tac_Toe game(incoming_pkt.buffer);
			} else if (incoming_pkt.type == YOU_WON){
				cout << "You won!" << endl;
				close(tcp_connection_fd);
				close(udp_socket_fd);
				exit(1);				
			} else if (incoming_pkt.type == TIE){
				cout << "It's a tie!" << endl;
				close(tcp_connection_fd);
				close(udp_socket_fd);
				exit(1);				
			} else if (incoming_pkt.type == YOU_LOSE){
				cout << "You lose!" << endl;
				close(tcp_connection_fd);
				close(udp_socket_fd);
				exit(1);				
			} else if (incoming_pkt.type == EXIT_GRANT){
				if (user_quit == true)
				{ cout << "You quit, you lose!" << endl; }
				else 
				{ cout << "Opponent quit, you win!" << endl; }
				close(tcp_connection_fd);
				close(udp_socket_fd);
				exit(1);
				}
					
    
	}
	

}



