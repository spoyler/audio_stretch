/*****************************************************************************\
*              efs - General purpose Embedded Filesystem library              *
*          --------------------- -----------------------------------          *
*                                                                             *
* Filename :  ls.c                                                            *
* Description : This file contains functions to list the files in a directory *
*                                                                             *
* This program is free software; you can redistribute it and/or               *
* modify it under the terms of the GNU General Public License                 *
* as published by the Free Software Foundation; version 2                     *
* of the License.                                                             *
                                                                              *
* This program is distributed in the hope that it will be useful,             *
* but WITHOUT ANY WARRANTY; without even the implied warranty of              *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               *
* GNU General Public License for more details.                                *
*                                                                             *
* As a special exception, if other files instantiate templates or             *
* use macros or inline functions from this file, or you compile this          *
* file and link it with other works to produce a work based on this file,     *
* this file does not by itself cause the resulting work to be covered         *
* by the GNU General Public License. However the source code for this         *
* file must still be made available in accordance with section (3) of         *
* the GNU General Public License.                                             *
*                                                                             *
* This exception does not invalidate any other reasons why a work based       *
* on this file might be covered by the GNU General Public License.            *
*                                                                             *
*                                                    (c)2006 Lennart Yseboodt *
*                                                    (c)2006 Michael De Nil   *
\*****************************************************************************/

/*****************************************************************************/
#include "ls.h"
/*****************************************************************************/

extern unsigned char m_env17_;

esint8 ls_openDir(DirList *dlist,FileSystem *fs,eint8* dirname)
{
	FileLocation loc;
	euint32 fc;
        
        dlist->mCount=0;
        dlist->fCount=0;
	dlist->fs=fs;
	
	if(fs_findFile(dlist->fs,dirname,&loc,&fc)!=2)
	{
		return(-1);
	}
	
	fs_initClusterChain(dlist->fs,&(dlist->Cache),fc);
	memClr(&(dlist->currentEntry),sizeof(dlist->currentEntry));
	dlist->rEntry=0;
	dlist->cEntry=0xFFFF;
	
	return(0);
}
/*****************************************************************************/

esint8 ls_getDirEntry(DirList *dlist)
{
	if(dlist->Cache.FirstCluster == 1){
		return(ls_getRootAreaEntry(dlist));
	}else if(dlist->Cache.FirstCluster){
		return(ls_getRealDirEntry(dlist));
	}
	return(-1);
}
/*****************************************************************************/
static unsigned short __n_frag(unsigned char *_frag)
{
unsigned char _fname[15];

  format_file_name(_fname,_frag);
  return _num_frag(_fname);
}



static esint8 _p_ls_getNext(DirList *dlist)
{
unsigned char _fname[15];
unsigned short _n_frag;
unsigned char _res;

        memset(_fname,'\0',15);
//VER4        
        memcpy(_fname,dlist->currentEntry.FileName,LIST_MAXLENFILENAME);
#if 1        
       if(m_env17_ == 0x01) //LKF книги
        do
        {
	  do{
		if(ls_getDirEntry(dlist))return(-1);
		dlist->rEntry++;
	  }while(!ls_isValidFileEntry(&(dlist->currentEntry)));
	  dlist->cEntry++;
        }
//мы не воспроизводим всякую левость           
        while(!((dlist->currentEntry.FileName[8] == 'L' && dlist->currentEntry.FileName[9] == 'K' && dlist->currentEntry.FileName[10] == 'F') || 
               (((dlist->currentEntry.Attribute & 0x10) == 0x10)&&strncmp(dlist->currentEntry.FileName,"SYS",3)!=0)));
       else
#endif         
        do
        {
	  do{
		if(ls_getDirEntry(dlist))return(-1);
		dlist->rEntry++;
	  }while(!ls_isValidFileEntry(&(dlist->currentEntry)));
	  dlist->cEntry++;
        }
//мы не воспроизводим всякую левость  
        while(!((dlist->currentEntry.FileName[8] == 'L' && dlist->currentEntry.FileName[9] == 'K' && dlist->currentEntry.FileName[10] == 'F') || 
            (dlist->currentEntry.FileName[8] == 'M' && dlist->currentEntry.FileName[9] == 'P' && dlist->currentEntry.FileName[10] == '3') || 
              (((dlist->currentEntry.Attribute & 0x10) == 0x10)&&strncmp(dlist->currentEntry.FileName,"SYS",3)!=0)));
        
//проверка на то, что мы перешли на другой entry        
        if(abs(strncmp(dlist->currentEntry.FileName,_fname,strlen(dlist->currentEntry.FileName))))
        {
          dlist->mCount++;
          if((dlist->currentEntry.Attribute & 0x10) == 0x10) //папка
            dlist->fCount++;
        }
        
        if(dlist->mCount == 1)
        {
          if(dlist->currentEntry.FileName[8] == 'L' && dlist->currentEntry.FileName[9] == 'K' && dlist->currentEntry.FileName[10] == 'F')
          {
            if(__n_frag(dlist->currentEntry.FileName) == 1)
               return(0);
            //ищим list фрагмента 0001.lkf
            do{ 
              _res=0x0;
	      do{
              if(ls_getDirEntry(dlist)){ _res=0x01; break;}
		dlist->rEntry++;
	      }while(!ls_isValidFileEntry(&(dlist->currentEntry)));
	      dlist->cEntry++;
              if(_res)
                break;
            }
            while(1 != __n_frag(dlist->currentEntry.FileName));
            
          }
          
        }
        
	return(0);
}



