#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#define CLEAR_CMD "cls"           // Команда очистки экрана для Windows
#define SLEEP(ms) Sleep(ms)       // Функция задержки для Windows
#else
#include <unistd.h>
#define CLEAR_CMD "clear"         // Команда очистки экрана для Unix/Linux
#define SLEEP(ms) usleep(ms * 1000) // Функция задержки для Unix/Linux
#endif

#define MAX 101              // Максимальный размер поля
#define MAX_LINE 101            // Максимальная длина строки в файле

// Перечисление направлений движения
typedef enum { UP, DOWN, LEFT, RIGHT } Dir;

// Структура для хранения координат позиции
typedef struct {
    int x, y;                     // Координаты x и y
} Position;

// Структура игрового поля
typedef struct {
    int w, h;                     // Ширина и высота поля
    char grid[MAX][MAX];          // Отображаемая сетка (с динозавром)
    char base[MAX][MAX];          // Базовая сетка (без динозавра)
    int x, y;                     // Текущие координаты динозавра
    int placed;                   // Флаг размещения динозавра (0/1)
} Field;


void clearScreen() {    // Очищает экран терминала
    system(CLEAR_CMD);
}



void delay(int ms) { // Задержка выполнения программы 
    SLEEP(ms);
}



void init(Field* f, int w, int h) { // Создание игрового поля
    f->w = w; f->h = h; f->placed = 0;
    // Заполняем все клетки символом пустой клетки '_'
    for (int i = 0; i < h; i++)
        for (int j = 0; j < w; j++)
            f->base[i][j] = f->grid[i][j] = '_';
}


void update(Field* f) { //Обновляет отображаемую сетку на основе базовой и добавляет динозавра в текущую позицию
    // Копируем базовую сетку
    for (int i = 0; i < f->h; i++)
        for (int j = 0; j < f->w; j++)
            f->grid[i][j] = f->base[i][j];
    // Если динозавр размещен, отображаем его символом '#'
    if (f->placed) f->grid[f->y][f->x] = '#';
}


int setDino(Field* f, int x, int y) { // Размещаем динозавра на поле
    // Проверка выхода за границы поля
    if (x < 0 || x >= f->w || y < 0 || y >= f->h) {
        fprintf(stderr, "Ошибка: позиция динозавра выходит за границы поля\n");
        return 0;
    }
    f->x = x; f->y = y; f->placed = 1;
    update(f);
    return 1;
}


int getAdjX(Field* f, Dir d) { // Вычисляем координату x соседней клетки в заданном направлении
    if (d == LEFT) return (f->x - 1 + f->w) % f->w;  // Влево с заворотом
    if (d == RIGHT) return (f->x + 1) % f->w;        // Вправо с заворотом
    return f->x;                                     // Без изменения
}


int getAdjY(Field* f, Dir d) { // Вычисляем координату y соседней клетки в заданном направлении
    if (d == UP) return (f->y - 1 + f->h) % f->h;    // Вверх с заворотом
    if (d == DOWN) return (f->y + 1) % f->h;         // Вниз с заворотом
    return f->y;                                     // Без изменения
}


int isObstacle(char c) { // Проверка, является ли клетка препятствием
    return c == '^' || c == '&' || c == '@';  // Гора, дерево или камень
}


int isEmpty(char c) { // Проверка, является ли клетка пустой или окрашенной
    return c == '_' || (c >= 'a' && c <= 'z');  // Пустая или буква a-z
}


int moveDino(Field* f, Dir d) { // Перемещает динозавра в заданном направлении
    if (!f->placed) {
        fprintf(stderr, "Ошибка: динозавар не размещён\n");
        return 0;
    }
    
    // Получаем координаты целевой клетки
    int nx = getAdjX(f, d), ny = getAdjY(f, d);
    char c = f->base[ny][nx];
    
    // Проверка на препятствие
    if (isObstacle(c)) {
        fprintf(stderr, "Осторожно: невозможно движение сквозь препятствие. Команда проигнорированна.\n");
        return 1;
    }
    // Проверка на яму
    if (c == '%') {
        fprintf(stderr, "Ошибка: Вы упали в яму! Игра окончена.\n");
        return 0;
    }
    
    // Перемещаем динозавра
    f->x = nx; f->y = ny;
    update(f);
    return 1;
}



