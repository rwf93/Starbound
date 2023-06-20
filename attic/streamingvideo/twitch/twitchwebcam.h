/********************************************************************************************
* Twitch Broadcasting SDK
*
* This software is supplied under the terms of a license agreement with Justin.tv Inc. and
* may not be copied or used except in accordance with the terms of that agreement
* Copyright (c) 2012-2013 Justin.tv Inc.
*********************************************************************************************/

#ifndef TTVSDK_TWITCH_WEBCAM_H
#define TTVSDK_TWITCH_WEBCAM_H

#include "twitchsdktypes.h"

#ifdef __cplusplus
extern "C"
{
#endif


/**
 * TTV_WebCamDeviceChange - Listing of the events which occur when a new device is plugged in or a device is unplugged from the system.
 */
typedef enum
{
	TTV_WEBCAM_DEVICE_FOUND,		//!< The device was plugged in and is ready to be started.
	TTV_WEBCAM_DEVICE_LOST			//!< The device was unplugged and is unavailable.  If it was capturing you will not receive a stop notification.

} TTV_WebCamDeviceChange;


/**
 * TTV_WebCamDeviceStatus - Listing of the states a device may be in once it is available in the system.
 */
typedef enum
{
	TTV_WEBCAM_DEVICE_UNINITIALIZED,	//!< The device hasn't been initialized.
	TTV_WEBCAM_DEVICE_STARTED,			//!< The device is currently capturing.
	TTV_WEBCAM_DEVICE_STOPPED			//!< The device is not currently capturing.

} TTV_WebCamDeviceStatus;


/**
 * TTV_WebcamFormat - Listing of the various formats a device may return a buffer in.  If you are unable to handle a buffer format in your application
 * you may call TTV_WebCam_ConvertToRGBA to convert the buffer to RGBA.
 */
typedef enum
{
	TTV_WEBCAM_FORMAT_UNKNOWN,

	// compressed video types
	//H264, // Not supported for now, since each frame can have a different size

	// NOTE: The following format names do not match their actual memory layout.
	// For example, ARGB32 is actually laid out in memory as BGRA.
	// This naming is inconsistent with the rest of the SDK (which has names that match the memory layout).
	// The reasoning for this is to match Direct3D, which also names things backwards.
	// That way when a webcam provides ARGB a user can create a texture of ARGB and not be confused.
	// See http://msdn.microsoft.com/en-us/library/windows/desktop/bb153349%28v=vs.85%29.aspx

	// NOTE: The following format names do not match their actual memory layout.
	// For example, ARGB32 is actually laid out in memory as BGRA.
	// This naming is inconsistent with the rest of the SDK (which has names that match the memory layout).
	// The reasoning for this is to match Direct3D, which also names things backwards.
	// That way when a webcam provides ARGB a user can create a texture of ARGB and not be confused.
	// See http://msdn.microsoft.com/en-us/library/windows/desktop/bb153349%28v=vs.85%29.aspx

	// raw rgb types
	TTV_WEBCAM_FORMAT_RGB1, // 1 bit per pixel, palettized
	TTV_WEBCAM_FORMAT_RGB4, // 4 bpp, palettized
	TTV_WEBCAM_FORMAT_RGB8, // 8 bpp, palettized
	TTV_WEBCAM_FORMAT_RGB555, // 16 bpp, XRRRRRGG GGGBBBBB
	TTV_WEBCAM_FORMAT_RGB565, // 16 bpp, RRRRRGGG GGGBBBBB
	TTV_WEBCAM_FORMAT_RGB24, // 24 bpp, R8G8B8
	TTV_WEBCAM_FORMAT_XRGB32, // 32 bpp, B8G8R8X8
	TTV_WEBCAM_FORMAT_ARGB1555, // 16 bpp, 1 bit alpha, ARRRRRGG GGGBBBBB
	TTV_WEBCAM_FORMAT_ARGB32, // 32 bpp, 8 bits alpha, B8G8R8A8
	TTV_WEBCAM_FORMAT_ARGB4444, // 16 bpp, 4 bits alpha
	TTV_WEBCAM_FORMAT_B10G10R10A2, // 32 bpp, 10 RGB, 2 alpha, BBBBBBBB BBGGGGGG GGGGRRRR RRRRRRAA
	TTV_WEBCAM_FORMAT_R10G10B10A2, // 32 bpp, 10 RGB, 2 alpha, RRRRRRRR RRGGGGGG GGGGBBBB BBBBBBAA

	// raw yuv types
	TTV_WEBCAM_FORMAT_AYUV, // AYUV 4:4:4 packed 8 bpp
	TTV_WEBCAM_FORMAT_YUY2, // YUY2 4:2:2 packed 8 bpp
	TTV_WEBCAM_FORMAT_UYVY, // UYVY 4:2:2 packed 8 bpp
	TTV_WEBCAM_FORMAT_IMC1, // IMC1 4:2:0 planar 8 bpp
	TTV_WEBCAM_FORMAT_IMC2, // IMC2 4:2:0 planar 8 bpp
	TTV_WEBCAM_FORMAT_IMC3, // IMC3 4:2:0 planar 8 bpp
	TTV_WEBCAM_FORMAT_IMC4, // IMC4 4:2:0 planar 8 bpp
	TTV_WEBCAM_FORMAT_YV12, // YV12 4:2:0 planar 8 bpp
	TTV_WEBCAM_FORMAT_NV12 // NV12 4:2:0 planar 8 bpp

} TTV_WebcamFormat;


/**
 * TTV_WebcamResolution - The resolution of an image.
 */
typedef struct
{
	unsigned int height;	//!< The buffer height.
	unsigned int width;		//!< The buffer width.

} TTV_WebcamResolution;


/**
 * TTV_WebCamDeviceCapability - The properties of a mode in which a device can capture.  Devices usually support more than one capability.
 */
typedef struct
{
	unsigned int			capabilityIndex;	//!< The unique index of the capability which is not simply the index in the list.
	TTV_WebcamResolution	resolution;			//!< The resolution of the capability.
	uint32_t 				frameRate;			//!< The approximate frame rate in frames per second.
	TTV_WebcamFormat 		format;				//!< The buffer format.
	bool 					isTopToBottom;		//!< Whether or not the buffer is packed from top to bottom.
	bool					isNative;			//!< Whether or not a conversion from the raw buffer needs to be done to provide this format.  Native formats are more efficient.

} TTV_WebCamDeviceCapability;


/**
 * TTV_WebCamDeviceCapabilityList - A list of capabilities for the device.
 */
typedef struct
{
	TTV_WebCamDeviceCapability* 	list;		//!< The list.
	unsigned int 					count;		//!< The number of elements in the list.

} TTV_WebCamDeviceCapabilityList;



#define kMaxDeviceNameLength 64		//!< The max number of characters in the friendly name of a device.
#define kMaxDeviceIdLength 256		//!< The max number of characters in the unique id a device.


/**
 * TTV_WebCamDevice - Information about one specific device instance.
 */
typedef struct
{
	utf8char 						uniqueId[kMaxDeviceIdLength+1]; //!< The unique id of the device which should be saved as a user preference once selected.
	utf8char 						name[kMaxDeviceNameLength+1];	//!< The human readable name of the device.
	unsigned int					deviceIndex;					//!< The device index for the current run of the game.  If the device is unplugged and plugged back in it will change.
	TTV_WebCamDeviceStatus			status;							//!< The current capture status.
	TTV_WebCamDeviceCapabilityList 	capabilityList;					//!< The list of resolutions and formats the device supports.

} TTV_WebCamDevice;


/**
 * TTV_WebCamDeviceList - A list of devices in the system.
 */
typedef struct
{
	TTV_WebCamDevice* 				list;		//!< The list.
	unsigned int 					count;		//!< The number of elements in the list.

} TTV_WebCamDeviceList;


/**
 * TTV_WebCamFrame - Information about a captured frame.
 */
typedef struct
{
	uint8_t* 						imageBuffer;	//!< The buffer containing the captured image.
	unsigned int 					deviceIndex;	//!< The index of the device from which the frame originated.
	unsigned int 					bufferSize;		//!< The length of thr buffer.
	TTV_WebCamDeviceCapability 		capability;		//!< The active capability at the time the frame was generated.
} TTV_WebCamFrame;


/**
 * TTV_WebCamInitializationCallback - The callback to the client indicated the result of the webcam system initializing.  Once this is called
 * and indicates success the client will receive device change events which indicate devices being found.
 *
 * @param error The result code of the initialization.  TTV_EC_SUCCESS indicates success.
 * @param userdata The custom data provided by the client.
 */
typedef void (*TTV_WebCamInitializationCallback) (TTV_ErrorCode error, void* userdata);


/**
 * TTV_WebCamShutdownCallback - The callback to the client indicated the result of the webcam system shutting down.  Once this is called the
 * webcam system is no longer available until reinitialized.
 *
 * @param error The result code of the initialization.  TTV_EC_SUCCESS indicates success.
 * @param userdata The custom data provided by the client.
 */
typedef void (*TTV_WebCamShutdownCallback) (TTV_ErrorCode error, void* userdata);

/**
 * TTV_WebCamDeviceStatusCallback - Callback called when a device starts or stops capturing.  The device data should be copied before the callback returns if the client
 * wants to keep it.  If the capability is in a format that the client doesn't know how to handle it should call TTV_WebCam_ConvertToRGBA to convert the buffer
 * to RGBA.  The status is contained in the device info.
 *
 * @param device The device info.
 * @param capability The capability which was started or stopped.
 * @param error Any error associated with the status change.  TTV_EC_SUCCESS indicates success.
 * @param userdata The custom data provided by the client.
 */
typedef void (*TTV_WebCamDeviceStatusCallback) (const TTV_WebCamDevice* device, const TTV_WebCamDeviceCapability* capability, TTV_ErrorCode error, void* userdata);

/**
 * TTV_WebCamDeviceChangeCallback - Callback called when a device is added or removed from the system.  The device data should be copied before the callback returns if the client
 * wants to keep it.
 *
 * @param change The type of change.
 * @param device The device the change occurred on.
 * @param error Any error associated with the status change.  TTV_EC_SUCCESS indicates success.
 * @param userdata The custom data provided by the client in the TTV_WebCamCallbacks structure during initialization.
 */
typedef void (*TTV_WebCamDeviceChangeCallback) (TTV_WebCamDeviceChange change, const TTV_WebCamDevice* device, TTV_ErrorCode error, void* userdata);

/**
 * TTV_WebCamCallbacks - The set of callbacks and their data which are called when unsolicited events
 * occur in the webcam system.
 */
typedef struct
{
	TTV_WebCamDeviceChangeCallback 		deviceChangeCallback;	//!< The callback to call when a device is found or lost.

	void* deviceChangeUserData;			//!< The userdata to pass along when deviceChangeCallback is called.

} TTV_WebCamCallbacks;


/**
 * TTV_WebCam_Init - Sets up the webcam system for use.
 *
 * @param interruptCallbacks The structure which configures the callbacks and user data passed along when an unsolicited event occurs.
 * @param initCallback The callback which is called when initialization of the systen is complete.  When the callback is called the device list will
 *					   be available and devices ready for use.
 * @param userdata The data to be passed into the initCallback.
 *
 * @return TTV_EC_SUCCESS, TTV_EC_ALREADY_INITIALIZED, TTV_EC_INVALID_ARG.
 */
TTV_ErrorCode TTV_WebCam_Init(const TTV_WebCamCallbacks* interruptCallbacks, TTV_WebCamInitializationCallback initCallback, void* userdata);

/**
 * TTV_WebCam_Shutdown - Shuts down and cleans up the internals of the webcam system.
 *
 * @param callback The callback which is called when shutdown of the systen is complete.
 * @param userdata The data to be passed into the callback.
 *
 * @return TTV_EC_SUCCESS, TTV_EC_NOT_INITIALIZED.
 */
TTV_ErrorCode TTV_WebCam_Shutdown(TTV_WebCamShutdownCallback callback, void* userdata);

/**
 * TTV_WebCam_Start - Begins capturing from the named device.  The unique id of the device can be obtained from a call to TTV_WebCam_GetDevices.  The callback will
 *					  be called when the start has been processed.  When capturing starts the frameReadyCallback in the configured interruptCallbacks will be
 *					  called periodically with the latest frame from the device.
 *
 * @param uniqueId The unique identifer of the device which can be found in the TTV_WebCamDevice struct.
 * @param capabilityIndex The index of the parameters to use when capturing frames from the device.
 * @param callback The callback to call when capturing begins.
 * @param userdata The data to pass to the callback.
 *
 * @return TTV_EC_SUCCESS, TTV_EC_NOT_INITIALIZED.
 */
TTV_ErrorCode TTV_WebCam_Start(int deviceIndex, int capabilityIndex, TTV_WebCamDeviceStatusCallback callback, void* userdata);

/**
 * TTV_WebCam_Stop - Stops the capturing of frames from the device. The callback will be called when the start has been processed.
 *
 * @param uniqueId The unique identifer of the device which can be found in the TTV_WebCamDevice struct.
 * @param callback The callback to call when capturing begins.
 * @param userdata The data to pass to the callback.
 *
 * @return TTV_EC_SUCCESS, TTV_EC_NOT_INITIALIZED.
 */
TTV_ErrorCode TTV_WebCam_Stop(int deviceIndex, TTV_WebCamDeviceStatusCallback callback, void* userdata);

/**
 * TTV_WebCam_FlushEvents - Causes callbacks from the webcam system to be flushed to the client.  This should be called very frequently (once per game frame ideally).
 *
 * @return TTV_EC_SUCCESS, TTV_EC_NOT_INITIALIZED.
 */
TTV_ErrorCode TTV_WebCam_FlushEvents();

/**
 * TTV_WebCam_IsFrameAvailable - Determines if there is a new frame available for the given device.
 *
 * @param deviceIndex The index of the device to check.
 * @param available The result.
 *
 * @return TTV_EC_SUCCESS, TTV_EC_NOT_INITIALIZED.
 */
TTV_ErrorCode TTV_WebCam_IsFrameAvailable(int deviceIndex, bool* available);

/**
 * TTV_WebCam_GetFrame - Fills the given buffer with the contents of the next available frame.
 *
 * NOTE: On iOS the image is not automatically rotated to compensate for the orientation of the device.
 * The provided image is captured from the cameras default orientation of UIDeviceOrientationPortrait.
 * Use the following code snippet to determine the device's current orientation and make changes to the way you display it.
 *
 * UIInterfaceOrientation orient = [[UIApplication sharedApplication] statusBarOrientation];
 *
 * @param deviceIndex The index of the device to get the image from.
 * @param buffer The buffer to fill.
 * @param pitch The number of bytes in each row of the buffer.
 *
 * @return TTV_EC_SUCCESS, TTV_EC_NOT_INITIALIZED.
 */
TTV_ErrorCode TTV_WebCam_GetFrame(int deviceIndex, void* buffer, unsigned int pitch);


#ifdef __cplusplus
}
#endif

#endif	/* TTVSDK_TWITCH_WEBCAM_H */
