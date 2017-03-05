/*****************************************************************************\
*              efs - General purpose Embedded Filesystem library              *
*          --------------------- -----------------------------------          *
*                                                                             *
* Filename : file.c                                                           *
* Description : This file contains functions dealing with files such as:      *
*               fopen, fread and fwrite.                                      *
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
#include "nand_file.h"
#include "includes.h"
/*****************************************************************************/
//надо проверять конец фрагмента!!!!!!!!



_file_param _frag_param;

euint32 efile_read(euint32 size,euint8 *buf)
{// 
unsigned char *_ptail; 

        if(size > PAGE_SIZE) //читать надо не более размера page
          return EMPTY_ID;  
        
        if(size == 0)
          return EMPTY_ID;
        
        if((_frag_param._begin_page == (PARAM_PAGE-1)) &&
           (_frag_param._begin_offset == PAGE_SIZE)
          )
        {
          _frag_param._begin_page = 0;
          _frag_param._begin_offset = 0;
          _frag_param._begin_block++;
          if(isTmpblock(_frag_param._begin_block))
            _frag_param._begin_block++;
          
          if(_frag_param._begin_block>((END_BLOCK-_get_nbad())-1))
            _frag_param._begin_block = BEGIN_BLOCK;
          
unsigned char _res1; 
          _curr_param._ctrl &= ~0x01;
          _res1 = _init_frag_list(_frag_param._begin_block);
          if(_res1)
            return EMPTY_ID;
          if(_frag_param._id != _frag_list[0]._id)
            return EMPTY_ID;
          _frag_param._end_page = _frag_list[0]._end_page;
          _frag_param._end_fill = _frag_list[0]._end_fill; 
          
        }
        
                
        if(size <= (PAGE_SIZE-_frag_param._begin_offset))
        {//buf вмещается в текущ страницу
          if(_frag_param._end_page != EMPTY_ID)
           if((_frag_param._end_page == _frag_param._begin_page) &&
             (_frag_param._begin_offset <= _frag_param._end_fill)
             )
               if(size >= (_frag_param._end_fill - _frag_param._begin_offset))
                size = _frag_param._end_fill - _frag_param._begin_offset;
          
          nand_ReadPage(page, GetValidVBlock(_frag_param._begin_block), _frag_param._begin_page);
          memcpy(buf,&page[_frag_param._begin_offset],size);
          if(size == (PAGE_SIZE-_frag_param._begin_offset))
          {
            if(_frag_param._begin_page == (PARAM_PAGE-1))
            {//последний page блока считали             
              _frag_param._begin_offset = PAGE_SIZE;              
            }
            else
            {
              _frag_param._begin_page++;
              _frag_param._begin_offset = 0;
            }
          }
          else
            _frag_param._begin_offset += size;          
        }
        else
        if((size > (PAGE_SIZE-_frag_param._begin_offset)) &&
           (_frag_param._begin_page < (PARAM_PAGE-1))
          )
        { //buf не вмещается в текущ страницу
          //buf вмещается в текущ блок
          if(_frag_param._end_page != EMPTY_ID)
           if((_frag_param._end_page == _frag_param._begin_page) &&
             (_frag_param._begin_offset <= _frag_param._end_fill)
             )
           {
               size = _frag_param._end_fill - _frag_param._begin_offset;
               if(!size)
                 return size;
               nand_ReadPage(page, GetValidVBlock(_frag_param._begin_block), _frag_param._begin_page);
               memcpy(buf,&page[_frag_param._begin_offset],size);
               return size;
           }
          
          nand_ReadPage(page, GetValidVBlock(_frag_param._begin_block), _frag_param._begin_page);
          memcpy(buf,&page[_frag_param._begin_offset],(PAGE_SIZE-_frag_param._begin_offset));
          _ptail = &buf[PAGE_SIZE-_frag_param._begin_offset];
          size-=(PAGE_SIZE-_frag_param._begin_offset);
          _frag_param._begin_page++;
          _frag_param._begin_offset = size;
          if(_frag_param._end_page != EMPTY_ID)
           if((_frag_param._end_page == _frag_param._begin_page) &&
             (size >= _frag_param._end_fill)
             )
           {
               size = _frag_param._end_fill;
           }          
          nand_ReadPage(page, GetValidVBlock(_frag_param._begin_block), _frag_param._begin_page);
          memcpy(_ptail,page,size);
          size = (_ptail+size) - buf;
           
        }
        else
        if((size > (PAGE_SIZE-_frag_param._begin_offset)) &&
           (_frag_param._begin_page == (PARAM_PAGE-1))
          )
        { //buf не вмещается в текущ страницу
          //buf не вмещается в текущ блок
          if(_frag_param._end_page != EMPTY_ID)
           if((_frag_param._end_page == _frag_param._begin_page) &&
             (_frag_param._begin_offset <= _frag_param._end_fill)
             )
           {
               size = _frag_param._end_fill - _frag_param._begin_offset;
               if(!size)
                 return size;               
               nand_ReadPage(page, GetValidVBlock(_frag_param._begin_block), _frag_param._begin_page);
               memcpy(buf,&page[_frag_param._begin_offset],size);
               return size;
           }
          
          nand_ReadPage(page, GetValidVBlock(_frag_param._begin_block), _frag_param._begin_page);
          memcpy(buf,&page[_frag_param._begin_offset],(PAGE_SIZE-_frag_param._begin_offset));
          _ptail = &buf[PAGE_SIZE-_frag_param._begin_offset];
          size-=(PAGE_SIZE-_frag_param._begin_offset);
          _frag_param._begin_page=0;
          _frag_param._begin_block++;
          if(isTmpblock(_frag_param._begin_block))
            _frag_param._begin_block++;
          
          if(_frag_param._begin_block>((END_BLOCK-_get_nbad())-1))
            _frag_param._begin_block = BEGIN_BLOCK;
          
unsigned char _res1;    
          _curr_param._ctrl &= ~0x01;
          _res1 = _init_frag_list(_frag_param._begin_block);
          if(_res1)
            return EMPTY_ID;
          if(_frag_param._id != _frag_list[0]._id)
            return EMPTY_ID;
          _frag_param._end_page = _frag_list[0]._end_page;
          _frag_param._end_fill = _frag_list[0]._end_fill;  
          
          
          _frag_param._begin_offset = size;
          if(_frag_param._end_page != EMPTY_ID)
           if((_frag_param._end_page == _frag_param._begin_page) &&
             (size >= _frag_param._end_fill)
             )
           {
               size = _frag_param._end_fill;
           }          
          nand_ReadPage(page, GetValidVBlock(_frag_param._begin_block), _frag_param._begin_page);
          memcpy(_ptail,page,size);
          size = (_ptail+size) - buf;
          
        }
  
	return size;
}

