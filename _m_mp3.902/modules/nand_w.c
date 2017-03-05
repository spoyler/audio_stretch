//��������: ����������� ������� �� nand  

#include <NXP/iolpc3131.h>
#include "includes.h"
#include "lpc_io_stereo.h"
#include "nand.h"
#include "nand_file.h"


/*
����� �������� �������� ������:

��� �����(BEGIN_BLOCK<->END_BLOCK) ����� �� �������� 
�������� CLASTER_SIZE. ������ �������� �����, ����� 
���������������� ������ ������� � ����������� ������ ��������
�������� �����(�� ����� ������ ����� ���)

������ �������� ������ ���� �������� �������� 
PARAM_PAGE � ����������� �����

���� SYSTEM_BLOCK �������� �������� SYSTEM_PAGE, ������� �������� 
��������� ���� ��������� �� �� ����������� ����� ������� � sram �
��������� � ����� ������ ������

*/

//�������� �������� PARAM_PAGE
//������ ����������, ������� ����� � �����

_param_page _frag_list[MAX_IN_BLOCK]; //���� ����� ������ �������� PARAM_PAGE
                                      //������ ����� ������ �������� PARAM_PAGE
                                      //����� ���������� �����
                                      //if _id == EMPTY_ID ��-� ������ �� �������� ����

//�������� �������� SYSTEM_PAGE
//�������� ������: ������ �������� ����������� �������� � _id,_type
//������ �������� ������ ��� ��������� ����� ��������� ��� ��������� _id

_system_page _claster_list[N_CLASTER];//���� ����� ������ �������� SYSTEM_PAGE
                               //������ ����� ������ �������� SYSTEM_PAGE

_end_param  _curr_param; //������� _id ��� ����� ����������
                                //������� block,page,offset

unsigned char _swap_id; //0 bit -> _id_radio ������ MAX � ���� 1
                        //1 bit -> _id_mic ������ MAX � ���� 1
                        //2 bit -> _id_lin ������ MAX � ���� 1

unsigned char page[PAGE_SIZE];
static unsigned long _claster;


unsigned  long BEGIN_BLOCK;
unsigned  long END_BLOCK;
unsigned  long SYSTEM_BLOCK;
unsigned  long TMP_BLOCK;



unsigned long _get_begin_block(void)
{  
unsigned long _img_size;

  nand_ReadPage(page, GetValidVBlock(IMG_BLOCK), 0);
  
  _img_size = ((*((unsigned long *)&page[0x1BE+12]))*512)/(PAGE_SIZE*PAGES_IN_BLOCK);
  _img_size = IMG_BLOCK+_img_size+2;
  
  return _img_size;
}


void _del_frag_list(unsigned long _page,unsigned long _offset)
{
unsigned long _i;

      if((_offset == 0) && (_page == 0))
      {
        _frag_list[0]._id = EMPTY_ID;
        _frag_list[1]._id = EMPTY_ID;
      }
}

void _del_frag_list1(void)
{
unsigned long _i;

        _frag_list[0]._id = EMPTY_ID;
        _frag_list[1]._id = EMPTY_ID;
}



void _restore_page(unsigned long _block,unsigned long _page,unsigned long _offset)
{
unsigned long _res,_i;
unsigned long _tmp_block;

    
  if(_offset > 0)
  {
      _claster = (unsigned long)((_curr_param._block-BEGIN_BLOCK)/CLASTER_SIZE);
      //_block ����������� _claster � ��������� ���� ����� ��������
      _tmp_block = ((_claster+1)*CLASTER_SIZE)-1+BEGIN_BLOCK;


      nand_EraseBlock(GetValidVBlock(_tmp_block));
      for(_i=0;_i<=_curr_param._page;_i++)
      {
        nand_ReadPage(page, GetValidVBlock(_curr_param._block), _i);
        nand_WritePage(page, GetValidVBlock(_tmp_block), _i);
      }
      nand_EraseBlock(GetValidVBlock(_curr_param._block));
      if(_curr_param._page > 0)
        for(_i=0;_i<=(_curr_param._page-1);_i++)
        {
          nand_ReadPage(page, GetValidVBlock(_tmp_block), _i);
          nand_WritePage(page, GetValidVBlock(_curr_param._block), _i);
        }
     
      //��������� �������� ������� � page ��� ������
      nand_ReadPage(page, GetValidVBlock(_tmp_block), _curr_param._page); 
      
  }
#if 1  
  else
    if((_offset == 0) && (_page == 0))
    {
      nand_EraseBlock(GetValidVBlock(_curr_param._block));
      //_init_frag_list ������ ���� ������ ���
      //������� ��� ��������� �� frag_list  
      _del_frag_list(_page,_offset);      
    }
#endif 
}

