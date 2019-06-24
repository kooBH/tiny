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
                emitLabel("__main");
            else
                emitLabel(tree->attr.name);

            /* body of func */
            //p1 = tree->child[2];
            //cGen(p1);

            emitFuncStart();

            break;

        case VarK:
#if DEBUG
            printf("Decl VarK %s\n",tree->attr.name);
#endif
            // if(!isGlobalVarsDone) {  }
            // // local variables
            // else
            // emitCode("sub     $sp,$sp,4");

            if(isGlobalVarsDone)
                emitCode("sub     $sp, $sp, 4");

            break;
        case ArrVarK:
            // if(!isGlobalVarsDone) {  }
            // // local variables
            // else
            if(isGlobalVarsDone)
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
        case CompK:
            emitComment("CompK entry");
            //p1 = tree->child[0] ;
            //p2 = tree->child[1] ;
            // emitComment("Compound Statment : var_decl");
            //cGen(p1);
            // emitComment("Compound Statment : stmt_list");
            // cGen(p2);
            break;

        case IfK:
#if DEBUG
            printf("IfK lineno %d\n",tree->lineno);
#endif
            p1 = tree->child[0] ;
            p2 = tree->child[1] ;
            p3 = tree->child[2] ;

            /***  IF  ***/
            emitComment("IfK if");
            emitComment("if ( E1 ) S1 else S2");
            emitString("\n");
            emitComment("Begine of E1");
            emitString("\n");
            cGen(p1);
            emitString("\n");
            emitComment("End of E1");

            p1->visited=TRUE;
            //savedLoc1 = emitSkip(1) ;
            emitPop("$t0");
            emitIfFalse(jumpCnt);
            savedLoc1 = jumpCnt;
            jumpCnt++;
#if DEBUG
            printf("savedLoc1 : %d\n",savedLoc1);
