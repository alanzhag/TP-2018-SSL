#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define YES 'S'
#define FDT '\0'
#define CENTINEL '%'
#define NO_MATCH (-1)
#define AUTOMATON_STATES_ROWS 8
#define CHARACTER_MATCHERS_COLUMNS 7

struct _Buffer;
struct _Automaton;
struct _AutomatonTable;
struct _CharacterStateMatcher;

//Utils functions defined at bottom

bool stateIsNotFinalNorFDT(struct _Automaton automaton);

bool askForAnotherLexicalCheck();

char *requestStringInputToCheck();

//State

typedef enum {
    INITIAL, FINAL, END_OF_TEXT, REJECTION, NULL_STATE, NONE
} StateProperty;

typedef struct _State {
    int id;
    StateProperty stateProperty;
} State;

//CharacterStateMatcher

typedef int (*Match)(struct _CharacterStateMatcher *self, char characterToMatch);

typedef struct _CharacterStateMatcher {
    int column;
    const char *charactersToMatch;
    Match match;
} CharacterStateMatcher;

int CharacterStateMatcher__match(CharacterStateMatcher *self, char characterToMatch) {
    const char *charactersToMatch = self->charactersToMatch;
    size_t lengthOfMatchableCharacters = strlen(charactersToMatch);

    if (lengthOfMatchableCharacters == 0) {
        return self->column;
    }

    for (int i = 0; i < lengthOfMatchableCharacters; i++) {
        if (charactersToMatch[i] == characterToMatch) {
            return self->column;
        }
    }

    return NO_MATCH;
}

CharacterStateMatcher CharacterStateMatcher__init(int column, const char *charactersToMatch) {
    CharacterStateMatcher characterStateMatcher = {
            .match = CharacterStateMatcher__match,
            .charactersToMatch = charactersToMatch,
            .column = column};
    return characterStateMatcher;
};

//CharacterStateMatcherService

typedef CharacterStateMatcher *(*GetCharacterStateMatchers)();

typedef struct _CharacterStateMatcherService {
    GetCharacterStateMatchers getCharacterStateMatchers;
} CharacterStateMatcherService;

CharacterStateMatcher *CharacterStateMatcherService__getCharacterStateMatchers() {
    CharacterStateMatcher *characterStateMatchers = malloc(CHARACTER_MATCHERS_COLUMNS * sizeof(CharacterStateMatcher));

    char dot[] = {'.'};
    char zeroAndOne[] = {'0', '1'};
    char fromTwoToNine[] = {'2', '3', '4', '5', '6', '7', '8', '9'};
    char b[] = {'B'};
    char others[] = {};
    char fdt[] = {FDT};

    CharacterStateMatcher__init(1, dot);
    CharacterStateMatcher__init(2, zeroAndOne);
    CharacterStateMatcher__init(3, fromTwoToNine);
    CharacterStateMatcher__init(4, b);
    CharacterStateMatcher__init(5, others);
    CharacterStateMatcher__init(6, fdt);

    return characterStateMatchers;
};

//AutomatonTableService

typedef State **(*GetTable)();

typedef struct _AutomatonTableService {
    GetTable getTable;
} AutomatonTableService;

