/***********************************************************************
 * $Id:: sdcard_no_dma_example.c 1868 2009-05-19 20:29:10Z wellsk      $
 *
 * Project: SDMMC driver example with interrupts (FIFO mode)
 *
 * Description:
 *     A SD card controller driver example using SD/MMC.
 *
 * Notes:
 *     This examples has no direct output. This code must be executed
 *     with a debugger to see how it works. The write functionality
 *     has been disabled by default to prevent unintended writes to
 *     the SD/MMC cards. To enable it, uncomment the MMCWRITE define.
 *     Be careful using MMCWRITE as it may make your cards unusable
 *     without a card reformat. Use only with cards that do not have
 *     important data!
 *
 ***********************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
 **********************************************************************/

#include "lpc_types.h"
#include "lpc_irq_fiq.h"
#include "lpc_arm922t_cp15_driver.h"
#include "phy3250_board.h"
#include "lpc32xx_intc_driver.h"
#include "lpc32xx_timer_driver.h"
#include "lpc32xx_sdcard_driver.h"
#include "sdmmc_dma_example.h"

/* Uncomment the following line to enable SDMMC write functionality */
#define MMCWRITE

/* Blocks to transfer and first block to use */
#define BLKSTOTRANFER 4
#define BLKFIRST      0

/* This array maps the SDMMC command enumeration to the hardware SDMMC
   command index, the controller setup value, and the expected response
   type */
SDMMC_CMD_CTRL_T sdmmc_cmds[SDMMC_INVALID_CMD] =
{
  /* SDMMC_IDLE */
  {0,  SDMMC_RESPONSE_NONE},
  /* MMC_SENDOP_COND */
  {1,  SDMMC_RESPONSE_R3},
  /* SDMMC_ALL_SEND_CID */
  {2,  SDMMC_RESPONSE_R2},
  /* SDMMC_SRA */
  {3,  SDMMC_RESPONSE_R1},
  /* MMC_PROGRAM_DSR */
  {4,  SDMMC_RESPONSE_NONE},
  /* SDMMC_SELECT_CARD */
  {7,  SDMMC_RESPONSE_R1},
                      /* SDMMC_SEND_IF_COND */
                      {8,  SDMMC_RESPONSE_R3},
  /* SDMMC_SEND_CSD */
  {9,  SDMMC_RESPONSE_R2},
  /* SDMMC_SEND_CID */
  {10, SDMMC_RESPONSE_R2},
  /* SDMMC_READ_UNTIL_STOP */
  {11, SDMMC_RESPONSE_R1},
  /* SDMMC_STOP_XFER */
  {12, SDMMC_RESPONSE_R1},
  /* SDMMC_SSTAT */
  {13, SDMMC_RESPONSE_R1},
  /* SDMMC_INACTIVE */
  {15, SDMMC_RESPONSE_NONE},
  /* SDMMC_SET_BLEN */
  {16, SDMMC_RESPONSE_R1},
  /* SDMMC_READ_SINGLE */
  {17, SDMMC_RESPONSE_R1},
  /* SDMMC_READ_MULTIPLE */
  {18, SDMMC_RESPONSE_R1},
  /* SDMMC_WRITE_UNTIL_STOP */
  {20, SDMMC_RESPONSE_R1},
  /* SDMMC_SET_BLOCK_COUNT */
  {23, SDMMC_RESPONSE_R1},
  /* SDMMC_WRITE_SINGLE */
  {24, SDMMC_RESPONSE_R1},
  /* SDMMC_WRITE_MULTIPLE */
  {25, SDMMC_RESPONSE_R1},
  /* MMC_PROGRAM_CID */
  {26, SDMMC_RESPONSE_R1},
  /* SDMMC_PROGRAM_CSD */
  {27, SDMMC_RESPONSE_R1},
  /* SDMMC_SET_WR_PROT */
  {28, SDMMC_RESPONSE_R1B},
  /* SDMMC_CLEAR_WR_PROT */
  {29, SDMMC_RESPONSE_R1B},
  /* SDMMC_SEND_WR_PROT */
  {30, SDMMC_RESPONSE_R1},
  /* SD_ERASE_BLOCK_START */
  {32, SDMMC_RESPONSE_R1},
  /* SD_ERASE_BLOCK_END */
  {33, SDMMC_RESPONSE_R1},
  /* MMC_ERASE_BLOCK_START */
  {35, SDMMC_RESPONSE_R1},
  /* MMC_ERASE_BLOCK_END */
  {36, SDMMC_RESPONSE_R1},
  /* MMC_ERASE_BLOCKS */
  {38, SDMMC_RESPONSE_R1B},
  /* MMC_FAST_IO */
  {39, SDMMC_RESPONSE_R4},
  /* MMC_GO_IRQ_STATE */
  {40, SDMMC_RESPONSE_R5},
  /* MMC_LOCK_UNLOCK */
  {42, SDMMC_RESPONSE_R1B},
  /* SDMMC_APP_CMD */
  {55, SDMMC_RESPONSE_R1},
  /* SDMMC_GEN_CMD */
  {56, SDMMC_RESPONSE_R1B},
                          /* SDMMC_READ_OCR */
                          {58, SDMMC_RESPONSE_R3}
};

