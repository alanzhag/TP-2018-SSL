#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define YES 'S'
#define FDT EOF

bool askForAnotherLexicalCheck();

char *requestStringInputToCheck();

struct _Buffer;

typedef char (*FetchNextCharacter)(struct _Buffer *self);

typedef struct _Automata {
    int id;
} Automata;

typedef struct _Buffer {
    char *input;
    FetchNextCharacter fetchNextCharacter;
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

/*
 * Algoritmo 3
- Intenta leer primer carácter del texto (porque el texto puede estar
vacío)
- Mientras no sea fdt, repetir:
(1) Estado actual del autómata: estado inicial
(2) Mientras no sea un estado final y no sea el estado FDT, repetir:
(2.1) Determinar el nuevo estado actual
(2.2) Actualizar el carácter a analizar
(3) Si el estado es final, la cadena procesada es una constante entera;
caso contrario, la cadena no pertenece al lenguaje.
 */

int main() {
    bool lexicalCheckRequired;
    Buffer buffer = {.fetchNextCharacter = Buffer__fetchNextCharacter};

    do {
        buffer.input = requestStringInputToCheck();

        char textCharacter = buffer.fetchNextCharacter(&buffer);
        /*while (!IsFDT(textCharacter)) { //- Mientras no sea fdt, repetir:
            automata.setActualStateToInitialState(); // (1) Estado actual del autómata: estado inicial
            while (!automata.stateIsFinalOrFDT()) { // (2) Mientras no sea un estado final y no sea el estado FDT, repetir:
                automata.determineCurrentState(textCharacter); // (2.1) Determinar el nuevo estado actual
                buffer.add(textCharacter);
                textCharacter = fetchNextCharacterFromInput(); // (2.2) Actualizar el carácter a analizar
            }
            if (automata.actualStateIsFinal()) { // (3) Si el estado es final, la cadena procesada es una constante entera;
                buffer.print(); // o hacer un buffer.getAll() con un %s.
                printf("¡La cadena pertenece al lenguaje!\n");
                buffer.clean();
            } else { // caso contrario, la cadena no pertenece al lenguaje.
                printf("La cadena no pertenece al lenguaje.\n");
                buffer.clean();
            }
        }*/
        free(buffer.input);
        lexicalCheckRequired = askForAnotherLexicalCheck();
    } while (lexicalCheckRequired);

    return 0;
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