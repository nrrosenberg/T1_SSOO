#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h> 
#include <time.h>
#include <stdlib.h>

#include "../file_manager/manager.h"

int indice_repartidor = 0;
int N = 5;
int pids_repartidores[5];
pid_t pid_fabrica_senal;

int distancia_semaforo_1 = 5;
int distancia_semaforo_2 = 10;
int distancia_semaforo_3 = 15;
int distancia_bodega = 20;

int tiempo_de_creacion = 5;
int envios_necesarios = 66;
int delays_semaforos[] = {7, 8, 9};

int semaforos[] = {1,1,1};

void alarm_repartidores_handler(int signum)
{
  pids_repartidores[indice_repartidor] = fork();
  if (pids_repartidores[indice_repartidor] == 0)
  {
    // pid fabrica
    char pid[50];
    snprintf(pid, sizeof(pid), "%i", pid_fabrica_senal);
    // distancia bodega
    char bod[50];
    snprintf(bod, sizeof(bod), "%i", distancia_bodega);
    // distancia 1
    char dis1[50];
    snprintf(dis1, sizeof(dis1), "%i", distancia_semaforo_1);
    // distancia 2
    char dis2[50];
    snprintf(dis2, sizeof(dis2), "%i", distancia_semaforo_2);
    // distancia 3
    char dis3[50];
    snprintf(dis3, sizeof(dis3), "%i", distancia_semaforo_3);
    // semaforo 1
    char sem1[50];
    snprintf(sem1, sizeof(sem1), "%i", semaforos[0]);
    // semaforo 2
    char sem2[50];
    snprintf(sem2, sizeof(sem2), "%i", semaforos[1]);
    // semaforo 3
    char sem3[50];
    snprintf(sem3, sizeof(sem3), "%i", semaforos[2]);
    char idx[50];
    snprintf(idx, sizeof(idx), "%i", indice_repartidor);
    char *arr[] = {pid, bod, dis1, dis2, dis3, sem1, sem2, sem3, idx, NULL};
    execv("./repartidor", arr);
  } else if (pids_repartidores[indice_repartidor] > 0)
  {
    indice_repartidor++;
    if (indice_repartidor < N)
    {
      alarm(tiempo_de_creacion);
    }
  }
}

void handle_semaforo(int sig, siginfo_t *siginfo, void *ucontext)
{
  int valor_recibido = siginfo -> si_value.sival_int;
  int id = valor_recibido/10;
  int color = valor_recibido - id*10;
  printf("Semaforo con id %i cambiando de color \n", id);
  semaforos[id-1] = color;
  for(int i=0; i<N; i++)
  {
    send_signal_with_int(pids_repartidores[i], valor_recibido);
  }
}


int main(int argc, char const *argv[])
{
  printf("I'm the DCCUBER process and my PID is: %i\n", getpid());

  char *filename = "input.txt";
  InputFile *data_in = read_file(filename);

  printf("Leyendo el archivo %s...\n", filename);
  printf("- Lineas en archivo: %i\n", data_in->len);
  printf("- Contenido del archivo:\n");

  printf("\t- ");
  for (int i = 0; i < 4; i++)
  {
    printf("%s, ", data_in->lines[0][i]);
  }
  printf("\n");

  printf("\t- ");
  for (int i = 0; i < 5; i++)
  {
    printf("%s, ", data_in->lines[1][i]);
  }
  printf("\n");

  printf("Liberando memoria...\n");
  input_file_destroy(data_in);



  // PROCESO PRINCIPAL
  pid_t pid_fabrica = fork();
  if (pid_fabrica == 0)
  {
    // PROCESO FABRICA
    pid_fabrica_senal = getpid();
    signal(SIGALRM, alarm_repartidores_handler);
    connect_sigaction(SIGUSR1, handle_semaforo);
    alarm(1);
    sleep(1);
    time_t seconds;
    time_t seconds2;
    time(&seconds);
    for (int j = 0; j < N; j++)
    {
      time(&seconds2);
      if(tiempo_de_creacion > seconds2 - seconds)
      {
        sleep(tiempo_de_creacion - (seconds2 - seconds));
      }
      int status;
      pid_t exited_child = wait(&status);
      printf("Proceso terminado: %i con status: %i \n", exited_child, status);
    }
  }
  else if (pid_fabrica > 0)
  {
    // continuación proceso principal
    int pids_semaforos[3];
    for (int i = 0; i < 3; i++)
    {
      pids_semaforos[i] = fork();
      if (pids_semaforos[i] == 0)
      {
        char pid[50];
        snprintf(pid, sizeof(pid), "%i", pid_fabrica);
        char num[50];
        snprintf(num, sizeof(num), "%i", i + 1);
        char delay[50];
        snprintf(delay, sizeof(delay), "%i", delays_semaforos[i]);
        char *arr[] = {num, delay, pid, NULL};
        execv("./semaforo", arr);
      }
    }
    // Proceso principal termina cuando terminan las otras mierdas.
    // WAIT FABRICA
    // DESPUES MANDAR SEÑAL A SEMAFOROS
    int status;
    pid_t exited_child = wait(&status);
  }
} 
