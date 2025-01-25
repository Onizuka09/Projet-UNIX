#include "ui.h"
bool status_Auth_btn = false ; 
GooeyWindow Authwin;
GooeyWindow SerWin; 
GooeyWindow msgBox;
GooeyLayout *layout;
GooeyTextbox *passwdIn, *usernameIN;
GooeyLabel *userLable, *passwdLabel;

GooeyLabel *connectionStatusLabel;
GooeyLabel *clientIDValue;
GooeyTextbox *pathTextBox;
GooeyTextbox *fileNameTextBox;
GooeyLabel *setDateLabel;
GooeyLabel *setElapsedTimeLabel;
GooeyButton *dateButton;
GooeyButton *elapsedTimeButton;
GooeyButton *lsDirsButton;
GooeyButton *catFilesButton;

GooeyList *resultList;
GooeyButton *END_BTN;

/*
Callbacks and other stuff
*/

extern int ntrial ; 

bool state = 0;
void OnCloseAction();
void onAuthenticate();
// void onAddTask();
// void messageBoxCallback(int option);

// void onButtonClick();
void onDateAction();
void onElapsedTimeAction();
void onLsDirsAction();
void onCatFilesAction();

void Create_authentification_layout()
{
    GooeyLabel *titleLabel = GooeyLabel_Add(&Authwin, "Authentication Form: ", FONT_SIZE + 0.2f, 10, 20); // Adjusted to start from (20, 20)

    // Layout for Labels
    GooeyLayout *labelLayout = GooeyLayout_Create(&Authwin, LAYOUT_VERTICAL, 20, 100, 100, 80); // Increased y-position for more space
    // Add Labels
    userLable = GooeyLabel_Add(&Authwin, "Enter Username:", FONT_SIZE, 0, 0);
    passwdLabel = GooeyLabel_Add(&Authwin, "Enter Password:", FONT_SIZE, 0, 0);
    GooeyLayout_AddChild(labelLayout, userLable);
    GooeyLayout_AddChild(labelLayout, passwdLabel);
    GooeyLayout_Build(labelLayout);

    // Layout for Text Boxes
    GooeyLayout *textboxLayout = GooeyLayout_Create(&Authwin, LAYOUT_VERTICAL, 140, 100, 120, 80); // Increased y-position for more space
    // Add Text Boxes
    usernameIN = GooeyTextBox_Add(&Authwin, 0, 0, 120, 30, "", NULL); // Width matches layout width
    passwdIn = GooeyTextBox_Add(&Authwin, 0, 0, 120, 30, "", NULL);
    GooeyLayout_AddChild(textboxLayout, usernameIN);
    GooeyLayout_AddChild(textboxLayout, passwdIn);
    GooeyLayout_Build(textboxLayout);

    // Horizontal Layout to Combine Labels and Text Boxes
    GooeyLayout *combinedLayout = GooeyLayout_Create(&Authwin, LAYOUT_HORIZONTAL, 20, 100, 250, 100); // Increased y-position for more space
    GooeyLayout_AddChild(combinedLayout, labelLayout);                                            // Labels on the left
    GooeyLayout_AddChild(combinedLayout, textboxLayout);                                          // Text boxes on the right
    GooeyLayout_Build(combinedLayout);

    // Button at the Bottom
    GooeyButton *addButton = GooeyButton_Add(&Authwin, "Authenticate", 20, 180, 100, 30, onAuthenticate); // Adjusted y-position for more space

    // Vertical Layout to Wrap All Components
    GooeyLayout *mainLayout = GooeyLayout_Create(&Authwin, LAYOUT_VERTICAL, 20, 20, 300, 220); // Increased height for more space
    GooeyLayout_AddChild(mainLayout, combinedLayout);                                      // Add the combined layout
    GooeyLayout_AddChild(mainLayout, addButton);                                           // Add the button below
    GooeyLayout_Build(mainLayout);
}