euint32 efile_write(euint32 size,euint8* buf)
{
unsigned long _i;  
        if(size > PAGE_SIZE) //писать надо не более размера page
          return EMPTY_ID;
        
        if((_curr_param._page == (PARAM_PAGE-1)) &&
           (_curr_param._offset == PAGE_SIZE)
          )
        {
          _curr_param._page = 0;
          _curr_param._offset = 0;
          _curr_param._ctrl = 0;
          _curr_param._block++;
          if(isTmpblock(_curr_param._block))
            _curr_param._block++;
          if(_curr_param._block>((END_BLOCK-_get_nbad())-1))
            _curr_param._block = BEGIN_BLOCK;
unsigned char _res1; 
          _curr_param._ctrl &= ~0x01;
          _res1 = _init_frag_list(_curr_param._block);
          if(_res1)
            return EMPTY_ID;
          //мы начинаем писать в новый блок надо удалить инфу из frag_list
          _del_frag_list(_curr_param._page,_curr_param._offset); 
          nand_EraseBlock(GetValidVBlock(_curr_param._block));
        }
        
        
          
        if(size <= (PAGE_SIZE-_curr_param._offset))
        {//buf вмещается в текущ страницу
          memcpy(&page[_curr_param._offset],buf,size);
          if(size == (PAGE_SIZE-_curr_param._offset))
          {
            nand_WritePage(page, GetValidVBlock(_curr_param._block), _curr_param._page);
            if(_curr_param._page == (PARAM_PAGE-1))
            {//последний page блока заполнили
              _frag_param._end_page = EMPTY_ID;
              _frag_list_modify(_frag_param._id, _frag_param._type,
                                _frag_param._begin_page,
                                _frag_param._begin_offset,
                                _frag_param._end_page,
                                _frag_param._end_fill
                                );
              _curr_param._ctrl |= 0x08; //надо сбрасывать когда
                                         //изменяется _curr_param._block 
              _frag_param._begin_page = EMPTY_ID;
              _frag_param._begin_offset = EMPTY_ID;
              //_frag_param._end_page = EMPTY_ID;
              _frag_param._end_fill = EMPTY_ID; 
                   
              _frag_list_commit(_curr_param._block);
              _curr_param._ctrl |= 0x02; //надо сбрасывать когда 
                                         //изменяется _curr_param._block
              _sys_page_modify(_curr_param._block,_frag_param._id,_frag_param._type);
              _curr_param._ctrl |= 0x04; //надо сбрасывать когда
                                         //изменяется _curr_param._block,
              _curr_param._offset = PAGE_SIZE;
              
              return 0x0;
            }
            else
            {
              _curr_param._page++;
              _curr_param._offset = 0;
              return 0x0;
            }
          }
          else
          {
            _curr_param._offset += size;
            return 0x0;
          }
        }
        if((size > (PAGE_SIZE-_curr_param._offset)) &&
           (_curr_param._page < (PARAM_PAGE-1))
          )
        { //buf не вмещается в текущ страницу
          //buf вмещается в текущ блок
          memcpy(&page[_curr_param._offset],buf,(PAGE_SIZE-_curr_param._offset));
          size-=(PAGE_SIZE-_curr_param._offset);
          nand_WritePage(page, GetValidVBlock(_curr_param._block), _curr_param._page);
          memcpy(page,&buf[(PAGE_SIZE-_curr_param._offset)],size);
          _curr_param._page++;
          _curr_param._offset = size;  
          return 0x0;
        }
        if((size > (PAGE_SIZE-_curr_param._offset)) &&
           (_curr_param._page == (PARAM_PAGE-1))
          )
        { //buf не вмещается в текущ страницу
          //buf не вмещается в текущ блок
          memcpy(&page[_curr_param._offset],buf,(PAGE_SIZE-_curr_param._offset));
          size-=(PAGE_SIZE-_curr_param._offset);
          nand_WritePage(page, GetValidVBlock(_curr_param._block), _curr_param._page);
          _frag_list_modify(_frag_param._id, _frag_param._type,
                            _frag_param._begin_page,
                            _frag_param._begin_offset,
                            _frag_param._end_page,
                            _frag_param._end_fill
                           );
          _frag_param._begin_page = EMPTY_ID;
          _frag_param._begin_offset = EMPTY_ID;
          _frag_param._end_page = EMPTY_ID;
          _frag_param._end_fill = EMPTY_ID; 
                   
          _frag_list_commit(_curr_param._block);
          _sys_page_modify(_curr_param._block,_frag_param._id,_frag_param._type);
          memcpy(page,&buf[(PAGE_SIZE-_curr_param._offset)],size);
          
          _curr_param._page = 0;
          _curr_param._offset = size;
          _curr_param._block++;
          if(isTmpblock(_curr_param._block))
            _curr_param._block++;          
          if(_curr_param._block>((END_BLOCK-_get_nbad())-1))
            _curr_param._block = BEGIN_BLOCK;
unsigned char _res1;  
          _curr_param._ctrl &= ~0x01;
          _res1 = _init_frag_list(_curr_param._block);
          if(_res1)
            return EMPTY_ID;  
          //мы начинаем писать в новый блок надо удалить инфу из frag_list
          _del_frag_list1();
          nand_EraseBlock(GetValidVBlock(_curr_param._block));
        }

                                
        
	return 0x0;
}
/*****************************************************************************/

