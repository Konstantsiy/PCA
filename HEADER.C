#include <time.h>

#define BAUD 12
#define PACKET_END 0
#define MAX_BUF_SIZ 512
#define TIMEOUT 0.02

// проверяем готовность передатчика для отправки следующего байта
int check_byte_send(int base_port) {
	unsigned char reg = inp(base_port + 0x05); // читаем значение регистра состояния линии (3fdh)
	return ((reg & 0x20) >> 5) == 0x01; // 0x20 = 00100000 -> проверяем значение 5-го бита (индикатор пустоты регистра хранения данных)
}

// проверка наличия новых байтов в порте
int check_rcv(int base_port) {
	unsigned char reg = inp(base_port + 0x05);
	return (reg & 0x01) == 0x01; // проверяем 1-й бит регистра состояния линии (1 - есть данные, 0 - данных нет)
}

// чтение байта данных из регистра данных приемника RBR
char com_inchar(int base_port) {
	return inp(base_port);
}

char* com_receive_string(int base_port) {
	int i = 0;
	char buffer[MAX_BUF_SIZ];
	time_t control = 0, checker = 0;
	do {
		while (!check_rcv(base_port)) { // пока в регистре состояния линии есть данные
			checker = time(0);
			if (control) {
				if (difftime(checker, control) > TIMEOUT) {
					buffer[i] = '\0';
					return buffer;
				}
			}
		};
		buffer[i] = com_inchar(base_port);
		control = time(0);
	} while (buffer[i++] != PACKET_END);
	return buffer;
}

void com_init(int base_port) {
	unsigned int reg;
	// устанавливаем бит DLAB регистра LCR
	// DLAB - при установке в 1 порты со смещением 0 и 1 работают в режиме установки делителя частоты
	reg = inp(base_port + 0x03); // в регистре управления линией (3fbh)
	outp(base_port + 0x03, reg | 0x80); // 0x80 = 10000000
	
	// записываем значение делителя частоты 9600 бод
	outp(base_port, 0x0C);			// DLM - младший байт делителя частоты в порт 3f8h
	outp(base_port + 0x01, 0x00);	// DLL - старший байт делителя частоты в порт 3f9h

	// настраиваем регистр управления линией 
	outp(base_port + 0x03, reg & 0x7f); // сбрасываем бит DLAB регистра LCR (0x7f = 01111111)
	
	outp(base_port + 0x01, 0x00); // отключаем прерывания (1-й бит регистра 3f9h)
   
	outp(base_port + 0x03, 0x1b); // 0x1b = 0 | 0 (выключить состояние перерыва передачи) | 011 (чётный) | 0 (1 стоповый бит) | 11 (8 бит)

	// настраиваем регистр управления модемом (3fch)
	outp(base_port + 0x04, 0x00); // DTR=0 и RTS=0
}

void com_send(char symbol, int base_port) {
	unsigned char status;
	unsigned char mcr = inp(base_port + 0x04); // читаем регистр управления модемом (3fch)
	outp(base_port + 0x04, mcr | 0x02);	// устанавливаем бит RTS

	do { // проверяем состояние линии перед записью данных в порт
		status = inp(base_port + 0x05) & 0x40;
	} while (status != 0x40);

	outp(base_port, symbol);

	while (check_byte_send(base_port)); // ждём готовности передатчика
	outp(base_port + 0x04, mcr | 0xfd); // 0xfd = 11111101 ---> сбрасываем бит RTS
}

void com_send_string(char *string, int base_port) {
	do {
		com_init(base_port);
		com_send(*string, base_port);
	} while (*(string++));
}

