
#include "adfs_private.c"

//TODO: check dirty before changing currentpage
//TODO: check dirty before changing currentpage
//TODO: check dirty before changing currentpage


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
	
	g_blockMapCacheFirstBlock = 0;
	g_blockMapCachePageAddr = GET_BLOCK_META_PAGEADDR(g_blockMapCacheFirstBlock, 0);
	
	//read needed page
	retCode = readRedundantPage(REDUNDANCY_BLOCKMAP, BLOCKMAP_OFFSET, BLOCKMAP_SIZE, 
							g_blockMapCachePageAddr, &g_blockMapCachePage);
	
	if(FS_E_OK != ret)
	{
		LOG(ERR, "FS fsInit: first blockmap page corrupted: %d", retCode);
		
	}

	
	return retCode;
}

ErrCode fsShutdown()
{
	ErrCode ret = FS_E_OK;
	uchar i;
	
	//close all opened files
	for(i=0; i<MAX_POOL_FILES; i++)
	{
		if(g_filePool[i].locked)
		{
			fclose(&(g_filePool[i].file));
		}
	}
	
	//is cache page dirty?
	if(g_blockMapCacheDirty)
	{
		//write back
		ret = writeRedundantPage(REDUNDANCY_BLOCKMAP, BLOCKMAP_OFFSET, BLOCKMAP_SIZE, 
						g_blockMapCachePageAddr, &g_blockMapCachePage);
		
		if(FS_E_OK != ret)
		{
			LOG(ERR, "FS %s: cannot write page [0x%x] corrupted: %d", _FUNCTION_, g_blockMapCachePageAddr, ret);
			continue;
		}
	}
}

ErrCode fclose(FsFile* f)
{
	if(!f)
	{
		return FS_E_BADPARAM;
	}
	
	ErrCode ret = fsync(f);
	
	if(FS_E_OK != ret)
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
		return FS_E_BADPARAM;
	}
	
	ErrCode ret = FS_E_OK;
	
	if(f->dirty)
	{
		//persist pageCache 
		writeCurrentFilePage(f);
		f->dirty = 0;
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
			*err = FS_E_BADPARAM;
			break;
		}
	
		uchar i = 0;
		if(mode & MODE_WRITE)
		{
			for(i=0; i<MAX_POOL_FILES; i++)
			{
				if(g_filePool[i].locked)
				{
					if(!strncmp(filename, g_filePool[i].file.fileEntry.fileName, MAX_FILENAME))
					{
						*err = FS_E_FILEINUSE;
						return NULL;
					}
				}
				else //don't break we want to check all fileslots
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
			*err = FS_E_NOFREESLOTS;
			break;
		}
		
		fileSlot->locked = 1;
		retFile = &(fileSlot->file);
				
		*err = findFile(filename, &(retFile->fileEntry));
		if(*err != FS_E_OK)
		{
			LOG(ERR, "FS findFile failed with [%d]", *err);
			
			if(mode & MODE_CREATE)
			{
				*err = fcreate(filename, fileSlot);
				if(*err != FS_E_OK)
				{
					LOG(ERR, "FS fopen: fcreate failed [%d]", *err);
					retFile = NULL;
					fileSlot->locked = 0;
					break;
				}
			}
			else
			{
				retFile = NULL;
				fileSlot->locked = 0;
				break;
			}
		}
		
		retFile->filePoolSlot = fileSlot;
		retFile->dirty = 0;
		
		
		uint32_t newPosition = 0xFFFFFFFF;
		retFile->filePtr.pos = 0;
		
		//init file pointer to max to determine size
		*err = fSetPtr(retFile, newPosition);
		if(FS_E_OK != *err)
		{
			if(FS_E_EOF == *err)
			{
				*err = FS_E_OK;
				retFile->fileSize = retFile->filePtr.pos;
				LOG(INFO, "FS fOpen: filesize: %d", retFile->fileSize);
			}
			else
			{				
				LOG(ERR, "FS fOpen: set file ptr failed with %d", *err);
				//TODO: exit?
			}
		}

		//if the file is not opened in append mode, reset position to 0
		if(!(mode & MODE_APPEND))
		{
			newPosition = 0;
			
			*err = fSetPtr(retFile, newPosition);
			if(FS_E_OK != *err)
			{
				LOG(ERR, "FS fOpen: set file ptr failed with %d", *err);
				//TODO exit?
			}
		}
	
		//read the current pointer page into the cache
		readCurrentFilePage(retFile, FS_PAGE);

	} while(0);
	
	LOG(DBG, "FS fopen: done");
	
	return retFile;

}