static unsigned char _mode;
unsigned long efile_fopen(unsigned long _id, unsigned char _type, eint8 mode)
{
unsigned char _res;

    _mode = mode;
    switch(mode)
    {
      case MODE_WRITE:
        _frag_param._type = _type;
        switch(_type)
        {
          case TYPE_RADIO:
            _curr_param._id_radio++;
            if(_curr_param._id_radio>MAX_ID)
            {
              _curr_param._id_radio = 1;
              _swap_id |= 0x01;
            }
            _frag_param._id = _curr_param._id_radio; 
          break;
          case TYPE_MIC:
            _curr_param._id_mic++;
            if(_curr_param._id_mic>MAX_ID)
            {
              _curr_param._id_mic = 1;
              _swap_id |= 0x02;
            }
            _frag_param._id = _curr_param._id_mic;             
          break;
          case TYPE_LIN:
            _curr_param._id_lin++;
            if(_curr_param._id_lin>MAX_ID)
            {
              _curr_param._id_lin = 1;
              _swap_id |= 0x04;
            }
            _frag_param._id = _curr_param._id_lin;             
          break;
          
        };
        _frag_param._begin_page = _curr_param._page;
        _frag_param._begin_offset = _curr_param._offset;
        _frag_param._end_page = EMPTY_ID;
        _frag_param._end_fill = EMPTY_ID; 
        _res = _init_frag_list(_curr_param._block);
        if(_res)
          return EMPTY_ID; //error         
       //в текущем блоке могут быть данные
       //их нужно записать
        _restore_page(_curr_param._block,_curr_param._page,_curr_param._offset);       
      break;
      case MODE_READ:
_b_frag _res1;        
        _frag_param._type = _type;
        _frag_param._id = _id;
        _res1 = _begin_frag(_id,_type);
        if(_res1._begin_block == EMPTY_ID)
          return EMPTY_ID; //error
        
        //читать будем всегда сначала фрагмента
        _frag_param._begin_block = _res1._begin_block;
        _frag_param._begin_page = _res1._begin_page;
        _frag_param._begin_offset = _res1._begin_offset;
        _frag_param._end_page = _res1._end_page;
        _frag_param._end_fill = _res1._end_fill;        
      break;
        
    };
    
    return _frag_param._id;
}
/*****************************************************************************/

