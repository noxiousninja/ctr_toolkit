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

int GetExtDataContext(EXTDATA_CONTEXT *ctx, FILE *extdataimg)
{
	memset(ctx,0,sizeof(EXTDATA_CONTEXT));
	
	//Reading DIFF
	fseek(extdataimg, 0x100, SEEK_SET);
	fread(&ctx->header.DIFF,sizeof(DIFF_STRUCT),1,extdataimg);
	if(ctx->header.DIFF.magic_0 == DISA_MAGIC){
		printf("[!] This is a SaveData Image\n");
		return DIFF_CORRUPT;
	}
	else if(ctx->header.DIFF.magic_0 != DIFF_MAGIC_0 || ctx->header.DIFF.magic_1 != DIFF_MAGIC_1){
		printf("[!] DIFF Header Corrupt\n");
		return DIFF_CORRUPT;
	}
	//Reading ExtData AES MAC
	fseek(extdataimg, 0x00, SEEK_SET);
	fread(&ctx->header.AES_MAC,0x10,1,extdataimg);
	
	if(ctx->header.DIFF.primary_partition_offset != 0){
		if(GetExtdataPartitionData(&ctx->partition[primary],ctx->header.DIFF.primary_partition_offset, ctx->header.DIFF.active_table_offset, extdataimg) != 0){
			printf("[!] Primary DIFI Partition Corrupt\n");
			return DIFI_CORRUPT;
		}
	}
	else{
		printf("[!] ExtData Image is Empty\n");
		return DIFI_CORRUPT;
	}
	
	if(ctx->header.DIFF.secondary_partition_offset != 0){
		if(GetExtdataPartitionData(&ctx->partition[secondary],ctx->header.DIFF.secondary_partition_offset, ctx->header.DIFF.active_table_offset, extdataimg) != 0){
			printf("[!] Secondary DIFI Partition Corrupt\n");
			return DIFI_CORRUPT;
		}
	}
	else{
		printf("[+] No Secondary Partition Present\n");
	}
	return 0;
}

int GetExtdataPartitionData(PARTITION_STRUCT *partition, u64 offset, u32 active_table_offset, FILE *extdataimg)
{
	memset(partition,0,sizeof(PARTITION_STRUCT));
	partition->DIFI_offset = offset;
		
	fseek(extdataimg,partition->DIFI_offset,SEEK_SET);
	fread(&partition->DIFI,sizeof(DIFI_STRUCT),1,extdataimg);
	
	//Storing IVFC blob
	fseek(extdataimg,(partition->DIFI_offset + partition->DIFI.ivfc_blob_offset),SEEK_SET);
	fread(&partition->IVFC,sizeof(IVFC_STRUCT),1,extdataimg);
	if(partition->IVFC.magic_0 != IVFC_MAGIC_0 || partition->IVFC.magic_1 != IVFC_MAGIC_1){
		//printf("[!] Primary Partition IVFC Blob Corrupt\n");
		return IVFC_CORRUPT;
	}

	//Storing DPFS blob
	fseek(extdataimg,(partition->DIFI_offset + partition->DIFI.dpfs_blob_offset),SEEK_SET);
	fread(&partition->DPFS,sizeof(DPFS_STRUCT),1,extdataimg);
	if(partition->DPFS.magic_0 != DPFS_MAGIC_0 || partition->DPFS.magic_1 != DPFS_MAGIC_1){
		//printf("[!] Primary Partition DPFS Blob Corrupt\n");
		return DPFS_CORRUPT;
	}
	
	//Storing DIFI hash
	fseek(extdataimg,(partition->DIFI_offset + partition->DIFI.hash_offset),SEEK_SET);
	fread(&partition->DIFI_HASH,sizeof(partition->DIFI_HASH),1,extdataimg);	
	return 0;
}


void print_extdata_header(EXTDATA_HEADER_CONTEXT header)
{
	printf("\n[+] ExtData Image Header\n");
	printf("Magic:                      DIFF\n");
	printf("AES MAC:                    "); u8_hex_print_be(header.AES_MAC, 0x10); putchar('\n');
	printf("Primary Partition Offset:   0x%llx\n",header.DIFF.primary_partition_offset);
	printf("Secondary Partition Offset: 0x%llx\n",header.DIFF.secondary_partition_offset);
	printf("Partition Table Length:     0x%llx\n",header.DIFF.partition_table_length);
	printf("Active Table(FB) Offset:    0x%llx\n",header.DIFF.active_table_offset);
	printf("File Base Size:             0x%llx\n",header.DIFF.file_base_size);
	printf("Active Table Hash:          "); u8_hex_print_be(header.DIFF.active_table_hash, 0x20); putchar('\n');
}

void print_partition_info(PARTITION_STRUCT partition)
{
	switch(partition.DIFI.flags[1]){
		case(0x1): printf("\n[+] Primary Partition\n"); break;
		case(0x0): printf("\n[+] Secondary Partition\n"); break;
	}	
	print_DIFI(partition);
	printf("\nIVFC Blob:\n");
	print_IVFC(partition);
	printf("\nDPFS Blob:\n");
	print_DPFS(partition);
}

