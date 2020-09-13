/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"
#include <stdio.h>

/* counter for variable memory locations */
//static int location = 0;
static ScopeList cur_scope;
static ScopeList temp_scope;
static ScopeList fun_scope;
static int insert_error=0;


/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
static void traverse( TreeNode * t,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) )
{ 
  if (t != NULL)
  { preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(t->child[i],preProc,postProc);
    }
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}


static void redefineError(TreeNode * t, char * message)
{ fprintf(listing,"Redefinition error at line %d: %s\n", t->lineno, message);
  Error = TRUE;
  insert_error = 1;
}

static void undefineError(TreeNode * t, char * message){
  fprintf(listing, "Undefinition error at line %d: %s\n", t->lineno, message);
  Error = TRUE;
  insert_error = 1;
}

/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static void insertNode( TreeNode * t)
{ 
  ScopeList check_scope;
  FunctionList fun;
  char buf[100];

  switch (t->nodekind)
  { case StmtK:
      switch (t->kind.stmt)
      { case FunK:
          if( (check_scope = scope_insert(t->attr.name, cur_scope)) == NULL){
            // Scope already exist which means
            // function name is invalid. Redefinition error.
            sprintf(buf, "Function name %s is already defined", t->attr.name);
            redefineError(t, buf);
          }
          else{
            // Insert function to st.
            st_insert(cur_scope, t->attr.name, t->type, t->lineno, 2);
            // Change scope. 
            // Temp scope and check_scope is diffrent now.
            temp_scope = cur_scope;
            cur_scope = check_scope;
            // Insert function table.
            ft_insert(t->attr.name, t->type);
          }
          break;
        case CompK:
          if (cur_scope == temp_scope){
            // It is if/while statement. Insert, change scope.
            sprintf(buf, "%s.%d", cur_scope->name, cur_scope->child_num); 
            cur_scope = scope_insert(buf, cur_scope);
          }
          temp_scope = cur_scope;
          break;
        default:
          break;
      }
      break;

    case ExpK:
      switch (t->kind.exp)
      { case IdK:   // Use single variable.
          if(st_lookup(cur_scope, t->attr.name) == NULL){
            // Variable is not defined. Undefined error.
            sprintf(buf, "Variable %s is not defined", t->attr.name);
            undefineError(t, buf);
          }
          else{
            st_add(cur_scope, t->attr.name, t->lineno);
          }

          break;

        case AidK:  // Use array variable.
          if(st_lookup(cur_scope, t->attr.name) == NULL){
            // Variable is not defined. Undefined error.
            sprintf(buf, "Variable %s is not defined", t->attr.name);
            undefineError(t, buf);
          }
          else{
            st_add(cur_scope, t->attr.name, t->lineno);
          }
          break;
        
        case VarK:   // Variable declaration.
          if(st_excluding_parent(cur_scope, t->attr.name) != NULL){
            // Variable aleady defined. Redefinition error.
             sprintf(buf, "Variable %s is already defined", t->attr.name);
            redefineError(t,buf);
          }
          else{
            st_insert(cur_scope, t->attr.name, t->type, t->lineno, 0);
          }
          break;

        case AvarK: // Array variable declaration.
          if(st_excluding_parent(cur_scope, t->attr.name) != NULL){
            // Variable is aleady defined. Redefinition error.
            sprintf(buf, "Variable %s is already defined", t->attr.name);
            redefineError(t, buf);
          }
          else{
            st_insert(cur_scope, t->attr.name, t->type, t->lineno, 1);
          }
          break;

        case CallK: // Call function.
          if(st_lookup(cur_scope, t->attr.name) == NULL){
            // Function is not defined. Undefined error.
            sprintf(buf, "Function %s is not defined", t->attr.name);
            undefineError(t,buf);
          }
          else{
            st_add( cur_scope, t->attr.name, t->lineno);
          }
          break;
        
        case SparamK: // Single parameter of function call.
          if(st_excluding_parent(cur_scope, t->attr.name) != NULL){
            // Parameter is already define. Redefinition error.
            sprintf(buf, "Parameter %s is already defined", t->attr.name);
            redefineError(t,buf);
          }
          else{
            st_insert( cur_scope, t->attr.name, t->type, t->lineno, 3);
            fun = ft_lookup(cur_scope->name);
            fun->paramType[fun->paramNum++] = Integer;
          }
          break;

        case AparamK:   // Array parameter of function call.
          if(st_excluding_parent(cur_scope, t->attr.name) != NULL){
            //Parameter is already defined. Redefinition error.
            sprintf(buf, "Parameter %s is already defined", t->attr.name);
            redefineError(t,buf);
          }
          else{
            st_insert( cur_scope, t->attr.name, t->type, t->lineno, 4);
            fun = ft_lookup(cur_scope->name);
            fun->paramType[fun->paramNum++] = IntegerArr;
          }
          break;

        default:
          break;
      }
      break;
    default:
      break;
  }
}

/* Function for exiting at the end of scope. (End of compound.)
*/
void exitScope(TreeNode * t){
  switch (t->nodekind){ 
    case StmtK:
      switch (t->kind.stmt){ 
        case CompK:     // At the end of the compoud, change scope to parent.
          cur_scope = cur_scope->parent;
          temp_scope = cur_scope;
          break;

        default:
          break;
      }
    default:
      break;
  }
}

