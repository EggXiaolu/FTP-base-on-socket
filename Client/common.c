#include "common.h"

extern char buffer[MAX_LENGTH];

int error(char *err)
{
    perror(err);
    exit(EXIT_FAILURE);
}

void hello()
{
    printf("===========================\n"
           "             FTP        \n"
           "===========================\n");
}

void help()
{
    printf("commands:\n"
           "    PUT  <filename> --上传指定文件\n"
           "    GET  <filename> --下载指定文件\n"
           "    MPUT <filename> --上传指定后缀文件\n"
           "    MGET <filename> --下载指定后缀文件\n"
           "    EXIT            --退出程序\n");
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

int send_file(int socket, char *fname)
{
    char fpath[MAX_LENGTH];
    strcpy(fpath, DSK);
    strcat(fpath, fname);
    FILE *file = fopen(fpath, "r");
    if (file == NULL)
    {
        printf("%s打开失败\n", fname);
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

int receive_file(int socket, char *fname)
{
    char buffer[MAX_LENGTH] = {0};
    char fpath[MAX_LENGTH];
    strcpy(fpath, DSK);
    strcat(fpath, fname);
    FILE *out_file = fopen(fpath, "wb");
    if (out_file == NULL)
        printf("%s创建失败\n", fname);
    else
    {
        bzero(buffer, MAX_LENGTH);
        int out_file_block_sz = 0;
        while ((out_file_block_sz = recv(socket, buffer, MAX_LENGTH, 0)) > 0)
        {
            int write_sz = fwrite(buffer, sizeof(char), out_file_block_sz, out_file);
            if (write_sz < out_file_block_sz)
            {
                error("文件写入失败！");
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
        printf("文件写入成功\n\n");
        fclose(out_file);
    }
    return 1;
}

void put_file(int socket, char *fname)
{
    // 检查文件是否存在
    if (!fileExist(fname))
    {
        printf("%s不存在\n", fname);
        return;
    }
    // 向服务器发送请求
    send(socket, buffer, MAX_LENGTH, 0);
    // 检查服务器回复
    bzero(buffer, MAX_LENGTH);
    recv(socket, buffer, MAX_LENGTH, 0);
    if (strcmp(buffer, "OKAY") == 0)
    {
        // 文件没有重复
        send_file(socket, fname);
    }
    else
    {
        // 文件重复
        printf("文件已存在! \n你想要覆盖%s吗 ? (yes/no)\n", fname);
        bzero(buffer, MAX_LENGTH);
        fgets(buffer, MAX_LENGTH, stdin);
        buffer[strcspn(buffer, "\n")] = 0;
        send(socket, buffer, MAX_LENGTH, 0);
        if (strcmp(buffer, "yes") == 0)
        {
            // 确定覆盖
            bzero(buffer, MAX_LENGTH);
            recv(socket, buffer, MAX_LENGTH, 0);
            if (strcmp(buffer, "OKAY") == 0)
            {
                send_file(socket, fname);
            }
        }
    }
    bzero(buffer, MAX_LENGTH);
    recv(socket, buffer, MAX_LENGTH, 0);
    if (strcmp(buffer, "SUCCESS"))
    {
        printf("上传失败，请再次尝试！\n");
    }
    else
    {
        printf("文件上传完成! \n\n");
    }
}

void get_file(int socket, char *fname)
{
    // 向服务器发送请求
    send(socket, buffer, MAX_LENGTH, 0);
    bzero(buffer, MAX_LENGTH);
    recv(socket, buffer, MAX_LENGTH, 0);

    if (strcmp(buffer, "READY") == 0)
    {
        if (fileExist(fname))
        {
            printf("文件已存在! \n你想要覆盖%s吗 ? (yes/no)\n", fname);
            bzero(buffer, MAX_LENGTH);
            fgets(buffer, MAX_LENGTH, stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            send(socket, buffer, MAX_LENGTH, 0);
        }
        bzero(buffer, MAX_LENGTH);
        strcpy(buffer, "yes");
        send(socket, buffer, MAX_LENGTH, 0);
        bzero(buffer, MAX_LENGTH);
        receive_file(socket, fname);
        bzero(buffer, MAX_LENGTH);
    }
    else
    {
        printf("%s未找到！\n", fname);
    }
}

void put_m_file(int socket, char *fext)
{
    DIR *di;
    struct dirent *dir;
    di = opendir(DSK);
    if ((dir = readdir(di)) == NULL)
    {
        printf("无目标文件\n");
        return;
    }
    while ((dir = readdir(di)) != NULL)
    {
        // 遍历文件夹
        char *fname = dir->d_name;
        char *ext = strrchr(fname, '.');
        if (strcmp(ext, fext) == 0)
        {
            bzero(buffer, MAX_LENGTH);
            strcpy(buffer, "PUT ");
            strcat(buffer, fname);
            put_file(socket, fname);
        }
    }
    closedir(di);
}

void get_m_file(int socket, char *fext)
{
    // 向服务器发送申请
    send(socket, buffer, MAX_LENGTH, 0);
    while (1)
    {
        bzero(buffer, MAX_LENGTH);
        recv(socket, buffer, MAX_LENGTH, 0);
        if (strcmp(buffer, "OVER") != 0)
        {
            char fname[MAX_LENGTH];
            strcpy(fname, buffer);
            bzero(buffer, MAX_LENGTH);
            if (fileExist(fname))
            {
                printf("%s已存在! \n你想要覆盖吗 ? (yes/no)\n", fname);
                bzero(buffer, MAX_LENGTH);
                fgets(buffer, MAX_LENGTH, stdin);
                buffer[strcspn(buffer, "\n")] = 0;
                if (strcmp(buffer, "yes") != 0)
                {
                    bzero(buffer, MAX_LENGTH);
                    strcpy(buffer, "SKIP");
                    send(socket, buffer, MAX_LENGTH, 0);
                    continue;
                }
            }
            bzero(buffer, MAX_LENGTH);
            strcpy(buffer, "SEND");
            send(socket, buffer, MAX_LENGTH, 0);

            bzero(buffer, MAX_LENGTH);
            receive_file(socket, fname);
        }
        else
        {
            break;
        }
    }
    printf("所有文件传输完毕！\n");
}