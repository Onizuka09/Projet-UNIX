#ifndef SERVICE_H 
#define SERVICE_H
// authenticate 
#include <stdbool.h>
#include <stddef.h>
#define PORT_LIST_DIR_SERVICE 5081
#define PORT_ELAPSED_TIME_SERVICE 5082
#define PORT_CAT_CONTENT_SERVICE 5083
#define PORT_TIME_DATE_SERVICE 5084
 

bool Authenticate(const char* username, const char* psswd); 
void ListDirectories(const char *path, char *buff, size_t buff_size);
void getDateTime(char* _time, size_t time_size, char* _date, size_t date_size) ;
void getFileContent(char* filename, char* buff,size_t buff_size); 
void getElapsedTime( bool fcapture ,  double* elapsed ); // date & heure 




#endif 