/* This array maps the SDMMC application specific command enumeration to
   the hardware SDMMC command index, the controller setup value, and the
   expected response type */
SDMMC_CMD_CTRL_T sdmmc_app_cmds[SD_INVALID_APP_CMD] =
{
  /* SD_SET_BUS_WIDTH */
  {6,  SDMMC_RESPONSE_R1},
  /* SD_SEND_STATUS */
  {13, SDMMC_RESPONSE_R1},
  /* SD_SEND_WR_BLOCKS */
  {22, SDMMC_RESPONSE_R1},
  /* SD_SET_ERASE_COUNT */
  {23, SDMMC_RESPONSE_R1},
  /* SD_SENDOP_COND */
  {41, SDMMC_RESPONSE_R3},
  /* SD_CLEAR_CARD_DET */
  {42, SDMMC_RESPONSE_R1},
  /* SD_SEND_SCR */
  {51, SDMMC_RESPONSE_R1}
};

static volatile INT_32 cmdresp, datadone;
INT_32 sddev;
UNS_32 rca;
UNS_32 databuff[512 * BLKSTOTRANFER / sizeof(UNS_32)];
INT_32 sdcardtype;

#if 0
/** EFSL */
#include "efs.h"
#include "ls.h"
#include "mkfs.h"
#include "debug.h"
//#include "lpc_config.h"
#include "inttypes.h"

#include "string.h"

EmbeddedFileSystem  efs;
EmbeddedFileSystem  efs_flash;
EmbeddedFile        file;
EmbeddedFile        file1;
DirList list;

/*********************************/
#define FOLDERNAME_LEN 80
#define DIRS_NUMBER 1//2
#define FILES_NUMBER 1//2
#define DIRS_LEVEL 1//2
#define FILES_SIZE  1*125000//*8 bytes; 50*125000 - 50Mbytes
#define MAX_DIRS_IN_ROOT_DIR 1//2

char FolderName[FOLDERNAME_LEN];
long cnt[2]={0,0};
long tmp;
unsigned char data[8];
short N=0;
/*********************************/
/*EFSL*/

/*EFSL*/
/* ****************************************************************************
* esint16 rmdir (FileSystem *fs, euint8* dir)
* Description: функция удаляет папку по имени, путем освобождения её цепочки
* кластеров, и удаления его записи из директории.
* Return value: 0 если О.К., -1 при ошибке типа файл не найден.
*/
esint16 rmdir (FileSystem *fs, euint8* dir)
{
    FileLocation loc;
    ClusterChain cache;
    euint8* buf;
    euint32 firstCluster=0;

    if((fs_findFile(fs, (eint8*)dir, &loc,0))==2)
    {
        buf=part_getSect(fs->part,loc.Sector,IOM_MODE_READWRITE);
        firstCluster = ex_getb16(buf,loc.Offset*32+20);
        firstCluster <<= 16;
        firstCluster += ex_getb16(buf,loc.Offset*32+26);
        /* Bugfix:
         * Очищая всю структуру, Вы отмечаете конец каталога.
         * If this is not the case, files that are further away cannot
         * be opened anymore by implementations that follow the spec. */
        /*memClr(buf+(loc.Offset*32),32);*/
        *(buf+(loc.Offset*32)+0) = 0xE5; /* Mark file deleted */
        part_relSect(fs->part,buf);
        cache.DiscCluster = cache.LastCluster = cache.Linear = cache.LogicCluster = 0;
        cache.FirstCluster = firstCluster;
        fat_unlinkClusterChain(fs,&cache);
        return(0);
    }
    return(-1);
}


void CreateFolderName(short Num)
{
 short i=0;



    while (Num>=0)
   {  Num-=100;
      i++;
   }

   FolderName[5] = i-1+0x30;
   i = 0;
   Num+=100;

   while (Num>=0)
   {  Num-=10;
      i++;
   }

   FolderName[6]= i-1+0x30;
   i = 0;
   Num+=10;

  FolderName[7] = Num+0x30;



}

void CreateNewFolderName(char *NewFName, short len, short Num)
{
   short i=0;

   //len = strlen(FName);

   while (Num>=0)
   {  Num-=100;
      i++;
   }

   NewFName[len+5] = i-1+0x30;
   i = 0;
   Num+=100;

   while (Num>=0)
   {  Num-=10;
      i++;
   }

   NewFName[len+6]= i-1+0x30;
   i = 0;
   Num+=10;

  NewFName[len+7] = Num+0x30;

}

void CreateFileName (char *NewFName, short len, short Num)
{
  short i=0;

   while (Num>=0)
   {  Num-=100;
      i++;
   }

   NewFName[len+5] = i-1+0x30;
   i = 0;
   Num+=100;

   while (Num>=0)
   {  Num-=10;
      i++;
   }

   NewFName[len+6]= i-1+0x30;
   i = 0;
   Num+=10;

  NewFName[len+7] = Num+0x30;

}

