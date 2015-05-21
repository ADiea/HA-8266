
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
		if(mode & MODE_W)
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
			retFile = NULL;
			fileSlot->locked = 0;
			break;
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
		if(mode & MODE_A)
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
		if(mode & MODE_W)
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
		
		//!! NOT ok to assume all bytes are 0xFF b/c card comes formatted with FAT32 fs ///
		
		
	}while(0);
	
	return err;
}

//reads and advances file pointer
uchar fread(uchar *dest, uchar size, ErrCode *err)
{
	uchar readBytes = 0;

	do
	{
	
		
	
	
	}while(0);


	return readBytes;
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
		//locate next free filetable entry
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

		//
	
	}while(0);

	return ret;
}



//set pointer position
ErrCode fSetPtr(uint32_t newPtr)
{

}

//set file pointer delta position
ErrCode fSetPtrDelta(int32_t newPtr)
{

}
