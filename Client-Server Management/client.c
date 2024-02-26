 #include <stdio.h> // Standard Input and Output operations
#include <stdlib.h>  //Standard Library   
#include <string.h>  // For String manipulations
#include <sys/socket.h>  // Socket-related functions and structures for networking
#include <arpa/inet.h>  //For address resolution protocol over Internet
#include <unistd.h>   //Access to POSIX operating system API  
#include <sys/types.h>   //Provides various data types used by system calls 

int server_p =8080;               // Port number for main server
int mirror_p= 8082;          // Port number for mirror server


// Function to check if a give string contains the date format and return 1 if the date is valid
int dateValidation(char* inputDate) 
{
    int Year, Month, Day;  // Year, month, day components
    if (sscanf(inputDate, "%d-%d-%d", &Year, &Month, &Day) != 3) //To check the date format in YYYY-MM-DD
    {
        return 0;
    }
    if (Year < 1 || Year > 9999 || Month < 1 || Month > 12 || Day < 1 || Day > 31) //Check the value of each components is valid
    {
        return 0;//outside range
    }
    return 1;//inside range
}



// Function to check whether a file list contains atleast one file and if there is no file it returns 0 , or returns 1
int filelistValidation(char* filelist)  
{
    if (strlen(filelist) == 0)  // Checking there is at least one file in the file list
    {
        return 0;
    }
    return 1;
}


// Function to check file extensions and validates by checking if there is one file extension provided
int extensionValidation(char* extension) 
{
    if (strlen(extension) == 0) // Checking at least one extension is present in the extensions list
    {
        return 0; //If extension is empty
    }
    return 1; //Extension not empty
}


