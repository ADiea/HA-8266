#include "adfs.h"

FsPageConfig gConfigPage;

FilePoolEntry* getFreeFileSlot();
ErrCode findFile(const char *fname, FileEntry *fentry);
ErrCode findUnusedBlock(BlockAddr startBlock, BlockAddr* destBlock);

//sectorSize is used together with the redundancylevel to compute the correct address
ErrCode readRedundantPage(uchar maxRedundancyLevel, PageAddr pageOffset, PageAddr sectorSize, PageAddr pageAddr, 
							FsPage *page);

ErrCode fsInit()
{
	//read first page 
	ErrCode retCode = readRedundantPage(REDUNDANCY_CONFIG, CONFIG_OFFSET, CONFIG_SIZE, CONFIG_PAGE_ADDR, 
										(FsPage*)(&gConfigPage));
	
	if(ERR_OK == retCode)
	{
		LOG(INFO, "FS Init: Version %d Blocks: %d", gConfigPage.d.ver, gConfigPage.d.numBlocks);
	}
	else
	{
		gConfigPage.d.ver = 0;
		gConfigPage.d.numBlocks = 0;
	}
	
	return retCode;
}

ErrCode fclose(FsFile* f)
{
	if(!f)
	{
		return E_BADPARAM;
	}
	
	ErrCode ret = fsync(f);
	
	if(E_OK != ret)
	{
		LOG(ERR, "FS fsync failed with [%d]", ret);
	}
	
	(f->filePoolSlot).locked = 0;
	
	return ret;
}


//TODO: fsync
ErrCode fsync(FsFile* f)
{
	if(!f)
	{
		return E_BADPARAM;
	}
	
	ErrCode ret = E_OK;
	
	if(f->dirty)
	{
		//persist pageCache 
		
	
	}
	
	return ret;
}

FsFile* fopen(const char *filename, const char mode, char *err)
{
	FsFile	*retFile = NULL;
	FilePoolEntry *fileSlot = NULL; 
	
	do
	{
		if(!err)
		{
			break;
		}
		
		if(!filename)
		{
			*err = E_BADPARAM;
			break;
		}
	
		uchar i = 0;
		if(mode & MODE_W)
		{
			for(i=0; i<MAX_POOL_FILES; i++)
			{
				if(g_filePool[i].locked)
				{
					if(!strncmp(filename, g_filePool[i].file.fileEntry.fileName, MAX_FILENAME))
					{
						*err = E_FILEINUSE;
						return NULL;
					}
				}
				else
				{
					fileSlot = &(g_filePool[i]);
				}
			}
		}
		else
		{
			fileSlot = getFreeFileSlot();
		}
		
		
		if(!fileSlot)
		{
			*err = E_NOFREESLOTS;
			break;
		}
		
		fileSlot->locked = 1;
		retFile = &(fileSlot->file);
		
		retFile->filepointer = 0;
		
		*err = findFile(filename, retFile);
		if(*err != E_OK)
		{
			LOG(ERR, "FS findFile failed with [%d]", *err);
			retFile = NULL;
			fileSlot->locked = 0;
			break;
		}
		
		retFile->curBlock = retFile->fileEntry.firstBlock;
		retFile->filePoolSlot = fileSlot;
		retFile->dirty = 0;
		retFile->nextReservedBlock = INVALID_BLOCK;
		
		//if opened in write mode, secure next free block
		if(mode & MODE_W)
		{
			*err = findUnusedBlock(retFile->curBlock, &(retFile->nextReservedBlock));
			
			if(*err != E_OK)
			{
				LOG(ERR, "FS findUnusedBlock failed with [%d]", *err);
			}
		}

	} while(0);
	
	return retFile;

}


//reads and advances file pointer
uchar fread(uchar *dest, uchar size, ErrCode *err)
{
	uchar readBytes = 0;



	return readBytes;
}

//set new read pointer position
ErrCode fSetReadP(uint32_t newPtr)
{

}

//set new write pointer position
ErrCode fSetReadP(uint32_t newPtr)
{

}





