#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define YES 'S'
#define FDT '\0'
#define NO_MATCH (-1)
#define AUTOMATON_STATES_ROWS 9
#define CHARACTER_MATCHERS_COLUMNS 8
#define CHARACTER_MATCHERS (CHARACTER_MATCHERS_COLUMNS - 1)

struct _Buffer;
struct _Automaton;
struct _AutomatonTable;
struct _CharacterStateMatcher;

char dot[] = ".";
char zeroAndOne[] = "01";
char fromTwoToNine[] = "23456789";
char b[] = "B";
char centinel[] = "%";
char others[] = ".0123456789B%";
char fdt[] = {FDT};

//Utils functions defined at bottom

bool stateIsNotFinalNorFDT(struct _Automaton automaton);

bool actualStateIsFinal(struct _Automaton automaton);

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

    if (lengthOfMatchableCharacters == 0) {//fdt
        return self->column;
    }

    for (int i = 0; i < lengthOfMatchableCharacters; i++) {
        if (charactersToMatch[i] == characterToMatch) {
            return self->column;
        }
    }

    return NO_MATCH;
}

int CharacterStateMatcher__otherMatch(CharacterStateMatcher *self, char characterToMatch) {
    const char *charactersToMatch = self->charactersToMatch;
    size_t lengthOfMatchableCharacters = strlen(charactersToMatch);

    if (characterToMatch == FDT) { //Por las dudas
        return NO_MATCH;
    }

    for (int i = 0; i < lengthOfMatchableCharacters; i++) {
        if (charactersToMatch[i] == characterToMatch) {
            return NO_MATCH;
        }
    }

    return self->column;
}

CharacterStateMatcher CharacterStateMatcher__init(int column, const char *charactersToMatch) {
    CharacterStateMatcher characterStateMatcher = {
            .match = CharacterStateMatcher__match,
            .charactersToMatch = charactersToMatch,
            .column = column};
    return characterStateMatcher;
};

//AutomatonTableService

typedef State **(*GetTable)();

typedef struct _AutomatonTableService {
    GetTable getTable;
} AutomatonTableService;