ErrCode formatFS(uchar *formatKey)
{
	ErrCode err = FS_E_OK;
	
	do
	{
		if(!formatKey)
		{
			err = FS_E_BADPARAM;
			break;
		}
		
		if(*formatKey != 'O' ||  *(formatKey+1) != 'K' )
		{
			err = FS_E_FORMAT_NOTAUTH;
			break;
		}
		
		//determine card size, construct first config page
		
		//erase block map, all blocks are unused
		
		//!! must erase all flash to ensure no pages have eraseCycles to big => invalid page ///
		
		
	}while(0);
	
	return err;
}

//reads and advances file pointer
uchar fread(FsFile *f, uchar *dst, uchar size, ErrCode *err)
{
	uchar readBytes = 0;
	unsigned short bytesAvailable;
	
	do
	{
		if(!f || !dst || !err)
		{
			return FS_E_BADPARAM;
		}
		
		do
		{
			//test end_of_file
			if(f->filePtr.curBytePosInPage >= f->filePtr.currentPageData.d.dataBytes)
			{
				*err = FS_E_EOF;
				break;
			}
		
			bytesAvailable = f->filePtr.currentPageData.d.dataBytes - f->filePtr.curBytePosInPage;
			if(bytesAvailable  <= size)
			{
				memcpy(dst + readBytes, f->filePtr.currentPageData.raw + SIZEOF_PAGEDATA + f->filePtr.curBytePosInPage, size);
				readBytes += size;
				
				f->filePtr.pos += size;
				f->filePtr.curBytePosInPage += size;
				size = 0;
				
			}
			else 
			{
				memcpy(dst + readBytes, f->filePtr.currentPageData.raw + SIZEOF_PAGEDATA + f->filePtr.curBytePosInPage, bytesAvailable);
				size -= bytesAvailable;
				readBytes += bytesAvailable;
				
				f->filePtr.pos += bytesAvailable;
				f->filePtr.curBytePosInPage += bytesAvailable;
				
				//change page
				if(f->filePtr.currentPageAddr < PAGES_PER_BLOCK - 1)
				{
					f->filePtr.currentPageAddr++;
					
					err = readCurrentFilePage(f, FS_PAGE);
					if(FS_E_OK != err)
					{
						LOG(ERR, "FS fread: cannot read file page 0x%x: %d", f->filePtr.currentPageAddr, err);
						*err = FS_E_READFAILURE;
						break;
					}
					
					if(f->filePtr.currentPageData.d.fileID != f->fileEntry.fileID)
					{
						//Page does not belong to this file
						f->filePtr.currentPageAddr--;
						*err = FS_E_EOF;
						break;
					}
					f->filePtr.curBytePosInPage = 0;
				}
				else
				{
					if(0xFFFF == f->filePtr.nextBlock)
					{
						LOG(DBG, "FS fread: This was the last block");
						*err = FS_E_EOF;
						break;					
					}
					
					f->filePtr.currentPageAddr = 0;
					f->filePtr.prevBlock = f->filePtr.currentBlock;
					f->filePtr.currentBlock = f->filePtr.nextBlock;
					f->filePtr.curBytePosInPage = 0;
					
					//read first page and determine next block
					err = readCurrentFilePage(f, FS_PAGE);
					if(FS_E_OK != err)
					{
						LOG(ERR, "FS fread: cannot read first page of block 0x%x: %d", f->filePtr.currentBlock, err);
						*err = FS_E_READFAILURE;
						break;
					}
					
					f->filePtr.nextBlock = (FsPageBlockStart)(f->filePtr.currentPageData).d.nextBlock;	
					
					/* fileid not for first page
					if(f->filePtr.currentPageData.d.fileID != f->fileEntry.fileID)
					{
						//Page does not belong to this file
						//f->filePtr.currentPageAddr--;
						
						*err = FS_E_EOF;
						break;
					}
					*/
				}
			}
		}
		while(size >0);
	}
	while(0);

	return readBytes;
}


