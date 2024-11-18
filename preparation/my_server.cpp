#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>

#define STR_CLOSE "close"
#define STR_QUIT "quit"

//***************************************************************************
// log messages

#define LOG_ERROR 0 // errors
#define LOG_INFO 1  // information and notifications
#define LOG_DEBUG 2 // debug messages

// debug flag
int g_debug = LOG_INFO;

pthread_t threads[64];
int thread_count = 0;
int connected_clients[64];

void log_msg(int t_log_level, const char *t_form, ...)
{
    const char *out_fmt[] = {
        "ERR: (%d-%s) %s\n",
        "INF: %s\n",
        "DEB: %s\n"};

    if (t_log_level && t_log_level > g_debug)
        return;

    char l_buf[1024];
    va_list l_arg;
    va_start(l_arg, t_form);
    vsprintf(l_buf, t_form, l_arg);
    va_end(l_arg);

    switch (t_log_level)
    {
    case LOG_INFO:
    case LOG_DEBUG:
        fprintf(stdout, out_fmt[t_log_level], l_buf);
        break;

    case LOG_ERROR:
        fprintf(stderr, out_fmt[t_log_level], errno, strerror(errno), l_buf);
        break;
    }
}

//***************************************************************************
// help

void help(int t_narg, char **t_args)
{
    if (t_narg <= 1 || !strcmp(t_args[1], "-h"))
    {
        printf(
            "\n"
            "  Socket server example.\n"
            "\n"
            "  Use: %s [-h -d] port_number\n"
            "\n"
            "    -d  debug mode \n"
            "    -h  this help\n"
            "\n",
            t_args[0]);

        exit(0);
    }

    if (!strcmp(t_args[1], "-d"))
        g_debug = LOG_DEBUG;
}

//***************************************************************************

void *handle_client(void *arg)
{
    int l_sock_client = *(int *)arg;
    free(arg);

    while (1)
    { // communication
        char l_buf[256];

        // read data from socket
        int l_len = read(l_sock_client, l_buf, sizeof(l_buf));
        if (!l_len)
        {
            log_msg(LOG_DEBUG, "Client closed socket!");
            close(l_sock_client);
            break;
        }
        else if (l_len < 0)
        {
            log_msg(LOG_ERROR, "Unable to read data from client.");
            close(l_sock_client);
            break;
        }
        else
            log_msg(LOG_DEBUG, "Read %d bytes from client.", l_len);

        // write data to client
        l_len = write(STDOUT_FILENO, l_buf, l_len);
        if (l_len < 0)
            log_msg(LOG_ERROR, "Unable to write data to stdout.");

        // check if the buffer contains the character '+'
        if (strchr(l_buf, '+') != NULL)
        {
            // Split the buffer into two parts
            char *first_part = strtok(l_buf, "+");
            char *second_part = strtok(NULL, "+");

            // Parse both parts to int
            int num1 = atoi(first_part);
            int num2 = atoi(second_part);
            int result = num1 + num2;

            // Build the result string
            char result_str[256];
            snprintf(result_str, sizeof(result_str), "%d+%d=%d\n", num1, num2, result);

            for (int i = 0; i < thread_count; i++)
            {
                l_len = write(connected_clients[i], result_str, strlen(result_str));
            }

            if (l_len < 0)
                log_msg(LOG_ERROR, "Unable to write data to client.");
        }

        if (strchr(l_buf, '-') != NULL)
        {
            // Split the buffer into two parts
            char *first_part = strtok(l_buf, "-");
            char *second_part = strtok(NULL, "-");

            // Parse both parts to int
            int num1 = atoi(first_part);
            int num2 = atoi(second_part);
            int result = num1 - num2;

            // Build the result string
            char result_str[256];
            snprintf(result_str, sizeof(result_str), "%d-%d=%d\n", num1, num2, result);

            for (int i = 0; i < thread_count; i++)
            {
                l_len = write(connected_clients[i], result_str, strlen(result_str));
            }
            
            if (l_len < 0)
                log_msg(LOG_ERROR, "Unable to write data to client.");
        }

        if (strncmp("spring", l_buf, strlen("spring")) == 0)
        {
            int file_fd = open("spring.jpg", O_RDONLY);

            char file_buf[1024];
            int bytes_read;
            while ((bytes_read = read(file_fd, file_buf, sizeof(file_buf))) > 0)
            {
                int bytes_written = write(l_sock_client, file_buf, bytes_read);
            }
            close(file_fd);
        }
        else if (strncmp("summer", l_buf, strlen("summer")) == 0)
        {
            int file_fd = open("summer.png", O_RDONLY);

            char file_buf[1024];
            int bytes_read;
            while ((bytes_read = read(file_fd, file_buf, sizeof(file_buf))) > 0)
            {
                int bytes_written = write(l_sock_client, file_buf, bytes_read);
            }
            close(file_fd);
        }
        else if (strncmp("autumn", l_buf, strlen("autumn")) == 0)
        {
            int file_fd = open("autumn.jpg", O_RDONLY);

            char file_buf[1024];
            int bytes_read;
            while ((bytes_read = read(file_fd, file_buf, sizeof(file_buf))) > 0)
            {
                int bytes_written = write(l_sock_client, file_buf, bytes_read);
            }
            close(file_fd);
        }
        else if (strncmp("winter", l_buf, strlen("winter")) == 0)
        {
            int file_fd = open("winter.png", O_RDONLY);

            char file_buf[1024];
            int bytes_read;
            while ((bytes_read = read(file_fd, file_buf, sizeof(file_buf))) > 0)
            {
                int bytes_written = write(l_sock_client, file_buf, bytes_read);
            }
            close(file_fd);
        }

        // close request?
        if (!strncasecmp(l_buf, "close", strlen(STR_CLOSE)))
        {
            log_msg(LOG_INFO, "Client sent 'close' request to close connection.");
            close(l_sock_client);
            log_msg(LOG_INFO, "Connection closed. Waiting for new client.");
            for (int i = 0; i < thread_count; i++)
            {
                if (connected_clients[i] == l_sock_client)
                {
                    connected_clients[i] = 0;
                    thread_count--;
                    break;
                }
            }

            break;
        }

        // request for quit
        if (!strncasecmp(l_buf, "quit", strlen(STR_QUIT)))
        {
            close(l_sock_client);
            log_msg(LOG_INFO, "Request to 'quit' entered");
            for (int i = 0; i < thread_count; i++)
            {
                if (connected_clients[i] == l_sock_client)
                {
                    connected_clients[i] = 0;
                    thread_count--;
                    break;
                }
            }

            break;
        }
    } // while communication

    return NULL;
}

