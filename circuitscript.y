%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int yylex();
extern int yylineno;
extern char* yytext;
extern FILE* yyin;

void yyerror(const char* s);
%}

%union {
    int integer;
    double floatval;
    char* string;
}

/* Tokens */
%token BOARD COMPONENT PLACE NET AS AT ROTATION SIDE TOP BOTTOM
%token WIDTH HEIGHT LAYERS PACKAGE PINS NAME NUM FUNCTION MANUFACTURER MPN
%token MM IN MIL
%token <integer> INTEGER
%token <floatval> FLOAT
%token <string> IDENTIFIER STRING

/* Non-terminals with types */
%type <floatval> dimension

%%

program
    : declarations
    ;

declarations
    : declaration
    | declarations declaration
    ;

declaration
    : board_decl
    | component_decl
    | place_decl
    | net_decl
    ;

board_decl
    : BOARD '{' board_properties '}'
    {
        printf("Board definition found\n");
    }
    ;

board_properties
    : board_property
    | board_properties board_property
    ;

board_property
    : NAME ':' STRING ';'
    {
        printf("Board name: %s\n", $3);
        free($3);
    }
    | WIDTH ':' dimension ';'
    {
        printf("Board width: %.2f\n", $3);
    }
    | HEIGHT ':' dimension ';'
    {
        printf("Board height: %.2f\n", $3);
    }
    | LAYERS ':' INTEGER ';'
    {
        printf("Board layers: %d\n", $3);
    }
    ;

component_decl
    : COMPONENT IDENTIFIER '{' component_properties '}'
    {
        printf("Component definition for %s\n", $2);
        free($2);
    }
    ;

component_properties
    : component_property
    | component_properties component_property
    ;

component_property
    : PACKAGE ':' IDENTIFIER ';'
    {
        printf("Component package: %s\n", $3);
        free($3);
    }
    | MANUFACTURER ':' STRING ';'
    {
        printf("Component manufacturer: %s\n", $3);
        free($3);
    }
    | MPN ':' STRING ';'
    {
        printf("Component part number: %s\n", $3);
        free($3);
    }
    | PINS ':' '[' pin_list ']' ';'
    {
        printf("Component pins defined\n");
    }
    ;

pin_list
    : pin_decl
    | pin_list ',' pin_decl
    ;

pin_decl
    : '{' pin_properties '}'
    {
        printf("Pin defined\n");
    }
    ;

pin_properties
    : pin_property
    | pin_properties ',' pin_property
    ;

pin_property
    : NUM ':' INTEGER
    {
        printf("Pin number: %d\n", $3);
    }
    | NAME ':' IDENTIFIER
    {
        printf("Pin name: %s\n", $3);
        free($3);
    }
    | FUNCTION ':' IDENTIFIER
    {
        printf("Pin function: %s\n", $3);
        free($3);
    }
    ;

place_decl
    : PLACE IDENTIFIER AS IDENTIFIER '{' place_properties '}'
    {
        printf("Placing component %s as %s\n", $2, $4);
        free($2);
        free($4);
    }
    ;

place_properties
    : place_property
    | place_properties place_property
    ;

place_property
    : AT ':' '(' dimension ',' dimension ')' ';'
    {
        printf("Component position: (%.2f, %.2f)\n", $4, $6);
    }
    | ROTATION ':' INTEGER ';'
    {
        printf("Component rotation: %d degrees\n", $3);
    }
    | SIDE ':' side_value ';'
    ;

side_value
    : TOP
    {
        printf("Component side: top\n");
    }
    | BOTTOM
    {
        printf("Component side: bottom\n");
    }
    ;

net_decl
    : NET IDENTIFIER '{' pin_refs '}'
    {
        printf("Net definition: %s\n", $2);
        free($2);
    }
    ;

pin_refs
    : pin_ref
    | pin_refs ',' pin_ref
    ;

pin_ref
    : IDENTIFIER '.' IDENTIFIER
    {
        printf("Pin reference: %s.%s\n", $1, $3);
        free($1);
        free($3);
    }
    | IDENTIFIER '.' NUM '(' INTEGER ')'
    {
        printf("Pin reference: %s.pin(%d)\n", $1, $5);
        free($1);
    }
    ;

dimension
    : INTEGER MM
    {
        $$ = $1;
        printf("Dimension: %d mm\n", $1);
    }
    | FLOAT MM
    {
        $$ = $1;
        printf("Dimension: %.2f mm\n", $1);
    }
    | INTEGER IN
    {
        $$ = $1 * 25.4;
        printf("Dimension: %d in (%.2f mm)\n", $1, $$);
    }
    | FLOAT IN
    {
        $$ = $1 * 25.4;
        printf("Dimension: %.2f in (%.2f mm)\n", $1, $$);
    }
    | INTEGER MIL
    {
        $$ = $1 * 0.0254;
        printf("Dimension: %d mil (%.2f mm)\n", $1, $$);
    }
    | FLOAT MIL
    {
        $$ = $1 * 0.0254;
        printf("Dimension: %.2f mil (%.2f mm)\n", $1, $$);
    }
    ;

%%

void yyerror(const char* s) {
    fprintf(stderr, "Parse error at line %d: %s near '%s'\n", yylineno, s, yytext);
}