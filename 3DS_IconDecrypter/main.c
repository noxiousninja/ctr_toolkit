#include "lib.h"
#include "main.h"

typedef enum
{
	gen_xorpad = 0,
	dec_icondata = 1,
} modes;

int xor_file(FILE *out, u8 *buff0, u8 *buff1, u32 size)
{
	char *xorpad = malloc(size);
	memset(xorpad,0,size);
	//memdump(stdout,"In  :",buff0,16);
	//memdump(stdout,"Out :",buff1,16);
	for(u32 i = 0; i < size; i++){
		xorpad[i] = (char)(buff0[i] ^ buff1[i]);
	}
	WriteBuffer(xorpad,size,0,out);
	free(xorpad);
	return 0;
}

int main(int argc, char *argv[])
{	
	printf("3DSGuy Icon Cache Tool\n");
	if(argc < 2){
		printf("Invalid Arguments\n");
		return 1;
	}
	int mode = strtol(argv[1],NULL,10);
	if(mode == gen_xorpad){
		printf("Generating XOR pad(s)\n");
		if(argc != 7){
			printf("usage: %s %d <00000005 image> <number of pre-existing icons> <num of icons to read> <dec icon in-dir> <xor out-dir>\n",argv[0],mode);
			return 1;
		}
		FILE *cacheD = fopen(argv[2],"rb");
		if(cacheD == NULL){
			printf("Failed to open '%s'\n",argv[2]);
			return 1;
		}
		int preexist_icon_num = strtol(argv[3],NULL,10);
		int icon_num = strtol(argv[4],NULL,10);
		char *decicon_dir = argv[5];
		char *xor_out_dir = argv[6];
		makedir(xor_out_dir);
		
		char cwd[1024];
		getcwdir(cwd,1024);
		
		chdir(decicon_dir);
		char indir[1024];
		getcwdir(indir,1024);
		chdir(cwd);
		
		chdir(xor_out_dir);
		char outdir[1024];
		getcwdir(outdir,1024);
		chdir(cwd);
		
		for(int i = preexist_icon_num; i < icon_num+preexist_icon_num; i++){
			u32 cur_offset = 0x18000 + 0x36c0*i;
			char deciconfile[100];
			memset(&deciconfile,0,100);
			sprintf(deciconfile,"icon_%d.icn",i);
			char xorfile[100];
			memset(&xorfile,0,100);
			sprintf(xorfile,"icon_%d.xor",i);
			
			chdir(indir);
			FILE *decicon = fopen(deciconfile,"rb");
			if(decicon == NULL){
				printf("Failed to open '%s'\n",deciconfile);
				return 1;
			}
			chdir(cwd);
			chdir(outdir);
			FILE *XOR_file = fopen(xorfile,"wb");
			if(XOR_file == NULL){
				printf("Failed to create '%s'\n",xorfile);
				return 1;
			}
			chdir(cwd);
			
			u8 *deciconbuff = malloc(0x36c0);
			ReadFile_64(deciconbuff,0x36c0,0,decicon);
			fclose(decicon);
			
			u8 *enciconbuff = malloc(0x36c0);
			ReadFile_64(enciconbuff,0x36c0,cur_offset,cacheD);
			
			xor_file(XOR_file,enciconbuff,deciconbuff,0x36c0);
			
			fclose(XOR_file);
		}
		
	}
	if(mode == dec_icondata){
		printf("Decrypting Icons\n");
		if(argc != 7){
			printf("usage: %s %d <00000005 image> <number of pre-existing icons> <num of icons to read> <xor in-dir> <icon out-dir>\n",argv[0],mode);
			return 1;
		}
		FILE *cacheD = fopen(argv[2],"rb");
		if(cacheD == NULL){
			printf("Failed to open '%s'\n",argv[2]);
			return 1;
		}
		int preexist_icon_num = strtol(argv[3],NULL,10);
		int icon_num = strtol(argv[4],NULL,10);
		char *xor_in_dir = argv[5];
		char *dec_icon_out_dir = argv[6];
		makedir(dec_icon_out_dir);
		
		char cwd[1024];
		getcwdir(cwd,1024);
		
		chdir(xor_in_dir);
		char indir[1024];
		getcwdir(indir,1024);
		chdir(cwd);
		
		chdir(dec_icon_out_dir);
		char outdir[1024];
		getcwdir(outdir,1024);
		chdir(cwd);
		
		for(int i = preexist_icon_num; i < icon_num+preexist_icon_num; i++){
			u32 cur_offset = 0x18000 + 0x36c0*i;
			char xor_file_path[100];
			memset(&xor_file_path,0,100);
			sprintf(xor_file_path,"icon_%d.xor",i);
			char dec_icn_path[100];
			memset(&dec_icn_path,0,100);
			sprintf(dec_icn_path,"icon_%d.icn",i);
			
			chdir(indir);
			FILE *XOR_file = fopen(xor_file_path,"rb");
			if(XOR_file == NULL){
				printf("Failed to open '%s'\n",xor_file_path);
				return 1;
			}
			chdir(cwd);
			chdir(outdir);
			FILE *dec_icon = fopen(dec_icn_path,"wb");
			if(dec_icon == NULL){
				printf("Failed to create '%s'\n",dec_icon);
				return 1;
			}
			chdir(cwd);
			
			u8 *xorbuff = malloc(0x36c0);
			ReadFile_64(xorbuff,0x36c0,0,XOR_file);
			fclose(XOR_file);
			
			u8 *enciconbuff = malloc(0x36c0);
			ReadFile_64(enciconbuff,0x36c0,cur_offset,cacheD);
			
			xor_file(dec_icon,enciconbuff,xorbuff,0x36c0);
			
			fclose(dec_icon);
		}
		
	}
	printf("Done\n");
}


