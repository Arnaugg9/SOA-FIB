#include <libc.h>

#define CLONE_THREAD 1

char buff[24];

char char_map2[] =
{
  '\0','\0','1','2','3','4','5','6',
  '7','8','9','0','\'','�','\0','\0',
  'q','w','e','r','t','y','u','i',
  'o','p','`','+','\0','\0','a','s',
  'd','f','g','h','j','k','l','�',
  '\0','�','\0','�','z','x','c','v',
  'b','n','m',',','.','-','\0','*',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','7',
  '8','9','-','4','5','6','+','1',
  '2','3','0','\0','\0','\0','<','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0'
};

char keyboard[128];
unsigned short* screen;

int pid;

void *thread_func_1(void *arg) {
  if (*(int*)arg == 1) write(1, "Hello from thread 1!\n", 22);
  pthread_exit();
}

void test_simple_clone() {
  int val = 1;
  int tid = clone(CLONE_THREAD, thread_func_1, &val, 1024);
  if (tid < 0)  write(1, "clone failed\n", 13);
  
  pause(50);
  write(1, "Done boss\n", 10);
}

void *thread_func_N(void *arg) {
  char* msg = (char *)arg;
  write(1, msg, strlen(msg));
  pthread_exit();
}

void test_multiple_threads() {
  int tid =  clone(CLONE_THREAD, thread_func_N, "Thread A\n", 1024);
  if (tid < 0) write(1, "clone failed\n", 13);

  tid = clone(CLONE_THREAD, thread_func_N, "Thread B\n", 1024);
  if (tid < 0) write(1, "clone failed\n", 13);

  tid = clone(CLONE_THREAD, thread_func_N, "Thread C\n", 1024);
  if (tid < 0) write(1, "clone failed\n", 13);

  pause(200);
  write(1, "Done boss, multiple\n", 20);
}

void *prio_func(void *arg) {
  int val = *((int*)arg);   // race condition...
  if (val == 0) {
    SetPriority(25);   // LOW
    pause(1);
  }
  else SetPriority(30);  // HIGH
  
  if (val == 0)
    for (int i = 0; i < 10; ++i)  write(1, "LOW\n", 4);

  else
    for (int i = 0; i < 10; ++i) { 
      if (i == 5) pause(1);			
      write(1, "HIGH\n", 5);
    }

  pthread_exit();
}

void test_priority() {
  int val1 = 0;
  clone(CLONE_THREAD, prio_func, &val1, 1024);  // LOW

  int val2 = 1;
  clone(CLONE_THREAD, prio_func, &val2, 1024);  // HIGH
  
  pause(1);
  write(1, "HIGH GOES BEFORE, RIGHT?\n", 25);
}

void draw_screen(unsigned short* screen) {

  if (screen != (void*) -1) {
    for (int i = 0; i < 2000; ++i) {
      screen[i] = ' ';
    }
  } 
  else perror("Screen error");

  unsigned short color = 0x07;

  screen[0] = color << 8 | 'k';
  screen[1] = color << 8 | 'e';
  screen[2] = color << 8 | 'y';
  screen[3] = color << 8 | 's';
  screen[4] = color << 8 | ' ';
  screen[5] = color << 8 | 'p';
  screen[6] = color << 8 | 'r';
  screen[7] = color << 8 | 'e';
  screen[8] = color << 8 | 's';
  screen[9] = color << 8 | 's';
  screen[10] = color << 8 | 'e';
  screen[11] = color << 8 | 'd';
  screen[12] = color << 8 | ':';
  screen[13] = color << 8 | ' ';
  
  while (1) {
    if (GetKeyboardState(keyboard) < 0) perror();
    int pos = 14;

    color = 1;
    for (int i = 0; i < 128; ++i) {
      if (keyboard[i] == 1) {
        screen[pos] = color << 8 | char_map2[i];
        pos += 2;
        ++color;
      }
    }
    for (int i = pos; i < 128; ++i) screen[i] = ' ';
    pause(1000);

  }
}

