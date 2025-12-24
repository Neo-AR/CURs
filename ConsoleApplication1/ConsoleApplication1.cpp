#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#define MAX_VERTICES 100
#define MAX_HISTORY 100
#define FILENAME "search_history.txt"

// Структуры данных
typedef struct {
    int size;
    int matrix[MAX_VERTICES][MAX_VERTICES];
    int transposed[MAX_VERTICES][MAX_VERTICES];
} Graph;

typedef struct {
    int vertices[MAX_VERTICES];
    int count;
} Component;

typedef struct {
    char timestamp[50];
    int vertex_count;
    int component_count;
    Component components[MAX_VERTICES];
} SearchResult;

// Глобальные переменные
SearchResult history[MAX_HISTORY];
int history_count = 0;
int N = 7; // Количество вершин по умолчанию

// Функции для работы с файлами
void save_to_file(SearchResult result) {
    FILE* file = fopen(FILENAME, "a");
    if (file == NULL) {
        printf("Ошибка открытия файла!\n");
        return;
    }

    fprintf(file, "Дата и время: %s\n", result.timestamp);
    fprintf(file, "Количество вершин: %d\n", result.vertex_count);
    fprintf(file, "Количество компонент связности: %d\n", result.component_count);

    for (int i = 0; i < result.component_count; i++) {
        fprintf(file, "Компонента %d: ", i + 1);
        for (int j = 0; j < result.components[i].count; j++) {
            fprintf(file, "%d ", result.components[i].vertices[j] + 1);
        }
        fprintf(file, "\n");
    }
    fprintf(file, "----------------------------------------\n");
    fclose(file);

    printf("Результаты сохранены в файл %s\n", FILENAME);
}

void load_history() {
    FILE* file = fopen(FILENAME, "r");
    if (file == NULL) {
        history_count = 0;
        return;
    }

    char line[256];
    history_count = 0;

    while (fgets(line, sizeof(line), file) && history_count < MAX_HISTORY) {
        if (strstr(line, "Дата и время:")) {
            sscanf(line, "Дата и время: %49[^\n]", history[history_count].timestamp);
        }
        else if (strstr(line, "Количество вершин:")) {
            sscanf(line, "Количество вершин: %d", &history[history_count].vertex_count);
        }
        else if (strstr(line, "Количество компонент связности:")) {
            sscanf(line, "Количество компонент связности: %d",
                &history[history_count].component_count);
        }
        else if (strstr(line, "Компонента")) {
            int comp_index;
            char comp_data[256];
            sscanf(line, "Компонента %d: %[^\n]", &comp_index, comp_data);

            // Парсинг вершин компоненты
            char* token = strtok(comp_data, " ");
            int vertex_count = 0;
            while (token != NULL && vertex_count < MAX_VERTICES) {
                history[history_count].components[comp_index - 1].vertices[vertex_count] =
                    atoi(token) - 1;
                vertex_count++;
                token = strtok(NULL, " ");
            }
            history[history_count].components[comp_index - 1].count = vertex_count;

            // Если это последняя компонента, увеличиваем счетчик
            if (comp_index == history[history_count].component_count) {
                history_count++;
            }
        }
    }

    fclose(file);
}

void view_history() {
    system("cls");
    printf("=== ИСТОРИЯ ПОИСКА ===\n\n");

    if (history_count == 0) {
        printf("История поиска пуста.\n");
        return;
    }

    for (int i = 0; i < history_count; i++) {
        printf("Запись %d:\n", i + 1);
        printf("  Дата и время: %s\n", history[i].timestamp);
        printf("  Количество вершин: %d\n", history[i].vertex_count);
        printf("  Количество компонент: %d\n", history[i].component_count);

        for (int j = 0; j < history[i].component_count; j++) {
            printf("  Компонента %d: ", j + 1);
            for (int k = 0; k < history[i].components[j].count; k++) {
                printf("%d ", history[i].components[j].vertices[k] + 1);
            }
            printf("\n");
        }
        printf("----------------------------------------\n");
    }

    printf("\nНажмите любую клавишу для продолжения...");
    getchar();
    getchar();
}

// Функции алгоритма Косарайю
void transpose_graph(Graph* g) {
    for (int i = 0; i < g->size; i++) {
        for (int j = 0; j < g->size; j++) {
            g->transposed[j][i] = g->matrix[i][j];
        }
    }
}

void fill_order(int v, int visited[], int stack[], int* index, Graph* g) {
    visited[v] = 1;

    for (int i = 0; i < g->size; i++) {
        if (g->matrix[v][i] && !visited[i]) {
            printf("  Переход из вершины %d в вершину %d\n", v + 1, i + 1);
            fill_order(i, visited, stack, index, g);
        }
    }

    stack[++(*index)] = v;
    printf("  Вершина %d добавлена в стек\n", v + 1);
}

