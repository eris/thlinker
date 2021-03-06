#include "symLib.h"
#include "symbols.h"
#include <string.h>

#define SYMTBLMAXENTRIES 2000

SYMBOL symTbl[SYMTBLMAXENTRIES];
int sysSymTbl;

STATUS symLibInit (void) {
	sysSymTbl=0;
	bzero(symTbl, SYMTBLMAXENTRIES*sizeof(SYMBOL));
	return(OK);
}

SYMTAB_ID symTblCreate(int hashSizeLog2, BOOL sameNameOk, PART_ID symPartId) {
	//There is only one symbol table for now
	return(symTbl);
}

STATUS symTblDelete(SYMTAB_ID symTblId) {
	//huh?
	return(ERROR);
}

//both chars are const 
STATUS symAdd(SYMTAB_ID symTblId, char *name, char *value, SYM_TYPE type, UINT16 group) {
	int c;
	c=0;
	do {
		if(symTbl[c].name==NULL) {
			symTbl[c].name=name;
			symTbl[c].value=value;
			symTbl[c].type=type;
			symTbl[c].group=group;
			return(OK);
		}
	} while(++c<SYMTBLMAXENTRIES);
	return(ERROR);
}

STATUS symRemove(SYMTAB_ID symTblId, char *name, SYM_TYPE type) {
	int c;
	c=0;
	do {
		if(strcmp(symTbl[c].name,name)==0) {
			symTbl[c].name=NULL;
			symTbl[c].value=NULL;
			symTbl[c].type=0;
			symTbl[c].group=0;
			return(OK);
		}
	} while(++c<SYMTBLMAXENTRIES);
	return(ERROR);
}

STATUS symFindByName(SYMTAB_ID symTblId, char *name, char **pValue, SYM_TYPE *pType) {
	int c;
	c=0;
	do {
		if(strcmp(symTbl[c].name,name)==0) {
			*pValue=symTbl[c].value;
			*pType=symTbl[c].type;
			return(OK);
		}
	} while(++c<SYMTBLMAXENTRIES);
	return(ERROR);
}

STATUS symFindByNameAndType(SYMTAB_ID symTblId, char *name, char **pValue, SYM_TYPE * pType, SYM_TYPE sType, SYM_TYPE mask) {
	return(ERROR);
}

STATUS symByValueFind(SYMTAB_ID symTblId, UINT value, char **pName, int *pValue, SYM_TYPE *pType) {
	int c;
	c=0;
	do {
		if(symTbl[c].value==(char *)pValue) {
			*pName=symTbl[c].name;
			*pType=symTbl[c].type;
			return(OK);
		}
	} while(++c<SYMTBLMAXENTRIES);
	return(ERROR);
}

STATUS symByValueAndTypeFind(SYMTAB_ID symTblId, UINT value, char **pName, int *pValue, SYM_TYPE *pType, SYM_TYPE sType, SYM_TYPE mask) {
	return(ERROR);
}

//OBSOLETE! (according to docs)
STATUS symFindByValue(SYMTAB_ID symTblId, UINT value,char *name, int *pValue, SYM_TYPE *pType) {
	return(ERROR);
}

STATUS symFindByValueAndType(SYMTAB_ID  symTblId, UINT value, char *name, int *pValue, SYM_TYPE *pType, SYM_TYPE sType, SYM_TYPE mask) {
	return(ERROR);
}

/*SYMBOL *symEach(SYMTAB_ID symTblId, FUNCPTR routine, int routineArg) {
	return(NULL);
}*/

