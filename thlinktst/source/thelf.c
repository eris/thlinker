#include <nds.h>
#include "elf.h"
#include <stdio.h>
#include <malloc.h>
#include "symLib.h"

//Get 'n'ame from 's'ection
#define elfGetName(s,n) (&sections[s].data[n])


typedef struct _elfsection_t {
	Elf32_Shdr *shdr;	//pointer to the section header
	char *data;		//section data
} elfSection_t;


void elfParseRel(elfSection_t *sections, elfSection_t *relsec) {
	Elf32_Shdr *rel; //current section hdr
	rel=relsec->shdr;

}

//borrowed from client server example
unsigned int endianFixl(unsigned int lc) {
        return((lc>>24)|((lc>>8)&0xff00)|((lc<<8)&0xff0000)|(lc<<24));
}

void elfEndianFix(u32 *buffer, int size) {
	size>>=2;
	while(size--) {
		*buffer=endianFixl(*buffer);
		buffer++;
	};
}

SYM_TYPE elf2VxType(int elftype) {
	SYM_TYPE vxtype;
	switch(elftype) {
		case STT_FUNC:
			vxtype=SYM_TEXT;
			break;
		case STT_OBJECT:
			vxtype=SYM_DATA;
			break;
		case STT_SECTION:
			vxtype=SYM_ABS;
			break;
		default:
			vxtype=SYM_GLOBAL;
			break;
	}
}

//First pass load loadable sections into ram, add known symbols + loadOffsets, fix undefs, enjoy!