int jumpDino(Field* f, Dir d, int steps) { // Выполняем прыжок динозавра на заданное количество клеток
    if (!f->placed) {
        fprintf(stderr, "Ошибка: Динозавр не размещён\n");
        return 0;
    }
    
    if (steps <= 0) {
        fprintf(stderr, "Ошибка: длинна прыжка не может быть отрицательной\n");
        return 0;
    }
    
    Position stopPos = {f->x, f->y};  // Позиция остановки
    int obstacleFound = 0;             // Флаг найденного препятствия
    
    // Проверяем весь путь прыжка
    for (int i = 1; i <= steps; i++) {
        int checkX, checkY;
        
        // Вычисляем позицию для проверки на i-м шаге
        switch(d) {
            case UP: 
                checkY = (f->y - i + f->h * i) % f->h;
                checkX = f->x;
                break;
            case DOWN: 
                checkY = (f->y + i) % f->h;
                checkX = f->x;
                break;
            case LEFT: 
                checkX = (f->x - i + f->w * i) % f->w;
                checkY = f->y;
                break;
            case RIGHT: 
                checkX = (f->x + i) % f->w;
                checkY = f->y;
                break;
        }
        
        // Проверяем препятствия на пути (кроме конечной позиции)
        if (i < steps && isObstacle(f->base[checkY][checkX])) {
            stopPos.x = checkX;
            stopPos.y = checkY;
            // Откатываем на одну позицию назад от препятствия
            switch(d) {
                case UP: stopPos.y = (stopPos.y + 1) % f->h; break;
                case DOWN: stopPos.y = (stopPos.y - 1 + f->h) % f->h; break;
                case LEFT: stopPos.x = (stopPos.x + 1) % f->w; break;
                case RIGHT: stopPos.x = (stopPos.x - 1 + f->w) % f->w; break;
            }
            fprintf(stderr, "Осторожно: Гора по направлению прыжка. Остановка в (%d,%d)\n", stopPos.x, stopPos.y);
            obstacleFound = 1;
            break;
        }
        
        // Проверяем яму на конечной позиции
        if (i == steps && f->base[checkY][checkX] == '%') {
            fprintf(stderr, "Ошибка: Вы упали в яму после прыжка! Игра окончена.\n");
            return 0;
        }
        
        // Запоминаем текущую позицию если препятствий нет
        if (!obstacleFound) {
            stopPos.x = checkX;
            stopPos.y = checkY;
        }
    }
    
    // Устанавливаем финальную позицию
    f->x = stopPos.x;
    f->y = stopPos.y;
    update(f);
    return 1;
}



int paint(Field* f, char color) { // Красим текущую клетку в заданный цвет
    if (!f->placed) {
        fprintf(stderr, "Ошибка: Динозавр не размещён\n");
        return 0;
    }
    
    // Проверка допустимости цвета
    if (color < 'a' || color > 'z') {
        fprintf(stderr, "Ошибка: Невозможный цвет. Цвета указываютсю: a-z\n");
        return 0;
    }
    
    f->base[f->y][f->x] = color;
    update(f);
    return 1;
}



int dig(Field* f, Dir d) { // Создает яму в соседней клетке
    if (!f->placed) {
        fprintf(stderr, "Ошибка: Динозавр не размещён\n");
        return 0;
    }
    
    int tx = getAdjX(f, d), ty = getAdjY(f, d);
    
    // Проверка копания под динозавром
    if (tx == f->x && ty == f->y) {
        fprintf(stderr, "Ошибка: Нельзя выкопать динозавра(это не потому что они вымерли)\n");
        return 0;
    }
    
    char* targetCell = &f->base[ty][tx];
    
    // Если на целевой клетке гора - засыпаем яму
    if (*targetCell == '^') {
        *targetCell = '_';
        printf("Снесли гору (%d,%d)\n", tx, ty);
    } else {
        *targetCell = '%';  // Создаем яму
        printf("Выкопали яму (%d,%d)\n", tx, ty);
    }
    
    update(f);
    return 1;
}



