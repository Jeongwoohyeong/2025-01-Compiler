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
"int"|"char"                    { yylval.stringVal = (char*)malloc(strlen(yytext) + 1);
                                  if(!yylval.stringVal)
                                  {
                                    printf(".l : TYPE token Mem Fail\n");
                                    exit(1);
                                  }
                                  strcpy(yylval.stringVal, yytext);                                  
                                  return TYPE; 
                                }
"struct"                        { yylval.stringVal = (char*)malloc(strlen(yytext) + 1);
                                  if(!yylval.stringVal)
                                  {
                                    printf(".l : STRUCT token Mem Fail\n");
                                    exit(1);
                                  }
                                  strcpy(yylval.stringVal, yytext);
                                  return STRUCT; 
                                } 
"NULL"                          { yylval.stringVal = (char*)malloc(strlen(yytext) + 1);
                                  if(!yylval.stringVal)
                                  {
                                    printf(".l : SYM_NULL token Mem Fail\n");
                                    exit(1);
                                  }
                                  strcpy(yylval.stringVal, yytext);
                                  return SYM_NULL;
                                }
"return"                        { yylval.stringVal = (char*)malloc(strlen(yytext) + 1);
                                  if(!yylval.stringVal)
                                  {
                                    printf(".l : RETURN token Mem Fail\n");
                                    exit(1);
                                  }
                                  strcpy(yylval.stringVal, yytext);
                                  return RETURN; 
                                }
"if"                            { yylval.stringVal = (char*)malloc(strlen(yytext) + 1);
                                  if(!yylval.stringVal)
                                  {
                                    printf(".l : IF token Mem Fail\n");
                                    exit(1);
                                  }
                                  strcpy(yylval.stringVal, yytext);
                                  return IF;
                                }
"else"                          { yylval.stringVal = (char*)malloc(strlen(yytext) + 1);
                                  if(!yylval.stringVal)
                                  {
                                    printf(".l : ELSE token Mem Fail\n");
                                    exit(1);
                                  }
                                  strcpy(yylval.stringVal, yytext);
                                  return ELSE;
                                }
"while"                         { yylval.stringVal = (char*)malloc(strlen(yytext) + 1);
                                  if(!yylval.stringVal)
                                  {
                                    printf(".l : WHILE token Mem Fail\n");
                                    exit(1);
                                  }
                                  strcpy(yylval.stringVal, yytext);
                                  return WHILE;
                                }
"for"                           { yylval.stringVal = (char*)malloc(strlen(yytext) + 1);
                                  if(!yylval.stringVal)
                                  {
                                    printf(".l : FOR token Mem Fail\n");
                                    exit(1);
                                  }
                                  strcpy(yylval.stringVal, yytext);
                                  return FOR;
                                }
"break"                         { yylval.stringVal = (char*)malloc(strlen(yytext) + 1);
                                  if(!yylval.stringVal)
                                  {
                                    printf(".l : BREAK token Mem Fail\n");
                                    exit(1);
                                  }
                                  strcpy(yylval.stringVal, yytext);
                                  return BREAK;
                                }
"continue"                      { yylval.stringVal = (char*)malloc(strlen(yytext) + 1);
                                  if(!yylval.stringVal)
                                  {
                                    printf(".l : CONTINUE token Mem Fail\n");
                                    exit(1);
                                  }
                                  strcpy(yylval.stringVal, yytext);
                                  return CONTINUE;
                                }

  /* Identifiers */
{id}                            {
                                  char *str = (char*)malloc(strlen(yytext) + 1);
                                  if(!str)
                                  {
                                    printf(".l : ID token Mem Fail\n");
                                    exit(1);
                                  }
                                  strcpy(str, yytext);
                                  yylval.stringVal = str;
                                  return ID;
                                }

  /* Integer constants */
{integer_const}                 {
                                  yylval.intVal = atoi(yytext);
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

char *GetFileName() {
  return file_name;
}

int get_lineno() {
   return lineno;
}

int main(int argc, char* argv[]) {
  if(argc >= 2) {
    yyin = fopen(argv[1], "r");
    /* 파일 입력 시에 할당 및 복사 */
    file_name = (char*)malloc((strlen(argv[1]) + 1) * sizeof(char));
    if(!file_name)
    {
      printf("Memory allocation Fail\n");
      exit(1);
    }
    strcpy(file_name, argv[1]);
  } else {
    yyin = stdin;
  }

  if(!yyin) {
    printf("Can't open input stream!\n");
    exit(1);
  }

  Push();
  GlobalTypeListInitialize();
  
  yyparse();
  fclose(yyin);

  AllGlobalListFree();  
  free(file_name);

  return 0;
}
