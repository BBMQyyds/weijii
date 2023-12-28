#define _POSIX_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "dispose.h"
#include "user.h"
#include "data.h"

#define PORT 8080
#define MAX_BUFFER_SIZE 8192
#define MAX_EVENTS 10
// 10 MB
#define MAX_FILE_SIZE (1024 * 1024 * 10)

void send_response(int client_socket, const char *status, const char *message);

void handle_client(int client_socket, User *users, Token *tokens, Other *others);

void sigint_handler(int signo);

// 全局标志变量，用于指示是否接收到中断信号
volatile sig_atomic_t shutdown_flag = 0;

// 定义共享内存的数据结构
typedef struct {
    User users[MAX_USERS];
    Token tokens[MAX_TOKENS];
    Other others[MAX_OTHERS];
} SharedMemory;

int main() {
    Data data = read_data("user.lzy");
    if (data.data == NULL) {
        printf("Error reading data\n");
        return 1;
    }

    // 读取用户数据
    char *user_data_string = data.data;

    // 创建共享内存
    int shmid = shmget(IPC_PRIVATE, sizeof(SharedMemory), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // 映射共享内存到进程地址空间
    SharedMemory *shared_memory = shmat(shmid, NULL, 0);
    if ((intptr_t) shared_memory == -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    // 初始化共享数据
    memset(&(shared_memory->users), 0, sizeof(User) * MAX_USERS);
    memset(&(shared_memory->tokens), 0, sizeof(Token) * MAX_TOKENS);
    memset(&(shared_memory->others), 0, sizeof(Other) * MAX_OTHERS);

    // 从文件中读取用户数据
    int num_users = read_users_from_string(user_data_string, shared_memory->users, MAX_USERS);
    print_users(shared_memory->users);

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 10) == -1) {
        perror("listen");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // 设置 SIGINT 信号处理函数，处理中断信号（Ctrl+C）
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create");
        exit(EXIT_FAILURE);
    }

    struct epoll_event event, events[MAX_EVENTS];
    event.events = EPOLLIN;
    event.data.fd = server_socket;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &event) == -1) {
        perror("epoll_ctl");
        close(epoll_fd);
        exit(EXIT_FAILURE);
    }

    while (!shutdown_flag) {
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

        if (num_events == -1) {
            if (errno == EINTR && shutdown_flag) {
                // 如果是中断信号，且已经设置了 shutdown_flag，就退出主循环
                break;
            }
            perror("epoll_wait");
            continue;
        }

        // 处理事件的逻辑
        for (int i = 0; i < num_events; ++i) {
            if (events[i].data.fd == server_socket) {
                // Accept a new connection
                client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &addr_len);
                if (client_socket == -1) {
                    perror("accept");
                    continue;
                }

                // 创建子进程处理连接
                pid_t pid = fork();
                if (pid == -1) {
                    perror("fork");
                    close(client_socket);
                    continue;
                }

                if (pid == 0) {
                    // 子进程
                    close(server_socket); // 子进程不需要监听套接字
                    handle_client(client_socket, shared_memory->users, shared_memory->tokens, shared_memory->others);
                    exit(EXIT_SUCCESS);
                } else {
                    // 父进程
                    close(client_socket);
                }
            } else {
                // Handle data on existing connections
                handle_client(events[i].data.fd, shared_memory->users, shared_memory->tokens, shared_memory->others);
            }
        }
    }

    // 释放内存等资源
    close(server_socket);
    close(epoll_fd);

    // 写入数据
    char *output = malloc(MAX_FILE_SIZE);
    if (output == NULL) {
        perror("malloc");
        return 1;
    }
    stringfy_user_data(output, shared_memory->users);
    printf("\nWriting data to file...\n");
    Data output_data = {output, strlen(output)};
    write_data("user.lzy", &output_data);

    free(data.data);
    free(output);

    // 解除映射
    if (shmdt(shared_memory) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }

    // 删除共享内存
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(EXIT_FAILURE);
    }

    return 0;
}

void send_response(int client_socket, const char *status, const char *message) {
    // 添加CORS头部，允许特定域的跨域请求
    char cors_header[] = "Access-Control-Allow-Origin: *\r\nAccess-Control-Allow-Credentials: true\r\n";

    // 发送 HTTP 响应
    char response[MAX_BUFFER_SIZE];
    snprintf(response, sizeof(response), "HTTP/1.1 %s\r\n%s\r\n\r\n%s", status, cors_header, message);
    send(client_socket, response, strlen(response), 0);
}

void handle_client(int client_socket, User *users, Token *tokens, Other *others) {
    char buffer[MAX_BUFFER_SIZE];
    ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

    if (bytes_received < 0) {
        perror("recv");
        close(client_socket);
        return;
    }

    if (bytes_received == 0) {
        close(client_socket);
        return;
    }

    buffer[bytes_received] = '\0';

    // 处理请求并获取相应的状态码和消息
    Response response;

    if (strstr(buffer, "GET") == NULL) {
        response.status = "405 Method Not Allowed";
        response.message = "Method Not Allowed";
    } else if (strstr(buffer, "/login") != NULL) {
        response = dispose_login_request(buffer, users, tokens, others);
    } else if (strstr(buffer, "/wait") != NULL) {
        response = dispose_wait_request(buffer, users, tokens, others);
    } else if (strstr(buffer, "/luozi") != NULL) {
        response = dispose_luozi_request(buffer, users, tokens, others);
    } else if (strstr(buffer, "/lose") != NULL) {
        response = dispose_lose_request(buffer, users, tokens, others);
    } else if (strstr(buffer, "/qipu") != NULL) {
        response = dispose_qipu_request(buffer, users, tokens, others);
    } else {
        // 默认返回 "400 Bad Request"，如果没有匹配的路径
        response.status = "400 Bad Request";
        response.message = "Invalid request";
    }

    send_response(client_socket, response.status, response.message);

    close(client_socket);
}

void sigint_handler(int signo) {
    // 设置中断标志
    shutdown_flag = 1;
}
