/*
*  For ECEN 602 
*  Programming Assignment 1, Group: Zheng, Kan   Liu, Bowei
*  
*
*/


#ifndef CHATROOM_UTILS_H_
#define CHATROOM_UTILS_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

//Enum of different messages possible.
typedef enum
{
  CONNECT,
  DISCONNECT,
  GET_USERS,
  SET_USERNAME,
  PUBLIC_MESSAGE,
  PRIVATE_MESSAGE,
  TOO_FULL,
  USERNAME_ERROR,
  SUCCESS,
  ERROR

} message_type;

// added due to programming assignment 1
typedef enum
{
  JOIN = 2,
  FWD,       //3
  SEND,      //4
  NAK,       //5
  OFFLINE,   //6
  ACK,       //7
  ONLINE,    //8
  IDLE,      //9
} header_type;

typedef enum {
  Reason = 1,
  Username,
  ClientCount,
  Message         //attribute type 
} attr_type;

//message structure
// typedef struct
// {
//   message_type type;
//   char username[21];
//   char data[256];

// } message;

//structure to hold client connection information
typedef struct connection_info
{
  int socket;
  struct sockaddr_in address;
  char username[20];
} connection_info;



typedef struct {
  short vrsn;
  short type;
  short length;
} SBCP_header;

typedef struct {
  short type;
  short length;
  char text[256]; 
} SBCP_attr;

typedef struct {
  unsigned int header;
  unsigned int attr[2];
  char texts[2][256]; 
} SBCP_message;


// Removes the trailing newline character from a string.
void trim_newline(char *text);

// discard any remaining data on stdin buffer.
void clear_stdin_buffer();

header_type getHeaderType(SBCP_message sbcpmsg);

void getMessageContent(SBCP_message sbcpmsg, char* msg_content);
void getUsername(SBCP_message sbcpmsg, char* username);

SBCP_message initSBCP(header_type h_type);
void pushAttr( SBCP_message* p_msg, attr_type a_type, char* content);
void send_too_full_message(int socket);
void send_duplicated_name(int socket);
void getCorrespondingAttr(SBCP_message sbcpmsg, char* corresponding_text, attr_type target_type );
#endif
