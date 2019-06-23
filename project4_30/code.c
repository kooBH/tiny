/****************************************************/
/* File: code.c                                     */
/* TM Code emitting utilities                       */
/* implementation for the TINY compiler             */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "code.h"

/* TM location number for current instruction emission */
static int emitLoc = 0 ;

/* Highest TM location emitted so far
   For use in conjunction with emitSkip,
   emitBackup, and emitRestore */
static int highEmitLoc = 0;


void emitJump2JumpLabel(int c){
    if (TraceCode) fprintf(code,"L%-3d: j J%-3d\n",emitLoc,c);
    emitLoc++;
}

void emitJumpLabel(int c){
    if (TraceCode) fprintf(code,"J%-3d: \n",c);
}

void emitIfFalse(int c){
    if (TraceCode) fprintf(code,"L%-3d: beqz $t0, J%-3d\n",emitLoc,c);
    emitLoc++;
}

void emitIfTrue(int c){
    if (TraceCode) fprintf(code,"L%-3d: bnez $t0, J%-3d\n",emitLoc,c);
    emitLoc++;


void emitStackPop(int offset){
    if (TraceCode) fprintf(code,"L%-3d: addi $sp, $sp, %d\n",emitLoc,offset);
    emitLoc++;
}

void emitString(char* c)
{
    if (TraceCode) fprintf(code,"%s",c);
}

void emitJal(char*c)
{
    if (TraceCode) fprintf(code,"%3d: jal %s\n",emitLoc,c);
    emitLoc++;
}

void emitLabel(char* c)
{
    if (TraceCode) fprintf(code,"%s:\n",c);
}

/* just write some codes*/
void emitCode(char* c)
{
    if (TraceCode) fprintf(code,"L%-3d: %s\n",emitLoc,c);
    ++emitLoc ;
}

void emitJAL(char* label)
{
    char* c = (char *)malloc(strlen(label) + 9);
    strcpy( c, "jal     " );
    strcat( c, label );
    emitCode( c );
}

void emitFuncStart()
{
#if DEBUG
    emitComment("Start of FuncK");
#endif
    emitCode("sub     $sp, $sp, 8");
    emitCode("sw      $fp, 4($sp)");
    emitCode("sw      $ra, 0($sp)");
    emitCode("sub     $fp, $sp, 4");
#if DEBUG
    emitComment("FuncK body start here");
#endif
}

void emitFuncEnd()
{
#if DEBUG
    emitComment("FuncK body end here");
#endif
    emitCode("lw      $ra, 0($sp)");
    emitCode("lw      $fp, 4($sp)");
    emitCode("addi    $sp, $sp, 4");
    emitCode("jr      $ra");
#if DEBUG
    emitComment("End of FuncK");
#endif
}

void emitCall(char* label)
{
    emitComment("Start of CallK");
    emitCode("sub     $sp, $sp, 4");
    emitCode("sw      $a0, 0($sp)");
    emitJAL(label);
    emitComment("Function return here");
    emitCode("addi    $sp, $sp, 4");
    emitComment("End of CallK");
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
    emitString("    .text\n");
    emitLabel("main");
    emitCode("jal     __main");
    emitCode("li      $v0, 10");
    emitCode("syscall");
    emitString("\n");

    emitComment("Code for void output(int) function");
    emitLabel("output");
    emitCode("sub     $sp, $sp, 8");
    emitCode("sw      $fp, 4($sp)");
    emitCode("sw      $ra, 0($sp)");
    emitCode("move    $fp, $sp");
    emitString("\n");
    emitCode("li      $v0, 1");
    emitCode("syscall");
    emitString("\n");
    emitCode("lw      $ra, 0($sp)");
    emitCode("lw      $fp, 4($sp)");
    emitCode("addi    $sp, $sp, 8");
    emitCode("jr      $ra");
    emitString("\n");

    emitComment("Code for int input(void) function");
    emitLabel("input");
    emitCode("sub     $sp, $sp, 8");
    emitCode("sw      $fp, 4($sp)");
    emitCode("sw      $ra, 0($sp)");
    emitCode("move    $fp, $sp");
    emitString("\n");
    emitCode("li      $v0, 5");
    emitCode("syscall");
    emitString("\n");
    emitCode("lw      $ra, 0($sp)");
    emitCode("lw      $fp, 4($sp)");
    emitCode("addi    $sp, $sp, 8");
    emitCode("jr      $ra");
    emitString("\n");
}
