
typedef enum
{
	red_mask = 0xF800,
	green_mask = 0x7E0,
	blue_mask = 0x1F
} RGB565_BITMASK;

typedef struct 
{ 
    u16   bfType; 
    u32  bfSize; 
    u16   bfReserved1; 
    u16   bfReserved2; 
    u32  bfOffBits; 
} _BITMAPFILEHEADER; 

typedef struct
{
    u32  biSize; 
    s32   biWidth; 
    s32   biHeight; 
    u16   biPlanes; 
    u16   biBitCount; 
    u32  biCompression; 
    u32  biSizeImage; 
    s32   biXPelsPerMeter; 
    s32   biYPelsPerMeter; 
    u32  biClrUsed; 
    u32  biClrImportant; 
} _BITMAPINFOHEADER; 

typedef struct
{
    u8  rgbtBlue;
    u8  rgbtGreen;
    u8  rgbtRed;
} _RGBTRIPLE;
