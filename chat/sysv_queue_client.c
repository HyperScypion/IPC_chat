#include "sysv_queue.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

int msqid;
long parentpid;
/* Obsługa wywołań systemowych CTRL + C i CTRL + D */
void wyjdz() {
	struct my_msgbuf buf;
	buf.pid = parentpid;
	/* Dodajemy pid który wysyła wiadomość serwera i opcję */
	buf.option = 3;
	msgsnd(msqid, (struct msgbuf *)&buf, sizeof(buf), 0);
	exit(0);
}


int main(int argc, char *argv[])
{
    struct my_msgbuf buf;
    key_t key;
    pid_t pid;

    parentpid = getpid();
    /* Obsługa sygnałów */
    signal(SIGINT, wyjdz);
    signal(SIGQUIT, wyjdz);
    /* Gdy agrmunetów jest mniej niż dwa połącz automatycznie kluczem wygenerowanym przez ftok */
    if (argc < 2) {

    	if ((key = ftok("/bin/cp", 'B')) == -1) {  
        	perror("ftok");
        	exit(1);
    	}
    }
    /* Gdy argumentów jest 2 to wpisz w klucz 2 argument */
    else {
	    long klucz = strtol(argv[1], NULL, 0);
	    key = klucz;
    }
    
    if ((msqid = msgget(key, 0644)) == -1) {
        perror("msgget");
        exit(1);
    }
    /* Wypisz klucz */
    printf("Key 0x%x\n", key);
    /* Okreś priorytet wiadomości, wypełnij odpowienie pola struktury */
    buf.mtype = 1;
    buf.pid = getpid();
    /* Wpisz nick użytkownika do struktury */
    strcpy(buf.nickname, getlogin());
    /*JOIN: 1, SEND: 2, QUIT: 3 */
    buf.option = 1;
    msgsnd(msqid, (struct msgbuf *)&buf, sizeof(buf), 0);

    if ((pid = fork()) < 0) {
	    perror("Blad:\n");
	}
    else if (pid == 0) {
    	for(;;) { 
		/* Potomek odbiera rodzic wysyła */
        	if (msgrcv(msqid, (struct msgbuf *)&buf, sizeof(buf), parentpid, 0) == -1) {
            		perror("msgrcv");
            		exit(1);
        	}
        	printf("[%s]: %s",buf.nickname, buf.mtext);
    	}
    }
    else {
	/* Okreś priorytem wiadomości, żeby nie czytać wszystkich komunikatów przez serwer */
	buf.mtype = 1;
	while(fgets(buf.mtext, MAXLINE, stdin) != NULL) {
		buf.option = 2;
		if(strcmp(buf.mtext, "/exit\n") == 0) {
			wyjdz();
		}
		if(strcmp(buf.mtext, "\n") != 0) {
			if (msgsnd(msqid, (struct msgbuf *)&buf, sizeof(buf), 0) == -1) {
				perror("msgsnd");
			}
		}
	}
    }
    wyjdz();
    return 0;
}
