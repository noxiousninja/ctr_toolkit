typedef enum
{
	NCCH_MAGIC = 0x4E434348,
	NCSD_MAGIC = 0x4E435344
} file_magic;

typedef enum
{
	KB = 1024,
	MB = 1048576,
	GB = 1073741824
} file_size;

typedef enum
{
	_unknown = 0,
	CXI,
	CFA_Manual,
	CFA_DLPChild,
	CFA_Update
} ncch_types;

typedef enum
{
	info = 0,
	restore = 1,
	trim = 2,
	supertrim = 3,
	extract = 4,
} flag_index;

//Variable Structs

typedef struct
{
	int active;
	char product_code[0x10];
	u8 content_type;
	u8 fs_type;
	u8 crypto_type;
	u32 offset;
	u32 size;
	u64 title_id;
} PARTITION_DATA;

typedef struct
{
	int type;
	u8 signature[0x100];
	u64 rom_size;
	u64 used_rom_size;
	u64 actual_rom_file_size;
	PARTITION_DATA partition_data[8];
} NCSD_STRUCT;

typedef struct
{		
	//Input Info
	OPTION_CTX outfile;
	OPTION_CTX romfile;
	
	//NCSD Data
	u8 ncsd_struct_malloc_flag;
	NCSD_STRUCT *ncsd_struct;
	
	//Settings
	u8 flags[5];
} __attribute__((__packed__)) 
ROM_CONTEXT;