void *son_thread(void *arg) {
  write(1, "THREAD RUNNING\n", 15);
  pause(500); 

  test_simple_clone();
  pthread_exit();
}

void *forking(void *arg) {

  int val = *((int*)arg); //el thread al llegar aqui tiene el puntero apuntando a un valor != 1  
  int pid = fork();
  if (pid > 0) {
    write(1, "Cloned and forked!\n",20);
  }
  else if (pid == 0){
    write(1, "My father is a clone...\n",25);  // 2.
    if (val == 1) write(1,"YES\n",4);
  }
  else write(1,"ERROR\n",6);

  pthread_exit();
}

void test_clone_and_fork() {
 int val = 1;
 int ret = clone(CLONE_THREAD, forking, &val, 8192);  
 if (ret == 0) write(1, "Cloned succesfully, forking?\n", 30);
}

void test_exit_from_main() {
  int pid = fork();
  if (pid == 0) {
    write(1, "CHILD: I will now call exit from main thread\n", 45);
    exit(); // Debería terminar el proceso entero 1.
  } 
  else {
    pause(100);
    write(1, "PARENT: If I see this, child exited correctly\n", 46);
  }
}

void *killer_thread(void *arg) {
  write(1, "THREAD: Calling exit from a thread!\n", 35);
  exit(); // Deberia matar a todo el proceso
  return 0;
}

void test_exit_from_thread() {
  int pid = fork();
  if (pid == 0) {
    int val = 0;
    int ret = clone(CLONE_THREAD, killer_thread, &val, 4096);
    if (ret < 0) perror();
    pause(1);
    write(1, "You cannot see me\n", 19);// No deberia llegar aqui, será matado por el thread
  } 
  else {
    pause(200);
    write(1, "PARENT: Child should have exited\n", 33);
  }
}


//Tests de semafors
int sem_id;

char aux[1];
void *sem_thread(void *arg) {
  char id = *(char*)arg;
  aux[0] = id;
  //buff[0] = id;   // --> Si es descomenta (i comenta la posterior), en estar fora de la zona d'exclusió mutua hi ha problemes de condicio de carrera
  
  write(1, "Thread trying to enter critical section: ", 41);
  write(1, aux, 1);
  write(1, "\n", 1);
  
  if (sem_wait(sem_id) == 0) {
    buff[0] = id;
    write(1, "Thread in critical section: ", 28);
    write(1, buff, 1);
    write(1, "\n", 1);
    
    pause(13000);

    write(1, "Thread leaving critical section: ", 33);
    write(1, buff, 1);
    write(1, "\n", 1);

    sem_post(sem_id);
  } else {
    write(1, "Thread failed to acquire semaphore\n", 34);
  }

  pthread_exit();
}

void test_semaphores() {
  write(1, "\n--- Testing Semaphores ---\n", 28);

  sem_id = sem_init(1); // inicializa con valor 1 (mutex)
  if (sem_id < 0) {
    perror();
    return;
  }

  // Crea varias letras distintas para identificar a los hilos
  char ids[3] = { 'A', 'B', 'C' };

  for (int i = 0; i < 3; ++i) {
    if (clone(CLONE_THREAD, sem_thread, &ids[i], 2048) < 0) {
      perror();
    }
    pause(10); // Pequeño delay para mezclar la ejecución
  }

  // Espera a que los hilos terminen
  sem_wait(sem_id);
  write(1, "All threads finished\n", 21);

  // Destruye el semáforo
  if (sem_destroy(sem_id) != 0) {
    perror();
  } else {
    write(1, "Semaphore destroyed successfully\n", 33);
  }

  write(1, "--- End of Semaphore Test ---\n\n", 32);
}

void put_char(int x, int y, char c, int color) {
  if (x < 0 || x >= 80 || y < 0 || y >= 25) return;
  screen[y * 80 + x] = (color << 8) | c;
}

void print_str(int x, int y, const char* str, int color) {
  while (*str) {
      put_char(x++, y, *str++, color);
  }
}