ErrCode findUnusedBlock(BlockAddr startBlock, BlockAddr* destBlock)
{
	if(!destBlock)
	{
		return E_BADPARAM;
	}
	ErrCode ret = E_OK;
	
	BlockAddr iBlockNumber = 0;
	BlockAddr curBlock;
	PageAddr neededPageAddr;
	PageAddr curPageAddr = 0;
	
	FsPage page;
	
	uchar foundEmptyBlock = 0;
	uchar i;
	uchar jumpOverBlock;
	
	//get locked blocks by current files
	BlockAddr lockedBlocks[MAX_POOL_FILES];
	for(i=0; i<MAX_POOL_FILES; i++ )
	{
		if(g_filePool[i].locked)
		{
			lockedBlocks[i] = g_filePool[i].file.nextReservedBlock;	
		}
		else
		{
			lockedBlocks[i] = INVALID_BLOCK;	
		}
	}
		
	//go around in a loop starting from startBlock
	for(; iBlockNumber < gConfigPage.d.numBlocks; iBlockNumber++)	
	{
		curBlock = (startBlock + iBlockNumber) % gConfigPage.d.numBlocks;
		
		jumpOverBlock = 0;
		
		for(i=0; i<MAX_POOL_FILES; i++ )
		{
			if(curBlock == lockedBlocks[i])	
			{
				jumpOverBlock = 1;
				break;
			}
		}
		
		if(jumpOverBlock)
		{
			continue;
		}

		neededPageAddr = GET_BLOCK_META_PAGEADDR(curBlock, 0);		
		if(neededPageAddr != curPageAddr)
		{
			//read needed page
			ret = readRedundantPage(REDUNDANCY_BLOCKMAP, BLOCKMAP_OFFSET, BLOCKMAP_SIZE, neededPageAddr, &page);
			
			if(E_OK != ret)
			{
				LOG(ERR, "FS findUnusedBlock: page [0x%x] corrupted: %d", neededPageAddr, ret);
				continue;
			}
			
			curPageAddr = neededPageAddr;
		}
		
		uchar curBlockData = page.raw[GET_BLOCK_META_PAGEOFFSET(curBlock)];
		
		if(BLOCK_UNUSED == curBlockData)
		{
			*destBlock = curBlock;
			foundEmptyBlock = 1;
			break;
		}
	}
	
	if(!foundEmptyBlock)
	{
		ret = E_FSFULL;
	}
	
	return ret;	
}




/* de folosit pt fread
		
		FsPageBlockStart page;
		firstBlockPageAddr = GET_BLOCK_ADDR( (startBlock + iBlockNumber) % gConfigPage.d.numBlocks );
		ret = _readPage(firstBlockPageAddr, (FsPage*)(&page));
		
		if(E_OK != ret)
		{
			//E_READFAILURE;
			LOG(ERR, "FS findUnusedBlock Failed to read page [%x], with %d", firstBlockPageAddr, ret);
			continue;
		}
		
		if(!PAGE_VALID(page))
		{
			LOG(ERR, "FS findUnusedBlock: Page [%x] invalid", firstBlockPageAddr);
			continue;
		}
		
		page.d.
		*/


ErrCode findFile(const char *fname, FileEntry *fentry)
{
	if(!fentry || !fname)
	{
		return E_BADPARAM;
	}
	uint32_t pageAddr = FILETABLE_OFFSET;
	
	ErrCode retCode = E_NOTFOUND;
	
	uint32_t tablePage = 0;
	FsPage page;
	
	//redundancy level
	uchar rLevel = 0;
	uchar foundFile = 0;

	uchar fileIdx;
		
	for(; (tablePage < FILETABLE_SIZE) && (!foundFile); tablePage++)
	{
		rLevel = 0;
		foundFile = 0;
		
		retCode = readRedundantPage(REDUNDANCY_FILETABLE, FILETABLE_OFFSET, FILETABLE_SIZE, tablePage, &page);
		
		if(!retCode)
		{
			LOG(ERR, "FS FTable page [%x] read fail: %d", tablePage, retCode);
			//carry on to next page
			continue;
		}
		
		for(fileIdx = 0; fileIdx < NUM_FILE_ENTRIES_PER_PAGE; fileIdx++)
		{
			if(!strncmp(fname, page.raw[GET_FILEENTRY_BYTE_ADDR(fileIdx)], MAX_FILENAME))
			{
				memcpy(fentry, page.raw[GET_FILEENTRY_BYTE_ADDR(fileIdx)], FILEENTRY_SIZE);
				foundFile = 1;
				retCode = E_OK;
				break;
			}
		}
	}
	return retCode;
}

FilePoolEntry* getFreeFileSlot()
{
	uchar i=0;
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

ErrCode readRedundantPage(uchar maxRedundancyLevel, PageAddr pageOffset, PageAddr sectorSize, 
							PageAddr pageAddr, FsPage *page)
{
	if(!page)
	{
		return E_BADPARAM;
	}
	
	ErrCode ret = E_OK;
	uchar rLevel = 0;
	uchar readPageOk = 0;
	PageAddr _pageAddr;
	do
	{
		_pageAddr = pageOffset + rLevel*sectorSize + pageAddr;
		ret = _readPage(_pageAddr, page);
		if(E_OK != ret)
		{
			LOG(ERR, "Read page [%x] failed with %d", _pageAddr, ret);
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
		ret = E_PAGE_CORRUPTED;
	}
	
	return ret;
}


