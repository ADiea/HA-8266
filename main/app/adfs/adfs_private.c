#include "adfs.h"

FsPageConfig gConfigPage;

FsPage g_blockMapCachePage;
BlockAddr g_blockMapCacheFirstBlock;
PageAddr g_blockMapCachePageAddr;
uint8_t g_blockMapCacheDirty;

FilePoolEntry* getFreeFileSlot();
ErrCode findFile(const char *fname, FileEntry *fentry);
ErrCode findUnusedBlock(BlockAddr* freeBlock);
ErrCode markBlockInMap(BlockAddr block, uint8_t blockStatus);

//sectorSize is used together with the redundancylevel to compute the correct address
ErrCode readRedundantPage(uint8_t maxRedundancyLevel, PageAddr pageOffset, PageAddr sectorSize, PageAddr pageAddr, 
							FsPage *page);

//sectorSize is used together with the redundancylevel to compute the correct address
ErrCode writeRedundantPage(uint8_t maxRedundancyLevel, PageAddr pageOffset, PageAddr sectorSize, PageAddr pageAddr, 
							FsPage *page);							
							

ErrCode readCurrentFilePage(FsFile *fsFile, unsigned short pageSize);
ErrCode readCurrentFilePageHeader(FsFile *fsFile);
ErrCode writeCurrentFilePage(FsFile *fsFile);

ErrCode createFileEntry(const char *fname, BlockAddr blockAddr, FsFile *file);
ErrCode invalidateFileEntry(PageAddr fEntryPage, uint16_t fEntryOffset);

ErrCode readCurrentFilePageHeader(FsFile *fsFile)
{
	return readCurrentFilePage(fsFile, SIZEOF_PAGEDATA); //include largest header possible
}

ErrCode readCurrentFilePage(FsFile *fsFile, unsigned short pageSize)
{
	if(!fsFile)
	{
		return FS_E_BADPARAM;
	}
	ErrCode _err = FS_E_OK;
	
	do
	{
		_err = _readPartOfPage(GET_BLOCK_ADDR( fsFile->filePtr.currentBlock ) + fsFile->filePtr.currentPageAddr, 
							(FsPage*)(&(fsFile->filePtr.currentPageData)), pageSize);
		if(FS_E_OK != _err)
		{
			LOG_E( "FS readCurrentFilePage: block 0x%x page 0x%x read fail: %d", fsFile->filePtr.currentBlock, fsFile->filePtr.currentPageAddr, _err);
			break;
		}
		
		if(!PAGE_VALID(fsFile->filePtr.currentPageData))
		{
			LOG_E( "FS readCurrentFilePage: block 0x%x page 0x%x INVALID", fsFile->filePtr.currentBlock, fsFile->filePtr.currentPageAddr);
			markBlockInMap(fsFile->filePtr.currentBlock, BLOCK_INVALID);
			
			if( 0 == fsFile->filePtr.currentPageAddr)
			{
				//TODO Move block, repair file entry in filetable
				//get free block....
				
			}
			else
			{
				//TODO Move block, repair chain from prev block to point to new block
				//get free block....
			}
			break;
		}	
	} while(0);
	
	return _err;
}

ErrCode writeCurrentFilePage(FsFile *fsFile)
{
	ErrCode _err = FS_E_OK;
	
	do
	{
		if(!fsFile)
		{
			_err = FS_E_BADPARAM;
			break;
		}
		
		if(!PAGE_VALID(fsFile->filePtr.currentPageData))
		{
			_err = FS_E_PAGE_CORRUPTED;
			break;
		}
		
		fsFile->filePtr.currentPageData.d.eraseCycles ++;
		
		_err = _writePage(GET_BLOCK_ADDR( fsFile->filePtr.currentBlock ) + fsFile->filePtr.currentPageAddr, 
							(FsPage*)(&(fsFile->filePtr.currentPageData)));
							
		if(FS_E_OK != _err)
		{
			LOG_E( "FS writeCurrentFilePage: block 0x%x page 0x%x write fail: %d", fsFile->filePtr.currentBlock, fsFile->filePtr.currentPageAddr, _err);
			break;
		}
		
		fsFile->dirty = 0;

	} while(0);
	
	return _err;
}