void CreateDir (char *FName, short Level)
{
  char NewFolderName[FOLDERNAME_LEN];
  char FileName[FOLDERNAME_LEN];
  short len;

  Level++;  if (Level>DIRS_LEVEL) {Level =0; return;}

  memset(NewFolderName,0,FOLDERNAME_LEN);
  memset(FileName,0,FOLDERNAME_LEN);

   len = strlen(FName);

   for (short i=0; i< len; i++)
   {
     NewFolderName[i] = FName[i];
     FileName[i] = FName[i];
   }

    NewFolderName[len]='/';
    NewFolderName[len+1]='F';
    NewFolderName[len+2]='O';
    NewFolderName[len+3]='L';
    NewFolderName[len+4]='D';


    FileName[len]='/';
    FileName[len+1]='f';
    FileName[len+2]='i';
    FileName[len+3]='l';
    FileName[len+4]='e';
    FileName[len+8]='.';
    FileName[len+9]='b';
    FileName[len+10]='i';
    FileName[len+11]='n';


  for (short i=0; i<DIRS_NUMBER; i++)
  {
    CreateNewFolderName(NewFolderName, len, i);
    if (mkdir(&efs.myFs, NewFolderName)!=-0) return;
  }

  for (short i=0; i<FILES_NUMBER; i++)
  {
    CreateFileName(FileName, len, i);
    if (file_fopen (&file , &efs.myFs , FileName , 'w' ) !=0) return;
    cnt[1] = 0;
    for (long i=0; i<FILES_SIZE; i++)
            {
              cnt[0] = ~cnt[1];

              data[0] = cnt[0] & 0x000000FF;
              data[1] = (cnt[0] & 0x0000FF00)>>8;
              data[2] = (cnt[0] & 0x00FF0000)>>16;
              data[3] = (cnt[0] & 0xFF000000)>>24;

              data[4] = cnt[1] & 0x000000FF;
              data[5] = (cnt[1] & 0x0000FF00)>>8;
              data[6] = (cnt[1] & 0x00FF0000)>>16;
              data[7] = (cnt[1] & 0xFF000000)>>24;

              if (file_write (&file, 8, data) != 8) {file_fclose(&file); break; }
              cnt[1]++;


            //if (IO1PIN & 0x02000000) IO1CLR |= 0x02000000;
            //else IO1SET |= 0x02000000; //P1.25

            }
         file_fclose(&file);

    if (file_fopen (&file , &efs.myFs , FileName , 'r' ) ==0)
    {

          while (file_read(&file, 8, data) == 8)
          {
            cnt[0]=0; cnt[1]=0;
            tmp = (long)data[0];
            cnt[0] |= tmp;
            tmp = (long)data[1];
            cnt[0] |= tmp<<8;
            tmp = (long)data[2];
            cnt[0] |= tmp<<16;
            tmp = (long)data[3];
            cnt[0] |= tmp<<24;

            tmp = (long)data[4];
            cnt[1] |= tmp;
            tmp = (long)data[5];
            cnt[1] |= tmp<<8;
            tmp = (long)data[6];
            cnt[1] |= tmp<<16;
            tmp = (long)data[7];
            cnt[1] |= tmp<<24;

            //if (IO1PIN & 0x02000000) IO1CLR |= 0x02000000;
            //else IO1SET |= 0x02000000; //P1.25

            if (cnt[0] != ~cnt[1])
              { //IO1SET |= 0x01000000; //P1.24; //Error!!!
                file_fclose(&file);
                break;
              }
          }
          file_fclose(&file);
    }
    else ;//IO1SET |= 0x01000000; //P1.24; //Error!!!
  }

  for (short i=0; i<DIRS_NUMBER; i++)
  {
    CreateNewFolderName(NewFolderName, len, i);

    CreateDir(NewFolderName, Level);
  }


}