void DFSUtil(int v, int visited[], Component* comp, Graph* g, int use_transposed) {
    visited[v] = 1;
    comp->vertices[comp->count++] = v;

    for (int i = 0; i < g->size; i++) {
        int edge_exists = use_transposed ? g->transposed[v][i] : g->matrix[v][i];
        if (edge_exists && !visited[i]) {
            printf("  Переход из вершины %d в вершину %d\n", v + 1, i + 1);
            DFSUtil(i, visited, comp, g, use_transposed);
        }
    }
}

// Алгоритм Косарайю
void kosaraju(Graph* g, SearchResult* result, int show_steps) {
    int stack[MAX_VERTICES];
    int stack_index = -1;
    int visited[MAX_VERTICES];

    // Инициализация
    for (int i = 0; i < g->size; i++) {
        visited[i] = 0;
    }

    if (show_steps) {
        printf("\n=== ШАГ 1: Первый обход в глубину ===\n");
    }

    // Первый обход в глубину для заполнения стека
    for (int i = 0; i < g->size; i++) {
        if (!visited[i]) {
            if (show_steps) {
                printf("Начинаем обход из вершины %d:\n", i + 1);
            }
            fill_order(i, visited, stack, &stack_index, g);
        }
    }

    // Транспонирование графа
    transpose_graph(g);

    // Сброс visited
    for (int i = 0; i < g->size; i++) {
        visited[i] = 0;
    }

    if (show_steps) {
        printf("\n=== ШАГ 2: Обход транспонированного графа ===\n");
    }

    // Второй обход в глубину по порядку из стека
    result->component_count = 0;
    while (stack_index >= 0) {
        int v = stack[stack_index--];

        if (!visited[v]) {
            if (show_steps) {
                printf("\nОбход из вершины %d (корень сильной компоненты):\n", v + 1);
            }

            Component new_comp;
            new_comp.count = 0;
            DFSUtil(v, visited, &new_comp, g, 1);

            result->components[result->component_count++] = new_comp;
        }
    }

    // Заполняем информацию о результате
    result->vertex_count = g->size;
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    strftime(result->timestamp, sizeof(result->timestamp),
        "%d.%m.%Y %H:%M:%S", t);
}

// Функции для работы с графами
void generate_directed_graph(Graph* g) {
    srand(time(NULL));

    for (int i = 0; i < g->size; i++) {
        for (int j = 0; j < g->size; j++) {
            if (i == j) {
                g->matrix[i][j] = 0; // Нет петель
            }
            else if (i < j) {
                int R = rand() % 100;
                if (R > 30) {
                    g->matrix[i][j] = rand() % 2;
                    g->matrix[j][i] = 0;
                }
                else {
                    g->matrix[i][j] = rand() % 2;
                    g->matrix[j][i] = rand() % 2;
                }
            }
        }
    }
}

void generate_undirected_graph(Graph* g) {
    srand(time(NULL));

    for (int i = 0; i < g->size; i++) {
        for (int j = i; j < g->size; j++) {
            if (i == j) {
                g->matrix[i][j] = 0;
            }
            else {
                g->matrix[i][j] = g->matrix[j][i] = rand() % 2;
            }
        }
    }
}

void print_matrix(Graph* g) {
    printf("\n   ");
    for (int j = 0; j < g->size; j++) {
        printf("%4d  ", j + 1);
    }
    printf("\n\n");

    for (int i = 0; i < g->size; i++) {
        printf(" %d ", i + 1);
        for (int j = 0; j < g->size; j++) {
            printf("%4d  ", g->matrix[i][j]);
        }
        printf("\n\n");
    }
}

void print_connections(Graph* g) {
    printf("\n=== СВЯЗИ ВЕРШИН ===\n");
    for (int i = 0; i < g->size; i++) {
        printf("Вершина %d связана с: ", i + 1);
        int has_connections = 0;

        for (int j = 0; j < g->size; j++) {
            if (g->matrix[i][j]) {
                printf("%d ", j + 1);
                has_connections = 1;
            }
        }

        if (!has_connections) {
            printf("нет связей");
        }
        printf("\n");
    }
}

void print_stack(int stack[], int size, int index) {
    printf("\nТекущее состояние стека (вершины в порядке завершения):\n");
    printf("Вершина стека → ");
    for (int i = index; i >= 0; i--) {
        printf("[%d] ", stack[i] + 1);
        if (i > 0) printf("→ ");
    }
    printf("\nИндексы:       ");
    for (int i = index; i >= 0; i--) {
        printf(" %2d  ", i);
        if (i > 0) printf("   ");
    }
    printf("\n");
}

// Основные функции меню
void print_header() {
    system("cls");
    printf("===================================================\n");
    printf("          КУРСОВАЯ РАБОТА\n");
    printf("          по дисциплине \"Логика и основы алгоритмизации\"\n");
    printf("          Тема: Реализация алгоритма выделения компонент связности графа,\n");
    printf("                используя поиск в глубину\n\n");
    printf("          Студент: Тусков Арсений Андреевич\n");
    printf("          Группа: 24-ВВВ1\n");
    printf("          Преподаватель: Юрова О.В.\n");
    printf("===================================================\n\n");
}

