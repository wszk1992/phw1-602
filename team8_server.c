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
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "chatroom_utils.h"

int MAX_CLIENTS;

void initialize_server(connection_info *server_info, int port)
{
  if((server_info->socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("Failed to create socket");
    exit(1);
  }

  server_info->address.sin_family = AF_INET;
  server_info->address.sin_addr.s_addr = INADDR_ANY;
  server_info->address.sin_port = htons(port);

  if(bind(server_info->socket, (struct sockaddr *)&server_info->address, sizeof(server_info->address)) < 0)
  {
    perror("Binding failed");
    exit(1);
  }

  const int optVal = 1;
  const socklen_t optLen = sizeof(optVal);
  if(setsockopt(server_info->socket, SOL_SOCKET, SO_REUSEADDR, (void*) &optVal, optLen) < 0)
  {
    perror("Set socket option failed");
    exit(1);
  }


  if(listen(server_info->socket, 3) < 0) {
    perror("Listen failed");
    exit(1);
  }

  //Accept and incoming connection
  printf("Waiting for incoming connections...\n");
}


// this is actually forword
void send_public_message(connection_info clients[], int sender, char *message_text)
{
  SBCP_message msg = initSBCP(FWD);
  // msg.type = PUBLIC_MESSAGE;
  pushAttr(&msg, Username, clients[sender].username);
  // strncpy(msg.username, clients[sender].username, 20);
  pushAttr(&msg, Message, message_text);
  // strncpy(msg.data, message_text, 256);
  int i = 0;
  for(i = 0; i < MAX_CLIENTS; i++)
  {
    if(i != sender && clients[i].socket != 0)
    {
      if(send(clients[i].socket, &msg, sizeof(msg), 0) < 0)
      {
        perror("Send failed");
        exit(1);
      }
    }
  }
}


//close all the sockets before exiting
void stop_server(connection_info connection[])
{
  int i;
  for(i = 0; i < MAX_CLIENTS; i++)
  {
    //send();
    close(connection[i].socket);
  }
  exit(0);
}



void handle_client_message(connection_info clients[], int sender)
{
  int read_size;
  SBCP_message msg;

  if((read_size = recv(clients[sender].socket, &msg, sizeof(msg), 0)) == 0)
  {
    printf("User disconnected: %s.\n", clients[sender].username);
    close(clients[sender].socket);
    clients[sender].socket = 0;
    clients[sender].username[0] = 0;
    //send_disconnect_message(clients, clients[sender].username);

  } else {
    // interprete msg
    header_type h_type = getHeaderType( msg );


    switch(h_type)
    {

      case JOIN: ;    
      
      // JOIN
        // duplicated name
      char tempname[21];
      getUsername(msg, tempname);
      int i;
      for(i = 0; i < MAX_CLIENTS; i++)
      {
        if(strcmp(clients[i].username, tempname) == 0)
        {
          puts("Dected one duplicated usename");
          send_duplicated_name(clients[sender].socket);
          close(clients[sender].socket);
          clients[sender].socket = 0;
          return;
        }
      }
      puts("Server detected one JOIN");
      strcpy(clients[sender].username, tempname);
      printf("User connected: %s\n", clients[sender].username);
          //send_connect_message(clients, sender);
      break;

      case SEND:
        puts("Server detected one SEND");
        char msg_content[256];
        getMessageContent(msg, msg_content);
        send_public_message(clients, sender, msg_content);  //TODO
        break;

        default:
        fprintf(stderr, "Unknown message type received.\n");
        break;
    }
  }
}

  int construct_fd_set(fd_set *set, connection_info *server_info,
    connection_info clients[])
  {
    FD_ZERO(set);
    FD_SET(STDIN_FILENO, set);
    FD_SET(server_info->socket, set);

    int max_fd = server_info->socket;
    int i;
    for(i = 0; i < MAX_CLIENTS; i++)
    {
      if(clients[i].socket > 0)
      {
        FD_SET(clients[i].socket, set);
        if(clients[i].socket > max_fd)
        {
          max_fd = clients[i].socket;
        }
      }
    }
    return max_fd;
  }

  void handle_new_connection(connection_info *server_info, connection_info clients[])
  {
    int new_socket;
    int address_len;
    new_socket = accept(server_info->socket, (struct sockaddr*)&server_info->address, (socklen_t*)&address_len);

    if (new_socket < 0)
    {
      perror("Accept Failed");
      exit(1);
    }

    int i;
    for(i = 0; i < MAX_CLIENTS; i++)
    {
      if(clients[i].socket == 0) {
        clients[i].socket = new_socket;
        break;

    } else if (i == MAX_CLIENTS -1) // if we can accept no more clients
    {
      send_too_full_message(new_socket);
      printf("new socket is rejected because room is full\n");
      close(new_socket);
    }
  }
}
//fullllllllllfullllllllllfullllllllllfullllllllllfullllllllllfullllllllllfullllllllll
void send_too_full_message(int socket)
{
  SBCP_message too_full_message = initSBCP(NAK);
  pushAttr(&too_full_message,Reason,"Sorry, chat room is FULL!");
  if(send(socket, &too_full_message, sizeof(too_full_message), 0)<0)
  {
    perror("Send full message failed");
    exit(1);
  }
}
//duplicated name
void send_duplicated_name(int socket)
{
  SBCP_message duplicated_name_message = initSBCP(NAK);
  pushAttr(&duplicated_name_message,Reason, "Your name has already been used. Please try another name\n");
  if (send(socket, &duplicated_name_message, sizeof(duplicated_name_message), 0)<0)
  {
    perror("send duplicated_name message failed");
    exit(1);
  }
}


void handle_user_input(connection_info clients[])
{
  char input[255];
  fgets(input, sizeof(input), stdin);
  trim_newline(input);

  if(input[0] == 'q') {
    stop_server(clients);
  }
}

int main(int argc, char *argv[])
{
  puts("Starting server.");

  fd_set file_descriptors;

  connection_info server_info;

  int i;


  if (argc != 4)
  {
    fprintf(stderr, "Usage:./%s <server_ip> <server_port> <max_client> \n", argv[0]);
    exit(1);
  }
  MAX_CLIENTS=atoi(argv[3]);
  //check the max clients
  //printf("%d",MAX_CLIENTS);

  connection_info *clients=(connection_info*)malloc(MAX_CLIENTS*sizeof(connection_info));
  
  
  for(i = 0; i < MAX_CLIENTS; i++)
  {
    clients[i].socket = 0;
  }
  
  
  initialize_server(&server_info, atoi(argv[2]));

  while(true)
  {
    int max_fd = construct_fd_set(&file_descriptors, &server_info, clients);

    if(select(max_fd+1, &file_descriptors, NULL, NULL, NULL) < 0)
    {
      perror("Select Failed");
      stop_server(clients);
    }

    if(FD_ISSET(STDIN_FILENO, &file_descriptors))
    {
      handle_user_input(clients);
    }

    if(FD_ISSET(server_info.socket, &file_descriptors))
    {
      handle_new_connection(&server_info, clients);
    }

    for(i = 0; i < MAX_CLIENTS; i++)
    {
      if(clients[i].socket > 0 && FD_ISSET(clients[i].socket, &file_descriptors))
      {
        handle_client_message(clients, i);
      }
    }
  }

  return 0;
}
