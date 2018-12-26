#include "sysv_queue.h"
#include <signal.h>

#define MAXUSER 100

/* Tablica pidów userów połączonych z serwerem */
long pidy[MAXUSER];
int msqid;


/* Usuwanie obiektu IPC */
void wyjdz(){
        printf("Wyjdz\n");	
	if(msgctl(msqid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
    }
    exit(0);
}



int main(void)
{
    struct my_msgbuf buf;
    key_t key;

    signal(SIGINT, wyjdz);
    signal(SIGQUIT, wyjdz);

    if ((key = ftok("/bin/cp", 'B')) == -1) {
        perror("ftok");
        exit(1);
    }
    /* Stworzenie obiektu IPC */
    if ((msqid = msgget(key, 0644 | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(1);
    }

    printf("Key 0x%x\n", key);

    buf.mtype = 1;
    int pom = 0; 
    while(1) {
        if (msgrcv(msqid, (struct msgbuf *)&buf, sizeof(buf), 1, 0) == -1) {
            perror("msgsnd");
	}
	printf("WSZEDLEM %ld\n", buf.mtype);
	/* Wpisanie połączonego clienta do teblicy userów */
	if (buf.option == 1) {
		printf("Proces pid: %ld dolaczyl\n", buf.pid);
		pidy[pom] = buf.pid;
		pom += 1;
	}
	/* Ustawienie opcji 2 oznacza broadcast serwera, tzn rozsyła wiadomość
	 * do wszystkich pidów w tablicy oprócz tego, który wysłał mu te 
	 * wiadomość */
	else if (buf.option == 2) {
		int i;
		printf("Option 2\n");
		for(i = 0; i < pom; ++i) {
			if (pidy[i] != buf.pid) {
				/* priorytet */
				buf.mtype = pidy[i];
				msgsnd(msqid, (struct msgbuf *)&buf, sizeof(buf), 0);
			}
		}
	}
	else if (buf.option == 3) {
		printf("Option 3\n");
		/* Opcja 3 odłączenie się usera od serwera zmaiana miejscami 
		 * ostatniego podłączonego pida clienta na miejsce 
		 * pidu clienta który chce się odłączyć i zmniejszenie
		 * wartości pomocniczej określającej ilość podłączonych
		 * clientów */
		int i;
		for (i = 0; i < pom; ++i) {
			if (pidy[i] == buf.pid) {
				pidy[i] = pidy[pom - 1];
				pom -= 1;
				break;
			}
		}
	}
    }
    return 0;
}