State **AutomatonTableService__getTable() {
    State **automatonTable;

    //automatonTable = malloc(AUTOMATON_STATES_ROWS * sizeof(State *));
    automatonTable = calloc(AUTOMATON_STATES_ROWS, sizeof(State *));
    for (int i = 0; i < AUTOMATON_STATES_ROWS; i++) {
        //automatonTable[i] = malloc(CHARACTER_MATCHERS_COLUMNS * sizeof(State));
        automatonTable[i] = calloc(CHARACTER_MATCHERS_COLUMNS, sizeof(State));
    }

    /*
     * Rows: 9 - Columns: 8
     *
     *       0             1       2     3    4   5     6      7
     * +------------+-----------+-----+-----+---+---+-------+-----+
     * |    AFD     | . (punto) | 0-1 | 2-9 | B | % | Otros | FDT |
     * +------------+-----------+-----+-----+---+---+-------+-----+
     * | 0-         |         7 |   1 |   7 | 5 | 0 |     7 |   8 |     0
     * | 1          |         2 |   4 |   7 | 5 | 0 |     7 |   8 |     1
     * | 2          |         7 |   3 |   3 | 7 | 0 |     7 |   8 |     2
     * | 3          |         7 |   5 |   5 | 7 | 0 |     7 |   8 |     3
     * | 4          |         7 |   4 |   7 | 5 | 0 |     7 |   8 |     4
     * | 5          |         7 |   7 |   7 | 7 | 6 |     7 |   6 |     5
     * | 6+         |         7 |   1 |   7 | 5 | 0 |     7 |   8 |     6
     * | 7(Rechazo) |         7 |   7 |   7 | 7 | 0 |     7 |   8 |     7
     * | 8(fdt)     |           |     |     |   |   |       |     |     8
     * +------------+-----------+-----+-----+---+---+-------+-----+
     */

    State nullState = {.stateProperty = NULL_STATE};
    State state0 = {.id = 0, .stateProperty = INITIAL};
    State state1 = {.id = 1, .stateProperty = NONE};
    State state2 = {.id = 2, .stateProperty = NONE};
    State state3 = {.id = 3, .stateProperty = NONE};
    State state4 = {.id = 4, .stateProperty = NONE};
    State state5 = {.id = 5, .stateProperty = NONE};
    State state6 = {.id = 6, .stateProperty = FINAL};
    State state7 = {.id = 7, .stateProperty = REJECTION};
    State state8 = {.id = 8, .stateProperty = END_OF_TEXT};

    //Hardcoded table with first column as all possible states for informative purposes

    //ROW 0 ['7', '1', '7', '5', '0', '7', '8']
    automatonTable[0][1] = state7;
    automatonTable[0][2] = state1;
    automatonTable[0][3] = state7;
    automatonTable[0][4] = state5;
    automatonTable[0][5] = state0;
    automatonTable[0][6] = state7;
    automatonTable[0][7] = state8;

    //ROW 1 ['2', '4', '7', '5', '0', '7', '8']
    automatonTable[1][1] = state2;
    automatonTable[1][2] = state4;
    automatonTable[1][3] = state7;
    automatonTable[1][4] = state5;
    automatonTable[1][5] = state0;
    automatonTable[1][6] = state7;
    automatonTable[1][7] = state8;

    //ROW 2 ['7', '3', '3', '7', '0', '7', '8']
    automatonTable[2][1] = state7;
    automatonTable[2][2] = state3;
    automatonTable[2][3] = state3;
    automatonTable[2][4] = state7;
    automatonTable[2][5] = state0;
    automatonTable[2][6] = state7;
    automatonTable[2][7] = state8;

    //ROW 3 ['7', '5', '5', '7', '0', '7', '8']
    automatonTable[3][1] = state7;
    automatonTable[3][2] = state5;
    automatonTable[3][3] = state5;
    automatonTable[3][4] = state7;
    automatonTable[3][5] = state0;
    automatonTable[3][6] = state7;
    automatonTable[3][7] = state8;

    //ROW 4 ['7', '4', '7', '5', '0', '7', '8']
    automatonTable[4][1] = state7;
    automatonTable[4][2] = state4;
    automatonTable[4][3] = state7;
    automatonTable[4][4] = state5;
    automatonTable[4][5] = state0;
    automatonTable[4][6] = state7;
    automatonTable[4][7] = state8;

    //ROW 5 ['7', '7', '7', '7', '6', '7', '6']
    automatonTable[5][1] = state7;
    automatonTable[5][2] = state7;
    automatonTable[5][3] = state7;
    automatonTable[5][4] = state7;
    automatonTable[5][5] = state6;
    automatonTable[5][6] = state7;
    automatonTable[5][7] = state6;

    //ROW 6 ['7', '1', '7', '5', '0', '7', '8']
    automatonTable[6][1] = state7;
    automatonTable[6][2] = state1;
    automatonTable[6][3] = state7;
    automatonTable[6][4] = state5;
    automatonTable[6][5] = state0;
    automatonTable[6][6] = state7;
    automatonTable[6][7] = state8;

    //ROW 7 ['7', '7', '7', '7', '0', '7', '8']
    automatonTable[7][1] = state7;
    automatonTable[7][2] = state7;
    automatonTable[7][3] = state7;
    automatonTable[7][4] = state7;
    automatonTable[7][5] = state0;
    automatonTable[7][6] = state7;
    automatonTable[7][7] = state8;

    //ROW 8 ['', '', '', '', '', '', '']
    automatonTable[8][1] = nullState;
    automatonTable[8][2] = nullState;
    automatonTable[8][3] = nullState;
    automatonTable[8][4] = nullState;
    automatonTable[8][5] = nullState;
    automatonTable[8][6] = nullState;
    automatonTable[8][7] = nullState;

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
    //self->characterStateMatchers = CharacterStateMatcherService__getCharacterStateMatchers(); //TODO: esto no!

    for (int i = 0; i < CHARACTER_MATCHERS; i++) {
        CharacterStateMatcher matcher = self->characterStateMatchers[i];
        if (matcher.match(&matcher, character) != NO_MATCH) {
            return self->table[state.id][matcher.column];
        }
    }

    return arrivalState;
};

//CharacterStateMatcherService

typedef void *(*GetCharacterStateMatchers)(struct _AutomatonTable *automatonTable);

typedef struct _CharacterStateMatcherService {
    GetCharacterStateMatchers getCharacterStateMatchers;
} CharacterStateMatcherService;

void *CharacterStateMatcherService__getCharacterStateMatchers(struct _AutomatonTable *automatonTable) {
    CharacterStateMatcher *characterStateMatchers = calloc(CHARACTER_MATCHERS, sizeof(CharacterStateMatcher));

    CharacterStateMatcher otherMatcher = {
            .column = 6,
            .charactersToMatch = others,
            .match = CharacterStateMatcher__otherMatch
    };

    characterStateMatchers[0] = CharacterStateMatcher__init(1, dot);
    characterStateMatchers[1] = CharacterStateMatcher__init(2, zeroAndOne);
    characterStateMatchers[2] = CharacterStateMatcher__init(3, fromTwoToNine);
    characterStateMatchers[3] = CharacterStateMatcher__init(4, b);
    characterStateMatchers[4] = CharacterStateMatcher__init(5, centinel);
    characterStateMatchers[5] = otherMatcher;
    characterStateMatchers[6] = CharacterStateMatcher__init(7, fdt);

    automatonTable->characterStateMatchers = characterStateMatchers;
}

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
    State resultState = automatonTable.makeTransitionFromState(&self->automatonTable, self->actualState,
                                                               character); //Memleak
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