void CreateDir2 (char *FName, short Level)
{
  char NewFolderName[FOLDERNAME_LEN];
  char FileName[FOLDERNAME_LEN];
  short len;
  short NUMBER;

  Level++;  if (Level>DIRS_LEVEL) {Level =0; return;}

  memset(NewFolderName,0,FOLDERNAME_LEN);
  memset(FileName,0,FOLDERNAME_LEN);

   len = strlen(FName);

   for (short i=0; i< len; i++)
   {
     NewFolderName[i] = FName[i];
     FileName[i] = FName[i];
   }

    NewFolderName[len]='/';
    NewFolderName[len+1]='F';
    NewFolderName[len+2]='O';
    NewFolderName[len+3]='L';
    NewFolderName[len+4]='D';


    FileName[len]='/';
    FileName[len+1]='f';
    FileName[len+2]='i';
    FileName[len+3]='l';
    FileName[len+4]='e';
    FileName[len+8]='.';
    FileName[len+9]='b';
    FileName[len+10]='i';
    FileName[len+11]='n';

    if (DIRS_NUMBER > FILES_NUMBER) NUMBER = DIRS_NUMBER;
    else NUMBER = FILES_NUMBER;

    for (short i=0; i<NUMBER; i++)
  {
    if (i < DIRS_NUMBER)
    {
     CreateNewFolderName(NewFolderName, len, i);
     if (mkdir(&efs.myFs, NewFolderName)!=-0) return;
    }

    if (i<FILES_NUMBER)
    {
      CreateFileName(FileName, len, i);
    if (file_fopen (&file , &efs.myFs , FileName , 'w' ) !=0) return;
    cnt[1] = 0;
    for (long i=0; i<FILES_SIZE; i++)
            {
              cnt[0] = ~cnt[1];

              data[0] = cnt[0] & 0x000000FF;
              data[1] = (cnt[0] & 0x0000FF00)>>8;
              data[2] = (cnt[0] & 0x00FF0000)>>16;
              data[3] = (cnt[0] & 0xFF000000)>>24;

              data[4] = cnt[1] & 0x000000FF;
              data[5] = (cnt[1] & 0x0000FF00)>>8;
              data[6] = (cnt[1] & 0x00FF0000)>>16;
              data[7] = (cnt[1] & 0xFF000000)>>24;

              if (file_write (&file, 8, data) != 8) {file_fclose(&file); break; }
              cnt[1]++;


            //if (IO1PIN & 0x02000000) IO1CLR |= 0x02000000;
            //else IO1SET |= 0x02000000; //P1.25

            }
         file_fclose(&file);

    if (file_fopen (&file , &efs.myFs , FileName , 'r' ) ==0)
    {

          while (file_read(&file, 8, data) == 8)
          {
            cnt[0]=0; cnt[1]=0;
            tmp = (long)data[0];
            cnt[0] |= tmp;
            tmp = (long)data[1];
            cnt[0] |= tmp<<8;
            tmp = (long)data[2];
            cnt[0] |= tmp<<16;
            tmp = (long)data[3];
            cnt[0] |= tmp<<24;

            tmp = (long)data[4];
            cnt[1] |= tmp;
            tmp = (long)data[5];
            cnt[1] |= tmp<<8;
            tmp = (long)data[6];
            cnt[1] |= tmp<<16;
            tmp = (long)data[7];
            cnt[1] |= tmp<<24;

            //if (IO1PIN & 0x02000000) IO1CLR |= 0x02000000;
            //else IO1SET |= 0x02000000; //P1.25

            if (cnt[0] != ~cnt[1])
              { //IO1SET |= 0x01000000; //P1.24; //Error!!!
                file_fclose(&file);
                break;
              }
          }
          file_fclose(&file);
    }
    else ;//IO1SET |= 0x01000000; //P1.24; //Error!!!
    }
  }

  for (short i=0; i<DIRS_NUMBER; i++)
  {
    CreateNewFolderName(NewFolderName, len, i);

    CreateDir2(NewFolderName, Level);
  }
}

void DeleteDir (char *FName)
/*Пробелы 0x20 в именах файлов и директорий не поддерживаются! 0x20 считаются концом имени*/
{
DirList list;

char NewFolderName[FOLDERNAME_LEN];
char FileName[FOLDERNAME_LEN];
short len;
short err;
short i;

//euint8 attr;
//euint8 *fileName;
//euint32 fileSize;

  memset(NewFolderName,0,FOLDERNAME_LEN);
  memset(FileName,0,FOLDERNAME_LEN);
  len = strlen(FName);
     for (short i=0; i< len; i++)
   {
     NewFolderName[i] = FName[i];
     FileName[i] = FName[i];
   }

    FileName[len] = '/';
    NewFolderName[len] = '/';


    if (ls_openDir(&list, &efs.myFs, FName)!=0) return;
    while(ls_getNext(&list)==0)
    {
      if (list.currentEntry.Attribute == 0x10) //directory
      {
        for (i=0; i<8; i++)
           {
             NewFolderName[len+1+i] = list.currentEntry.FileName[i];
             if (list.currentEntry.FileName[i]==0x20) break;
           }
             if (list.currentEntry.FileName[8]!=0x20)
               {
               NewFolderName[len+1+i] = '.';
               NewFolderName[len+1+i+1] = list.currentEntry.FileName[8];
               if (list.currentEntry.FileName[9] != 0x20) NewFolderName[len+1+i+2] = list.currentEntry.FileName[9];
               if (list.currentEntry.FileName[10] != 0x20) NewFolderName[len+1+i+3] = list.currentEntry.FileName[10];
               }
              else NewFolderName[len+1+i] = 0x00;

         DeleteDir(NewFolderName);
      }
      else //file
      {
        for (i=0; i<8; i++)
           {
             FileName[len+1+i] = list.currentEntry.FileName[i];
             if (list.currentEntry.FileName[i]==0x20) break;
           }
             if (list.currentEntry.FileName[8]!=0x20)
               {
               FileName[len+1+i] = '.';
               FileName[len+1+i+1] = list.currentEntry.FileName[8];
               if (list.currentEntry.FileName[9] != 0x20) FileName[len+1+i+2] = list.currentEntry.FileName[9];
               if (list.currentEntry.FileName[10] != 0x20) FileName[len+1+i+3] = list.currentEntry.FileName[10];
               }
              else FileName[len+1+i] = 0x00;

        err = rmfile(&efs.myFs, FileName);
        err++;
      }

    }
   rmdir(&efs.myFs, FName);

}