//���� ����� ������������ tmp ��������
unsigned char isTmpblock(unsigned long _block)
{
unsigned long _tmp_block;

  _claster = (unsigned long)((_block-BEGIN_BLOCK)/CLASTER_SIZE);
  //_block ����������� _claster � ��������� ���� ����� ��������
  _tmp_block = ((_claster+1)*CLASTER_SIZE)-1+BEGIN_BLOCK;
      
  if (_block == _tmp_block)
    return 0x01;
  
  return 0x0;
}

//���� ����������� ���������� ������������ ������ ������
//���� ����� ���������� ����� ��������� ����� ��� ������������ 
int GetValidVBlock(unsigned long _mod)
{
  int block=BEGIN_BLOCK; 
  LoadBadBlockList();
  for (int i=BEGIN_BLOCK; i <= _mod; i++)
  {
    while (IfBlockBad(block)) 
        block++;         
    block++;
  }
  
  return block-1; 
}

void _init_param(void)
{
  InitNAND();
  
  BEGIN_BLOCK = _get_begin_block();
  END_BLOCK = BEGIN_BLOCK+(N_CLASTER*CLASTER_SIZE);
  SYSTEM_BLOCK = END_BLOCK;
  TMP_BLOCK = END_BLOCK+1;
}

//�������� ��� _claster_list
unsigned char _init_nand_w(void)
{
unsigned long _res,_i;

  if((sizeof(_claster_list)+sizeof(_curr_param))>(PAGE_SIZE-1))
    return 0x01;



  
  _nbad_blocks(BEGIN_BLOCK,END_BLOCK);
  _swap_id = 0;
  

  
  nand_ReadPage(page, GetValidVBlock(SYSTEM_BLOCK-_get_nbad()), SYSTEM_PAGE);
  
  _res=0;
  for(_i=0;_i<N_CLASTER;_i++)
  {
    _claster_list[_i]._id = EMPTY_ID;
    _claster_list[_i]._radio_span[0]._id_begin = EMPTY_ID;
    _claster_list[_i]._radio_span[0]._id_end = EMPTY_ID;
    _claster_list[_i]._radio_span[1]._id_begin = EMPTY_ID;
    _claster_list[_i]._radio_span[1]._id_end = EMPTY_ID;
    
    _claster_list[_i]._mic_span[0]._id_begin = EMPTY_ID;
    _claster_list[_i]._mic_span[0]._id_end = EMPTY_ID;
    _claster_list[_i]._mic_span[1]._id_begin = EMPTY_ID;
    _claster_list[_i]._mic_span[1]._id_end = EMPTY_ID;
    
    _claster_list[_i]._lin_span[0]._id_begin = EMPTY_ID;
    _claster_list[_i]._lin_span[0]._id_end = EMPTY_ID;
    _claster_list[_i]._lin_span[1]._id_begin = EMPTY_ID;
    _claster_list[_i]._lin_span[1]._id_end = EMPTY_ID;
    
  }
  memset(&_curr_param,0x0,sizeof(_curr_param));
  _curr_param._block = BEGIN_BLOCK;
  _curr_param._page = 0;
  
  _res=0;
  for(_i=0;_i<100;_i++)
   _res += page[_i]; //����� crc ������ 100 8bits ����
  if(page[sizeof(_claster_list)+sizeof(_curr_param)]==(_res%256))
  {
    if(_res!=0) 
    {
      memcpy(_claster_list,page,sizeof(_claster_list));
      memcpy(&_curr_param,&page[sizeof(_claster_list)],sizeof(_curr_param));
    }
  }
  if(!_curr_param._page)
   _curr_param._page = 0; 
  
 

  return 0x0; //��� ������
}