/*****************************************************************************/
//экспериментально, пытаюсь взять пред. file по отношению к текущему
static esint8 _p_ls_getPrev(DirList *dlist)
{
unsigned char _fname[15];
unsigned short _n_frag;
unsigned char _res;
unsigned char _prev_file=0x0;

        memset(_fname,'\0',15);
//VER4        
        memcpy(_fname,dlist->currentEntry.FileName,LIST_MAXLENFILENAME);
        if((dlist->currentEntry.Attribute & 0x10) == 0x0) //файл
          _prev_file=0x01;
        if((dlist->currentEntry.Attribute & 0x10) == 0x10) //папка
          _prev_file=0x02;
#if 1        
       if(m_env17_ == 0x01) //LKF книги
        do
        {
	  do{
            dlist->rEntry--;
            if(ls_getDirEntry(dlist))return(-1);		
          }while(!ls_isValidFileEntry(&(dlist->currentEntry)));
	dlist->cEntry--;
        }
//мы не воспроизводим всякую левость        
        while(!((dlist->currentEntry.FileName[8] == 'L' && dlist->currentEntry.FileName[9] == 'K' && dlist->currentEntry.FileName[10] == 'F') || //
           (((dlist->currentEntry.Attribute & 0x10) == 0x10)&&strncmp(dlist->currentEntry.FileName,"SYS",3)!=0)));
       else
#endif         
        do
        {
	  do{
            dlist->rEntry--;
            if(ls_getDirEntry(dlist))return(-1);		
          }while(!ls_isValidFileEntry(&(dlist->currentEntry)));
	dlist->cEntry--;
        }
//мы не воспроизводим всякую левость        
        while(!((dlist->currentEntry.FileName[8] == 'L' && dlist->currentEntry.FileName[9] == 'K' && dlist->currentEntry.FileName[10] == 'F') || //
           (dlist->currentEntry.FileName[8] == 'M' && dlist->currentEntry.FileName[9] == 'P' && dlist->currentEntry.FileName[10] == '3') || //
            (((dlist->currentEntry.Attribute & 0x10) == 0x10)&&strncmp(dlist->currentEntry.FileName,"SYS",3)!=0)));
        
        
//проверка на то, что мы перешли на другой entry        
        if(abs(strncmp(dlist->currentEntry.FileName,_fname,strlen(dlist->currentEntry.FileName))))
        {
            if(dlist->mCount > 0)
              dlist->mCount--;
            if(dlist->fCount > 0)
            {
              if((dlist->currentEntry.Attribute & 0x10) == 0x10 && (_prev_file==0x0)) //папка
                dlist->fCount--;            
              if((dlist->currentEntry.Attribute & 0x10) == 0x0 && (_prev_file==0x02)) //папка
                dlist->fCount--;            
              if((dlist->currentEntry.Attribute & 0x10) == 0x10 && (_prev_file==0x02)) //папка
                dlist->fCount--;            
            }
        }
        
        if(dlist->mCount != 0)
        {
          if(dlist->currentEntry.FileName[8] == 'L' && dlist->currentEntry.FileName[9] == 'K' && dlist->currentEntry.FileName[10] == 'F')
          {
            if(__n_frag(dlist->currentEntry.FileName) == dlist->mCount)
               return(0);
            //ищим list фрагмента dlist->mCount.lkf
            do{ 
            _res=0x0;
	      do{
              dlist->rEntry--;
              if(ls_getDirEntry(dlist)){ _res=0x01; break;}		
              }while(!ls_isValidFileEntry(&(dlist->currentEntry)));
	      dlist->cEntry--;
              if(_res)
                break;
            }
            while(dlist->mCount != __n_frag(dlist->currentEntry.FileName));
            
          }
          
        }
        
        
	return(0);
}


