#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

#define SI 'S'

bool askForAnotherLexicalCheck();

int main() {
    bool lexicalCheckRequired;

    do {
        lexicalCheckRequired = askForAnotherLexicalCheck();
    } while (lexicalCheckRequired);

    return 0;
}

bool askForAnotherLexicalCheck() {
    char answer = 'N';
    printf("¿Desea ingresar otra cadena? S/N: ");
    scanf(" %c", &answer);
    fflush(stdout);
    return toupper(answer) == SI ? true : false;
}