void Test1_2(void)
{
  memset(FolderName,0,FOLDERNAME_LEN);

    FolderName[0]='/';
    FolderName[1]='F';
    FolderName[2]='O';
    FolderName[3]='L';
    FolderName[4]='D';

    while(N<MAX_DIRS_IN_ROOT_DIR)
    {
    CreateFolderName(N);

    if (mkdir(&efs.myFs, FolderName)!=0) break;

    CreateDir(FolderName, 0);
    N++;
    }

}

int Test3(void)
{
    DeleteDir((char*)"Fold000");
    fs_umount(&(efs.myFs));

      if(efs_init(&efs,"\\")!=0)
    {
      DBG((TXT("Could not open filesystem.\n")));
      return(-1);
    }
      else
    {
    if (mkdir(&efs.myFs, (char*)"Fold000")!=0) return 0;
    CreateDir((char*)"Fold000", 0);
    }
    return(1);
}

void Test4_5(void)
{

  memset(FolderName,0,FOLDERNAME_LEN);

    FolderName[0]='/';
    FolderName[1]='F';
    FolderName[2]='O';
    FolderName[3]='L';
    FolderName[4]='D';

    while(N<MAX_DIRS_IN_ROOT_DIR)
    {
    CreateFolderName(N);

    if (mkdir(&efs.myFs, FolderName)!=0) break;

    CreateDir2(FolderName, 0);
    N++;
    }
}

int Test6(void)
{
     DeleteDir((char*)"Fold000");
     fs_umount(&(efs.myFs));

      if(efs_init(&efs,"\\")!=0)
    {
      DBG((TXT("Could not open filesystem.\n")));
      return(-1);
    }
      else
    {
    if (mkdir(&efs.myFs, (char*)"Fold000")!=0) return 0;
    CreateDir2((char*)"Fold000", 0);
    }
    return(1);
}

/*EFSL*/
#endif

/***********************************************************************
 *
 * Function: wait4cmddone
 *
 * Purpose: Sets command completion flag (callback)
 *
 * Processing:
 *     Sets the cmdresp flag to 1.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void wait4cmddone(void)
{
  cmdresp = 1;
}

/***********************************************************************
 *
 * Function: wait4datadone
 *
 * Purpose: Sets data completion flag (callback)
 *
 * Processing:
 *     Sets the datadone flag to 1.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void wait4datadone(void)
{
  datadone = 1;
}

/***********************************************************************
 *
 * Function: sdmmc_cmd_setup
 *
 * Purpose: Setup and SD/MMC command structure
 *
 * Processing:
 *     From the passed arguments, sets up the command structure for the
 *     SD_ISSUE_CMD IOCTL call to the SD card driver.
 *
 * Parameters:
 *     pcmdsetup : Pointer to command structure to fill
 *     cmd       : Command to send
 *     arg       : Argument to send
 *     resp_type : Expected response type for this command
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
static void sdmmc_cmd_setup(SD_CMD_T *pcmdsetup,
                            UNS_32 cmd,
                            UNS_32 arg,
                            SDMMC_RESPONSE_T resp_type)
{
  INT_32 tmp = 0;

  /* Determine response size */
  switch (resp_type)
  {
    case SDMMC_RESPONSE_NONE:
      tmp = 0;
      break;

    case SDMMC_RESPONSE_R1:
    case SDMMC_RESPONSE_R1B:
    case SDMMC_RESPONSE_R3:
      tmp = 48;
      break;

    case SDMMC_RESPONSE_R2:
      tmp = 136;
      break;
  }

  /* Setup SD command structure */
  pcmdsetup->cmd = cmd;
  pcmdsetup->arg = arg;
  pcmdsetup->cmd_resp_size = tmp;
}

/***********************************************************************
 *
 * Function: sdmmc_cmd_start_data
 *
 * Purpose: Read SD/MMC data blocks
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     cmd  : Pointer to command structure
 *     resp : Pointer to response structure to fill
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
static INT_32 sdmmc_cmd_start_data(SD_CMDDATA_T *cmd,
                                   SD_CMDRESP_T *resp)
{
  INT_32 status = 0;

  /* Issue command and wait for it to complete */
  datadone = 0;
  if (cmd->cmd.arg == 16386)
  {
    datadone++;
    datadone = 0;
  }
  sdcard_ioctl(sddev, SD_ISSUE_CMD, (INT_32) cmd);
  while (datadone == 0);

  /* Get the data transfer state */
  sdcard_ioctl(sddev, SD_GET_CMD_RESP, (INT_32) resp);
  if ((resp->data_status & SD_DATABLK_END) == 0)
  {
    status = -1;
  }

  return status;
}

