#include "common.h"

void error(char *err)
{
    perror(err);
    exit(EXIT_FAILURE);
}

int fileExist(char *fname)
{
    int found = 0;
    DIR *di;
    struct dirent *dir;
    di = opendir(DSK);
    while ((dir = readdir(di)) != NULL)
    {
        if (strcmp(dir->d_name, fname) == 0)
        {
            found = 1;
            break;
        }
    }
    closedir(di);
    return found;
}

int receive_file(int socket, char *fname)
{
    char buffer[MAX_LENGTH] = {0};
    char fpath[MAX_LENGTH];
    strcpy(fpath, DSK);
    strcat(fpath, fname);
    FILE *out_file = fopen(fpath, "wb");
    if (out_file == NULL)
    {
        return 0;
    }
    else
    {
        bzero(buffer, MAX_LENGTH);
        int out_file_block_sz = 0;
        while ((out_file_block_sz = recv(socket, buffer, MAX_LENGTH, 0)) > 0)
        {
            int write_sz = fwrite(buffer, sizeof(char), out_file_block_sz, out_file);
            if (write_sz < out_file_block_sz)
            {
                error("文件写入失败");
                return 0;
            }

            bzero(buffer, MAX_LENGTH);
            if (out_file_block_sz == 0 || out_file_block_sz != MAX_LENGTH)
            {
                break;
            }
        }
        if (out_file_block_sz < 0)
        {
            return 0;
        }
        fclose(out_file);
    }
    return 1;
}

int send_file(int socket, char *fname)
{
    char buffer[MAX_LENGTH] = {0};
    char fpath[MAX_LENGTH];
    strcpy(fpath, DSK);
    strcat(fpath, fname);
    FILE *file = fopen(fpath, "r");
    if (file == NULL)
    {
        return 0;
    }
    bzero(buffer, MAX_LENGTH);
    int fs_block_sz;
    while ((fs_block_sz = fread(buffer, sizeof(char), MAX_LENGTH, file)) > 0)
    {
        if (send(socket, buffer, fs_block_sz, 0) < 0)
        {
            fprintf(stderr, "错误: %s传输失败(errorid = %d)\n", fname, errno);
            break;
        }
        bzero(buffer, MAX_LENGTH);
    }
    return 1;
}

void put_file(int accept_sockfd, char *fname)
{
    char buffer[MAX_LENGTH];
    if (fileExist(fname))
    {
        // 文件重复
        bzero(buffer, MAX_LENGTH);
        strcpy(buffer, "CONTINUE");
        send(accept_sockfd, buffer, MAX_LENGTH, 0);

        bzero(buffer, MAX_LENGTH);
        recv(accept_sockfd, buffer, MAX_LENGTH, 0);
        if (strcmp(buffer, "yes") != 0)
        {
            // 否定覆盖
            return;
        }
    }
    bzero(buffer, MAX_LENGTH);
    strcpy(buffer, "OKAY");
    send(accept_sockfd, buffer, MAX_LENGTH, 0);
    if (receive_file(accept_sockfd, fname))
    {

        printf("上传成功！\n");
        bzero(buffer, MAX_LENGTH);
        strcpy(buffer, "SUCCESS");
        send(accept_sockfd, buffer, MAX_LENGTH, 0);
    }
}

void get_file(int accept_sockfd, char *fname)
{
    char buffer[MAX_LENGTH];
    if (fileExist(fname))
    {
        bzero(buffer, MAX_LENGTH);
        strcpy(buffer, "READY");
        send(accept_sockfd, buffer, MAX_LENGTH, 0);
        recv(accept_sockfd, buffer, MAX_LENGTH, 0);
        if (strcmp(buffer, "yes") == 0)
        {
            // 确定传输
            send_file(accept_sockfd, fname);
        }
        else
        {
            // 否定传输
            return;
        }
    }
    else
    {
        bzero(buffer, MAX_LENGTH);
        strcpy(buffer, "CANCEL");
        send(accept_sockfd, buffer, MAX_LENGTH, 0);
    }
}

void put_m_file(int socket, char *fext)
{
}

void get_m_file(int accept_sockfd, char *fext)
{
    char buffer[MAX_LENGTH];
    DIR *di;
    struct dirent *dir;
    di = opendir(DSK);
    bzero(buffer, MAX_LENGTH);
    while ((dir = readdir(di)) != NULL)
    {
        char *fname = dir->d_name;
        char *ext = strrchr(fname, '.');
        if (ext == NULL)
        {
            continue;
        }
        if (strcmp(ext, fext) == 0)
        {
            bzero(buffer, MAX_LENGTH);
            strcpy(buffer, fname);
            send(accept_sockfd, buffer, MAX_LENGTH, 0);
            bzero(buffer, MAX_LENGTH);
            recv(accept_sockfd, buffer, MAX_LENGTH, 0);
            if (strcmp(buffer, "SKIP") == 0)
            {
                continue;
            }
            else if (strcmp(buffer, "SEND") == 0)
            {
                bzero(buffer, MAX_LENGTH);
                send_file(accept_sockfd, fname);
            }
        }
    }
    bzero(buffer, MAX_LENGTH);
    strcpy(buffer, "OVER");
    send(accept_sockfd, buffer, MAX_LENGTH, 0);
    closedir(di);
}