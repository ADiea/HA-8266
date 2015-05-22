#ifndef ADFS_H
#define ADFS_H

//EXTERN CONSTANTS
#define MODE_WRITE 		0x1
#define MODE_READ 		0x2
#define MODE_APPEND		0x4
#define MODE_CREATE		0x8


#define FS_E_OK 					0
#define FS_E_BADPARAM 				1
#define FS_E_FILEINUSE 				2
#define FS_E_NOFREESLOTS 			3
#define FS_E_READFAILURE 			4
#define FS_E_PAGE_CORRUPTED 		5 //page or all redundancy levels are corrupted
#define FS_E_NOTFOUND				6 //file was not found
#define FS_E_FSFULL					7
#define FS_E_EOF					8 //end of file reached
#define FS_E_BADFILEBLOCKCHAIN		9 //file chain unexpectedly interrupted
#define FS_E_FORMAT_NOTAUTH			10 //format was not authenticated corectly using OK string

typedef unsigned char ErrCode;

//CONSTANTS
#define MAX_PAGES 65536
#define FS_PAGE   512 //bytes

#define MAX_ERASECYCLES 65500

#define FS_BLOCK  65536 //bytes

#define MAX_FILES 65536
#define MAX_FILENAME 58 //bytes

#define CONFIG_PAGES 1


//ERASE CYCLES
#define SIZEOF_ERASECYCLES 2
#define DATA_PAGE_OFFSET SIZEOF_ERASECYCLES

//BLOCK
#define PAGES_PER_BLOCK (FS_BLOCK / FS_PAGE)


//REDUNDANCY
#define NO_REDUNDANCY 0
#define REDUNDANCY_CONFIG (PAGES_PER_BLOCK / CONFIG_PAGES)
#define REDUNDANCY_BLOCKMAP 4
#define REDUNDANCY_FILETABLE 2

//FILE
#define BLOCK_ADDR_SIZEOF 2
#define FILE_ID_SIZEOF 2
#define FILE_META_SIZEOF 1
#define FILEENTRY_SIZE (MAX_FILENAME + BLOCK_ADDR_SIZEOF + FILE_META_SIZEOF + FILE_ID_SIZEOF)
#define NUM_FILE_ENTRIES_PER_PAGE ((FS_PAGE - SIZEOF_ERASECYCLES) / FILEENTRY_SIZE)

//BLOCKMAP
#define BLOCKENTRY_SIZEOF 1
#define NUM_BLOCK_ENTRIES_PER_PAGE ( (FS_PAGE - SIZEOF_ERASECYCLES) / BLOCKENTRY_SIZEOF)

#define BLOCK_UNUSED 	  0xFF
#define BLOCK_INUSE		  0x00

//FS LAYOUT
#define CONFIG_OFFSET 0
#define CONFIG_SIZE CONFIG_PAGES 
#define CONFIG_TOTAL_SIZE (REDUNDANCY_CONFIG * CONFIG_SIZE)
#define CONFIG_PAGE_ADDR CONFIG_OFFSET

#define BLOCKMAP_OFFSET (CONFIG_OFFSET + CONFIG_TOTAL_SIZE)
#define BLOCKMAP_SIZE	(MAX_PAGES / NUM_BLOCK_ENTRIES_PER_PAGE)
#define BLOCKMAP_TOTAL_SIZE (REDUNDANCY_BLOCKMAP * BLOCKMAP_SIZE)

#define FILETABLE_OFFSET (BLOCKMAP_OFFSET + BLOCKMAP_TOTAL_SIZE)
#define FILETABLE_SIZE (MAX_FILES / NUM_FILE_ENTRIES_PER_PAGE)
#define FILETABLE_TOTAL_SIZE (REDUNDANCY_FILETABLE * FILETABLE_SIZE)

#define FIRST_DATA_BLOCK_ADDR (FILETABLE_OFFSET + FILETABLE_TOTAL_SIZE)
#define NUM_DATA_BLOCKS MAX_FILES

