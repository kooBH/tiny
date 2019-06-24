/****************************************************/
/* File: code.c                                     */
/* TM Code emitting utilities                       */
/* implementation for the TINY compiler             */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "code.h"

/* SPIM label number for current instruction emission */
static int emitLoc = 0 ;

void emitLa(char* Rdest, int loc)
{
    if (TraceCode)
        fprintf(code,"L%-3d: la      %s, %d($fp)\n",emitLoc,Rdest,loc);
    else
        fprintf(code,"    la      %s, %d($fp)\n",Rdest,loc);
    emitLoc++;
}

void emitMove(char*Rdest, char*Rsrc)
{
    if (TraceCode)
        fprintf(code,"L%-3d: move    %s, %s\n",emitLoc,Rdest,Rsrc);
    else
        fprintf(code,"    move    %s, %s\n",Rdest,Rsrc);
    emitLoc++;
}

void emitLi(char*Rdest,int v)
{
    if (TraceCode)
        fprintf(code,"L%-3d: li      %s, %-3d\n",emitLoc,Rdest,v);
    else
        fprintf(code,"    li      %s, %-3d\n",Rdest,v);
    emitLoc++;
}

void emitLw(char* Rdest, int offset)
{
    if (TraceCode)
        fprintf(code,"L%-3d: lw      %s, %3d($fp)\n",emitLoc,Rdest,offset);
    else
        fprintf(code,"    lw      %s, %3d($fp)\n",Rdest,offset);
    emitLoc++;
}

void emitLwAddr(char* Rdest, char* Rsrc, int offset)
{
    if (TraceCode)
        fprintf(code,"L%-3d: lw      %s, %3d(%s)\n",emitLoc,Rdest,offset,Rsrc);
    else
        fprintf(code,"    lw      %s, %3d(%s)\n",Rdest,offset,Rsrc);
    emitLoc++;
}

void emitSw(char* Rsrc, int offset)
{
    if (TraceCode)
        fprintf(code,"L%-3d: sw      %s, %3d($fp)\n",emitLoc,Rsrc,offset);
    else
        fprintf(code,"    sw      %s, %3d($fp)\n",Rsrc,offset);
    emitLoc++;
}

void emitSwAddr(char* Rsrc, char* Rdest, int offset)
{
    if (TraceCode)
        fprintf(code,"L%-3d: sw      %s, %3d(%s)\n",emitLoc,Rsrc,offset,Rdest);
    else
        fprintf(code,"    sw      %s, %3d(%s)\n",Rsrc,offset,Rdest);
    emitLoc++;
}

void emitJump2JumpLabel(int c)
{
    if (TraceCode)
        fprintf(code,"L%-3d: j       J%d\n",emitLoc,c);
    else
        fprintf(code,"    j        J%d\n",c);
    emitLoc++;
}

void emitJumpLabel(int c)
{
    fprintf(code,"J%d:\n",c);
}

void emitIfFalse(int c)
{
    if (TraceCode)
        fprintf(code,"L%-3d: beqz    $t0, J%d\n",emitLoc,c);
    else
        fprintf(code,"    beqz    $t0, J%d\n",c);
    emitLoc++;
}

void emitIfTrue(int c)
{
    if (TraceCode)
        fprintf(code,"L%-3d: bnez    $t0, J%d\n",emitLoc,c);
    else
        fprintf(code,"    bnez    $t0, J%d\n",c);
    emitLoc++;
}

void emitStackPop(int offset)
{
    if (TraceCode)
        fprintf(code,"L%-3d: addi    $sp, $sp, %d\n",emitLoc,offset);
    else
        fprintf(code,"    addi    $sp, $sp, %d\n",offset);
    emitLoc++;
}
void emitStackPush(int offset)
{
    if (TraceCode)
        fprintf(code,"L%-3d: addi    $sp, $sp, -%d\n",emitLoc,offset);
    else
        fprintf(code,"    addi    $sp, $sp, -%d\n",offset);
    emitLoc++;
}

void emitString(char* c)
{
    if (TraceCode) fprintf(code,"%s",c);
}

