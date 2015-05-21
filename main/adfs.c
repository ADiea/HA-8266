
#include "adfs_private.c"

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
		retFile->nextReservedBlock = INVALID_BLOCK;
		
		*err = updateFileSize(retFile);
		if(FS_E_OK != err)
		{
			LOG(ERR, "FS fOpen: updateFileSize failed with %d", *err);
		}
		
		uint32_t newPosition = 0;
		if(mode & MODE_APPEND)
		{
			newPosition = 0xFFFFFFFF;
		}
		
		//init file pointer
		*err = setFilePtr(newPosition, retFile);
		if(FS_E_OK != *err)
		{
			LOG(ERR, "FS fOpen: set file ptr failed with %d", *err);
			//TODO: exit?
		}
		
		//read the current pointer page into the cache
		readCurrentFilePage(retFile, FS_PAGE);
						
		//if opened in write mode, secure next free block
		if(mode & MODE_WRITE)
		{
			*err = findUnusedBlock(retFile->curBlock, &(retFile->nextReservedBlock));
			
			if(*err != FS_E_OK)
			{
				LOG(ERR, "FS fopen: findUnusedBlock failed with [%d]", *err);
			}
		}

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
uchar fread(FsFile *f, uchar *dest, uchar size, ErrCode *err)
{
	uchar readBytes = 0;
	unsigned short bytesAvailable;
	
	do
	{
		if(!f || !dest || !err)
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
				memcpy(dst, f->filePtr.currentPageData.raw + SIZEOF_PAGEDATA, size);
				size = 0;
				readBytes += size;
				
				f->filePtr.pos += size;
				f->filePtr.curBytePosInPage += size;
				
			}
			else 
			{
				memcpy(dst, f->filePtr.currentPageData.raw + SIZEOF_PAGEDATA, bytesAvailable);
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
					
					//read first page and determine next block
					err = readCurrentFilePage(f, FS_PAGE);
					if(FS_E_OK != err)
					{
						LOG(ERR, "FS fread: cannot read first page of block 0x%x: %d", f->filePtr.currentBlock, err);
						*err = FS_E_READFAILURE;
						break;
					}
			
					f->filePtr.curBytePosInPage = 0;
					f->filePtr.nextBlock = (FsPageBlockStart)(f->filePtr.currentPageData).d.nextBlock;	
				}
			}
		}
		while(size >0);
	}
	while(0);

	return readBytes;
}

uchar fwrite(FsFile *f, uchar *source, uchar size, ErrCode *err)
{
	uchar writeBytes = 0;

	do
	{
		if(!f || !source || !err)
		{
			return FS_E_BADPARAM;
		}	
		
	
	
	}while(0);


	return writeBytes;
}


ErrCode fcreate(const char *filename, FilePoolEntry *fEntry)
{
	ErrCode ret = FS_E_OK;
	do
	{
		if(!filename)
		{
			ret = FS_E_BADPARAM;
			break;
		}

		//locate free block
		//locate next free filetable entry = startBlock=0xffff
		//create filetable entry
		//create firstblock page
		//writecurrentpage
	
	}while(0);


	return ret;

}

ErrCode fdelete(FsFile *f)
{
	ErrCode ret = FS_E_OK;
	do
	{
		if(!f)
		{
			ret = FS_E_BADPARAM;
			break;
		}

		//walk blocks mark unused in temporary cache -> persist to blockmap
		//mark startblock 0xffff in fileentry = free fileentry
	
	}while(0);

	return ret;
}

//set pointer position
ErrCode fSetPtr(FsFile *f, uint32_t newPtr)
{

}

//set file pointer delta position
ErrCode fSetPtrDelta(FsFile *f, int32_t newPtr)
{

}