void print_DIFI(PARTITION_STRUCT partition)
{
	printf("Header:                     DIFI\n");
	printf("SHA-256 Hash:               ");
	u8_hex_print_be(partition.DIFI_HASH, 0x20);
	putchar('\n');
	printf("IVFC Blob:\n");
	printf(" > Relative Offset:         0x%llx\n",partition.DIFI.ivfc_blob_offset/** + partition.DIFI_offset**/);
	printf(" > Adjusted Offset:         0x%llx\n",partition.DIFI.ivfc_blob_offset + partition.DIFI_offset);
	printf(" > Size:                    0x%llx\n",partition.DIFI.ivfc_blob_size);
	printf("DPFS Blob:\n");
	printf(" > Relative Offset:         0x%llx\n",partition.DIFI.dpfs_blob_offset/** + partition.DIFI_offset**/);
	printf(" > Adjusted Offset:         0x%llx\n",partition.DIFI.dpfs_blob_offset + partition.DIFI_offset);
	printf(" > Size:                    0x%llx\n",partition.DIFI.dpfs_blob_size);
	//printf("Hash Blob:\n");
	//printf(" > Offset:                  0x%llx\n",partition.DIFI.hash_offset + partition.DIFI_offset);
	//printf(" > Size:                    0x%llx\n",partition.DIFI.hash_size);
	printf("Flags:                      0x");
	u8_hex_print_be(partition.DIFI.flags, 0x4);
	putchar('\n');
	if(partition.DIFI.flags[0] == 0x1){
		printf(" > Partition Type:          DATA\n");
		printf(" > Data Partition Offset:   0x%llx\n",partition.DIFI.data_partition_offset);
		printf("   (+ DPFS 'IVFC' Offset)\n");
	}
	else if(partition.DIFI.flags[0] == 0x0 && partition.DIFI.flags[1] == 0x1){
		printf(" > Partition Type:          FS\n");
		printf(" > FS Partition Offset:     0x%llx\n",partition.IVFC.level_4_fs_relative_offset + partition.active_table_offset + partition.DPFS.ivfc_offset);
	}
	else if(partition.DIFI.flags[0] == 0x0 && partition.DIFI.flags[1] == 0x0){
		printf(" > Partition Type:          FS\n");
		printf(" > FS Partition Offset:     0x%llx\n",partition.IVFC.level_4_fs_relative_offset + partition.active_table_offset + partition.DPFS.ivfc_length + partition.DPFS.ivfc_offset);
	}
}

void print_IVFC(PARTITION_STRUCT partition)
{
	//printf("Header:                     IVFC\n");
	printf("Master Hash Size:           0x%llx\n",partition.IVFC.master_hash_size);
	printf("Level 1\n");
	printf(" > Relative Offset:         0x%llx\n",partition.IVFC.level_1_relative_offset);
	//printf(" > Adjusted Offset:         0x%x\n",partition.IVFC.level_1_relative_offset + partition.active_table_offset + partition.DPFS.ivfc_offset);
	printf(" > Hash Data Size:          0x%llx\n",partition.IVFC.level_1_hashdata_size);
	printf(" > Block Size:              0x%x\n",1 << partition.IVFC.level_1_block_size);
	printf("Level 2\n");
	printf(" > Relative Offset:         0x%llx\n",partition.IVFC.level_2_relative_offset);
	//printf(" > Adjusted Offset:         0x%x\n",partition.IVFC.level_2_relative_offset + partition.active_table_offset + partition.DPFS.ivfc_offset);
	printf(" > Hash Data Size:          0x%llx\n",partition.IVFC.level_2_hashdata_size);
	printf(" > Block Size:              0x%x\n",1 << partition.IVFC.level_2_block_size);
	printf("Level 3\n");
	printf(" > Relative Offset:         0x%llx\n",partition.IVFC.level_3_relative_offset);
	//printf(" > Adjusted Offset:         0x%x\n",partition.IVFC.level_3_relative_offset + partition.active_table_offset + partition.DPFS.ivfc_offset);
	printf(" > Hash Data Size:          0x%llx\n",partition.IVFC.level_3_hashdata_size);
	printf(" > Block Size:              0x%x\n",1 << partition.IVFC.level_3_block_size);
	printf("Level 4 (File System)\n");
	printf(" > FS Relative Offset:      0x%llx\n",partition.IVFC.level_4_fs_relative_offset);
	//printf(" > Adjusted Offset:         0x%x\n",partition.IVFC.level_4_fs_relative_offset + partition.active_table_offset + partition.DPFS.ivfc_offset);
	printf(" > FS Size:                 0x%llx\n",partition.IVFC.level_4_fs_size);
	printf(" > FS Block Size:           0x%x\n",1 << partition.IVFC.level_4_fs_block_size);
}

void print_DPFS(PARTITION_STRUCT partition)
{
	//printf("Header:                     DPFS\n");
	printf("Table 1\n");
	printf(" > Relative Offset:         0x%llx\n",partition.DPFS.table_1_offset);
	//printf(" > Adjusted Offset:         0x%x\n",partition.DPFS.table_1_offset + partition.active_table_offset);
	printf(" > Length:                  0x%llx\n",partition.DPFS.table_1_length);
	printf(" > Block Size:              0x%x\n",1 << partition.DPFS.table_1_block_size);
	printf("Table 2\n");
	printf(" > Relative Offset:         0x%llx\n",partition.DPFS.table_2_offset);
	//printf(" > Adjusted Offset:         0x%x\n",partition.DPFS.table_2_offset + partition.active_table_offset);
	printf(" > Length:                  0x%llx\n",partition.DPFS.table_2_length);
	printf(" > Block Size:              0x%x\n",1 << partition.DPFS.table_2_block_size);
	printf("IVFC\n");
	printf(" > Relative Offset:         0x%llx\n",partition.DPFS.ivfc_offset);
	//printf(" > Adjusted Offset:         0x%x\n",partition.DPFS.ivfc_offset + partition.active_table_offset);
	printf(" > Length:                  0x%llx\n",partition.DPFS.ivfc_length);
	printf(" > Block Size:              0x%x\n",1 << partition.DPFS.ivfc_block_size);
}