//writes and advances file pointer; updates filesize
uchar fwrite(FsFile *f, uchar *src, uchar size, ErrCode *err)
{
	uchar writeBytes = 0;
	unsigned short bytesAvailable;
	
	BlockAddr nextFreeBlock;
	
	do
	{
		if(!f || !src || !err)
		{
			return FS_E_BADPARAM;
		}
		
		do
		{
			bytesAvailable = MAX_PAGE_DATABYTES - f->filePtr.curBytePosInPage;
			if(bytesAvailable  <= size)
			{
				memcpy(f->filePtr.currentPageData.raw + SIZEOF_PAGEDATA + f->filePtr.curBytePosInPage, src + writeBytes, size);
				
				writeBytes += size;
				
				if(f->filePtr.currentPageData.d.dataBytes > f->filePtr.curBytePosInPage + size)
				{
					f->fileSize -= (f->filePtr.currentPageData.d.dataBytes - f->filePtr.curBytePosInPage - size);
				}
				else
				{
					f->fileSize += (f->filePtr.curBytePosInPage + size - f->filePtr.currentPageData.d.dataBytes);
				}
				
				f->filePtr.currentPageData.d.dataBytes = f->filePtr.curBytePosInPage + size;
				f->filePtr.pos += size;
				f->filePtr.curBytePosInPage += size;
				
				f->dirty = 1;
				size = 0;
			}
			else 
			{
				memcpy(f->filePtr.currentPageData.raw + SIZEOF_PAGEDATA + f->filePtr.curBytePosInPage, src + writeBytes, bytesAvailable);
				size -= bytesAvailable;
				writeBytes += bytesAvailable;
				
				if(f->filePtr.currentPageData.d.dataBytes > f->filePtr.curBytePosInPage + bytesAvailable)
				{
					f->fileSize -= (f->filePtr.currentPageData.d.dataBytes - f->filePtr.curBytePosInPage - bytesAvailable);
				}
				else
				{
					f->fileSize += (f->filePtr.curBytePosInPage + bytesAvailable - f->filePtr.currentPageData.d.dataBytes);
				}
				
				f->filePtr.currentPageData.d.dataBytes = f->filePtr.curBytePosInPage + size;
				
				
				f->filePtr.pos += bytesAvailable;
				f->filePtr.curBytePosInPage += bytesAvailable;
				
				//current page is full, persist it
				writeCurrentFilePage(f);
				
				//change page
				if(f->filePtr.currentPageAddr < PAGES_PER_BLOCK - 1)
				{
					f->filePtr.currentPageAddr++;
					
					err = readCurrentFilePage(f, FS_PAGE);
					if(FS_E_OK != err)
					{
						LOG(ERR, "FS fread: cannot read file page 0x%x: %d", f->filePtr.currentPageAddr, err);
						*err = FS_E_READFAILURE;
						break;
					}

					if(f->filePtr.currentPageData.d.fileID != f->fileEntry.fileID)
					{
						//Page does not belong to this file, make it belong
						f->filePtr.currentPageData.d.fileID = f->fileEntry.fileID;
						f->filePtr.currentPageData.d.dataBytes = 0;
						f->dirty = 1;
					}
					f->filePtr.curBytePosInPage = 0;
				}
				else
				{
					if(0xFFFF == f->filePtr.nextBlock)
					{
						LOG(DBG, "FS fwrite: Last block reached, searching free block");
						*err = findUnusedBlock(&nextFreeBlock);
			
						if(*err != FS_E_OK)
						{
							LOG(ERR, "FS %s: findUnusedBlock failed with [%d]", _FUNCTION_, *err);
							break;
						}
						
						f->filePtr.currentPageAddr = 0;
						
						//reread first block page to update next block info
						err = readCurrentFilePage(f, FS_PAGE);
						if(FS_E_OK != err)
						{
							LOG(ERR, "FS fread: (1)cannot read file page 0x%x: %d", f->filePtr.currentPageAddr, err);
							*err = FS_E_READFAILURE;
							break;
						}
						f->filePtr.nextBlock = nextFreeBlock;
						(FsPageBlockStart)(f->filePtr.currentPageData).d.nextBlock = nextFreeBlock;
						f->filePtr.currentPageData.d.dataBytes = 0;
						//this overlaps nextblock!! f->filePtr.currentPageData.d.fileID = f->fileEntry.fileID;
						writeCurrentFilePage(f);
						
						markBlockInMap(nextFreeBlock, BLOCK_INUSE);
					}
					
					f->filePtr.currentPageAddr = 0;
					f->filePtr.prevBlock = f->filePtr.currentBlock;
					f->filePtr.currentBlock = f->filePtr.nextBlock;
					f->filePtr.curBytePosInPage = 0;
					
					//read first page and determine next block
					err = readCurrentFilePage(f, FS_PAGE);
					if(FS_E_OK != err)
					{
						LOG(ERR, "FS fread: cannot read first page of block 0x%x: %d", f->filePtr.currentBlock, err);
						*err = FS_E_READFAILURE;
						break;
					}
					
					/* file id not for first page
					//check if page is of the same file
					if(f->filePtr.currentPageData.d.fileID != f->fileEntry.fileID)
					{
						//Page does not belong to this file, make it belong
						f->filePtr.currentPageData.d.fileID = f->fileEntry.fileID;
						f->filePtr.currentPageData.d.dataBytes = 0;
						f->dirty = 1;
					}
					*/
					
					f->filePtr.nextBlock = (FsPageBlockStart)(f->filePtr.currentPageData).d.nextBlock;	
				}
			}
		}
		while(size >0);
	}
	while(0);

	return writeBytes;
}

