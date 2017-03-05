//Описание: хранение m_place.* 
//тк нет backup memory будем хранить в рег. RTC, они 
//питаются от VBAT pin

//выше сказанное только идея, а пока будем сохранять 
//m_place.* на flash

#include "nand.h"
#include "lpc32xx_slcnand_driver.h"

#include "includes.h"

#define _START_PARAM  0x00030000 //стартовый адрес внутр flash для записи параметров
#define _START_KEY_B  0x00040000 //стартовый адрес сохран статуса блокировка
#define _START_DISK_S  0x00050000 //стартовый адрес для сохранения размера 1 файла первой папки
#define _START_VER    0x0007B000 //стартовый адрес для сохранения версии


extern INT_32 nand;

extern NAND_GEOM_T geom;


//очистим область под версию
void _put_empty_ver()
{
unsigned long _flash_addr;
unsigned char _buff[5];

      _flash_addr=_START_VER;
      
      memset(_buff,0x0,5);
#if 0 //FIX_DEBUG     
      memcpy(_src_buff,(unsigned char *)_buff,5);
      if(_iap_find_erase_prepare_sector(60*1024, _flash_addr) == 0)
       _iap_write_data(60*1024,_flash_addr,_src_buff, _IAP_BUFF_SIZE);
#endif  
}


//будем хранить версию проги в формате 4bytes версия, 5 byte CRC
void _put_ver(unsigned char *_p)
{
unsigned long _flash_addr;
unsigned char _buff[5];
unsigned long _crc;
unsigned char _i;
      _flash_addr=_START_VER;
      
      _crc=0;
      for(_i=0;_i<4;_i++)
      {
        _buff[_i]=_p[_i];
        _crc+=_p[_i];
      }
      _buff[_i]=_crc%256;
#if 0 //FIX_DEBUG     
      memcpy(_src_buff,(unsigned char *)_buff,5);
      if(_iap_find_erase_prepare_sector(60*1024, _flash_addr) == 0)
       _iap_write_data(60*1024,_flash_addr,_src_buff, _IAP_BUFF_SIZE);
#endif  
}

//взять версию проги
//returned 0x01 not valid
unsigned char _get_ver(unsigned char *_p)
{
unsigned char *_flash_addr;
unsigned char _buff[5];
unsigned long _crc;
unsigned char _i;
      _flash_addr=(unsigned char *)_START_VER;
      
      _crc=0;
#if 0//FIX_DEBUG      
      for(_i=0;_i<4;_i++)
      {
        _p[_i]=_flash_addr[_i];
        _crc+=_flash_addr[_i];
      }      
      if((_crc%256!=_flash_addr[_i]) || ((_crc%256)==0x0))
        return 0x01;
      
      return 0x0;
#else
      return 0x01;      
#endif      
}

unsigned char page[PAGE_SIZE];

#define _PARAM_BLOCK  18

