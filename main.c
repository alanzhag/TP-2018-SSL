#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define YES 'S'
#define FDT '\0'
#define CENTINEL '%'

struct _Buffer;
struct _Automaton;
struct _AutomatonTable;

bool stateIsNotFinalNorFDT(struct _Automaton automaton);

bool askForAnotherLexicalCheck();

bool notFDT(struct _Buffer buffer);

char *requestStringInputToCheck();

typedef enum {
    INITIAL, FINAL, END_OF_TEXT, END_OF_SEQUENCE, NONE
} STATE_PROPERTY;

typedef struct _State {
    int id;
    STATE_PROPERTY stateProperty;
} State;

typedef State (*GetInitialState)(struct _AutomatonTable *self);

typedef State (*MakeTransitionFromState)(struct _AutomatonTable *self, State state, char character);

typedef struct _AutomatonTable {
    GetInitialState getInitialState;
    MakeTransitionFromState makeTransitionFromState;
} AutomatonTable;

typedef void (*SetActualStateToInitialState)(struct _Automaton *self);

typedef void (*DetermineCurrentState)(struct _Automaton *self, char character);

typedef struct _Automaton {
    State actualState;
    AutomatonTable automatonTable;
    SetActualStateToInitialState setActualStateToInitialState;
    DetermineCurrentState determineCurrentState;
} Automaton;

void Automaton__setActualStateToInitialState(Automaton *self) {
    //PRIORIDAD!!!!
    //TODO: implementar la tabla. Necesito poder decirle que solito vaya al estado inicial (de verdad / no hardcoded).
    //State initialState = {.id = 0, .stateProperty = INITIAL};
    AutomatonTable automatonTable = self->automatonTable;
    self->actualState = automatonTable.getInitialState(&automatonTable);
}

void Automaton__determineCurrentState(Automaton *self, char character) {
    AutomatonTable automatonTable = self->automatonTable;
    State resultState = automatonTable.makeTransitionFromState(&automatonTable, self->actualState, character);
    self->actualState = resultState;
}

typedef char (*FetchNextCharacter)(struct _Buffer *self);

typedef char (*IsFDT)(struct _Buffer *self);

typedef void (*Clean)(struct _Buffer *self);

typedef void (*Push)(struct _Buffer *self, char character);

typedef struct _Buffer {
    char *input;
    FetchNextCharacter fetchNextCharacter;
    IsFDT isFDT;
    Clean clean;
    Push push;
} Buffer;

char Buffer__fetchNextCharacter(Buffer *self) {
    char *wholeBufferInput = self->input;
    char nextCharacter = wholeBufferInput[0];
    size_t sizeOfInput = strlen(wholeBufferInput);
    char *inputWithoutFirstCharacter = malloc(sizeOfInput - 1);
    memcpy(inputWithoutFirstCharacter, wholeBufferInput + 1, sizeOfInput - 1);
    self->clean(self);
    self->input = inputWithoutFirstCharacter;
    return nextCharacter;
}

char Buffer__isFDT(Buffer *self) {
    char *c = self->input;
    return ((c != NULL) && (c[0] == FDT));
}

void Buffer__clean(Buffer *self) {
    free(self->input);
}