void elfLoad(elfSection_t *sections, int hdrcnt, elfSection_t *symSection, int headerStringTable) {
	int cnt;
	int c;
	Elf32_Shdr *shdr; //current section hdr
	char *sdata;
	Elf32_Sym *sym;
	u16 upper;
	u16 lower;
	//add known symbols + loadOffsets
	cnt=0;
	printf("First pass... (define known syms within each loaded section)\n");
	do {
		shdr=sections[cnt].shdr;
		sdata=sections[cnt].data;
		if(shdr->sh_flags&SHF_ALLOC) {	//Section is loadable
			c=0;
			int maxcnt;
			Elf32_Shdr *symSecHdr;
			symSecHdr=symSection->shdr;
			maxcnt=symSecHdr->sh_size/symSecHdr->sh_entsize;	//seems an odd way of deriving this info
//			printf("maxcnt:%d\n",maxcnt);
			sym=(Elf32_Sym*)symSection->data;
//			printf("section[%d] \"%s\"\n",cnt, elfGetName(headerStringTable, shdr->sh_name));
			do {
				if((sym->st_shndx==cnt)&&(*elfGetName(symSecHdr->sh_link, sym->st_name)!=0)) { //symbol is for this section and not blank
					symAdd(sysSymTbl, elfGetName(symSecHdr->sh_link, sym->st_name), (char*)sdata+sym->st_value, elf2VxType(ELF32_ST_TYPE(sym->st_info)), cnt);
//					printf("[%d] \"%s\" ", c, elfGetName(symSecHdr->sh_link, sym->st_name));
//					printf("value:%08x size: %d info: %d other: %x shndx: %d\n", sdata+sym->st_value, sym->st_size, sym->st_info, sym->st_other, sym->st_shndx);

				}
				sym++;
			} while(++c<maxcnt);
		}
	} while(++cnt<hdrcnt);
//	printf("Correcting endians....\n");
//	elfEndianFix(sections[1].data, sections[1].shdr->sh_size);
	printf("Second pass (relocation, correct address references)...\n");
	//fixUndefs
	cnt=0;
	do {
		shdr=sections[cnt].shdr;
		if(shdr->sh_flags&SHF_ALLOC) {	//Section is loadable
			if(shdr->sh_flags&SHF_EXECINSTR) { //Section is executable (deal w/reloc tbl)
				Elf32_Sym *syms;
				Elf32_Rel *rel;
				Elf32_Shdr *relSecHdr;
				int maxcnt;
				relSecHdr=sections[cnt+1].shdr;		//THIS CANT BE BEST WAY TO DETERMINE REL
				c=0;
				rel=(Elf32_Rel*)sections[cnt+1].data;
				syms=(Elf32_Sym*)symSection->data;
				maxcnt=relSecHdr->sh_size/relSecHdr->sh_entsize;
//printf("sec:%d\n",maxcnt);
				do {
					sym=&syms[ELF32_R_SYM(rel->r_info)];

//printf("External %s  type:%x ", elfGetName(sections[relSecHdr->sh_link].shdr->sh_link, sym->st_name), ELF32_R_TYPE(rel->r_info));					printf("info:%x sym:%d ", rel->r_info, ELF32_R_SYM(rel->r_info));
//					printf("name %08x address %08x\n", elfGetName(sections[relSecHdr->sh_link].shdr->sh_link, sym->st_name), rel->r_offset);
					char *addr;
					char *reladdr;
					SYM_TYPE type;
					char *name;
					name=elfGetName(sections[relSecHdr->sh_link].shdr->sh_link, sym->st_name);
					if(sym->st_shndx==0) {	//extern linkage (lookup from symtab)
						printf("ext %s\n",name);
						if(symFindByName(sysSymTbl, name, &addr, &type)==ERROR) {
							printf("cannot find symbol %s?!\n",name);
							while(1);
						}
					} else {	//internal linkage
						printf("Int from %s\n", elfGetName(headerStringTable, sections[sym->st_shndx].shdr->sh_name));
						addr=sections[sym->st_shndx].data;
						
					}
//printf("sv: %08x,", sym->st_value);
					addr+=sym->st_value;
					reladdr=sections[cnt].data+rel->r_offset;
					int val=((reladdr[3])<<24)|((reladdr[2])<<16)|((reladdr[1])<<8)|(reladdr[0]);
//printf("t:%d,",ELF32_R_TYPE(rel->r_info));
					unsigned int cond;
					switch(ELF32_R_TYPE(rel->r_info)) {
						case R_ARM_ABS32:
							val+=(unsigned int)addr;
					reladdr[3]=val>>24;
					reladdr[2]=val>>16;
					reladdr[1]=val>>8;
					reladdr[0]=val;

							break;
						case R_ARM_CALL:	
						case R_ARM_THM_PC22: // aka R_ARM_THM_CALL?
							upper=(reladdr[1]<<8)|reladdr[0];
							lower=(reladdr[3]<<8)|reladdr[2];
							val=((upper&0x07ff)<<11)|(lower&0x07ff)<<1;
							val=(val^(1<<22))-(1<<22);
							val=(int)addr-(int)reladdr+val;
							val>>=1;
							upper=((val>>11)&0x7ff)|(upper&0xf800);
							lower=(val&0x7ff)|(lower&0xf800);
							reladdr[1]=upper>>8;
							reladdr[0]=upper;
							reladdr[3]=lower>>8;
							reladdr[2]=lower;

//printf("v:%08x\n",*(int *)reladdr);
//							val=((((unsigned int)addr-(unsigned int)reladdr)+val)>>2)&0x00ffffff;
//							val|=cond;
							break;
						default:
							break;
					}
//					printf("fv:%08x\n",*((int*)reladdr));
//					section[sym->st_shndx].data
					rel++;
				} while(++c<maxcnt);
//				elfParseRel(sections, &sections[cnt+1]); //reloc table
			} 
		}
	} while(++cnt<hdrcnt);
}

