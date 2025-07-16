/*                                                                          */
/* File Name    : subc.l                                                    */
/* Description  : a skeleton flex input                                     */
/*                                                                          */

%{

/* C Code Section */
/* Any code placed inside the brackets will be literally copied into the generated C file. */

#include "subc.h"
#include "subc.tab.h"
static int lineno = 1;
static char* file_name;

%}

/* Definitions Section */
/* This section contains simple name definitions and declarations of start conditions. */

letter        [A-Za-z_]
digit         [0-9]
id            {letter}({letter}|{digit})*
integer_const ([1-9]{digit}*)|"0"
whitespace    [ \t]
special_char  \\[nt]

%x COMMENT

%%

  /* Rules Section */
  /* This section contains series of rules for recognizing tokens. */

  /* Keywords */
"int"|"char"                    { return TYPE; }
"struct"                        { return STRUCT; } 
"NULL"                          { return SYM_NULL; }
"return"                        { return RETURN; }
"if"                            { return IF; }
"else"                          { return ELSE; }
"while"                         { return WHILE; }
"for"                           { return FOR; }
"break"                         { return BREAK; }
"continue"                      { return CONTINUE; }

  /* Identifiers */
{id}                            {
                                  return ID;
                                }

  /* Integer constants */
{integer_const}                 {
                                  return INTEGER_CONST;
                                }

  /* String variables */
\"({special_char}|[^\\\"\n])*\" {
                                  return STRING;
                                }

  /* Character variables */
'({special_char}|[^\\'\n])'     {    
                                  return CHAR_CONST;
                                }

  /* Operators */
","|";"|"("|")"|"["|"]"|"{"|"}" { return yytext[0]; }
"!"|"*"|"/"|"%"|"+"|"-"|"&"     { return yytext[0]; }
"="|"."                         { return yytext[0]; }

"&&"                            { return LOGICAL_AND; }
"||"                            { return LOGICAL_OR; }
"<"|"<="|">"|">="               { return RELOP; }
"=="|"!="                       { return EQUOP; }
"++"                            { return INCOP; }
"--"                            { return DECOP; }
"->"                            { return STRUCTOP; }

  /* Whitespaces */
{whitespace}


  /* Comment blocks */
"/*"                            {
                                  BEGIN (COMMENT);
                                }
<COMMENT>"*/"                   { 
                                  BEGIN (INITIAL);
                                }
<COMMENT>.

  /* Line number tracking */
<*>\n       { ++lineno; }

%%
   
/* User Code Section */
/* The user code section is simply copied to lex.yy.c verbatim. */

int get_lineno() {
   return lineno;
}

int main(int argc, char* argv[]) {
  if(argc >= 2) {
    yyin = fopen(argv[1], "r");
  } else {
    yyin = stdin;
  }

  if(!yyin) {
    printf("Can't open input stream!\n");
    exit(1);
  }
  
  yyparse();
  fclose(yyin);

  return 0;
}