ErrCode fcreate(const char *filename, FilePoolEntry *fEntry)
{
	ErrCode ret = FS_E_OK;
	BlockAddr nextFreeBlock;
	
	do
	{
		if(!filename || !fEntry)
		{
			ret = FS_E_BADPARAM;
			break;
		}

		//faster
		FsFile *f = &(fEntry->file);
		
		//locate free block & create firstblock page
		ret = findUnusedBlock(&nextFreeBlock);
			
		if(*err != FS_E_OK)
		{
			LOG(ERR, "FS %s: findUnusedBlock failed with [%d]", _FUNCTION_, *err);
			break;
		}		
				
		f->fileSize = 0;
		
		f->filePtr.pos = 0;
		f->filePtr.currentPageAddr = 0;
		f->filePtr.curBytePosInPage = 0;
		f->filePtr.prevBlock = nextFreeBlock;
		f->filePtr.currentBlock = nextFreeBlock;
		f->filePtr.nextBlock = 0xFFFF;
		
		//read first block page 
		err = readCurrentFilePage(f, FS_PAGE);
		if(FS_E_OK != err)
		{
			LOG(ERR, "FS %s (1) fread cannot read first file page: %d", _FUNCTION_, err);
			*err = FS_E_READFAILURE;
			break;
		}
						
		(FsPageBlockStart)(f->filePtr.currentPageData).d.nextBlock = 0xFFFF;
		f->filePtr.currentPageData.d.dataBytes = 0;
		//mark current page as dirty
		f->dirty = 1;
						
		err = markBlockInMap(nextFreeBlock, BLOCK_INUSE);
		if(FS_E_OK != err)
		{
			LOG(ERR, "FS %s markBlockInMap failed with %d", _FUNCTION_, err);
		}
				
		//locate next free filetable entry = startBlock=0xffff
		//locate free and fill filetable entry + copy entry to file
		err = createFileEntry(filename, nextFreeBlock, f);
		if(FS_E_OK != err)
		{
			LOG(ERR, "FS %s createFileEntry failed with %d", _FUNCTION_, err);
		}
	}
	while(0);

	return ret;
}