int elfHandleSectionHeaders(elfSection_t *sections, int hdrcnt, int headerStringTable) {
	int cnt;
	Elf32_Shdr *shdr; //current section hdr
	cnt=0;
	int retval;
	retval=0;
	do {
		shdr=sections[cnt].shdr;
//		printf("Section [%d]: \"%s\" ",cnt, elfGetName(headerStringTable, shdr->sh_name));
//		printf("size: %d flags: %x ", shdr->sh_size, shdr->sh_flags);
//		printf("addr: %08x link: %d info: %x align: %x esize %d:",shdr->sh_addr, shdr->sh_link, shdr->sh_info, shdr->sh_addralign, shdr->sh_entsize);
		symAdd(sysSymTbl, elfGetName(headerStringTable, shdr->sh_name), (char*)sections[cnt].data, SYM_LOCAL, cnt);
		int maxcnt;
		if(shdr->sh_entsize) {
			maxcnt=shdr->sh_size/shdr->sh_entsize;
//			printf("maxcnt:%d ",maxcnt);
		}
		switch (shdr->sh_type) {
			case SHT_STRTAB:
//				printf("string table!\n");
				break;
			case SHT_SYMTAB:
				printf("symbol table!\n");
				retval=cnt;
				int c;
				c=0;
				int maxcnt;
				Elf32_Sym *sym;
				maxcnt=shdr->sh_size/shdr->sh_entsize;	//seems an odd way of deriving this info
/*				printf("maxcnt:%d\n",maxcnt);
				sym=(Elf32_Sym*)sections[cnt].data;
				do {
					printf("[%d] \"%s\" ", c, elfGetName(shdr->sh_link, sym->st_name));
					printf("value:%08x size: %d info: %d other: %x shndx: %d\n", sym->st_value, sym->st_size, sym->st_info, sym->st_other, sym->st_shndx);
					sym++;
				} while(++c<maxcnt);
*/
				break;
			case SHT_PROGBITS:
				if(strcmp(elfGetName(headerStringTable, shdr->sh_name),".text")==NULL)	
					printf("%s @ %x\n", elfGetName(headerStringTable, shdr->sh_name), sections[cnt].data);
//				printf("program data!\n");
				break;
			case SHT_REL:
			//	printf("relocation entries!\n");
				if(strcmp(elfGetName(headerStringTable, shdr->sh_name),".rel.text")==NULL)	
					printf("%s @ %x\n", elfGetName(headerStringTable, shdr->sh_name), sections[cnt].data);
/*				int c;
				Elf32_Sym *syms;
				Elf32_Sym *sym;
				Elf32_Rel *rel;
				c=0;
				rel=(Elf32_Rel*)sections[cnt].data;
				syms=(Elf32_Sym*)sections[shdr->sh_link].data;
				do {
					sym=&syms[ELF32_R_SYM(rel->r_info)];
printf("%d\n",ELF32_R_SYM(rel->r_info));
					
					printf("%s  type:%x ",
elfGetName(sections[shdr->sh_link].shdr->sh_link, sym->st_name),
 ELF32_R_TYPE(rel->r_info));

					printf("%x %d", rel->r_info, sections[shdr->sh_link].shdr->sh_link);
					printf("address %08x\n", rel->r_offset);
					rel++;
				} while(++c<maxcnt);
	*/			break;
			case SHT_HASH:
//				printf("symbol hash table!\n");
				break;
			case SHT_DYNAMIC:
//				printf("dynamic linking information!\n");
				break;
			case SHT_DYNSYM:
//				printf("dynamic linker symbol table\n");
				break;
			case SHT_NOBITS:
					printf("%s @ %x\n", elfGetName(headerStringTable, shdr->sh_name), sections[cnt].data);
				break;
			default:
				if((shdr->sh_type>=SHT_LOPROC)&&(shdr->sh_type<SHT_HIPROC)) {
//					printf("unkown processor specific header %08x\n",shdr->sh_type);
				} else {
					if(shdr->sh_type!=0)
						printf("unknown header type: %d\n",shdr->sh_type);
				}
				break;
		};
//		printf("%08x\n",shdr->sh_link);
	} while(++cnt<hdrcnt);
	return(retval);
}


//Load section header data
int elfReadSectionHeaders(FILE *file, int offset, Elf32_Shdr *shdrs, elfSection_t *sections, int nsections) {
	int cnt;
	cnt=0;
	fseek(file, offset,SEEK_SET);
	fread(shdrs, sizeof(Elf32_Shdr), nsections, file);	//read section headers
	do {
		sections[cnt].shdr=&shdrs[cnt];
		if(shdrs[cnt].sh_size!=0) {
			sections[cnt].data=(char*)malloc(shdrs[cnt].sh_size);
			fseek(file, shdrs[cnt].sh_offset, SEEK_SET);
			fread(sections[cnt].data, sizeof(char), shdrs[cnt].sh_size, file);
		} else {
			sections[cnt].data=NULL;
		}
	} while(++cnt<nsections);	//read section data
	return(1);
}