State **AutomatonTableService__getTable() {
    State **automatonTable;

    automatonTable = malloc(AUTOMATON_STATES_ROWS * sizeof(State *));
    for (int i = 0; i < AUTOMATON_STATES_ROWS; i++) {
        automatonTable[i] = malloc(CHARACTER_MATCHERS_COLUMNS * sizeof(State));
    }

    /*
     * Rows: 8 Columns: 7
     *       0            1        2     3    4     5      6
     * +------------+-----------+-----+-----+---+-------+-----+
     * |    AFD     | . (punto) | 0-1 | 2-9 | B | Otros | FDT |
     * +------------+-----------+-----+-----+---+-------+-----+
     * | 0-         |         6 |   1 |   6 | 5 |     6 |   7 |     0
     * | 1          |         2 |   4 |   6 | 5 |     6 |   7 |     1
     * | 2          |         6 |   3 |   3 | 6 |     6 |   7 |     2
     * | 3          |         6 |   5 |   5 | 6 |     6 |   7 |     3
     * | 4          |         6 |   4 |   6 | 5 |     6 |   7 |     4
     * | 5+         |           |     |     |   |       |     |     5
     * | 6(rechazo) |         6 |   6 |   6 | 6 |     6 |   7 |     6
     * | 7(fdt)     |           |     |     |   |       |     |     7
     * +------------+-----------+-----+-----+---+-------+-----+
     */

    State nullState = {.stateProperty = NULL_STATE};
    State initialState = {.id = 0, .stateProperty = INITIAL};
    State state1 = {.id = 1, .stateProperty = NONE};
    State state2 = {.id = 2, .stateProperty = NONE};
    State state3 = {.id = 3, .stateProperty = NONE};
    State state4 = {.id = 4, .stateProperty = NONE};
    State state5 = {.id = 5, .stateProperty = FINAL};
    State state6 = {.id = 6, .stateProperty = REJECTION};
    State state7 = {.id = 7, .stateProperty = END_OF_TEXT};

    //Hardcoded table with first column as all possible states for informative purposes
    automatonTable[0][0] = initialState;
    automatonTable[1][0] = state1;
    automatonTable[2][0] = state2;
    automatonTable[3][0] = state3;
    automatonTable[4][0] = state4;
    automatonTable[5][0] = state5;
    automatonTable[6][0] = state6;
    automatonTable[7][0] = state7;

    //ROW 0          6 |   1 |   6 | 5 |     6 |   7
    automatonTable[0][1] = state6;
    automatonTable[0][2] = state1;
    automatonTable[0][3] = state6;
    automatonTable[0][4] = state5;
    automatonTable[0][5] = state6;
    automatonTable[0][6] = state7;
    //ROW 1          2 |   4 |   6 | 5 |     6 |   7
    automatonTable[1][1] = state2;
    automatonTable[1][2] = state4;
    automatonTable[1][3] = state6;
    automatonTable[1][4] = state5;
    automatonTable[1][5] = state6;
    automatonTable[1][6] = state7;
    //ROW 2          6 |   3 |   3 | 6 |     6 |   7
    automatonTable[2][1] = state6;
    automatonTable[2][2] = state3;
    automatonTable[2][3] = state3;
    automatonTable[2][4] = state6;
    automatonTable[2][5] = state6;
    automatonTable[2][6] = state7;
    //ROW 3          6 |   5 |   5 | 6 |     6 |   7
    automatonTable[3][1] = state6;
    automatonTable[3][2] = state5;
    automatonTable[3][3] = state5;
    automatonTable[3][4] = state6;
    automatonTable[3][5] = state6;
    automatonTable[3][6] = state7;
    //ROW 4          6 |   4 |   6 | 5 |     6 |   7
    automatonTable[4][1] = state6;
    automatonTable[4][2] = state4;
    automatonTable[4][3] = state6;
    automatonTable[4][4] = state5;
    automatonTable[4][5] = state6;
    automatonTable[4][6] = state7;
    //ROW 5
    automatonTable[5][1] = nullState;
    automatonTable[5][2] = nullState;
    automatonTable[5][3] = nullState;
    automatonTable[5][4] = nullState;
    automatonTable[5][5] = nullState;
    automatonTable[5][6] = nullState;
    //ROW 6          6 |   6 |   6 | 6 |     6 |   7
    automatonTable[6][1] = state6;
    automatonTable[6][2] = state6;
    automatonTable[6][3] = state6;
    automatonTable[6][4] = state6;
    automatonTable[6][5] = state6;
    automatonTable[6][6] = state7;
    //ROW 7
    automatonTable[7][1] = nullState;
    automatonTable[7][2] = nullState;
    automatonTable[7][3] = nullState;
    automatonTable[7][4] = nullState;
    automatonTable[7][5] = nullState;
    automatonTable[7][6] = nullState;

    return automatonTable;
}

// AutomatonTable

typedef State (*MakeTransitionFromState)(struct _AutomatonTable *self, State state, char character); //TODO

typedef State (*GetInitialState)(struct _AutomatonTable *self);

typedef struct _AutomatonTable {
    State **table;
    CharacterStateMatcher *characterStateMatchers;
    GetInitialState getInitialState;
    MakeTransitionFromState makeTransitionFromState;
} AutomatonTable;

