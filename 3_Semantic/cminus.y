/****************************************************/
/* File: tiny.y                                     */
/* The TINY Yacc/Bison specification file           */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode *
/*static char * savedName;*/ /* for use in assignments */
//static int savedLineNo;  /* ditto */
static int savedType;
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yylex(void); // added 11/2/11 to ensure no conflict with lex

%}

%token IF THEN ELSE END REPEAT UNTIL READ WRITE WHILE RETURN VOID INT
%token ID NUM 
%token ASSIGN EQ LT PLUS MINUS TIMES OVER LPAREN RPAREN SEMI NE LE GE GT COMMA LCURLY RCURLY LBRACE RBRACE 
%token ERROR
%nonassoc LT_ELSE
%nonassoc ELSE

%% /* Grammar for C-minus */


program     : dclar_list
                 { savedTree = $1;}
            ;

dclar_list  : dclar_list dclar
                 { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1; }
                     else $$ = $2;
                 }
            | dclar
                 { $$ = $1;}
            ;

dclar       : var_dclar
                 { $$ = $1;}
            | fun_dclar
                 { $$ = $1;}
            ;

var_dclar   : type_specif 
              ID { $$ = newExpNode(VarK);
                   /*$$->kind.exp = VarK;*/
                   $$->type = savedType;
                   $$->attr.name = copyString(prevString);
                   $$->lineno = lineno;
                 }
              SEMI
              { $$ = $3; }
            | type_specif ID
                 { $$ = newExpNode(AvarK);
                   $$->type = savedType;
                   /*$$->kind.exp = AvarK;*/
                   $$->child[0] = newExpNode(ConstK);
                   $$->attr.name = copyString(prevString);
                   $$->lineno = lineno; 
                 }
            LBRACE NUM
                 { $$ = $3;
                   $$->child[0]->attr.val = atoi(tokenString);
                 }
            RBRACE SEMI
            { $$ = $3;
            }
            ;

type_specif : INT
                 { /*$$ = newExpNode(VarK);
                   $$->type = Integer;*/
                   savedType = Integer;
                 }
            | VOID
                 { /*$$ = newExpNode(VarK);
                   $$->type = VOID;*/
                   savedType = Void;
                 }
            ;

fun_dclar  : type_specif ID 
                { $$ = newStmtNode(FunK);
                  $$->type = savedType;
                  $$->attr.name = copyString(prevString);
                  $$->lineno = lineno; 

                }
              LPAREN params RPAREN comp_stmt
                { $$ = $3;
                  $$->child[0] = $5;
                  $$->child[1] = $7;
                }
            ;

params      : param_list
                { $$ = $1; }
            | VOID
                { $$ = $1; }
            ;

param_list  : param_list COMMA param
                { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $3;
                     $$ = $1; }
                     else $$ = $3;
                }
            | param
                { $$ = $1; }
            ;

param       : type_specif ID
                { $$ = newExpNode(SparamK);
                  $$->type = savedType;
                  /*$$->kind.exp = SparamK;*/
                  $$->attr.name = copyString(prevString);
                  $$->lineno = lineno;
                }
            | type_specif ID
                { $$ = newExpNode(AparamK);
                  $$->type = savedType;
                  /*$$->kind.exp = AparamK;*/
                  $$->attr.name = copyString(prevString);
                  $$->lineno = lineno;
                }
              LBRACE RBRACE
              { $$ = $3; }
            ;


comp_stmt   : LCURLY loc_declar stmt_list RCURLY
                { $$ = newStmtNode(CompK);
                  $$->child[0] = $2;
                  $$->child[1] = $3;
                }
            ;

loc_declar  :loc_declar var_dclar
                 { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1; }
                     else $$ = $2;
                 }
            | /*empty*/
                { $$ = NULL; }
            ;

stmt_list   : stmt_list stmt
                 { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1; }
                     else $$ = $2;
                 }
            | /*empty*/
                { $$ = NULL; }
            ;

stmt        : exp_stmt { $$ = $1; }
            | comp_stmt { $$ = $1; }
            | selec_stmt { $$ = $1; }
            | itera_stmt { $$ = $1; }
            | ret_stmt { $$ = $1; }
            ;

