
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <signal.h>

static int fd;
static void sig_func(int sig)
{
	int val;
	read(fd, &val, 4);
	printf("get button : 0x%x\n", val);
}

/*
 * ./button_test /dev/100ask_button0
 *
 */
int main(int argc, char **argv)
{
	int val;
	struct pollfd fds[1];
	int timeout_ms = 5000;
	int ret;
	int	flags;
	
	/* 1. 判断参数 */
	if (argc != 2) 
	{
		printf("Usage: %s <dev>\n", argv[0]);
		return -1;
	}

	signal(SIGIO, sig_func);

	/* 2. 打开文件 */
	fd = open(argv[1], O_RDWR);
	if (fd == -1)
	{
		printf("can not open file %s\n", argv[1]);
		return -1;
	}

	/*
		作用：设置接收异步通知信号的进程（通常是当前进程，将getpid的返回值填入）。

		F_SETOWN：告诉内核，当文件描述符 fd 有事件发生时，应该向哪个进程发送信号（SIGIO）。
		getpid()：获取当前进程的PID，表示信号应该发送给当前进程。
		效果：当设备驱动（如按键驱动）检测到事件（如按键按下）时，内核会向当前进程发送 SIGIO 信号
	*/
	fcntl(fd, F_SETOWN, getpid());

	/*
		作用：获取文件描述符 fd 的当前文件状态标志（File Status Flags）。

		F_GETFL：读取 fd 的当前标志位（如 O_RDONLY、O_NONBLOCK 等）。
		flags 变量存储当前的标志值。
		为什么需要这一步？

		因为 FASYNC 标志不能直接设置，而是需要先获取当前标志，再按位或（|）添加 FASYNC，最后重新设置回去。
	*/
	flags = fcntl(fd, F_GETFL);
	
	/*
		作用：启用文件描述符的异步通知模式（FASYNC）。

		F_SETFL：设置文件描述符的新标志。
		flags | FASYNC：在原有标志的基础上，添加 FASYNC 标志。
		FASYNC 的作用：

		告诉内核，该文件描述符需要支持异步通知（SIGIO）。
		当设备驱动调用 kill_fasync() 时，内核会向之前设置的进程（F_SETOWN）发送 SIGIO 信号
	*/
	fcntl(fd, F_SETFL, flags | FASYNC);

	while (1)
	{
		printf("www.100ask.net \n");
		sleep(2);
	}
	
	close(fd);
	
	return 0;
}