void set_vertex_count() {
    printf("Текущее количество вершин: %d\n", N);
    printf("Введите новое количество вершин (2-%d): ", MAX_VERTICES);

    int new_count;
    scanf("%d", &new_count);

    if (new_count >= 2 && new_count <= MAX_VERTICES) {
        N = new_count;
        printf("Количество вершин изменено на %d\n", N);
    }
    else {
        printf("Некорректное значение!\n");
    }

    printf("Нажмите любую клавишу для продолжения...");
    getchar();
    getchar();
}

void process_directed_graph() {
    Graph g;
    g.size = N;

    printf("\n=== ОРИЕНТИРОВАННЫЙ ГРАФ ===\n");
    generate_directed_graph(&g);
    print_matrix(&g);
    print_connections(&g);

    int show_steps;
    printf("\nПоказывать шаги алгоритма? (1 - да, 0 - нет): ");
    scanf("%d", &show_steps);

    SearchResult result;
    kosaraju(&g, &result, show_steps);

    printf("\n=== РЕЗУЛЬТАТЫ ===\n");
    printf("Количество сильных компонент связности: %d\n", result.component_count);

    for (int i = 0; i < result.component_count; i++) {
        if (result.components[i].count > 1) {
            printf("Компонента сильной связности %d: ", i + 1);
            for (int j = 0; j < result.components[i].count; j++) {
                printf("%d ", result.components[i].vertices[j] + 1);
            }
            printf("\n");
        }
    }

    char save;
    printf("\nСохранить результаты в файл? (y/n): ");
    scanf(" %c", &save);

    if (save == 'y' || save == 'Y') {
        save_to_file(result);
        history[history_count++] = result;
    }

    printf("Нажмите любую клавишу для продолжения...");
    getchar();
    getchar();
}

void process_undirected_graph() {
    Graph g;
    g.size = N;

    printf("\n=== НЕОРИЕНТИРОВАННЫЙ ГРАФ ===\n");
    generate_undirected_graph(&g);
    print_matrix(&g);
    print_connections(&g);

    // Для неориентированного графа используем упрощенный алгоритм
    int visited[MAX_VERTICES] = { 0 };
    SearchResult result;
    result.component_count = 0;
    result.vertex_count = g.size;

    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    strftime(result.timestamp, sizeof(result.timestamp),
        "%d.%m.%Y %H:%M:%S", t);

    printf("\n=== ПОИСК КОМПОНЕНТ СВЯЗНОСТИ ===\n");

    for (int i = 0; i < g.size; i++) {
        if (!visited[i]) {
            Component comp;
            comp.count = 0;

            // DFS для неориентированного графа
            int stack[MAX_VERTICES];
            int top = -1;

            stack[++top] = i;
            visited[i] = 1;

            while (top >= 0) {
                int v = stack[top--];
                comp.vertices[comp.count++] = v;

                for (int j = 0; j < g.size; j++) {
                    if (g.matrix[v][j] && !visited[j]) {
                        visited[j] = 1;
                        stack[++top] = j;
                    }
                }
            }

            result.components[result.component_count++] = comp;
        }
    }

    printf("\n=== РЕЗУЛЬТАТЫ ===\n");
    printf("Количество компонент связности: %d\n", result.component_count);

    for (int i = 0; i < result.component_count; i++) {
        if (result.components[i].count > 1) {
            printf("Компонента связности %d: ", i + 1);
            for (int j = 0; j < result.components[i].count; j++) {
                printf("%d ", result.components[i].vertices[j] + 1);
            }
            printf("\n");
        }
    }

    char save;
    printf("\nСохранить результаты в файл? (y/n): ");
    scanf(" %c", &save);

    if (save == 'y' || save == 'Y') {
        save_to_file(result);
        history[history_count++] = result;
    }

    printf("Нажмите любую клавишу для продолжения...");
    getchar();
    getchar();
}

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    load_history();

    while (1) {
        print_header();

        printf("МЕНЮ:\n");
        printf("1. Установить количество вершин (текущее: %d)\n", N);
        printf("2. Ориентированный граф (алгоритм Косарайю)\n");
        printf("3. Неориентированный граф\n");
        printf("4. Просмотреть историю поиска\n");
        printf("5. Выход\n");
        printf("\nВыберите пункт меню: ");

        int choice;
        scanf("%d", &choice);

        switch (choice) {
        case 1:
            set_vertex_count();
            break;
        case 2:
            process_directed_graph();
            break;
        case 3:
            process_undirected_graph();
            break;
        case 4:
            view_history();
            break;
        case 5:
            printf("Выход из программы...\n");
            return 0;
        default:
            printf("Некорректный выбор!\n");
            printf("Нажмите любую клавишу для продолжения...");
            getchar();
            getchar();
            break;
        }
    }

    return 0;
}