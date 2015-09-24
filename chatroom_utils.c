/*
*  For ECEN 602 
*  Programming Assignment 1, Group: Zheng, Kan   Liu, Bowei
*  
*
*/


#include "chatroom_utils.h"

#include <string.h>

void trim_newline(char *text)
{
  int len = strlen(text) - 1;
  if (text[len] == '\n')
{
      text[len] = '\0';
  }
}

void clear_stdin_buffer()
{
  int c;
  while((c = getchar()) != '\n' && c != EOF)
    /* discard content*/ ;
}

SBCP_message initSBCP(header_type h_type){
  SBCP_message tbr;
  tbr.header = 0;
  tbr.attr[0] = 0;
  tbr.attr[1] = 0;
  tbr.header = htonl ( (3 << 23) + (h_type<<16) + sizeof(tbr));
  return tbr;
}

void pushAttr( SBCP_message* p_msg, attr_type a_type, char* content){
  int i;
  for (i = 0; i < 2; i++ ){
    if (p_msg->attr[i] == 0){
      // find a empty slot
      p_msg->attr[i] = htonl( (a_type << 16) +  (strlen(content)&0x00001111)  );
      memset(p_msg -> texts[i], '\0', sizeof(p_msg -> texts[i]));
      strcpy(p_msg -> texts[i], content);
      break;
    }
  }
  if (i == 2)
    printf("[ERROR] error in pushAttr\n");
}


header_type getHeaderType(SBCP_message sbcpmsg){
  unsigned int header_int  = ntohl(sbcpmsg.header);
  header_int = header_int >> 16;
  return (header_type) (header_int&0x007F); 
}

void getUsername(SBCP_message sbcpmsg, char* username){
  int i = 0;
  // printf("%08x",sbcpmsg.attr[0]);
  // printf("%08x",sbcpmsg.attr[1]);
  for (i = 0; i < 2; i ++){
    unsigned int attr = (unsigned int) ntohl(sbcpmsg.attr[i]);
    attr_type a_type = (attr_type)(attr >>16);
    if (a_type == Username){

      strcpy(username, sbcpmsg.texts[i]);
      return;
    }
  }
  puts("did not find username");
}

void getMessageContent(SBCP_message sbcpmsg, char* msg_content){
  int i = 0;
  for (i = 0; i < 2; i ++){
    unsigned int attr = (unsigned int) ntohl(sbcpmsg.attr[i]);
    attr_type a_type = (attr_type)(attr >>16);
    if (a_type == Message){
      strcpy(msg_content, sbcpmsg.texts[i]);
      return;
    }
  }
  puts("did not find  Message Content");
}

void getCorrespondingAttr(SBCP_message sbcpmsg, char* corresponding_text, attr_type target_type ){
  int i = 0;
  memset(corresponding_text, '\0', strlen(corresponding_text));
  for (i = 0; i < 2; i ++){
    unsigned int attr = (unsigned int) ntohl(sbcpmsg.attr[i]);
    attr_type a_type = (attr_type)(attr >>16);
    if (a_type == target_type){
      strcpy(corresponding_text, sbcpmsg.texts[i]);
      return;
    }
  }
  puts("did not find corresponding_text");

}

