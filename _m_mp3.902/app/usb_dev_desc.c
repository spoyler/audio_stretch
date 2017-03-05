/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2008
 *
 *    File name   : usb_desc.c
 *    Description : USB descriptors module
 *
 *    History :
 *    1. Date        : 31, October 2008
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: 30870 $
 **************************************************************************/
#include "usb_dev_desc.h"

#pragma data_alignment=4
const UsbStandardDeviceDescriptorStr_t UsbStandardDeviceDescriptorStr =
{
  sizeof(UsbStandardDeviceDescriptorStr_t),
  UsbDescriptorDevice,
  0x0110,
  0,
  0,
  0,
  Ep0MaxSize,
  0xFFFF,
  0x002A,
  0x0000,
  iManufacturerStr,
  iProductStr,
  iSerialNumberStr,
  1,
};

#pragma data_alignment=4
static const Int8U UsbFS_Cfg0[] =
{
  // Configuration Descriptor
  sizeof(UsbStandardConfigurationDescriptor_t),
  UsbDescriptorConfiguration,
  _2BL(sizeof(UsbStandardConfigurationDescriptor_t) +
       /* Interface 0 */
       sizeof(UsbStandardInterfaceDescriptor_t) +
       sizeof(UsbClassSpecificACInterfaceDescriptor_t) +
       sizeof(UsbInputTerminalDescriptor_t) +
       sizeof(UsbOutputTerminalDescriptor_t)+
       sizeof(ClassSpecFeatureUnitDescriptor_t) + 3 +
       /* Interface 1 */
       sizeof(UsbStandardInterfaceDescriptor_t) +
       sizeof(UsbStandardInterfaceDescriptor_t) +
       sizeof(UsbClassSpecificASInterfaceDescriptor_t) +
       sizeof(UsbTypeIFormatTypeDescriptor_t) + 3 +
       sizeof(StandardASEndpointDescriptor_t) +
       sizeof(ClassSpecASEndpointDescriptor_t)
  ),
  2,
  1,
  0,
  UsbConfigurationCommmonAttr,
  UsbConfigPower_mA(100),

  // Interface 0 Standard AC Interface Descriptor
  sizeof(UsbStandardInterfaceDescriptor_t),
  UsbDescriptorInterface,
  USB_CTRL_INTERFACE,
  0,
  0,
  UsbDeviceClassAudio,
  AUDIOCONTROL,
  PR_PROTOCOL_UNDEFINED,
  0,

  // Class-Specific AC Interface Descriptor
  sizeof(UsbClassSpecificACInterfaceDescriptor_t),
  CS_INTERFACE,
  HEADER,
  _2BL(0x0100),
  _2BL(sizeof(UsbClassSpecificACInterfaceDescriptor_t) +
       sizeof(UsbInputTerminalDescriptor_t) +
       sizeof(UsbOutputTerminalDescriptor_t)+
       sizeof(ClassSpecFeatureUnitDescriptor_t) + 3),
  1,
  1,

  // Input Terminal Descriptor - SPK
  sizeof(UsbInputTerminalDescriptor_t),
  CS_INTERFACE,
  INPUT_TERMINAL,
  SpkInTermID,
  _2BL(AudioUsbStreaming),
  0,
  1,
  _2BL(AUDIO_CHANNEL_M),
  0,
  0,

  // Output Terminal Descriptor - SPK
  sizeof(UsbOutputTerminalDescriptor_t),
  CS_INTERFACE,
  OUTPUT_TERMINAL,
  SpkOutTermID,
  _2BL(AudioOutputSpk),
  0,
  FeatUnit1Id,
  0,

  // Feature Unit Descriptor - SPK
  sizeof(ClassSpecFeatureUnitDescriptor_t) + 3,
  CS_INTERFACE,
  FEATURE_UNIT,
  FeatUnit1Id,
  SpkInTermID,
  1,
  FeatUnitMute+FeatUnitVolume,
  0,
  0,

  // Interface 1
  // AudioStreaming Interface Descriptor
  // Zero-bandwidth Alternate Setting 0
  // Standard AS Interface Descriptor
  sizeof(UsbStandardInterfaceDescriptor_t),
  UsbDescriptorInterface,
  USB_SPK_INTERFACE,
  0,
  0,
  UsbDeviceClassAudio,
  AUDIOSTREAMING,
  PR_PROTOCOL_UNDEFINED,
  0,

  // Operational Alternate Setting 1
  // Standard AS Interface Descriptor
  sizeof(UsbStandardInterfaceDescriptor_t),
  UsbDescriptorInterface,
  1,
  1,
  1,
  UsbDeviceClassAudio,
  AUDIOSTREAMING,
  PR_PROTOCOL_UNDEFINED,
  0,

  // Class-Specific AS General Interface Descriptor
  sizeof(UsbClassSpecificASInterfaceDescriptor_t),
  CS_INTERFACE,
  AS_GENERAL,
  SpkInTermID,
  2,
  _2BL(PCM),

  // USB Spk Type I Format Type Descriptor
  sizeof(UsbTypeIFormatTypeDescriptor_t)+ 3,
  CS_INTERFACE,
  FORMAT_TYPE,
  FORMAT_TYPE_I,
  1,
  SubFrameSize,
  8*SubFrameSize,
  1,
  (Int8U)(SampFreq),(Int8U)(SampFreq>>8),(Int8U)(SampFreq>>16),

  // USB SPK Standard Endpoint Descriptor
  sizeof(StandardASEndpointDescriptor_t),
  UsbDescriptorEp,
  UsbEpOut(SpkEp>>1),
  (Int8U)UsbEpTransferIsochronous + (Int8U)UsbEpSynchAdaptive,
  _2BL(SpkEpMaxSize),
  1,
  0,
  0,

  // USB SPK Class-Specific Isoc. Audio Data Endpoint Descriptor
  sizeof(ClassSpecASEndpointDescriptor_t),
  CS_ENDPOINT,
  EP_GENERAL,
  0,
  0,
  _2BL(0),

  0,
};

