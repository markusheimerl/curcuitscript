%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pcb_model.h"
#include "parser.h"

void yyerror(const char *s);
%}

%option noyywrap

DIGIT       [0-9]
INTEGER     {DIGIT}+
FLOAT       {INTEGER}\.{INTEGER}
IDENTIFIER  [a-zA-Z][a-zA-Z0-9_]*
STRING      \"[^\"]*\"
WHITESPACE  [ \t\n\r]
COMMENT     \/\/[^\n]*

%%

{WHITESPACE}    { /* Ignore whitespace */ }
{COMMENT}       { /* Ignore comments */ }

"board"         { return BOARD; }
"component"     { return COMPONENT; }
"place"         { return PLACE; }
"net"           { return NET; }
"as"            { return AS; }
"at"            { return AT; }
"rotation"      { return ROTATION; }
"side"          { return SIDE; }
"top"           { return TOP; }
"bottom"        { return BOTTOM; }
"width"         { return WIDTH; }
"height"        { return HEIGHT; }
"layers"        { return LAYERS; }
"package"       { return PACKAGE; }
"pins"          { return PINS; }
"name"          { return NAME; }
"num"           { return NUM; }
"function"      { return FUNCTION; }
"manufacturer"  { return MANUFACTURER; }
"mpn"           { return MPN; }

"{"             { return '{'; }
"}"             { return '}'; }
"["             { return '['; }
"]"             { return ']'; }
"("             { return '('; }
")"             { return ')'; }
","             { return ','; }
":"             { return ':'; }
";"             { return ';'; }
"."             { return '.'; }

"mm"            { return MM; }
"in"            { return IN; }
"mil"           { return MIL; }

{INTEGER}       { 
                  yylval.integer = atoi(yytext);
                  return INTEGER;
                }

{FLOAT}         {
                  yylval.floatval = atof(yytext);
                  return FLOAT;
                }

{IDENTIFIER}    {
                  yylval.string = strdup(yytext);
                  return IDENTIFIER;
                }

{STRING}        {
                  yytext[strlen(yytext)-1] = '\0';
                  yylval.string = strdup(yytext+1);
                  return STRING;
                }

.               { yyerror("Unexpected character"); }

%%