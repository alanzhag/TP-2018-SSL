#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define YES 'S'

struct _Buffer;
struct _Automata;

bool askForAnotherLexicalCheck();

bool notFDT(struct _Buffer buffer);

char *requestStringInputToCheck();

typedef enum {
    INITIAL, FINAL, NONE
} STATE_PROPERTY;

typedef struct _State {
    int id;
    STATE_PROPERTY stateProperty;
} State;

typedef void (*SetActualStateToInitialState)(struct _Automata *self);

typedef struct _Automata {
    State actualState;
    SetActualStateToInitialState setActualStateToInitialState;
} Automata;

void Automata__setActualStateToInitialState(Automata *self) {
    //TODO: implementar la tabla. Necesito poder decirle que solito vaya al estado inicial (de verdad / no hardcoded).
    State initialState = {.id = 0, .stateProperty = INITIAL};
    self->actualState = initialState;
}

typedef char (*FetchNextCharacter)(struct _Buffer *self);

typedef char (*IsFDT)(struct _Buffer *self);

typedef void (*Clean)(struct _Buffer *self);

typedef struct _Buffer {
    char *input;
    FetchNextCharacter fetchNextCharacter;
    IsFDT isFDT;
    Clean clean;
} Buffer;

char Buffer__fetchNextCharacter(Buffer *self) {
    char *wholeBufferInput = self->input;
    char nextCharacter = wholeBufferInput[0];
    size_t sizeOfInput = strlen(wholeBufferInput);
    char *inputWithoutFirstCharacter = malloc(sizeOfInput - 1);
    memcpy(inputWithoutFirstCharacter, wholeBufferInput + 1, sizeOfInput - 1);
    free(self->input);
    self->input = inputWithoutFirstCharacter;
    return nextCharacter;
}

char Buffer__isFDT(Buffer *self) {
    char *c = self->input;
    return ((c != NULL) && (c[0] == '\0'));
}

void Buffer__clean(Buffer *self) {
    free(self->input);
}

/*
 * Algoritmo 3
 *
 * - Intenta leer primer carácter del texto (porque el texto puede estar vacío)
 * - Mientras no sea fdt, repetir:
 * (1) Estado actual del autómata: estado inicial
 * (2) Mientras no sea un estado final y no sea el estado FDT, repetir:
 * (2.1) Determinar el nuevo estado actual
 * (2.2) Actualizar el carácter a analizar
 * (3) Si el estado es final, la cadena procesada es una constante entera;
 * caso contrario, la cadena no pertenece al lenguaje.
 *
 * ER: [01]\.[0-9]{2}|[01]*B Centinela: %
 * //TODO: Preguntar si pueden ingresar caracters que no forman parte del alfabeto. ¿A que estado voy?
 * +-----+-----------+-----+-----+---+
 * | AFD | . (punto) | 0-1 | 2-9 | B |
 * +-----+-----------+-----+-----+---+
 * | 0-  |           |  1  |     | 5 |
 * | 1   |     2     |  4  |     | 5 |
 * | 2   |           |  3  |  3  |   |
 * | 3   |           |  5  |  5  |   |
 * | 4   |           |  4  |     | 5 |
 * | 5+  |           |     |     |   |
 * +-----+-----------+-----+-----+---+
 *
 */

int main() {
    bool lexicalCheckRequired;
    Buffer buffer = {.fetchNextCharacter = Buffer__fetchNextCharacter,
            .isFDT = Buffer__isFDT,
            .clean = Buffer__clean};
    Automata automata = {.setActualStateToInitialState = Automata__setActualStateToInitialState};

    do {

        buffer.input = requestStringInputToCheck();
        char textCharacter = buffer.fetchNextCharacter(&buffer);
        //TODO:diferenciar FDT del centinela.
        //FDT para el buffer es un concepto. Responde indicando si le quedan caracteres por entregar.
        //El centinela, si lo encuentro, marca un corte.
        while (notFDT(buffer)) { //- Mientras no sea fdt, repetir:
            automata.setActualStateToInitialState(&automata); // (1) Estado actual del autómata: estado inicial
            /*while (!automata.stateIsFinalOrFDT()) { // (2) Mientras no sea un estado final y no sea el estado FDT, repetir:
                automata.determineCurrentState(textCharacter); // (2.1) Determinar el nuevo estado actual
                prettyPrinter.append(textCharacter);
                textCharacter = buffer.fetchNextCharacter(&buffer); // (2.2) Actualizar el carácter a analizar
                if (textCharacter == SENTINEL) {
                    break;
                }
            }
            if (automata.actualStateIsFinal()) { // (3) Si el estado es final, la cadena procesada es una constante entera;
                //Deprecado en favor de pretty print <- buffer.print(); // o hacer un buffer.getAll() con un %s.
                printf("¡La cadena pertenece al lenguaje!\n");
                prettyPrinter.persistString(palabra);
                //PP <- buffer.clean();
            } else { // caso contrario, la cadena no pertenece al lenguaje.
                printf("La cadena no pertenece al lenguaje.\n");
                prettyPrinter.flush();
                //PP <- buffer.clean();
            }*/
            printf("LOL\n");
            break;
        }
        buffer.clean(&buffer);
        //prettyPrinter.printResults()
        lexicalCheckRequired = askForAnotherLexicalCheck();
    } while (lexicalCheckRequired);

    return 0;
}

bool notFDT(Buffer buffer) {
    return !buffer.isFDT(&buffer);
}

bool askForAnotherLexicalCheck() {
    char answer;
    printf("¿Desea ingresar otra cadena? S/N: ");
    answer = (char) getc(stdin);
    while ((getchar()) != '\n');

    return toupper(answer) == YES ? true : false;
}

char *requestStringInputToCheck() {
    int c;
    char *string;
    string = malloc(sizeof(char));
    string[0] = '\0';
    printf("Ingrese la cadena a analizar: ");
    for (int i = 0; (c = getchar()) != '\n' && c != EOF; i++) {
        string = realloc(string, (i + 2) * sizeof(char));
        string[i] = (char) c;
        string[i + 1] = '\0';
    }

    return string;
}