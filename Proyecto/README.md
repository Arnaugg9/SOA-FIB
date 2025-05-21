# 🟡 Pac-Man zeos version

Este es un juego de **Pac-Man**, implementado usando el sistema operativo **zeos**.

---

## 🎮 Cómo se juega

- Usa las teclas `W`, `A`, `S`, `D` para mover a Pac-Man.
- El objetivo es recoger todos los puntos (`o`) del mapa mientras evitas a los fantasmas.
- Si recoges un punto especial (`$`), los fantasmas se vuelven vulnerables durante un tiempo y puedes comértelos.
- Si te come un fantasma no vulnerable, pierdes.
- Si recoges todos los puntos, ganas.

---

## 🧱 Elementos del mapa

| Símbolo | Significado                  |
|--------:|------------------------------|
| `#`     | Pared (azul)                 |
| `o`     | Punto normal (blanco)        |
| `$`     | Power-up de vulnerabilidad   |
| `@`     | Pac-Man                      |
| `X`     | Fantasma                     |

---

## 👻 Fantasmas

- Cada fantasma inicia en una esquina diferente del mapa.
- Siguen los comportamientos clásicos:
  - **Blinky** persigue directamente a Pac-Man.
  - **Pinky** persigue la posición 2 casillas delante de la dirección de movimiento de Pac-Man
  - **Inky** Se mueve usando como referencia un vector entre la posición de Pac-Man y Blinky
  - **Clyde** Persigue a PAc-Man hasta estar a 5 casillas, cuando huye hacia su esquina
- Al recoger un `$`, los fantasmas huyen a sus respectivas esquinas.
- Si los comes en ese estado, desaparecen un tiempo y luego reaparecen en su esquina.

---

## 🧵 Hilos y sincronización

- El juego tiene dos hilos principales:
  - `game_thread`: gestiona toda la lógica del juego (escenas, movimientos, colisiones, etc.).
  - `main thread`: gestiona la entrada por teclado mediante `GetKeyboardState()` y se bloquea con `pause()` entre lecturas.

- Ambos hilos usan un **semáforo (`sem_game`)** para proteger los accesos a los datos compartidos (como la posición de Pac-Man).

---

## ⏱️ FPS

- Se muestran los FPS actuales del juego en la **esquina superior izquierda**.
- Se calculan a partir de los ticks del sistema (18 ticks por segundo).

---

## 📺 Escenas del juego

1. **Pantalla de inicio**  
   Al lanzar el juego, se muestra una pantalla de título.  
   Pulsa cualquier tecla para comenzar.

2. **Pantalla de juego**  
   Se juega el nivel completo con los controles mencionados.

3. **Pantalla de victoria o derrota**  
   Si ganas o mueres, se muestra la pantalla correspondiente.  
   Pulsa cualquier tecla para volver a la pantalla de inicio.

---
