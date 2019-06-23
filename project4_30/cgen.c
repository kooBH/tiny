/****************************************************/
/* File: cgen.c                                     */
/* The code generator implementation                */
/* for the TINY compiler                            */
/* (generates code for the TM machine)              */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "code.h"
#include "cgen.h"

static int jumpCnt=0;

static int  isGlobalVarsDone=FALSE;

/* tmpOffset is the memory offset for temps
   It is decremented each time a temp is
   stored, and incremeted when loaded again
   */
static int tmpOffset = 0;

/* prototype for internal recursive code generator */
static void cGen (TreeNode * tree);

/* Procedure genStmt generates code at a statement node */
static void genDecl( TreeNode * tree)
{
    TreeNode * p1, * p2, * p3;
    int savedLoc1,savedLoc2,currentLoc;
    int offset;
#if DEBUG
    printf("genDecl\n");
#endif
    switch(tree->kind.decl)
    {
        case FuncK:
#if DEBUG
            printf("Decl FuncK %s\n",tree->attr.name);
#endif
            /* global vars 선언에서 바로 main 으로*/
            if(!isGlobalVarsDone)
            {
                isGlobalVarsDone=TRUE;
                emitStartup();
            }
            /* main 함수 부분 라벨 예외*/
            if(!strcmp(tree->attr.name,"main"))
            {
                emitLabel("__main");
            }
            else
            {
                emitLabel(tree->attr.name);
            }

            /* body of func */
            //p1 = tree->child[2];
            //cGen(p1);
            
            emitFuncStart();
            
            break;
        case VarK:
#if DEBUG
            printf("Decl VarK %s\n",tree->attr.name);
#endif
            if(!isGlobalVarsDone)
            {
            }
            // local variables
            else
            {
                emitCode("sub $sp,$sp,4");
            }

            break;
        case ArrVarK:
            if(!isGlobalVarsDone)
            {

            }
            // local variables
            else
            {
                offset = tree->attr.arr.size*4;
                emitStackPush(offset);
            }
            break;
    }
}
/*genDecl*/

/* Procedure genStmt generates code at a statement node */
static void genStmt( TreeNode * tree)
{ 
    TreeNode * p1, * p2, * p3;
    int savedLoc1,savedLoc2,currentLoc;
    int loc;

#if DEBUG
    printf("genStmt\n");
#endif
    switch (tree->kind.stmt)
    {
        case IfK:
#if DEBUG
            printf("IfK lineno %d\n",tree->lineno);
#endif
            p1 = tree->child[0] ;
            p2 = tree->child[1] ;
            p3 = tree->child[2] ;

            /***  IF  ***/
            emitComment("IfK if");
            cGen(p1);
            //savedLoc1 = emitSkip(1) ;
            emitPop("t0");
            emitIfFalse(jumpCnt);
            savedLoc1 = jumpCnt;
            jumpCnt++;
            printf("savedLoc1 : %d\n",savedLoc1);

            /*** THEN ***/
            emitComment("IfK Then");
            cGen(p2);
            //savedLoc2 = emitSkip(1) ;
            if(p3 !=NULL){
              emitJump2JumpLabel(jumpCnt);
              savedLoc2 = jumpCnt;
              jumpCnt++;
              printf("savedLoc2 : %d\n",savedLoc2);
            }
            emitJumpLabel(savedLoc1);
           /*** ELSE ***/
            if(p3 != NULL){
              emitComment("IfK Else");
              cGen(p3);
              emitJumpLabel(savedLoc2);
            }
            printf("BB\n");
          break;


        case CompK:
            emitComment("CompK entry");
            //p1 = tree->child[0] ;
            //p2 = tree->child[1] ;
            // emitComment("Compound Statment : var_decl");
            //cGen(p1);
            // emitComment("Compound Statment : stmt_list");
            // cGen(p2);
            break;
        case IterK:
            p1 = tree->child[0] ;
            p2 = tree->child[1] ;

            emitJumpLabel(jumpCnt);
            savedLoc1 = jumpCnt;
            jumpCnt++;
            /* While(p1) */
            cGen(p1);
            emitPop("t0");
            emitIfTrue(jumpCnt);
            savedLoc2 = jumpCnt;
            jumpCnt++;

            /*  { p2} */
            cGen(p2);
            emitJump2JumpLabel(savedLoc1);
            emitJumpLabel(savedLoc2);
            break;
        case RetK:
            emitPop("v0");
            emitComment("RetK");
            break;
        default:
            break;
    }
} /* genStmt */