exp_stmt    : exp SEMI
                { $$ = $1; }
            | SEMI
                { $$ = NULL;}
            ;

selec_stmt   : IF LPAREN exp RPAREN stmt
                { $$ = newStmtNode(If1K);
                  $$->child[0] = $3;
                  $$->child[1] = $5;
                } %prec LT_ELSE;
            | IF LPAREN exp RPAREN stmt ELSE stmt
                { $$ = newStmtNode(IfElseK);
                  $$->child[0] = $3;
                  $$->child[1] = $5;
                  $$->child[2] = $7;
                }
            ;

itera_stmt  : WHILE LPAREN exp RPAREN stmt
                { $$ = newStmtNode(WhileK);
                  $$->child[0] = $3;
                  $$->child[1] = $5;
                }
            ;

ret_stmt    : RETURN SEMI
                { $$ = newStmtNode(ReturnK);}
            | RETURN exp SEMI
                { $$ = newStmtNode(ReturnK);
                  $$->child[0] = $2;
                }
            ;

exp         : var ASSIGN exp
                { $$ = newExpNode(AssignK);
                  $$->child[0] = $1;
                  $$->child[1] = $3;
                }
            | simple_exp
                { $$ = $1; }
            ;

var         : ID 
                { $$ = newExpNode(IdK);
                  $$->attr.name = copyString(prevString);
                }
            |ID 
                { $$ = newExpNode(AidK);
                  $$->attr.name = copyString(prevString);

                }
              LBRACE exp RBRACE
                { $$ = $2;
                  $$->child[0] = $4;
                }
            ;
                
simple_exp  : add_exp relop add_exp
                {
                  $$ = $2;
                  $$->child[0] = $1;
                  $$->child[1] = $3;
                }
            | add_exp
                { $$ = $1; }   
            ;

relop       : EQ { $$ = newExpNode(OpK);
                   $$->attr.op = EQ;
                 }
            | NE { $$ = newExpNode(OpK);
                   $$->attr.op = NE;
                 }
            | LT { $$ = newExpNode(OpK);
                   $$->attr.op = LT;
                 }
            | GT { $$ = newExpNode(OpK);
                   $$->attr.op = GT;
                 }
            | LE { $$ = newExpNode(OpK);
                   $$->attr.op = LE;
                 }
            | GE { $$ = newExpNode(OpK);
                   $$->attr.op = GE;
                 }
            ;

add_exp     : add_exp addop term
                { $$ = $2;
                  $$->child[0] = $1;
                  $$->child[1] = $3;
                }
            | term { $$ = $1; }
            ;

addop       : PLUS { $$ = newExpNode(OpK);
                     $$->attr.op = PLUS;
                   }
            | MINUS { $$ = newExpNode(OpK);
                      $$->attr.op = MINUS;
                    }
            ;

term        : term mulop factor
                { $$ = $2;
                  $$->child[0] = $1;
                  $$->child[1] = $3;
                }
            | factor { $$ = $1; }
            ;

mulop       : TIMES { $$ = newExpNode(OpK);
                      $$->attr.op = TIMES;
                    }
            | OVER  { $$ = newExpNode(OpK);
                      $$->attr.op = OVER;
                    }
            ;

factor      : LPAREN exp RPAREN { $$ = $2; }
            | var { $$ = $1; }
            | call { $$ = $1; }
            | NUM {$$ = newExpNode(ConstK);
                   $$->attr.val = atoi(tokenString);
                   $$->type = Integer;
                  }
            ;

call        : ID 
                { $$ = newExpNode(CallK); 
                  $$->attr.name = copyString(prevString);
                }
              LPAREN args RPAREN
                { $$ = $2;
                  $$->child[0] = $4;
                }
            ;

args        : arg_list { $$= $1; }
            | /*empty*/
                { $$ = NULL; }
            ;

arg_list   : arg_list COMMA exp
                 { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $3;
                     $$ = $1; }
                     else $$ = $3;
                 }
            | exp {$$ = $1;}
            ;

%%

int yyerror(char * message)
{ fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  printToken(yychar,tokenString);
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the TINY scanner
 */
static int yylex(void)
{ return getToken(); }

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}

