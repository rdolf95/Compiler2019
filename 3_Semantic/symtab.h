/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for the TINY compiler     */
/* (allows only one symbol table)                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "globals.h"

/* SIZE is the size of the hash table */
#define SIZE 211



typedef struct LineListRec
    { int lineno;
      struct LineListRec * next;
    } * LineList;

typedef struct BucketListRec
    { char * name;
      // 0: variable, 1: array, 2: function 
      // 3: single parameter, 4: array parameter
      int variable_type;  
      ExpType type;
      LineList lines;
      int memloc;
      struct BucketListRec * next;
    } * BucketList;

typedef struct ScopeListRec
    { char * name;
      BucketList bucket[SIZE];
      struct ScopeListRec * parent;
      struct ScopeListRec * next;
      int child_num;
      int current_child;  // Counter for if/while compound when type checking.
      int location;     // Counter for memory location.
    } * ScopeList;

typedef struct FunctionListRec
    { char * name;
      int paramNum;
      ExpType paramType[100];   //0: integer, 1: array
      ExpType type;
      struct FunctionListRec * next;
    } * FunctionList;


/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( ScopeList scope, char * name, ExpType type, int lineno, int variable_type);

void st_add( ScopeList cur_scope, char * name, int lineno);
/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
ScopeList scope_insert( char * scope, ScopeList parent);
BucketList st_lookup ( ScopeList scope, char * name);
BucketList st_excluding_parent ( ScopeList scope, char * name);
ScopeList scope_lookup( char * scope);
FunctionList ft_insert(char * name, ExpType type);
FunctionList ft_lookup(char * name);
/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing);
void printFunTab(FILE * listing);
void printGlobalTab(FILE * listing);
void printFunVarTab(FILE * listing);

#endif
