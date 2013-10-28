typedef enum
{
	retail = 1,
	dev_internal_SDK,
	dev_external_SDK,
	nand
} ncsd_types;

typedef enum
{
	SDK_1_0_0,
	SDK_5_0_0
} NCCH_Structure;

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

static u8 TitleKeyHash_dev3rd[0x20] = 
{
	0x7D, 0x7C, 0x9A, 0x36, 0xED, 0x39, 
	0x5D, 0xAB, 0xCF, 0xF4, 0x73, 0xC1, 
	0x8C, 0x37, 0x9E, 0x40, 0x1D, 0x1C,
	0x1B, 0x63, 0x2B, 0x1D, 0xE2, 0x11,
	0x42, 0xDA, 0x6C, 0xFD, 0xF5, 0xEF,
	0xFD, 0xE3,
};

static u8 TitleKeyHash_Empty[0x20] = 
{
	0x5A, 0xC6, 0xA5, 0x94, 0x5F, 0x16, 
	0x50, 0x09, 0x11, 0x21, 0x91, 0x29, 
	0x98, 0x4B, 0xA8, 0xB3, 0x87, 0xA0, 
	0x6F, 0x24, 0xFE, 0x38, 0x3C, 0xE4, 
	0xE8, 0x1A, 0x73, 0x29, 0x40, 0x65, 
	0x46, 0x1B,
};

static u8 empty_hash[0x20] = 
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00
};

int NCSDProcess(ROM_CONTEXT *ctx);
int GetNCSDData(ROM_CONTEXT *ctx);
int TrimROM(ROM_CONTEXT *ctx);
int RestoreROM(ROM_CONTEXT *ctx);
int ExtractROMPartitions(ROM_CONTEXT *ctx);
void WriteDummyBytes(FILE *file, u8 dummy_byte, u64 len);
void PrintNCSDHeaderData(NCSD_STRUCT *ctx, NCSD_HEADER *header, CARD_INFO_HEADER *card_info, DEV_CARD_INFO_HEADER *dev_card_info);
void PrintNCSDPartitionData(NCSD_STRUCT *ctx, NCSD_HEADER *header, CARD_INFO_HEADER *card_info, DEV_CARD_INFO_HEADER *dev_card_info);
void GetCHIPFullSize(u64 ROM_CHIP_SIZE, int type);
void GetROMUsedSize(u64 ROM_TRIM_SIZE, int type);
void GetROMImageStatus(u64 ROM_IMAGE_FILE_SIZE, u8 ROM_IMAGE_STATUS, int type);
void GetMin3DSFW(char *FW_STRING, CARD_INFO_HEADER *card_info);