ErrCode fdelete(FsFile *f)
{
	ErrCode ret = FS_E_OK;
	BlockAddr currentBlock;
	BlockAddr nextBlock;
	
	FsPage page;
	
	do
	{
		if(!f)
		{
			ret = FS_E_BADPARAM;
			break;
		}

		currentBlock = f->fileEntry.firstBlock;
		
		//invalidate fileentry
		ret = invalidateFileEntry(f->fileEntryPageAddr, f->fileEntryPageOffset);
		if(FS_E_OK != ret)
		{
			LOG(ERR, "FS %s invalidateFileEntry failed with %d", _FUNCTION_, err);
		}			
		
		//walk blocks mark unused in temporary cache -> persist to blockmap
		do
		{
			LOG(INF, "FS Erasing block 0xk%", currentBlock);
			
			ret = _readPartOfPage(GET_BLOCK_ADDR( currentBlock ), &page, FS_PAGE);

			if(FS_E_OK != ret)
			{
				LOG(ERR, "FS %s: block 0x%x page 0x0 read fail: %d", _FUNCTION_, currentBlock, _err);
				break;
			}

			nextBlock = ((FsPageBlockStart)page).d.nextBlock;
			
			if(!PAGE_VALID(page))
			{
				LOG(ERR, "FS %s: block 0x%x page 0x0 INVALID", _FUNCTION_, fsFile->filePtr.currentBlock);
				markBlockInMap(fsFile->filePtr.currentBlock, BLOCK_INVALID);
			}
			else
			{
				markBlockInMap(currentBlock, BLOCK_UNUSED);
				
				((FsPageBlockStart)page).d.nextBlock = 0xFFFF;
				*page.d.eraseCycles ++;
				ret = _writePage(GET_BLOCK_ADDR( currentBlock ), &page, FS_PAGE);

				if(FS_E_OK != ret)
				{
					LOG(ERR, "FS %s: block 0x%x page 0x0 write fail: %d", _FUNCTION_, currentBlock, _err);
					break;
				}
			}
			currentBlock = nextBlock;
		}
		while(currentBlock != 0xFFFF);

	}while(0);

	return ret;
}

//set file pointer delta position
ErrCode fSetPtrDelta(FsFile *fsFile, int32_t pos)
{
	uint32_t pos = fsFile->filePtr.pos;
	
	if(deltaPos < 0)
	{
		if(pos < (-1)*deltaPos)
		{
			pos = 0;
		}
		else
		{
			pos += deltaPos;
		}
	}
	else
	{
		pos += deltaPos;
	}
	
	return fSetPtr(fsFile, pos);
}

