/* VISAN Rares-Stefan - 312CC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// lista dublu inlantuita
typedef struct Node {
    char elem;
    struct Node *prev;
    struct Node *next;
} Node, *DList;

// banda magica
typedef struct MagicTape {
    DList finger;
    DList list;
} *MagicTape;

// lista simplu inlantuita
typedef struct Cell {
    char elem[30];
    struct Cell *next;
} Cell, *List;

// coada de comenzi
typedef struct Queue {
    List front, rear;
} Queue;

// stiva undo/redo
typedef struct Stack {
    DList elem;
    struct Stack *next;
} *Stack;

// initializare banda magica
MagicTape init_tape(MagicTape tape) {
    tape->list = (DList)calloc(1, sizeof(Node));
    tape->list->prev = NULL;
    // introduc primul element #
    DList new = (DList)calloc(1, sizeof(Node));
    new->elem = '#';
    new->next = NULL;
    new->prev = tape->list;
    tape->list->next = new;
    // setez pozitia degetului
    tape->finger = tape->list->next;
    return tape;
}
// initializare coada
Queue init_queue(Queue q) {
    q.front = NULL;
    q.rear = NULL;
    return q;
}
// adaugare comanda in coada
void enqueue(Queue *q, char *s) {
    List new = (List)calloc(1, sizeof(Cell));
    strcpy(new->elem, s);
    new->next = NULL;
    if (q->front == NULL) {
        q->front = new;
        q->rear = new;
    } else {
        q->rear->next = new;
        q->rear = new;
    }
}
// eliminare comanda din coada
char *dequeue(Queue *q) {
    char *s;
    s = q->front->elem;
    if (q->front == q->rear) {
        q->front = NULL;
    } else {
        q->front = q->front->next;
    }
    return s;
}
// adaugare element in stiva
void push(Stack s, DList x) {
    Stack new = (Stack)calloc(1, sizeof(struct Stack));
    new->elem = x;
    new->next = s->next;
    s->next = new;
}
// eliminare element stiva
DList pop(Stack s) {
    Stack p = s->next;
    if (p != NULL) {
        DList x = p->elem;
        s->next = p->next;
        free(p);
        return x;
    }
    return 0;
}
// comanda MOVE_LEFT
void move_left(MagicTape tape, Stack stack_undo) {
    if (tape->finger->prev->prev != NULL) {
        // adaug poz deget in stiva undo
        push(stack_undo, tape->finger);
        // mut degetul
        tape->finger = tape->finger->prev;
    }
}
// comanda MOVE_RIGHT
void move_right(MagicTape tape, Stack stack_undo) {
    // adaug poz deget in stiva undo
    push(stack_undo, tape->finger);
    // cazul cand sunt pe ultimul element din banda
    if (tape->finger->next == NULL) {
        DList p = tape->list;
        // adaug element nou
        DList new = (DList)calloc(1, sizeof(Node));
        new->elem = '#';
        new->next = NULL;
        while (p->next != NULL) {
            p = p->next;
        }
        new->prev = p;
        p->next = new;
        // mut degetul
        tape->finger = new;
    } else {
        // mut degetul
        tape->finger = tape->finger->next;
    }
}
// comanda MOVE_LEFT_CHAR <C>
void move_left_char(MagicTape tape, char x, FILE *fout) {
    int ok = 0;
    DList p = tape->finger;
    while (p->prev != NULL) {
        // gasesc element
        if (p->elem == x) {
            tape->finger = p;
            ok = 1;
            break;
        }
        p = p->prev;
    }
    // nu gasesc element
    if (ok == 0) {
        fprintf(fout, "ERROR\n");
    }
}
// comanda MOVE_RIGHT_CHAR <C>
void move_right_char(MagicTape tape, char x) {
    int ok = 0;
    // caz: degetul este deja pe elem. cautat
    if (tape->finger->elem == x) {
        ok = 1;
    } else {
        while (tape->finger->next != NULL) {
            tape->finger = tape->finger->next;
            // gasesc elem.
            if (tape->finger->elem == x) {
                ok = 1;
                break;
            }
        }
    }
    // nu gasesc elem. si adaug unul nou
    if (ok == 0) {
        DList new = (DList)calloc(1, sizeof(Node));
        new->elem = '#';
        new->next = NULL;
        new->prev = tape->finger;
        tape->finger->next = new;
        // mut degetul
        tape->finger = new;
    }
}
// comanda WRITE <C>
void write(MagicTape tape, char x) { 
    tape->finger->elem = x; 
}
// comanda INSERT_LEFT <C>
void insert_left(MagicTape tape, char x, FILE *fout) {
    // caz: degetul este pe primul element
    if (tape->finger->prev->prev == NULL) {
        fprintf(fout, "ERROR\n");
    } else {
        // adaug element
        DList new = (DList)calloc(1, sizeof(Node));
        new->elem = x;
        new->prev = tape->finger->prev;
        new->next = tape->finger;
        tape->finger->prev->next = new;
        // mut degetul
        tape->finger = new;
    }
}
// comanda INSERT_RIGHT <C>
void insert_right(MagicTape tape, char x) {
    // adaug element
    DList new = (DList)calloc(1, sizeof(Node));
    new->elem = x;
    new->prev = tape->finger;
    new->next = tape->finger->next;
    tape->finger->next = new;
    // mut degetul
    tape->finger = new;
}
// comanda SHOW_CURRENT
void show_current(MagicTape tape, FILE *fout) {
    fprintf(fout, "%c\n", tape->finger->elem);
}
// comanda SHOW
void show(MagicTape tape, FILE *fout) {
    DList p = tape->list->next;
    while (p != NULL) {
        if (p == tape->finger) {
            fprintf(fout, "|");
            fprintf(fout, "%c", p->elem);
            fprintf(fout, "|");
        } else {
            fprintf(fout, "%c", p->elem);
        }
        p = p->next;
    }
    fprintf(fout, "\n");
}
// comanda EXECUTE
void execute(Queue *q, MagicTape tape, FILE *fout, Stack stack_undo) {
    int x;
    char *command;
    // extrag comanda din coada
    command = dequeue(q);
    // verific daca este o comanda cu <C>
    if (strchr(command, ' ')) {
        // memorez caracterul <C> din comanda
        x = command[strlen(command) - 1];
        char aux[30] = {0};
        // separ comanda de <C>
        strncpy(aux, command, strlen(command) - 2);
        strcpy(command, aux);
    }
    // apelez functia ceruta
    if (strcmp(command, "MOVE_LEFT") == 0) {
        move_left(tape, stack_undo);
    } else if (strcmp(command, "MOVE_RIGHT") == 0) {
        move_right(tape, stack_undo);
    } else if (strcmp(command, "MOVE_LEFT_CHAR") == 0) {
        move_left_char(tape, x, fout);
    } else if (strcmp(command, "MOVE_RIGHT_CHAR") == 0) {
        move_right_char(tape, x);
    } else if (strcmp(command, "WRITE") == 0) {
        write(tape, x);
    } else if (strcmp(command, "INSERT_LEFT") == 0) {
        insert_left(tape, x, fout);
    } else if (strcmp(command, "INSERT_RIGHT") == 0) {
        insert_right(tape, x);
    }
    free(command);
}
// comanda UNDO
void undo(MagicTape tape, Stack stack_undo, Stack stack_redo) {
    DList p = pop(stack_undo);
    push(stack_redo, tape->finger);
    // mut degetul pe pozitia anterioara
    tape->finger = p;
}
// comanda REDO
void redo(MagicTape tape, Stack stack_undo, Stack stack_redo) {
    DList p = pop(stack_redo);
    push(stack_undo, tape->finger);
    // mut degetul pe pozitia anterioara
    tape->finger = p;
}
// functie eliberare memorie lista dublu inlantuita
void free_dlist(DList list) {
    DList p = list, next;
    while (p != NULL) {
        next = p->next;
        free(p);
        p = next;
    }
}
// functie eliberare memorie stiva
void free_stack(Stack stack) {
    Stack s = stack, next;
    while (s != NULL) {
        next = s->next;
        free(s);
        s = next;
    }
}
// functie eliberare memorie coada
void free_queue(Queue *queue) {
    List p = queue->front, next;
    while (p != NULL) {
        next = p->next;
        free(p);
        p = next;
    }
}

int main() {
    MagicTape tape = (MagicTape)calloc(1, sizeof(struct MagicTape));
    tape = init_tape(tape);
    Queue q = init_queue(q);
    Stack stack_undo = (Stack)calloc(1, sizeof(struct Stack));
    Stack stack_redo = (Stack)calloc(1, sizeof(struct Stack));
    char command[30];
    int nr_command, i;
    FILE *fin, *fout;
    fout = fopen("tema1.out", "w");
    fin = fopen("tema1.in", "r");
    // citesc numarul de comenzi
    fscanf(fin, "%d", &nr_command);
    fgetc(fin);
    for (i = 0; i < nr_command; i++) {
        // citesc comanda
        fgets(command, 30, fin);
        command[strlen(command) - 1] = '\0';
        // apelez functia pentru comanda citita
        if (strcmp(command, "EXECUTE") == 0) {
            execute(&q, tape, fout, stack_undo);
        } else if (strcmp(command, "UNDO") == 0) {
            undo(tape, stack_undo, stack_redo);
        } else if (strcmp(command, "REDO") == 0) {
            redo(tape, stack_undo, stack_redo);
        } else if (strcmp(command, "SHOW_CURRENT") == 0) {
            show_current(tape, fout);
        } else if (strcmp(command, "SHOW") == 0) {
            show(tape, fout);
        } else {
            // adaug comanda in coada
            enqueue(&q, command);
        }
    }
    // eliberez memoria alocata
    free_dlist(tape->list);
    free_stack(stack_undo);
    free_stack(stack_redo);
    free_queue(&q);
    free(tape);
    // inchid fisierele
    fclose(fin);
    fclose(fout);
    return 0;
}