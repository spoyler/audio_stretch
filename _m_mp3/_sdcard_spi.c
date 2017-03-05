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
#include "lpc32xx_gpio_driver.h"
#include "lpc32xx_spi_driver.h"
#include "lpc32xx_clkpwr.h"


#define GO_IDLE_STATE 0 //ѕрограммна€ перезагрузка 
#define SEND_IF_COND 8 //ƒл€ SDC V2 - проверка диапазона напр€жений 
#define READ_SINGLE_BLOCK 17 //„тение указанного блока данных 
#define WRITE_SINGLE_BLOCK 24 //«апись указанного блока данных
#define SD_SEND_OP_COND 41 //Ќачало процесса инициализации 
#define APP_CMD 55 //√лавна€ команда из ACMD  команд
#define READ_OCR 58 //„тение регистра OCR

static uint8_t  SDHC;
static INT_32 spidev;

/* SPI device configuration */
static SPI_CONFIG_T spicfg;

static UNS_8 _spi_read(void){
UNS_8 response;
UNS_8 data;

    spi_ioctl(spidev, SPI_DRIVE_SSEL, 0);//CS_DISABLE; 
    spi_write(spidev, &data, 1);                      //dummy write
    spi_ioctl(spidev, SPI_DELAY, 2);
    spi_ioctl(spidev, SPI_DRIVE_SSEL, 1);//CS_DISABLE; 
    spi_read(spidev, &response, 1);                       //read the data
    spi_ioctl(spidev, SPI_DELAY, 1);
    
    return response;
}

static uint8_t SD_sendCommand(uint8_t cmd, uint32_t arg)
{
  uint8_t response, wait=0, tmp; 
  UNS_8 data;
 
  //дл€ карт пам€ти SD выполнить коррекцию адреса, т.к. дл€ них адресаци€ побайтна€ 
  if(SDHC == 0)		
  if(cmd == READ_SINGLE_BLOCK || cmd == WRITE_SINGLE_BLOCK )  
    arg = arg << 9;
  //дл€ SDHC коррекцию адреса блока выполн€ть не нужно(постранична€ адресаци€)	

  spicfg.transmitter = TRUE;                        //SPI1 is a Tx
  spi_ioctl(spidev, SPI_TXRX, (INT_32) & spicfg);  
  spi_ioctl(spidev, SPI_DRIVE_SSEL, 0);//CS_ENABLE;
  
  
  //передать код команды и ее аргумент
  data = (cmd | 0x40); 
  spi_write(spidev, &data, 1); //spi_send(cmd | 0x40);
  spi_ioctl(spidev, SPI_DELAY, 2);
  data = arg>>24;
  spi_write(spidev, &data, 1);//spi_send(arg&gt;&gt;24);
  spi_ioctl(spidev, SPI_DELAY, 2);
  data = (arg>>16);
  spi_write(spidev, &data, 1);//spi_send(arg&gt;&gt;16);
  spi_ioctl(spidev, SPI_DELAY, 2);
  data = (arg>>8);
  spi_write(spidev, &data, 1);//spi_send(arg&gt;&gt;8);
  spi_ioctl(spidev, SPI_DELAY, 2);
  data = arg;
  spi_write(spidev, &data, 1);//spi_send(arg);
  spi_ioctl(spidev, SPI_DELAY, 2);
 
  //передать CRC (учитываем только дл€ двух команд)
  if(cmd == SEND_IF_COND) {  
#if 1 
    data = 0x0;
    spi_write(spidev, &data, 1);
    spi_ioctl(spidev, SPI_DELAY, 2);
#endif     
    data = 0x0f;
    spi_write(spidev, &data, 1);//spi_send(0x87);
    spi_ioctl(spidev, SPI_DELAY, 2);
  }    
  else  {
#if 1
      data = 0x0;
      spi_write(spidev, &data, 1);
      spi_ioctl(spidev, SPI_DELAY, 2);
#endif 
    data = 0x95;
    spi_write(spidev, &data, 1);//spi_send(0x95);
    spi_ioctl(spidev, SPI_DELAY, 2);
  }
  spi_ioctl(spidev, SPI_CLEAR_RX_BUFFER, 0);  

  spicfg.transmitter = FALSE;                       //SPI1 is a Rx
  spi_ioctl(spidev, SPI_TXRX, (INT_32) & spicfg);

  response = 0xff;
  while(response == 0xff){
    response=_spi_read();
  }
 
  //проверка ответа если посылалась команда READ_OCR
  if(response == 0x01 && cmd == 58)     
  {
    tmp = _spi_read();                      //прочитать один байт регистра OCR            
    if(tmp & 0x40) SDHC = 1;               //обнаружена карта SDHC 
    else           SDHC = 0;               //обнаружена карта SD
    //прочитать три оставшихс€ байта регистра OCR
    _spi_read(); 
    _spi_read(); 
    _spi_read(); 
  }
 
  _spi_read();
 
  spi_ioctl(spidev, SPI_DRIVE_SSEL, 1);//CS_DISABLE; 
 
  return response;
}

