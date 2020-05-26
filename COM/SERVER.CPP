#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#include "header.c"

#define BASE_PORT 0x3f8 // базовый адрес порта COM1
#define DELAY_TIME 20	// время задержки

int check_byte_send(int);
int check_rcv(int);

union REGS i, temp;
const int labNameSize = 9;
int counter = DELAY_TIME;
int flag1 = 0;
int flag2 = 0; 
int flag3 = 0;
int in = 0;
unsigned int vector_base = 0xA0; // второй контроллер (команды)

char* library[] = {
		"lab3.exe",
		"lab4.exe",
		"lab6.exe",
};

char* comments[] = {
		"Available labs:",
		"Request processing...\n"
};

void interrupt(*oldint08)(...);
void interrupt newint08(...);
void interrupt newint09(...);
void call_int16(int);


int main() {
	char* request;
	printf("Connection with %s\n", com_receive_string(BASE_PORT));
	com_init(BASE_PORT);
	com_send_string(comments[0], BASE_PORT);

	com_send_string(library[0], BASE_PORT);
	com_send_string(library[1], BASE_PORT);
	com_send_string(library[2], BASE_PORT);

	//for (int i = 0; i < 3; i++) {
	//	com_send_string(labs[i], BASE_PORT);
	//}
	request = com_receive_string(BASE_PORT);
	printf("Lab: %s", request);
	com_init(BASE_PORT);
	com_send_string(comments[1], BASE_PORT);

	setvect(vector_base + 0x00, newint08);
	setvect(vector_base + 0x01, newint09);

	disable();
	outp(0x20, 0x11); // инициализация ведущего контроллера
	outp(0x21, vector_base); // переопределение вектора прерываний
	outp(0x21, 0x04);
	outp(0x21, 0x01);
	enable();

	switch (*request) {
	case '1':
		printf("%s", library[0]);
		flag1 = 1;
		break;
	case '2':
		printf("%s: \n", library[1]);
		flag2 = 1;
		break;
	case '3':
		printf("%s: \n", library[2]);
		flag3 = 1;
		break;
	}
	i.h.ah = 0x31;
	i.h.al = 0x00;
	i.x.dx = 0x8000; // адрес первого байта за резидентным участком программы (смещение от PSP)
	// int 27h - завершение работы программы с оставлением ее резидентной в памяти
	int86(0x27, &i, &i); // выполнения программного прерывания
	return 0;
}

void interrupt newint08(...) {
	if ((flag1 || flag2 || flag3) && !counter) {
		counter = DELAY_TIME;
		if (flag1) {
			if (in == 0) {
				call_int16(13); // 13 - D, коретка
				in++;
			}
			else if (in == labNameSize) {
				in = 0;
				flag1 = 0;
				call_int16(13);
			}
			else {
				call_int16(library[0][in - 1]);
				in++;
			}
		}
		else if (flag2) {
			if (in == labNameSize) {
				in = 0;
				flag2 = 0;
				call_int16(13);
			}
			else if (in == 0) {
				call_int16(13);
				in++;
			}
			else {
				call_int16(library[1][in - 1]);
				in++;
			}
		}
		else if (flag3) {
			if (in == labNameSize) {
				in = 0;
				flag3 = 0;
				call_int16(13);
			}
			else if (in == 0) {
				call_int16(13);
				in++;
			}
			else {
				call_int16(library[2][in - 1]);
				in++;
			}
		}
	}
	else counter--;
	int86(0x08, &temp, &temp);
}

void interrupt newint09(...) {
	int86(0x09, &temp, &temp);
}

void call_int16(int chr) {
	i.h.ah = 0x05;
	i.h.cl = chr;
	i.h.ch = 0;
	int86(0x16, &i, &i); // выходной параметр регистров (конкретно здесь не нужен)
}