//���� ����� �����, _id,_type ���������
//��� ��� ���� ������ � _claster_list
//�������� ��-� ������

void _sys_page_modify(unsigned long _block,unsigned long _id,unsigned long _type)
{
 
  _claster = (unsigned long)((_block-BEGIN_BLOCK)/CLASTER_SIZE);
  switch(_type)
  {
    case TYPE_RADIO:
      if(_swap_id & 0x01)
      {
        _swap_id &= ~0x01;
        _claster_list[_claster]._radio_span[0]._id_begin = _id;
        _claster_list[_claster]._radio_span[0]._block_begin = _block;
        _claster_list[_claster]._radio_span[0]._id_end = _id;
        _claster_list[_claster]._radio_span[0]._block_end = _block; 
        _claster_list[_claster]._radio_span[1]._id_begin = EMPTY_ID;
        _claster_list[_claster]._radio_span[1]._id_end = EMPTY_ID;        
      }
      else
      {
      if(_claster_list[_claster]._radio_span[0]._id_begin == EMPTY_ID)
      {
        _claster_list[_claster]._radio_span[0]._id_begin = _id;
        _claster_list[_claster]._radio_span[0]._block_begin = _block;
        _claster_list[_claster]._radio_span[0]._id_end = _id;
        _claster_list[_claster]._radio_span[0]._block_end = _block;
      }      
      else
        if(((_claster_list[_claster]._radio_span[0]._block_end) <= _block) &&
           (_claster_list[_claster]._radio_span[1]._id_begin == EMPTY_ID)
          )
        {
          _claster_list[_claster]._radio_span[0]._id_end = _id;
          _claster_list[_claster]._radio_span[0]._block_end = _block;
        }
        else
        {
            if(_claster_list[_claster]._radio_span[1]._id_begin == EMPTY_ID)
            {
              _claster_list[_claster]._radio_span[1]._id_begin = _id;
              _claster_list[_claster]._radio_span[1]._block_begin = _block;
              _claster_list[_claster]._radio_span[1]._id_end = _id;
              _claster_list[_claster]._radio_span[1]._block_end = _block;
            }
            else
            {
              if(((_claster_list[_claster]._radio_span[1]._block_end) <= _block)
                )
              {
                _claster_list[_claster]._radio_span[1]._id_end = _id;
                _claster_list[_claster]._radio_span[1]._block_end = _block;
              }
              if(_claster_list[_claster]._radio_span[1]._block_end >= 
                 _claster_list[_claster]._radio_span[0]._block_end)
              {
                _claster_list[_claster]._radio_span[0]._id_begin = _claster_list[_claster]._radio_span[1]._id_begin;
                _claster_list[_claster]._radio_span[0]._block_begin = _claster_list[_claster]._radio_span[1]._block_begin;
                _claster_list[_claster]._radio_span[0]._id_end = _claster_list[_claster]._radio_span[1]._id_end;
                _claster_list[_claster]._radio_span[0]._block_end = _claster_list[_claster]._radio_span[1]._block_end; 
                _claster_list[_claster]._radio_span[1]._id_begin = EMPTY_ID;
                _claster_list[_claster]._radio_span[1]._id_end = EMPTY_ID;
              }
            }
        } 
      }
    break;
    case TYPE_MIC:
      if(_swap_id & 0x02)
      {
        _swap_id &= ~0x02;
        _claster_list[_claster]._mic_span[0]._id_begin = _id;
        _claster_list[_claster]._mic_span[0]._block_begin = _block;
        _claster_list[_claster]._mic_span[0]._id_end = _id;
        _claster_list[_claster]._mic_span[0]._block_end = _block;  
        _claster_list[_claster]._mic_span[1]._id_begin = EMPTY_ID;
        _claster_list[_claster]._mic_span[1]._id_end = EMPTY_ID;        
      }
      else
      {      
      if(_claster_list[_claster]._mic_span[0]._id_begin == EMPTY_ID)
      {
        _claster_list[_claster]._mic_span[0]._id_begin = _id;
        _claster_list[_claster]._mic_span[0]._block_begin = _block;
        _claster_list[_claster]._mic_span[0]._id_end = _id;
        _claster_list[_claster]._mic_span[0]._block_end = _block;
      }      
      else
        if(((_claster_list[_claster]._mic_span[0]._block_end) <= _block) &&
           (_claster_list[_claster]._mic_span[1]._id_begin == EMPTY_ID)
          )
        {
          _claster_list[_claster]._mic_span[0]._id_end = _id;
          _claster_list[_claster]._mic_span[0]._block_end = _block;
        }
        else
        {
            if(_claster_list[_claster]._mic_span[1]._id_begin == EMPTY_ID)
            {
              _claster_list[_claster]._mic_span[1]._id_begin = _id;
              _claster_list[_claster]._mic_span[1]._block_begin = _block;
              _claster_list[_claster]._mic_span[1]._id_end = _id;
              _claster_list[_claster]._mic_span[1]._block_end = _block;
            }
            else
            {
              if(((_claster_list[_claster]._mic_span[1]._block_end) <= _block)
                )
              {
                _claster_list[_claster]._mic_span[1]._id_end = _id;
                _claster_list[_claster]._mic_span[1]._block_end = _block;
              }
              if(_claster_list[_claster]._mic_span[1]._block_end >= 
                 _claster_list[_claster]._mic_span[0]._block_end)
              {
                _claster_list[_claster]._mic_span[0]._id_begin = _claster_list[_claster]._mic_span[1]._id_begin;
                _claster_list[_claster]._mic_span[0]._block_begin = _claster_list[_claster]._mic_span[1]._block_begin;
                _claster_list[_claster]._mic_span[0]._id_end = _claster_list[_claster]._mic_span[1]._id_end;
                _claster_list[_claster]._mic_span[0]._block_end = _claster_list[_claster]._mic_span[1]._block_end; 
                _claster_list[_claster]._mic_span[1]._id_begin = EMPTY_ID;
                _claster_list[_claster]._mic_span[1]._id_end = EMPTY_ID;
              }
            }
        }
      }
    break;
    case TYPE_LIN:
      if(_swap_id & 0x04)
      {
        _swap_id &= ~0x04;
        _claster_list[_claster]._lin_span[0]._id_begin = _id;
        _claster_list[_claster]._lin_span[0]._block_begin = _block;
        _claster_list[_claster]._lin_span[0]._id_end = _id;
        _claster_list[_claster]._lin_span[0]._block_end = _block;  
        _claster_list[_claster]._lin_span[1]._id_begin = EMPTY_ID;
        _claster_list[_claster]._lin_span[1]._id_end = EMPTY_ID;        
      }
      else
      {      
      if(_claster_list[_claster]._lin_span[0]._id_begin == EMPTY_ID)
      {
        _claster_list[_claster]._lin_span[0]._id_begin = _id;
        _claster_list[_claster]._lin_span[0]._block_begin = _block;
        _claster_list[_claster]._lin_span[0]._id_end = _id;
        _claster_list[_claster]._lin_span[0]._block_end = _block;
      }      
      else
        if(((_claster_list[_claster]._lin_span[0]._block_end) <= _block) &&
           (_claster_list[_claster]._lin_span[1]._id_begin == EMPTY_ID)
          )
        {
          _claster_list[_claster]._lin_span[0]._id_end = _id;
          _claster_list[_claster]._lin_span[0]._block_end = _block;
        }
        else
        {
            if(_claster_list[_claster]._lin_span[1]._id_begin == EMPTY_ID)
            {
              _claster_list[_claster]._lin_span[1]._id_begin = _id;
              _claster_list[_claster]._lin_span[1]._block_begin = _block;
              _claster_list[_claster]._lin_span[1]._id_end = _id;
              _claster_list[_claster]._lin_span[1]._block_end = _block;
            }
            else
            {
              if(((_claster_list[_claster]._lin_span[1]._block_end) <= _block)
                )
              {
                _claster_list[_claster]._lin_span[1]._id_end = _id;
                _claster_list[_claster]._lin_span[1]._block_end = _block;
              }
              if(_claster_list[_claster]._lin_span[1]._block_end >= 
                 _claster_list[_claster]._lin_span[0]._block_end)
              {
                _claster_list[_claster]._lin_span[0]._id_begin = _claster_list[_claster]._lin_span[1]._id_begin;
                _claster_list[_claster]._lin_span[0]._block_begin = _claster_list[_claster]._lin_span[1]._block_begin;
                _claster_list[_claster]._lin_span[0]._id_end = _claster_list[_claster]._lin_span[1]._id_end;
                _claster_list[_claster]._lin_span[0]._block_end = _claster_list[_claster]._lin_span[1]._block_end; 
                _claster_list[_claster]._lin_span[1]._id_begin = EMPTY_ID;
                _claster_list[_claster]._lin_span[1]._id_end = EMPTY_ID;
              }
            }
        } 
      }
    break;
    
  };
  _claster_list[_claster]._id = _claster;
}