//FILE 
//redIndex = redundancy index
#define GET_FILEENTRY_PAGE_ADDR(numFile, redIndex) (FILETABLE_OFFSET + redIndex*FILETABLE_SIZE + ((numFile / NUM_FILE_ENTRIES_PER_PAGE )))
#define GET_FILEENTRY_BYTE_ADDR(numFile) (DATA_PAGE_OFFSET + (numFile % NUM_FILE_ENTRIES_PER_PAGE)*FILEENTRY_SIZE)

//get block address in pages
#define GET_BLOCK_ADDR(block) (FIRST_DATA_BLOCK_ADDR + PAGES_PER_BLOCK * (block))

//redIndex redundancy index
#define GET_BLOCK_META_PAGEADDR(block, redIndex) (BLOCKMAP_OFFSET + (redIndex)*BLOCKMAP_SIZE + ((block) / NUM_BLOCK_ENTRIES_PER_PAGE))
#define GET_BLOCK_META_PAGEOFFSET(block) (DATA_PAGE_OFFSET + ((block) % NUM_BLOCK_ENTRIES_PER_PAGE))

#define PAGE_VALID(p) ((p).d.eraseCycles < MAX_ERASECYCLES || (p).d.eraseCycles == 0xffff)
#define INVALID_BLOCK 0xFFFF

typedef unsigned short BlockAddr;
typedef unsigned short PageAddrInBlock;
typedef uint32_t PageAddr;

#define SIZEOF_PAGEDATA_CONFIG (2 + 2 + 4 + 2 + 2)
typedef union _fsPageCfg
{
	unsigned char raw[FS_PAGE];
	
	struct 
	{
		unsigned short eraseCycles;
		unsigned char  sig[4]; 		//ADFS
		unsigned short CRC;			//16 bit crc
		unsigned short ver;			//version
		unsigned short numBlocks;	//number of data blocks on disk
		unsigned char data[FS_PAGE - SIZEOF_PAGEDATA_CONFIG];
	} d;
} FsPageConfig;

#define SIZEOF_PAGEDATA (6)

typedef union _fsPageBlock
{
	unsigned char raw[FS_PAGE];
	
	struct 
	{
		unsigned short eraseCycles;
		unsigned short dataBytes;	//file bytes present on current page
		BlockAddr  nextBlock; 		//next block containing file data
		unsigned char data[FS_PAGE - SIZEOF_PAGEDATA];
	} d;
} FsPageBlockStart;


typedef union _fsPage
{
	unsigned char raw[FS_PAGE];
	
	struct 
	{
		unsigned short eraseCycles;	
		unsigned short dataBytes; //file bytes present on current page		
		unsigned short fileID;
		unsigned char data[FS_PAGE - SIZEOF_PAGEDATA];
	} d;
} FsPage;

#define MAX_PAGE_DATABYTES (FS_PAGE - SIZEOF_PAGEDATA)
#define MAX_BLOCK_DATABYTES (MAX_PAGE_DATABYTES * PAGES_PER_BLOCK )

extern FsPageConfig gConfigPage;

//8 fileentries per page
typedef struct _fsFileEntry
{
	unsigned char fileName[MAX_FILENAME];
	BlockAddr	firstBlock;
	unsigned short fileID; //CRC of fileName
	unsigned char fileMeta;
} FileEntry;


typedef struct _fsPointer
{
	uint32_t pos; //position relative to begining of file
	BlockAddr currentBlock;
	PageAddrInBlock currentPageAddr;
	unsigned short curBytePosInPage;//cur byte in page
	BlockAddr prevBlock; //needed to remake chain when current block 
							//is found bad => find new block and copy over
	BlockAddr nextBlock;
	
	FsPage currentPageData;
	
} FsPointer;

typedef struct _fsFile
{
	FileEntry fileEntry;
	FsPointer filePtr;
	FilePoolEntry *filePoolSlot;

	uchar dirty;	
	uint32_t fileSize;

} FsFile;

typedef struct _filePoolEntry
{
	FsFile file;
	unsigned char locked; //in use
} FilePoolEntry;


#define MAX_POOL_FILES 8
extern FilePoolEntry g_filePool[MAX_POOL_FILES];



#endif /*ADFS_H*/