int mound(Field* f, Dir d) { // Создает гору в соседней клетке
    if (!f->placed) {
        fprintf(stderr, "Ошибка: Динозавр не размещён\n");
        return 0;
    }
    
    int tx = getAdjX(f, d), ty = getAdjY(f, d);
    
    // Проверка строительства под динозавром
    if (tx == f->x && ty == f->y) {
        fprintf(stderr, "Ошибка: Нельзя построить гору на динозавре(жалко)\n");
        return 0;
    }
    
    char* targetCell = &f->base[ty][tx];
    
    // Если на целевой клетке яма - засыпаем ее
    if (*targetCell == '%') {
        *targetCell = '_';
        printf("Закопали яму (%d,%d)\n", tx, ty);
    } else {
        *targetCell = '^';  // Создаем гору
        printf("Построили гору (%d,%d)\n", tx, ty);
    }
    
    update(f);
    return 1;
}



int grow(Field* f, Dir d) { // Выращиваем дерево в соседней клетке
    if (!f->placed) {
        fprintf(stderr, "Ошибка: динозавр не размещён\n");
        return 0;
    }
    
    int tx = getAdjX(f, d), ty = getAdjY(f, d);
    
    // Проверка выращивания под динозавром
    if (tx == f->x && ty == f->y) {
        fprintf(stderr, "Ошибка: нельзя вырастить дерево под динозавром(ох уж этот динозавр)\n");
        return 0;
    }
    
    // Дерево можно выращивать только на пустых клетках
    if (f->base[ty][tx] != '_') {
        fprintf(stderr, "Ошибка: дерево можно вырастить только в пустых клетках\n");
        return 0;
    }
    
    f->base[ty][tx] = '&';  // Создаем дерево
    update(f);
    printf("Вырастиили дерево (%d,%d)\n", tx, ty);
    return 1;
}



int cut(Field* f, Dir d) { //Срубаем дерево в соседней клетке
    if (!f->placed) {
        fprintf(stderr, "Ошибка: динозавр не размещён\n");
        return 0;
    }
    
    int tx = getAdjX(f, d), ty = getAdjY(f, d);
    
    // Проверка что в целевой клетке есть дерево
    if (f->base[ty][tx] != '&') {
        fprintf(stderr, "Ошибка: нет дерева которое можно спилить\n");
        return 0;
    }
    
    f->base[ty][tx] = '_';  // Удаляем дерево
    update(f);
    printf("Спилили дерево (%d,%d)\n", tx, ty);
    return 1;
}



int makeStone(Field* f, Dir d) { // Создаем камень в соседней клетке
    if (!f->placed) {
        fprintf(stderr, "Ошибка: Динозавр не размещён\n");
        return 0;
    }
    
    int tx = getAdjX(f, d), ty = getAdjY(f, d);
    
    // Проверка создания под динозавром
    if (tx == f->x && ty == f->y) {
        fprintf(stderr, "Ошибка: нельзя создать динозавра под камнем\n");
        return 0;
    }
    
    // Камень можно создавать только на пустых клетках
    if (f->base[ty][tx] != '_') {
        fprintf(stderr, "Ошибка: Камень можно создать только на пустых клетках\n");
        return 0;
    }
    
    f->base[ty][tx] = '@';  // Создаем камень
    update(f);
    printf("Камень создан (%d,%d)\n", tx, ty);
    return 1;
}



int pushStone(Field* f, Dir d) { // Перемещение камня в соседнюю клетку(толчок)
    if (!f->placed) {
        fprintf(stderr, "Ошибка: динозавр не размещён\n");
        return 0;
    }
    
    int sx = getAdjX(f, d), sy = getAdjY(f, d);
    
    // Проверка что в целевой клетке есть камень
    if (f->base[sy][sx] != '@') {
        fprintf(stderr, "Ошибка: нет камня который можно подвигать\n");
        return 0;
    }
    
    // Определяем направление движения камня
    int dx = 0, dy = 0;
    if (d == UP) dy = -1;
    else if (d == DOWN) dy = 1;
    else if (d == LEFT) dx = -1;
    else dx = 1;
    
    // Вычисляем новую позицию камня с учетом тора
    int nx = (sx + dx + f->w) % f->w;
    int ny = (sy + dy + f->h) % f->h;
    
    char* target = &f->base[ny][nx];
    
    // Проверка на препятствие на пути камня
    if (isObstacle(*target)) {
        fprintf(stderr, "Ошибка: нельзя толкать камни на препятствие\n");
        return 0;
    }
    
    // Если камень попадает в яму - засыпаем ее
    if (*target == '%') {
        *target = '_';
        printf("Заполнили яму камнем (%d,%d)\n", nx, ny);
    } else if (*target != '_') {
        // Камень можно толкать только в пустые клетки или ямы
        fprintf(stderr, "Ошибка: камень можно толкать только в пустые клетки или ямы\n");
        return 0;
    }
    
    // Перемещаем камень
    *target = '@';
    f->base[sy][sx] = '_';
    update(f);
    printf("Камень перемещён(%d,%d)\n", nx, ny);
    return 1;
}



