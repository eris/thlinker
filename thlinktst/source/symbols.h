/*
	symbols.h - compatible with VxWork's symbols.h 

  "If you use the symLib subroutines to manipulate the VxWorks system symbol table (whose ID is recorded in the global sysSymTbl) are"
*/
#ifndef SYMBOLS_H
#define SYMBOLS_H

//vxWorks types required (put these someplace more proper l8r!)
typedef int PART_ID;
typedef unsigned int UINT;
typedef int STATUS;
typedef int BOOL;
typedef unsigned int UINT16;

typedef enum {
	SYM_UNDF=0,
	SYM_LOCAL,
	SYM_GLOBAL,
	SYM_ABS,
	SYM_TEXT,
	SYM_DATA,
	SYM_BSS,
	SYM_COMM,
} SYM_TYPE; 

typedef struct _SYMBOL {
	char *name;
	char *value;
	SYM_TYPE  type;
	UINT16    group;
} SYMBOL;

#endif
