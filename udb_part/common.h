#ifndef COMMON_H_ 
#define COMMON_H_
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define NMAX 100

typedef struct { 
  int n ; 
  int arr[NMAX]; 
}message ;


#endif 