void showField(const Field* f) { // Отображаем текущее состояние поля в консоли
    for (int i = 0; i < f->h; i++) {
        for (int j = 0; j < f->w; j++)
            printf("%c ", f->grid[i][j]);
        printf("\n");
    }
}



void printSeparator() { // Разделитель(так красивенько просто)
    printf("--------------------\n");
}



void save(const Field* f, const char* name) { // Сохраняем конечное состояние поля в файл
    FILE* file = fopen(name, "w");
    if (!file) {
        fprintf(stderr, "Ошибка: нельзя создать конечный файл\n");
        return;
    }
    // Записываем поле построчно
    for (int i = 0; i < f->h; i++) {
        for (int j = 0; j < f->w; j++)
            fputc(f->grid[i][j], file);
        fputc('\n', file);
    }
    fclose(file);
}



Dir getDir(const char* s) { // Преобразуем строку направления в значение перечисления Dir
    if (strcmp(s, "UP") == 0) return UP;
    if (strcmp(s, "DOWN") == 0) return DOWN;
    if (strcmp(s, "LEFT") == 0) return LEFT;
    if (strcmp(s, "RIGHT") == 0) return RIGHT;
    return UP;  // Значение по умолчанию
}



void upper(char* s) { // Преобразуем строку в верхний регистр
    for (int i = 0; s[i]; i++) s[i] = toupper(s[i]);
}



void clean(char* s) { // Очищает строку от комментариев и лишних пробелов
    // Удаляем комментарии (все что после //)
    char* comment = strstr(s, "//");
    if (comment) *comment = '\0';
    
    // Удаляем пробелы в конце строки
    int len = strlen(s);
    while (len > 0 && isspace(s[len-1])) {
        s[len-1] = '\0';
        len--;
    }
    
    // Удаляем пробелы в начале строки
    int start = 0;
    while (s[start] && isspace(s[start])) start++;
    if (start > 0) {
        memmove(s, s + start, len - start + 1);
    }
}