//будем хранить m_place.* на внутр flash по addr _START_PARAM
void _param_put(unsigned char _p1)
{
unsigned char _i0,_i1=1;
int err;

  for (int _i0=1; _i0 < _PARAM_BLOCK; _i0++)
  {
    while (IfBlockBad(_i1)) 
      _i1++;         
    _i1++;
  } 

    err = slcnand_read_sector(nand, _i1*geom.pages_per_block+0, (void *)(0x80000000+0*PAGE_SIZE), 0); //lkf param
    if (err == _ERROR)
      asm("nop");
    err = slcnand_read_sector(nand, _i1*geom.pages_per_block+1, (void *)(0x80000000+1*PAGE_SIZE), 0); //mp3 param
    if (err == _ERROR)
      asm("nop");
    err = slcnand_read_sector(nand, _i1*geom.pages_per_block+2, (void *)(0x80000000+2*PAGE_SIZE), 0); //mp3 USB param
    if (err == _ERROR)
      asm("nop"); 
    err = slcnand_read_sector(nand, _i1*geom.pages_per_block+3, (void *)(0x80000000+3*PAGE_SIZE), 0); //fm param
    if (err == _ERROR)
      asm("nop");  
    err = slcnand_read_sector(nand, _i1*geom.pages_per_block+4, (void *)(0x80000000+4*PAGE_SIZE), 0); //lin,mic param
    if (err == _ERROR)
      asm("nop");  
    err = slcnand_read_sector(nand, _i1*geom.pages_per_block+5, (void *)(0x80000000+5*PAGE_SIZE), 0); //button test param
    if (err == _ERROR)
      asm("nop");   
    err = slcnand_read_sector(nand, _i1*geom.pages_per_block+6, (void *)(0x80000000+6*PAGE_SIZE), 0); //linux boot param
    if (err == _ERROR)
      asm("nop");   
    err = slcnand_read_sector(nand, _i1*geom.pages_per_block+7, (void *)(0x80000000+7*PAGE_SIZE), 0); //tts robot param
    if (err == _ERROR)
      asm("nop");    
    
    err = slcnand_erase_block(nand, _i1);
    
    memcpy((void *)(0x80000000+1*PAGE_SIZE),(void *)&m_place,sizeof(m_place));
    
    err = slcnand_write_sector(nand, _i1*geom.pages_per_block+0, (void *)(0x80000000+0*PAGE_SIZE), 0); //lkf param
    if (err == _ERROR)
      asm("nop");
    err = slcnand_write_sector(nand, _i1*geom.pages_per_block+1, (void *)(0x80000000+1*PAGE_SIZE), 0); //mp3 param
    if (err == _ERROR)
      asm("nop");
    err = slcnand_write_sector(nand, _i1*geom.pages_per_block+2, (void *)(0x80000000+2*PAGE_SIZE), 0); //mp3 USB param
    if (err == _ERROR)
      asm("nop"); 
    err = slcnand_write_sector(nand, _i1*geom.pages_per_block+3, (void *)(0x80000000+3*PAGE_SIZE), 0); //fm param
    if (err == _ERROR)
      asm("nop");  
    err = slcnand_write_sector(nand, _i1*geom.pages_per_block+4, (void *)(0x80000000+4*PAGE_SIZE), 0); //lin,mic param
    if (err == _ERROR)
      asm("nop");  
    err = slcnand_write_sector(nand, _i1*geom.pages_per_block+5, (void *)(0x80000000+5*PAGE_SIZE), 0); //button test param
    if (err == _ERROR)
      asm("nop");   
    err = slcnand_write_sector(nand, _i1*geom.pages_per_block+6, (void *)(0x80000000+6*PAGE_SIZE), 0); //linux boot param
    if (err == _ERROR)
      asm("nop");   
    err = slcnand_write_sector(nand, _i1*geom.pages_per_block+7, (void *)(0x80000000+7*PAGE_SIZE), 0); //tts robot param
    if (err == _ERROR)
      asm("nop");    
}



//будем брать m_place.* из внутр flash по addr _START_PARAM
unsigned char _param_get()
{
unsigned char _i0,_i1=1;
int err;

  for (int _i0=1; _i0 < _PARAM_BLOCK; _i0++)
  {
    while (IfBlockBad(_i1)) 
      _i1++;         
    _i1++;
  }
  
#if 0 //PORT
    if(m_env33)
      return 0x0;
#endif   
  

    err = slcnand_read_sector(nand, _i1*geom.pages_per_block+1, page, 0);
    if (err == _ERROR)
      asm("nop");
    memcpy((void *)&m_place,page,sizeof(m_place));
    
      
#if !_FIX1        
   
    if(strstr(m_place._sample,(unsigned char *)"LKF\0") == NULL)
       return 0x01;
#endif      
        
  return 0x0; //парам загружены

}


//статус блокировки клавы
unsigned char _get_key_block()
{
unsigned char *_p;
      _p = (unsigned char *)_KEYBS_LOCK; 

      
#if 0//FIX_DEBUG //PORT
      if((*_p & 0x02) == 0x02)
        return 0x01; //блокирована
#endif      
      return 0x0;
}


//сохранить статус блокировки клавы
unsigned char _keep_key_block(unsigned char _code)
{
             

  return 0x01;
}

unsigned long _file_size;
//сравним размер текущего первого файла с тем что во flash
unsigned char _get_file_size(unsigned long _curr)
{
unsigned char *_p;
      _p = (unsigned char *)_FIRST_FILE_SIZE; 


#if 0//FIX_DEBUG PORT     
      _file_size=*((unsigned long *)((unsigned char *)_p));
      if(_file_size == _curr)
        return 0x01; //одинаковы
#endif      
      return 0x0; //нет
}


//сохранить размер первого файла 
unsigned char _keep_file_size(unsigned long _code)
{

unsigned char *_p;
      _p = (unsigned char *)_FIRST_FILE_SIZE;
      
#if 0//FIX_DEBUG PORT
      *(unsigned long *)_p = _code;
#endif              

  return 0x01;
}



