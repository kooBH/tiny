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

/* Procedure emitRO emits a register-only
 * TM instruction
 * op = the opcode
 * r = target register
 * s = 1st source register
 * t = 2nd source register
 * c = a comment to be printed if TraceCode is TRUE
 */
void emitRO( char *op, int r, int s, int t, char *c)
{
    fprintf(code,"%3d:  %5s  %d,%d,%d ",emitLoc++,op,r,s,t);
    if (TraceCode) fprintf(code,"\t%s",c) ;
    fprintf(code,"\n") ;
    if (highEmitLoc < emitLoc) highEmitLoc = emitLoc ;
} /* emitRO */

/* Procedure emitRM emits a register-to-memory
 * TM instruction
 * op = the opcode
 * r = target register
 * d = the offset
 * s = the base register
 * c = a comment to be printed if TraceCode is TRUE
 */
void emitRM( char * op, int r, int d, int s, char *c)
{
    fprintf(code,"%3d:  %5s  %d,%d(%d) ",emitLoc++,op,r,d,s);
    if (TraceCode) fprintf(code,"\t%s",c) ;
    fprintf(code,"\n") ;
    if (highEmitLoc < emitLoc)  highEmitLoc = emitLoc ;
} /* emitRM */

/* Function emitSkip skips "howMany" code
 * locations for later backpatch. It also
 * returns the current code position
 */
int emitSkip( int howMany)
{
    int i = emitLoc;
    emitLoc += howMany ;
    if (highEmitLoc < emitLoc)  highEmitLoc = emitLoc ;
    return i;
} /* emitSkip */

/* Procedure emitBackup backs up to 
 * loc = a previously skipped location
 */
void emitBackup( int loc)
{
    if (loc > highEmitLoc) emitComment("BUG in emitBackup");
    emitLoc = loc ;
} /* emitBackup */

/* Procedure emitRestore restores the current 
 * code position to the highest previously
 * unemitted position
 */
void emitRestore(void)
{
    emitLoc = highEmitLoc;
}

/* Procedure emitRM_Abs converts an absolute reference 
 * to a pc-relative reference when emitting a
 * register-to-memory TM instruction
 * op = the opcode
 * r = target register
 * a = the absolute location in memory
 * c = a comment to be printed if TraceCode is TRUE
 */
void emitRM_Abs( char *op, int r, int a, char * c)
{
    fprintf(code,"%3d:  %5s  %d,%d(%d) ",
               emitLoc,op,r,a-(emitLoc+1),pc);
    ++emitLoc ;
    if (TraceCode) fprintf(code,"\t%s",c) ;
    fprintf(code,"\n") ;
    if (highEmitLoc < emitLoc) highEmitLoc = emitLoc ;
} /* emitRM_Abs */
