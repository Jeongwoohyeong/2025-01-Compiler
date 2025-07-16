/*
 * File Name    : subc.y
 * Description  : a skeleton bison input
 */

%{

/* Prologue section */

#include "subc.h"

int   yylex ();
int   yyerror (char* s);
int   get_lineno();
char *GetFileName();
void error_preamble(void);
void error_undeclared(void);
void error_redeclaration(void);
void error_assignable(void);
void error_incompatible(void);
void error_null(void);
void error_binary(void);
void error_unary(void);
void error_comparable(void);
void error_indirection(void);
void error_addressof(void);
void error_struct(void);
void error_strurctp(void);
void error_member(void);
void error_array(void);
void error_subscript(void);
void error_incomplete(void);
void error_return(void);
void error_function(void);
void error_arguments(void);
int yyerror (char* s);

%}

/* Bison declarations section */

/* yylval types */
%union {
  int   intVal;
  char  *stringVal;
  ExtendedTypeInfo *Typeinfo;
  Variable *Var;
}

/* Precedences and Associativities */
%left   ','
%right  '='
%left   LOGICAL_OR
%left   LOGICAL_AND
%left   EQUOP
%left   RELOP
%left   '+' '-'
%left   '*' '/' '%'
%right  '!' '&' INCOP DECOP
%left   '[' ']' '(' ')' '.' STRUCTOP

%precedence IF
%precedence ELSE

/* Tokens and Types */
%token<stringVal> STRUCT RETURN WHILE FOR BREAK CONTINUE SYM_NULL TYPE
%token<intVal> INTEGER_CONST
%token<stringVal> ID CHAR_CONST STRING// identifier name

%type<stringVal> type_specifier struct_specifier pointers def func_decl stmt
%type<Typeinfo> unary binary expr expr_e args
%type<Var> param_decl param_list

/* Grammar rules */
%%
program
  : ext_def_list
  ;

ext_def_list
  : ext_def_list ext_def 
  | %empty 
  ;

ext_def
  : type_specifier pointers ID ';' 
  | type_specifier pointers ID '[' INTEGER_CONST ']' ';' 
  | struct_specifier ';' 
  | func_decl compound_stmt
  ;

type_specifier
  : TYPE              { $$ = $1; }
  | struct_specifier  { $$ = $1; }
  ;

struct_specifier
  : STRUCT ID {    
    if(LookUpStruct($2))
    {
      error_redeclaration();
      $2 = NULL;
    }
  }
  '{' { if($2) Push(); } def_list '}' {
    if($2)
    {
      DeclareStruct($2);
      Pop();
    }
    else
    {
      $$ = NULL;
    }
  }
  | STRUCT ID {
    $$ = LookUpStruct($2);
  }
  ;

func_decl
  : type_specifier pointers ID '(' ')' {
    if(LookUpFunction($3))
    {
      error_redeclaration();
      $$ = NULL;
      TempFunction = NULL;
    }
    else
    {
      DeclareFunction($1, $2, $3);
    }
  }
  | type_specifier pointers ID '(' param_list ')' {
    if(LookUpFunction($3))
    {
      error_redeclaration();
      $$ = NULL;
      TempFunction = NULL;
    }
    else
    {
      DeclareFunction($1, $2, $3);
      AddParameterListHead($5);
      TempParameterList = $5;
    }
  } 
  ;

pointers
  : '*'       { $$ = "*"; }
  | %empty    { $$ = NULL; }
  ;

param_list  
  : param_decl  { $$ = $1; }
  | param_list ',' param_decl {
    $$ = CreateParameterList($1, $3);
  }
  ;		

param_decl 
  : type_specifier pointers ID {
    if(LookUpVariable($3))
    {
      error_redeclaration();
      $$ = NULL;
    }
    else
    {
      $$ = DeclareParameter($1, $2, $3);
    }
  }
  | type_specifier pointers ID '[' INTEGER_CONST ']' {
    if(LookUpVariable($3))
    {
      error_redeclaration();
      $$ = NULL;
    }
    else
    {
      $$ = DeclareParameter($1, $2, $3);
      $$ = AddArrayType($$);
    }
  }
  ;

