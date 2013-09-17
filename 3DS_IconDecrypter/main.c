#include "lib.h"

#define SMDH_SIZE 0x36c0
#define MAX_CACHE_NUM 360
#define CACHE_EXTDATA_OFFSET 0x18000 

typedef enum
{
	MAJOR = 0,
	MINOR = 2
} app_version;

typedef enum
{
	gen_xorpad = 0,
	dec_icondata = 1,
} modes;

typedef enum
{
	CTR = 1,
	SRL,
	BAD_ICON,
} icon_type;

const static u8 SRL_ICON_MAGIC[8] = {0x24, 0xFF, 0xAE, 0x51, 0x69, 0x9A, 0xA2, 0x21};
const static char SMDH_MAGIC[4] = {"SMDH"};

int xor_file(u8 *out, u8 *buff0, u8 *buff1, u32 size);
void app_title(void);
void help(char *app_name);
int CheckDecIcon(u8 *icon);


int main(int argc, char *argv[])
{	
	if(argc < 2){
		printf("[!] Invalid Arguments (Not Enough Arguments)\n");
		help(argv[0]);
		return 1;
	}
	int mode = -1;
	// Deciding Mode of Operation
	for(int i = 1; i < argc; i++){
		if(strcmp(argv[i],"-g") == 0 || strcmp(argv[i],"--genxor") == 0){ 
			mode = gen_xorpad; 
			break;
		}
		else if(strcmp(argv[i],"-d") == 0 || strcmp(argv[i],"--decrypt") == 0){ 
			mode = dec_icondata; 
			break;
		}
	}
	if(mode == -1){
		printf("[!] Invalid Arguments (No Mode Specified)\n");
		help(argv[0]);
		return 1;
	}
	
	// Getting Current Working Directory
	char CWD[1024];
	memset(CWD,0,1024);
	getcwdir(CWD,1024);
	
	// Getting Data from args
	s32 pre_existing_icons = -1;
	s32 icons_to_crypt = -1;
	char *xor_dir = NULL;
	char *dec_icn_dir = NULL;
	char *cacheD_path = NULL;
	char xor_abs_dir[1024];
	memset(xor_abs_dir,0,1024);
	char dec_icn_abs_dir[1024];
	memset(dec_icn_abs_dir,0,1024);
	
	for(int i = 1; i < argc; i++){
		if(strcmp(argv[i],"-i") == 0 && dec_icn_dir == NULL && i < argc-1){ 
			dec_icn_dir = argv[i+1];
		}
		else if(strncmp(argv[i],"--decdata=",10) == 0 && dec_icn_dir == NULL){
			dec_icn_dir = (char*)(argv[i]+10);
		}
		else if(strcmp(argv[i],"-x") == 0 && xor_dir == NULL && i < argc-1){ 
			xor_dir = argv[i+1];
		}
		else if(strncmp(argv[i],"--xorpaddir=",12) == 0 && xor_dir == NULL){
			xor_dir = (char*)(argv[i]+12);
		}
		else if(strcmp(argv[i],"-c") == 0 && cacheD_path == NULL && i < argc-1){ 
			cacheD_path = argv[i+1];
		}
		else if(strncmp(argv[i],"--iconcache=",12) == 0 && cacheD_path == NULL){
			cacheD_path = (char*)(argv[i]+12);
		}
		else if(strcmp(argv[i],"-0") == 0 && pre_existing_icons == -1 && i < argc-1){ 
			pre_existing_icons = strtoul(argv[i+1],NULL,10);
		}
		else if(strncmp(argv[i],"--unused_slots=",15) == 0 && pre_existing_icons == -1){
			pre_existing_icons = strtoul((argv[i]+15),NULL,10);
		}
		else if(strcmp(argv[i],"-1") == 0 && icons_to_crypt == -1 && i < argc-1){ 
			icons_to_crypt = strtoul(argv[i+1],NULL,10);
		}
		else if(strncmp(argv[i],"--num_decrypt=",14) == 0 && icons_to_crypt == -1){
			icons_to_crypt = strtoul((argv[i]+14),NULL,10);
		}
	}
	
	// Sorting out bad input
	if(pre_existing_icons < 0 || pre_existing_icons > 360){
		printf("[!] Invalid input or no input, for option '-0'/'--unused_slots='\n");
		help(argv[0]);
		return 1;
	}
	if(icons_to_crypt < 1 || icons_to_crypt > 360){
		printf("[!] Invalid input or no input, for option '-1'/'--num_decrypt='\n");
		help(argv[0]);
		return 1;
	}
	if(dec_icn_dir == NULL){
		printf("[!] No Plaintext Icon directory was specified\n");
		help(argv[0]);
		return 1;
	}
	if(xor_dir == NULL){
		printf("[!] No XOR pad Directory was specified\n");
		help(argv[0]);
		return 1;
	}
	if(cacheD_path == NULL){
		printf("[!] No Encryption Icon Cache was specified\n");
		help(argv[0]);
		return 1;
	}
	// Opening Icon Cache File
	FILE *cacheD = fopen(cacheD_path,"rb");
	if(cacheD == NULL){
		printf("[!] Failed to open '%s'\n",cacheD_path);
		return 1;
	}
	
	// Allocating Buffers
	u8 *DecIconData = malloc(SMDH_SIZE);
	u8 *EncIconData = malloc(SMDH_SIZE);
	u8 *XORpad = malloc(SMDH_SIZE);
	if(DecIconData == NULL || EncIconData == NULL || XORpad == NULL){
		printf("[!] Memory Error\n");
		return 1;
	}
	
	// Getting Absolute Dir Paths for Icon Dir and XOR pad DIR
	if(mode == gen_xorpad) makedir(xor_dir);
	else if(mode == dec_icondata) makedir(dec_icn_dir);
	
	chdir(xor_dir);
	getcwdir(xor_abs_dir,1024);
	chdir(CWD);
	chdir(dec_icn_dir);
	getcwdir(dec_icn_abs_dir,1024);
	chdir(CWD);
	
	// Informing the user what the program will do
	if(mode == gen_xorpad) printf("[+] Generating XOR pad(s)\n");
	else if(mode == dec_icondata) printf("[+] Decrypting Icon Data\n");
	
	// Init File Pointers / Arrays / Values
	FILE *dec_icon_fp = NULL;
	FILE *xor_pad_fp = NULL;
	char deciconfile[100];
	char xorfile[100];
	u32 enc_icon_pos = 0;
	
	for(int i = pre_existing_icons; i < (pre_existing_icons+icons_to_crypt); i++){
		enc_icon_pos = CACHE_EXTDATA_OFFSET + SMDH_SIZE*i;
		
		// Getting File Names
		memset(&deciconfile,0,100);
		memset(&xorfile,0,100);
		sprintf(deciconfile,"icon_%d.icn",i);
		sprintf(xorfile,"icon_%d.xor",i);
		
		
		// Getting Enc Icon
		ReadFile_64(EncIconData,SMDH_SIZE,enc_icon_pos,cacheD);
		
		// Getting File Pointers and Storing Data
		if(mode == gen_xorpad){
			chdir(dec_icn_abs_dir);
			dec_icon_fp = fopen(deciconfile,"rb");
			if(dec_icon_fp == NULL){
				printf("[!] Failed To Open '%s'\n",deciconfile);
				return 1;
			}
			chdir(xor_abs_dir);
			xor_pad_fp = fopen(xorfile,"wb");
			if(xor_pad_fp == NULL){
				printf("[!] Failed To Create '%s'\n",xorfile);
				return 1;
			}
			
			// Storing Dec Icon
			ReadFile_64(DecIconData,SMDH_SIZE,0,dec_icon_fp);
			int icon_check = CheckDecIcon(DecIconData);
			if(icon_check == BAD_ICON) printf("[!] Caution '%s' does not appear to be an icon file, '%s' may be invalid\n",deciconfile,xorfile);
		}
		else if(mode == dec_icondata){
			chdir(dec_icn_abs_dir);
			dec_icon_fp = fopen(deciconfile,"wb");
			if(dec_icon_fp == NULL){
				printf("[!] Failed To Create '%s'\n",deciconfile);
				return 1;
			}
			chdir(xor_abs_dir);
			xor_pad_fp = fopen(xorfile,"rb");
			if(xor_pad_fp == NULL){
				printf("[!] Failed To Open '%s'\n",xorfile);
				return 1;
			}
			
			// Storing XOR pad
			ReadFile_64(XORpad,SMDH_SIZE,0,xor_pad_fp);
		}
		
		// Perform XOR crypto
		if(mode == gen_xorpad) xor_file(XORpad, DecIconData, EncIconData, SMDH_SIZE);
		else if(mode == dec_icondata){
			xor_file(DecIconData, XORpad, EncIconData, SMDH_SIZE);
			int icon_check = CheckDecIcon(DecIconData);
			if(icon_check == BAD_ICON) printf("[!] '%s' does not appear to have decrypted properly\n",deciconfile);
		}
		
		// Writing Output to File
		if(mode == gen_xorpad) WriteBuffer(XORpad, SMDH_SIZE, 0, xor_pad_fp);
		else if(mode == dec_icondata) WriteBuffer(DecIconData, SMDH_SIZE, 0, dec_icon_fp);
		
		fclose(dec_icon_fp);
		fclose(xor_pad_fp);
	}
	
	// Freeing Buffers and Icon Cache File Pointer
	free(EncIconData);
	free(DecIconData);
	free(XORpad);
	fclose(cacheD);
	
	printf("[*] Done\n");
	return 0;
}

