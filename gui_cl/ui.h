#ifndef UI_H 
#define UI_H 

#include <Gooey/gooey.h>

// auth layout 



#define FONT_SIZE 0.25f


void clear_window();




void create_services_layout();
void setUserID(int ID);
void Create_authentification_layout();
void set_ConnectionStatus(bool st);
char* getUserName();
char* getPasswd();
char* getFilename(); 
char* getPath(); 
void printOUputContent(); 
void setDateTime(char* date,char* time); 
void setElapsedTime(double etime); 

#endif 