%{
#include <cstring>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "parser.hpp"

extern "C" int yylex();
extern "C" int yyparse();
extern "C" FILE *yyin;

std::vector<MonsterType> monsterTypeList;
std::vector<ObjectType> objectTypeList;
MonsterType curr_monster;
ObjectType curr_object;

int parse(const char *filename) {
    yyin = fopen(filename, "r");
    if (!yyin) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return 1;
    }
    yyparse();
    if (yyin) {
        fclose(yyin);
    }

    return 0;
}

void yyerror(const char *s) {
    std::cerr << "Parse error: " << s << std::endl;
}

void reset_monster() {
    curr_monster = MonsterType();
}

void reset_object() {
    curr_object = ObjectType();
}
%}

%code requires {
    #include <string>
    #include <vector>
}

%union {
    int num;
    char ch;
    char *str;
    bool bo;
    std::vector<std::string> *str_vec;
}

%token <str> HEADER_MONSTER HEADER_OBJECT BEGIN_MONSTER END_MONSTER BEGIN_OBJECT END_OBJECT
%token <str> NAME DESC COLOR SPEED ABIL HP DAM SYMB RRTY TYPE HIT DODGE DEF WEIGHT ATTR VAL ART
%token <str> COLOR_VAL ABIL_VAL TYPE_VAL DICE STRING
%token <ch> SYMBOL
%token <num> RARITY
%token <bo> ART_VAL
%token NEWLINE PERIOD 

%type <str_vec> color_content abil_content val_content
%type <str> desc_content

%%

file: HEADER_MONSTER monsters   ;
    | HEADER_OBJECT objects     ;

monsters:
        | monsters monster
        | monsters  error { 
            std::cerr << "Error in monster definition, discarding" << std::endl; 
            reset_monster(); 
        }
        ;

objects:
        | objects object
        | objects  error { 
            std::cerr << "Error in object definition, discarding" << std::endl; 
            reset_object(); 
        }
        ;

monster: BEGIN_MONSTER monster_fields END_MONSTER {
            if (curr_monster.fields.size() != 9 || !curr_monster.valid) {
                curr_monster.valid = false;
                std::cerr << "Parse error: Invalid monster (incomplete or duplicate), discarding" << std::endl;
            }
            else {
                monsterTypeList.push_back(curr_monster);
            }
            reset_monster();
        }

object: BEGIN_OBJECT object_fields END_OBJECT {
            if (curr_object.fields.size() != 14 || !curr_object.valid) {
                curr_object.valid = false;
                std::cerr << "Parse error: Invalid object" << std::endl;
            }
            objectTypeList.push_back(curr_object);
            reset_object();
        }

monster_fields:
              | monster_fields monster_field
              ;

object_fields:
             | object_fields object_field
             ;

monster_field: NAME STRING NEWLINE {
                  if (!curr_monster.fields.insert("NAME").second) curr_monster.valid = false;
                  curr_monster.name = $2;
                  free($2);
              }
              | DESC desc_content PERIOD {
                  if (!curr_monster.fields.insert("DESC").second) curr_monster.valid = false;
                  curr_monster.desc = $2;
                  free($2);
              }
              | COLOR color_content NEWLINE {
                  if (!curr_monster.fields.insert("COLOR").second) curr_monster.valid = false;
                  curr_monster.colors = *$2;
                  delete $2;
              }
              | SPEED DICE NEWLINE {
                  if (!curr_monster.fields.insert("SPEED").second) curr_monster.valid = false;
                  int set = sscanf($2, "%d+%dd%d", &curr_monster.speed.base, &curr_monster.speed.rolls, &curr_monster.speed.sides);
                  if (set != 3) {
                        curr_monster.valid = false;
                        std::cerr << "Parse error: Invalid Speed format" << std::endl;
                  }
                  free($2);
              }
              | ABIL abil_content NEWLINE {
                  if (!curr_monster.fields.insert("ABIL").second) curr_monster.valid = false;
                  curr_monster.abils = *$2;
                  delete $2;
              }
              | HP DICE NEWLINE {
                  if (!curr_monster.fields.insert("HP").second) curr_monster.valid = false;
                  int set = sscanf($2, "%d+%dd%d", &curr_monster.hp.base, &curr_monster.hp.rolls, &curr_monster.hp.sides);
                  if (set != 3) {
                        curr_monster.valid = false;
                        std::cerr << "Parse error: Invalid HP format" << std::endl;
                  }
                  free($2);
              }
              | DAM DICE NEWLINE {
                  if (!curr_monster.fields.insert("DAM").second) curr_monster.valid = false;
                  int set = sscanf($2, "%d+%dd%d", &curr_monster.dam.base, &curr_monster.dam.rolls, &curr_monster.dam.sides);
                  if (set != 3) {
                        curr_monster.valid = false;
                        std::cerr << "Parse error: Invalid Damage format" << std::endl;
                  }
                  free($2);
              }
              | SYMB SYMBOL NEWLINE {
                  if (!curr_monster.fields.insert("SYMB").second) curr_monster.valid = false;
                  curr_monster.symbol = $2;
              }
              | RRTY RARITY NEWLINE {
                  if (!curr_monster.fields.insert("RRTY").second) curr_monster.valid = false;
                  curr_monster.rarity = $2;
              }
              | error NEWLINE { 
                  std::cerr << "Error in field, monster is invalid" << std::endl; 
                  curr_monster.valid = false; 
              }
              ;