int main(int t_narg, char **t_args)
{
    if (t_narg <= 1)
        help(t_narg, t_args);

    int l_port = 0;

    // parsing arguments
    for (int i = 1; i < t_narg; i++)
    {
        if (!strcmp(t_args[i], "-d"))
            g_debug = LOG_DEBUG;

        if (!strcmp(t_args[i], "-h"))
            help(t_narg, t_args);

        if (*t_args[i] != '-' && !l_port)
        {
            l_port = atoi(t_args[i]);
            break;
        }
    }

    if (l_port <= 0)
    {
        log_msg(LOG_INFO, "Bad or missing port number %d!", l_port);
        help(t_narg, t_args);
    }

    log_msg(LOG_INFO, "Server will listen on port: %d.", l_port);

    // socket creation
    int l_sock_listen = socket(AF_INET, SOCK_STREAM, 0);
    if (l_sock_listen == -1)
    {
        log_msg(LOG_ERROR, "Unable to create socket.");
        exit(1);
    }

    in_addr l_addr_any = {INADDR_ANY};
    sockaddr_in l_srv_addr;
    l_srv_addr.sin_family = AF_INET;
    l_srv_addr.sin_port = htons(l_port);
    l_srv_addr.sin_addr = l_addr_any;

    // Enable the port number reusing
    int l_opt = 1;
    if (setsockopt(l_sock_listen, SOL_SOCKET, SO_REUSEADDR, &l_opt, sizeof(l_opt)) < 0)
        log_msg(LOG_ERROR, "Unable to set socket option!");

    // assign port number to socket
    if (bind(l_sock_listen, (const sockaddr *)&l_srv_addr, sizeof(l_srv_addr)) < 0)
    {
        log_msg(LOG_ERROR, "Bind failed!");
        close(l_sock_listen);
        exit(1);
    }

    // listenig on set port
    if (listen(l_sock_listen, 1) < 0)
    {
        log_msg(LOG_ERROR, "Unable to listen on given port!");
        close(l_sock_listen);
        exit(1);
    }

    log_msg(LOG_INFO, "Enter 'quit' to quit server.");

    // go!
    while (1)
    {
        int l_sock_client = -1;

        // list of fd sources
        pollfd l_read_poll[2];

        l_read_poll[0].fd = STDIN_FILENO;
        l_read_poll[0].events = POLLIN;
        l_read_poll[1].fd = l_sock_listen;
        l_read_poll[1].events = POLLIN;

        while (1) // wait for new client
        {
            // select from fds
            int l_poll = poll(l_read_poll, 2, -1);

            if (l_poll < 0)
            {
                log_msg(LOG_ERROR, "Function poll failed!");
                exit(1);
            }

            if (l_read_poll[0].revents & POLLIN)
            { // data on stdin
                char buf[128];
                int len = read(STDIN_FILENO, buf, sizeof(buf));
                if (len < 0)
                {
                    log_msg(LOG_DEBUG, "Unable to read from stdin!");
                    exit(1);
                }

                log_msg(LOG_DEBUG, "Read %d bytes from stdin");
                // request to quit?
                if (!strncmp(buf, STR_QUIT, strlen(STR_QUIT)))
                {
                    log_msg(LOG_INFO, "Request to 'quit' entered.");
                    close(l_sock_listen);
                    exit(0);
                }

                if (strncmp("debug", buf, strlen("debug")) == 0)
                {
                }
            }

            if (l_read_poll[1].revents & POLLIN)
            { // new client?
                sockaddr_in l_rsa;
                int l_rsa_size = sizeof(l_rsa);
                // new connection
                l_sock_client = accept(l_sock_listen, (sockaddr *)&l_rsa, (socklen_t *)&l_rsa_size);
                if (l_sock_client == -1)
                {
                    log_msg(LOG_ERROR, "Unable to accept new client.");
                    close(l_sock_listen);
                    exit(1);
                }

                int *client_sock = (int *)malloc(sizeof(int));
                *client_sock = l_sock_client;

                connected_clients[thread_count] = l_sock_client;
                pthread_create(&threads[thread_count++], NULL, handle_client, client_sock);

                uint l_lsa = sizeof(l_srv_addr);
                // my IP
                getsockname(l_sock_client, (sockaddr *)&l_srv_addr, &l_lsa);
                log_msg(LOG_INFO, "My IP: '%s'  port: %d",
                        inet_ntoa(l_srv_addr.sin_addr), ntohs(l_srv_addr.sin_port));
                // client IP
                getpeername(l_sock_client, (sockaddr *)&l_srv_addr, &l_lsa);
                log_msg(LOG_INFO, "Client IP: '%s'  port: %d",
                        inet_ntoa(l_srv_addr.sin_addr), ntohs(l_srv_addr.sin_port));
            }

        } // while wait for client
    } // while ( 1 )

    return 0;
}