void create_services_layout()
{
    // Title Layout (Horizontal)
    GooeyLayout *titleLayout = GooeyLayout_Create(&SerWin, LAYOUT_HORIZONTAL, 20, 20, 400, 50);
    GooeyLabel *titleLabel = GooeyLabel_Add(&SerWin, "Services", FONT_SIZE + 0.2f, 0, 0);
    GooeyLayout_AddChild(titleLayout, titleLabel);

    // Connection Status Label (Top-Right Corner)
    connectionStatusLabel = GooeyLabel_Add(&SerWin, "Not Connected", FONT_SIZE + 0.15f, 350, 20);
    GooeyLabel_SetColor(connectionStatusLabel, "0xFF0000");
    GooeyLayout_AddChild(titleLayout, connectionStatusLabel); // Add to title layout
    GooeyLayout_Build(titleLayout);

    // Client ID Layout (Horizontal)
    GooeyLayout *clientIDLayout = GooeyLayout_Create(&SerWin, LAYOUT_HORIZONTAL, 20, 60, 400, 50);
    GooeyLabel *clientIDLabel = GooeyLabel_Add(&SerWin, "Client ID:", FONT_SIZE + 0.15f, 0, 0);
    clientIDValue = GooeyLabel_Add(&SerWin, "-1", FONT_SIZE + 0.15f, 50, 0); // Placeholder for Client ID
    GooeyLayout_AddChild(clientIDLayout, clientIDLabel);
    GooeyLayout_AddChild(clientIDLayout, clientIDValue);
    GooeyLayout_Build(clientIDLayout);

    // Vertical Layout 1 (Labels)
    GooeyLayout *verticalLayout1 = GooeyLayout_Create(&SerWin, LAYOUT_VERTICAL, 20, 120, 100, 200);
    GooeyLabel *dateLabel = GooeyLabel_Add(&SerWin, "Date & Time Service:", FONT_SIZE, 0, 0);
    GooeyLabel *elapsedTimeLabel = GooeyLabel_Add(&SerWin, "Elapsed Time  Serivce:", FONT_SIZE, 0, 0);
    GooeyLabel *lsDirsLabel = GooeyLabel_Add(&SerWin, "Ls dirs Service:", FONT_SIZE, 0, 0);
    GooeyLabel *catFilesLabel = GooeyLabel_Add(&SerWin, "Cat files Service :", FONT_SIZE, 0, 0);
    GooeyLayout_AddChild(verticalLayout1, dateLabel);
    GooeyLayout_AddChild(verticalLayout1, elapsedTimeLabel);
    GooeyLayout_AddChild(verticalLayout1, lsDirsLabel);
    GooeyLayout_AddChild(verticalLayout1, catFilesLabel);
    GooeyLayout_Build(verticalLayout1);

    // Vertical Layout 2 (Outputs and Input Fields)
    GooeyLayout *verticalLayout2 = GooeyLayout_Create(&SerWin, LAYOUT_VERTICAL, 140, 120, 200, 200);
    setDateLabel = GooeyLabel_Add(&SerWin, "Empty", FONT_SIZE, 0, 0);        // Output for Date
    setElapsedTimeLabel = GooeyLabel_Add(&SerWin, "Empty", FONT_SIZE, 0, 0); // Output for Elapsed Time
    pathTextBox = GooeyTextBox_Add(&SerWin, 0, 0, 180, 25, "", NULL);        // Path input for Ls dirs
    fileNameTextBox = GooeyTextBox_Add(&SerWin, 0, 0, 180, 25, "", NULL);    // Filename input for Cat files
    GooeyLayout_AddChild(verticalLayout2, setDateLabel);
    GooeyLayout_AddChild(verticalLayout2, setElapsedTimeLabel);
    GooeyLayout_AddChild(verticalLayout2, pathTextBox);
    GooeyLayout_AddChild(verticalLayout2, fileNameTextBox);
    GooeyLayout_Build(verticalLayout2);

    // Vertical Layout 3 (Buttons)
    GooeyLayout *verticalLayout3 = GooeyLayout_Create(&SerWin, LAYOUT_VERTICAL, 360, 120, 100, 200);
    dateButton = GooeyButton_Add(&SerWin, "Get Date&Time Service", 0, 0, 50, 25, onDateAction);                  // Action for Date
    elapsedTimeButton = GooeyButton_Add(&SerWin, "Get Elapsed Time Service", 0, 0, 50, 25, onElapsedTimeAction); // Action for Elapsed Time
    lsDirsButton = GooeyButton_Add(&SerWin, "Get Ls Service", 0, 0, 50, 25, onLsDirsAction);                     // Action for Ls dirs
    catFilesButton = GooeyButton_Add(&SerWin, "Get Cat Service ", 0, 0, 50, 25, onCatFilesAction);               // Action for Cat files
    GooeyLayout_AddChild(verticalLayout3, dateButton);
    GooeyLayout_AddChild(verticalLayout3, elapsedTimeButton);
    GooeyLayout_AddChild(verticalLayout3, lsDirsButton);
    GooeyLayout_AddChild(verticalLayout3, catFilesButton);
    GooeyLayout_Build(verticalLayout3);

    // Main Horizontal Layout (Combining All Vertical Layouts)
    GooeyLayout *mainLayout = GooeyLayout_Create(&SerWin, LAYOUT_HORIZONTAL, 20, 100, 500, 300);
    GooeyLayout_AddChild(mainLayout, verticalLayout1); // Add Label Layout
    GooeyLayout_AddChild(mainLayout, verticalLayout2); // Add Outputs and Inputs Layout
    GooeyLayout_AddChild(mainLayout, verticalLayout3); // Add Buttons Layout
    GooeyLayout_Build(mainLayout);

    // Result List at the Bottom
    resultList = GooeyList_Add(&SerWin, 20, 410, 500, 400, NULL); // Large textbox for displaying results
    // GooeyTextBox_SetMultiline(resultList, true); // Enable multiline support for the result list
    GooeyList_ShowSeparator(resultList, false);
    END_BTN = GooeyButton_Add(&SerWin, "Close", 10, 840, 50, 30, OnCloseAction); // Action for Date
}

