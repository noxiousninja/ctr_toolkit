/**
Copyright 2013 3DSGuy

This file is part of extdata_tool.

extdata_tool is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

extdata_tool is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with extdata_tool. If not, see <http://www.gnu.org/licenses/>.
**/
#include "lib.h"
#include "extdata.h"
#include "vsxe.h"

static VSXE_INTERNAL_CONTEXT vsxe_ctx;

// Private Prototypes
int VSXE_SetupInternalContext(VSXEContext *ctx);
void VSXE_PrintFSInfo(void);
void VSXE_SetupOutputFS(void);
int VSXE_WriteExtdataFiles(void);
int VSXE_ExportExtdataImagetoFile(char *inpath, char *outpath, u8 *ExtdataUniqueID);
void VSXE_ReturnDirPath(u32 file_id, char *path, u8 platform);
void VSXE_ReturnExtdataMountPath(u32 file_id, char *path, u8 platform);
void VSXE_InterpreteFolderTable(void);
void VSXE_InterpreteFileTable(void);

// Code
int IsVSXEFileSystem(u8 *vsxe)
{
	vsxe_header *header = (vsxe_header*)vsxe;
	
	if(u8_to_u32(header->magic,BE) == vsxe_magic && u8_to_u32(header->magic_id,LE) == vsxe_magic_id){
		return True;
	}
	
	return False;
}

int ProcessExtData_FS(VSXEContext *ctx)
{
	memset(&vsxe_ctx,0,sizeof(VSXE_INTERNAL_CONTEXT));
	
	u8 result = VSXE_SetupInternalContext(ctx);
	if(result) return result;
	
	if(ctx->Flags[vsxe_show_fs] == True)
		VSXE_PrintFSInfo();
	
	if(ctx->Flags[vsxe_extract] == True){
		VSXE_SetupOutputFS();
		if(VSXE_WriteExtdataFiles() != 0){
			printf("[!] Failed to Extract ExtData images\n");
			return 1;
		}
	}
	return 0;
}

int VSXE_SetupInternalContext(VSXEContext *ctx)
{
	vsxe_ctx.vsxe = ctx->vsxe;
	vsxe_ctx.input = ctx->input;
	vsxe_ctx.output = ctx->output;
	vsxe_ctx.platform = ctx->platform;
	
	vsxe_ctx.header = (vsxe_header*)vsxe_ctx.vsxe;
	vsxe_ctx.data_table = (vsxe_data_table*)(vsxe_ctx.vsxe + u8_to_u64(vsxe_ctx.header->data_table_offset,LE));
	
	if(u8_to_u32(vsxe_ctx.header->magic,BE) != vsxe_magic || u8_to_u32(vsxe_ctx.header->magic_id,LE) != vsxe_magic_id){
		printf("[!] Is not a VSXE File Table\n");
		return 1;
	}
	
	
	vsxe_ctx.folder_table_offset = u8_to_u64(vsxe_ctx.data_table->folder_table_offset,LE);
	folder_table_header *header = (folder_table_header*)(vsxe_ctx.vsxe + vsxe_ctx.folder_table_offset);
	u64 folderTableMaxSize = (sizeof(folder_table_header) + ((u8_to_u32(header->max_slots,LE) - 2)*sizeof(folder_entry)));
	vsxe_ctx.file_table_offset = align_value(vsxe_ctx.folder_table_offset+folderTableMaxSize,0x1000);
	
	VSXE_InterpreteFolderTable();
	VSXE_InterpreteFileTable();
	
	return 0;
}