static const Int8U * const UsbFS_CfgArray[] =
{
  UsbFS_Cfg0,
  NULL,
};

static const UsbEP_ExtDesc_t UsbEPExt_Cfg0EP1 =
{
  1,                  // Configuration
  UsbEpOut(SpkEp>>1),  // EP address
  {
    TRUE
  }
};

static const UsbEP_ExtDesc_t * const UsbFS_EPExtArray[] =
{
  &UsbEPExt_Cfg0EP1,
  NULL,
};

#pragma data_alignment=4
const Int8U UsbLanguagesStr [] =
{
  // Length of LanguagesStr + 2
  6,
  // Descriptor Type
  UsbDescriptorString,
  // Languages ID
  // Languages1 ID English
  0x09,0x04,
  // Languages2 ID Bulgarian
  0x02,0x04
};

#pragma data_alignment=4
const Int8U ManufacturerStrLan1 [] =
{
  24, // length
  UsbDescriptorString,  // Descriptor
  'I',0,'A',0,'R',0,' ',0,'S',0,'y',0,'s',0,'t',0,'e',0,'m',0,'s',0, // Str
};

#pragma data_alignment=4
const Int8U ManufacturerStrLan2 [] =
{
  24, //length
  UsbDescriptorString, // Descriptor
  'È',0,'À',0,'Ð',0,' ',0,'Ñ',0,'è',0,'ñ',0,'ò',0,'å',0,'ì',0,'è',0 // Str
};

#pragma data_alignment=4
const Int8U ProductStrLan1 [] =
{
  28, //length
  UsbDescriptorString, // Descriptor
  'I',0,'A',0,'R',0,' ',0,'L',0,'P',0,'C',0,'-',0,'2',0,'4',0,'7',0,'8',0// Str
};

