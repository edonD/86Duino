#ifndef __DMP_USB_DESC_H
#define __DMP_USB_DESC_H

#include "dmpcfg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char  BYTE;
typedef unsigned short WORD;



// -----------------------------------
//      USB Standard Descriptors
// -----------------------------------
//
// Device Descriptor
#ifdef DMP_DOS_DJGPP
    #ifdef HANDLE_PRAGMA_PACK_PUSH_POP
        #undef HANDLE_PRAGMA_PACK_PUSH_POP
    #endif
    #define HANDLE_PRAGMA_PACK_PUSH_POP 1  // enable "#pragma pack" in DJGPP
#endif

#if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM)
    #pragma pack(push, 1)  // avoid to optimize memory alignment of "struct" members
#endif
// #ifdef DMP_DOS_WATCOM
// #pragma pack(push, 1)
// #endif

// #ifdef DMP_DOS_DJGPP
// typedef struct __attribute__((packed)) {
// #else
typedef struct {
// #endif
	BYTE bLength;						// Size of this Descriptor in Bytes
	BYTE bDescriptorType;				// Descriptor Type (=1)
	WORD bcdUSB;						// USB Spec Release Number in BCD
	BYTE bDeviceClass;					// Device Class Code
	BYTE bDeviceSubClass;				// Device Subclass Code
	BYTE bDeviceProtocol;				// Device Protocol Code
	BYTE bMaxPacketSize0;				// Maximum Packet Size for EP0
	WORD idVendor;						// Vendor ID
	WORD idProduct;						// Product ID
	WORD bcdDevice;						// Device Release Number in BCD
	BYTE iManufacturer;					// Index of String Desc for Manufacturer
	BYTE iProduct;						// Index of String Desc for Product
	BYTE iSerialNumber;					// Index of String Desc for SerNo
	BYTE bNumConfigurations;			// Number of possible Configurations
} Device_Descriptor;
//
// Configuration Descriptor
// #ifdef DMP_DOS_DJGPP
// typedef struct __attribute__((packed)) {
// #else
typedef struct {
// #endif
	BYTE bLength;						// Size of this Descriptor in Bytes
	BYTE bDescriptorType;				// Descriptor Type (=2)
	WORD wTotalLength;					// Total Length of Data for this Conf
	BYTE bNumInterfaces;				// number of Interfaces supported by Conf
	BYTE bConfigurationValue;			// Designator Value for *this* Conf
	BYTE iConfiguration;				// Index of String Desc for this Conf
	BYTE bmAttributes;					// Configuration Characteristics
	BYTE bMaxPower;						// Max. Power Consumption in Conf (*2mA)
} Configuration_Descriptor;
//
// Interface Descriptor
// #ifdef DMP_DOS_DJGPP
// typedef struct __attribute__((packed)) {
// #else
typedef struct {
// #endif
	BYTE bLength;						// Size of this Descriptor in Bytes
	BYTE bDescriptorType;				// Descriptor Type (=4)
	BYTE bInterfaceNumber;				// Number of *this* Interface (0..)
	BYTE bAlternateSetting;				// Alternative for this Interface
	BYTE bNumEndpoints;					// No of EPs used by this IF (excl. EP0)
	BYTE bInterfaceClass;				// Interface Class Code
	BYTE bInterfaceSubClass;			// Interface Subclass Code
	BYTE bInterfaceProtocol;			// Interface Protocol Code
	BYTE iInterface;					// Index of String Desc for Interface
} Interface_Descriptor;
//
// Endpoint Descriptor
// #ifdef DMP_DOS_DJGPP
// typedef struct __attribute__((packed)) {
// #else
typedef struct {
// #endif
	BYTE bLength;						// Size of this Descriptor in Bytes
	BYTE bDescriptorType;				// Descriptor Type (=5)
	BYTE bEndpointAddress;				// Endpoint Address (Number + Direction)
	BYTE bmAttributes;					// Endpoint Attributes (Transfer Type)
	WORD wMaxPacketSize;				// Max. Endpoint Packet Size
	BYTE bInterval;						// Polling Interval (Interrupt) ms
} Endpoint_Descriptor;