void VSXE_PrintFSInfo(void)
{
	printf("Last used Extdata Details:\n");
	printf(" ExtData Image ID:     %08x\n",u8_to_u32(vsxe_ctx.header->last_used_file_extdata_id,LE));
	printf(" ExtData Mount Path:   %s\n",vsxe_ctx.header->last_used_file);
	printf(" ExtData Action:       ");
	switch(u8_to_u32(vsxe_ctx.header->last_used_file_action,LE)){
		case vsxe_fs_deleted: printf("Deleted or Unique Extdata ID was blanked\n"); break;
		case vsxe_fs_modified: printf("Modified\n"); break;
	}
	
	printf("ExtData Image Mount locations\n");
	char *path = malloc(EXTDATA_FS_MAX_PATH_LEN);
	for(u32 i = 2; i < vsxe_ctx.filecount; i++){
		if(u8_to_u32(vsxe_ctx.files[i].unk2,LE)){
			memset(path,0x0,EXTDATA_FS_MAX_PATH_LEN);
			sprintf(path,"/");
			VSXE_ReturnExtdataMountPath(i,path,UNIX);
			printf(" Image %08x is mounted at: '%s'\n",i,path);
		}
		//else
		//	printf(" Image %08x does not exist or isn't mounted\n",i);
	}
	free(path);
	/**
	printf("Data Table Info:\n");
	for(int i = 1; i < 40; i += 4){
		//printf("%03d : %08x %08x %08x %08x\n",i,u8_to_u32(ctx->header.table.unk0[i],LE));
		for(int j = 0; j < 4; j++)
			printf(" %08x",u8_to_u32(ctx->header.table.unk0[i+j],LE));
		printf("\n");
	}
	**/
	
	/**
	for(int i = 2; i < vsxe_ctx.foldercount; i++){
		printf("------------------------------------\n\n");
		printf("Parent Folder Index:   %d\n",u8_to_u32(vsxe_ctx.folders[i].parent_folder_index,LE));
		printf("Folder Name:           %s\n",vsxe_ctx.folders[i].filename);
		printf("Folder Index:          %d\n",u8_to_u32(vsxe_ctx.folders[i].unk0,LE));
		printf("UNK1:                  %d\n",u8_to_u32(vsxe_ctx.folders[i].unk1,LE));
		printf("Last File Index:       %d\n",u8_to_u32(vsxe_ctx.folders[i].unk2,LE));
		printf("UNK2:                  %d\n",u8_to_u32(vsxe_ctx.folders[i].unk3,LE));
		printf("UNK3:                  %d\n",u8_to_u32(vsxe_ctx.folders[i].unk4,LE));
		printf("\n------------------------------------\n");
	}
	**/
	/**
	printf("\nFiles\n");
	for(int i = 2; i < vsxe_ctx.filecount; i++){
		printf("------------------------------------\n\n");
		printf("Parent Folder Index:   %d\n",u8_to_u32(vsxe_ctx.files[i].parent_folder_index,LE));
		printf("File Name:             %s\n",vsxe_ctx.files[i].filename);
		printf("File Index:            %d\n",u8_to_u32(vsxe_ctx.files[i].unk0,LE));
		printf("UNK1:                  %x\n",u8_to_u32(vsxe_ctx.files[i].unk1,LE));
		printf("Block Offset:          %x\n",u8_to_u32(vsxe_ctx.files[i].unk2,LE));
		printf("Unique Extdata ID:     %llx\n",u8_to_u64(vsxe_ctx.files[i].unique_extdata_id,BE));
		printf("UNK2:                  %x\n",u8_to_u32(vsxe_ctx.files[i].unk4,LE));
		printf("UNK3:                  %x\n",u8_to_u32(vsxe_ctx.files[i].unk5,LE));
		printf("\n------------------------------------\n");
	}
	**/
	
}

void VSXE_SetupOutputFS(void)
{
	char *path = malloc(IO_PATH_LEN);
	for(int i = 1; i < vsxe_ctx.foldercount; i++){
		memset(path,0,IO_PATH_LEN);
		sprintf(path,"%s",vsxe_ctx.output);
		VSXE_ReturnDirPath(i,path,vsxe_ctx.platform);
		//printf("%s\n",path);
		chdir(path);
		for(int j = 2; j < vsxe_ctx.foldercount; j++){
			if(u8_to_u32(vsxe_ctx.folders[j].parent_folder_index,LE) == i){
				//printf(" %s\n",vsxe->folders[j].filename);
				makedir(vsxe_ctx.folders[j].filename);
			}
		}
	}
	free(path);
}

int VSXE_WriteExtdataFiles(void)
{
	char *inpath = malloc(IO_PATH_LEN);
	char *outpath = malloc(IO_PATH_LEN);
	for(u32 i = 2; i < vsxe_ctx.filecount; i++){
		if(u8_to_u32(vsxe_ctx.files[i].unk2,LE)){
			memset(inpath,0x0,IO_PATH_LEN);
			memset(outpath,0x0,IO_PATH_LEN);
			sprintf(inpath,"%s%c%08x.dec",vsxe_ctx.input,vsxe_ctx.platform,i);
			sprintf(outpath,"%s%c",vsxe_ctx.output,vsxe_ctx.platform);
			VSXE_ReturnExtdataMountPath(i,outpath,vsxe_ctx.platform);
			FILE *extdata = fopen(inpath,"rb");
			if(extdata == NULL){
				free(inpath);
				free(outpath);
				printf("[!] Failed to open %s\n",inpath);
				return 1;
			}
			FILE *outfile = fopen(outpath,"wb");
			if(outfile == NULL){
				free(inpath);
				free(outpath);
				printf("[!] Failed to create %s\n",outpath);
				return 1;
			}
			fclose(extdata);
			fclose(outfile);		
			if(VSXE_ExportExtdataImagetoFile(inpath,outpath,vsxe_ctx.files[i].unique_extdata_id) != 0){
				printf("[!] Failed to Extract '%s' to '%s'\n",inpath,outpath);
				return 1;
			}
		}
	}
	free(inpath);
	free(outpath);
	return 0;
}