// void clear_window()
// {
//     memset(win.buttons, 0, sizeof(*win.buttons));
//     memset(win.labels, 0, sizeof(*win.labels));
//     memset(win.layouts, 0, sizeof(*win.layouts));
//     memset(win.textboxes, 0, sizeof(*win.textboxes));
    
//     win.label_count = 0;
//     win.layout_count = 0;
//     win.textboxes_count = 0;
//     win.layout_count = 0;
//     win.button_count = 0;
//     GooeyWindow_Redraw(&win);
// }

void OnCloseAction()
{
    printf("Closing window");
    exit(0);
}
void onAuthenticate()
{
    char* t = getUserName();
    char* p = getPasswd();
    printf("%s %s \n", t,p);
    status_Auth_btn = true  ; 
    
}
// void messageBoxCallback(int option)
// {
//     LOG_INFO("MessageBox option: %d", option);
// }

// void onButtonClick()
// {
//     state = !state;
//     GooeyWindow_MakeVisible(&msgBox, state);
// }
void onDateAction()
{
}
void onElapsedTimeAction()
{
}
void onLsDirsAction()
{
}
void onCatFilesAction()
{
}

void set_ConnectionStatus(bool st)
{
 GooeyLabel_SetColor(connectionStatusLabel, st ? "0x00FF00" : "0xFF0000");
    GooeyLabel_SetText(connectionStatusLabel, st ? "Connected To Server" : "NOT Connected To Server");
}

void setUserID(int ID)
{
char snum[12]; // Enough to store int max value as a string
    snprintf(snum, sizeof(snum), "%d", ID);
    printf("ID: %s\n", snum);
}

char *getUserName()
{
    char *userIN;
    userIN = GooeyTextbox_GetText(usernameIN);
    printf("USER IN %s\n ", userIN);
    return userIN;
}
char *getPasswd()
{
    char *passwd;
    passwd = GooeyTextbox_GetText(passwdIn);

    printf("PASS %s\n ", passwd);
    return passwd;
}
char *getFilename()
{
    char *tmp;
    tmp = GooeyTextbox_GetText(fileNameTextBox);

    printf("USER IN %s\n ", tmp);
    return tmp;
}
char *getPath()
{
    char *tmp;
    tmp = GooeyTextbox_GetText(pathTextBox);

    printf("USER IN %s\n ", tmp);
    return tmp;
}
void printOUputContent()
{
}
void setDateTime(char *date, char *time)
{
    char tmp[255];
    snprintf(tmp, sizeof(tmp) , "%s : %s", date, time);
    GooeyLabel_SetText(connectionStatusLabel, tmp);
}
void setElapsedTime(double etime)
{
    char tmp[255];
    snprintf(tmp, sizeof(tmp), "%f", etime );
    GooeyLabel_SetText(connectionStatusLabel, tmp);
}