// -----------------------------------
//       USB CDC ACM Descriptor
// -----------------------------------
//
// Header Functional Descriptor
// #ifdef DMP_DOS_DJGPP
// typedef struct __attribute__((packed)) {
// #else
typedef struct {
// #endif
    BYTE bLength;
    BYTE bDescriptorType;
    BYTE bDescriptorSubtype;
    WORD bcdCDC;
} Header_Func_Descriptor;
//
// Call Management Functional Descriptor
// #ifdef DMP_DOS_DJGPP
// typedef struct __attribute__((packed)) {
// #else
typedef struct {
// #endif
    BYTE bLength;
    BYTE bDescriptorType;
    BYTE bDescriptorSubtype;
    BYTE bmCapabilities;
    BYTE bDataInterface;
} Call_Mana_Func_Descriptor;
//
// Abstract Control Management Functional Descriptor
// #ifdef DMP_DOS_DJGPP
// typedef struct __attribute__((packed)) {
// #else
typedef struct {
// #endif
    BYTE bLength;
    BYTE bDescriptorType;
    BYTE bDescriptorSubtype;
    BYTE bmCapabilities;
} Abst_Control_Mana_Descriptor;
//
// Union Functional Descriptor
// #ifdef DMP_DOS_DJGPP
// typedef struct __attribute__((packed)) {
// #else
typedef struct {
// #endif
	BYTE bLength;
	BYTE bDescriptorType;
	BYTE bDescriptorSubtype;
	BYTE bMasterInterface;
	BYTE bSlaveInterface0;
} Union_Func_Descriptor;



//---------------------------------------------
//             String Descriptor
//---------------------------------------------
// #ifdef DMP_DOS_DJGPP
// typedef struct __attribute__((packed)) {
// #else
typedef struct {
// #endif
	BYTE bLength;
	BYTE bDescriptorType;
	WORD bString[256];
} String_Descriptor;



//---------------------------------------------
//    CDC ACM Configuration Descriptor Set
//---------------------------------------------
// #ifdef DMP_DOS_DJGPP
// typedef struct __attribute__((packed)) {
// #else
typedef struct {
// #endif
	Configuration_Descriptor				desc_CDC_config;

		// Communication Interface Class
		Interface_Descriptor				desc_CDC_comm;
			Header_Func_Descriptor			desc_CDC_head;
			Call_Mana_Func_Descriptor		desc_CDC_call;
			Abst_Control_Mana_Descriptor	desc_CDC_abst;
			Union_Func_Descriptor			desc_CDC_union;
			Endpoint_Descriptor				desc_CDC_EP_INT_in;

		// Data Interface Class
		Interface_Descriptor				desc_CDC_data;
			Endpoint_Descriptor				desc_CDC_EP_BULK_in;
			Endpoint_Descriptor				desc_CDC_EP_BULK_out;

} Configuration_Desc_Set;

// #ifdef DMP_DOS_WATCOM
// #pragma pack(pop)
// #endif
#if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM)
	#pragma pack(pop)
#endif

#define EP0_MAX_PACKET_SIZE				(0x08)
#define EP1_MAX_PACKET_SIZE_IN			(0x08)
#define EP1_MAX_PACKET_SIZE_OUT			(0x40)
#define EP2_MAX_PACKET_SIZE_IN			(0x40)
#define EP2_MAX_PACKET_SIZE_OUT			(0x40)
#define EP3_MAX_PACKET_SIZE_IN			(0x40)
#define EP3_MAX_PACKET_SIZE_OUT			(0x40)

//---------------------------------------------
//  Definition of Standard Descriptor Fields
//---------------------------------------------
//
// Device Descriptor -> bDescriptorType
#define DSC_TYPE_DEVICE					(0x01)
#define DSC_TYPE_CONFIG					(0x02)
#define DSC_TYPE_STRING					(0x03)
#define DSC_TYPE_INTERFACE				(0x04)
#define DSC_TYPE_ENDPOINT				(0x05)
//
// Configuration Descriptor -> bmAttributes
#define DSC_CONFIG_ATR_BASE				(0x80)
#define DSC_CONFIG_ATR_SELF_POWERED		(0x40)
#define DSC_CONFIG_ATR_REMOTE_WAKEUP	(0x20)
//
// Configuration Descriptor -> bMaxPower
#define DSC_MAXPOWER(mA)				((mA + 1) >> 1)
//
// Endpoint Descriptor -> bEndpointAddress
#define EP1_OUT							(0x01)
#define EP2_OUT							(0x02)
#define EP3_OUT							(0x03)
#define EP1_IN							(0x81)
#define EP2_IN							(0x82)
#define EP3_IN							(0x83)
//
// Endpoint Descriptor -> bmAttributes
#define DSC_EP_CONTROL					(0x00)
#define DSC_EP_ISOCHRONOUS				(0x01)
#define DSC_EP_BULK						(0x02)
#define DSC_EP_INTERRUPT				(0x03)



//---------------------------------------------
//   Definition of CDC ACM Descriptor Fields
//---------------------------------------------
//
// CDC Descriptor -> bDescriptorType
#define DSC_TYPE_CS_INTERFACE			(0x24)
#define DSC_TYPE_CS_ENDPOINT			(0x25)
//
// CDC Descriptor -> bDescriptorSubtype
#define DSC_SUBTYPE_CS_HEADER			(0x00)
#define DSC_SUBTYPE_CS_CALL_MANAGE		(0x01)
#define DSC_SUBTYPE_CS_ABST_CONTROL		(0x02)
#define DSC_SUBTYPE_CS_UNION			(0x06)


#ifdef __cplusplus
}
#endif

#endif