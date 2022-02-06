#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <sys/file.h>
#include <fcntl.h>
#include<pthread.h>
#include<semaphore.h>

#include "db.c"
#include "scheduler.c"
// #include "scheduler.h"

#define PORT "3490" 
#define BACKLOG 10 

char *posted_data[8000];

void sigchld_handler(int s)
{
    (void)s; 
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
    errno = saved_errno;
}

void start_reaper(void)
{
    struct sigaction sa;

    sa.sa_handler = sigchld_handler; 
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int get_listener_socket(char *port)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int yes = 1;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; 

    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1)
        {
            perror("setsockopt");
            close(sockfd);
            freeaddrinfo(servinfo); 
            return -2;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); 
    if (p == NULL)
    {
        fprintf(stderr, "webserver: failed to find local address\n");
        return -3;
    }

    if (listen(sockfd, BACKLOG) == -1)
    {
        close(sockfd);
        return -4;
    }

    return sockfd;
}


int send_response(int fd, char *header, char *content_type, char *body)
{
    char response[65536];
    int content_length = strlen(body);

    char formatted_time[1024];
    time_t current_time = time(NULL);
    struct tm *p = localtime(&current_time);
    strftime(formatted_time, 1024, "%A, %B %d %X %Z %Y", p);

    int response_length = sprintf(response,
                                  "%s\n"
                                  "Date: %s\n"
                                  "Connection: close\n"
                                  "Content-Length: %d\n"
                                  "Content-Type: %s\n"
                                  "\n"
                                  "%s",

                                  header, formatted_time, content_length, content_type, body);
    int count = send(fd, response, response_length, 0);

    if (count < 0)
        perror("error in send_response");

    return count;
}


void resp_404(int fd, char *path)
{
    char response_body[1024];

    sprintf(response_body, "404: %s not found\n", path);

    send_response(fd, "HTTP/1.1 404 NOT FOUND", "text/html", response_body);
}

void get_root(int fd)
{
    char *response_body = "<html><head></head><body><h1>Welcome to Money Heist!</h1></body></html>\n";

    send_response(fd, "HTTP/1.1 200 OK", "text/html", response_body);
}


void get_username(int fd)
{
    char *response_body = "<html><head></head><body><h1>username</h1></body></html>\n";


    send_response(fd, "HTTP/1.1 200 OK", "text/html", response_body);
}


void get_date(int fd)
{
    char response_body[1024];

    time_t current_time = time(NULL);
    struct tm *p = localtime(&current_time);
    strftime(response_body, 1024, "%A, %B %d %X %Z %Y\n", p);

    send_response(fd, "HTTP/1.1 200 OK", "text/html", response_body);
}


void save_data(char *s)
{
    FILE *fp;

    fp = fopen("server_data.txt", "w");
    fputs(s, fp);
    fclose(fp);
}


char *read_data(char *buffer, size_t size)
{
    FILE *fp;

    fp = fopen("io.html", "r");

    if (!fp)
        return NULL;

    int num_bytes = fread(buffer, sizeof(char), size - 1, fp);
    buffer[num_bytes] = '\0';
    fclose(fp);

    return buffer;
}


void post_save(int fd, char *body)
{
    char *response_body = "{\"user1\":\"name\"}\n";
    printf("%s", body); 
    send_response(fd, "HTTP/1.1 200 OK", "application/json", response_body);

    save_data(body);
}


void get_save(int fd)
{
    char buffer[8192];
    char *response_body = read_data(buffer, 8192);

    send_response(fd, "HTTP/1.1 200 OK", "text/html", response_body);
}