ErrCode fSetPtr(FsFile *fsFile, uint32_t pos)
{
	if(!fsFile)
	{
		return FS_E_BADPARAM;
	}

	ErrCode _err = FS_E_OK;
	uint32_t bytesLeft;
		
	if( pos < fsFile->filePtr.pos)
	{
		//LOG(DBG, "setFilePtr: ");
		LOG(INFO, "setFilePtr: go backwards");
		fsFile->filePtr.pos = 0;
		fsFile->filePtr.currentBlock = fsFile->fileEntry.firstBlock;
		fsFile->filePtr.currentPageAddr = 0;
		fsFile->filePtr.curBytePosInPage = 0;
		fsFile->filePtr.prevBlock = fsFile->fileEntry.firstBlock;
		
		//read first page and determine next block
		_err = readCurrentFilePageHeader(fsFile);
		if(FS_E_OK != _err)
		{
			return _err;
		}
		
		fsFile->filePtr.nextBlock = (FsPageBlockStart)(fsFile->filePtr.currentPageData).d.nextBlock;
	}
	
	uint32_t dif = pos - fsFile->filePtr.pos;
	LOG(DBG, "FS >> setFilePtr: reqPos=%d curPos=%d dif=%d", pos, fsFile->filePtr.pos, dif);
	
	uchar done = 0;

	do
	{
		bytesLeft = fsFile->filePtr.pos % MAX_BLOCK_DATABYTES;
		if(bytesLeft + dif > MAX_BLOCK_DATABYTES) //dif bigger than block boundary
		{
			//get bytes left in block
			bytesLeft = MAX_BLOCK_DATABYTES - bytesLeft;
			
			//move to next block if possible
			if(0xFFFF == fsFile->filePtr.nextBlock)
			{
				//no next block this was the last one
				//search for end of file in this block; 
				//the error will be eof
				dif = bytesLeft - 1;
				_err = FS_E_EOF;
			}
			else
			{
				dif -= bytesLeft;
				fsFile->filePtr.pos += bytesLeft;
						
				fsFile->filePtr.currentPageAddr = 0;
				fsFile->filePtr.curBytePosInPage = 0;
				fsFile->filePtr.prevBlock = fsFile->filePtr.currentBlock;
				fsFile->filePtr.currentBlock = fsFile->filePtr.nextBlock;
				
				_err = readCurrentFilePageHeader(fsFile);
				if(FS_E_OK != _err)
				{
					_err = FS_E_PAGE_CORRUPTED;
					break;	
				}
				fsFile->filePtr.nextBlock = (FsPageBlockStart)(fsFile->filePtr.currentPageData).d.nextBlock;
			}
		}
		else // dif smaller than block boundary. 
		{
			bytesLeft = fsFile->filePtr.curBytePosInPage;//fsFile->filePtr.pos % MAX_PAGE_DATABYTES;
			
			if(fsFile->filePtr.curBytePosInPage != fsFile->filePtr.pos % MAX_PAGE_DATABYTES)
			{
				LOG(ERR, "FS %s file ptr inconsistency: pos=%d curBytePosInPage=%d page=%d file=%d", 
					_FUNCTION_, fsFile->filePtr.pos, fsFile->filePtr.curBytePosInPage, fsFile->filePtr.currentPageAddr,
					);
			}
			
			if(bytesLeft + dif > MAX_PAGE_DATABYTES) //dif bigger than page boundary
			{
				//get bytes left in page
				bytesLeft = MAX_BLOCK_DATABYTES - bytesLeft;
				
				dif -= bytesLeft;
				fsFile->filePtr.pos += bytesLeft;
				fsFile->filePtr.curBytePosInPage += bytesLeft;
				
				//move to next page if possible
				if(fsFile->filePtr.currentPageAddr + 1 < PAGES_PER_BLOCK)
				{
					fsFile->filePtr.currentPageAddr++;
					
					_err = readCurrentFilePageHeader(fsFile);
					if(FS_E_OK != _err)
					{
						_err = FS_E_PAGE_CORRUPTED;
						break;	
					}
					
					//is next page is from another file ?
					if(fsFile->filePtr.currentPageData.d.fileID != fsFile->fileEntry.fileID)
					{
						fsFile->filePtr.currentPageAddr--;
						
						_err = FS_E_EOF;
						done = 1;		
					}
					else //page changed ok
					{
						fsFile->filePtr.curBytePosInPage = 0;
					}
				}
				else 
				{
					_err = FS_E_EOF;
					done = 1;					
				}
			}
			else //dif smaller than page boundary
			{
				fsFile->filePtr.pos += dif;
				fsFile->filePtr.curBytePosInPage += dif;
				dif = 0;
				done = 1;
			}
		}
	}while(!done);

	LOG(DBG, "FS << setFilePtr: reqPos=%d curPos=%d dif=%d", pos, fsFile->filePtr.pos, dif);
	
	return _err;
}