static uint8_t SD_sendCommand_fake(uint8_t cmd, uint32_t arg)
{
  uint8_t response, wait=0, tmp; 
  UNS_8 data;
 
  //дл€ карт пам€ти SD выполнить коррекцию адреса, т.к. дл€ них адресаци€ побайтна€ 
  if(SDHC == 0)		
  if(cmd == READ_SINGLE_BLOCK || cmd == WRITE_SINGLE_BLOCK )  
    arg = arg << 9;
  //дл€ SDHC коррекцию адреса блока выполн€ть не нужно(постранична€ адресаци€)	

  spicfg.transmitter = TRUE;                        //SPI1 is a Tx
  spi_ioctl(spidev, SPI_TXRX, (INT_32) & spicfg);  
  spi_ioctl(spidev, SPI_DRIVE_SSEL, 0);//CS_ENABLE;
  
  
  //передать код команды и ее аргумент
  data = (cmd | 0x40); 
  spi_write(spidev, &data, 1); //spi_send(cmd | 0x40);
  spi_ioctl(spidev, SPI_DELAY, 2);
  data = arg>>24;
  spi_write(spidev, &data, 1);//spi_send(arg&gt;&gt;24);
  spi_ioctl(spidev, SPI_DELAY, 2);
  data = (arg>>16);
  spi_write(spidev, &data, 1);//spi_send(arg&gt;&gt;16);
  spi_ioctl(spidev, SPI_DELAY, 2);
  data = (arg>>8);
  spi_write(spidev, &data, 1);//spi_send(arg&gt;&gt;8);
  spi_ioctl(spidev, SPI_DELAY, 2);
  data = arg;
  spi_write(spidev, &data, 1);//spi_send(arg);
  spi_ioctl(spidev, SPI_DELAY, 2);
 

  data = 0x95;
  spi_write(spidev, &data, 1);//spi_send(0x95);
  spi_ioctl(spidev, SPI_DELAY, 2);  
  spi_ioctl(spidev, SPI_CLEAR_RX_BUFFER, 0);  

  spicfg.transmitter = FALSE;                       //SPI1 is a Rx
  spi_ioctl(spidev, SPI_TXRX, (INT_32) & spicfg);

  response = 0xff;
  while(response == 0xff){
    response=_spi_read();
  }
 
 
  _spi_read();
 
  spi_ioctl(spidev, SPI_DRIVE_SSEL, 1);//CS_DISABLE; 
 
  return response;
}


