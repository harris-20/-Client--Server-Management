
#include <stdio.h>          // Standard Input and Output operations
#include <sys/socket.h>     // Socket-related functions and structures for networking
#include <arpa/inet.h>      //For address resolution protocol over Internet
#include <unistd.h>         //Access to POSIX operating system API      
#include <sys/wait.h>       //Functions and constants for handling child processes      
#include <errno.h>          //Defines error numbers and related macros      
#include <sys/stat.h>       //Provides functions for obtaining information about files
#include <stdlib.h>         //Standard Library      
#include <string.h>         // For String manipulations
#include <time.h>           //Provides functions for working with date and time      
#include <dirent.h>         //Defines structures and functions for directory manipulation 
#include <sys/types.h>      //Provides various data types used by system calls   


int mirror_p= 8082;  //Port number for mirror
int buffer= 1024;    // The size of buffer


// Function to search for a file in directories

void search_dir(char* destination, char* file_name, char* command) 
{
    DIR* dir;                      // Structure for managing directoy
    struct dirent* entry;          // Structure for directory entry
    struct stat st;                // Structure for file status 

    if ((dir = opendir(destination)) == NULL) {  // Statement attempts to open a specific directory 
       // printf("Errot to open the directory %s\n", destination);
        perror("Unable to open directory");//Prints error if unsuccessful
        return;
    }
    while ((entry = readdir(dir)) != NULL)  //Statement used to traverse with each directories using readdir 
    {  
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) // Skip current and parent directories entry
        {
            continue;
        }
        char completePath[1024];   
        snprintf(completePath, sizeof(completePath), "%s/%s", destination, entry->d_name); // Constructs a complete path by combining a current destination file or directory paths
        if (lstat(completePath, &st) == 0) // Use lstat to retrive information about the file and checks if it is successful or not
        {
            if (S_ISDIR(st.st_mode)) //If entry is a directory, a recursive function is called in the sub directories
            {
                search_dir(completePath, file_name, command);  // Recurse into the subdirectory
            } 
            else if (S_ISREG(st.st_mode)) //If file is found with the desired filename, it will collect info like creation time, size, file premissions
            {
               if (strcmp(entry->d_name, file_name) == 0) // Checking whether the file has the desired name
               {
    time_t c_time = st.st_ctime;   //File creation time
    mode_t permissions = st.st_mode; //File permissions
    struct tm *tm_info = localtime(&c_time); // Convert time information to local time structure

    char perm_str[11]; //Store file info in a command buffer
    snprintf(perm_str, sizeof(perm_str), "%c%c%c%c%c%c%c%c%c",
             (permissions & S_IRUSR) ? 'r' : '-', // User has read permission
             (permissions & S_IWUSR) ? 'w' : '-', // User has write permission
             (permissions & S_IXUSR) ? 'x' : '-', //User has execute permission 
             (permissions & S_IRGRP) ? 'r' : '-', //Group has read permission
             (permissions & S_IWGRP) ? 'w' : '-', // Group has write permission
             (permissions & S_IXGRP) ? 'x' : '-', //Group has execute permission 
             (permissions & S_IROTH) ? 'r' : '-', //Others has read permission
             (permissions & S_IWOTH) ? 'w' : '-', // Others has write permission
             (permissions & S_IXOTH) ? 'x' : '-'); //Others has execute permission 

    strftime(command, sizeof(command), "%a %d %b %Y %I:%M:%S %p %Z\n", tm_info); //Collects information
    sprintf(command + strlen(command), ", \n%ld, %s, %s, %s\n", st.st_size, entry->d_name, completePath, perm_str); //Appends Information

    closedir(dir); //Closing directory
    return;
     }
   }
  } 
  else 
   {
            printf("No Information provided regarding file %s\n", completePath); //Print No information provided
   }
  }

    closedir(dir);  // Closing directory
}

