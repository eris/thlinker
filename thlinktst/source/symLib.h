/*
  symLib.h - VxWorks compatible symbol library
*/

#ifndef SYMLIB_H
#define SYMLIB_H
#include "symbols.h"

typedef SYMBOL *SYMTAB_ID;	//prolly not how vxWorks does it, at all... 
extern int sysSymTbl;

#define OK 0
#define ERROR -1

STATUS symLibInit (void);
SYMTAB_ID symTblCreate(int hashSizeLog2, BOOL sameNameOk, PART_ID symPartId);
STATUS symTblDelete(SYMTAB_ID symTblId);
STATUS symAdd(SYMTAB_ID symTblId, char *name, char *value, SYM_TYPE type, UINT16 group);
STATUS symRemove(SYMTAB_ID symTblId, char *name, SYM_TYPE type);
STATUS symFindByName(SYMTAB_ID symTblId, char *name, char **pValue, SYM_TYPE * pType);
STATUS symFindByNameAndType(SYMTAB_ID symTblId, char *name, char **pValue, SYM_TYPE * pType, SYM_TYPE sType, SYM_TYPE mask);
STATUS symByValueFind(SYMTAB_ID symTblId, UINT value, char **pName, int *pValue, SYM_TYPE *pType);
STATUS symByValueAndTypeFind(SYMTAB_ID symTblId, UINT value, char **pName, int *pValue, SYM_TYPE *pType, SYM_TYPE sType, SYM_TYPE mask);
STATUS symFindByValue(SYMTAB_ID symTblId, UINT value,char *name, int *pValue, SYM_TYPE *pType);
STATUS symFindByValueAndType(SYMTAB_ID  symTblId, UINT value, char *name, int *pValue, SYM_TYPE *pType, SYM_TYPE sType, SYM_TYPE mask);
//SYMBOL *symEach(SYMTAB_ID symTblId, FUNCPTR routine, int routineArg);

#endif