object_field: NAME STRING NEWLINE {
                  if (!curr_object.fields.insert("NAME").second) curr_object.valid = false;
                  curr_object.name = $2;
                  free($2);
              }
              | DESC desc_content PERIOD {
                  if (!curr_object.fields.insert("DESC").second) curr_object.valid = false;
                  curr_object.desc = $2;
                  free($2);
              }
              | TYPE val_content NEWLINE {
                  if (!curr_object.fields.insert("TYPE").second) curr_object.valid = false;
                  curr_object.type = *$2;
                  delete $2;
              }
              | COLOR color_content NEWLINE {
                  if (!curr_object.fields.insert("COLOR").second) curr_object.valid = false;
                  curr_object.colors = *$2;
                  delete $2;
              }
              | HIT DICE NEWLINE {
                  if (!curr_object.fields.insert("HIT").second) curr_object.valid = false;
                  int set = sscanf($2, "%d+%dd%d", &curr_object.hit.base, &curr_object.hit.rolls, &curr_object.hit.sides);
                  if (set != 3) {
                        curr_monster.valid = false;
                        std::cerr << "Parse error: Invalid Hit format" << std::endl;
                  }
                  free($2);
              }
              | DAM DICE NEWLINE {
                  if (!curr_object.fields.insert("DAM").second) curr_object.valid = false;
                  int set = sscanf($2, "%d+%dd%d", &curr_object.dam.base, &curr_object.dam.rolls, &curr_object.dam.sides);
                  if (set != 3) {
                        curr_monster.valid = false;
                        std::cerr << "Parse error: Invalid Damage format" << std::endl;
                  }
                  free($2);
              }
              | DODGE DICE NEWLINE {
                  if (!curr_object.fields.insert("DODGE").second) curr_object.valid = false;
                  int set = sscanf($2, "%d+%dd%d", &curr_object.dodge.base, &curr_object.dodge.rolls, &curr_object.dodge.sides);
                  if (set != 3) {
                        curr_monster.valid = false;
                        std::cerr << "Parse error: Invalid Dodge format" << std::endl;
                  }
                  free($2);
              }
              | DEF DICE NEWLINE {
                  if (!curr_object.fields.insert("DEF").second) curr_object.valid = false;
                  int set = sscanf($2, "%d+%dd%d", &curr_object.def.base, &curr_object.def.rolls, &curr_object.def.sides);
                  if (set != 3) {
                        curr_monster.valid = false;
                        std::cerr << "Parse error: Invalid Defense format" << std::endl;
                  }
                  free($2);
              }
              | WEIGHT DICE NEWLINE {
                  if (!curr_object.fields.insert("WEIGHT").second) curr_object.valid = false;
                  int set = sscanf($2, "%d+%dd%d", &curr_object.weight.base, &curr_object.weight.rolls, &curr_object.weight.sides);
                  if (set != 3) {
                        curr_monster.valid = false;
                        std::cerr << "Parse error: Invalid Weight format" << std::endl;
                  }
                  free($2);
              }
              | SPEED DICE NEWLINE {
                  if (!curr_object.fields.insert("SPEED").second) curr_object.valid = false;
                  int set = sscanf($2, "%d+%dd%d", &curr_object.speed.base, &curr_object.speed.rolls, &curr_object.speed.sides);
                  if (set != 3) {
                        curr_monster.valid = false;
                        std::cerr << "Parse error: Invalid Speed format" << std::endl;
                  }
                  free($2);
              }
              | ATTR DICE NEWLINE {
                  if (!curr_object.fields.insert("ATTR").second) curr_object.valid = false;
                  int set = sscanf($2, "%d+%dd%d", &curr_object.attr.base, &curr_object.attr.rolls, &curr_object.attr.sides);
                  if (set != 3) {
                        curr_monster.valid = false;
                        std::cerr << "Parse error: Invalid Attribute format" << std::endl;
                  }
                  free($2);
              }
              | VAL DICE NEWLINE {
                  if (!curr_object.fields.insert("VAL").second) curr_object.valid = false;
                  int set = sscanf($2, "%d+%dd%d", &curr_object.val.base, &curr_object.val.rolls, &curr_object.val.sides);
                  if (set != 3) {
                        curr_monster.valid = false;
                        std::cerr << "Parse error: Invalid Value format" << std::endl;
                  }
                  free($2);
              }
              | ART ART_VAL NEWLINE {
                  if (!curr_object.fields.insert("ART").second) curr_object.valid = false;
                  curr_object.art = $2;
              }
              | RRTY RARITY NEWLINE {
                  if (!curr_object.fields.insert("RRTY").second) curr_object.valid = false;
                  curr_object.rarity = $2;
              }
              | error NEWLINE { 
                  std::cerr << "Error in field, object is invalid" << std::endl; 
                  curr_object.valid = false; 
              }
              ;

desc_content: STRING { $$ = strdup($1); free($1); }
              | desc_content STRING {
                  std::string temp = std::string($1) + $2;
                  $$ = strdup(temp.c_str());
                  free($1);
                  free($2);
              }
              ;

color_content: COLOR_VAL {
                $$ = new std::vector<std::string>();
                $$->push_back($1);
                free($1);
             }
             | color_content COLOR_VAL {
                $$ = $1;
                $$->push_back($2);
                free($2);
             }
             ;

abil_content:  /* empty */ {
                $$ = new std::vector<std::string>();
            }
            | ABIL_VAL {
                $$ = new std::vector<std::string>();
                $$->push_back($1);
                free($1);
            }
            | abil_content ABIL_VAL {
                $$ = $1;
                $$->push_back($2);
                free($2);
            }
            ;

val_content: TYPE_VAL {
                $$ = new std::vector<std::string>();
                $$->push_back($1);
                free($1);
            }
            | val_content TYPE_VAL {
                $$ = $1;
                $$->push_back($2);
                free($2);
            }
%%