def_list    
  : def_list def 
  | %empty 
  ;

def
  : type_specifier pointers ID ';' {
    if(LookUpVariable($3))
    {
      error_redeclaration();
      $$ = NULL;
    }
    else
    {
      InsertVariable($1, $2, $3);
    }
  }
  | type_specifier pointers ID '[' INTEGER_CONST ']' ';' {
    if(LookUpVariable($3))
    {
      error_redeclaration();
      $$ = NULL;
    }
    else
    {
      InsertVariable($1, $2, $3);
      InsertVariable("[", "]", NULL);
    }
  }
  ;

compound_stmt
  : '{' { Push(); } def_list stmt_list '}' { Pop(); } /* Mid-rule Action */
  ;

stmt_list
  : stmt_list stmt 
  | %empty 
  ;

stmt
  : expr ';' 
  | RETURN expr ';' {
    if(!CompareReturnType($2))
    {
      error_return();
    }  
  }
  | BREAK ';' 
  | CONTINUE ';' 
  | ';' 
  | compound_stmt 
  | IF '(' expr ')' stmt {
    if($3 == 0)
    {
      $$ = NULL;
    }
    else if($3 >= 1)
    {

    }    
  }
  | IF '(' expr ')' stmt ELSE stmt {
    if($3 == 0)
    {
      $$ = NULL;
    }
    else if($3 >= 1)
    {

    }    
  }
  | WHILE '(' expr ')' stmt 
  | FOR '(' expr_e ';' expr_e ';' expr_e ')' stmt 
  ;

expr_e
  : expr    { $$ = $1; }
  | %empty  { $$ = NULL; }
  ;

expr
  : unary '=' expr{
    if($1 && $3)
    {
      if(!LValueCheck($1))
      {
        error_assignable();
        $$ = NULL;
      }
      else if(!AssignmentTypeCheck($1, $3))
      {
        error_incompatible();
      }
      else
      {
        $$ = NULL;
      }
    }
  }
  | binary { $$ = $1; }
  ;

binary
  : binary RELOP binary {
    if($1 && $1)
    {
      $$ = RelationOperationCheck($1, $3);
      if(!$$)
      {
        error_comparable();
      }
    }
    else
    {
      $$ = NULL;
    }
  }
  | binary EQUOP binary 
  | binary '+' binary {
    $$ = BinaryOperandTypeCheck($1, $3);
    if(!$$)
    {
      error_binary();
    }
    $1->Type = NULL;
    $3->Type = NULL;
    free($1);
    free($3);
  }
  | binary '-' binary 
  | binary '*' binary 
  | binary '/' binary 
  | binary '%' binary 
  | unary %prec '=' { $$ = $1; }
  | binary LOGICAL_AND binary 
  | binary LOGICAL_OR binary 
  ;

