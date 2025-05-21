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
    perror();
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










/////////// CODI PEL JOC //////////////
//defines pel control del joc
#define NUM_COL 80

#define KEY_N 49
#define KEY_W 17
#define KEY_A 30
#define KEY_S 31
#define KEY_D 32

#define COLOR_WHITE 0x7
#define COLOR_DARK_BLUE 0x1
#define COLOR_LIGHT_BLUE 0x9
#define COLOR_RED 0xC
#define COLOR_CYAN 0x3
#define COLOR_GREEN 0xA
#define COLOR_YELLOW 0xE
#define COLOR_DARK_YELLOW 0x6
#define COLOR_WHITE 0x7

#define PACMAN_INIT_POS_X 40
#define PACMAN_INIT_POS_Y 14

#define FIXED_UPDATE_TICKS 80

//Structs d'objectes
enum scenes {MENU_SCENE, GAME_SCENE, GAMEOVER_SCENE, WIN_SCENE};
enum directions {NONE, LEFT, RIGHT, UP, DOWN};
enum ghost_states {STOP, MOVE, VULNERABLE, DEAD};

//Struct pel pacman i pels ghosts
typedef struct {
  //Caracteristiques comunes
  int pos_x, pos_y;
  int dir;
  char character;
  int color;
  //Caracteristiques úniques dels ghosts
  int state;
  int respawn_timer;
} Entity;

//Variables
//char keyboard[128];         --> Definits a dalt, s'utilitzen en altres tests
//unsigned short* screen;
int sem_game;

//Variables d'entitats del joc
Entity pacman;
Entity ghosts[4];

//Variables GameState
int current_scene;
int points;
char map[24][80] = {
  "                                                                                ",
  "                                                                                ",
  "                                                                                ",
  "            #######################################################             ",
  "            #######################################################             ",
  "            ### o o o o o o o o o o o ### o o o o o o o o o o o ###             ",
  "            ### o ####### o ####### o ### o ####### o ####### o ###             ",
  "            ### $ ####### o ####### o ### o ####### o ####### $ ###             ",
  "            ### o o o o o o o o o o o o o o o o o o o o o o o o ###             ",
  "            ### o ####### o ##### o o o o o o ##### o ####### o ###             ",
  "            ### o ####### o ##### o o o o o o ##### o ####### o ###             ",
  "            ### o o o o o o o o o o o o o o o o o o o o o o o o ###             ",
  "            ############# o ####### o ####### o ####### o #########             ",
  "            ############# o ####### o ####### o ####### o #########             ",
  "            ### o o o o o o o o o o o o o o o o o o o o o o o o ###             ",
  "            ### $ ######### o ######### o ####### o ######### $ ###             ",
  "            ### o ######### o ######### o ####### o ######### o ###             ",
  "            ### o o o o o o o o o o o o o o o o o o o o o o o o ###             ",
  "            #######################################################             ",
  "            #######################################################             ",
  "                                                                                ",
  "                                                                                ",
  "                                                                                ",
  "                                                                                "
};
unsigned short back_buffer[25*80];

//Variables per controlar velocitat joc
int ticks_passed;
int ticks_last_time;


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
  print_str(20, y++, "______  ___  _____ ___  ___  ___   _   _ ", COLOR_YELLOW);
  print_str(20, y++, "| ___ \\/ _ \\/  __ \\|  \\/  | / _ \\ | \\ | |", COLOR_YELLOW);
  print_str(20, y++, "| |_/ / /_\\ \\ /  \\/| .  . |/ /_\\ \\|  \\| |", COLOR_YELLOW);
  print_str(20, y++, "|  __/|  _  | |    | |\\/| ||  _  || . ` |", COLOR_YELLOW);
  print_str(20, y++, "| |   | | | | \\__/\\| |  | || | | || |\\  |", COLOR_YELLOW);
  print_str(20, y++, "\\_|   \\_| |_/\\____/\\_|  |_/\\_| |_/\\_| \\_/", COLOR_YELLOW);

  y++; 

  // 2. Subtitol
  print_str(33, y++, "Zeos ver.", COLOR_YELLOW);

  y += 2; 

  // 3. ASCII ART de PACMAN
  print_str(24, y++, "   .--.", COLOR_WHITE);                   
  print_str(24, y++, "  / _.-' .-.  .-.  .-.  .-.  .-. ", COLOR_WHITE);
  print_str(24, y++, " |  '-.  '-'  '-'  '-'  '-'  '-' ", COLOR_WHITE);
  print_str(24, y++, "  \\__.'", COLOR_WHITE);

  y += 3;

  // 4. Noms i curs
  print_str(34, y++, "Roger Cot", COLOR_WHITE);
  print_str(33, y++, "Arnau Garcia", COLOR_WHITE);
  print_str(34, y++, "SOA 2025", COLOR_WHITE);
}