void Buffer__push(Buffer *self, char character) {
    char *wholeBufferInput = self->input;
    size_t sizeOfInput = strlen(wholeBufferInput);
    char *inputWithPushedCharacterAtBeginning = malloc(sizeOfInput + 1);
    inputWithPushedCharacterAtBeginning[0] = character;
    memcpy(inputWithPushedCharacterAtBeginning + 1, wholeBufferInput, sizeOfInput);
    self->clean(self);
    self->input = inputWithPushedCharacterAtBeginning;
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
 * Reflexionando que pasa si el Automaton no tiene a donde ir por caracter desconocido. Si no agrego columnas
 * onda "otros" y fdt, deberia cortar y seguir sacando del buffer hasta el centinela.
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
 * +------------+-----------+-----+-----+---+---+-------+-----+
 * |    AFD     | . (punto) | 0-1 | 2-9 | B | % | Otros | FDT |
 * +------------+-----------+-----+-----+---+---+-------+-----+
 * | 0-         |         6 |   1 |   6 | 5 |   |     6 |   7 |
 * | 1          |         2 |   4 |   6 | 5 |   |     6 |   7 |
 * | 2          |         6 |   3 |   3 | 6 |   |     6 |   7 |
 * | 3          |         6 |   5 |   5 | 6 |   |     6 |   7 |
 * | 4          |         6 |   4 |   6 | 5 |   |     6 |   7 |
 * | 5+         |           |     |     |   |   |       |     |
 * | 6(rechazo) |         6 |   6 |   6 | 6 |   |     6 |   7 |
 * | 7(fdt)     |           |     |     |   |   |       |     |
 * +------------+-----------+-----+-----+---+---+-------+-----+
 *
 * Hipotesis de trabajo:
 * La cadena ingresad no va contener FDT. Se deduce fin de texto cuando el buffer se vacia.
 * Si el usuario ingresa una palabra con caracteres fuera del alfabeto de la ER, sacamos caracteres hasta el centinela.
 *
 *
 * -Estado inicial es unico? SI
 * -FDT en cadena de usuario. Es el \0.
 * -Uso del centinela.
 * -Cuando llegan caracteres que no pertenecen al lenguaje, que estado se toma? Corto hasta el centinela?
 *
 * Disruptivo: se crea el estado "Otros". Es un pozo hasta el centinela que tambien es otro estado.
 */

int main() {
    bool lexicalCheckRequired;
    AutomatonTable automatonTable = {}; //TODO: implementar la tabla.
    Buffer buffer = {.fetchNextCharacter = Buffer__fetchNextCharacter,
            .clean = Buffer__clean,
            .push = Buffer__push};
    Automaton automaton = {.setActualStateToInitialState = Automaton__setActualStateToInitialState,
            .determineCurrentState = Automaton__determineCurrentState,
            .automatonTable = automatonTable};

    do {

        buffer.input = requestStringInputToCheck();
        char textCharacter = buffer.fetchNextCharacter(&buffer);
        //TODO:diferenciar FDT del centinela.
        //FDT para el buffer es un concepto. Responde indicando si le quedan caracteres por entregar.
        //El centinela, si lo encuentro, marca un corte.
        //TODO: ver que pasa si mando %%%%%%%%%%% (muchas "cadenas vacias")
        while (textCharacter != FDT) { //- Mientras no sea fdt, repetir:
            automaton.setActualStateToInitialState(&automaton); // (1) Estado actual del autómata: estado inicial
            // (2) Mientras no sea un estado final y no sea el estado FDT, repetir
            while (stateIsNotFinalNorFDT(automaton) && textCharacter != CENTINEL) {
                automaton.determineCurrentState(&automaton, textCharacter); // (2.1) Determinar el nuevo estado actual
                /*    prettyPrinter.append(textCharacter); //esto agrega el %.
                    textCharacter = buffer.fetchNextCharacter(&buffer); // (2.2) Actualizar el carácter a analizar
                }
                if (Automaton.actualStateIsFinal()) { // (3) Si el estado es final, la cadena procesada es una constante entera;
                    //Deprecado en favor de pretty print <- buffer.print(); // o hacer un buffer.getAll() con un %s.
                    printf("¡La cadena pertenece al lenguaje!\n");
                    prettyPrinter.persistString(); //en teoria contiene al %. Debo volarlo si esta al persistirlo.
                    //TODO: como me traje un 0, debe retornarlo al buffer. Esta en textCharacter.
                    buffer.push(&buffer, textCharacter);
                    //PP <- buffer.clean();
                } else { // caso contrario, la cadena no pertenece al lenguaje.
                    printf("La cadena no pertenece al lenguaje.\n");
                    prettyPrinter.flush();
                    //PP <- buffer.clean(); se caga el resto. No usar.*/
            }
            printf("LOL\n");
            break;
        }

        buffer.clean(&buffer);
        //prettyPrinter.printResults()
        lexicalCheckRequired = askForAnotherLexicalCheck();
    } while (lexicalCheckRequired);

    return 0;
}

bool stateIsNotFinalNorFDT(Automaton automaton) {
    STATE_PROPERTY stateProperty = automaton.actualState.stateProperty;
    return stateProperty != FINAL && stateProperty != END_OF_TEXT;
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
        string[i + 1] = FDT;
    }

    return string;
}