// Function receives commands form clients ,process it and sends them back.It contains different commands such as getfn, getfz, getfdb, getfda, getft and quitc. 
void processclient(int socketFileDescriptor) 
{
    char recvBuffer[1024] = {0};         // Receive buffer
    char command[1024] = {0};      // Command buffer
    char tempBuffer[1024] = {0};           // Temporary buffer

    while (1) //Infinite loop to read commands fro clients
    {
        memset(recvBuffer, 0, sizeof(recvBuffer)); //Initializes buffer to 0
        memset(command, 0, sizeof(command)); //Initializes buffer to 0
        memcpy(tempBuffer, recvBuffer, sizeof(recvBuffer));//To create a copy of the received data in another buffer

        int receiveValue = read(socketFileDescriptor, recvBuffer, sizeof(recvBuffer)); //Read data from the client socket
        recvBuffer[receiveValue] = '\0';//To ensure that the received data is null-terminated

        char* Token = strtok(recvBuffer, " ");   // Parsing command
        if (Token == NULL) 
        {
            sprintf(command, "Invalid Syntax\n"); //Print error if command is empty or invalid
        } 
       
else if (strcmp(Token, "getfn") == 0)  // Execution for "getfn" command
{
    char* file_name = strtok(NULL, " "); //To extract the file name from the received command.
    if (file_name == NULL) //Checks if a file name is provided
    {
        sprintf(command, "Invalid Syntax\n");//print error
    }
    else
    {
        char primaryPath[1024];
        const char* home_dir = getenv("HOME");  // Get the home directory path

        if (home_dir == NULL) { //If the vlaue is null
            //sprintf(command, "Error finding home directory\n");
            perror("Error finding home directory");//print error
            return;
        }

        sprintf(primaryPath, "%s", home_dir);  // Construct the full path by appending the home directory path

        char command_buf[buffer];
        sprintf(command_buf, "find %s -maxdepth 3 -name %s -exec stat --printf='%%n,%%s,%%y,%%A\n' {} +",
                primaryPath, file_name); // Build the command to find the file, including file information using 'stat'
        //sprintf(command_buf, "find %s -maxdepth 3 -name %s -exec stat --printf='%%n,%%s,%%A\n' {} +",
            //    primaryPath, file_name);

        FILE* fp = popen(command_buf, "r"); // Open a pipe to run the command and read its output
        char line[buffer];
        if (fgets(line, buffer, fp) != NULL) //Checks if the buffer is empty or not
        {
            char* filename = strrchr(line, '/'); // Read the first line from the command output
            if (filename != NULL) //Checks if the file name is not null
            {
                sprintf(command, "%s", filename + 1); //prints the file name
            } else {
                sprintf(command, "%s", line); //error
            }
        }
        else
        {
            //sprintf(command, "Unable to find the file\n");
            perror("Error Finding the file"); //error
        }
        pclose(fp); //Closing pipe
    }
}
    else if (strcmp(Token, "getfz") == 0) // Execution for getfz command
    {
    char* strSize1 = strtok(NULL, " "); //to extract size1 parameters from the received command
    char* strSize2 = strtok(NULL, " "); //to extract size2 parameters from the received command

    if (strSize1 == NULL || strSize2 == NULL) //Checks if both size parameters are provided
    {
        //sprintf(command, "Entered syntax is not valid. Please enter size1 and size2.\n");
        perror("Invalid Input.Enter sizes again "); //error
    } 
    else 
    {
        int size1 = atoi(strSize1); // Convert string size1 to integer
        int size2 = atoi(strSize2); // Convert string size2 to integer

        if (size1 < 0 || size2 < 0 || size1 > size2)  // Check if the provided size range is valid
        { 
            //sprintf(command, "Enter valid size range.\n");
            perror("Enter Valid Size range"); //error
        } 
        else 
        {
            char primaryPath[1024]; // Specify the primary directory path
            sprintf(primaryPath, "/home/");

            char command_buf[buffer]; // Finding files matching the size range
            sprintf(command_buf, "find %s -type f -size +%ldc -size -%ldc -print0 | xargs -0 tar -czf /home/raj/Documents/MAC/ASPProject/f23project/temp.tar.gz", primaryPath, size1, size2); // Build the command to find files within the specified size range

            int status = system(command_buf); // Execute the command using the system call

            if (status == 0) // Check the status of the command execution
            {
                sprintf(command, "Files matching the size range have been stored in temp.tar.gz\n"); //if found and stored
            } 
            else 
            {
                sprintf(command, "No files found in the specified size range.\n"); //if not found
            }
        }
    }
}
        // Execution for getfdb commanf
  else if (strcmp(Token, "getfdb") == 0)
   {
    char *strDate = strtok(NULL, " "); //get the date str from the command

    if (strDate == NULL) //check if the date value is null
    {
       // sprintf(command, "Entered syntax is not valid. Please Enter again.\n");
        perror("Invalid input Syntax"); //error
    }
    else
    {
        char primaryPath[1024]; // Specify the primary directory path
        sprintf(primaryPath, "/home/");

        char command_buf[buffer]; // Finding files that match the date range
        sprintf(command_buf, "find %s -type f ! -newermt \"%s\" -print0 | xargs -0 tar -czf /home/raj/Documents/MAC/ASPProject/f23project/temp.tar.gz",primaryPath, strDate); //To find files that are not newer than the specified date and create a tar file.
        system(command_buf); // Execute the command using the system call

        sprintf(command, "Files are retrieved successfully.\n");//Success message
    }
}

// Execution for getfa commanf
else if (strcmp(Token, "getfda") == 0)
{
    char *strDate = strtok(NULL, " "); //get the date str from the command

    if (strDate == NULL) //check if the date value is null
    {
       // sprintf(command, "Entered syntax is not valid. Please Enter again.\n");
        perror("Invalid input Syntax"); //error
    }
    else
    {
        char primaryPath[1024]; // Specify the primary directory path
        sprintf(primaryPath, "/home/");

        char command_buf[buffer]; // Finding files that match the date range
        sprintf(command_buf, "find %s -type f -newermt \"%s\" -print0 | xargs -0 tar -czf /home/raj/Documents/MAC/ASPProject/f23project/temp.tar.gz",primaryPath, strDate); //To find files that are not newer than the specified date and create a tar file.
        system(command_buf); // Execute the command using the system call

        sprintf(command, "Files are retrieved successfully.\n"); //Success Message
    }
}


         // Execution for getft command
  else if (strcmp(Token, "getft") == 0)
   {
    char *extension1 = strtok(NULL, " "); // Get the first extension from the command
    char *extension2 = strtok(NULL, " ");  // Get the second extension from the command
    char *extension3 = strtok(NULL, " ");  // Get the third extension from the command

    char primaryPath[1024]; // Specify the primary directory path
    sprintf(primaryPath, "/home/");

    char command_buf[buffer]; // Checking whether any of the specified files are present
    sprintf(command_buf, "find %s -type f \\( ", primaryPath);
    if (extension1 != NULL) // Append conditions to the command for each provided extension 1
        sprintf(command_buf + strlen(command_buf), "-iname \"*.%s\" -o ", extension1);
    if (extension2 != NULL) // Append conditions to the command for each provided extension 2
        sprintf(command_buf + strlen(command_buf), "-iname \"*.%s\" -o ", extension2);
    if (extension3 != NULL) // Append conditions to the command for each provided extension 3
        sprintf(command_buf + strlen(command_buf), "-iname \"*.%s\" -o ", extension3);
        
    sprintf(command_buf + strlen(command_buf), "-false \\) -print0 | xargs -0 tar -czf /home/raj/Documents/MAC/ASPProject/f23project/temp.tar.gz"); //To build a command that, when executed, creates an empty tar file 

    int status_getft = system(command_buf); // Execute the command using the system call

    if (status_getft == 0) // Check if the files were found
    {
        sprintf(command, "Files are retrieved successfully.\n"); //If found
    }
    else
    {
        sprintf(command, "No file found.\n"); //If not found
    }
}
        //quitc command Execution
        else if (strcmp(Token, "quitc") == 0) 
        {
            break; //End the program
        } 
        else 
        {
           // sprintf(command, "Entered syntax is not valid. Please Enter again.\n");
            perror("Invalid Input Syntax"); //error
        }
        send(socketFileDescriptor, command, strlen(command), 0); // Sending response to client
    }

    close(socketFileDescriptor); //Closing the socket file descriptor
    exit(0);
}


