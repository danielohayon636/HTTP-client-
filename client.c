
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#define MAXBUFFSIZE 20

int findUrlIndex(int argc, char **argv)
{
    int index = -1;
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == 'h' && argv[i][1] == 't' && argv[i][2] == 't' && argv[i][3] == 'p' && argv[i][4] == ':' && argv[i][5] == '/')
        {
            if (strcmp("-p", argv[i - 1]) != 0) //url is not part of argument
            {
                index = i;
                break;
            }
        }
    }
    return index;
}
/* search for "-r" in argv */
int Rfinder(int argc, char **argv)
{
    int index = -1;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp("-r", argv[i]) == 0 && atoi(argv[i + 1]) != 0)
        {
            index = i;
            //valid index was found
        }
        else if (strcmp("-r", argv[i]) == 0 && atoi(argv[i + 1]) == 0)
        {
            fprintf(stderr, "Usage: client [-p <text>] [-r n < pr1=value1 pr2=value2 …>] <URL>\n");
            exit(0);
            //the argument next to "-r" is not a number
        }
        else if (strcmp("-r", argv[i]) == 0 && strcmp("-p", argv[i - 1]) == 0)
        {
            //"-r" is the text for -p
        }
    }
    return index;
}
/* search for "-p" in argv */
int Pfinder(int argc, char **argv)
{
    int index = -1;
    for (int i = 1; i < argc - 1; i++)
    {
        if (strcmp("-p", argv[i]) == 0 && strcmp("-p", argv[i - 1]) != 0) //make sure -p is not text for another -p
        {
            index = i;
        }
    }
    return index;
}

bool checkValidArgument(char *arg) //check if a string is of "x=y" form
{
    bool haveEquals = false;
    for (int i = 0; i < strlen(arg); i++)
    {
        if (arg[i] == '=' && !haveEquals)
        {
            haveEquals = true;
            if (i == strlen(arg) - 1 || i == 0) //make sure its not a "x=" form or "=y" form
            {
                haveEquals = false;
            }
        }
        if (arg[i] == ' ') //illegal char
        {
            haveEquals = false;
            break;
        }
    }
    return haveEquals;
}