#pragma data_alignment=4
const Int8U ProductStrLan2 [] =
{
  28, //length
  UsbDescriptorString, // Descriptor
  'È',0,'À',0,'Ð',0,' ',0,'Ë',0,'Ï',0,'Ö',0,'-',0,'2',0,'4',0,'7',0,'8',0// Str
};

#pragma data_alignment=4
const Int8U SerialNumberStrLan1 [] =
{
  18, //length
  UsbDescriptorString, // Descriptor
  '0',0,'0',0,'0',0,'0',0,'0',0,'0',0,'0',0,'1',0, // Str
};

const Int8U * const UsbLanguages1Strings[] =
{
  // iManufacturerStr
  ManufacturerStrLan1,
  // iProductStr
  ProductStrLan1,
  // iSerialNumberStr
  SerialNumberStrLan1,
  // Terminator
  NULL
};

const Int8U * const UsbLanguages2Strings[] =
{
  // iManufacturerStr
  ManufacturerStrLan2,
  // iProductStr
  ProductStrLan2,
  // iSerialNumberStr
  SerialNumberStrLan1,
  // Terminator
  NULL
};

const Int8U * const * const UsbFS_StringArray[] =
{
  UsbLanguages1Strings,
  UsbLanguages2Strings,
  NULL,
};


#if USB_HIGH_SPEED > 0
#pragma data_alignment=4
const UsbStandardDeviceQualifierDescriptor_t UsbQualifierDescriptor =
{
  sizeof(UsbStandardDeviceQualifierDescriptor_t),
  UsbDescriptorDeviceQualifier,
  0x200,
  0,
  0,
  0,
  Ep0MaxSize,
  1,
  0
};