int main(int argc, char const* argv[]) 
{
    int serverFileDescriptor, newSocket; //Variables holds the file descriptor for the server socket and for a newly accepted client socket
    struct sockaddr_in address; //Structure holds information about the server address, including the address family, IP address, and port number
    int opt = 1; //To set socket options
    int addrlen = sizeof(address); //To store the size of the address structure.
  
    if ((serverFileDescriptor = socket(AF_INET, SOCK_STREAM, 0)) == 0)  // Create socket file descriptor
    {
       // Error if socket fails
        perror("Socket failed"); //error
        exit(EXIT_FAILURE);
    }

    if (setsockopt(serverFileDescriptor, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) // Set socket options
    {
        perror("setsockopt");//error
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET; //To set the address family of the address structure
    address.sin_addr.s_addr = INADDR_ANY; //To set the IP address of the server
    address.sin_port = htons(mirror_p); //To set the port number of the server

    if (bind(serverFileDescriptor, (struct sockaddr*)&address, sizeof(address)) < 0) //Binds the socket to the specified port using bind()
    {
        perror("Bind"); //error
        exit(EXIT_FAILURE);
    }
    if (listen(serverFileDescriptor, 3) < 0) //Starts listening for incoming connections using listen()
    {
        perror("Listen"); //error
        exit(EXIT_FAILURE);
    }
    while (1)  //Endless Loop 
    {
        if ((newSocket = accept(serverFileDescriptor, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) //Accepts an incoming connection using accept()
        {
            perror("Accept"); //error
            exit(EXIT_FAILURE);
        }

        printf("New client connected --->. Forking child process\n"); //print client connection and forking success
       // if (activeClients < 6 || (activeClients > 10 && activeClients % 2)) // Load balancing from server to mirror
       printf("Mirror Server");
       // if (activeClients <= 3) // Load balancing from server to mirror
       // { 
            int prcsid = fork(); // Fork a new process
            if (prcsid == -1) //error
            {
                perror("Fork Error"); //print error
                exit(EXIT_FAILURE);
            }
             else if (prcsid == 0) // Child process
            {  
                close(serverFileDescriptor); //Closing server file descriptor
                processclient(newSocket); // Handling the client in the child process
            } 
            else // Parent process
            {  
                close(newSocket); //Closing the client file descriptor
                while (waitpid(-1, NULL, WNOHANG) > 0); // Clean up zombie processes
            }
       // } 
       
    }

    return 0;// success
}
