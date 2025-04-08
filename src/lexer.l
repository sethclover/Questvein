%{
#include <stdlib.h>
#include <string.h>
#include <stdio.h> /// for printf

#include "parser.tab.h"

extern "C" int yylex();

int curr_context = 0; /* monster = 1, object = 2 */

void yyerror(const char *s);
%}

%option noyywrap

%x MONSTER_STATE OBJECT_STATE DESC_STATE FIELD_STATE

%%
"RLG327 MONSTER DESCRIPTION 1" { 
    curr_context = 1;
    BEGIN(MONSTER_STATE);
    printf("monster header ");
    return HEADER_MONSTER;
}
"RLG327 OBJECT DESCRIPTION 1" { 
    curr_context = 2;
    BEGIN(OBJECT_STATE);
    printf("object header ");
    return HEADER_OBJECT;
}

<MONSTER_STATE,FIELD_STATE,DESC_STATE>[ \t]+    { printf("ws "); }
<MONSTER_STATE,FIELD_STATE,DESC_STATE>\n    { printf("new>\n"); return NEWLINE; }

<MONSTER_STATE>"BEGIN MONSTER"  { printf("begin monster "); return BEGIN_MONSTER; }
<MONSTER_STATE>"NAME"           { printf("name "); BEGIN(FIELD_STATE); return NAME; }
<MONSTER_STATE>"DESC"           { printf("desc "); BEGIN(DESC_STATE); return DESC; }
<MONSTER_STATE>"COLOR"          { printf("color "); BEGIN(FIELD_STATE); return COLOR; }
<MONSTER_STATE>"SPEED"          { printf("speed "); BEGIN(FIELD_STATE); return SPEED; }
<MONSTER_STATE>"ABIL"           { printf("abil "); BEGIN(FIELD_STATE); return ABIL; }
<MONSTER_STATE>"HP"             { printf("hp "); BEGIN(FIELD_STATE); return HP; }
<MONSTER_STATE>"DAM"            { printf("dam "); BEGIN(FIELD_STATE); return DAM; }
<MONSTER_STATE>"SYMB"           { printf("symb "); BEGIN(FIELD_STATE); return SYMB; }
<MONSTER_STATE>"RRTY"           { printf("rrty "); BEGIN(FIELD_STATE); return RRTY; }
<MONSTER_STATE>"END"            { printf("end "); return END_MONSTER; }

<FIELD_STATE>"BLACK"|"RED"|"GREEN"|"YELLOW"|"BLUE"|"MAGENTA"|"CYAN"|"WHITE" { // must be able to get multipe colors
    printf("color=%s ", yytext);
    BEGIN(curr_context == 1 ? MONSTER_STATE : OBJECT_STATE);
    yylval.str = strdup(yytext);
    return COLOR_VAL;
}
<FIELD_STATE>"SMART"|"TELE"|"TUNNEL"|"ERRATIC"|"PASS"|"PICKUP"|"DESTROY"|"UNIQ"|"BOSS" {
    printf("abil=%s ", yytext);
    BEGIN(curr_context == 1 ? MONSTER_STATE : OBJECT_STATE);
    yylval.str = strdup(yytext);
    return ABIL_VAL;
}
<FIELD_STATE>[1-9][0-9]?|100                { printf("rrty=%s ", yytext); BEGIN(curr_context == 1 ? MONSTER_STATE : OBJECT_STATE); yylval.num = atoi(yytext); return RARITY; }         
<FIELD_STATE>[0-9]+\+[0-9]+d[0-9]+          { printf("dice=%s ", yytext); BEGIN(curr_context == 1 ? MONSTER_STATE : OBJECT_STATE); yylval.str = strdup(yytext); return DICE; }
<FIELD_STATE>[&$%+=?/!^a-zA-Z0-9]           { printf("symb=%s ", yytext); BEGIN(curr_context == 1 ? MONSTER_STATE : OBJECT_STATE); yylval.ch = yytext[0]; return SYMBOL; }
<FIELD_STATE>[a-zA-Z][a-zA-Z0-9 -]*         { printf("string=%s ", yytext); BEGIN(curr_context == 1 ? MONSTER_STATE : OBJECT_STATE); yylval.str = strdup(yytext); return STRING; }

<DESC_STATE>^\.  { printf("desc_period "); BEGIN(curr_context == 1 ? MONSTER_STATE : OBJECT_STATE); return PERIOD; }
<DESC_STATE>[^\n]+       { printf("desc_line "); yylval.str = strdup(yytext); return STRING; }

<MONSTER_STATE,FIELD_STATE,DESC_STATE>.     { yyerror("Unknown character"); return ERROR; }

%%