/***********************************************************************
 *
 * Function: sdmmc_cmd_send
 *
 * Purpose: Process a SDMMC command and response (without data transfer)
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     cmd       : Command to send
 *     arg       : Argument to send
 *     resp_type : Expected response type for this command
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void sdmmc_cmd_send(SDMMC_COMMAND_T cmd,
                    UNS_32 arg,
                    SD_CMDRESP_T *resp)
{
  SD_CMDDATA_T sdcmd;

  /* Perform command setup from standard MMC command table */
  sdmmc_cmd_setup(&sdcmd.cmd, sdmmc_cmds[cmd].cmd, arg,
                  sdmmc_cmds[cmd].resp);

  /* No data for this setup */
  sdcmd.data.dataop = SD_DATAOP_NONE;

  /* Issue command and wait for it to complete */
  cmdresp = 0;
  sdcard_ioctl(sddev, SD_ISSUE_CMD, (INT_32) &sdcmd);
  while (cmdresp == 0);

  sdcard_ioctl(sddev, SD_GET_CMD_RESP, (INT_32) resp);
}

/***********************************************************************
 *
 * Function: app_cmd_send
 *
 * Purpose: Process a SD APP command and response
 *
 * Processing:
 *     See function.
 *
 * Parameters: TBD
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void app_cmd_send(SD_APP_CMD_T cmd,
                  UNS_32 arg,
                  SD_CMDRESP_T *resp)
{
  SD_CMDDATA_T sdcmd;

  /* Perform command setup from SD APP command table */
  sdmmc_cmd_setup(&sdcmd.cmd, sdmmc_app_cmds[cmd].cmd, arg,
                  sdmmc_app_cmds[cmd].resp);

  /* No data for this setup */
  sdcmd.data.dataop = SD_DATAOP_NONE;

  /* Issue command and wait for it to complete */
  cmdresp = 0;
  sdcard_ioctl(sddev, SD_ISSUE_CMD, (INT_32) &sdcmd);
  while (cmdresp == 0);

  sdcard_ioctl(sddev, SD_GET_CMD_RESP, (INT_32) resp);
}

/***********************************************************************
 *
 * Function: sdmmc_read_block
 *
 * Purpose: Read SD/MMC data blocks
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     buff    : Pointer to word aligned buffer
 *     numblks : Must be 1
 *     index   : Read offset on the card
 *     resp    : Pointer to repsonse structure to fill
 *
 * Outputs: None
 *
 * Returns: <1 on an error
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 sdmmc_read_block(UNS_32 *buff,
                        INT_32 numblks, /* Must be 1 */
                        UNS_32 index,
                        SD_CMDRESP_T *resp)
{
  SD_CMDDATA_T sdcmd;

  /* Setup read data */
  sdcmd.data.dataop = SD_DATAOP_READ;
  sdcmd.data.blocks = numblks;
  sdcmd.data.buff = buff;
  sdcmd.data.usependcmd = FALSE;
  sdcmd.data.stream = FALSE;  
  
  if (sdcardtype == 1 || sdcardtype == 2) index = index << 9; 

  //while(1){  
  /* Setup block count to data size */  
  //sdmmc_cmd_send(SDMMC_SET_BLEN, SDMMC_BLK_SIZE, resp);
  //asm("nop");
  //}  
  
  

  /* Perform command setup from standard MMC command table */
  sdmmc_cmd_setup(&sdcmd.cmd, sdmmc_cmds[SDMMC_READ_SINGLE].cmd,
                  index, sdmmc_cmds[SDMMC_READ_SINGLE].resp);

  /* Read data from the SD card */
  return sdmmc_cmd_start_data(&sdcmd, resp);
}

/***********************************************************************
 *
 * Function: sdmmc_write_block
 *
 * Purpose: Write SD/MMC data blocks
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     buff    : Pointer to word aligned buffer
 *     numblks : Must be 1
 *     index   : Read offset on the card
 *     resp    : Pointer to repsonse structure to fill
 *
 * Outputs: None
 *
 * Returns: <1 on an error
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 sdmmc_write_block(UNS_32 *buff,
                         INT_32 numblks, /* Must be 1 */
                         UNS_32 index,
                         SD_CMDRESP_T *resp)
{
  SD_CMDDATA_T sdcmd;
  INT_32 status;
  UNS_32 tmp;

  /* Setup read data */
  sdcmd.data.dataop = SD_DATAOP_WRITE;
  sdcmd.data.blocks = numblks;
  sdcmd.data.buff = buff;
  sdcmd.data.usependcmd = FALSE;
  sdcmd.data.stream = FALSE;   
  
  if (sdcardtype == 1 || sdcardtype == 2) index = index << 9; 

  /* Setup block count to data size */
  //sdmmc_cmd_send(SDMMC_SET_BLEN, SDMMC_BLK_SIZE, resp);

  /* Perform command setup from standard MMC command table */
  sdmmc_cmd_setup(&sdcmd.cmd, sdmmc_cmds[SDMMC_WRITE_SINGLE].cmd,
                  index, sdmmc_cmds[SDMMC_WRITE_SINGLE].resp);

  /* Write data to the SD card */
  status = sdmmc_cmd_start_data(&sdcmd, resp);

  /* Poll status from the device until transfer is complete */
  tmp = 0;
  while ((tmp & 0x1F00) != 0x900)
  {
    /* Currently in non-TRANS state, keep polling card until it
       returns to TRANS state */
    sdmmc_cmd_send(SDMMC_SSTAT, (rca << 16), resp);
    tmp = resp->cmd_resp [1];
  }

  return status;
}