void draw_win_screen() {
  int y = 4;

  // Limpiar pantalla
  for (int i = 0; i < 80 * 25; ++i) {
      screen[i] = (0x0 << 8) | ' ';
  }

  print_str(2, y++, " _____                             _         _       _   _                 ", COLOR_YELLOW);
  print_str(2, y++, "/  __ \\                           | |       | |     | | (_)                ", COLOR_YELLOW);
  print_str(2, y++, "| /  \\/ ___  _ __   __ _ _ __ __ _| |_ _   _| | __ _| |_ _  ___  _ __  ___ ", COLOR_YELLOW);
  print_str(2, y++, "| |    / _ \\| '_ \\ / _` | '__/ _` | __| | | | |/ _` | __| |/ _ \\| '_ \\/ __|", COLOR_YELLOW);
  print_str(2, y++, "| \\__/\\ (_) | | | | (_| | | | (_| | |_| |_| | | (_| | |_| | (_) | | | \\__ \\", COLOR_YELLOW);
  print_str(2, y++, " \\____/\\___/|_| |_|\\__, |_|  \\__,_|\\__|\\__,_|_|\\__,_|\\__|_|\\___/|_| |_|___/", COLOR_YELLOW);
  print_str(2, y++, "                    __/ |                                                  ", COLOR_YELLOW);
  print_str(2, y++, "                   |___/                                                   ", COLOR_YELLOW);

  y++; 
  y++;

  print_str(20, y++, "__   __            _    _             ", COLOR_LIGHT_BLUE);
  print_str(20, y++, "\\ \\ / /           | |  | |            ", COLOR_LIGHT_BLUE);
  print_str(20, y++, " \\ V /___  _   _  | |  | | ___  _ __  ", COLOR_LIGHT_BLUE);  
  print_str(20, y++, "  \\ // _ \\| | | | | |/\\| |/ _ \\| '_ \\ ", COLOR_LIGHT_BLUE);  
  print_str(20, y++, "  | | (_) | |_| | \\  /\\  / (_) | | | |", COLOR_LIGHT_BLUE);  
  print_str(20, y++, "  \\_/\\___/ \\__,_|  \\/  \\/ \\___/|_| |_|", COLOR_LIGHT_BLUE);  
}

void draw_game_over_screen() {
  int y = 4;

  // Limpiar pantalla
  for (int i = 0; i < 80 * 25; ++i) {
      screen[i] = (0x0 << 8) | ' ';
  }

  print_str(14, y++, " _____                        _____                ", COLOR_RED);
  print_str(14, y++, "|  __ \\                      |  _  |               ", COLOR_RED);
  print_str(14, y++, "| |  \\/ __ _ _ __ ___   ___  | | | |_   _____ _ __ ", COLOR_RED);
  print_str(14, y++, "| | __ / _` | '_ ` _ \\ / _ \\ | | | \\ \\ / / _ \\ '__|", COLOR_RED);
  print_str(14, y++, "| |_\\ \\ (_| | | | | | |  __/ \\ \\_/ /\\ V /  __/ |   ", COLOR_RED);
  print_str(14, y++, " \\____/\\__,_|_| |_| |_|\\___|  \\___/  \\_/ \\___|_|   ", COLOR_RED);


  y++; 
  y++;

  print_str(16, y++, " _____             ___              _       ", COLOR_WHITE);
  print_str(16, y++, "|_   _|           / _ \\            (_)      ", COLOR_WHITE);
  print_str(16, y++, "  | |_ __ _   _  / /_\\ \\ __ _  __ _ _ _ __  ", COLOR_WHITE);
  print_str(16, y++, "  | | '__| | | | |  _  |/ _` |/ _` | | '_ \\ ", COLOR_WHITE);
  print_str(16, y++, "  | | |  | |_| | | | | | (_| | (_| | | | | |", COLOR_WHITE);
  print_str(16, y++, "  \\_/_|   \\__, | \\_| |_/\\__, |\\__,_|_|_| |_|", COLOR_WHITE);
  print_str(16, y++, "           __/ |         __/ |              ", COLOR_WHITE);
  print_str(16, y++, "          |___/         |___/               ", COLOR_WHITE);

}