int xor_file(u8 *out, u8 *buff0, u8 *buff1, u32 size)
{
	//memdump(stdout,"In  :",buff0,16);
	//memdump(stdout,"Out :",buff1,16);
	memset(out,0,size);
	for(u32 i = 0; i < size; i++){
		out[i] = (buff0[i] ^ buff1[i]);
	}	
	return 0;
}

int CheckDecIcon(u8 *icon)
{
	u8 *SRL_Check = (icon+0xc0);
	char *SMDH_Check = (char*)(icon);
	
	if(strncmp(SMDH_Check,SMDH_MAGIC,4) == 0) return CTR;
	if(memcmp(SRL_Check,SRL_ICON_MAGIC,8) == 0) return SRL;
	return BAD_ICON;
}

void app_title(void)
{
	printf("CTR_Toolkit - Home Menu Icon Cache Decrypter\n");
	printf("Version %d.%d (C) 3DSGuy 2013\n",MAJOR,MINOR);
	
}

void help(char *app_name)
{
	app_title();
	printf("Usage: %s [options]\n", app_name);
	putchar('\n');
	printf("OPTIONS                 Possible Values       Explanation\n");
	printf(" -i, --decdata=         Directory             Specify Plaintext Icon Data directory\n");
	printf(" -x, --xorpaddir=       Directory             Specify XOR pad directory\n");
	printf(" -c, --iconcache=       File-in               Specify Encrypted Icon Cache\n");
	printf(" -0, --unused_slots=    Decimal Value         Specify Number of icons that exist before the one you want to decrypt\n");
	printf(" -1, --num_decrypt=     Decimal Value         Specify Number of icons to decrypt\n");
	printf(" -g, --genxor                                 Generate XOR pad(s)\n");
	printf(" -d, --decrypt                                Decrypt Icon Data using XOR pad(s).\n");
}