static uint8_t SD_init(void)
{
  uint8_t   i;
  uint8_t   response;
  uint8_t   SD_version = 2;	          //по умолчанию верси€ SD = 2
  uint16_t  retry = 0 ;
  unsigned char data;
 
  
  
  spi_ioctl(spidev, SPI_DRIVE_SSEL, 1);//CS_DISABLE;
  data = 0xff;
  spicfg.transmitter = TRUE;                        //SPI1 is a Tx
  spi_ioctl(spidev, SPI_TXRX, (INT_32) & spicfg);    
  for(i=0;i<10;i++) 
    spi_write(spidev, &data, 1);
  spi_ioctl(spidev, SPI_CLEAR_RX_BUFFER, 0);
  
  SD_sendCommand_fake(GO_IDLE_STATE, 0);
  spicfg.transmitter = TRUE;                        //SPI1 is a Tx
  spi_ioctl(spidev, SPI_TXRX, (INT_32) & spicfg);  
  spi_write(spidev, &data, 1); //send 0xff
  spi_write(spidev, &data, 1); //send 0xff
  spi_ioctl(spidev, SPI_CLEAR_RX_BUFFER, 0);
  
#if 1
  //spi_ioctl(spidev, SPI_DRIVE_SSEL, 0);
  while(SD_sendCommand(GO_IDLE_STATE, 0)!=0x01)                                   
    if(retry++>0x20)  return 1;
#endif  
  //spi_ioctl(spidev, SPI_DRIVE_SSEL, 1);

  
  spicfg.transmitter = TRUE;                        //SPI1 is a Tx
  spi_ioctl(spidev, SPI_TXRX, (INT_32) & spicfg);  
  spi_write(spidev, &data, 1); //send 0xff
  spi_write(spidev, &data, 1); //send 0xff
  spi_ioctl(spidev, SPI_CLEAR_RX_BUFFER, 0);
  
#if 0 
  retry = 0;                                     
  while(SD_sendCommand(SEND_IF_COND,0x000001AA)!=0x01)
  { 
    if(retry++>0xfe) 
    { 
      SD_version = 1;
      break;
    } 
  }
 
 retry = 0;                                     
 //do
 //{
   response = SD_sendCommand(APP_CMD,0); 
   response = SD_sendCommand(SD_SEND_OP_COND,0x40000000);
   retry++;
   //if(retry>0xffe) return 1;                     
 //}while(response != 0x00);                      
#endif 
 
 //читаем регистр OCR, чтобы определить тип карты
 retry = 0;
 SDHC = 0;
 if (SD_version == 2)
 { 
   //while(SD_sendCommand(READ_OCR,0)!=0x01)
	// if(retry++>0xfe)  break;
   SD_sendCommand(READ_OCR,0);
 }
 
 return 0; 
}


uint8_t SD_ReadSector(uint32_t BlockNumb,uint8_t *buff)
{ 
  uint16_t i=0;
 
  //послать команду "чтение одного блока" с указанием его номера
  //if(SD_sendCommand(READ_SINGLE_BLOCK, BlockNumb)) return 1;
  SD_sendCommand(READ_SINGLE_BLOCK, BlockNumb);  
  spi_ioctl(spidev, SPI_DRIVE_SSEL, 0);//CS_ENABLE;
  //ожидание  маркера данных
  while(_spi_read() != 0xfe)                
  if(i++ > 0xfffe) {
    spi_ioctl(spidev, SPI_DRIVE_SSEL, 1);//CS_DISABLE; 
    return 1;
  }       
 
  //чтение 512 байт	выбранного сектора
  for(i=0; i<512; i++) 
    *buff++ = _spi_read();
    
  spi_ioctl(spidev, SPI_DRIVE_SSEL, 1);//CS_DISABLE;
 
  return 0;
}



static void _spi_init(void){
 /* The SPI can not control the SSEL signal, it will be driven
     by write/read routines as needed and accessed as an output
     GPIO pin. This EEPROM can be use with SPI mode 3 when the
     chip select must remain asserted for the entire cycle.     */
  GPIO->p2_mux_clr = P2_GPIO05_SSEL0;
  GPIO->p2_dir_set = P2_DIR_GPIO(5);
  GPIO->p3_outp_set = P3_STATE_GPIO(5);

  /* Select SPI1 pins */
  GPIO->p_mux_clr = P_SPI1DATAIO_SSP0_MOSI |
                    P_SPI1DATAIN_SSP0_MISO | P_SPI1CLK_SCK0;
  CLKPWR->clkpwr_spi_clk_ctrl |= CLKPWR_SPICLK_USE_SPI1;

  /* Open SPI */
  spicfg.databits = 8;
  spicfg.highclk_spi_frames = TRUE;
  spicfg.usesecond_clk_spi = TRUE;
  spicfg.spi_clk = 1000000;
  spicfg.msb = FALSE;
  spicfg.transmitter = TRUE;
  spicfg.busy_halt = FALSE;
  spicfg.unidirectional = TRUE;
  spidev = spi_open(SPI1, (INT_32) & spicfg);
  if (spidev == 0)
  {
    /* Error */
    asm("nop");
    return;
  }
  spi_ioctl(spidev, SPI_ENABLE, 1);

  /* Enable IRQ interrupts in the ARM core */
  enable_irq();
  
}

static uint8_t Buff[512];                  //буфер дл€ чтени€ и записи сектора карты
void _sdcard_spi(void)
{
UNS_8 data[3];


  _spi_init();
 
  SD_init();                          //выполнить инициализацию
  SD_ReadSector(1, Buff);            //копировать сектор є2 в буфер  

 /* Disable interrupts in ARM core */
  disable_irq_fiq();

  /* Close SPI */
  spi_close(spidev);  

  asm("nop");
  return;

}
