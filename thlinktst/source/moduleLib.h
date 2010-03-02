/*
  moduleLib.h - modified version of vxWorks' moduleLib with some 
*/

#ifndef MODULELIB_H
#define MODULELIB_H

typedef struct _module_t {
	char *name;
	int format;
	int flags;
} module_t;

#define MAXMODULES 20

static module_t modules[MAXMODULES]={0};

//this is actually in loadLib in vxWorks but for simplicity i have put it here..
MODULE_ID loadModule(int fd, int symFlag);

typedef int MODULE_ID;

//Partial implementation of moduleLib()
MODULE_ID moduleCreate(char *name, int format, int flags);
MODULE_ID moduleCreate(char *name, int format, int flags){
	int c=0;
	do {
		if(modules[c].name=NULL) {
			modules[c].name=name;
			modules[c].format=format;
			modules[c].flags=flags;
			return(c);
		}
	} while(++c<MAXMODULES);
	RETURN NULL;
}
STATUS moduleDelete(MODULE_ID moduleId);
STATUS moduleDelete(MODULE_ID moduleId) {
	if((moduleID)&&(modules[moduleID].name)) {
		modules[moduleID].name=NULL;
		return(OK);
	}
	return(ERROR);
}
MODULE_ID moduleFindByName(char *moduleName);

#endif