/* Procedure genExp generates code at an expression node */
static void genParam( TreeNode * tree){
#if DEBUG
    printf("genParam\n");
#endif
    emitComment("paramK");
}/* genParam*/


/* Procedure genExp generates code at an expression node */
static void genExp( TreeNode * tree)
{
#if DEBUG
    printf("genExp\n");
#endif
    int loc;
   TreeNode *p1, *p2;
    switch (tree->kind.exp)
    {
        case ConstK :

#if DEBUG
            printf("genExp ConstK\n");
#endif

            if (TraceCode) emitComment("-> Const") ;
            /* gen code to load integer constant using LDC */
            if (TraceCode)  emitComment("<- Const") ;
            break; /* ConstK */

        case IdK :
#if DEBUG
            printf("genExp IdK\n");
#endif
            if (TraceCode) emitComment("-> Id") ;
            if (TraceCode)  emitComment("<- Id") ;
            break; /* IdK */

        case OpK :
#if DEBUG
              printf("genExp OpK\n");
#endif
            if (TraceCode) emitComment("-> Op") ;
            p1 = tree->child[0];
            p2 = tree->child[1];
           switch (tree->attr.op) {
                default:
                    emitComment("BUG: Unknown operator");
                    break;
            } /* case op */
            if (TraceCode)  emitComment("<- Op") ;
            break; /* OpK */
        case CallK:
#if DEBUG
            printf("ExpK CallK %s\n",tree->attr.name);
#endif
            emitCall(tree->attr.name);
            break;   

        case AssignK:
            if (TraceCode) emitComment("-> assign") ;
            /* generate code for rhs */
#if DEBUG
            printf("CompK %s\n",tree->attr.name);
#endif
            if (TraceCode)  emitComment("<- assign") ;
            break; /* assign_k */

        default:
            break;
    }
} /* genExp */

/* Procedure cGen recursively generates code by
 * tree traversal
 */
static void cGen( TreeNode * tree)
{ 

    int i=0;
    int offset=0;
    TreeNode * temp;
    if (tree != NULL)
    {
#if DEBUG
        printf("cGen lineno %d\n",tree->lineno);
#endif
        tree->visited = 1;
        switch (tree->nodekind) {
            case StmtK:
                genStmt(tree);
                break;
            case ExpK:
                genExp(tree);
                break;
            case ParamK:
                genParam(tree);
                break;
            case DeclK:
                genDecl(tree);
                break;
            default:
                break;
        }
        for( i=0; i<MAXCHILDREN;i++){
            cGen(tree->child[i]);
            if(tree->child[i]==NULL)
                continue;

            switch(tree->nodekind){
                case DeclK:
                    switch(tree->kind.decl){
                        case FuncK:
                            // Function 끝에 return
                            if(tree->child[i]->nodekind == StmtK){
                                emitFuncEnd();
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                case StmtK:
                    switch(tree->kind.stmt){
                        case CompK:
                           offset = 0;
                            // Compound 끝에 stack관리
                            //emitCode("move $sp,$s0");
                            temp = tree->child[0];
                            while(temp!=NULL){
                              switch(temp->kind.decl){
                                case VarK:
                                  offset+=4;
                                  break;
                                case ArrVarK:
                                  offset +=4*temp->attr.arr.size;
                                  break;
                              
                              }
                              temp =temp->sibling;
#if DEBUG 
                              printf("stack offset %d\n",offset);

#endif
                            }

                            emitComment("end of CompK : stack manage");
                            emitStackPop(offset);
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
        }
        cGen(tree->sibling);
    }
#if DEBUG
    if(tree!=NULL)
        printf("cGen lineno %d finished\n",tree->lineno);
#endif
}

/**********************************************/
/* the primary function of the code generator */
/**********************************************/
/* Procedure codeGen generates code to a code
 * file by traversal of the syntax tree. The
 * second parameter (codefile) is the file name
 * of the code file, and is used to print the
 * file name as a comment in the code file
 */
void codeGen(TreeNode * syntaxTree, char * codefile)
{
    char * s = malloc(strlen(codefile)+7);

    strcpy(s,"File : ");
    strcat(s,codefile);

    emitComment(s);
    emitComment("Language : C-");
    emitString("    .data\n");
    emitComment("Area for global Variables");
    emitString("\n");
    emitComment("End of area for global Variables");
    emitString("\n");
    cGen(syntaxTree);
#if DEBUG
    printf("cGen(SyntaxTree) Finished\n");
#endif
    emitString("\n");
    emitComment("End of code.");
    emitString("\n");
#if DEBUG
    printf("\nCODEGEN\n");
#endif
}
