#include "common.h"

// 子线程
void *service(void *sock_fd)
{
    int accept_sockfd = *((int *)sock_fd);
    char buffer[MAX_LENGTH];
    bzero(buffer, MAX_LENGTH);
    while (1)
    {
        // receive command from user
        recv(accept_sockfd, buffer, MAX_LENGTH, 0);
        char cmd_line[MAX_LENGTH];
        strcpy(cmd_line, buffer);
        char *cmd = strtok(cmd_line, " ");
        // printf("%s\n", cmd);
        if (strcmp(cmd, "EXIT") == 0 || strcmp(cmd, "exit") == 0)
        {
            // 退出
            break;
        }
        else if (strcmp(cmd, "GET") == 0 || strcmp(cmd, "get") == 0)
        {
            // 下载文件
            cmd = strtok(NULL, " ");
            get_file(accept_sockfd, cmd);
        }
        else if (strcmp(cmd, "PUT") == 0 || strcmp(cmd, "put") == 0)
        {
            // 上传文件
            cmd = strtok(NULL, " ");
            put_file(accept_sockfd, cmd);
        }
        else if (strcmp(cmd, "MGET") == 0 || strcmp(cmd, "mget") == 0)
        {
            // 批量获取文件
            cmd = strtok(NULL, " ");
            get_m_file(accept_sockfd, cmd);
        }
        else if (strcmp(cmd, "MPUT") == 0 || strcmp(cmd, "mput") == 0)
        {
            // 批量获取文件
            // cmd = strtok(NULL, " ");
            // put_m_file(accept_sockfd, cmd);
        }
        else
        {
            continue;
        }
    }
    close(accept_sockfd);
    pthread_exit(NULL);
}

int main()
{
    int server_sock;
    struct sockaddr_in client_addr;
    char buffer[MAX_LENGTH];
    bzero(buffer, MAX_LENGTH);

    // 创建套接字
    if (!(server_sock = socket(AF_INET, SOCK_STREAM, 0)))
        error("创建套接字失败");

    // 绑定套接字
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = INADDR_ANY;
    client_addr.sin_port = htons(PORT);
    if (bind(server_sock, (struct sockaddr *)&client_addr, (socklen_t)sizeof(client_addr)) < 0)
        error("绑定套接字失败");

    // 监听端口
    if (listen(server_sock, MAX_CONN) < 0)
        error("监听接口失败");

    // 循环创建线程
    while (1)
    {
        int accept_sockfd;
        int addr_len = sizeof(client_addr);

        if ((accept_sockfd = accept(server_sock, (struct sockaddr *)&client_addr, (socklen_t *)&addr_len)) < 0)
            error("accept");

        pthread_t thread;
        if (pthread_create(&thread, NULL, service, (void *)&accept_sockfd) != 0)
            error("线程创建失败");

        pthread_detach(thread); // 可能需要调整线程的管理方式，如join等
    }
    close(server_sock);

    return 0;
}