int VSXE_ExportExtdataImagetoFile(char *inpath, char *outpath, u8 *ExtdataUniqueID)
{
	ExtdataContext ext;
	InitaliseExtdataContext(&ext);
	ext.extdata.size = GetFileSize_u64(inpath);
	ext.extdata.buffer = malloc(ext.extdata.size);
	memcpy(ext.VSXE_Extdata_ID,ExtdataUniqueID,8);
	if(ext.extdata.buffer == NULL) return VSXE_MEM_FAIL;
	FILE *infile = fopen(inpath,"rb");
	ReadFile_64(ext.extdata.buffer,ext.extdata.size,0,infile);
	fclose(infile);
	u8 result = GetExtdataContext(&ext);
	if(result)
		return result;
	if(ext.ExtdataType != DataPartition)
		return VSXE_BAD_EXTDATA_TYPE;
	if(ext.Files.Count != 1)
		return VSXE_UNEXPECTED_DATA_IN_EXTDATA;
	if(ext.VSXE_Extdata_ID_Match != True){
		printf("[!] Caution, extdata image '%s' did not have expected identifier\n",inpath);
	}
	FILE *outfile = fopen(outpath,"wb");
	WriteBuffer((ext.extdata.buffer + ext.Files.Data[0].offset),ext.Files.Data[0].size,0,outfile);
	FreeExtdataContext(&ext);
	fclose(outfile);
	return 0;
}

void VSXE_ReturnDirPath(u32 file_id, char *path, u8 platform)
{
	u8 path_part_count = 0;
	u8 present_dir = u8_to_u32(vsxe_ctx.folders[file_id].parent_folder_index,LE);
	while(present_dir > 1){
		path_part_count++;
		present_dir = u8_to_u32(vsxe_ctx.folders[present_dir].parent_folder_index,LE);
	}
	u8 folderlocation[path_part_count];
	present_dir = u8_to_u32(vsxe_ctx.folders[file_id].parent_folder_index,LE);
	folderlocation[path_part_count] = present_dir;
	for(int i = path_part_count - 1; present_dir > 1; i--){
		present_dir = u8_to_u32(vsxe_ctx.folders[present_dir].parent_folder_index,LE);
		folderlocation[i] = present_dir;
	}

	sprintf(path,"%s%c",path,platform);
	
	for(int i = 1; i < path_part_count + 1; i++){
		u8 folder_id = folderlocation[i];	
		sprintf(path,"%s%s%c",path,vsxe_ctx.folders[folder_id].filename,platform);
	}
	sprintf(path,"%s%s",path,vsxe_ctx.folders[file_id].filename);	
}

void VSXE_ReturnExtdataMountPath(u32 file_id, char *path, u8 platform)
{
	u8 path_part_count = 0;
	u8 present_dir = u8_to_u32(vsxe_ctx.files[file_id].parent_folder_index,LE);
	while(present_dir > 1){
		path_part_count++;
		present_dir = u8_to_u32(vsxe_ctx.folders[present_dir].parent_folder_index,LE);
	}
	u8 folderlocation[path_part_count];
	present_dir = u8_to_u32(vsxe_ctx.files[file_id].parent_folder_index,LE);
	folderlocation[path_part_count] = present_dir;
	for(int i = path_part_count - 1; present_dir > 1; i--){
		present_dir = u8_to_u32(vsxe_ctx.folders[present_dir].parent_folder_index,LE);
		folderlocation[i] = present_dir;
	}
	for(int i = 1; i < path_part_count + 1; i++){
		u8 folder_id = folderlocation[i];
		sprintf(path,"%s%s%c",path,vsxe_ctx.folders[folder_id].filename,platform);
	}
	sprintf(path,"%s%s",path,vsxe_ctx.files[file_id].filename);	
}

