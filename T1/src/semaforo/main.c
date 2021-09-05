#include <stdio.h>
#include <unistd.h>
#include "../file_manager/manager.h"

int color = 1;
void handler(int entero, siginfo_t sig, void* vacio)
{
  // enviar color
  send_signal_with_int(entero, color);
}

int main(int argc, char const *argv[])
{
  pid_t pid_fabrica = atoi(argv[2]);
  int id = atoi(argv[0]);

  printf("I'm the SEMAFORO process and my PID is: %i\n", getpid());
  printf("ID: %s \n", argv[0]);
  printf("DELAY: %s \n", argv[1]);
  printf("ID FABRICA: %s \n", argv[2]);

  int delay = atoi(argv[1]);
  while(1)
  {
    sleep(delay);
    color = 1-color;
    send_signal_with_int(pid_fabrica, id*10 + color);
  }

}