#pragma data_alignment=4
static const Int8U UsbFS_OtherCfg0[] =
{
  // Configuration Descriptor
  sizeof(UsbStandardConfigurationDescriptor_t),
  UsbDescriptorConfiguration,
  _2BL(sizeof(UsbStandardConfigurationDescriptor_t) +
       /* Interface 0 */
       sizeof(UsbStandardInterfaceDescriptor_t) +
       sizeof(UsbClassSpecificACInterfaceDescriptor_t) +
       sizeof(UsbInputTerminalDescriptor_t) +
       sizeof(UsbOutputTerminalDescriptor_t)+
       sizeof(ClassSpecFeatureUnitDescriptor_t) + 3 +
       /* Interface 1 */
       sizeof(UsbStandardInterfaceDescriptor_t) +
       sizeof(UsbStandardInterfaceDescriptor_t) +
       sizeof(UsbClassSpecificASInterfaceDescriptor_t) +
       sizeof(UsbTypeIFormatTypeDescriptor_t) + 3 +
       sizeof(StandardASEndpointDescriptor_t) +
       sizeof(ClassSpecASEndpointDescriptor_t)
  ),
  2,
  1,
  0,
  UsbConfigurationCommmonAttr,
  UsbConfigPower_mA(100),

  // Interface 0 Standard AC Interface Descriptor
  sizeof(UsbStandardInterfaceDescriptor_t),
  UsbDescriptorInterface,
  USB_CTRL_INTERFACE,
  0,
  0,
  UsbDeviceClassAudio,
  AUDIOCONTROL,
  PR_PROTOCOL_UNDEFINED,
  0,

  // Class-Specific AC Interface Descriptor
  sizeof(UsbClassSpecificACInterfaceDescriptor_t),
  CS_INTERFACE,
  HEADER,
  _2BL(0x0100),
  _2BL(sizeof(UsbClassSpecificACInterfaceDescriptor_t) +
       sizeof(UsbInputTerminalDescriptor_t) +
       sizeof(UsbOutputTerminalDescriptor_t)+
       sizeof(ClassSpecFeatureUnitDescriptor_t) + 3),
  1,
  1,

  // Input Terminal Descriptor - SPK
  sizeof(UsbInputTerminalDescriptor_t),
  CS_INTERFACE,
  INPUT_TERMINAL,
  SpkInTermID,
  _2BL(AudioUsbStreaming),
  0,
  1,
  _2BL(AUDIO_CHANNEL_M),
  0,
  0,

  // Output Terminal Descriptor - SPK
  sizeof(UsbOutputTerminalDescriptor_t),
  CS_INTERFACE,
  OUTPUT_TERMINAL,
  SpkOutTermID,
  _2BL(AudioOutputSpk),
  0,
  FeatUnit1Id,
  0,

  // Feature Unit Descriptor - SPK
  sizeof(ClassSpecFeatureUnitDescriptor_t) + 3,
  CS_INTERFACE,
  FEATURE_UNIT,
  FeatUnit1Id,
  SpkInTermID,
  1,
  FeatUnitMute+FeatUnitVolume,
  0,
  0,

  // Interface 1
  // AudioStreaming Interface Descriptor
  // Zero-bandwidth Alternate Setting 0
  // Standard AS Interface Descriptor
  sizeof(UsbStandardInterfaceDescriptor_t),
  UsbDescriptorInterface,
  USB_SPK_INTERFACE,
  0,
  0,
  UsbDeviceClassAudio,
  AUDIOSTREAMING,
  PR_PROTOCOL_UNDEFINED,
  0,

  // Operational Alternate Setting 1
  // Standard AS Interface Descriptor
  sizeof(UsbStandardInterfaceDescriptor_t),
  UsbDescriptorInterface,
  1,
  1,
  1,
  UsbDeviceClassAudio,
  AUDIOSTREAMING,
  PR_PROTOCOL_UNDEFINED,
  0,

  // Class-Specific AS General Interface Descriptor
  sizeof(UsbClassSpecificASInterfaceDescriptor_t),
  CS_INTERFACE,
  AS_GENERAL,
  SpkInTermID,
  2,
  _2BL(PCM),

  // USB Spk Type I Format Type Descriptor
  sizeof(UsbTypeIFormatTypeDescriptor_t)+ 3,
  CS_INTERFACE,
  FORMAT_TYPE,
  FORMAT_TYPE_I,
  1,
  SubFrameSize,
  8*SubFrameSize,
  1,
  (Int8U)(SampFreq),(Int8U)(SampFreq>>8),(Int8U)(SampFreq>>16),

  // USB SPK Standard Endpoint Descriptor
  sizeof(StandardASEndpointDescriptor_t),
  UsbDescriptorEp,
  UsbEpOut(SpkEp>>1),
  (Int8U)UsbEpTransferIsochronous + (Int8U)UsbEpSynchAdaptive,
  _2BL(SpkEpMaxSize),
  1,
  0,
  0,

  // USB SPK Class-Specific Isoc. Audio Data Endpoint Descriptor
  sizeof(ClassSpecASEndpointDescriptor_t),
  CS_ENDPOINT,
  EP_GENERAL,
  0,
  0,
  _2BL(0),

  0,
};

static const Int8U * const UsbFS_OtherCfgArray[] =
{
  UsbFS_OtherCfg0,
  NULL,
};
#endif // USB_HIGH_SPEED > 0

const void * const UsbDescArray[] =
{
  UsbFS_CfgArray,
  UsbFS_EPExtArray,
  UsbLanguagesStr,
  UsbFS_StringArray,
#if USB_HIGH_SPEED > 0
  UsbFS_OtherCfgArray,
  UsbFS_CfgArray,
  UsbFS_EPExtArray,
  UsbLanguagesStr,
  UsbFS_StringArray,    // Full and High speed device have same strings descriptors
  UsbFS_OtherCfgArray,
#endif // USB_HIGH_SPEED > 0
  NULL,
};
