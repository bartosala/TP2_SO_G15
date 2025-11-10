# Sistema Operativo - TP2

Sistema operativo educativo que implementa conceptos fundamentales como planificación de procesos, gestión de memoria, sincronización e IPC (Inter-Process Communication).

## Integrantes

- Francisco Trivelloni (64141)
- Bartolome Sala (64556)

## Índice

- [Instrucciones de Compilación y Ejecución](#instrucciones-de-compilación-y-ejecución)
- [Instrucciones de Replicación](#instrucciones-de-replicación)
  - [Comandos y Tests Disponibles](#comandos-y-tests-disponibles)
  - [Pipes y Background](#pipes-y-background)
  - [Atajos de Teclado](#atajos-de-teclado)
  - [Ejemplos de Uso](#ejemplos-de-uso)
- [Limitaciones](#limitaciones)

---

## Instrucciones de Compilación y Ejecución

### Requisitos Previos

El sistema utiliza Docker para garantizar un entorno de compilación consistente. Asegúrese de tener Docker instalado en su sistema.

### Compilación

Para compilar el sistema operativo, utilice el script de compilación:

```bash
sudo ./compile.sh [OPCIÓN]
```

**Opciones disponibles:**
- `-buddy`: Compila con el gestor de memoria Buddy Allocator (predeterminado)
- `-bitmap`: Compila con el gestor de memoria Bitmap Allocator

**Ejemplos:**

```bash
# Compilar con Buddy Allocator (predeterminado)
sudo ./compile.sh -buddy

# Compilar con Bitmap Allocator
sudo ./compile.sh -bitmap
```

### Ejecución

Una vez compilado, ejecute el sistema con:

```bash
sudo ./run.sh
```

Esto iniciará el sistema operativo en QEMU.

### Inicialización del Container Docker

Si es la primera vez que ejecuta el proyecto, necesita inicializar el contenedor Docker:

```bash
sudo ./init.sh
```

---

## Instrucciones de Replicación

### Comandos y Tests Disponibles

El sistema incluye una shell interactiva que soporta los siguientes comandos:

#### Comandos de Sistema

| Comando | Descripción | Parámetros |
|---------|-------------|------------|
| `help` | Muestra la lista de comandos disponibles | Ninguno |
| `clear` | Limpia la pantalla | Ninguno |
| `echo` | Imprime texto en pantalla | `<texto>` |
| `exit` | Cierra el sistema | Ninguno |

#### Gestión de Procesos

| Comando | Descripción | Parámetros |
|---------|-------------|------------|
| `ps` | Lista todos los procesos activos con PID, nombre, prioridad y estado | Ninguno |
| `kill` | Termina un proceso | `<pid>` |
| `block` | Bloquea un proceso | `<pid>` |
| `unblock` | Desbloquea un proceso bloqueado | `<pid>` |
| `nice` | Cambia la prioridad de un proceso (0-5) | `<pid> <prioridad>` |
| `loop` | Ejecuta un bucle infinito imprimiendo un mensaje | Ninguno |

#### Tests de Procesos

| Comando | Descripción | Parámetros |
|---------|-------------|------------|
| `test_processes` | Crea, bloquea y mata procesos aleatoriamente | `<cantidad>` (máx. 10) |
| `test_prio` | Demuestra el funcionamiento del scheduler con prioridades | Ninguno |

**Ejemplo:**
```bash
> test_processes 5
> ps
> nice 3 2
> kill 3
```

#### Gestión de Memoria

| Comando | Descripción | Parámetros |
|---------|-------------|------------|
| `memInfo` | Muestra el estado actual de la memoria del sistema | Ninguno |
| `test_mm` | Ejecuta pruebas de asignación y liberación de memoria | `<max_memory>` |
| `test_malloc_free` | Prueba las funciones malloc y free | Ninguno |

**Ejemplo:**
```bash
> memInfo
> test_mm 1024
```

#### Sincronización e IPC

| Comando | Descripción | Parámetros |
|---------|-------------|------------|
| `test_sync` | Prueba el funcionamiento de semáforos | `<flag_semaforos> <cantidad_semaforos>` |

**Nota:** El test de sincronización valida operaciones de wait y post sobre semáforos.

#### Utilidades

| Comando | Descripción | Parámetros |
|---------|-------------|------------|
| `wc` | Cuenta líneas de entrada (útil con pipes) | Ninguno |
| `filter` | Filtra vocales de la entrada | Ninguno |
| `cat` | Muestra el contenido de entrada | Ninguno |

---

### Pipes y Background

#### Pipes (`|`)

El sistema soporta **un único pipe por línea de comando**, permitiendo encadenar dos comandos donde la salida del primero se convierte en la entrada del segundo.

**Sintaxis:**
```bash
comando1 | comando2
```

**Limitaciones:**
- Solo se permite un pipe por línea
- No se soportan múltiples pipes encadenados (ej: `cmd1 | cmd2 | cmd3`)
- Los comandos built-in (`kill`, `block`, `unblock`) no pueden usarse con pipes
- Ambos procesos se ejecutan en foreground

**Ejemplos válidos:**
```bash
> help | wc
Cantidad de líneas: 25

> cat | filter
Hola Mundo
Hl Mnd

> loop | wc
# Cuenta las líneas generadas por loop
```

#### Procesos en Background (`&`)

Se puede ejecutar un proceso en segundo plano agregando `&` al final del comando.

**Sintaxis:**
```bash
comando &
```

**Características:**
- El proceso se ejecuta sin bloquear la shell
- No puede ser interrumpido con Ctrl+C
- Su stdin es redirigido (no lee del teclado)

**Ejemplo:**
```bash
> loop &
[Proceso 5 ejecutándose en background]
> ps
# El proceso loop aparece como READY/RUNNING
```

---

### Atajos de Teclado

| Atajo | Función |
|-------|---------|
| `Ctrl + C` | Interrumpe la ejecución del proceso en foreground actual |
| `Ctrl + D` | Envía EOF (End Of File) - cierra la entrada estándar |
| `Backspace` | Borra el último carácter ingresado |
| `Enter` | Ejecuta el comando ingresado |

**Notas importantes:**
- `Ctrl + C` no afecta procesos en background
- `Ctrl + C` mata ambos procesos si están conectados por pipe
- Algunos tests especiales (como `test_processes`) no pueden interrumpirse con `Ctrl + C`

---

### Ejemplos de Uso

#### Ejemplo 1: Gestión Básica de Procesos

```bash
> ps                    # Ver procesos activos
> loop &                # Iniciar loop en background
> ps                    # Verificar que loop está corriendo
> kill 3                # Matar el proceso con PID 3
> ps                    # Confirmar que el proceso terminó
```

#### Ejemplo 2: Modificación de Prioridades

```bash
> test_prio             # Inicia test de prioridades
# Observar cómo los procesos con mayor prioridad obtienen más tiempo de CPU
> ps                    # Ver prioridades de los procesos
> nice 4 5              # Cambiar prioridad del proceso 4 a 5 (máxima)
# Observar cómo el proceso 4 ahora ejecuta más frecuentemente
```

#### Ejemplo 3: Uso de Pipes

```bash
> help | wc
Cantidad de líneas: 25

> help | filter
# Muestra el help sin vocales

> loop | wc
# Cuenta las líneas que genera loop indefinidamente
# Presionar Ctrl+C para detener ambos procesos
```

#### Ejemplo 4: Test de Procesos

```bash
> test_processes 5
# Crea 5 procesos que ejecutan bucles infinitos
# Automáticamente los bloquea, desbloquea y mata aleatoriamente
# Observar la gestión dinámica de procesos
> ps                    # Ver el estado cambiante de los procesos
```

#### Ejemplo 5: Gestión de Memoria

```bash
> memInfo              # Ver estado inicial de memoria
Total: XXXXXX bytes
Used: XXXXXX bytes
Free: XXXXXX bytes

> test_mm 2048         # Ejecutar test de memoria con 2048 bytes máximo
# El test realiza múltiples allocations y frees

> memInfo              # Verificar estado final (debería ser similar al inicial)
```

#### Ejemplo 6: Sincronización (Test de Semáforos)

```bash
> test_sync 10         # Prueba con 10 operaciones sobre semáforos
# Verifica que las operaciones de wait/post funcionan correctamente
# Sin condiciones de carrera
```

---

### Requerimientos Faltantes o Parcialmente Implementados



✅ Gestión de procesos con scheduler round-robin con prioridades  
✅ Bloqueo y desbloqueo de procesos  
✅ Semáforos para sincronización  
✅ Pipes para IPC  
✅ Dos gestores de memoria (Buddy y Bitmap)  
✅ Procesos en background  


Dentro de la Shell interactiva, los siguientes comandos estan parcialmente implementados:

-nice
-block
-cat
-wc
-filter
-mvar

**Limitaciones conocidas:**
- Solo se soporta un pipe por línea de comando
- Máximo 100 semáforos
- Máximo 32 pipes
- Stack de 4KB por proceso
- Heap comienza en la direccion 0x600000

---

## Limitaciones

### Gestión de Memoria

#### Buddy Allocator
- **Tamaño mínimo de bloque:** 16 bytes (2^4)
- **Tamaño máximo teórico:** 2^64 bytes (limitado por memoria física disponible)
- **Heap total:** 256 MB en dirección 0x600000
- **Stack por proceso:** 4 KB

#### Bitmap Allocator
- **Tamaño de bloque fijo:** 64 bytes
- **Heap total:** 256 MB
- **Stack por proceso:** 4 KB

### Limitaciones de la Shell

- Los comandos built-in (`kill`, `block`, `unblock`) no funcionan con pipes
- Si un proceso pipeado termina inesperadamente, el otro puede quedar colgado
- Los file descriptors no se heredan del proceso padre
- Todos los comandos con pipe se ejecutan en foreground


## Arquitectura del Sistema

El sistema está estructurado en módulos claramente separados:

- **Kernel:** Scheduler, gestión de memoria, syscalls, drivers
- **Userland:** Shell, comandos, tests, librería estándar
- **Shared:** Estructuras compartidas entre kernel y userland
- **Bootloader:** Pure64 y BMFS para carga inicial

### Compilación Modular

El sistema permite seleccionar el gestor de memoria en tiempo de compilación mediante la variable `MM`:

```bash
make MM=BUDDY   # Buddy Allocator
make MM=BITMAP  # Bitmap Allocator
```