void draw_title_screen() {
  int y = 3;

  // Limpiar pantalla
  for (int i = 0; i < 80 * 25; ++i) {
      screen[i] = (0x0 << 8) | ' ';
  }

  // 1. TÍTOL PACMAN
  print_str(20, y++, "______  ___  _____ ___  ___  ___   _   _ ", 0xE);
  print_str(20, y++, "| ___ \\/ _ \\/  __ \\|  \\/  | / _ \\ | \\ | |", 0xE);
  print_str(20, y++, "| |_/ / /_\\ \\ /  \\/| .  . |/ /_\\ \\|  \\| |", 0xE);
  print_str(20, y++, "|  __/|  _  | |    | |\\/| ||  _  || . ` |", 0xE);
  print_str(20, y++, "| |   | | | | \\__/\\| |  | || | | || |\\  |", 0xE);
  print_str(20, y++, "\\_|   \\_| |_/\\____/\\_|  |_/\\_| |_/\\_| \\_/", 0xE);

  y++; 

  // 2. Subtitol
  print_str(33, y++, "Zeos ver.", 0xE);

  y += 2; 

  // 3. ASCII ART de PACMAN
  print_str(30, y++, "  .--.", 0xF);                   
  print_str(29, y++, " / _.-' .-.  .-.  .-.  .-.  .-. ", 0xF);
  print_str(28, y++, "|  '-. '-'  '-'  '-'  '-'  '-' ", 0xF);
  print_str(29, y++, " \\__.'", 0xF);

  y += 3;

  // 4. Noms i curs
  print_str(34, y++, "Roger Cot", 0x7);
  print_str(33, y++, "Arnau Garcia", 0x7);
  print_str(34, y++, "SOA 2025", 0x7);
}

//defines pel control del joc
#define MENU_SCENE 0
#define GAME_SCENE 1
#define GAMEOVER_SCENE 2
#define WIN_SCENE 3

#define KEY_N 49
#define KEY_W 17
#define KEY_A 30
#define KEY_S 31
#define KEY_D 32
int current_scene = MENU_SCENE;
int pacman_pos_x = 30;
void game_status_thread() {

  draw_title_screen(screen);

  while (1) {
    if (current_scene == MENU_SCENE) {
      getpid();     //Si no el poso no detecta mai el canvi d'escena?
      //draw_title_screen(screen);
      //if (keyboard[KEY_N] == 1) ++current_scene;
    }
    else if (current_scene == GAME_SCENE) {
      for (int i = 0; i < 80 * 25; ++i) {
        write(1, "change scene\n", 13);
        for (int i = 0; i < 80 * 25; ++i) {
          screen[i] = ' ';
      }
      }
    }
  }
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  for (int i = 0; i < 128; ++i) keyboard[i] = 0;
  write(1, "\nHello!\n", 8);

  screen = (unsigned short*)StartScreen();
  if (screen == (void*)-1) perror("Error al acceder a la pantalla");
  else {
    for (int i = 0; i < 80 * 25; ++i) {
        screen[i] = ' ';
    }
  }

  /*
  int val = 0;
  clone(CLONE_THREAD, game_status_thread,  &val, 2048);
  */
 for (int i = 0; i < 12; ++i) {
    int id = fork();
    if (id < 0) perror();
    test_semaphores();
    if (id == 0) exit();
 }
  

  //test_simple_clone();
  //test_multiple_threads();
  //test_priority();

  // thread hace fork
  //test_clone_and_fork();

  //test_semaphores();


/*
  int pid = fork();
  if (pid < 0) perror();
  else if (pid > 0) {
     //exit desde el thread principal
    test_exit_from_main();
    //exit desde thread secundario
    test_exit_from_thread();
    draw_screen(screen);
  }
  else {
    // Hijo crea thread
    int val = 0;
    int ret = clone(CLONE_THREAD, son_thread, &val, 4096);
    if (ret < 0) perror();
    while(1) {

    }
  }
*/

while(1) {
  if (GetKeyboardState(keyboard) < 0) perror();
  if (current_scene == MENU_SCENE) {
    if (keyboard[KEY_N] == 1) ++current_scene;
  }
  pause(1000);
}
}
