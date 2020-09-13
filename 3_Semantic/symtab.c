/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"
#include "util.h"

/* SIZE is the size of the hash table */
//#define SIZE 211

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4

/* the hash function */
static int hash ( char * key )
{ int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  { temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}


/* the scope hash table*/
static ScopeList scopeTable[SIZE];
static FunctionList functionTable[SIZE];
static int scope_num = 0;

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( ScopeList scope, char * name, ExpType type, int lineno, int variable_type)
{ int h = hash(name);

  //Get buckets inside of the scope.
  BucketList l = scope->bucket[h];
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;

  if (l == NULL) /* variable not yet in table */
  { l = (BucketList) malloc(sizeof(struct BucketListRec));
    l->name = name;
    l->lines = (LineList) malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->memloc = scope->location++;
    l->variable_type = variable_type;
    l->type = type;
    l->lines->next = NULL;
    l->next = scope->bucket[h];
    scope->bucket[h] = l;
  }
  else /* found in table, so just add line number */
  { LineList t = l->lines;
    while (t->next != NULL) t = t->next;
    t->next = (LineList) malloc(sizeof(struct LineListRec));
    t->next->lineno = lineno;
    t->next->next = NULL;
  }
} /* st_insert */

void st_add( ScopeList cur_scope, char * name, int lineno){
  int h = hash(name);
  ScopeList s = cur_scope;
  BucketList l;
  
  while (s != NULL){
    l = s->bucket[h];
    while ((l != NULL) && (strcmp(name, l->name) != 0)){
      l = l->next;
    }
    if (l == NULL)
      s = s->parent;
    else
      break;
  }

  if (s == NULL){
    // There is no variable in st.
    fprintf(listing, "Fatal error. There is no variable in symtab.");
    return;
  }
  else{
    // Found bucket.
    LineList t = l->lines;
    while (t->next != NULL) t = t->next;
    t->next = (LineList) malloc(sizeof(struct LineListRec));
    t->next->lineno = lineno;
    t->next->next = NULL;
  }
}

ScopeList scope_insert( char * scope, ScopeList parent)
{

  ScopeList s;
  
  int i, check = 0;

  for(i = 0; i<scope_num; i++){
    s = scopeTable[i];
    if(s != NULL && (strcmp(scope, s->name) ==0)){
      fprintf(listing,"Error. Scope already exist.");
      check =1;
      break;
    }
  }
 
  /* Time to make new scope. */
  if(check == 0){ 
    s = (ScopeList) malloc(sizeof(struct ScopeListRec));
    s->name = copyString(scope);   
    for(i = 0; i<SIZE; i++){
      s->bucket[i] = NULL;
    }

    s->parent = parent;
    s->child_num = 0;
    s->current_child = 0;
    s->location = 0;
    scopeTable[scope_num++] = s;

    if(parent != NULL){
      // This is possible when insert global scope.
      parent->child_num ++;
    }
    return s;
  }
  return NULL;
}

/* Function st_lookup returns the memory 
 * location of a variable or NULL if not found
 */
BucketList st_lookup ( ScopeList scope, char * name )
{ int h = hash(name);
  ScopeList s = scope;

  while(s != NULL){
    // Search from currnt scope to global scope.
    BucketList l = s->bucket[h];
    while ((l != NULL) && (strcmp(name,l->name) != 0))
      l = l->next;
    if (l == NULL){
      s = s->parent;
    }
    else return l;
  }
  // There is no variable or function in st.
  return NULL;
}

BucketList st_excluding_parent ( ScopeList scope, char * name){
  int h = hash(name);
  ScopeList s = scope;

  BucketList l = s->bucket[h];
  while ((l != NULL) && (strcmp(name, l->name) != 0))
    l = l->next;
  if (l == NULL){
    return NULL;
  }
  else return l;  
}

ScopeList scope_lookup( char * scope){
  int i, check = 0;
  ScopeList s;
  for(i = 0; i<scope_num; i++){
    s = scopeTable[i];
    if((s != NULL) && (strcmp(scope, s->name) != 0)){
      continue;
    }
    if( s != NULL && (strcmp(scope, s->name) ==0)){
      check = 1;
      break;
    }
  }

  if(check == 1){
    return s;
  }
  else{
    return NULL;
  }
}

FunctionList ft_insert(char * name, ExpType type){
  int h = hash(name);
  
  FunctionList f = functionTable[h];
  
  int i;
 
  while (f != NULL && (strcmp(name, f->name) != 0)){
    f = f->next;
  }

  if (f != NULL){
    fprintf(listing, "Error. Function aleady exist.");
    return NULL;  
  }
  else /* Time to make new function table. */
  { f = (FunctionList) malloc(sizeof(struct FunctionListRec));
    f->name = name;
    for(i = 0; i<100; i++){
      f->paramType[i] = 0;
    }
    f->type = type;
    f->paramNum = 0;
    f->next = functionTable[h];
    functionTable[h] = f;
    return f;
  }
}

FunctionList ft_lookup(char * name){
  int h = hash(name);
  FunctionList f = functionTable[h];
  
  while (f != NULL && (strcmp(name, f->name) != 0)){
    f = f->next;
  }
  return f;
}

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */

void printSymTab(FILE * listing)
{ int i, j;
  ScopeList s;

  fprintf(listing,"Variable Name  Variable Type  Scope Name  Location   Line Numbers\n");
  fprintf(listing,"-------------  -------------  ----------  --------   ------------\n");


  for (i=0; i<SIZE; ++i){
    if ((s = scopeTable[i]) != NULL){
      for (j=0; j<SIZE; ++j){
        BucketList l = s->bucket[j];
        while (l != NULL){
          LineList t= l->lines;
          fprintf(listing, "%-14s ", l->name);
          switch (l->variable_type){
            case 0:
            case 3:
              switch (l->type){
                case Integer:
                  fprintf(listing, "%-14s ", "Integer");
                  break;
                default:
                  break;
              }
              break;
            case 1:
            case 4:
              switch (l->type){
                case Integer:
                  fprintf(listing, "%-14s ", "IntegerArray");
                  break;
                default:
                  break;
              }
              break;

            case 2:
              fprintf(listing, "%-14s ", "Function");
              break;

            default:
              break;
          }
          fprintf(listing, "%-14s ", s->name);
          fprintf(listing, "%-8d  ", l->memloc);
          while (t != NULL){
            fprintf(listing, "%4d ", t->lineno);
            t = t->next;
          }
          fprintf(listing, "\n");
          l = l->next;
        }
      }
    }
  }
}
  /* printSymTab */

void printFunTab(FILE * listing){
  int i, j, check=0;
  ScopeList g = scopeTable[0];  //Global scope
  ScopeList s;

  fprintf(listing,"Function Name  Scope Name  Return Type  Parameter Name   Parameter Type\n");
  fprintf(listing,"-------------  ----------  -----------  --------------   --------------\n");
  
  while(g != NULL && (strcmp("global", g->name) != 0)){
    g = g->next;
  }
  
  if (g == NULL){
    fprintf(listing,"Fatal error. global scope not exist.\n");
    return;
  }

  else {  // g is global scope.
    for (i = 0; i<SIZE; ++i){
      BucketList l = g->bucket[i];
      while (l != NULL){
        if(l->variable_type == 2){   // it's function.
          fprintf(listing, "%-14s ", l->name);
          fprintf(listing, "%-14s ", g->name);
          switch(l->type){
            case Integer:
              fprintf(listing, "%-14s ", "Integer");
              break;
            case Void:
              fprintf(listing, "%-14s ", "Void");
              break;
            default:
              break;
          }

          s = scope_lookup(l->name);
          while (s != NULL && (strcmp(l->name, s->name) != 0)){
            s = s->next;
          }
          if (s == NULL){
            fprintf(listing, "Fatal error. Function %s scope not exist.",l->name);
            return;
          }

          else{  // s is function's scope.
            for (j = 0; j<SIZE; ++j){
              BucketList p = s->bucket[j];
              
              while (p != NULL){
                if(p->variable_type == 3 || p->variable_type == 4){
                  check =1;
                  // print parameter name.
                  fprintf(listing, "\n%-14s %-14s %-14s %-14s ","","","", p->name);
                  
                  switch(p->type){  // print parameter type,
                    case Integer:
                      if(p->variable_type == 3)
                        fprintf(listing, "%-14s ", "Integer");
                      else
                        fprintf(listing, "%-14s ", "IntegerArray");
                      break; 
                    default:
                      break;
                  }
                }
                p = p->next;
              }
            }

            // If check is still 0, it means parameter is void.
            if(check == 0){
              fprintf(listing, "%-14s %-14s ","", "Void");
            }
            check = 0;
            fprintf(listing, "\n");
          }  
        }
        l = l->next;
      }
    }
  }
}

void printGlobalTab(FILE * listing){
  int i;
  ScopeList g = scopeTable[0];  //global scope

  fprintf(listing,"ID Name  ID Type  Data Type\n");
  fprintf(listing,"-------  -------  ---------\n");
  
  while(g != NULL && (strcmp("global", g->name) != 0)){
    g = g->next;
  }
  
  if (g == NULL){
    fprintf(listing,"Fatal error. global scope not exist.\n");
    return;
  }

  else {  // g is global scope.
    for (i = 0; i<SIZE; ++i){
      BucketList l = g->bucket[i];
      while (l != NULL){
        fprintf(listing, "%-14s ", l->name);
        switch(l->variable_type){
          case 0: 
          case 1:
            fprintf(listing, "%-14s ", "Variable");
            break;
          case 2:
            fprintf(listing, "%-14s ", "Function");
            break;
          default:
            break;
        }

        switch(l->type){
          case Integer:
            if (l->variable_type == 1){
              fprintf(listing, "%-14s ", "Integer Array");
            }
            else{ 
              fprintf(listing, "%-14s ", "Integer");
            }
            break;
          case Void: 
            fprintf(listing, "%-14s ", "Void");
            break;
          default:
            break;
        }
        fprintf(listing, "\n");
        l = l->next;
      }
    }
  }
}

void printFunVarTab(FILE * listing){
  int i, j, level=1;
  ScopeList s, temp;

  fprintf(listing,"Scope Name  Nested Level  ID Name  Data Type\n");
  fprintf(listing,"----------  ------------  -------  ---------\n");
  
  for (i = 0; i<SIZE; ++i){
    s = scopeTable[i];
    while (s != NULL){
      if(strcmp(s->name, "global") == 0){
        // Global scope is not used here.
        s = s->next;
        continue;
      }
      level = 1;
      temp = s;

      while (strcmp(temp->parent->name, "global") != 0){
        temp = temp->parent;
        level ++;
        // Temp will be largest scope except global contain current scope.
      }

      for (j=0; j<SIZE; ++j){
        BucketList l = s->bucket[j];
        while (l != NULL){
          fprintf(listing, "%-14s ", temp->name);
          fprintf(listing, "%-8d ", level);
          fprintf(listing, "%-14s ", l->name);
          switch(l->variable_type){
            case 0:
            case 3:
              switch(l->type){
                case Integer:
                  fprintf(listing, "%-14s ", "Integer");
                  break;
                default:
                  break;
              }
              break;
            
            case 1:
            case 4:
              switch(l->type){
                case Integer:
                  fprintf(listing, "%-14s ", "IntegerArray");
                  break;
                default:
                  break;
              }
          }
          fprintf(listing, "\n");
          l = l->next;
        }
      }
      s = s->next;
      fprintf(listing, "\n");
    }
  }
}