int main(int argc, char *argv[])
{

    int url_argv_index = -1;                //argument index for the url
    int p_argv_index = -1;                  //argument index for the "-p"
    int r_argv_index = -1;                  //argument index for the "-r"
    int p_text_argv_index = -1;             //argument index for the text after "-p"
    int r_num_of_arguments_argv_index = -1; //argument index for the number after "-r"
    int r_num_of_arguments = -1;            //number of arguments after "-r"
    int p_text_length = 0;                  //lenght of text after "-p"
    int port_index = -1;                    //index of the port inside the url (if necceary)
    int port = 80;                          //defult value
    int port_lenght = 0;                    //length of the port number
    char *cmd = NULL;                       //will store HOST/GET for the request
    url_argv_index = findUrlIndex(argc, argv);
    r_argv_index = Rfinder(argc, argv);
    p_argv_index = Pfinder(argc, argv);
    int args_total_lenght = 0; //total length of all the "-r" arguments
    int unUsed[argc];
    /* this array will remember if we used each elenment in the argv. if not, there are garbage values */
    for (int i = 0; i < argc; i++)
    {
        unUsed[i] = 0; //init all values to 0 (not used)
    }
    unUsed[0] = 1;
    unUsed[url_argv_index] = 1;
    if (r_argv_index > 0) //if theres -r in args
    {
        unUsed[r_argv_index] = 1;
        unUsed[r_argv_index + 1] = 1;
    }
    if (p_argv_index > 0) //if there in -p in args
    {
        unUsed[p_argv_index] = 1;
        unUsed[p_argv_index + 1] = 1;
    }
    if (r_argv_index > 0) //ther is -r in the args
    {
        cmd = "GET ";
        r_num_of_arguments_argv_index = r_argv_index + 1;
        r_num_of_arguments = atoi(argv[r_num_of_arguments_argv_index]); //int value of the number of values
        int g = 0;
        for (int j = 0; j < r_num_of_arguments; j++)
        {
            if(r_num_of_arguments_argv_index + 1 + j >= argc){
                fprintf(stderr, "Usage: client [-p <text>] [-r n < pr1=value1 pr2=value2 …>] <URL>\n");
                exit(0);
            }
            if (!checkValidArgument(argv[r_num_of_arguments_argv_index + 1 + j]))
            { //check validation to all arguments after -r
                fprintf(stderr, "Usage: client [-p <text>] [-r n < pr1=value1 pr2=value2 …>] <URL>\n");
                exit(0);
            }
            else
            {
                args_total_lenght += strlen(argv[r_num_of_arguments_argv_index + 1 + j]); //arg is valid
                unUsed[r_num_of_arguments_argv_index + 1 + j] = 1;
            }
            g = j;
        }
        if (r_num_of_arguments_argv_index + 1 + g + 1 < argc)
            if (checkValidArgument(argv[r_num_of_arguments_argv_index + 1 + g + 1]))
            { //make sure there are not too many arguments
                fprintf(stderr, "Usage: client [-p <text>] [-r n < pr1=value1 pr2=value2 …>] <URL>\n");
                exit(0);
            }
    }
    char *p_text; //will store the text after -p
    if (p_argv_index > 0)
    {
        cmd = "POST ";
        p_text_argv_index = p_argv_index + 1;
        p_text_length = strlen(argv[p_text_argv_index]);
        p_text = (char *)malloc(sizeof(char) * (p_text_length + 10));
        if (p_text == NULL)
        { //memory allocation check
            perror("allocation failed\n");
        }
        int i = 0;
        for (i = 0; i < p_text_length; i++)
        {
            p_text[i] = argv[p_text_argv_index][i]; //copy text char by char
        }
        p_text[i + 1] = '\0';
    }
    if (p_argv_index < 0 && r_argv_index < 0)
    { //no -r no -p so cmd will be GET
        cmd = "GET ";
    }

    int url_length;
    char *url; //will store the url
    if (url_argv_index > 0)
    {
        url_length = strlen(argv[url_argv_index]);
        url = (char *)malloc(1 + url_length);
        if (url == NULL)
        { //memory allocation check
            perror("allocation failed\n");
        }
        int q;
        for (q = 0; q < url_length; q++)
        { //copy chay by char
            url[q] = argv[url_argv_index][q];
        }
        url[q] = '\0';
    }
    else
    {
        fprintf(stderr, "Usage: client [-p <text>] [-r n < pr1=value1 pr2=value2 …>] <URL>\n");
        exit(0);
    }
    int path_index = -1;
    int www_start_index_url = 7; //after "http://"
    int adderess_lenght = 0;

    for (int i = www_start_index_url; i < url_length; i++)
    { //get port from url
        if (url[i] == '/' || url[i] == '\0' || url[i] == ':')
        {
            if (url[i] == '/')
            {
                path_index = i + 1;
            }
            if (url[i] == ':') //port starts
            {
                port_index = i + 1;
                while (url[i] != '/' && url[i] != '\0')
                {
                    i++;
                    port_lenght++;
                }
                if (url[i] == '/') //path starts
                {
                    path_index = i + 1;
                }
            }
            break;
        }
        else
        {
            adderess_lenght++;
        }
    }
    char *port_str = NULL;
    if (port_index > 0)
    { //port exsist in url
        port_str = (char *)malloc(port_lenght);
        if (port_str == NULL)
        { //memory allocation check
            perror("allocation failed\n");
        }
        //copy port into new string
        for (int i = port_index; i < port_index + port_lenght; i++)
        {
            port_str[i - port_index] = url[i];
        }
        port = atoi(port_str);
    }

    char *host = (char *)malloc(sizeof(char) * (adderess_lenght + 1));
    if (host == NULL)
    { //memory allocation check
        perror("allocation failed\n");
    }
    int counter = 0;
    for (int i = www_start_index_url; i < www_start_index_url + adderess_lenght; i++)
    { //copy only the host address
        host[counter] = url[i];
        counter++;
    }
    host[counter] = '\0';

    char *http = "HTTP/1.0"; // protocol
    char *host1 = "Host: ";  //header
    char *path;              // will store the path only
    int path_length = 0;
    if (path_index > 0)
    {
        path_length = url_length - path_index;
        path = (char *)malloc(sizeof(char) * (path_length + 1));
        if (path == NULL)
        { //memory allocation check
            perror("allocation failed\n");
        }
        int k = 0;
        for (k = path_index; k < url_length; k++)
        {
            path[k - path_index] = url[k]; //copy rhe path char by char
        }
        path[path_length] = '\0';
    }

    int txtsize = 0;
    if (p_argv_index > 0)
    {
        txtsize = strlen(argv[p_text_argv_index]);
    }
    int t = txtsize;
    int count = 0;
    while (t > 0)
    {
        t = t / 10;
        count++;
    }
    int to_add = 0; //count how much to add to the request string according to the parameters
    if (p_argv_index > 0)
    {
        to_add += 19;
    }
    if (r_argv_index > 0)
    {
        to_add += 3;
    }

    //check there is no GARBAGE value in argv:
    for (int i = 0; i < argc; i++)
    {
        if (unUsed[i] == 0)
        {
            fprintf(stderr, "Usage: client [-p <text>] [-r n < pr1=value1 pr2=value2 …>] <URL>\n");
            if (path_index > 0)
                free(path);
            if (p_argv_index > 0)
                free(p_text);
            if (port_index > 0)
                free(port_str);
            free(host);
            free(url);
            exit(0);
        }
    }
    int lenght = to_add + args_total_lenght + count + p_text_length + strlen(cmd) + path_length + strlen(host) + strlen(host1) + strlen(http) + 6;
    char *request = (char *)malloc(lenght + 10);
    if (request == NULL)
    { //memory allocation check
        perror("allocation failed\n");
    }
    strcpy(request, cmd);
    strcat(request, "/");

    if (path_length > 0) //if there is path, add it to the request
        strcat(request, path);

    /* BUILD REQUEST PART BY PART */
    if (r_num_of_arguments > 0)
    {
        strcat(request, "?");
        for (int i = 0; i < r_num_of_arguments; i++)
        {
            strcat(request, argv[r_num_of_arguments_argv_index + 1 + i]); //all args of -r in a row
            if (i < r_num_of_arguments - 1)
                strcat(request, "&"); //between each value
        }
        strcat(request, " "); //as required
    }
    else
    {
        strcat(request, " "); //as required
    }
    strcat(request, http);   //as required
    strcat(request, "\r\n"); //as required
    strcat(request, host1);  //as required
    strcat(request, host);
    char *txtnum;
    if (p_text_argv_index > 0)
    {
        strcat(request, "\r\n");
        strcat(request, "Content-length:");
        strcat(request, "\0");
        txtnum = (char *)malloc(count + 1);
        if (txtnum == NULL)
        { //memory allocation check
            perror("allocation failed\n");
        }
        sprintf(txtnum, "%d", txtsize);
        strcat(request, txtnum);
    }
    strcat(request, "\r\n\r\n"); //as required
    if (p_text_argv_index > 0)
    {
        strcat(request, argv[p_text_argv_index]);
    }
    printf("HTTP request =\n%s\nLEN = %ld\n", request, strlen(request)); //print request
    int sockfd = -1, rc = -1;
    struct sockaddr_in serv_addr;  // connector's address information
    struct hostent *server = NULL; // IP address information
    char bufRead[MAXBUFFSIZE];

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) //get the socket FD
    {
        perror("Err to opening socket\n");
    }

    if ((server = gethostbyname(host)) == NULL) // get the host info
    {
        herror("Err while using gethostbyname");
        exit(0);
    }
    serv_addr.sin_family = AF_INET; // host byte order
    bcopy((char *)server->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port); // short, network byte order

    if (connect(sockfd, (const struct sockaddr *)&serv_addr,
                sizeof(serv_addr)) < 0)
    {
        perror("Err to connect\n");
    }
    rc = write(sockfd, request, strlen(request));
    if (rc == -1)
    {
        perror("Err to write\n");
    }
    int numread = 0;
    while (1)
    {
        if ((rc = read(sockfd, bufRead, sizeof(bufRead) - 1)) == -1)
        {
            perror("Err to read");
        }
        if (rc == 0)
            break;
        numread = numread + rc;
        bufRead[rc] = '\0';
        printf("%s", bufRead);
    }
    printf("\nTotal received response bytes: %d\n", numread);
    close(sockfd); //close socket

    //Free all memory that was in use:
    if (path_index > 0)
        free(path);
    if (p_argv_index > 0)
        free(p_text);
    if (port_index > 0)
        free(port_str);
    if (p_text_argv_index > 0)
        free(txtnum);
    free(request);
    free(host);
    free(url);

    return 0;
}