#endif

            /*** THEN ***/
            emitComment("IfK Then");
            emitString("\n");
            cGen(p2);
            p2->visited=TRUE;
            emitString("\n");
            emitComment("IfK Then end");

            //savedLoc2 = emitSkip(1) ;
            if(p3 !=NULL){
                emitComment("Jump to end of Else");
                emitJump2JumpLabel(jumpCnt);
                savedLoc2 = jumpCnt;
                jumpCnt++;
#if DEBUG
                printf("savedLoc2 : %d\n",savedLoc2);
#endif
            }

            emitJumpLabel(savedLoc1);

            /*** ELSE ***/
            if(p3 != NULL){
                emitComment("IfK Else");
                emitString("\n");
                cGen(p3);
                p3->visited=TRUE;
                emitString("\n");
                emitComment("End of Else");
                emitJumpLabel(savedLoc2);
            }
            break;

        case IterK:
            // while( p1 ) p2
            emitComment(">>IterK");

            p1 = tree->child[0]; // Exp
            p2 = tree->child[1]; // Stmt

            emitJumpLabel(jumpCnt);
            savedLoc1 = jumpCnt;
            jumpCnt++;

            /* While(p1) */
            cGen(p1);
            p1->visited = TRUE;

            emitPop("$t0");
            emitIfFalse(jumpCnt);

            savedLoc2 = jumpCnt;
            jumpCnt++;

            /*  { p2} */
            cGen(p2);
            p2->visited = TRUE;
            emitJump2JumpLabel(savedLoc1);
            emitJumpLabel(savedLoc2);
            emitComment("<<IterK");
            break;

        case RetK:
            // Exp
            p1 = tree->child[0] ;
            cGen(p1);
            p1->visited = TRUE;
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
    int i;
    int loc;
    char buff[10];
    TreeNode *p1, *p2;
    if( tree->visited == TRUE ) return;
    switch (tree->kind.exp)
    {
        case AssignK:
#if DEBUG
            printf("ExpK AssignK \n");
#endif
            emitComment(">>ExpK AssignK");
            p1 = tree->child[0];
            p2 = tree->child[1];
            cGen(p2);

            emitComment("Assign to location");
            if( p1->kind.exp == IdK )
            {
                emitPop("$t1");
                emitSw("$t1",p1->location);
            }
            else if( p1->kind.exp == ArrIdK )
            {
                cGen( p1->child[0] ); // arr offset
                emitPop("$t2"); // arr[offset] -> offset
                emitCode("add     $t2, $t2, $t2");
                emitCode("add     $t2, $t2, $t2"); // offset*4
                emitCode("add     $t3, $fp, $t2"); 
                emitPop("$t1");
                emitSwAddr("$t1","$t3",p1->location);
            }
            else
            {
                emitPop("$t1");
                printf("Assign to non-Id");
            }

            // emitPush("$t1");

            p1->visited = TRUE;
            p2->visited = TRUE;

            emitComment("<<ExpK AssignK");
            break; /* assign_k */

        case OpK :
            if(tree->visited)break;
#if DEBUG
            printf("ExpK OpK\n");
#endif
            emitComment(">>ExpK OpK Plus");
            p1 = tree->child[0];
            p2 = tree->child[1];
            cGen(p2);
            cGen(p1);
            p1->visited=TRUE;
            p2->visited=TRUE;
            emitPop("$t0");
            emitPop("$t1");
            switch (tree->attr.op) {
                case PLUS:
                    emitCode("add     $t2, $t0, $t1 ");
                    break;
                case MINUS:
                    emitCode("sub     $t2, $t0, $t1 ");
                    break;
                case TIMES:
                    emitCode("mul     $t2, $t0, $t1 ");
                    break;
                case OVER:
                    emitCode("div     $t2, $t0, $t1 ");
                    break;
                case LT:
                    emitCode("slt     $t2, $t0, $t1 ");
                    break;
                case LTEQ:
                    emitCode("sle     $t2, $t0, $t1 ");
                    break;
                case GT:
                    emitCode("sgt     $t2, $t0, $t1 ");
                    break;
                case GTEQ:
                    emitCode("sge     $t2, $t0, $t1 ");
                    break;
                case EQ:
                    emitCode("seq     $t2, $t0, $t1 ");
                    break;
                case NEQ:
                    emitCode("sne     $t2, $t0, $t1 ");
                    break;

                default:
                    emitComment("BUG: Unknown operator");
                    break;
            } /* case op */
            emitPush("$t2");
            tree->visited = TRUE;
            emitComment("<<ExpK OpK");

            break; /* OpK */

        case ConstK :
            if(tree->visited)break;
#if DEBUG
            printf("ExpK ConstK\n");
#endif
            emitComment(">>ExpK CosntK");
            emitLi("$t0",tree->attr.val);
            emitPush("$t0");
            emitComment("<<ExpK CosntK");
            tree->visited=TRUE;
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
            tree->visited=TRUE;
            break; /* IdK */

        case ArrIdK :
            if(tree->visited)break;
#if DEBUG
            printf("ExpK ArrIdK | location %d\n",tree->location);
#endif
            emitComment(">>ExpK ArrIdK");
            p1 = tree->child[0];
            cGen(p1);
            emitPop("$t1"); // arr[offset] -> offset
            emitCode("add     $t1, $t1, $t1");
            emitCode("add     $t1, $t1, $t1"); // offset*4
            emitCode("add     $t2, $fp, $t1"); 
            emitLwAddr("$t0","$t2",tree->location);
            emitPush("$t0");

            emitComment("<<ExpK ArrIdK");
            tree->visited=TRUE;
            break; /* IdK */

        case CallK:
            if(tree->visited)break;
#if DEBUG
            printf("ExpK CallK %s\n",tree->attr.name);
#endif
            i = 0;
            p1 = tree->child[0]; // argement
            emitComment(">>ExpK CallK");
            while( p1 != NULL )
            {
                //param gen
                emitComment(">>Param Eval");
                cGen(p1);
                emitComment("<<Param Eval");
                //add $a0-3
                // emitCode("move    $a0, $t0");
                // sprintf(buff, "$a%d", i);
                i = i + 1;
                // emitPop(buff);
                p1 = p1->sibling;
            }
            emitCall(tree->attr.name);
            emitComment("<<ExpK CallK");
            // if( strcmp( tree->attr.name, "output" ) == 0 )
            // {
            // //do nothing
            // }
            // else if( strcmp( tree->attr.name, "input" ) == 0 || // name is input of
            // tree->type == INT ) // have return value
            // {
            // emitPop("$t0");
            // }

            emitStackPop(4*i);

            if( tree->type == INT ) // return int
            {
                emitPush("$v0");
            }
            tree->visited=TRUE;
            break;


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
    if (tree != NULL && tree->visited == FALSE)
    {
#if DEBUG
        printf("cGen lineno %d\n",tree->lineno);
#endif
        /*
           if(!(tree->visited)){
           emitComment(">>>Print for debugging");
           emitCode("li $v0, 1");
           emitLi("$t0",tree->lineno);
           emitCode("move $a0, $t0");
           emitCode("syscall");
           emitCode("li $v0, 4");
           emitCode("la $a0, nextline");
           emitCode("syscall");
           emitComment("<<<Print for debugging");
           }
           else
           return;
           */
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
        }

        switch(tree->nodekind){
            case DeclK:
                // switch(tree->kind.decl){
                // case FuncK:
                // // Function 끝에 return
                // emitFuncEnd( tree );
                // break;
                // default:
                // break;
                // }
                if( tree->kind.decl == FuncK )
                    emitFuncEnd( tree );
                break;

            case StmtK:
                switch(tree->kind.stmt){
                    case CompK:
                        if(tree->visited) break;
                        // Compound 끝에 stack관리
                        //emitCode("move $sp,$s0");

                        // offset calculation
                        offset = 0;
                        temp = tree->child[0];
                        while(temp!=NULL) {
                            switch(temp->kind.decl) {
                                case VarK:
                                    offset+=4;
                                    break;
                                case ArrVarK:
                                    offset +=4*temp->attr.arr.size;
                                    break;
                                default:
                                    break;

                            }
                            temp = temp->sibling;
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
