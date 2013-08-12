/**
Copyright 2013 3DSGuy

This file is part of extdata_tool.

extdata_tool is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

extdata_tool is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with extdata_tool.  If not, see <http://www.gnu.org/licenses/>.
**/
#include "lib.h"
#include "ExtData.h"
#include "titledb.h"
#include "vsxe.h"
#include "main.h"

//Version
typedef enum
{
	MAJOR = 1,
	MINOR = 6
} app_version;

void app_title(void);
void help(char *app_name);

int main(int argc, char *argv[])
{
	app_title();
	
	if (argc < 3 || argc > 18){
		printf("\n[!] Must Specify Arguments\n");
		help(argv[0]);
		return 1;
	}

	INPUT_CONTEXT ctx;
	memset(&ctx,0,sizeof(ctx));
	
	for(int i = 1; i < argc - 1; i++){
		if (strcmp(argv[i], "--mode") == 0 || strcmp(argv[i], "-m") == 0){
			if(strcmp(argv[i+1],"IMG") == 0)
				ctx.mode = Image;
			else if(strcmp(argv[i+1],"FS") == 0)
				ctx.mode = Directory;
			else{
				printf("[!] Unrecognised mode: '%s'\n",argv[i+1]);
				return 1;
			}
		}
	}
	if(ctx.mode == 0){
		printf("[!] No mode was specified\n");
		return 1;
	}
	
	if (getcwdir(ctx.cwd,IO_PATH_LEN) == NULL){
		printf("[!] Could not store Current Working Directory\n");
		return IO_FAIL;
	}
	
#ifdef _WIN32
	ctx.platform = WIN_32;
#else
	ctx.platform = UNIX;
#endif

	ctx.extdataimg_path = malloc(IO_PATH_LEN);
	ctx.input = malloc(IO_PATH_LEN);
	memset(ctx.extdataimg_path,0,IO_PATH_LEN);
	memset(ctx.input,0,IO_PATH_LEN);
	switch(ctx.mode){
		case Image:
			memcpy(ctx.input,argv[argc - 1],strlen(argv[argc - 1]));
			memcpy(ctx.extdataimg_path,argv[argc - 1],strlen(argv[argc - 1]));
			break;
		case Directory:
			chdir(argv[argc - 1]);
			getcwdir(ctx.input,IO_PATH_LEN);
			chdir(ctx.cwd);
			sprintf(ctx.extdataimg_path,"%s%c00000001.dec",ctx.input,ctx.platform);
			break;
	}
	ctx.extdataimg = fopen(ctx.extdataimg_path,"rb");
	if(ctx.extdataimg == NULL){
		printf("[!] Failed to Open '%s'\n",ctx.extdataimg_path);
		return IO_FAIL;
	}
	
	for(int i = 1; i < argc - 1; i++){
		if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0){
			help(argv[0]);
			return 1;
		}
		else if (strcmp(argv[i], "--info") == 0 || strcmp(argv[i], "-i") == 0){
			ctx.info = True;
		}
		else if (strcmp(argv[i], "--extract") == 0 || strcmp(argv[i], "-x") == 0){
			ctx.extract = True;
			ctx.output = malloc(IO_PATH_LEN);
			memset(ctx.output,0,IO_PATH_LEN); 
			switch(ctx.mode){
				case(Image): memcpy(ctx.output,argv[i+1],strlen(argv[i+1])); break;
				case(Directory):
					chdir(argv[i+1]);
					getcwdir(ctx.output,IO_PATH_LEN);
					chdir(ctx.cwd);
					break;
			}
		}
		else if (strcmp(argv[i], "--viewFS") == 0 || strcmp(argv[i], "-v") == 0){
			ctx.fs_info = True;
		}
		else if (strcmp(argv[i], "--titledb") == 0 || strcmp(argv[i], "-t") == 0){
			ctx.titledb_read = True;
		}
		else if (strcmp(argv[i], "--listdb") == 0 || strcmp(argv[i], "-l") == 0){
			ctx.listdb = True;
		}
	}
	
	//ctx.extdataimg
	GetExtDataContext(&ctx.data,ctx.extdataimg);
	//Performing Functions Now
	
	if(ctx.info == True){
		print_extdata_header(ctx.data.header);
		print_partition_info(ctx.data.partition[primary]);
		if(ctx.data.header.DIFF.secondary_partition_offset != 0){
			print_partition_info(ctx.data.partition[secondary]);
		}
	}
	
	if(ctx.titledb_read == True || ctx.listdb == True){
		if(ctx.mode != Image){
			printf("[!] Database ExtData images (*.db) do not support being stored in Title ExtData directories\n");
			return 1;
		}
		int db_mode = Normal;
		if(ctx.listdb == True)
			db_mode = ByTID;
		if(ProcessTitleDB(ctx.extdataimg, db_mode,(ctx.data.header.DIFF.active_table_offset + ctx.data.partition[primary].IVFC.level_4_fs_relative_offset + ctx.data.partition[primary].DPFS.ivfc_offset)) != 0){
			if(ctx.data.partition[primary].DIFI.flags[0] == 0x0){
				if(ProcessTitleDB(ctx.extdataimg, db_mode,(ctx.data.header.DIFF.active_table_offset + ctx.data.partition[primary].DPFS.ivfc_length + ctx.data.partition[secondary].DPFS.ivfc_offset + ctx.data.partition[primary].IVFC.level_4_fs_relative_offset)) != 0){
					printf("[!] %s is Corrupt, or is not a Title Database ExtData Image\n",ctx.extdataimg_path);
				}
			}
			else
				printf("[!] %s is Corrupt, or is not a Title Database ExtData Image\n",ctx.extdataimg_path);
		}
	}
	
	if(ctx.extract == True && ctx.mode == Image){
		if(ctx.data.partition[primary].DIFI.flags[0] == 1){//ONE FILE
			u64 offset = ctx.data.partition[primary].DIFI.data_partition_offset + ctx.data.partition[primary].DPFS.ivfc_offset;
			u64 size = ctx.data.partition[primary].IVFC.level_4_fs_size;
			FILE *out = fopen(ctx.output,"wb");
			if(out == NULL){
				printf("[!] Could not create output file\n");
				return IO_ERROR;
			}
			if(ExportFileToFile(ctx.extdataimg,out,size,offset,0) != 0){
				printf("[!] Failed to Extract ExtData\n");
				fclose(out);
				return Fail;
			}
			fclose(out);
		}
		else if(ctx.data.partition[primary].DIFI.flags[0] == 0){//TWO Versions of ONE FILE
			char out_name[IO_PATH_LEN];
			u64 offset[2];
			u64 size[2];
			memset(&offset,0x0,sizeof(offset));
			memset(&size,0x0,sizeof(size));
			//File 0 Details
			offset[0] = ctx.data.header.DIFF.active_table_offset + ctx.data.partition[primary].IVFC.level_4_fs_relative_offset + ctx.data.partition[primary].DPFS.ivfc_offset;
			size[0] = ctx.data.partition[primary].IVFC.level_4_fs_size;
			//File 1 Details
			offset[1] = ctx.data.header.DIFF.active_table_offset + ctx.data.partition[primary].DPFS.ivfc_length + ctx.data.partition[secondary].DPFS.ivfc_offset + ctx.data.partition[primary].IVFC.level_4_fs_relative_offset;
			size[1] = ctx.data.partition[secondary].IVFC.level_4_fs_size;
			for(int i = 0; i < 2; i++){
				memset(&out_name,0,IO_PATH_LEN);
				sprintf(out_name, "%d_%s",i,ctx.output);
				FILE *out = fopen(out_name,"wb");
				if(out == NULL){
					printf("[!] Could not create output file\n");
					return IO_ERROR;
				}
				if(ExportFileToFile(ctx.extdataimg,out,size[i],offset[i],0) != 0){
					printf("[!] Failed to Extract ExtData\n");
					fclose(out);
					return Fail;
				}
				fclose(out);
			}
		}
	}
	
	if((ctx.extract == True || ctx.fs_info == True) && ctx.mode == Directory){
		if(ProcessExtData_FS(&ctx) != 0){
			printf("[!] ExtData FileSystem could not be processed\n");
			return 1;
		}
	}
	
	
	free(ctx.input);
	if(ctx.extract == True){
		free(ctx.output);
	}
	free(ctx.extdataimg_path);
	fclose(ctx.extdataimg);
	printf("[*] Done\n");
	return 0;
}

void app_title(void)
{
	printf("CTR_Toolkit - ExtData Tool\n");
	printf("Version %d.%d (C) 3DSGuy 2013\n",MAJOR,MINOR);
}

void help(char *app_name)
{
	printf("\nUsage: %s [options] <extdata image/directory>\n", app_name);
	putchar('\n');
	printf("OPTIONS                 Possible Values       Explanation\n");
	//printf(" -v, --verbose                                Enable verbose output.\n");
	printf(" -m, --mode             IMG/FS                Specify mode, Single Image or ExtData FS\n");
	printf(" -h, --help                                   Print this help.\n");
  	printf(" -i, --info                                   Display ExtData Info.\n");
	printf(" -x, --extract          Out-dir/Out-file      Extract Data from an ExtData Image or FS.\n");
	printf("ExtData FS Options:\n");
	printf(" -v, --viewFS                                 Display ExtData FS.\n");
	printf("Database ExtData Options:\n");
	printf(" -t, --titledb                                Display Data in Title Database\n");
	printf(" -l, --listdb                                 Generate a Title List from TDB(use with '-t' option)\n");
}