esint8 efile_fclose(void)
{
unsigned char _i;

  if(_mode == MODE_WRITE)
  {
        if((_curr_param._page == (PARAM_PAGE-1)) &&
           (_curr_param._offset == PAGE_SIZE)
          )
        {
          _curr_param._page = 0;
          _curr_param._offset = 0;
          _curr_param._ctrl = 0;
          _curr_param._block++;
          if(isTmpblock(_curr_param._block))
            _curr_param._block++;
          if(_curr_param._block>((END_BLOCK-_get_nbad())-1))
            _curr_param._block = BEGIN_BLOCK;
unsigned char _res1; 
          _curr_param._ctrl &= ~0x01;
          _res1 = _init_frag_list(_curr_param._block);
          if(_res1)
            return EMPTY_ID;
          //мы начинаем писать в новый блок надо удалить инфу из frag_list
          _del_frag_list(_curr_param._page,_curr_param._offset);             
        }
        
 
    _frag_param._end_page = _curr_param._page;
    _frag_param._end_fill = _curr_param._offset; 
    if(_curr_param._offset != 0)
      nand_WritePage(page, GetValidVBlock(_curr_param._block), _curr_param._page);
    
    for(_i=(_curr_param._page+1);_i<=((PARAM_PAGE-1));_i++)
        nand_WritePage(page, GetValidVBlock(_curr_param._block), _i);
    
    if(!(_curr_param._ctrl & 0x08))
      _frag_list_modify(_frag_param._id, _frag_param._type,
                        _frag_param._begin_page,
                        _frag_param._begin_offset,
                        _frag_param._end_page,
                        _frag_param._end_fill
                     );
    if(!(_curr_param._ctrl & 0x02))
      _frag_list_commit(_curr_param._block);    

     
    if(!(_curr_param._ctrl & 0x04))
      _sys_page_modify(_curr_param._block,_frag_param._id,_frag_param._type);
  }
        
  return 0x0;
}