//�������� SYSTEM_PAGE
//_p == 1 -> ������ ������ SYSTEM_BLOCK
void _sys_page_commit(unsigned char _p1)
{
unsigned long _res;
unsigned char _i;
unsigned char *_p=(unsigned char *)_claster_list;

  
  nand_EraseBlock(GetValidVBlock(SYSTEM_BLOCK-_get_nbad()));
  if(_p1)
    return;
 
  _res=0;
  for(_i=0;_i<100;_i++)
   _res += *_p++; //����� crc ������ 100 8bits ����
  
  memcpy(page,_claster_list,sizeof(_claster_list));
  memcpy(&page[sizeof(_claster_list)],&_curr_param,sizeof(_curr_param));
  page[sizeof(_claster_list)+sizeof(_curr_param)] = (_res%256);
  nand_WritePage(page, GetValidVBlock(SYSTEM_BLOCK-_get_nbad()), SYSTEM_PAGE);
}
//----------------------------------------

//�������� ��� _frag_list

//��� �������� ����� ������ _frag_list
unsigned char _init_frag_list(unsigned _block)
{
unsigned long _res,_i;
unsigned char page1[PAGE_SIZE];

  if(sizeof(_frag_list)>PAGE_SIZE)
    return 0x01;
  
  if(_curr_param._ctrl & 0x01)
    return 0x0; //������ ��� ������

  for(_i=0;_i<MAX_IN_BLOCK;_i++)
    _frag_list[_i]._id = EMPTY_ID;  
  nand_ReadPage(page1, GetValidVBlock(_block), PARAM_PAGE);
  memcpy(_frag_list,page1,sizeof(_frag_list));
  
  _curr_param._block = _block; 
  _curr_param._ctrl |= 0x01; //������ �������
  _curr_param._ctrl &= ~0x02; //����� ����������
  
  return 0x0;
}

