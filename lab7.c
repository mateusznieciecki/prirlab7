#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"
#define REZERWA 500
#define PORT 1
#define ODPLYNIECIE 2
#define PODROZ 3
#define DOPLYNIECIE 4
#define TANKOWANIE 5
#define ZATONIECIE 6
#define TANKUJ 1500
int paliwo = 2000, dlugosc_podrozy = 0;
int CUMUJ=1, NIE_CUMUJ=0;
int liczba_procesow;
int nr_procesu;
int ilosc_statkow;
int ilosc_miejsc=5;
int ilosc_zajetych_miejsc=0;
int tag=2115;
int wyslij[2];
int odbierz[2];
MPI_Status mpi_status;

void Wyslij(int nr_statku, int stan)
{
	wyslij[0]=nr_statku;
	wyslij[1]=stan;
	MPI_Send(&wyslij, 2, MPI_INT, 0, tag, MPI_COMM_WORLD);
	sleep(1);
}

void Port(int liczba_procesow){
	int nr_statku, status;
	ilosc_statkow = liczba_procesow - 1;
	if(rand()%2==1){
		printf("Na morzu dzis piekna pogoda, nic tylko zeglowac\n");
	}
	else{
		printf("Prawdziwy sztorm, zegluga raczej nie bedzie przyjemna...\n");
	}
	printf("Jest %d miejsc w porcie\n", ilosc_miejsc);
	sleep(2);
	while(ilosc_miejsc<=ilosc_statkow){
		MPI_Recv(&odbierz,2,MPI_INT,MPI_ANY_SOURCE,tag,MPI_COMM_WORLD, &mpi_status);
		nr_statku=odbierz[0];
		status=odbierz[1];
		if(status==1){
			printf("Statek o numerze %d stoi na postoju\n", nr_statku);
		}
		if(status==2){
			printf("Statek o numerze %d wyplywa z portu z miejsca nr %d\n", nr_statku, ilosc_zajetych_miejsc);
			ilosc_zajetych_miejsc--;
		}
		if(status==3){
			printf("Statek o numerze %d plynie po szerokim morzu\n", nr_statku);
		}
		if(status==4){
			if(ilosc_zajetych_miejsc<ilosc_miejsc){
				ilosc_zajetych_miejsc++;
				MPI_Send(&CUMUJ, 1, MPI_INT, nr_statku, tag, MPI_COMM_WORLD);
			}
			else{
				MPI_Send(&NIE_CUMUJ, 1, MPI_INT, nr_statku, tag, MPI_COMM_WORLD);
			}
		}
		if(status==5){
			printf("Statek o numerze %d tankuje\n", nr_statku);
		}
		if(status==6){
			ilosc_statkow--;
			printf("Ilosc statkow %d\n", ilosc_statkow);
		}
	}
	printf("Program zakonczyl dzialanie\n");
}

void Statek(){
	int stan,suma,i;
	stan=PODROZ;
	while(1){
		if(stan==1){
			if(rand()%2==1){
				stan=ODPLYNIECIE;
				dlugosc_podrozy = 0;
				printf("Statek o numerze %d jest gotowy do podrozy\n",nr_procesu);
				Wyslij(nr_procesu,stan);
			}
			else{
				Wyslij(nr_procesu,stan);
			}
		}
		else if(stan==2){
			printf("Statek o numerze %d wyplywa z portu\n",nr_procesu);
			stan=PODROZ;
			Wyslij(nr_procesu,stan);
		}
		else if(stan==3){
			paliwo-=rand()%500; 
			dlugosc_podrozy += rand()%500;
			if(dlugosc_podrozy >= 2500){
                stan = DOPLYNIECIE;
		dlugosc_podrozy = 0;
                printf("Statek o numerze %d przeplynal ponad 2500 mil morskich\n",nr_procesu);
                Wyslij(nr_procesu, stan);
            }
			else if(paliwo<=REZERWA){
				stan=TANKOWANIE;
				printf("Statek o numerze %d chce zatankowac\n",nr_procesu);
				Wyslij(nr_procesu, stan);
			}
			else{
				for(i=0; rand()%10000;i++);
			}
		}
		else if(stan==4){
			int temp;
			MPI_Recv(&temp, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &mpi_status);
			if(temp == CUMUJ){
				stan=PORT;
				dlugosc_podrozy = 0;
				printf("Statek o numerze %d doplynal do portu\n", nr_procesu);
			}
			else
				{
				dlugosc_podrozy += rand()%500;
				int wypadek = rand()%10;
				if(wypadek == 3){
					stan = ZATONIECIE;
					printf("Statek o numerze %d brał udzial w katastrofie morskiej\n", nr_procesu);
					Wyslij(nr_procesu,stan);
				}
				else if(paliwo>0){
					stan=TANKOWANIE;
					Wyslij(nr_procesu,stan);
				}
				else{
					stan=ZATONIECIE;
					printf("Statek bral udzial w kolizji\n");
					Wyslij(nr_procesu,stan);
					return;
				}
			}
		}
		else if(stan == 5){
			printf("Statek o numerze %d tankuje\n", nr_procesu);
			paliwo = TANKUJ;
			stan = PODROZ;
			Wyslij(nr_procesu,stan);
		}
	}
}
int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&nr_procesu);
	MPI_Comm_size(MPI_COMM_WORLD,&liczba_procesow);
	srand(time(NULL));
	if(nr_procesu == 0)
		Port(liczba_procesow);
	else 
		Statek();
	MPI_Finalize();
	return 0;
}