ErrCode markBlockInMap(BlockAddr block, uint8_t blockStatus)
{
	ErrCode ret = FS_E_OK;
	PageAddr neededPageAddr = GET_BLOCK_META_PAGEADDR(block, 0);		
	
	do
	{
		if(neededPageAddr != g_blockMapCachePageAddr)
		{
			//is cache page dirty?
			if(g_blockMapCacheDirty)
			{
				//write back
				ret = writeRedundantPage(REDUNDANCY_BLOCKMAP, BLOCKMAP_OFFSET, BLOCKMAP_SIZE, 
								g_blockMapCachePageAddr, &g_blockMapCachePage);
				
				if(FS_E_OK != ret)
				{
					LOG_E( "FS %s: cannot write page [0x%x] corrupted: %d", _FUNCTION, _g_blockMapCachePageAddr, ret);
					break;
				}
				
				g_blockMapCacheDirty = 0;
			}
		
			g_blockMapCachePageAddr = neededPageAddr;
			g_blockMapCacheFirstBlock = (neededPageAddr - BLOCKMAP_OFFSET) * NUM_BLOCK_ENTRIES_PER_PAGE;
			
			//read needed page
			ret = readRedundantPage(REDUNDANCY_BLOCKMAP, BLOCKMAP_OFFSET, BLOCKMAP_SIZE, 
							g_blockMapCachePageAddr, &g_blockMapCachePage);
			
			if(FS_E_OK != ret)
			{
				LOG_E( "FS %s: page [0x%x] corrupted: %d", _FUNCTION, neededPageAddr, ret);
				break;
			}
		}
		
		g_blockMapCachePage.raw[GET_BLOCK_META_PAGEOFFSET(block)] = blockStatus;
		g_blockMapCacheDirty = 1;
	}
	while(0);
	
	return ret;
}

ErrCode findUnusedBlock(BlockAddr* freeBlock)
{
	if(!freeBlock)
	{
		return FS_E_BADPARAM;
	}
	ErrCode ret = FS_E_OK;
	
	BlockAddr iBlockNumber = 0;
	BlockAddr curBlock;
	PageAddr neededPageAddr;
	
	uint8_t foundEmptyBlock = 0;
	uint8_t i;
	
	FsPage page;
	
	BlockAddr startBlock = g_blockMapCacheFirstBlock;

	//go around in a loop starting from startBlock
	for(; iBlockNumber < gConfigPage.d.numBlocks; iBlockNumber++)	
	{
		curBlock = (startBlock + iBlockNumber) % gConfigPage.d.numBlocks;

		neededPageAddr = GET_BLOCK_META_PAGEADDR(curBlock, 0);		
		if(neededPageAddr != g_blockMapCachePageAddr)
		{
			//is cache page dirty?
			if(g_blockMapCacheDirty)
			{
				//write back
				ret = writeRedundantPage(REDUNDANCY_BLOCKMAP, BLOCKMAP_OFFSET, BLOCKMAP_SIZE, 
								g_blockMapCachePageAddr, &g_blockMapCachePage);
				
				if(FS_E_OK != ret)
				{
					LOG_E( "FS findUnusedBlock: cannot write page [0x%x] corrupted: %d", g_blockMapCachePageAddr, ret);
					continue;
				}
				
				g_blockMapCacheDirty = 0;
			}
		
			g_blockMapCachePageAddr = neededPageAddr;
			g_blockMapCacheFirstBlock = (neededPageAddr - BLOCKMAP_OFFSET) * NUM_BLOCK_ENTRIES_PER_PAGE;
			
			//read needed page
			ret = readRedundantPage(REDUNDANCY_BLOCKMAP, BLOCKMAP_OFFSET, BLOCKMAP_SIZE, 
							g_blockMapCachePageAddr, &g_blockMapCachePage);
			
			if(FS_E_OK != ret)
			{
				LOG_E( "FS findUnusedBlock: page [0x%x] corrupted: %d", neededPageAddr, ret);
				continue;
			}
		}
		
		uint8_t curBlockData = g_blockMapCachePage.raw[GET_BLOCK_META_PAGEOFFSET(curBlock)];
		
		if(BLOCK_UNUSED == curBlockData)
		{
			//determine if block is valid
			//err = readFilePage(*, FS_PAGE);
			
			err = _readPartOfPage(GET_BLOCK_ADDR( curBlock ), &page, FS_PAGE);
			if(FS_E_OK != err)
			{
				LOG_E( "FS %s: block 0x%x first page read fail: %d", _FUNCTION_, curBlock, err);
				break;
			}
			
			if(PAGE_VALID(page))
			{
				*freeBlock = curBlock;
				foundEmptyBlock = 1;
				break;
			}
			else
			{
				LOG(INF, "FS %s: block 0x%x invalid", _FUNCTION_, curBlock);
			}				
		}
	}
	
	if(!foundEmptyBlock)
	{
		ret = FS_E_FSFULL;
	}
	
	return ret;
}