//���� ��������� ���������,
//�������� ������ ��������� ��������, 
//��������� ���������,
//������� ������ ������ ��������� ��������   
  
void _frag_list_modify(unsigned long _id, unsigned char _type,
                       unsigned long _begin_page,unsigned long _begin_offset,
                       unsigned long _end_page,unsigned long _end_fill
                      )
{
unsigned long _i,_j;  

    switch(_type)
    {
        case TYPE_RADIO:
          _curr_param._id_radio = _id; 
        break;
        case TYPE_MIC:
          _curr_param._id_mic = _id; 
        break;
        case TYPE_LIN:
          _curr_param._id_lin = _id; 
        break;
    };

    if((_begin_page == EMPTY_ID) && (_end_page != EMPTY_ID))
    { //�������� ������� �� �������� �����,� ���������� � �������
      _frag_list[0]._id = _id;
      _frag_list[0]._type = _type;
      _frag_list[0]._begin_page = _begin_page;
      _frag_list[0]._end_page = _end_page;
      _frag_list[0]._end_fill = _end_fill;
      _j=1;
      for(_i=0;_i<MAX_IN_BLOCK;_i++)
      {
        if(_frag_list[_j]._id != EMPTY_ID)
          if(((_end_page*PAGE_SIZE)+_end_fill) <
             ((_frag_list[_i]._begin_page*PAGE_SIZE)+_frag_list[_i]._begin_offset)
            )
          {
            _frag_list[_j]._id = _frag_list[_i]._id;
            _frag_list[_j]._type = _frag_list[_i]._type;
            _frag_list[_j]._begin_page = _frag_list[_i]._begin_page;
            _frag_list[_j]._begin_offset = _frag_list[_i]._begin_offset;
            _frag_list[_j]._end_page = _frag_list[_i]._end_page;    
            _frag_list[_j++]._end_fill = _frag_list[_i]._end_fill;
          }
      }
      if(_j<MAX_IN_BLOCK)
        _frag_list[_j]._id = EMPTY_ID;
    }
    if((_begin_page != EMPTY_ID) && (_end_page == EMPTY_ID))
    { //�������� ������� � ������� �����, �� �� ���������� � ���
      for(_i=0;_i<MAX_IN_BLOCK;_i++)
      {
        if(((_begin_page*PAGE_SIZE)+_begin_offset) < 
           ((_frag_list[_i]._end_page*PAGE_SIZE)+_frag_list[_i]._end_fill)
          )
        {
          _frag_list[_i]._id = _id;
          _frag_list[_i]._type = _type;
          _frag_list[_i]._begin_page = _begin_page;
          _frag_list[_i]._begin_offset = _begin_offset;
          _frag_list[_i]._end_page = _end_page;
          break;
        }
      }      
      if((_i+1) <MAX_IN_BLOCK)
        _frag_list[_i+1]._id = EMPTY_ID;
        
    }
    if((_begin_page == EMPTY_ID) && (_end_page == EMPTY_ID))
    { //�������� ������� �� �������� ����� � �� ���������� � ���
      for(_i=0;_i<MAX_IN_BLOCK;_i++)
        _frag_list[_i]._id = EMPTY_ID;
      _frag_list[0]._id = _id;
      _frag_list[0]._type = _type;
      _frag_list[0]._begin_page = _begin_page;
      _frag_list[0]._end_page = _end_page;
      _frag_list[1]._id = EMPTY_ID;
    }
    if((_begin_page != EMPTY_ID) && (_end_page != EMPTY_ID))
    { //�������� ������� � ������� ����� � � ��� ����������
      _curr_param._page = _end_page;
      _curr_param._offset = _end_fill;

      for(_i=0;_i<MAX_IN_BLOCK;_i++)
      {
        if((((_begin_page*PAGE_SIZE)+_begin_offset) < 
           ((_frag_list[_i]._end_page*PAGE_SIZE)+_frag_list[_i]._end_fill)) ||
           (_frag_list[_i]._id == EMPTY_ID)
          )
        {
          
          _frag_list[_i]._type = _type;
          _frag_list[_i]._begin_page = _begin_page;
          _frag_list[_i]._begin_offset = _begin_offset;
          _frag_list[_i]._end_page = _end_page;
          _frag_list[_i]._end_fill = _end_fill;
          _j = _i+1;
          if(_frag_list[_i]._id == EMPTY_ID)
          {
            _frag_list[_i]._id = _id;
            break;
          }
          _frag_list[_i]._id = _id;
        }
        else    
        if(((_end_page*PAGE_SIZE)+_end_fill) <
           ((_frag_list[_i]._begin_page*PAGE_SIZE)+_frag_list[_i]._begin_offset)
          )
          {
            _frag_list[_j]._id = _frag_list[_i]._id;
            _frag_list[_j]._type = _frag_list[_i]._type;
            _frag_list[_j]._begin_page = _frag_list[_i]._begin_page;
            _frag_list[_j]._begin_offset = _frag_list[_i]._begin_offset;
            _frag_list[_j]._end_page = _frag_list[_i]._end_page;    
            _frag_list[_j++]._end_fill = _frag_list[_i]._end_fill;
          }
      }
      if(_j<MAX_IN_BLOCK)
        _frag_list[_j]._id = EMPTY_ID;
    }
}

