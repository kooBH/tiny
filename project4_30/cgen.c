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
            emitPop("$t0");
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
            emitPop("$t0");
            emitIfTrue(jumpCnt);
            savedLoc2 = jumpCnt;
            jumpCnt++;

            /*  { p2} */
            cGen(p2);
            emitJump2JumpLabel(savedLoc1);
            emitJumpLabel(savedLoc2);
            break;
        case RetK:
            emitPop("$v0");
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
          if(tree->visited)break;
#if DEBUG
            printf("ExpK ConstK\n");
#endif
            emitComment(">>ExpK CosntK");
            emitLi("$t0",tree->attr.val);
            emitPush("$t0");
            emitComment("<<ExpK CosntK");
            break; /* ConstK */

        case IdK :
          if(tree->visited)break;
#if DEBUG
            printf("ExpK IdK | location %d\n",tree->location);
#endif
            emitComment(">>ExpK IdK");
            emitLw("$t0",tree->location);
            emitPush("$t0");
            
            emitComment("<<ExpK IdK");
            break; /* IdK */

        case OpK :
            if(tree->visited)break;
#if DEBUG
              printf("ExpK OpK\n");
#endif
            p1 = tree->child[0];
            p2 = tree->child[1];
           switch (tree->attr.op) {
                case PLUS:
            emitComment(">>ExpK OpK Plus");
                  cGen(p2);
                  cGen(p1);
                  p1->visited=TRUE;
                  p2->visited=TRUE;

                  emitPop("$t1");
                  emitPop("$t0");
                  emitCode("add $t2, $t0, $t1 ");
                  emitPush("$t2");
            emitComment("<<ExpK OpK Plus");
                break;
                case MINUS:
            emitComment(">>ExpK OpK Minus");
                  cGen(p2);
                  cGen(p1);
                  p1->visited=TRUE;
                  p2->visited=TRUE;

                  emitPop("$t1");
                  emitPop("$t0");
                  emitCode("sub $t2, $t0, $t1 ");
                  emitPush("$t2");
            emitComment("<<ExpK OpK Minus");
                break;
                case TIMES:
                emitComment(">>ExpK OpK Times");
                  cGen(p2);
                  cGen(p1);
                  p1->visited=TRUE;
                  p2->visited=TRUE;

                  emitPop("$t1");
                  emitPop("$t0");
                  emitCode("mul $t2, $t0, $t1 ");
                  emitPush("$t2");
                  emitComment("<<ExpK OpK Times");

                break;
                case OVER:
                  emitComment(">>ExpK OpK Over");
                  cGen(p2);
                  cGen(p1);
                  p1->visited=TRUE;
                  p2->visited=TRUE;

                  emitPop("$t1");
                  emitPop("$t0");
                  emitCode("div $t2, $t0, $t1 ");
                  emitPush("$t2");
                  emitComment("<<ExpK OpK Over");
                break;
                case LT:
                break;
                case LTEQ:
                break;
                case GT:
                break;
                case GTEQ:
                break;
                case EQ:
                break;
                case NEQ:
                break;

                default:
                    emitComment("BUG: Unknown operator");
                    break;
              tree->visited = TRUE;
            } /* case op */
            break; /* OpK */
        case CallK:
#if DEBUG
            printf("ExpK CallK %s\n",tree->attr.name);
#endif
            emitCall(tree->attr.name);
            break;   

        case AssignK:
#if DEBUG
            printf("ExpK AssignK \n");
#endif
            emitComment(">>ExpK AssignK");
            p1 = tree->child[0];
            p2 = tree->child[1];
            cGen(p2);
            p1->visited = TRUE;
            p2->visited = TRUE;

            emitPop("$t1");
            emitSw("$t1",p1->location);

            emitComment("<<ExpK AssignK");
            break; /* assign_k */

        default:
          printf("Default\n");
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
                          if(tree->visited)break;
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
                            tree->visited=TRUE;
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