void VSXE_InterpreteFolderTable(void)
{
	folder_table_header *header = (folder_table_header*)(vsxe_ctx.vsxe + vsxe_ctx.folder_table_offset);
	vsxe_ctx.foldercount = u8_to_u32(header->used_slots,LE); // -1
	vsxe_ctx.folders = (folder_entry*)(vsxe_ctx.vsxe + vsxe_ctx.folder_table_offset + sizeof(folder_table_header) - (2*sizeof(folder_entry)));
	return;
}

void VSXE_InterpreteFileTable(void)
{
	file_table_header *header = (file_table_header*)(vsxe_ctx.vsxe + vsxe_ctx.file_table_offset);
	vsxe_ctx.filecount = u8_to_u32(header->used_slots,LE)+1; // -1
	vsxe_ctx.files = (file_entry*)(vsxe_ctx.vsxe + vsxe_ctx.file_table_offset + sizeof(file_table_header) - (2*sizeof(file_entry)));
	return;
}

/**
void read_vsxe(FILE *file, u32 offset)
{
	VSXE_INTERNAL_CONTEXT ctx;
	ctx.extdata = file;
	ctx.offset = offset;
	
	fseek(ctx.extdata,ctx.offset,SEEK_SET);
	fread(&ctx.header,sizeof(vsxe_header),1,ctx.extdata);
	
	if(VerifyVSXE(&ctx) != 0){
		printf("[!] Is not a VSXE File Table\n");
		return;
	}
	
	ctx.folder_table_offset = 0x1000 + ctx.offset;
	ctx.file_table_offset = 0x2000 + ctx.offset;
	
	VSXE_InterpreteFolderTable(&ctx);
	VSXE_InterpreteFileTable(&ctx);
	
	printf("Last Extdata Mount Details:\n");
	u32 last_extdata_id = u8_to_u32(ctx.header.last_used_file_extdata_id,LE);
	printf(" ExtData Image ID:     %08x\n",last_extdata_id);
	printf(" ExtData Mount Path:   %s\n",ctx.header.last_used_file); 
	
	printf("ExtData Image Mount locations\n");
	char *path = malloc(0x100);
	for(u32 i = 2; i < ctx.filecount; i++){
		memset(path,0x0,0x100);
		sprintf(path,"/");
		VSXE_ReturnExtdataMountPath(&ctx,i,path,UNIX);
		printf(" Image %08x is mounted at: '%s'\n",i,path);
	}
	free(path);
	
	
	free(ctx.folders);
	free(ctx.files);

	fseek(ctx.extdata,ctx.offset+0x1000+(sizeof(folder_entry)*2),SEEK_SET);
	printf("\nFolders\n");
	for(int i = 0; i < ctx.foldercount - 2; i++){
		folder_entry tmp;
		fread(&tmp,sizeof(folder_entry),1,ctx.extdata);
		printf("------------------------------------\n\n");
		printf("Parent Folder Index:   %d\n",u8_to_u32(tmp.parent_folder_index,LE));
		printf("Folder Name:           %s\n",tmp.filename);
		printf("Folder Index:          %d\n",u8_to_u32(tmp.folder_index,LE));
		printf("UNK1:                  %d\n",u8_to_u32(tmp.unk1,LE));
		printf("Last File Index:       %d\n",u8_to_u32(tmp.last_file_index,LE));
		printf("UNK2:                  %d\n",u8_to_u32(tmp.unk2,LE));
		printf("UNK3:                  %d\n",u8_to_u32(tmp.unk3,LE));
		printf("\n------------------------------------\n");
	}
	fseek(ctx.extdata,ctx.offset+0x2000+(sizeof(file_entry)*1),SEEK_SET);
	printf("\nFiles\n");
	for(int i = 0; i < ctx.filecount - 2; i++){
		file_entry tmp;
		fread(&tmp,sizeof(file_entry),1,ctx.extdata);
		printf("------------------------------------\n\n");
		printf("Parent Folder Index:   %d\n",u8_to_u32(tmp.parent_folder_index,LE));
		printf("File Name:             %s\n",tmp.filename);
		printf("File Index:            %d\n",u8_to_u32(tmp.index,LE));
		printf("UNK1:                  %x\n",u8_to_u32(tmp.unk1,LE));
		printf("Block Offset:          %x\n",u8_to_u32(tmp.block_offset,LE));
		printf("File Size:             %llx\n",u8_to_u64(tmp.file_size,LE));
		printf("UNK2:                  %x\n",u8_to_u32(tmp.unk2,LE));
		printf("UNK3:                  %x\n",u8_to_u32(tmp.unk3,LE));
		printf("\n------------------------------------\n");
	}
}
**/
