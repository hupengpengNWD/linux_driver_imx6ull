
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <poll.h>

/*
 * ./button_test /dev/100ask_button0
 *
 */
int main(int argc, char **argv)
{
	int fd;
	int val;
	struct pollfd fds[1];/* 只检测一个文件 */
	int timeout_ms = 5000;
	int ret;
	
	/* 1. 判断参数 */
	if (argc != 2) 
	{
		printf("Usage: %s <dev>\n", argv[0]);
		return -1;
	}

	/* 2. 打开文件 */
	fd = open(argv[1], O_RDWR);
	if (fd == -1)
	{
		printf("can not open file %s\n", argv[1]);
		return -1;
	}

	fds[0].fd = fd;/* 监测的文件描述符 */
	fds[0].events = POLLIN;/* 查询的事件类型 */
	

	while (1)
	{
		/* 3. 读文件 
		    参数1代表fds数组的大小，
		    参数timeout_ms超时时间（也就是休眠时间） 
			poll返回值=1表示有一个监控的文件有POLLIN类型事件发生了
			没有事件就会休眠
		*/
		ret = poll(fds, 1, timeout_ms);/* 可能在此处休眠处 */

		/* 条件含义：监控的文件中有1个文件发生了事件，并且事件类型是 POLLIN*/
		if((ret == 1) && (fds[0].revents & POLLIN))
		{
			read(fd, &val, 4);
			printf("get button : 0x%x\n", val);
		}
		else
		{
			/* 休眠时间到了之后会输出“timeout” */
			printf("timeout\n");
		}
	}
	
	close(fd);
	
	return 0;
}


