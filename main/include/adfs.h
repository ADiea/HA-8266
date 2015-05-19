#ifndef ADFS_H
#define ADFS_H

//EXTERN CONSTANTS
#define MODE_W 0x1
#define MODE_R 0x2


#define E_OK 					0
#define E_BADPARAM 				1
#define E_FILEINUSE 			2
#define E_NOFREESLOTS 			3
#define E_READFAILURE 			4
#define E_PAGE_CORRUPTED 		5 //page or all redundancy levels are corrupted
#define E_NOTFOUND				6 //file was not found
#define E_FSFULL				7
#define E_EOF					8 //end of file reached



typedef unsigned char ErrCode;

//CONSTANTS
#define MAX_PAGES 65536
#define FS_PAGE   512 //bytes

#define MAX_ERASECYCLES 65500

#define FS_BLOCK  65536 //bytes

#define MAX_FILES 65536
#define MAX_FILENAME 60 //bytes

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
#define FILE_META_SIZEOF 1
#define FILEENTRY_SIZE (MAX_FILENAME + BLOCK_ADDR_SIZEOF + FILE_META_SIZEOF)
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

#define SIZEOF_PAGEDATA_BLOCKSTART (2 + 2 + 2 + 2)
typedef union _fsPageBlock
{
	unsigned char raw[FS_PAGE];
	
	struct 
	{
		unsigned short eraseCycles;
		unsigned short dataBytes;	//file bytes present on current page		
		BlockAddr  nextBlock; 		//next block containing file data
		unsigned short CRC;			//(and file table pages)
		unsigned char data[FS_PAGE - SIZEOF_PAGEDATA_BLOCKSTART];
	} d;
} FsPageBlockStart;

#define SIZEOF_PAGEDATA (2)
typedef union _fsPage
{
	unsigned char raw[FS_PAGE];
	
	struct 
	{
		unsigned short eraseCycles;	
		unsigned char data[FS_PAGE - SIZEOF_PAGEDATA];
	} d;
} FsPage;


extern FsPageConfig gConfigPage;

//8 fileentries per page
typedef struct _fsFileEntry
{
	unsigned char fileName[MAX_FILENAME];
	BlockAddr	firstBlock;
	unsigned char fileMeta;
} FileEntry;


typedef struct _fsPointer
{
	uint32_t pos; //position relative to begining of file
	BlockAddr currentBlock;
	PageAddrInBlock currentPage;
	BlockAddr prevPage; //needed to remake chain when current block 
							//is found bad => find new block and copy over
	unsigned short dataBytesCurPage;
	BlockAddr nextBlock;
	
} FsPointer;

typedef struct _fsFile
{
	FileEntry fileEntry;
	
	//filepointer
	uint32_t ptr;
	
	uchar dirty;
	
	BlockAddr nextReservedBlock;

	//read and write pointers
	FsPointer readP;
	FsPointer writeP;
	


	FilePoolEntry *filePoolSlot;
	
	//	
	FsPage pageCache;
	
} FsFile;

typedef struct _filePoolEntry
{
	FsFile file;
	unsigned char locked; //in use
} FilePoolEntry;


#define MAX_POOL_FILES 8
extern FilePoolEntry g_filePool[MAX_POOL_FILES];



#endif /*ADFS_H*/