char Buffer__fetchNextCharacter(Buffer *self) {
    //TODO: si no tiene nada no puede fetchear y rompe.
    //TODO: anda para el culo. no termina achicando el buffer.
    char *wholeBufferInput = self->input;
    char nextCharacter = wholeBufferInput[0];
    size_t sizeOfInput = strlen(wholeBufferInput);
    if (sizeOfInput != 0) {
        //char *inputWithoutFirstCharacter = malloc(sizeOfInput - 1);
        char *inputWithoutFirstCharacter = calloc(sizeOfInput - 1, sizeof(char));
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
    //char *inputWithPushedCharacterAtBeginning = malloc(sizeOfInput + 1);
    char *inputWithPushedCharacterAtBeginning = calloc(sizeOfInput + 1, sizeof(char));
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
            .table = automatonTableService.getTable(),//Memleak
            //.characterStateMatchers = characterStateMatcherService.getCharacterStateMatchers(),//Memleak
            .getInitialState = AutomatonTable__getInitialState,
            .makeTransitionFromState = AutomatonTable__makeTransitionFromState};

    characterStateMatcherService.getCharacterStateMatchers(&automatonTable);

    Buffer buffer = {
            .fetchNextCharacter = Buffer__fetchNextCharacter,
            .clean = Buffer__clean,
            .push = Buffer__push};
    Automaton automaton = {
            .setActualStateToInitialState = Automaton__setActualStateToInitialState,
            .determineCurrentState = Automaton__determineCurrentState,
            .automatonTable = automatonTable};

    do {
        //TODO: Arreglar bug de characterStateMatchers que desaparecen
        buffer.input = requestStringInputToCheck();
        char textCharacter = buffer.fetchNextCharacter(&buffer);
        //TODO:diferenciar FDT del centinela.
        //FDT para el buffer es un concepto. Responde indicando si le quedan caracteres por entregar.
        //El centinela, si lo encuentro, marca un corte.
        //TODO: ver que pasa si mando %%%%%%%%%%% (muchas "cadenas vacias")
        //TODO: ver que pasa si mando 0100010101100B (muchas "cadenas vacias")
        while (textCharacter != FDT) { //- Mientras no sea fdt, repetir:
            automaton.setActualStateToInitialState(&automaton); // (1) Estado actual del autómata: estado inicial
            // (2) Mientras no sea un estado final y no sea el estado FDT, repetir
            //TODO: Si hay error, debo leer hasta el proximo % o fdt. Tiro la basura.
            while (stateIsNotFinalNorFDT(automaton)) {
                automaton.determineCurrentState(&automaton, textCharacter); // (2.1) Determinar el nuevo estado actual
                //prettyPrinter.append(textCharacter); //esto agrega el %.
                textCharacter = buffer.fetchNextCharacter(&buffer); // (2.2) Actualizar el carácter a analizar
            }
            // (3) Si el estado es final, la cadena procesada es una constante entera;
            if (actualStateIsFinal(automaton)) {
                //Deprecado en favor de pretty print <- buffer.print(); // o hacer un buffer.getAll() con un %s.
                printf("¡La cadena pertenece al lenguaje!\n");
                //prettyPrinter.persistString(); //en teoria contiene al %. Debo volarlo si esta al persistirlo.
                //TODO: como me traje un 0, debe retornarlo al buffer. Esta en textCharacter.
                //buffer.push(&buffer, textCharacter);
                //PP <- buffer.clean();
            } else { // caso contrario, la cadena no pertenece al lenguaje.
                printf("La cadena no pertenece al lenguaje.\n");
                //prettyPrinter.flush();
                //PP <- buffer.clean(); se caga el resto. No usar.
            }
            //TODO: Fixear el bucle infinito
            printf("LOL\n");
            //break;
        }

        buffer.clean(&buffer);
        //prettyPrinter.printResults()
        lexicalCheckRequired = askForAnotherLexicalCheck();
    } while (lexicalCheckRequired);

    //Como el sol cuando amanece
    free(automatonTable.characterStateMatchers);
    for (int i = 0; i < AUTOMATON_STATES_ROWS; i++) {
        free(automatonTable.table[i]);
    }
    free(automatonTable.table);

    return 0;
}

bool stateIsNotFinalNorFDT(Automaton automaton) {
    StateProperty stateProperty = automaton.actualState.stateProperty;
    return stateProperty != FINAL && stateProperty != END_OF_TEXT;
}

bool actualStateIsFinal(Automaton automaton) {
    return automaton.actualState.stateProperty == FINAL;
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
    //string = malloc(sizeof(char));
    string = calloc(1, sizeof(char));
    string[0] = '\0';
    printf("Ingrese la cadena a analizar: ");
    for (int i = 0; (c = getchar()) != '\n' && c != EOF; i++) { //TODO: el getchar rompe structs
        string = realloc(string, (i + 2) * sizeof(char));
        string[i] = (char) c;
        string[i + 1] = FDT;
    }

    return string;
}