%{
#include <cstring>
#include <iostream>
#include <set>
#include <string>
#include <vector>

extern "C" int yylex();
extern "C" int yyparse();
extern "C" FILE *yyin;

void yyerror(const char *s);

struct Monster { //import from dungeon.h
    std::string name, desc, color, speed, abil, hp, dam;
    char symbol = '\0';
    int rarity = 0;
    bool valid = true;
    std::set<std::string> fields;
};

struct Object { // add and import from dungeon.h
    std::string name, desc, type, color, hit, dam, dodge, def, weight, speed, attr, val, art;
    int rarity = 0;
    bool valid = true;
    std::set<std::string> fields;
};

std::vector<Monster> monsters;
std::vector<Object> objects;
Monster current_monster;
Object curr_object;

void reset_monster() {
    current_monster = Monster();
}

void reset_object() {
    curr_object = Object();
}

void print_monsters() {
    std::cout << "---\n";
    for (const auto& m : monsters) {
        if (m.valid) {
            std::cout << "Name: " << m.name << "\n";
            std::cout << "Description: " << m.desc << "\n";
            std::cout << "Color: " << m.color << "\n";
            std::cout << "Speed: " << m.speed << "\n";
            std::cout << "Abilities: " << m.abil << "\n";
            std::cout << "Hitpoints: " << m.hp << "\n";
            std::cout << "Attack Damage: " << m.dam << "\n";
            std::cout << "Symbol: " << m.symbol << "\n";
            std::cout << "Rarity: " << m.rarity << "\n---\n";
        }
    }
}

void print_objects() {
    std::cout << "---\n";
    for (const auto& o : objects) {
        if (o.valid) {
            std::cout << "Name: " << o.name << "\n";
            std::cout << "Description: " << o.desc << "\n";
            std::cout << "Type: " << o.type << "\n";
            std::cout << "Color: " << o.color << "\n";
            std::cout << "Hit bonus: " << o.hit << "\n";
            std::cout << "Dagmage bonus: " << o.dam << "\n";
            std::cout << "Dodge bonus: " << o.dodge << "\n";
            std::cout << "Defense bonus: " << o.def << "\n";
            std::cout << "Weight: " << o.weight << "\n";
            std::cout << "Speed bonus: " << o.speed << "\n";
            std::cout << "Special Attribute: " << o.attr << "\n";
            std::cout << "Value: " << o.val << "\n";
            std::cout << "Artifact status: " << o.art << "\n";
            std::cout << "Rarity: " << o.rarity << "\n---\n";
        }
    }
}
%}

%union {
    int num;
    char ch;
    char *str;
}

%token <str> HEADER_MONSTER HEADER_OBJECT BEGIN_MONSTER END_MONSTER BEGIN_OBJECT END_OBJECT
%token <str> NAME DESC COLOR SPEED ABIL HP DAM SYMB RRTY TYPE HIT DODGE DEF WEIGHT ATTR VAL ART
%token <str> COLOR_VAL ABIL_VAL TYPE_VAL ART_VAL DICE STRING
%token <ch> SYMBOL
%token <num> RARITY
%token NEWLINE PERIOD ERROR    

%type <str> desc_content

%%

file: HEADER_MONSTER NEWLINE monsters { print_monsters(); }
    | HEADER_OBJECT NEWLINE objects { print_objects(); }
    | error { std::cerr << "Invalid header\n"; exit(1); }
    ;

monsters:
        | monsters opt_newlines monster
        ;

objects:
        | objects object
        ;

monster: BEGIN_MONSTER NEWLINE monster_fields END_MONSTER {
            if (current_monster.fields.size() != 9 || !current_monster.valid) {
                current_monster.valid = false;
                std::cerr << "Parse error: Invalid monster\n";
            }
            monsters.push_back(current_monster);
            reset_monster();
        }
        ;

object: BEGIN_OBJECT NEWLINE object_fields END_OBJECT {
            if (curr_object.fields.size() != 13 || !curr_object.valid) {
                curr_object.valid = false;
                std::cerr << "Parse error: Invalid object\n";
            }
            objects.push_back(curr_object);
            reset_object();
        }
        | BEGIN_OBJECT object_fields ERROR {
            curr_object.valid = false;
            std::cerr << "Parse error: Invalid object\n";
            reset_object();
            while (yylex() != BEGIN_MONSTER && yylex() != BEGIN_OBJECT && yylex() != 0);
        }
        ;

monster_fields:
              | monster_fields monster_field
              ;

object_fields:
             | object_fields object_field
             ;

