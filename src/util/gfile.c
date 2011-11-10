#include <ctype.h>
#include <string.h>
#include "gfile.h"


void openfile(char* st)
{
    file = fopen(st, "r");
    if(!file){
        printf("Erreur d'ouverture du fichier %s fileName\n",st);
    }
}


void closefile()
{
    fclose(file);
}


char* readLine()
{
    indiceToken = 0 ;
    return fgets(line,MAX_LINE_LENGTH, file);
}

char* readUncommentedLine()
{
    char *l;
    do{
        l = readLine();
    }while((l!=NULL) && (l[0]=='#')) ;
    return l;
}



/*!
 * returns next token from line (token = all symbols except space/tab/newline)
 *
 * \return the next token
 */
char* getNextToken()
{
    int strlenline = strlen(line) ;


    while ((isspace(line[indiceToken]) || line[indiceToken]==';'|| line[indiceToken]==':') && (indiceToken<strlenline-1)) {
    //while ((isspace(line[indiceToken]) || line[indiceToken]==';') && (indiceToken<strlenline-1)) {
        indiceToken++ ;
    }

    int indiceDebut = indiceToken ;
    while (!(isspace(line[indiceToken]) || line[indiceToken]==';'|| line[indiceToken]==':')&& (indiceToken<strlenline)) {
    //while (!(isspace(line[indiceToken]) || line[indiceToken]==';')&& (indiceToken<strlenline)) {
        token[indiceToken-indiceDebut]=line[indiceToken] ;
        indiceToken++ ;
    }
    token[indiceToken-indiceDebut]='\0' ;
    if (token[0]=='\0') return NULL ;
    return token ;
}