State AutomatonTable__getInitialState(AutomatonTable *self) {
    //TODO: Puedo leer la columna 0 para ver los estados.
    State **table = self->table;
    for (int i = 0; i < AUTOMATON_STATES_ROWS; i++) {
        State currentState = table[i][0];
        if (currentState.stateProperty == INITIAL) {
            return currentState;
        }
    }
    return table[0][0];
}

State AutomatonTable__makeTransitionFromState(AutomatonTable *self, State state, char character) {
    State arrivalState = {};

    for (int i = 0; i < CHARACTER_MATCHERS_COLUMNS; i++) {
        CharacterStateMatcher matcher = self->characterStateMatchers[i];
        if (matcher.match(&matcher, character) != NO_MATCH) {
            arrivalState = self->table[state.id][matcher.column];
        }
    }

    return arrivalState;
};

// Automaton

typedef void (*SetActualStateToInitialState)(struct _Automaton *self);

typedef void (*DetermineCurrentState)(struct _Automaton *self, char character);

typedef struct _Automaton {
    State actualState;
    AutomatonTable automatonTable;
    SetActualStateToInitialState setActualStateToInitialState;
    DetermineCurrentState determineCurrentState;
} Automaton;

void Automaton__setActualStateToInitialState(Automaton *self) {
    AutomatonTable automatonTable = self->automatonTable;
    self->actualState = automatonTable.getInitialState(&automatonTable);
}

void Automaton__determineCurrentState(Automaton *self, char character) {
    AutomatonTable automatonTable = self->automatonTable;
    State resultState = automatonTable.makeTransitionFromState(&automatonTable, self->actualState, character);
    self->actualState = resultState;
}

//Buffer

typedef void (*Clean)(struct _Buffer *self);

typedef void (*Push)(struct _Buffer *self, char character);

typedef char (*FetchNextCharacter)(struct _Buffer *self);

typedef struct _Buffer {
    char *input;
    FetchNextCharacter fetchNextCharacter;
    Clean clean;
    Push push;
} Buffer;

char Buffer__fetchNextCharacter(Buffer *self) { //TODO: si no tiene nada no puede fetchear y rompe.
    char *wholeBufferInput = self->input;
    char nextCharacter = wholeBufferInput[0];
    size_t sizeOfInput = strlen(wholeBufferInput);
    if (sizeOfInput != 0) {
        char *inputWithoutFirstCharacter = malloc(sizeOfInput - 1);
        memcpy(inputWithoutFirstCharacter, wholeBufferInput + 1, sizeOfInput - 1);
        self->clean(self);
        self->input = inputWithoutFirstCharacter;
    }

    return nextCharacter;
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
    AutomatonTableService automatonTableService = {
            .getTable = AutomatonTableService__getTable};

    CharacterStateMatcherService characterStateMatcherService = {
            .getCharacterStateMatchers = CharacterStateMatcherService__getCharacterStateMatchers};

    AutomatonTable automatonTable = {
            .table = automatonTableService.getTable(),
            .characterStateMatchers = characterStateMatcherService.getCharacterStateMatchers(),
            .getInitialState = AutomatonTable__getInitialState,
            .makeTransitionFromState = AutomatonTable__makeTransitionFromState};
    Buffer buffer = {
            .fetchNextCharacter = Buffer__fetchNextCharacter,
            .clean = Buffer__clean,
            .push = Buffer__push};
    Automaton automaton = {
            .setActualStateToInitialState = Automaton__setActualStateToInitialState,
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
            //TODO: Si hay error, debo leer hasta el proximo % o fdt. Tiro la basura.
            while (stateIsNotFinalNorFDT(automaton) && textCharacter != CENTINEL) {
                automaton.determineCurrentState(&automaton, textCharacter); // (2.1) Determinar el nuevo estado actual
                //TODO: Fixear el bucle infinito
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
            //break;
        }

        buffer.clean(&buffer);
        //prettyPrinter.printResults()
        lexicalCheckRequired = askForAnotherLexicalCheck();
    } while (lexicalCheckRequired);

    return 0;
}

bool stateIsNotFinalNorFDT(Automaton automaton) {
    StateProperty stateProperty = automaton.actualState.stateProperty;
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