void emitJal(char*c)
{
    if (TraceCode)
        fprintf(code,"%3d: jal     %s\n",emitLoc,c);
    else
        fprintf(code,"    jal     %s\n",c);
    emitLoc++;
}

void emitLabel(char* c)
{
    fprintf(code,"%s:\n",c);
}

/* just write some codes*/
void emitCode(char* c)
{
    if ( TraceCode )
        fprintf(code,"L%-3d: %s\n",emitLoc,c);
    else
        fprintf(code,"    %s\n",c);
    ++emitLoc ;
}

void emitJAL(char* label)
{
    char* c = (char *)malloc(strlen(label) + 9);
    strcpy( c, "jal     " );
    strcat( c, label );
    emitCode( c );
    free(c);
}

void emitPush(char* reg)
{
    char* c = (char *)malloc(strlen(reg) + 17);
    emitCode("sub     $sp, $sp, 4");
    strcat( c, "sw      " );
    strcat( c, reg );
    strcat( c, ", 0($sp)" );
    emitCode( c );
    free(c);
}

void emitPop(char* reg)
{
    char* c = (char *)malloc(strlen(reg) + 17);
    strcat( c, "lw      " );
    strcat( c, reg );
    strcat( c, ", 0($sp)" );
    emitCode( c );
    emitCode("addi    $sp, $sp, 4");
    free(c);
}

void emitFuncStart(TreeNode* tree)
{
    emitComment("Start of FuncK");
    emitComment("Build Activation Recode");
    emitCode("sub     $sp, $sp, 8");
    emitCode("sw      $fp, 4($sp)");
    emitCode("sw      $ra, 0($sp)");
    emitCode("add     $fp, $sp, 4");
    emitComment("FuncK body start here");
}

void emitFuncEnd(TreeNode* tree)
{
    emitComment("FuncK body end here");
    emitComment("Destruct Activation Recode");
    emitCode("lw      $ra, 0($sp)");
    emitCode("lw      $fp, 8($sp)");
    emitCode("sub     $sp, $sp, 8");
    emitCode("jr      $ra");
    emitComment("End of FuncK");
}

void emitCall(char* label)
{
    emitJAL(label);
}

/* Procedure emitComment prints a comment line 
 * with comment c in the code file
 */
void emitComment( char * c )
{
    if (TraceCode) fprintf(code,"# %s\n",c);
}

void emitStartup()
{
    emitString("    .data\n");
    emitString("nextline:    .asciiz \"\\n\"\n");
    emitString("    .text\n");
    emitLabel("main");
    emitCode("add     $fp, $sp, 4");
    emitCode("jal     __main");
    emitCode("li      $v0, 10");
    emitCode("syscall");
    emitString("\n");

    emitComment("Code for void output(int) function");
    emitLabel("output");
    emitCode("sub     $sp, $sp, 8");
    emitCode("sw      $fp, 4($sp)");
    emitCode("sw      $ra, 0($sp)");
    emitCode("add     $fp, $sp, 4");
    emitString("\n");
    emitCode("li      $v0, 1");
    emitCode("syscall");
    emitString("\n");

    /* add next line*/
    emitCode("li $v0, 4");
    emitCode("la $a0, nextline");
    emitCode("syscall");

    emitCode("lw      $ra, 0($sp)");
    emitCode("lw      $fp, 4($sp)");
    emitCode("add     $sp, $sp, 8");
    emitCode("jr      $ra");
    emitString("\n");

    emitComment("Code for int input(void) function");
    emitLabel("input");
    emitCode("sub     $sp, $sp, 8");
    emitCode("sw      $fp, 4($sp)");
    emitCode("sw      $ra, 0($sp)");
    emitCode("add     $fp, $sp, 4");
    emitString("\n");
    emitCode("li      $v0, 5");
    emitCode("syscall");
    emitString("\n");
    emitCode("lw      $ra, 0($sp)");
    emitCode("lw      $fp, 4($sp)");
    emitCode("add     $sp, $sp, 8");
    emitPush("$v0");
    emitCode("jr      $ra");
    emitString("\n");
}