//TODO: populate f->fileEntryPageAddr, f->fileEntryPageOffset
ErrCode createFileEntry(const char *fname, BlockAddr blockAddr, FsFile *file)
{
	if(!fname)
	{
		return FS_E_BADPARAM;
	}
	uint32_t pageAddr = FILETABLE_OFFSET;
	
	ErrCode retCode = FS_E_OK;
	
	uint32_t tablePage = 0;
	FsPage page;

	uint8_t foundFile = 0;
	uint8_t fileIdx;
	uint8_t i;
	uint32_t offset;

	for(; (tablePage < FILETABLE_SIZE) && (!foundFile); tablePage++)
	{
		foundFile = 0;
		
		retCode = readRedundantPage(REDUNDANCY_FILETABLE, FILETABLE_OFFSET, FILETABLE_SIZE, tablePage, &page);
		
		if(FS_E_OK != retCode)
		{
			LOG_E( "FS %s FTable page [%x] read fail: %d", _FUNCTION_, tablePage, retCode);
			//carry on to next page
			continue;
		}
		
		for(fileIdx = 0; fileIdx < NUM_FILE_ENTRIES_PER_PAGE; fileIdx++)
		{
			offset = GET_FILEENTRY_BYTE_ADDR(fileIdx);
			
			if(0xFF == page.raw[offset + MAX_FILENAME] &&
			   0xFF == page.raw[offset + MAX_FILENAME + 1] )
			{ 
				//found empty fileentry, populate it
				i = 0;
				while(*(fname+i) && i<MAX_FILENAME)
				{
					page.raw[offset + i] = *(fname+i);
					i++;
				}
				page.raw[offset + MAX_FILENAME - 1] = 0; //terminate string
				
				memcpy(page.raw[offset + MAX_FILENAME ], (uint8_t*)&blockAddr, 2);
				
				uint16_t fileId = _crc16(fname, strlen(fname));
				
				memcpy(page.raw[offset + MAX_FILENAME + 2], (uint8_t*)&fileId, 2);
				
				//copy resulted fileentry into the file
				memcpy(file->fileEntry, page.raw[GET_FILEENTRY_BYTE_ADDR(fileIdx)], FILEENTRY_SIZE);
				
				retCode = writeRedundantPage(REDUNDANCY_FILETABLE, FILETABLE_OFFSET, FILETABLE_SIZE, tablePage, &page);
				
				if(FS_E_OK != retCode)
				{
					LOG_E( "FS %s FTable page [%x] write fail: %d", _FUNCTION_, tablePage, retCode);
				}
				
				f->fileEntryPageAddr = tablePage;
				f->fileEntryPageOffset = offset;
				
				foundFile = 1;
				break;
			}
		}
	}
	return retCode;
}

ErrCode invalidateFileEntry(PageAddr fEntryPage, uint16_t fEntryOffset)
{
	ErrCode retCode = FS_E_OK;
	
	FsPage page;

	do
	{
		retCode = readRedundantPage(REDUNDANCY_FILETABLE, FILETABLE_OFFSET, FILETABLE_SIZE, fEntryPage, &page);
			
		if(FS_E_OK != retCode)
		{
			LOG_E( "FS %s FTable page [%x] read fail: %d", _FUNCTION_, fEntryPage, retCode);
			break;
		}
			
		page.raw[offset + MAX_FILENAME + 0] = 0xFF;
		page.raw[offset + MAX_FILENAME + 1] = 0xFF;
		
		retCode = writeRedundantPage(REDUNDANCY_FILETABLE, FILETABLE_OFFSET, FILETABLE_SIZE, fEntryPage, &page);				
					
		if(FS_E_OK != retCode)
		{
			LOG_E( "FS %s FTable page [%x] write fail: %d", _FUNCTION_, fEntryPage, retCode);
			break;
		}
	}
	while(0);
	
	return retCode;
}