esint8 ls_getPrev(DirList *dlist)
{
unsigned char _fname[15];
unsigned short _n_frag;
unsigned char _res;
unsigned short _mCount;


        
        if(dlist->currentEntry.FileName[8] == 'L' && dlist->currentEntry.FileName[9] == 'K' && dlist->currentEntry.FileName[10] == 'F')
        {
          _mCount = dlist->mCount;
          _n_frag = __n_frag(dlist->currentEntry.FileName); //номер след фраг
          do{ //ищим пред фрагмент назад
__3456_0987___:            
            _res=0x0;
	    do{
              dlist->rEntry--;
              if(ls_getDirEntry(dlist)){ _res=0x01; break;}		
            }while(!ls_isValidFileEntry(&(dlist->currentEntry)));
	    dlist->cEntry--;
//мы не воспроизводим всякую левость
#if 1            
           if(m_env17_ == 0x01) //LKF книги
            if(!((dlist->currentEntry.FileName[8] == 'L' && dlist->currentEntry.FileName[9] == 'K' && dlist->currentEntry.FileName[10] == 'F') || //
            (((dlist->currentEntry.Attribute & 0x10) == 0x10)&&strncmp(dlist->currentEntry.FileName,"SYS",3)!=0)))
              if(_res!=0x01)
                goto __3456_0987___;
             
           else
#endif             
            if(!((dlist->currentEntry.FileName[8] == 'L' && dlist->currentEntry.FileName[9] == 'K' && dlist->currentEntry.FileName[10] == 'F') || //
            (dlist->currentEntry.FileName[8] == 'M' && dlist->currentEntry.FileName[9] == 'P' && dlist->currentEntry.FileName[10] == '3') || //
            (((dlist->currentEntry.Attribute & 0x10) == 0x10)&&strncmp(dlist->currentEntry.FileName,"SYS",3)!=0)))
              if(_res!=0x01)
                goto __3456_0987___;
            
            if(_res)
              break;
          }
          while((_n_frag-1) != __n_frag(dlist->currentEntry.FileName));
          if(!_res)
          {
            dlist->mCount = _mCount-1;
           return(0); //нашли след фраг 
          }
          else
          {
            dlist->rEntry++;
            do{ //ищим пред фрагмент вперед
              _res=0x0;
              if(_p_ls_getNext(dlist)!=0)
                break;
              _res=0x01;
            }
            while((_n_frag-1) != __n_frag(dlist->currentEntry.FileName));
            dlist->mCount=_mCount;
            if(!_res)
              return(-1); //первый фраг был
            
            dlist->mCount = _mCount-1;
            return (0); //нашли пред фраг
          }
          

          
        }
        else
          return _p_ls_getPrev(dlist); //у нас не LKF работаем по старому алг
        
        
        
	return(0);
}


esint8 ls_getNext(DirList *dlist)
{
unsigned char _fname[15];
unsigned short _n_frag;
unsigned char _res;
unsigned short _mCount;

        if(dlist->currentEntry.FileName[8] == 'L' && dlist->currentEntry.FileName[9] == 'K' && dlist->currentEntry.FileName[10] == 'F')
        {
          _mCount = dlist->mCount;
          _n_frag = __n_frag(dlist->currentEntry.FileName); //номер след фраг
          do{ //ищим след фрагмент вперед
__3456_09876___:            
            _res=0x0;
	    do{
              if(ls_getDirEntry(dlist)){ _res=0x01; break;}
		dlist->rEntry++;
	    }while(!ls_isValidFileEntry(&(dlist->currentEntry)));
	    dlist->cEntry++;
//мы не воспроизводим всякую левость
#if 1            
           if(m_env17_ == 0x01) //LKF книги
            if(!((dlist->currentEntry.FileName[8] == 'L' && dlist->currentEntry.FileName[9] == 'K' && dlist->currentEntry.FileName[10] == 'F') || //
            (((dlist->currentEntry.Attribute & 0x10) == 0x10)&&strncmp(dlist->currentEntry.FileName,"SYS",3)!=0)))
              if(_res!=0x01)
                goto __3456_09876___;
             
           else
#endif             
            if(!((dlist->currentEntry.FileName[8] == 'L' && dlist->currentEntry.FileName[9] == 'K' && dlist->currentEntry.FileName[10] == 'F') || //
            (dlist->currentEntry.FileName[8] == 'M' && dlist->currentEntry.FileName[9] == 'P' && dlist->currentEntry.FileName[10] == '3') || //
            (((dlist->currentEntry.Attribute & 0x10) == 0x10)&&strncmp(dlist->currentEntry.FileName,"SYS",3)!=0)))
              if(_res!=0x01)
                goto __3456_09876___;
            
            if(_res)
              break;
          }
          while((_n_frag+1) != __n_frag(dlist->currentEntry.FileName));
          if(!_res)
          {
            dlist->mCount = _mCount+1;
           return(0); //нашли след фраг 
          }
          else
          {
            
            do{ //ищим след фрагмент назад
              _res=0x0;
              dlist->mCount=0;
              if(_p_ls_getPrev(dlist)!=0)
                break;
              _res=0x01;
            }
            while((_n_frag+1) != __n_frag(dlist->currentEntry.FileName));
            dlist->mCount=_mCount;
            if(!_res)
            {
              dlist->rEntry++;
              do{ //бежим в конец
__34222_0987___:                
                _res=0x0;
                do{
                if(ls_getDirEntry(dlist)){ _res=0x01; break;}
		  dlist->rEntry++;
	        }while(!ls_isValidFileEntry(&(dlist->currentEntry)));
	        dlist->cEntry++;
//мы не воспроизводим всякую левость
#if 1                
               if(m_env17_ == 0x01) //LKF книги
                if(!((dlist->currentEntry.FileName[8] == 'L' && dlist->currentEntry.FileName[9] == 'K' && dlist->currentEntry.FileName[10] == 'F') || //
                (((dlist->currentEntry.Attribute & 0x10) == 0x10)&&strncmp(dlist->currentEntry.FileName,"SYS",3)!=0)))
                 if(_res!=0x01)
                  goto __34222_0987___;
                 
               else
#endif                 
                if(!((dlist->currentEntry.FileName[8] == 'L' && dlist->currentEntry.FileName[9] == 'K' && dlist->currentEntry.FileName[10] == 'F') || //
                (dlist->currentEntry.FileName[8] == 'M' && dlist->currentEntry.FileName[9] == 'P' && dlist->currentEntry.FileName[10] == '3') || //
                (((dlist->currentEntry.Attribute & 0x10) == 0x10)&&strncmp(dlist->currentEntry.FileName,"SYS",3)!=0)))
                 if(_res!=0x01)
                  goto __34222_0987___;
                
                if(_res)
                  break;
              }
              while(1);
              dlist->mCount = _mCount+1;
              dlist->currentEntry.FileName[8]='\0'; //мы в конце папки, не надо детектить левый файл
              return(-1); //послед фраг был
            }
            
            dlist->mCount = _mCount+1;
            return (0); //нашли след фраг
          }
          

          
        }
        else
          return _p_ls_getNext(dlist); //у нас не LKF работаем по старому алг
}
/*****************************************************************************/

