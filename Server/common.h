#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <pthread.h>

#define PORT 5555
#define MAX_LENGTH 512
#define MAX_CONN 5
#define DSK "./dsk_server/"

// 处理上传文件
void put_file(int socket, char *fname);
// 处理下载文件
void get_file(int socket, char *fname);
// 批量上传文件
void put_m_file(int socket, char *fext);
// 批量下载文件
void get_m_file(int socket, char *fext);
// 接收文件，
int receive_file(int socket, char *fname);
// 传输文件
int send_file(int socket, char *fname);
// 查找文件
int fileExist(char *fname);

void error(char *err);