//Comprova si la posició es vàlida, 1 si ho es, 0 altrament (Les posicions no valides son les pareds)
int  can_move(int x, int y) {
  return map[y][x] != '#';
}

void move_entity(Entity *entity) {
  switch (entity->dir)
  {
  case LEFT:
    if (can_move(entity->pos_x - 2, entity->pos_y)) {
      entity->pos_x -= 2;
    }
    break;

  case RIGHT:
    if (can_move(entity->pos_x + 2, entity->pos_y)) {
      entity->pos_x += 2;
    }
    break;
  
  case UP:
    if (can_move(entity->pos_x, entity->pos_y - 1)) {
      entity->pos_y -= 1;
    }
    break;

  case DOWN:
    if (can_move(entity->pos_x, entity->pos_y + 1)) {
      entity->pos_y += 1;
    }
    break;

  default:
    break;
  }
}

void updateGame() {
  //Movem al Pacman
  move_entity(&pacman);

  //Comprobar si hi ha punts en la nova posicio del pacman
  if (map[pacman.pos_y][pacman.pos_x] == 'o') {
    ++points;
    map[pacman.pos_y][pacman.pos_x] = ' ';
  }
  else if (map[pacman.pos_y][pacman.pos_x] == '$') {
    ++points;
    map[pacman.pos_y][pacman.pos_x] = ' ';
    //activate_powerup();
  }

  //Movem als fantasmes


  if (points >= 168) current_scene = WIN_SCENE;
}

void renderGame() {
  //Render map -> En el buffer auxiliar
  int idx = NUM_COL;
  int color = COLOR_WHITE;
  for (int i = 0; i < 24; ++i) {
    for (int j = 0; j < 80; ++j) {
      if (map[i][j] == '#') color = COLOR_DARK_BLUE;  //Blau pels murs
      else if (map[i][j] == '$') color = COLOR_CYAN;  //Cian per powerup
      else color = COLOR_WHITE;                       //Blanc resta
      back_buffer[idx++] = color << 8 | map[i][j];  
    }
  }

  //Render Entities -> En el buffer auxiliar
  idx = NUM_COL + pacman.pos_y * NUM_COL + pacman.pos_x;
  back_buffer[idx] = pacman.color << 8 | pacman.character;
  
  for (int i = 0; i < 4; ++i) {
    idx = NUM_COL + ghosts[i].pos_y * NUM_COL + ghosts[i].pos_x;
    back_buffer[idx] = ghosts[i].color <<8 | ghosts[i].character;
  }


  //Draw the auxiliar buffer
  for (int i = 0; i < 24*80; ++i) {
    screen[i] = back_buffer[i];
  }
}

