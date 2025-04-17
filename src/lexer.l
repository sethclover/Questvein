%{
#include <string.h>

#include "parser.tab.h"

extern "C" int yylex();

int curr_context = 0;
%}

%option noyywrap nounput

%x MONSTER_STATE OBJECT_STATE LIST_STATE FIELD_STATE DESC_STATE BOOL_STATE

%%
"RLG327 MONSTER DESCRIPTION 1\n" { 
    curr_context = 1;
    return HEADER_MONSTER;
}
"RLG327 OBJECT DESCRIPTION 1\n" { 
    curr_context = 2;
    return HEADER_OBJECT;
}
[ \t\n]+                                                                 ;
<MONSTER_STATE,OBJECT_STATE,LIST_STATE,FIELD_STATE,DESC_STATE>[ \t]+     ;
<MONSTER_STATE,OBJECT_STATE,FIELD_STATE>\n                               { return NEWLINE; }

"BEGIN MONSTER\n"               { BEGIN(MONSTER_STATE); return BEGIN_MONSTER; }
"BEGIN OBJECT\n"                {  BEGIN(OBJECT_STATE); return BEGIN_OBJECT; }

<MONSTER_STATE,OBJECT_STATE>"DESC\n"         { BEGIN(DESC_STATE); return DESC; }
<MONSTER_STATE,OBJECT_STATE>"NAME"           { BEGIN(FIELD_STATE); return NAME; }
<MONSTER_STATE,OBJECT_STATE>"COLOR"          { BEGIN(LIST_STATE); return COLOR; }
<MONSTER_STATE,OBJECT_STATE>"SPEED"          { BEGIN(FIELD_STATE); return SPEED; }
<MONSTER_STATE,OBJECT_STATE>"DAM"            { BEGIN(FIELD_STATE); return DAM; }
<MONSTER_STATE,OBJECT_STATE>"RRTY"           { BEGIN(FIELD_STATE); return RRTY; }

<MONSTER_STATE>"ABIL"           { BEGIN(LIST_STATE); return ABIL; }
<MONSTER_STATE>"HP"             { BEGIN(FIELD_STATE); return HP; }
<MONSTER_STATE>"SYMB"           { BEGIN(FIELD_STATE); return SYMB; }
<MONSTER_STATE>"END"            { BEGIN(INITIAL); return END_MONSTER; }

<OBJECT_STATE>"TYPE"            { BEGIN(LIST_STATE); return TYPE; }
<OBJECT_STATE>"HIT"             { BEGIN(FIELD_STATE); return HIT; }
<OBJECT_STATE>"DODGE"           { BEGIN(FIELD_STATE); return DODGE; }
<OBJECT_STATE>"DEF"             { BEGIN(FIELD_STATE); return DEF; }
<OBJECT_STATE>"WEIGHT"          { BEGIN(FIELD_STATE); return WEIGHT; }
<OBJECT_STATE>"ATTR"            { BEGIN(FIELD_STATE); return ATTR; }
<OBJECT_STATE>"VAL"             { BEGIN(FIELD_STATE); return VAL; }
<OBJECT_STATE>"ART"             { BEGIN(BOOL_STATE); return ART; }
<OBJECT_STATE>"END"             { BEGIN(INITIAL); return END_OBJECT; }

<BOOL_STATE>"TRUE"              { BEGIN(OBJECT_STATE); return ART_VAL_TRUE; }
<BOOL_STATE>"FALSE"             { BEGIN(OBJECT_STATE); return ART_VAL_FALSE; }

<LIST_STATE>"BLACK"|"RED"|"GREEN"|"YELLOW"|"BLUE"|"MAGENTA"|"CYAN"|"WHITE" {
    yylval.str = strdup(yytext);
    return COLOR_VAL;
}
<LIST_STATE>"SMART"|"TELE"|"TUNNEL"|"ERRATIC"|"PASS"|"PICKUP"|"DESTROY"|"UNIQ"|"BOSS" {
    yylval.str = strdup(yytext);
    return ABIL_VAL;
}
<LIST_STATE>"WEAPON"|"OFFHAND"|"RANGED"|"ARMOR"|"HELMET"|"CLOAK"|"GLOVES"|"BOOTS"|"RING"|"AMULET"|"LIGHT"|"SCROLL"|"BOOK"|"FLASK"|"GOLD"|"AMMUNITION"|"FOOD"|"WAND"|"CONTAINER" {
    yylval.str = strdup(yytext);
    return TYPE_VAL;
}
<LIST_STATE>\n      { BEGIN(curr_context == 1 ? MONSTER_STATE : OBJECT_STATE); return NEWLINE; }

<FIELD_STATE>[1-9][0-9]?|100                { BEGIN(curr_context == 1 ? MONSTER_STATE : OBJECT_STATE); yylval.num = atoi(yytext); return RARITY; }         
<FIELD_STATE>[0-9]+\+[0-9]+d[0-9]+          { BEGIN(curr_context == 1 ? MONSTER_STATE : OBJECT_STATE); yylval.str = strdup(yytext); return DICE; }
<FIELD_STATE>[&$%+=?/!^a-zA-Z0-9]           { BEGIN(curr_context == 1 ? MONSTER_STATE : OBJECT_STATE); yylval.ch = yytext[0]; return SYMBOL; }
<FIELD_STATE>[a-zA-Z].*                     { BEGIN(curr_context == 1 ? MONSTER_STATE : OBJECT_STATE); yylval.str = strdup(yytext); return STRING; }

<DESC_STATE>^\.\n           { BEGIN(curr_context == 1 ? MONSTER_STATE : OBJECT_STATE); return PERIOD; }
<DESC_STATE>.*\n            { yylval.str = strdup(yytext); return STRING; }
%%