unary
  : '(' expr ')' {
    if($2)
    {
      $$ = $2;
    }
    else
    {
      $$ = NULL;
    }
  }
  | '(' unary ')' {
    if($2)
    {
      $$ = $2;
    }
    else
    {
      $$ = NULL;
    }
  }
  | INTEGER_CONST     { $$ = GetDefaultTypeConst(0); }
  | CHAR_CONST        { $$ = GetDefaultTypeConst(1); }
  | STRING 
  | ID { 
    if(LookUpFunction($1))
    {
      $$ = NULL;
    }
    else
    {
      $$ = LookUpVariable($1);
      if(!$$)
      {
        error_undeclared();        
      }

    }
  }
  | '-' unary %prec '!' {
    if($2)
    {
      $$ = UnarySignCheck($2);
    }
    else
    {
      $$ = NULL;
    }
  }
  | '!' unary
  | unary INCOP %prec STRUCTOP {
    if($1)
    {
      $$ = UnaryOperationCheck($1);
      if(!$$)
      {
        error_unary();
      }      
    }
    else
    {
      $$ = NULL;
    }
  }
  | unary DECOP %prec STRUCTOP 
  | INCOP unary %prec '!' {
    if($2)
    {
      $$ = UnaryOperationCheck($2);
      if(!$$)
      {
        error_unary();
      }      
    }
    else
    {
      $$ = NULL;
    }
  }
  | DECOP unary %prec '!' 
  | '&' unary {
    if($2)
    {
       $$ = LValueCheck($2);
       if(!$$)
       {
        error_addressof();
       }
    }
    else
    {
      $$ = NULL;
    }
  }
  | '*' unary %prec '!' {
    if($2)
    {
      $$ = LValueCheck($2);
      if(!$$)
      {
        error_addressof();
      }
    }
    else
    {
      $$ = NULL;
    }
  }
  | unary '[' expr ']' {
    $$ = ArrayElementCheck($1, $3);
    if(!$$)
    {
      error_subscript();
    }
  }
  | unary '.' ID {
    $$ = LookUpStructMemberVar($1, $3);
  }
  | unary STRUCTOP ID {
    $$ = StructOperationCheck($1, $3);
  }
  | unary '(' args ')' {
    if(!$1)
    {
      $$ = ArgumentTypeCheck((TypeInfo**)$3);
      if(!$$)
      {
        error_arguments();
      }
    }
  }
  | unary '(' ')' {
    $$ = GetFunctionReturnType();
   }
  | SYM_NULL 
  ;

args
  : expr {    
   if($1)
   {
    $$ = (ExtendedTypeInfo*)CopyArgument($1);
   }
  }
  | args ',' expr {
    $$ = (ExtendedTypeInfo*)CreateArgumentList((ExtendedTypeInfo**)$1, $3);
  }
  ;

%%

/* Epilogue section */

// Print the preamble of error message.
void error_preamble(void) {
  // TODO
  // Implement this function using get_lineno() function.
  // need to get the filename too
  printf("%s:%d: error: ", GetFileName(), get_lineno());
}

void error_undeclared(void) {
  error_preamble();
  printf("use of undeclared identifier\n");
}

void error_redeclaration(void) {
  error_preamble();
  printf("redeclaration\n");
}

void error_assignable(void) {
  error_preamble();
  printf("lvalue is not assignable\n");
}

void error_incompatible(void) {
  error_preamble();
  printf("incompatible types for assignment operation\n");
}

void error_null(void) {
  error_preamble();
  printf("cannot assign 'NULL' to non-pointer type\n");
}

void error_binary(void) {
  error_preamble();
  printf("invalid operands to binary expression\n");
}
void error_unary(void) {
  error_preamble();
  printf("invalid argument type to unary expression\n");
}

void error_comparable(void) {
  error_preamble();
  printf("types are not comparable in binary expression\n");
}

void error_indirection(void) {
  error_preamble();
  printf("indirection requires pointer operand\n");
}

void error_addressof(void) {
  error_preamble();
  printf("cannot take the address of an rvalue\n");
}

void error_struct(void) {
  error_preamble();
  printf("member reference base type is not a struct\n");
}

void error_strurctp(void){
  error_preamble();
  printf("member reference base type is not a struct pointer\n");
}

void error_member(void) {
  error_preamble();
  printf("no such member in struct\n");
}

void error_array(void) {
  error_preamble();
  printf("subscripted value is not an array\n");
}

void error_subscript(void) {
  error_preamble();
  printf("array subscript is not an integer\n");
}

void error_incomplete(void) {
  error_preamble();
  printf("incomplete type\n");
}

void error_return(void) {
  error_preamble();
  printf("incompatible return types\n");
}

void error_function(void) {
  error_preamble();
  printf("not a function\n");
}

void error_arguments(void) {
  error_preamble();
  printf("incompatible arguments in function call\n");
}

int yyerror (char* s) {
  fprintf (stderr, "yyerror: %s\n", s);
  return 0;
}