//Main Function
int main(int argc, char const *argv[]) {

   //Intilizing necessay variables
    int socketFileDescriptor; //Variable holding the file descriptor for the server socket
    struct sockaddr_in serverAddress, mirrorAddress; //Structure holds information about the server address, mirror address including the address family, IP address, and port number
    char buffer[1024] = {0}; //to store the input entered by the user.
    char command[1024] = {0}; //to store the constructed command that will be sent to the server
    char arr_store[2048]={0}; //to store the input from user and clears the array
    int syntaxValidStatus; //to track whether the syntax of the user-entered command is valid

    //SOcket()
    if ((socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) { // Create socket file descriptor
      //  printf("\n Error creating Socket\n");
        perror("\n Error creating Socket\n"); //error
        return -1;
    }

    memset(&serverAddress, '0', sizeof(serverAddress)); //Ensures that there are no unintended values in the structure.
    serverAddress.sin_family = AF_INET; //To set the address family of the address structure
    serverAddress.sin_port = htons(server_p);  //To set the port number of the server

    // Converting addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0) //Converts the IP address from text to binary form and stores it
    {
       // printf("\n Invalid Address / Address  --- not supported \n");
        perror("Invalid Address");
        return -1;
    }

    if (connect(socketFileDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) //o establish a connection to the server using the created socket
    {
       // printf("\nConnection -- Failed \n");
        perror("Connection Failed");
        return -1;
    }

    printf("Connected  to Server.\n");
    printf("Enter command or 'quitc' to exit:\n");

    
    while (1) //Endless Loop
    {
        syntaxValidStatus = 1; //If syntax is valid

      //  *memset copies the character c (an unsigned char) to the first n characters of the string pointed to, by the argument str.
        memset(buffer, 0, sizeof(buffer)); // Clear buffer
        memset(command, 0, sizeof(command)); // Clear buffer
        fgets(buffer, sizeof(buffer), stdin); // Read user input from stdin
        memcpy(arr_store, buffer, sizeof(buffer)); //copy data to other array
        buffer[strcspn(buffer, "\n")] = 0; // To remove newline character

        // Parse command
        char* token = strtok(buffer, " "); // To tokenize a string based on a delimiter
        if (token == NULL) { //Check if the token value is null
            syntaxValidStatus = 0; //Status 0
        } 
        else if (strcmp(token, "getfn") == 0) // Function with "getfn" command
        { 
            char* fname = strtok(NULL, " ");//It means there was no second token 
            if (fname == NULL) //If null
            {
                syntaxValidStatus = 0;//Status value 0
           
                } 
                else
                 {
            sprintf(command, "getfn %s", fname); //The command string will be "getfn" followed by the filename.
        }
    } 
    else if (strcmp(token, "getfz") == 0) //Function with getfz command
    {  
        char* size1 = strtok(NULL, " "); //to extract size1 parameters from the received command
        char* size2 = strtok(NULL, " "); //to extract size2 parameters from the received command
        if (size1 == NULL || size2 == NULL || atoi(size1) < 0 || atoi(size2) < 0) //Checks if both size parameters are provided
        {
            syntaxValidStatus = 0;
             perror("Enter Valid"); //error
        } 
        else 
        {
                sprintf(command, "getfz %s %s", size1, size2); //The command string will be "getfz" followed by the two sizes.
                
            
        }
    } 
    else if (strcmp(token, "getfdb") == 0) // Function with getfdb command
    {  
    char* date_str1 = strtok(NULL, " "); //get the date str from the command
    if (date_str1 == NULL || !dateValidation(date_str1)) //check if the date value is null or invalid
    {
        syntaxValidStatus = 0; //Status 0
        perror("Enter a valid date.\n");//error
    } 
    else 
    {
        sprintf(command, "getfdb %s", date_str1); //The command string will be "getdb" followed by a date.
    }
}

else if (strcmp(token, "getfda") == 0) {  // Function with getfda command
    char* date_str2 = strtok(NULL, " ");  //get the date str from the command
    if (date_str2 == NULL || !dateValidation(date_str2))  //check if the date value is null or invalid
     {
        syntaxValidStatus = 0; //Status 0
        perror("Enter a valid date.\n"); //error
    } 
    else 
    {
        sprintf(command, "getfda %s", date_str2); //The command string will be "getdb" followed by a date.
    }
}

    
    else if (strcmp(token, "getft") == 0)  // Function with getft command
    {
    char* extension1 = strtok(NULL, " "); //get the extension1 str from the command
    char* extension2 = strtok(NULL, " ");  //get the extension2 str from the command
    char* extension3 = strtok(NULL, " ");  //get the extension3 str from the command
   // char* npsd_exten4 = strtok(NULL, " ");
    if(extension1 == NULL && extension2 ==NULL) //If no extensions given as input
    {
        printf("Enter extension");//error
    }
    //Function -- with getft command
        arr_store[strcspn(arr_store, "\n")]='\0'; //returns the length of the initial segment of arr_store
        sprintf(command, arr_store); //to format and store a string in the command variable
        printf("%s", command); //print command
    } 
    else if (strcmp(token, "quitc") == 0) // Function with quitc command
    {
        sprintf(command, "quitc"); //print quitc
    } 
    else 
    {
        syntaxValidStatus = 0;//Status 0
    }

    if (!syntaxValidStatus) //Check is the syntax valid
    { 
        printf("Invalid syntax\n"); //error
        continue;
    }

    // Send command to server
    send(socketFileDescriptor, command, strlen(command), 0);

    // Handle response from server
    char response[1024]={0}; //array to store response 
    int readValue=read(socketFileDescriptor, response, sizeof(response)); //To read data from the socket associated with socketFileDescriptor
    printf("%s\n", response); //print response
    response[strcspn(response, "\n")] = '\0';// Removes the newline character from the response array,
    if (strcmp(response, "8082") == 0) //if response equals mirror port
    {
        close(socketFileDescriptor);   // closing the current server connection
        printf("Mirror\n");
        socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0); // Creating new socket for the mirror server
        if (socketFileDescriptor == -1) //check the value of the socket file descriptor
        { 
            perror("socket"); //error
            exit(EXIT_FAILURE);
        }

        memset(&mirrorAddress, '\0', sizeof(mirrorAddress)); //Initializing the mirrorAddress structure by setting all its bytes to zero.
        mirrorAddress.sin_family = AF_INET; //Setting the address family of the mirrorAddress structure
        mirrorAddress.sin_port = htons(mirror_p); //setting the port of the mirrorAddress structure.
        mirrorAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); //Setting the IP address of the mirrorAddress structure to "127.0.0.1"

    if (connect(socketFileDescriptor, (struct sockaddr *)&mirrorAddress, sizeof(mirrorAddress)) == -1) // Connect to the mirror server
    {
            perror("connect");//error
            exit(EXIT_FAILURE);
    }
     
    }
    else 
    printf("%s\n", response); //prints response
    if (strcmp(command, "quitc") == 0) //Checks if the command is "quitc". If it is, the loop is exited,end the program.
    {
        break;
    }

    printf("Enter a command 'quitc' to exit \n");
}

close(socketFileDescriptor); //Close connection
printf("Connection is closed.\n");

return 0;//Success
}
