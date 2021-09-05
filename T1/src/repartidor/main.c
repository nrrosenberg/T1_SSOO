#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "../file_manager/manager.h"

int recorrido = 0;
int bodega = 100;
pid_t pid_fabrica;
int semaforos[3];
int distancias_semaforos[3];
int cantidad_turnos[4];
char* id_repartidor;

void write_file() 
{
  printf("Escribiendo archivo de reparidor con ID: %s \n", id_repartidor);
  FILE *fp;

  int count = atoi(id_repartidor);
  char filename[40];
  sprintf(filename, "./repartidor_%i.txt", count);
  printf("filename: %s", filename);

  fp = fopen(filename, "w+");
  int sem1 = recorrido >= distancias_semaforos[0] ? cantidad_turnos[0] : -1;
  int sem2 = recorrido >= distancias_semaforos[1] ? cantidad_turnos[1] : -1;
  int sem3 = recorrido >= distancias_semaforos[2] ? cantidad_turnos[2] : -1;
  int bod = recorrido == bodega ? cantidad_turnos[3] : -1;
  fprintf(fp, "%i, %i, %i, %i", sem1, sem2, sem3, bod);
  fclose(fp);
}


void handle_semaforo(int sig, siginfo_t *siginfo, void *ucontext)
{
  int valor_recibido = siginfo -> si_value.sival_int;
  int id = valor_recibido/10;
  int color = valor_recibido - id*10;
  semaforos[id-1] = color;
}

int main(int argc, char const *argv[])
{
  printf("I'm the REPARTIDOR process and my PID is: %i\n", getpid());
  bodega = atoi(argv[1]);
  id_repartidor = argv[8];
  pid_fabrica = atoi(argv[0]);
  distancias_semaforos[0] = atoi(argv[2]);
  distancias_semaforos[1] = atoi(argv[3]);
  distancias_semaforos[2] = atoi(argv[4]);
  semaforos[0] = atoi(argv[5]);
  semaforos[1] = atoi(argv[6]);
  semaforos[2] = atoi(argv[7]);
  int semaforos_pasados = 0;
  int turnos = 0;

  connect_sigaction(SIGUSR1, handle_semaforo);

  
  while(recorrido < bodega)
  {
    sleep(1);
    turnos++;
    if (recorrido < distancias_semaforos[semaforos_pasados] - 1)
    {
      recorrido++;
      printf("Repartidor %i: No hay semáforo, avanzo a %i \n", getpid(), recorrido);
    }
    else if (recorrido == distancias_semaforos[semaforos_pasados] - 1)
    {
      if (semaforos[semaforos_pasados])
      {
        recorrido++;
        printf("Repartidor %i: Semáforo en verde, avanzo a %i\n", getpid(), recorrido);
        cantidad_turnos[semaforos_pasados] = turnos;
        semaforos_pasados++;
        
      }
      else {
        printf("Repartidor %i: Semáforo en rojo, no puedo avanzar y me quedo en %i \n", getpid(), recorrido);
      }
    }
    else
    {
      recorrido++;
      printf("Repartidor %i: No hay semáforo, avanzo a %i \n", getpid(), recorrido);
    }
  }
  cantidad_turnos[3] = turnos;
  printf("Repartidor %i: Turnos para cruzar el primer semáforo: %i \n", getpid(), cantidad_turnos[0]);
  printf("Repartidor %i: Turnos para cruzar el segundo semáforo: %i \n", getpid(), cantidad_turnos[1]);
  printf("Repartidor %i: Turnos para cruzar el tercer semáforo: %i \n", getpid(), cantidad_turnos[2]);
  printf("Repartidor %i: llegó a la bodega  en %i turnos\n", getpid(), cantidad_turnos[3]);
  write_file();
}
