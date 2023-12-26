#include "common.h"
char buffer[MAX_LENGTH];
int main()
{
	int client_sock;
	struct sockaddr_in server_addr;
	bzero(buffer, MAX_LENGTH);

	// 创建套接字
	if (!(client_sock = socket(AF_INET, SOCK_STREAM, 0)))
		error("创建套接字失败\n");

	// 绑定套接字
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	if (inet_pton(AF_INET, ADDRESS, &server_addr.sin_addr) <= 0)
		error("套接字绑定失败\n");

	if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
		error("服务器连接失败\n");

	while (1)
	{
		hello();
		printf("请输入指令(详情输入help):");
		bzero(buffer, MAX_LENGTH);
		fgets(buffer, MAX_LENGTH, stdin);
		buffer[strcspn(buffer, "\n")] = 0;

		char cmd_line[MAX_LENGTH];
		strcpy(cmd_line, buffer);
		char *cmd = strtok(cmd_line, " ");

		if (strcmp("PUT", cmd) == 0 || strcmp("put", cmd) == 0)
		{
			// 上传指定文件
			cmd = strtok(NULL, " ");
			put_file(client_sock, cmd);
		}
		else if (strcmp("GET", cmd) == 0 || strcmp("get", cmd) == 0)
		{
			// 下载指定文件
			cmd = strtok(NULL, " ");
			get_file(client_sock, cmd);
		}
		else if (strcmp("MPUT", cmd) == 0 || strcmp("mput", cmd) == 0)
		{
			// 上传指定后缀文件
			cmd = strtok(NULL, " ");
			put_m_file(client_sock, cmd);
		}
		else if (strcmp("MGET", cmd) == 0 || strcmp("mget", cmd) == 0)
		{
			// 下载指定后缀文件
			cmd = strtok(NULL, " ");
			get_m_file(client_sock, cmd);
		}
		else if (strcmp("EXIT", buffer) == 0 || strcmp("exit", buffer) == 0)
		{
			// 上传指定后缀文件
			send(client_sock, buffer, MAX_LENGTH, 0);
			break;
		}
		else if (strcmp("HELP", buffer) == 0 || strcmp("help", buffer) == 0)
		{
			help();
		}
		else
		{
			printf("命令错误\n");
			continue;
		}
	}
	close(client_sock);
	return 0;
}