//TODO populate f->fileEntryPageAddr, f->fileEntryPageOffset; step1: parameter fsfile instead of fentry
ErrCode findFile(const char *fname, FileEntry *fentry)
{
	if(!fentry || !fname)
	{
		return FS_E_BADPARAM;
	}
	uint32_t pageAddr = FILETABLE_OFFSET;
	
	ErrCode retCode = FS_E_NOTFOUND;
	
	uint32_t tablePage = 0;
	FsPage page;
	

	uint8_t foundFile = 0;

	uint8_t fileIdx;
		
	for(; (tablePage < FILETABLE_SIZE) && (!foundFile); tablePage++)
	{
		
		foundFile = 0;
		
		retCode = readRedundantPage(REDUNDANCY_FILETABLE, FILETABLE_OFFSET, FILETABLE_SIZE, tablePage, &page);
		
		if(!retCode)
		{
			LOG_E( "FS FTable page [%x] read fail: %d", tablePage, retCode);
			//carry on to next page
			continue;
		}
		
		for(fileIdx = 0; fileIdx < NUM_FILE_ENTRIES_PER_PAGE; fileIdx++)
		{
			if(!strncmp(fname, page.raw[GET_FILEENTRY_BYTE_ADDR(fileIdx)], MAX_FILENAME))
			{
				memcpy(fentry, page.raw[GET_FILEENTRY_BYTE_ADDR(fileIdx)], FILEENTRY_SIZE);
				foundFile = 1;
				retCode = FS_E_OK;
				break;
			}
		}
	}
	return retCode;
}

FilePoolEntry* getFreeFileSlot()
{
	uint8_t i=0;
	FilePoolEntry* ret = NULL;
	
	for(; i<MAX_POOL_FILES; i++)
	{
		if(!g_filePool[i].locked)
		{
			ret = &(g_filePool[i]);
			break;
		}
	}	
	return ret;
}

ErrCode writeRedundantPage(uint8_t maxRedundancyLevel, PageAddr pageOffset, PageAddr sectorSize, 
							PageAddr pageAddr, FsPage *page)
{
	if(!page)
	{
		return FS_E_BADPARAM;
	}
	
	ErrCode ret = FS_E_OK;
	uint8_t rLevel = 0;
	uint8_t writePageOk = 0;
	PageAddr _pageAddr;
	do
	{
		_pageAddr = pageOffset + rLevel*sectorSize + pageAddr;

		if(!PAGE_VALID(*page))
		{
			continue;
		}
		
		*page.d.eraseCycles ++;
		
		ret = _writePage(_pageAddr, page);
		if(FS_E_OK != ret)
		{
			LOG_E( "Read page [%x] failed with %d", _pageAddr, ret);
			continue;
		}

		rLevel++;
	}
	while(rLevel < maxRedundancyLevel);	
	
	if(writePageOk == 0)
	{
		ret = FS_E_PAGE_CORRUPTED;
	}
	
	return ret;
}

ErrCode readRedundantPage(uint8_t maxRedundancyLevel, PageAddr pageOffset, PageAddr sectorSize, 
							PageAddr pageAddr, FsPage *page)
{
	if(!page)
	{
		return FS_E_BADPARAM;
	}
	
	ErrCode ret = FS_E_OK;
	uint8_t rLevel = 0;
	uint8_t readPageOk = 0;
	PageAddr _pageAddr;
	do
	{
		_pageAddr = pageOffset + rLevel*sectorSize + pageAddr;
		ret = _readPage(_pageAddr, page);
		if(FS_E_OK != ret)
		{
			LOG_E( "Read page [%x] failed with %d", _pageAddr, ret);
			continue;
		}
		
		if(PAGE_VALID(*page))
		{
			readPageOk = 1;
			break;
		}
		
		rLevel++;
	}
	while(rLevel < maxRedundancyLevel);	
	
	if(readPageOk == 0)
	{
		ret = FS_E_PAGE_CORRUPTED;
	}
	
	return ret;
}