/***********************************************************************
 *
 * Function: sdmmc_setup
 *
 * Purpose: Setup SDMMC card and controller
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: TRUE if the card was initialized ok, otherwise FALSE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 sdmmc_setup(void)
{
  INT_32 ocrtries, validocr;
  SD_CMDRESP_T resp;
  SDC_PRMS_T params;
  SDC_XFER_SETUP_T dataset;

  /* Open SD card controller driver */
  sddev = sdcard_open(SDCARD, 0);
  if (sddev == 0)
  {
    return FALSE;
  }

  /* Setup controller parameters */
  params.opendrain = TRUE;
  params.powermode = SD_POWER_ON_MODE;
  params.pullup0 = 1;
  params.pullup1 = 1;
  params.pullup23 = 1;
  params.pwrsave = FALSE;
  params.sdclk_rate = SDMMC_OCR_CLOCK;
  params.use_wide = FALSE;
  if (sdcard_ioctl(sddev, SD_SETUP_PARAMS,
                   (INT_32) &params) == _ERROR)
  {
    return FALSE;
  }

  /* Setup data transfer paramaters */
  dataset.data_callback = (PFV) wait4datadone;
  dataset.cmd_callback = (PFV) wait4cmddone;
  dataset.blocksize = SDMMC_BLK_SIZE;
  dataset.data_to = 0x001FFFFF; /* Long timeout for slow MMC cards */
  dataset.use_dma = FALSE;
  sdcard_ioctl(sddev, SD_SETUP_DATA_XFER, (INT_32) &dataset);

  /* Issue IDLE command */
  sdmmc_cmd_send(SDMMC_IDLE, 0, &resp);

  /* After the IDLE command, a small wait is needed to allow the cards
     to initialize */
  timer_wait_ms(TIMER_CNTR0, 100);
  
  /*modification*/
  
  /*CMD8*/  
  sdmmc_cmd_send(SDMMC_SEND_IF_COND, 0x01aa, &resp);
  if ((resp.cmd_status & SD_CMD_RESP_RECEIVED) == 0)
        {
          sdcardtype = 1; //no response from card: may be sd 1.0
        }    
  else 
  {
  
  if ((resp.cmd_resp[0] & 0x04 ) != 0) //R1_ILLEGAL_COMMAND
  {
    sdcardtype = 1; //response from card, sd 1.0
  } 
   else 
   {
    if (resp.cmd_resp[1] != 0x000001AA)
    
      {
        return FALSE; //init failed
      }
    sdcardtype = 2; //sd 2.0
   } 
  }
  
  switch (sdcardtype)
  {
  case 1: 
    sdmmc_cmd_send(SDMMC_READ_OCR, 0, &resp);
    
    ocrtries = SDMMC_MAX_OCR_RET;
    validocr = 0;
    
    resp.cmd_resp [1] = 0;
    while ((validocr == 0) && (ocrtries >= 0))
    {
      /* SD card init sequence */
      app_cmd_send(SD_SENDOP_COND, OCRVAL, &resp);
      if ((resp.cmd_resp [1] & SDMMC_OCR_MASK) == 0)
      {
        /* Response received and busy, so try again */
        sdmmc_cmd_send(SDMMC_APP_CMD, 0, &resp);
      }
      else
      {
        validocr = 1;
      }

      ocrtries--;
    }

    if (validocr == 0)
    {
      return FALSE;
    }
    
    break; 
    
  case 2:
    
    ocrtries = SDMMC_MAX_OCR_RET;
    validocr = 0;
    
    resp.cmd_resp [1] = 0;
    while ((validocr == 0) && (ocrtries >= 0))
    {
      /* SD card init sequence */
      app_cmd_send(SD_SENDOP_COND, OCRVAL | 0x40000000, &resp); // Enable HCS Flag
      if ((resp.cmd_resp [1] & SDMMC_OCR_MASK) == 0)
      {
        /* Response received and busy, so try again */
        sdmmc_cmd_send(SDMMC_APP_CMD, 0 , &resp); 
      }
      else
      {
        validocr = 1;
      }

      ocrtries--;
    }

      if (validocr == 0)
    {
      return FALSE;
    }
    
    sdmmc_cmd_send(SDMMC_READ_OCR, 0, &resp);
    if ((resp.cmd_resp [1] & 0x40000000) != 0 && (resp.cmd_resp [1] & 0x80000000) != 0)  
                                          // Check "Card Capacity Status (CCS)", bit 30 which is only valid
    {                                     // if the "Card power up status bit", bit 31 is set. 
      
      sdcardtype = 3; //sd 2.0 HC
    }
    
    
    
    break;     
  }
  
  /*modification*/
    

 /*
  // Issue APP command, only SD cards will respond to this 
  sdcardtype = 0;
  sdmmc_cmd_send(SDMMC_APP_CMD, 0, &resp);
  if ((resp.cmd_status & SD_CMD_RESP_RECEIVED) != 0)
  {
    sdcardtype = 1;
  }

  ocrtries = SDMMC_MAX_OCR_RET;
  validocr = 0;

  // If this is an SD card, use the SD sendop command 
  if (sdcardtype == 1)
  {
    
    resp.cmd_resp [1] = 0;
    while ((validocr == 0) && (ocrtries >= 0))
    {
      // SD card init sequence 
      app_cmd_send(SD_SENDOP_COND, OCRVAL, &resp);
      if ((resp.cmd_resp [1] & SDMMC_OCR_MASK) == 0)
      {
        // Response received and busy, so try again 
        sdmmc_cmd_send(SDMMC_APP_CMD, 0, &resp);
      }
      else
      {
        validocr = 1;
      }

      ocrtries--;
    }

    if (validocr == 0)
    {
      return FALSE;
    }
  */
  
    sdmmc_cmd_send(SDMMC_SET_BLEN, SDMMC_BLK_SIZE, &resp);
    /* Enter push-pull mode and switch to fast clock */
    params.opendrain = FALSE;
    params.sdclk_rate = SD_NORM_CLOCK;
    sdcard_ioctl(sddev, SD_SETUP_PARAMS, (INT_32) &params);

    /* Get CID */
    sdmmc_cmd_send(SDMMC_ALL_SEND_CID, 0, &resp);

    /* Get relative card address */
    sdmmc_cmd_send(SDMMC_SRA, 0, &resp);
    rca = (resp.cmd_resp [1] >> 16) & 0xFFFF;

    /* Select card (required for bus width change) */
    sdmmc_cmd_send(SDMMC_SELECT_CARD, (rca << 16), &resp);

    /* Set bus width to 4 bits */
    sdmmc_cmd_send(SDMMC_APP_CMD, (rca << 16), &resp);
    app_cmd_send(SD_SET_BUS_WIDTH, 2, &resp);

    /* Switch controller to 4-bit data bus */
    params.use_wide = TRUE;
    sdcard_ioctl(sddev, SD_SETUP_PARAMS, (INT_32) &params);
    
  /* }
  else
    
  {
    resp.cmd_resp [1] = 0;
    while ((validocr == 0) && (ocrtries >= 0))
    {
      // MMC card init sequence 
      sdmmc_cmd_send(MMC_SENDOP_COND, OCRVAL, &resp);
      if ((resp.cmd_resp [1] & SDMMC_OCR_MASK) != 0)
      {
        validocr = 1;
      }

      ocrtries--;
    }

    if (validocr == 0)
    {
      return FALSE;
    }

    // Enter push-pull mode and switch to fast clock 
    params.opendrain = FALSE;
    params.sdclk_rate = MMC_NORM_CLOCK;
    sdcard_ioctl(sddev, SD_SETUP_PARAMS, (INT_32) &params);

    // Get CID 
    sdmmc_cmd_send(SDMMC_ALL_SEND_CID, 0, &resp);

    // Get relative card address 
    rca = 0x1234;
    sdmmc_cmd_send(SDMMC_SRA, (rca << 16), &resp);
  }
*/

  /* Deselect card */
  sdmmc_cmd_send(SDMMC_SELECT_CARD, 0, &resp);

  /* Status request */
  sdmmc_cmd_send(SDMMC_SSTAT, (rca << 16), &resp);

  /* Select card */
  sdmmc_cmd_send(SDMMC_SELECT_CARD, (rca << 16), &resp);
  
  

  return TRUE;
}

/***********************************************************************
 *
 * Function: sdmmc_close
 *
 * Purpose: Close SDMMC card and controller
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void sdmmc_close(void)
{
  sdcard_close(sddev);
}

/***********************************************************************
 *
 * Function: c_entry
 *
 * Purpose: Application entry point
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Always returns 1
 *
 * Notes: None
 *
 **********************************************************************/
static uint8_t d[1024];
void _sdcard(void)
{
  UNS_32 blk, sblk;
  SD_CMDRESP_T resp;
  UNS_32 *dbuff;
  char device;

  disable_irq();

  /* Enable SDMMC power */
  phy3250_sdpower_enable(TRUE);
#if 0
  /* Exit if no SDMMC card installed */
  if (phy3250_sdmmc_card_inserted() == FALSE)
  {
    return;
  }
#endif
 
//efsl sdcard 

  enable_irq();
   

  /* Setup SDMMC card and controller */
  if (sdmmc_setup() == FALSE)
  {
    return;
  }
  
  

  

  
}
