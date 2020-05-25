#include <dos.h>
#include <stdio.h>
#include <conio.h>
#include "header.c"

#define BASE_PORT 0x2f8
#define BAUD 12

typedef unsigned int uint;

int com_check_rhr(int);
int com_check_tbe(int);

int main(int argc, char* argv[]) {
	char a;
	int flag = 1;
	com_init(BASE_PORT);
	com_send_string(argv[0], BASE_PORT);

	printf("%s\n", com_receive_string(BASE_PORT));

	printf("1. %s\n", com_receive_string(BASE_PORT));
	printf("2. %s\n", com_receive_string(BASE_PORT));
	printf("3. %s\n", com_receive_string(BASE_PORT));

	com_init(BASE_PORT);
	do {
		a = getchar();
		switch (a) {
		case '1':
			com_send_string("1", BASE_PORT);
			flag = 0;
			break;
		case '2':
			com_send_string("2", BASE_PORT);
			flag = 0;
			break;
		case '3':
			com_send_string("3", BASE_PORT);
			flag = 0;
			break;
		}
	} while (flag);

	printf("%s\n", com_receive_string(BASE_PORT));
	return 0;
}