monster_field: NAME STRING NEWLINE {
                  if (!current_monster.fields.insert("NAME").second) current_monster.valid = false;
                  current_monster.name = $2;
                  free($2);
              }
              | DESC NEWLINE desc_content NEWLINE PERIOD NEWLINE {
                  if (!current_monster.fields.insert("DESC").second) current_monster.valid = false;
                  current_monster.desc = $3;
                  free($3);
              }
              | COLOR COLOR_VAL NEWLINE {
                  if (!current_monster.fields.insert("COLOR").second) current_monster.valid = false;
                  current_monster.color = $2;
                  free($2);
              }
              | SPEED DICE NEWLINE {
                  if (!current_monster.fields.insert("SPEED").second) current_monster.valid = false;
                  current_monster.speed = $2;
                  free($2);
              }
              | ABIL ABIL_VAL NEWLINE {
                  if (!current_monster.fields.insert("ABIL").second) current_monster.valid = false;
                  current_monster.abil = $2;
                  free($2);
              }
              | HP DICE NEWLINE {
                  if (!current_monster.fields.insert("HP").second) current_monster.valid = false;
                  current_monster.hp = $2;
                  free($2);
              }
              | DAM DICE NEWLINE {
                  if (!current_monster.fields.insert("DAM").second) current_monster.valid = false;
                  current_monster.dam = $2;
                  free($2);
              }
              | SYMB SYMBOL NEWLINE {
                  if (!current_monster.fields.insert("SYMB").second) current_monster.valid = false;
                  current_monster.symbol = $2;
              }
              | RRTY RARITY NEWLINE {
                  if (!current_monster.fields.insert("RRTY").second) current_monster.valid = false;
                  current_monster.rarity = $2;
              }
              ;

object_field: NAME STRING NEWLINE {
                  if (!curr_object.fields.insert("NAME").second) curr_object.valid = false;
                  curr_object.name = $2;
                  free($2);
              }
              | DESC NEWLINE desc_content NEWLINE PERIOD NEWLINE {
                  if (!curr_object.fields.insert("DESC").second) curr_object.valid = false;
                  curr_object.desc = $3;
                  free($3);
              }
              | TYPE TYPE_VAL NEWLINE {
                  if (!curr_object.fields.insert("TYPE").second) curr_object.valid = false;
                  curr_object.type = $2;
                  free($2);
              }
              | COLOR COLOR_VAL NEWLINE {
                  if (!curr_object.fields.insert("COLOR").second) curr_object.valid = false;
                  curr_object.color = $2;
                  free($2);
              }
              | HIT DICE NEWLINE {
                  if (!curr_object.fields.insert("HIT").second) curr_object.valid = false;
                  curr_object.hit = $2;
                  free($2);
              }
              | DAM DICE NEWLINE {
                  if (!curr_object.fields.insert("DAM").second) curr_object.valid = false;
                  curr_object.dam = $2;
                  free($2);
              }
              | DODGE DICE NEWLINE {
                  if (!curr_object.fields.insert("DODGE").second) curr_object.valid = false;
                  curr_object.dodge = $2;
                  free($2);
              }
              | DEF DICE NEWLINE {
                  if (!curr_object.fields.insert("DEF").second) curr_object.valid = false;
                  curr_object.def = $2;
                  free($2);
              }
              | WEIGHT DICE NEWLINE {
                  if (!curr_object.fields.insert("WEIGHT").second) curr_object.valid = false;
                  curr_object.weight = $2;
                  free($2);
              }
              | SPEED DICE NEWLINE {
                  if (!curr_object.fields.insert("SPEED").second) curr_object.valid = false;
                  curr_object.speed = $2;
                  free($2);
              }
              | ATTR DICE NEWLINE {
                  if (!curr_object.fields.insert("ATTR").second) curr_object.valid = false;
                  curr_object.attr = $2;
                  free($2);
              }
              | VAL DICE NEWLINE {
                  if (!curr_object.fields.insert("VAL").second) curr_object.valid = false;
                  curr_object.val = $2;
                  free($2);
              }
              | ART ART_VAL NEWLINE {
                  if (!curr_object.fields.insert("ART").second) curr_object.valid = false;
                  curr_object.art = $2;
                  free($2);
              }
              | RRTY RARITY NEWLINE {
                  if (!curr_object.fields.insert("RRTY").second) curr_object.valid = false;
                  curr_object.rarity = $2;
              }
              ;

desc_content: STRING { $$ = strdup($1); free($1); }
            | desc_content NEWLINE STRING {
                std::string temp = std::string($1) + "\n" + $3;
                $$ = strdup(temp.c_str());
                free($1);
                free($3);
            }
            ;

opt_newlines:
            | opt_newlines NEWLINE
            ;
%%

int main(int argc, char **argv) {
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            std::cerr << "Cannot open file: " << argv[1] << "\n";
            return 1;
        }
    }
    yyparse();
    if (yyin) fclose(yyin);
    return 0;
}

void yyerror(const char *s) {
    std::cerr << "Parse error: " << s << "\n";
}