int main(int argc, char* argv[]) { // Главный босс, или просто собираем все вместе
    char* inputFile = NULL;       // Имя входного файла
    char* outputFile = NULL;      // Имя выходного файла
    double interval = 1.0;        // Интервал между шагами (секунды)
    int displayEnabled = 1;       // Флаг визуализации
    int saveEnabled = 1;          // Флаг сохранения в файл
    
    // Проверка минимального количества аргументов
    if (argc < 3) {
        fprintf(stderr, "Usage: %s input.txt output.txt [interval N] [no-display] [no-save]\n", argv[0]);
        fprintf(stderr, "Examples:\n");
        fprintf(stderr, "  %s input.txt output.txt\n", argv[0]);
        fprintf(stderr, "  %s input.txt output.txt interval 0.5\n", argv[0]);
        fprintf(stderr, "  %s input.txt output.txt no-display\n", argv[0]);
        fprintf(stderr, "  %s input.txt output.txt no-save\n", argv[0]);
        fprintf(stderr, "  %s input.txt output.txt interval 0.2 no-save\n", argv[0]);
        return 1;
    }
    
    inputFile = argv[1];
    outputFile = argv[2];
    
    // Обработка дополнительных аргументов командной строки
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "interval") == 0) {
            // Обработка интервала
            if (i + 1 < argc) {
                interval = atof(argv[++i]);
                if (interval < 0) {
                    fprintf(stderr, "Ошибка: Задержка не может быть отрицательной\n");
                    return 1;
                }
            } else {
                fprintf(stderr, "Ошибка: интервал требует значения\n");
                return 1;
            }
        } else if (strcmp(argv[i], "no-display") == 0) {
            // Отключение визуализации
            displayEnabled = 0;
        } else if (strcmp(argv[i], "no-save") == 0) {
            // Отключение сохранения
            saveEnabled = 0;
        } else {
            fprintf(stderr, "Ошибка: неизвестная опция: %s\n", argv[i]);
            return 1;
        }
    }
    
    // Вывод информации о настройках
    printf("=== MovDino Interpreter ===\n");
    printf("Входной файл: %s\n", inputFile);
    printf("Конечный файл: %s\n", outputFile);
    printf("Задержка: %.1f секунд\n", interval);
    printf("Экран: %s\n", displayEnabled ? "включен" : "отключен");
    printf("Сохранение: %s\n", saveEnabled ? "включено" : "оключено");
    printf("============================\n\n");
    
    Field f;  // Структура игрового поля
    FILE* file = fopen(inputFile, "r");
    if (!file) {
        fprintf(stderr, "Ошибка: нельзя открыть входящий файл '%s'\n", inputFile);
        return 1;
    }
    
    char line[MAX_LINE];          // Буфер для чтения строк
    int lineNumber = 0, step = 0; // Счетчики строк и шагов
    int sizeDefined = 0;          // Флаг определения размера поля
    int startDefined = 0;         // Флаг размещения динозавра
    int firstCommandFound = 0;    // Флаг первой команды
    
    // Начальная задержка перед выполнением
    if (displayEnabled && interval > 0) {
        printf("Начало через 2 секунды...\n");
        delay(2000);
    }
    
    // Основной цикл чтения и выполнения команд
    while (fgets(line, sizeof(line), file)) {
        lineNumber++;
        clean(line);  // Очистка строки от комментариев и пробелов
        if (strlen(line) == 0) continue;  // Пропуск пустых строк
        
        // Проверка левых пробелов (не допускаются)
        if (line[0] == ' ' || line[0] == '\t') {
            fprintf(stderr, "Ошибка в линии %d: левые пробелы не допускаются\n", lineNumber);
            fclose(file);
            return 1;
        }
        
        char cmd[20] = "", a1[20] = "", a2[20] = "";
        int args = sscanf(line, "%19s %19s %19s", cmd, a1, a2);
        
        if (strlen(cmd) == 0) continue;
        
        // Проверка что первая команда - SIZE
        if (!firstCommandFound) {
            upper(cmd);
            if (strcmp(cmd, "SIZE") != 0) {
                fprintf(stderr, "Ошибка в линии %d: первая комнанда должна быть - SIZE\n", lineNumber);
                fclose(file);
                return 1;
            }
            firstCommandFound = 1;
        }
        
        upper(cmd);  // Приведение команды к верхнему регистру
        
        int ok = 1;  // Флаг успешного выполнения команды
        
        // Обработка различных команд
        if (strcmp(cmd, "SIZE") == 0) {
            // Команда SIZE: создание поля
            if (sizeDefined) {
                fprintf(stderr, "Ошибка в линии %d: SIZE может использоваться только единожды\n", lineNumber);
                fclose(file);
                return 1;
            }
            int w = atoi(a1), h = atoi(a2);
            if (w > 0 && h > 0 && w < MAX && h < MAX) {
                init(&f, w, h);
                sizeDefined = 1;
                if (displayEnabled) printf("Поле инициализированно: %dx%d\n", w, h);
            } else {
                fprintf(stderr, "Ошибка в линии %d: Некорректные размеры поля \n", lineNumber);
                fclose(file);
                return 1;
            }
        }
        else if (strcmp(cmd, "START") == 0) {
            // Команда START: размещение динозавра
            if (!sizeDefined) {
                fprintf(stderr, "Ошибка в линии %d: команда SIZE должна быть задана перед командой START\n", lineNumber);
                fclose(file);
                return 1;
            }
            if (startDefined) {
                fprintf(stderr, "Ошибка в линии %d: START может быть использованна только единожды\n", lineNumber);
                fclose(file);
                return 1;
            }
            int x = atoi(a1), y = atoi(a2);
            if (setDino(&f, x, y)) {
                startDefined = 1;
                if (displayEnabled) printf("Динозавр размещён: (%d, %d)\n", x, y);
            } else {
                fclose(file);
                return 1;
            }
        }
        else if (!startDefined) {
            // Проверка что START выполнен до других команд
            fprintf(stderr, "Ошибка в линии %d: START должна быть раньше %s\n", lineNumber, cmd);
            fclose(file);
            return 1;
        }
        // Обработка остальных команд (MOVE, JUMP, PAINT, DIG, MOUND, GROW, CUT, MAKE, PUSH)
        else if (strcmp(cmd, "MOVE") == 0) {
            if (strlen(a1) == 0) {
                fprintf(stderr, "Ошибка в линии %d: MOVE требует направления\n", lineNumber);
                fclose(file);
                return 1;
            }
            ok = moveDino(&f, getDir(a1));
        }
        else if (strcmp(cmd, "JUMP") == 0) {
            if (strlen(a1) == 0 || strlen(a2) == 0) {
                fprintf(stderr, "Ошибка в линии %d: JUMP требует направления и длинну прыжка\n", lineNumber);
                fclose(file);
                return 1;
            }
            ok = jumpDino(&f, getDir(a1), atoi(a2));
        }
        else if (strcmp(cmd, "PAINT") == 0) {
            if (strlen(a1) == 0) {
                fprintf(stderr, "Ошибка в линии %d: PAINT требует цвет\n", lineNumber);
                fclose(file);
                return 1;
            }
            ok = paint(&f, a1[0]);
        }
        else if (strcmp(cmd, "DIG") == 0) {
            if (strlen(a1) == 0) {
                fprintf(stderr, "Ошибка в линии %d: DIG требует направления\n", lineNumber);
                fclose(file);
                return 1;
            }
            ok = dig(&f, getDir(a1));
        }
        else if (strcmp(cmd, "MOUND") == 0) {
            if (strlen(a1) == 0) {
                fprintf(stderr, "Ошибка в линии %d: MOUND требует направления\n", lineNumber);
                fclose(file);
                return 1;
            }
            ok = mound(&f, getDir(a1));
        }
        else if (strcmp(cmd, "GROW") == 0) {
            if (strlen(a1) == 0) {
                fprintf(stderr, "Ошибка в линии %d: GROW требует направления\n", lineNumber);
                fclose(file);
                return 1;
            }
            ok = grow(&f, getDir(a1));
        }
        else if (strcmp(cmd, "CUT") == 0) {
            if (strlen(a1) == 0) {
                fprintf(stderr, "Ошибка в линии %d: CUT требует направления\n", lineNumber);
                fclose(file);
                return 1;
            }
            ok = cut(&f, getDir(a1));
        }
        else if (strcmp(cmd, "MAKE") == 0) {
            if (strlen(a1) == 0) {
                fprintf(stderr, "Ошибка в линии %d: MAKE требует направления\n", lineNumber);
                fclose(file);
                return 1;
            }
            ok = makeStone(&f, getDir(a1));
        }
        else if (strcmp(cmd, "PUSH") == 0) {
            if (strlen(a1) == 0) {
                fprintf(stderr, "Ошибка в линии %d: PUSH требует направления\n", lineNumber);
                fclose(file);
                return 1;
            }
            ok = pushStone(&f, getDir(a1));
        }
        else {
            // Неизвестная команда
            fprintf(stderr, "Ошибка в линии %d: Неизвестная команда: %s\n", lineNumber, cmd);
            fclose(file);
            return 1;
        }
        
        // Визуализация текущего состояния
        if (displayEnabled) {
            clearScreen();
            if (step > 0) {
                printSeparator();
            }
            printf("Шаг %d: %s\n", ++step, line);
            showField(&f);
            
            // Проверка успешности выполнения команды
            if (!ok) {
                printf("\n❌ Выполнение не удалось!\n");
                fclose(file);
                return 1;
            }
            
            // Задержка перед следующим шагом
            if (interval > 0) {
                printf("\n[Следующий шаг через %.1fс...]", interval);
                fflush(stdout);
                delay((int)(interval * 1000));
            }
        } else {
            // Режим без визуализации
            step++;
            if (!ok) {
                fclose(file);
                return 1;
            }
        }
    }
    
    fclose(file);
    
    // Вывод финального отчета
    if (displayEnabled) {
        printf("\n🎉 Выполнение завершено! (%d выполненных команд)\n", step);
    }
    
    // Сохранение результата в файл
    if (saveEnabled) {
        save(&f, outputFile);
        printf("💾 Результат сохранён в: %s\n", outputFile);
    } else {
        printf("📭 Сохранение отключено - конечный файл не был создан\n");
    }
    
    return 0;
}