//проверим параметры на пригодность к текущ карте
unsigned char _valid_get()
{
EmbeddedFile        file;
unsigned long res;
unsigned char _fname[20];
unsigned char _fname1[10];
unsigned char *p_point;
unsigned char _folder[100];
DirList        list1;
unsigned char _file_name[15];


  
#if !_FIX1
      _real_folder(m_place._folder_0, _folder);
#endif      
      if(ls_openDir(&list1, &(efs.myFs), _folder) != 0)
       return 0x01; //парам непригодны 
      ls_getNext(&list1);
      while(_num_frag(m_place._sample) != list1.mCount)
      {
          res = ls_getNext(&list1);
          if(res != 0)
            break;
      }
      
      p_point = strchr(m_place._sample,' ');
      if(p_point != NULL)
      {
        *p_point = '\0';
        format_file_name(_file_name, list1.currentEntry.FileName);
        if(strlen(_folder)==1)
          sprintf(_fname,"/%s\0",_file_name);
        else
          sprintf(_fname,"%s/%s\0",_folder,_file_name);
        *p_point = ' ';
      }
      else
      {
        if(m_place._fpos != 0xffffffff)
          return 0x01;
        format_file_name(_file_name, list1.currentEntry.FileName);
        if(strlen(_folder)==1)
          sprintf(_fname,"/%s\0",_file_name);
        else        
          sprintf(_fname,"%s/%s\0",_folder,_file_name);
      }
      
      if(file_fopen(&file,&(efs.myFs),_fname,'r')==0)
      {
          if(m_place._fpos > file.FileSize)
          {
            file_fclose(&file);
            return 0x01;
          }
      
      }
      else
        return 0x01; //парам непригодны
      
  file_fclose(&file);      
  return 0x0; //парам пригодны
}


unsigned char _file__[10];
unsigned char _ext__[10];

void _patch_dot__(unsigned char *_src)
{

   memset(_file__,'\0',10);
   memset(_ext__,'\0',10);
   
   
     memcpy(_file__,_src,8);
     _trim(_file__); //убрать заверш пробелы
     memcpy(_ext__,_src+8,3);
     _trim(_ext__); //убрать заверш пробелы

}

static unsigned char       file_name[100];
//по таблице уровней делаем реальный путь
//return 0x01 если папка последняя
unsigned char _real_folder(struct _level_tab *_in, unsigned char *_out)
{
unsigned char *_p1;
unsigned short _num;
DirList _list1;
unsigned char _i;
unsigned char *_pout=_out;
unsigned short _end_dir=0x0;

  

   
  _i=0x0;
  *_pout='\0';
  if(!m_place._folder_0[0]._folder_num)
  {
    sprintf(_out,"/\0");
    return 0x0;
  }  
  if(m_place._folder_0[0]._folder_num==0xffff)
  { //надо на последнюю книгу бежать
    sprintf(_pout,"/\0");
    _num=0x0;
    ls_openDir(&_list1, &(efs.myFs), _out);
    while(!ls_getNext(&_list1))
      if((_list1.currentEntry.Attribute & 0x10) == 0x10)
      {
        _num++;
      }
    ls_getPrev(&_list1);
    m_place._folder_0[0]._folder_num=_num;
    _pout += strlen(_pout);
    
    _patch_dot__(_list1.currentEntry.FileName);
    if(_list1.currentEntry.FileName[8]!=0x20)
    {
      sprintf(_pout,"%s.%s\0",_file__,_ext__);
    }
    else  
      sprintf(_pout,"%s\0",_list1.currentEntry.FileName);

//---------    
    _trim(_out); //убрать заверш пробелы
    _pout += strlen(_pout);
    //скажем фразу "Переход на книгу" 
     sprintf(file_name,"/SYS/_33.mp3\0");
    _play_rem(file_name,0);
#if !_FIX1    
    sprintf(file_name,"%d.",m_place._folder_0[0]._folder_num);
//воспроизв номер книги    
    _play_frag_number(file_name,2);
#endif
    
    return 0x0;    
  }
  
  while(m_place._folder_0[_i]._folder_num && _i<_LEVEL_MAX)
  {
    sprintf(_pout,"/\0");
    _num = m_place._folder_0[_i++]._folder_num;  
    ls_openDir(&_list1, &(efs.myFs), _out);
    while(_num != 0)
    {
    _reset_WD(); //reset WD
     if(ls_getNext(&_list1) != 0)
     {
       _out[0]='\0';
       return 0x0; //нет такой папки
     }
    
     if((_list1.currentEntry.Attribute & 0x10) == 0x10) //папка 
      _num--;
     
    }       
    _pout += strlen(_pout);
    _patch_dot__(_list1.currentEntry.FileName);
    if(_list1.currentEntry.FileName[8]!=0x20)
    {
      sprintf(_pout,"%s.%s\0",_file__,_ext__); //для папок с .
    }
    else  
      sprintf(_pout,"%s\0",_list1.currentEntry.FileName);
    _trim(_out); //убрать заверш пробелы
    _pout += strlen(_pout);
    if(_i==1)
    {
//может последняя книга  
      while(!ls_getNext(&_list1))
      if((_list1.currentEntry.Attribute & 0x10) == 0x10)
      {
        _end_dir=0x01; //не последняя папка
        break;
        
      }
      
    }
  }
  if(strlen(_out)==0)
  {
    sprintf(_out,"/\0");
    return 0x0;
  }
  
  if(_end_dir)
    return 0x0;
  
  return 0x01;
}
