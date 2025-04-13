%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "pcb_model.h"  /* This include is already present but make sure it's here */
#include "gerber_export.h"

extern int yylex();
extern int yylineno;
extern char* yytext;
extern FILE* yyin;

void yyerror(const char* s);

// Global variables for building the PCB model
Board* current_board = NULL;
Component* current_component = NULL;
Placement* current_placement = NULL;
Net* current_net = NULL;
Pin current_pin;
%}

%union {
    int integer;
    double floatval;
    char* string;
    Point point;
    PinReference pin_ref;
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
%type <pin_ref> pin_ref

%%

program
    : declarations
    {
        printf("CircuitScript parsing completed successfully.\n");
    }
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
    : BOARD '{'
    {
        current_board = create_board();
    }
    board_properties '}'
    {
        printf("Board definition processed: %s\n", 
               current_board->name ? current_board->name : "unnamed");
    }
    ;

board_properties
    : board_property
    | board_properties board_property
    ;

board_property
    : NAME ':' STRING ';'
    {
        if (current_board->name) free(current_board->name);
        current_board->name = $3;
    }
    | WIDTH ':' dimension ';'
    {
        current_board->width = $3;
    }
    | HEIGHT ':' dimension ';'
    {
        current_board->height = $3;
    }
    | LAYERS ':' INTEGER ';'
    {
        current_board->layers = $3;
    }
    ;

component_decl
    : COMPONENT IDENTIFIER 
    {
        current_component = create_component($2);
    }
    '{' component_properties '}'
    {
        add_component(current_board, current_component);
        printf("Component definition processed: %s\n", current_component->name);
        current_component = NULL;
    }
    ;

component_properties
    : component_property
    | component_properties component_property
    ;

component_property
    : PACKAGE ':' IDENTIFIER ';'
    {
        if (current_component->package) free(current_component->package);
        current_component->package = $3;
    }
    | MANUFACTURER ':' STRING ';'
    {
        if (current_component->manufacturer) free(current_component->manufacturer);
        current_component->manufacturer = $3;
    }
    | MPN ':' STRING ';'
    {
        if (current_component->mpn) free(current_component->mpn);
        current_component->mpn = $3;
    }
    | PINS ':' '[' 
    {
        // Reset the pin array
        if (current_component->pins) {
            for (int i = 0; i < current_component->pin_count; i++) {
                if (current_component->pins[i].name) free(current_component->pins[i].name);
                if (current_component->pins[i].function) free(current_component->pins[i].function);
            }
            free(current_component->pins);
            current_component->pins = NULL;
            current_component->pin_count = 0;
            current_component->pin_capacity = 0;
        }
    }
    pin_list ']' ';'
    {
        printf("Component pins processed: %d pins defined\n", current_component->pin_count);
    }
    ;

pin_list
    : pin_decl
    | pin_list ',' pin_decl
    ;

pin_decl
    : '{'
    {
        // Initialize the current pin
        memset(&current_pin, 0, sizeof(Pin));
        current_pin.number = -1;
    }
    pin_properties '}'
    {
        add_pin_to_component(current_component, current_pin);
    }
    ;

pin_properties
    : pin_property
    | pin_properties ',' pin_property
    ;

pin_property
    : NUM ':' INTEGER
    {
        current_pin.number = $3;
    }
    | NAME ':' IDENTIFIER
    {
        if (current_pin.name) free(current_pin.name);
        current_pin.name = $3;
    }
    | FUNCTION ':' IDENTIFIER
    {
        if (current_pin.function) free(current_pin.function);
        current_pin.function = $3;
    }
    ;

place_decl
    : PLACE IDENTIFIER 
    {
        current_placement = create_placement();
        current_placement->component_name = $2;
    }
    AS IDENTIFIER 
    {
        current_placement->ref_designator = $5;
    }
    '{' place_properties '}'
    {
        add_placement(current_board, current_placement);
        printf("Placement processed: %s as %s\n", 
               current_placement->component_name, 
               current_placement->ref_designator);
        current_placement = NULL;
    }
    ;

place_properties
    : place_property
    | place_properties place_property
    ;

place_property
    : AT ':' '(' dimension ',' dimension ')' ';'
    {
        current_placement->position.x = $4;
        current_placement->position.y = $6;
    }
    | ROTATION ':' INTEGER ';'
    {
        current_placement->rotation = $3;
    }
    | SIDE ':' side_value ';'
    ;

side_value
    : TOP
    {
        current_placement->top_side = true;
    }
    | BOTTOM
    {
        current_placement->top_side = false;
    }
    ;

net_decl
    : NET IDENTIFIER 
    {
        current_net = create_net($2);
    }
    '{' pin_refs '}'
    {
        add_net(current_board, current_net);
        printf("Net processed: %s with %d connections\n", 
               current_net->name, current_net->connection_count);
        current_net = NULL;
    }
    ;

pin_refs
    : pin_ref
    {
        add_connection_to_net(current_net, $1);
    }
    | pin_refs ',' pin_ref
    {
        add_connection_to_net(current_net, $3);
    }
    ;

pin_ref
    : IDENTIFIER '.' IDENTIFIER
    {
        $$.instance = $1;
        $$.pin_name = $3;
        $$.pin_number = -1;
    }
    | IDENTIFIER '.' IDENTIFIER '(' INTEGER ')'
    {
        $$.instance = $1;
        $$.pin_name = $3;
        $$.pin_number = $5;
    }
    ;

dimension
    : INTEGER MM
    {
        $$ = $1;
    }
    | FLOAT MM
    {
        $$ = $1;
    }
    | INTEGER IN
    {
        $$ = $1 * 25.4; // Convert inches to mm
    }
    | FLOAT IN
    {
        $$ = $1 * 25.4; // Convert inches to mm
    }
    | INTEGER MIL
    {
        $$ = $1 * 0.0254; // Convert mils to mm
    }
    | FLOAT MIL
    {
        $$ = $1 * 0.0254; // Convert mils to mm
    }
    ;

%%

void yyerror(const char* s) {
    fprintf(stderr, "Parse error at line %d: %s near '%s'\n", yylineno, s, yytext);
}