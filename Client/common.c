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
           "    put  <filename>    --上传指定文件\n"
           "    get  <filename>    --下载指定文件\n"
           "    mget <filename>    --上传指定后缀文件\n"
           "    mput <filename>    --下载指定后缀文件\n"
           "    lremove <filename> --删除本地文件\n"
           "    rremove <filename> --删除远程文件\n"
           "    EXIT               --退出程序\n");
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

void remove_local_file(char *fname)
{
    if (!fileExist(fname))
    {
        printf("%s不存在\n", fname);
        return;
    }
    printf("确定要删除本地文件%s?(yes/no):", fname);
    bzero(buffer, MAX_LENGTH);
    fgets(buffer, MAX_LENGTH, stdin);
    buffer[strcspn(buffer, "\n")] = 0;
    if (strcmp(buffer, "yes") != 0)
    {
        return;
    }
    char fpath[MAX_LENGTH];
    strcpy(fpath, DSK);
    fname = strcat(fpath, fname);
    if (remove(fname) == -1)
    {
        printf("%s删除失败\n", fname);
        return;
    }
    printf("%s删除成功\n", fname);
}

void remove_remote_file(int socket, char *fname)
{
    send(socket, buffer, MAX_LENGTH, 0);
    bzero(buffer, MAX_LENGTH);
    recv(socket, buffer, MAX_LENGTH, 0);
    if (strcmp(buffer, "NOTFOUND") == 0)
    {
        printf("%s不存在\n", fname);
        return;
    }

    printf("确定要删除远程文件%s?(yes/no):", fname);
    bzero(buffer, MAX_LENGTH);
    fgets(buffer, MAX_LENGTH, stdin);
    buffer[strcspn(buffer, "\n")] = 0;
    if (strcmp(buffer, "yes") != 0)
    {
        return;
    }
    send(socket, buffer, MAX_LENGTH, 0);

    bzero(buffer, MAX_LENGTH);
    recv(socket, buffer, MAX_LENGTH, 0);
    if (strcmp(buffer, "SUCCESS") != 0)
    {
        printf("%s删除失败\n", fname);
        return;
    }
    printf("%s删除成功\n", fname);
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
        printf("%s下载成功\n\n", fname);
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
        printf("%s已存在! \n你想要覆盖吗 ? (yes/no)\n", fname);
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
        printf("%s上传失败，请再次尝试！\n", fname);
    }
    else
    {
        printf("%s上传完成!\n\n", fname);
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
            printf("%s已存在! \n你想要覆盖吗 ? (yes/no)\n", fname);
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
        printf("%s不存在！\n", fname);
    }
}

void put_m_file(int socket, char *fext)
{
    DIR *di;
    struct dirent *dir;
    di = opendir(DSK);
    while ((dir = readdir(di)) != NULL)
    {
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