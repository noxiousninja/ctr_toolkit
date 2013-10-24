typedef enum
{
	retail = 1,
	dev_internal_SDK,
	dev_external_SDK,
	nand
} ncsd_types;

// Flag Enums
typedef enum 
{
	MEDIA_6X_SAVE_CRYPTO = 1,
	MEDIA_CARD_DEVICE = 3,
	MEDIA_PLATFORM_INDEX = 4,
	MEDIA_TYPE_INDEX = 5,
	MEDIA_UNIT_SIZE = 6,
	MEDIA_CARD_DEVICE_OLD = 7
}FlagIndex;

typedef enum
{
	CARD_DEVICE_NOR_FLASH = 1,
	CARD_DEVICE_NONE = 2,
	CARD_DEVICE_BT = 3
} _CardDevice;

typedef enum
{
	CTR,
} _PlatformIndex;

typedef enum
{
	INNER_DEVICE,
	CARD1,
	CARD2,
	EXTENDED_DEVICE
} _TypeIndex;

//
typedef enum
{
	EUR_ROM = 0x00017102,
	JPN_ROM = 0x00017202,
	USA_ROM = 0x00017302,
	CHN_ROM = 0x00017402,
	KOR_ROM = 0x00017502,
	TWN_ROM = 0x00017602
} CVER_UID_REGION;

typedef struct
{
	u8 offset[4];
	u8 size[4];
} partition_offsetsize;

typedef struct
{
	u8 magic[4];
	u8 rom_size[4];
	u8 title_id[8];
	u8 partitions_fs_type[8];
	u8 partitions_crypto_type[8];
	partition_offsetsize offsetsize_table[8];
	u8 exheader_hash[0x20];
	u8 additional_header_size[0x4];
	u8 sector_zero_offset[0x4];
	u8 partition_flags[8];
	u8 partition_id_table[8][8];
	u8 reserved[0x30];
} NCSD_HEADER;

typedef struct
{
	u8 magic[4];
	u8 content_size[4];
	u8 title_id[8];
	u8 maker_code[2];
	u8 version[2];
	u8 reserved_0[4];
	u8 program_id[8];
	u8 temp_flag;
	u8 reserved_1[0xF];
	u8 logo_sha_256_hash[0x20];
	u8 product_code[0x10];
	u8 extended_header_sha_256_hash[0x20];
	u8 extended_header_size[4];
	u8 reserved_2[4];
	u8 flags[8];
	u8 plain_region_offset[4];
	u8 plain_region_size[4];
	u8 reserved_3[8];
	u8 exefs_offset[4];
	u8 exefs_size[4];
	u8 exefs_hash_size[4];
	u8 reserved_4[4];
	u8 romfs_offset[4];
	u8 romfs_size[4];
	u8 romfs_hash_size[4];
	u8 reserved_5[4];
	u8 exefs_sha_256_hash[0x20];
	u8 romfs_sha_256_hash[0x20];
} __attribute__((__packed__)) 
NCCH_HEADER;

typedef struct
{
	u8 card_info[8];
	u8 reserved_0[0xf8];
	u8 rom_size_used[8];
	u8 reserved_1[0x18];
	u8 cver_title_id[8];
	u8 cver_title_version[2];
	u8 reserved_2[0xcd6];
	u8 partition_0_title_id[8];
	u8 reserved_3[8];
	u8 initial_data[0x30];
	u8 reserved_4[0xc0];
	NCCH_HEADER partition_0_header;
} CARD_INFO_HEADER;

typedef struct
{
	u8 CardDeviceReserved1[0x200];
	u8 TitleKey[0x10];
	u8 CardDeviceReserved2[0xf0];
} DEV_CARD_INFO_HEADER;

/**
typedef struct
{
	int valid;
	int sig_valid;
	int type;
	u8 signature[0x100];
	u8 ncsd_header_hash[0x20];
	NCSD_HEADER header;
	CARD_INFO_HEADER card_info;
	DEV_CARD_INFO_HEADER dev_card_info;
	
	u64 rom_size;
	u64 used_rom_size;
	PARTITION_DATA partition_data[8];
} NCSD_STRUCT;
**/
/**
typedef struct
{
	int active;
	int sig_valid;
	u8 fs_type;
	u8 crypto_type;
	u32 offset;
	u32 size;
	u64 title_id;
} PARTITION_DATA;

**/

int NCSDProcess(ROM_CONTEXT *ctx);
int GetNCSDData(ROM_CONTEXT *ctx);
int TrimROM(ROM_CONTEXT *ctx);
int RestoreROM(ROM_CONTEXT *ctx);
int ExtractROMPartitions(ROM_CONTEXT *ctx);
void WriteDummyBytes(FILE *file, u8 dummy_byte, u64 len);
void PrintNCSDData(NCSD_STRUCT *ctx, NCSD_HEADER *header, CARD_INFO_HEADER *card_info, DEV_CARD_INFO_HEADER *dev_card_info);
void GetCHIPFullSize(u64 ROM_CHIP_SIZE, int type);
void GetROMUsedSize(u64 ROM_TRIM_SIZE, int type);
void GetROMImageStatus(u64 ROM_IMAGE_FILE_SIZE, u8 ROM_IMAGE_STATUS, int type);
void GetMin3DSFW(char *FW_STRING, CARD_INFO_HEADER *card_info);