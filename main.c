#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define YES 'S'
#define FDT '\0'
#define CENTINEL '%'
#define NO_MATCH -1
#define AUTOMATON_STATES_QTY 6
#define CHARACTER_MATCHERS_QTY 6
#define INITIAL_STATE_ID 0

struct _Buffer;
struct _Automaton;
struct _AutomatonTable;
struct _CharacterStateMatcher;
struct _Matcher;

//Utils functions defined at bottom

bool stateIsNotFinalNorFDT(struct _Automaton automaton);

bool askForAnotherLexicalCheck();

char *requestStringInputToCheck();

//State

typedef enum {
    INITIAL, FINAL, END_OF_TEXT, END_OF_SEQUENCE, REJECTION, NONE
} StateProperty;

typedef struct _State {
    int id;
    StateProperty stateProperty;
} State;

//Matcher
typedef bool (*MatcherFunction)(struct _Matcher *self, char character);

//TODO: Esto podría ser parte del CharacterStateMatcher(columna, [caracteres que matchean]) y .match(caracter)
typedef struct _Matcher {
    const char *charactersToMatch;
    MatcherFunction match;
} Matcher;

bool Matcher__match(Matcher *self, char character) {
    const char *charactersToMatch = self->charactersToMatch;
    size_t lengthOfMatchableCharacters = strlen(charactersToMatch);
    for (int i = 0; i < lengthOfMatchableCharacters; i++) {
        if (charactersToMatch[i] == character) {
            return true;
        }
    }
    return false;
}

Matcher Matcher__init(const char *charactersToMatch) {
    Matcher matcher = {.charactersToMatch = charactersToMatch, .match = Matcher__match};
    return matcher;
};

//CharacterStateMatcher

typedef int (*Match)(struct _CharacterStateMatcher *self, char characterToMatch);

typedef struct _CharacterStateMatcher {
    int column;
    Match match;
    Matcher matcher;

} CharacterStateMatcher;

int CharacterStateMatcher__match(CharacterStateMatcher *self, char characterToMatch) {
    Matcher matcher = self->matcher;
    if (matcher.match(&matcher, characterToMatch)) {
        return self->column;
    } else {
        return NO_MATCH;
    };
}

CharacterStateMatcher CharacterStateMatcher__init(int column, char *charactersToMatch) {
    CharacterStateMatcher characterStateMatcher = {
            .match = CharacterStateMatcher__match,
            .matcher = Matcher__init(charactersToMatch),
            .column = column};
    return characterStateMatcher;
};

//CharacterStateMatcherService

typedef CharacterStateMatcher *(*GetCharacterStateMatchers)();

typedef struct _CharacterStateMatcherService {
    GetCharacterStateMatchers getCharacterStateMatchers;
} CharacterStateMatcherService;

CharacterStateMatcher *CharacterStateMatcherService__getCharacterStateMatchers() {
    CharacterStateMatcher *characterStateMatchers = malloc(CHARACTER_MATCHERS_QTY * sizeof(CharacterStateMatcher));
    char dot[] = {'.'};
    char zeroAndOne[] = {'0', '1'};
    char fromTwoToNine[] = {'2', '3', '4', '5', '6', '7', '8', '9'};
    char b[] = {'B'};
    char others[] = {};
    char fdt[] = {FDT};
    CharacterStateMatcher__init(0, dot);
    CharacterStateMatcher__init(1, zeroAndOne);
    CharacterStateMatcher__init(2, fromTwoToNine);
    CharacterStateMatcher__init(3, b);
    CharacterStateMatcher__init(4, others);
    CharacterStateMatcher__init(5, fdt);
    return characterStateMatchers;
};

//AutomatonTableService

typedef State **(*GetTable)();

typedef struct _AutomatonTableService {
    GetTable getTable;
} AutomatonTableService;

State **AutomatonTableService__getTable() {
    State **automatonTable;

    automatonTable = malloc(AUTOMATON_STATES_QTY * sizeof(State *));
    for (int i = 0; i < AUTOMATON_STATES_QTY; i++) {
        automatonTable[i] = malloc(CHARACTER_MATCHERS_QTY * sizeof(State));
    }

    //TODO: Crear los estados y meterlos en el **.


    return automatonTable;
}

// AutomatonTable

typedef State (*MakeTransitionFromState)(struct _AutomatonTable *self, State state, char character); //TODO

typedef State (*GetInitialState)();

typedef struct _AutomatonTable {
    State **table;
    CharacterStateMatcher *characterStateMatchers;
    GetInitialState getInitialState;
    MakeTransitionFromState makeTransitionFromState;
} AutomatonTable;

State AutomatonTable__getInitialState() {
    State initialState = {.id = INITIAL_STATE_ID, .stateProperty = INITIAL};
    return initialState;
}

State AutomatonTable__makeTransitionFromState(AutomatonTable *self, State state, char character) {
    State arrivalState = {};
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
    self->actualState = automatonTable.getInitialState();
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
    char *inputWithoutFirstCharacter = malloc(sizeOfInput - 1);
    memcpy(inputWithoutFirstCharacter, wholeBufferInput + 1, sizeOfInput - 1);
    self->clean(self);
    self->input = inputWithoutFirstCharacter;
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
 * +------------+-----------+-----+-----+---+-------+-----+
 * |    AFD     | . (punto) | 0-1 | 2-9 | B | Otros | FDT |
 * +------------+-----------+-----+-----+---+-------+-----+
 * | 0-         |         6 |   1 |   6 | 5 |     6 |   7 |
 * | 1          |         2 |   4 |   6 | 5 |     6 |   7 |
 * | 2          |         6 |   3 |   3 | 6 |     6 |   7 |
 * | 3          |         6 |   5 |   5 | 6 |     6 |   7 |
 * | 4          |         6 |   4 |   6 | 5 |     6 |   7 |
 * | 5+         |           |     |     |   |       |     |
 * | 6(rechazo) |         6 |   6 |   6 | 6 |     6 |   7 |
 * | 7(fdt)     |           |     |     |   |       |     |
 * +------------+-----------+-----+-----+---+-------+-----+
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

    CharacterStateMatcherService characterStateMatcherService = {};

    AutomatonTable automatonTable = {
            .table = automatonTableService.getTable(),
            .characterStateMatchers = NULL,
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
            break;
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