void init_game() {
  current_scene = MENU_SCENE;

  draw_title_screen();

  //Block no está implementado -> Seria útil
  while(current_scene == MENU_SCENE) {
    getpid();
  }

  ticks_passed = 0;
  ticks_last_time = gettime();

  //Borrem pantalla
  for (int i = 0; i < 80 * 25; ++i) {
    screen[i] = ' ';
  }

  //Inicialitzem Entities
    //Pacman
  pacman.character = '@';
  pacman.color = COLOR_YELLOW;
  pacman.dir = NONE;
  pacman.pos_x = PACMAN_INIT_POS_X;
  pacman.pos_y = PACMAN_INIT_POS_Y;

    //Ghosts
  for (int i = 0; i < 4; ++i) {
    ghosts[i].character = 'X';
    ghosts[i].state = STOP;
    ghosts[i].respawn_timer = 0;
    ghosts[i].dir = NONE;
  }

  ghosts[0].color = COLOR_RED;
  ghosts[0].pos_x = 26;
  ghosts[0].pos_y = 5;

  ghosts[1].color = COLOR_LIGHT_BLUE;
  ghosts[1].pos_x = 52;
  ghosts[1].pos_y = 5;

  ghosts[2].color = COLOR_DARK_YELLOW;
  ghosts[2].pos_x = 16;
  ghosts[2].pos_y = 17;

  ghosts[3].color = COLOR_GREEN;
  ghosts[3].pos_x = 62;
  ghosts[3].pos_y = 17;
  //Inicialitzem GameState
  points = 0;

  while (current_scene == GAME_SCENE) {
    int current_time = gettime();
    ticks_passed += current_time - ticks_last_time;
    ticks_last_time = current_time;

    if (ticks_passed >= FIXED_UPDATE_TICKS) {
      ticks_passed = 0;
      sem_wait(sem_game);
      updateGame();
      sem_post(sem_game);
    }

    sem_wait(sem_game);
    renderGame();
    sem_post(sem_game);
    //pause(100);
  }

  if (current_scene == WIN_SCENE) {  
    draw_win_screen();
  }

  while (1) {;}
}

void read_keyboard_thread() {
  while(1) {
    sem_wait(sem_game);

    if (GetKeyboardState(keyboard) < 0) perror();

    if (current_scene == MENU_SCENE) {
      for (int i = 0; i < 128; ++i) {
        if (keyboard[i] == 1) {
          ++current_scene;
          break;
        }
      }
    }
    else if (current_scene == GAME_SCENE) {
      if (keyboard[KEY_W] == 1) {
        pacman.dir = UP;
      }
      else if (keyboard[KEY_S] == 1) {
        pacman.dir = DOWN;
      }
      else if (keyboard[KEY_A] == 1) {
        pacman.dir = LEFT;
      }
      else if (keyboard[KEY_D] == 1) {
        pacman.dir = RIGHT;
      }
    }
    else if (current_scene == GAMEOVER_SCENE || current_scene == WIN_SCENE) {
      for (int i = 0; i < 128; ++i) {
        if (keyboard[i] == 1) {
          current_scene = MENU_SCENE;
          break;
        }
      }
    }
      

    sem_post(sem_game);
    pause(1000);
  }
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  for (int i = 0; i < 128; ++i) keyboard[i] = 0;
  write(1, "\nHello!\n", 8);
/*
  screen = (unsigned short*)StartScreen();
  if (screen == (void*)-1) perror("Error al acceder a la pantalla");
  else {
    for (int i = 0; i < 80 * 25; ++i) {
        screen[i] = ' ';
    }
  }

  sem_game = sem_init(1); 
  if (sem_game < 0) {
      write(1, "Error initializing semaphore\n", 30);
      return -1;
  }
  
  //Creamos el thread auxiliar que controlará el juego
  int tid1 = clone(CLONE_THREAD, init_game, 0, 1024);
  if (tid1 < 0) {
      perror();
      sem_destroy(sem_game);
      return -1;
  }

  //Thread principal lee el teclado();
  read_keyboard_thread();
*/
  
    for (int i = 0; i < 12; ++i) {
        int id = fork();
        if (id < 0) perror();
        if (id > 0) test_semaphores();
        if (id == 0) exit();
    }
    write(1, "\nFINISHED\n", 10);
    while(1);

  //test_simple_clone();
  //test_multiple_threads();
  //test_priority();

  // thread hace fork
  //test_clone_and_fork();


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

}