void enterScope(TreeNode * t){
  char buf[100];
  switch(t->nodekind){
    case StmtK:
      switch(t->kind.stmt){
        case FunK:
          cur_scope = scope_lookup(t->attr.name);
          fun_scope = cur_scope;
          break;

        case CompK:
          if(temp_scope == cur_scope){
            // It's if/while statememt.
            sprintf(buf, "%s.%d", cur_scope->name, cur_scope->current_child++); 
            cur_scope = scope_lookup(buf);
          }     // It can be dangerous here.
          temp_scope = cur_scope;
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}




/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree)
{ 
  ScopeList temp;
  FunctionList fun;

  cur_scope = scope_insert("global", NULL);
  // Insert built - in function.
  st_insert(cur_scope, "input", Integer, 0, 2);
  scope_insert("input", cur_scope);
  ft_insert("input", Integer);
  st_insert(cur_scope, "output", Void, 0, 2);
  temp = scope_insert("output", cur_scope);
  fun = ft_insert("output", Void);
  // Insert no-name parameter.
  st_insert(temp, "", Integer, 0, 3);
  fun->paramType[fun->paramNum++] = Integer;

  temp_scope = cur_scope;
  traverse(syntaxTree,insertNode,exitScope);
  if (TraceAnalyze)
  { fprintf(listing,"\n< Symbol Table >\n");
    printSymTab(listing);
    fprintf(listing, "\n< Function Table >\n");
    printFunTab(listing);
    fprintf(listing, "\n< Function And Global Variables >\n");
    printGlobalTab(listing);
    fprintf(listing, "\n< Function Parameters and Local Variables >\n");
    printFunVarTab(listing);

  }
}

static void typeError(TreeNode * t, char * message)
{ fprintf(listing,"Type error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}



/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t)
{ 
  BucketList b;
  int i;
  ExpType type;
  TreeNode * child;

  
  FunctionList fun;
  switch (t->nodekind)
  { case ExpK:
      switch (t->kind.exp)
      { 
        case OpK:
          if((t->child[0]->type != Integer) || (t->child[1]->type != Integer)){
            typeError(t, "Operation between diffrent type.");
          }
          else{
            t->type = Integer;
          }
            break;

        case VarK:      // Variable declaration.
          if (t->type == Void){
            typeError(t, "type 'void' is not avaliable for variable");
          }
          break;
        case AvarK:
          if(t->type == Void){ 
            typeError(t, "type 'void' is not avaliable for variable");
          }
          //fprintf(listing,"avar done\n");
          break;
        
        case AssignK:
          if(t->child[1]->kind.exp == ConstK){
            type = t->child[1]->type;
          }
          else{
            type = t->child[1]->type;
          }

          if (t->child[0]->type != type){
            typeError(t->child[1],"assignment type mismatch");
          }
          break;

        
        case CallK:
          if(st_excluding_parent(cur_scope, t->attr.name) != NULL){
            // When a variable with same name of function exist,
            // function call is invalid\n
            typeError(t, "invalid function call. It's not a function in current scope");
          }
          fun = ft_lookup(t->attr.name);
          t->type = fun->type;

          if(fun->paramNum == 0){
            if(t->child[0] != NULL){
              typeError(t, "invalid function call. Argument exist on void function.");
            }
          }
          else{ // Parameter exist.
            child = t->child[0];
            for(i=0; i<fun->paramNum; i++){ //check each parameter
              if(child == NULL){
                typeError(t, "invalid function call. Number of argument mismatch.");
                break;
              }
              
              if(fun->paramType[i] != child->type){
                typeError(t, "invalid function call. Argument type mismatch."); 
                break;
              }
              
              else{
                if(child == NULL){
                  typeError(t, "invalid function call. Number of parameter mismatch");
                  break;
                }
                child = child->sibling;
              }
            }
            if((child != NULL) && (child->sibling != NULL)){
              typeError(t, "invalid function call. Number of parameter mismatch");
            }
          }
          break;
        case IdK:
          b = st_lookup(cur_scope, t->attr.name);
          if(b == NULL){
            t->type = Void;
            break;
          }
          else if(b->variable_type == 1 || b->variable_type == 4){
            t->type = IntegerArr;
          }
          else{
            t->type = b->type;
          }
          break;
        case AidK:
          b = st_lookup(cur_scope, t->attr.name);
          if(b == NULL){
            t->type = Void;
            break;
          }
          else{
            b = st_lookup(cur_scope, t->attr.name);
            t->type = b->type;
          }
          break;

        default:
          break;
      }
      break;
    case StmtK:
      
      switch (t->kind.stmt)
      { 
        case CompK:     // At the end of the compoud, change scope to parent.
          cur_scope = cur_scope->parent;
          temp_scope = cur_scope;
          break;

        case ReturnK:
          b = st_lookup(fun_scope, fun_scope->name);
          if ((b->type == Void) && (t->child[0] != NULL)){
            typeError(t, "invalid return type. Return type should be void");
          }
          else if ((b->type != Void) && (t->child[0] == NULL)){
            typeError(t, "invalid return type. There is no return value on non void function.");
          }
          
          else if ((b->type == Integer) && (t->child[0]->type != Integer)){ 
            typeError(t, "invalid return type. Return type should be int");
          }
          break;

        default:
          break;
      }
      break;
    default:
      break;

  }
}

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree)
{   cur_scope = scope_lookup("global");
    temp_scope = cur_scope;

    traverse(syntaxTree, enterScope, checkNode);
}
