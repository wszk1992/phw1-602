/*
*  For ECEN 602 
*  Programming Assignment 1, Group: Zheng, Kan   Liu, Bowei
*  
*
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>


#include <sys/socket.h>
#include <arpa/inet.h>

#include "chatroom_utils.h"

// get a username from the user.
void get_username(char *username, char* user_input_name)
{
  if(strlen(user_input_name) > 20)
  {
    // clear_stdin_buffer();
    puts("Username must be 20 characters or less.");
    exit(1);

  } else {
    memset(username, 0, 1000);
    strcpy(username, user_input_name);
  }
}

//send local username to the server.
void send_join(connection_info *connection)
{
  SBCP_message msg =  initSBCP(JOIN);
  pushAttr( &msg, Username, connection->username);
  if(send(connection->socket, (void*)&msg, sizeof(msg), 0) < 0)
  {
    perror("Send failed");
    exit(1);
  }
}

void stop_client(connection_info *connection)
{
  close(connection->socket);
  exit(0);
}

//initialize connection to the server.
void connect_to_server(connection_info *connection, char* user_input_name ,char *address, char *port)
{

  while(true)
  {
    get_username(connection->username, user_input_name );

    //Create socket
    if ((connection->socket = socket(AF_INET, SOCK_STREAM , IPPROTO_TCP)) < 0)
    {
        perror("Could not create socket");
    }

    connection->address.sin_addr.s_addr = inet_addr(address);
    connection->address.sin_family = AF_INET;
    connection->address.sin_port = htons(atoi(port));

    //Connect to remote server
    if (connect(connection->socket, (struct sockaddr *)&connection->address , sizeof(connection->address)) < 0)
    {
        perror("Connect failed.");
        exit(1);
    }

    send_join(connection);
    
    // there is no ACK, so no need to check following part
    /*
     SBCP_message msg;
     ssize_t recv_val = recv(connection->socket, &msg, sizeof(SBCP_message), 0);
    if(recv_val < 0)
     {
        perror("recv failed");
       exit(1);

    }
    else if(recv_val == 0)
     {
       close(connection->socket);
       printf("also full", connection->username);
       exit(1);
     }
     */
    break;
  }


  puts("Connected to server.");
  puts("Type your messages.");
}


void handle_user_input(connection_info *connection)
{
  char input[255];
  fgets(input, 255, stdin);
  trim_newline(input);

  // message msg;
  SBCP_message msg = initSBCP( SEND );
  // msg.type = PUBLIC_MESSAGE;
  pushAttr(&msg, Username, connection->username);

  // strncpy(msg.username, connection->username, 20);

  //clear_stdin_buffer();

  if(strlen(input) == 0) {
      return;
  }
  


  // strncpy(msg.data, input, 255);
  pushAttr(&msg, Message, input );

  //Send some data
  if(send(connection->socket, &msg, sizeof(msg), 0) < 0)
  {
      perror("Send failed");
      exit(1);
  }
}

void handle_server_message(connection_info *connection)
{
  SBCP_message msg;
  // message msg;

  //Receive a reply from the server
  ssize_t recv_val = recv(connection->socket, &msg, sizeof(msg), 0);
  if(recv_val < 0)
  {
      perror("recv failed");
      exit(1);

  }
  else if(recv_val == 0)
  {
    close(connection->socket);
    puts("Server disconnected.");
    exit(0);
  }
  header_type h_type = getHeaderType( msg);

  switch(h_type)
  {
    case FWD:;
      char username[20];
      char msg_content[256];
      getUsername(msg, username);
      getMessageContent(msg, msg_content);
      printf("%s: %s\n", username, msg_content);
    break;
    case NAK:;
      char corresponding_text[512];
	    getCorrespondingAttr(msg, corresponding_text, Reason ); //
      printf("%s\n",corresponding_text);
      close(connection->socket);
      exit(0);
    break;
    default:
      fprintf(stderr, "Unknown message type received.\n");
    break;
  }
}

int main(int argc, char *argv[])
{
  connection_info connection;
  fd_set file_descriptors;

  if (argc != 4) {
    fprintf(stderr,"Usage: %s <username> <IP> <port>\n", argv[0]);
    exit(1);
  }

  connect_to_server(&connection, argv[1], argv[2], argv[3]);

  //keep communicating with server
  while(true)
  {
    FD_ZERO(&file_descriptors);
    FD_SET(STDIN_FILENO, &file_descriptors);
    FD_SET(connection.socket, &file_descriptors);
    fflush(stdin);

    if(select(connection.socket+1, &file_descriptors, NULL, NULL, NULL) < 0)
    {
      perror("Select failed.");
      exit(1);
    }

    if(FD_ISSET(STDIN_FILENO, &file_descriptors))
    {
      handle_user_input(&connection);
    }

    if(FD_ISSET(connection.socket, &file_descriptors))
    {
      handle_server_message(&connection);
    }
  }

  close(connection.socket);
  return 0;
}