//Free section header data
void elfFreeSectionHeaders(elfSection_t *sections, int nsections) {
	int cnt;
	cnt=0;
	do {
		if(sections[cnt].data) {
			free(sections[cnt].data);
		}
	} while(++cnt<nsections);
}

//4 test
typedef void(*_otherFunc)(void);
typedef int(*_someFunc)(int,char*);

int elfStart(char *filename) {
	Elf32_Ehdr ehdr;
	FILE *f;
	elfSection_t *sections;
	printf("jaja..\n");
	symLibInit();
	SYMTAB_ID syms;
	syms=symTblCreate(0,0,0);	//this rly does nothing
	symAdd(syms, "puts", (char*)&puts, SYM_TEXT, sysSymTbl);
	symAdd(syms, "printf", (char*)&printf, SYM_TEXT, sysSymTbl);
	char *value;
	SYM_TYPE type;
	symFindByName(syms, "puts", &value, &type);
//	printf("found %08x %d\n",value, type);
	printf("opening: %s\n", filename);
	f=fopen(filename,"rb");
	fread(&ehdr, sizeof(Elf32_Ehdr), 1, f);
	if(memcmp(ehdr.e_ident, ELFMAG, SELFMAG)==0) {
		printf("Zomg its elf! Data-Type: %x\n",ehdr.e_ident[EI_DATA]);
/*
	printf("Machine: %d\n", ehdr.e_machine);
	printf("OBJFileType: %d\n", ehdr.e_type);
	printf("Version: %d\n", ehdr.e_version);
	printf("Entry: %08x\n", ehdr.e_entry);
	printf("ProgHdrSz: %d\n", ehdr.e_phentsize);
	printf("ProgHdrOset: %08x\n", ehdr.e_phoff);
	printf("ProgHdrCnt: %d\n", ehdr.e_phnum);
	printf("SectionHdrSize: %d\n", ehdr.e_shentsize);
	printf("SectionHdrCnt: %d\n", ehdr.e_shnum);
	printf("SectionHdrOffset: %08x\n", ehdr.e_shoff);
	printf("SectionHdrStrngTblNdx: %d\n", ehdr.e_shstrndx);
*/		Elf32_Shdr *shdrs;
		shdrs=(Elf32_Shdr*)malloc(sizeof(Elf32_Shdr)*ehdr.e_shnum);
		if(shdrs) {
			sections=(elfSection_t *)malloc(sizeof(elfSection_t)*ehdr.e_shnum);
			if(sections) {
				int symSecNdx;
				printf("Reading headers...\n");
				elfReadSectionHeaders(f, ehdr.e_shoff, shdrs, sections, ehdr.e_shnum);
				printf("Handle headers...\n");
				symSecNdx=elfHandleSectionHeaders(sections, ehdr.e_shnum, ehdr.e_shstrndx);
				printf("Linking...\n");
				elfLoad(sections, ehdr.e_shnum, &sections[symSecNdx],ehdr.e_shstrndx);
				if(symFindByName(syms, "otherFunc", &value, &type)==OK) {
					printf("cross yer finger! ((_otherFunc)%08x)():\n", value);
					((_otherFunc)value)();
				}
				if(symFindByName(syms, "someFunc", &value, &type)==OK) {
					printf("cross yer finger! ((_someFunc)%08x)():\n", value);
					printf("return val=%d\n",((_someFunc)value)(100,elfGetName(ehdr.e_shstrndx, 2)));
				}
//printf("%08x @ %08x\n",*((int*)sections[1].data), sections[1].data);
//	symFindByName(syms, "puts", &value, &type);
//	printf("found %08x=%08x ?, %d\n",value, &puts, type);
while(1);
				elfFreeSectionHeaders(sections, ehdr.e_shnum);
				free(sections);
			}
			free(shdrs);
		} else {
			printf("Unable to handle section headers\n");
		}
	}
	fclose(f);
	return(1);
}