char *find_end_of_header(char *header)
{
    char *p;
    p = strstr(header, "\n\n");

    if (p != NULL)
        return p;

    p = strstr(header, "\r\n\r\n");

    if (p != NULL)
        return p;

    p = strstr(header, "\r\r");
    return p;
}
// ////////////////////////////
void make_req(char * body) {
    char data[8000];
    strcpy(data,body);
    printf("%s",data);
    char *p = strtok (data,","),uname1[100]="",uname2[100]="",amt[100]="";
    strcpy(uname1,p);
    while (p!= NULL) { 
        p = strtok (NULL, ",");
        if (strlen(uname2)==0) strcpy(uname2,p);
        else if (strlen(amt)==0) strcpy(amt,p);
    }
    ReqNode req;
    req.user1=uname1;
    req.user2=uname2;
    req.amount=atoi(amt);
    req.type=1;
    add_request(req);
}
// ///////////////////
void handle_http_request(int fd)
{
    const int request_buffer_size = 65536; 
    char request[request_buffer_size];
    char *p;
    int bytes_recvd = recv(fd, request, request_buffer_size - 1, 0);

    if (bytes_recvd < 0)
    {
        perror("recv");
        return;
    }

    request[bytes_recvd] = '\0';

    char *first_line = request;
    p = strchr(first_line, '\n');
    *p = '\0';
    char *header = p + 1; 
    p = find_end_of_header(header);

    if (p == NULL)
    {
        printf("Could not find end of header\n");
        exit(1);
    }

    char *body = p;

    char method[16];
    char path[1024];

    sscanf(request, "%s %s", method, path);

    printf("Method: %s\n", method);
    printf("Path: %s\n", path);


    if (strcmp(method, "GET") == 0)
    {
        if (strcmp(path, "/") == 0)
        {
            get_root(fd);
        }
        else if (strcmp(path, "/username") == 0)
        {
            srand(time(NULL));
            get_username(fd);
        }
        else if (strcmp(path, "/date") == 0)
        {
            get_date(fd);
        }
        
        else if (strcmp(path, "/save") == 0)
        {
            get_save(fd);
        }
        else
        {
            resp_404(fd, path);
        }
    }
    else if (strcmp(method, "POST") == 0)
    {
        if (strcmp(path, "/save") == 0)
        {
            post_save(fd, body);
        }
        else if (strcmp(path, "/saveln") == 0)
        {
            char *posted_data = body;
            // printf("%s",body);
            make_req(body);
            send_response(fd, "HTTP/1.1 200 OK", "text/html", body);

        }
        else
        {
            resp_404(fd, path);
        }
    }
}


void listen_Server()
{
    int newfd;                          
    struct sockaddr_storage their_addr; 
    char s[INET6_ADDRSTRLEN];

    start_reaper();

    int listenfd = get_listener_socket(PORT);

    if (listenfd < 0)
    {
        fprintf(stderr, "webserver: fatal error getting listening socket\n");
        exit(1);
    }

    printf("webserver: waiting for connections...\n");

    while (1)
    {
        socklen_t sin_size = sizeof their_addr;
        newfd = accept(listenfd, (struct sockaddr *)&their_addr, &sin_size);
        if (newfd == -1)
        {
            perror("accept");
            continue;
        }
        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (fork() == 0)
        {
            close(listenfd);
            handle_http_request(newfd);
            exit(0);
        }

        close(newfd);
    }
}

int main() {
    create_db();
    clean_table("net_dues");
    clean_table("users");
    clean_table("trans");

    sem_init(&mutex_list, 0, 1);
    sem_init(&mutex_execute, 0, 1);

    req_head = NULL;
    req_tail = req_head;
    pthread_t ptid1,ptid2;
	pthread_t executers[10];

    pthread_t ptid3;
    pthread_create(&ptid3, NULL, &test_gen_requests, NULL);


    pthread_create(&ptid2, NULL, &listen_Server, NULL);
    pthread_create(&ptid2, NULL, &wake_up_request_Priority_Scheduling, NULL);
    
    int num = 2;
    int w;
	for(w=0;w<num;w++)
	{
		pthread_create(&executers[w],NULL,&wake_up_request_Priority_Scheduling,NULL); 
        printf("\nExecutor num %d : %d\n",(w+1),executers[w]);
    }


    for(w=0;w<num;w++)
	{
        pthread_join(executers[w], NULL);
    }



    pthread_join(ptid1,NULL);
    pthread_join(ptid2,NULL);
    return 0;
}