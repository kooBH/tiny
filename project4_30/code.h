/****************************************************/
/* File: code.h                                     */
/* Code emitting utilities for the TINY compiler    */
/* and interface to the TM machine                  */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _CODE_H_
#define _CODE_H_

/* code emitting utilities */

void emitString(char* c);

/* just Emit Label */
void emitLabel(char* c);

/* emitJal*/
void emitJal(char* c);

/* just write some codes with location
 * */
void emitCode(char* c);

void emitJAL(char* label);

void emitPush(char* reg);
void emitPop(char* reg);

void emitStackPop(int offset);

void emitFuncStart();
void emitFuncEnd();
void emitCall(char* label);

/* Procedure emitComment prints a comment line 
 * with comment c in the code file
 */
void emitComment( char * c );
void emitStartup();

#endif