esint8 ls_getRealDirEntry(DirList *dlist)
{
	euint8* buf;
	
	if(dlist->Cache.FirstCluster<=1)return(-1);
	
	if(fat_LogicToDiscCluster(dlist->fs,
						   &(dlist->Cache),
						   (dlist->rEntry)/(16 * dlist->fs->volumeId.SectorsPerCluster))){
		return(-1);
	}
	
	buf = part_getSect(dlist->fs->part,
					   fs_clusterToSector(dlist->fs,dlist->Cache.DiscCluster) + (dlist->rEntry/16)%dlist->fs->volumeId.SectorsPerCluster,
				       IOM_MODE_READONLY);
	
	/*memCpy(buf+(dlist->rEntry%16)*32,&(dlist->currentEntry),32);*/
	ls_fileEntryToDirListEntry(dlist,buf,32*(dlist->rEntry%16));
	
	part_relSect(dlist->fs->part,buf);
	
	return(0);
}
/*****************************************************************************/

esint8 ls_getRootAreaEntry(DirList *dlist)
{
	euint8 *buf=0;
	
	if((dlist->fs->type != FAT12) && (dlist->fs->type != FAT16))return(-1);
	if(dlist->rEntry>=dlist->fs->volumeId.RootEntryCount)return(-1);
	
	buf = part_getSect(dlist->fs->part,
					   dlist->fs->FirstSectorRootDir+dlist->rEntry/16,
					   IOM_MODE_READONLY);
	/*memCpy(buf+32*(dlist->rEntry%16),&(dlist->currentEntry),32);*/
	ls_fileEntryToDirListEntry(dlist,buf,32*(dlist->rEntry%16));
	part_relSect(dlist->fs->part,buf);
	return(0);
}
/*****************************************************************************/

esint8 ls_isValidFileEntry(ListDirEntry *entry)
{
	if(entry->FileName[0] == 0 || entry->FileName[0] == 0xE5 || entry->FileName[0] == '.')return(0);
	if((entry->Attribute&0x0F)==0x0F)return(0);
 	return(1);
}
/*****************************************************************************/

void ls_fileEntryToDirListEntry(DirList *dlist, euint8* buf, euint16 offset)
{
	if(offset>480 || offset%32)return;
	
	buf+=offset;
	memCpy(buf+OFFSET_DE_FILENAME,dlist->currentEntry.FileName,LIST_MAXLENFILENAME);
	dlist->currentEntry.Attribute = *(buf+OFFSET_DE_ATTRIBUTE);
	dlist->currentEntry.FileSize = ex_getb32(buf,OFFSET_DE_FILESIZE);
}
/*****************************************************************************/

