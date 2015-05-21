#include "adfs.h"

FsPageConfig gConfigPage;

FilePoolEntry* getFreeFileSlot();
ErrCode findFile(const char *fname, FileEntry *fentry);
ErrCode findUnusedBlock(BlockAddr startBlock, BlockAddr* destBlock);

//sectorSize is used together with the redundancylevel to compute the correct address
ErrCode readRedundantPage(uchar maxRedundancyLevel, PageAddr pageOffset, PageAddr sectorSize, PageAddr pageAddr, 
							FsPage *page);

ErrCode updateFileSize(FsFile *fsFile);

ErrCode readCurrentFilePage(FsFile *fsFile, unsigned short pageSize);
ErrCode readCurrentFilePageHeader(FsFile *fsFile);



ErrCode writeCurrentFilePage(FsFile *fsFile);

//TODO move block wise first!
ErrCode updateFileSize(FsFile *fsFile)
{
	ErrCode err = FS_E_OK;

	unsigned short dataBytes;
	
	do
	{
		fsFile->fileSize = 0;
		
		fsFile->filePtr.pos = 0;
		fsFile->filePtr.currentBlock = fsFile->fileEntry.firstBlock;
		fsFile->filePtr.currentPageAddr = 0;
		fsFile->filePtr.curBytePosInPage = 0;
		fsFile->filePtr.prevBlock = fsFile->fileEntry.firstBlock;
		
		//read first page and determine next block
		err = readCurrentFilePageHeader(fsFile);
		if(FS_E_OK != err)
		{
			LOG(ERR, "FS updateFileSize: cannot read first file page: %d", err);
			break;
		}
		
		fsFile->filePtr.nextBlock = (FsPageBlockStart)(fsFile->filePtr.currentPageData).d.nextBlock;		
		
		
		do
		{
			dataBytes = fsFile->filePtr.currentPageData.d.dataBytes;
			fsFile->fileSize += dataBytes;
			fsFile->filePtr.pos += dataBytes;
			fsFile->filePtr.curBytePosInPage = dataBytes;
			
			if(dataBytes < MAX_PAGE_DATABYTES)
			{
				//last page reached
				break;
			}
			
			
			if(fsFile->filePtr.currentPageAddr < PAGES_PER_BLOCK - 1)
			{
				fsFile->filePtr.currentPageAddr++;
				
				err = readCurrentFilePageHeader(fsFile);
				if(FS_E_OK != err)
				{
					LOG(ERR, "FS updateFileSize: cannot read file page 0x%x: %d", fsFile->filePtr.currentPageAddr, err);
				}
				
				if(fsFile->filePtr.currentPageData.d.fileID != fsFile->fileEntry.fileID)
				{
					//last page reached
					break;
				}
				
			}
			else
			{
				if(0xFFFF == fsFile->filePtr.nextBlock)
				{
					LOG(DBG, "FS updateFileSize: This was the last block");
					break;					
				}
				
				fsFile->filePtr.currentPageAddr = 0;
				fsFile->filePtr.prevBlock = fsFile->filePtr.currentBlock;
				fsFile->filePtr.currentBlock = fsFile->filePtr.nextBlock;
				
				//read first page and determine next block
				err = readCurrentFilePageHeader(fsFile);
				if(FS_E_OK != err)
				{
					LOG(ERR, "FS updateFileSize: cannot read first page of block 0x%x: %d", fsFile->filePtr.currentBlock, err);
					break;
				}
		
				fsFile->filePtr.nextBlock = (FsPageBlockStart)(fsFile->filePtr.currentPageData).d.nextBlock;	
			}
			
		}while(1);

	}while(0);
	
	return err;
}

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
			LOG(ERR, "FS readCurrentFilePage: block 0x%x page 0x%x read fail: %d", fsFile->filePtr.currentBlock, fsFile->filePtr.currentPageAddr, _err);
			break;
		}
		
		if(!PAGE_VALID(fsFile->filePtr.currentPageData))
		{
			LOG(ERR, "FS readCurrentFilePage: block 0x%x page 0x%x INVALID", fsFile->filePtr.currentBlock, fsFile->filePtr.currentPageAddr);
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
	if(!fsFile)
	{
		return FS_E_BADPARAM;
	}
	ErrCode _err = FS_E_OK;
	
	do
	{
		fsFile->filePtr.currentPageData.d.eraseCycles ++;
		
		_err = _writePage(GET_BLOCK_ADDR( fsFile->filePtr.currentBlock ) + fsFile->filePtr.currentPageAddr, 
							(FsPage*)(&(fsFile->filePtr.currentPageData)));
							
		if(FS_E_OK != _err)
		{
			LOG(ERR, "FS writeCurrentFilePage: block 0x%x page 0x%x write fail: %d", fsFile->filePtr.currentBlock, fsFile->filePtr.currentPageAddr, _err);
			break;
		}

	} while(0);
	
	return _err;
}

ErrCode findUnusedBlock(BlockAddr startBlock, BlockAddr* destBlock)
{
	if(!destBlock)
	{
		return FS_E_BADPARAM;
	}
	ErrCode ret = FS_E_OK;
	
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
			
			if(FS_E_OK != ret)
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
		ret = FS_E_FSFULL;
	}
	
	return ret;	
}

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
				retCode = FS_E_OK;
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
		return FS_E_BADPARAM;
	}
	
	ErrCode ret = FS_E_OK;
	uchar rLevel = 0;
	uchar readPageOk = 0;
	PageAddr _pageAddr;
	do
	{
		_pageAddr = pageOffset + rLevel*sectorSize + pageAddr;
		ret = _readPage(_pageAddr, page);
		if(FS_E_OK != ret)
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
		ret = FS_E_PAGE_CORRUPTED;
	}
	
	return ret;
}