//�������� ����� ����� 
void _frag_list_commit(unsigned _block)
{
unsigned long _res,_i;

      
  memcpy(page,_frag_list,sizeof(_frag_list));
  nand_WritePage(page, GetValidVBlock(_block), PARAM_PAGE);
      

  
  
  _curr_param._ctrl &= ~0x01; //������ commited
  _curr_param._ctrl |= 0x02; //������ commited
}
//----------------------------------------


//���� _id,_type
//���� �����:   
//_begin_block; //��������� ���� ��������� ��� ������
//_begin_page; //��������� ���������
//_begin_offset; //�������� ������ ��������� �������� 

_b_frag _begin_frag(unsigned long _id,unsigned long _type)
{
unsigned long _i,_j,_k;
unsigned char _res1;
_b_frag _res;
unsigned long _claster = 0;

  _res._begin_block = EMPTY_ID;

  for(_k=_claster;_k<N_CLASTER;_k++)
  {
        
      if(_type == TYPE_RADIO)
      {
        if((_id >=_claster_list[_k]._radio_span[0]._id_begin) &&
           (_id <=_claster_list[_k]._radio_span[0]._id_end)
          )
        {
            _claster = _k; 
        }
        else  
          if((_id >=_claster_list[_k]._radio_span[1]._id_begin) &&
            (_id <=_claster_list[_k]._radio_span[1]._id_end)
            )
          {
              _claster = _k; 
          }
          else
            continue;
      }
    
      if(_type == TYPE_MIC)
      {
        if((_id >=_claster_list[_k]._mic_span[0]._id_begin) &&
           (_id <=_claster_list[_k]._mic_span[0]._id_end)
          )
        {
            _claster = _k; 
        }
        else  
          if((_id >=_claster_list[_k]._mic_span[1]._id_begin) &&
            (_id <=_claster_list[_k]._mic_span[1]._id_end)
            )
          {
              _claster = _k; 
          }
          else
            continue;
      }

      if(_type == TYPE_LIN)
      {
        if((_id >=_claster_list[_k]._lin_span[0]._id_begin) &&
           (_id <=_claster_list[_k]._lin_span[0]._id_end)
          )
        {
            _claster = _k; 
        }
        else  
          if((_id >=_claster_list[_k]._lin_span[1]._id_begin) &&
            (_id <=_claster_list[_k]._lin_span[1]._id_end)
            )
          {
              _claster = _k; 
          }
          else
            continue;
      }
      
  
      for(_i=0;_i<CLASTER_SIZE;_i++)
      {
        _curr_param._ctrl &= ~0x01;
        _res1 = _init_frag_list(BEGIN_BLOCK+(CLASTER_SIZE*_claster)+_i);
        if(_res1)
        {
          _res._begin_block = EMPTY_ID;
          break;
        }
        for(_j=0;_j<MAX_IN_BLOCK;_j++)
        {
          if(_frag_list[_j]._id == EMPTY_ID)
          {
            _res._begin_block = EMPTY_ID;
            break;
          }
          if( (_frag_list[_j]._id == _frag_param._id) &&
            (_frag_list[_j]._type == _frag_param._type) &&
            (_frag_list[_j]._begin_page != EMPTY_ID)
            )
          {
            _res._begin_block = BEGIN_BLOCK+(CLASTER_SIZE*_claster)+_i;
            _res._begin_page = _frag_list[_j]._begin_page;
            _res._begin_offset = _frag_list[_j]._begin_offset;
            _res._end_page = _frag_list[_j]._end_page;
            _res._end_fill = _frag_list[_j]._end_fill;            
            return _res;
          }
        }
      }
  }
  
  return _res;
}


