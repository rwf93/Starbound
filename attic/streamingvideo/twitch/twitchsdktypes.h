/********************************************************************************************
* Twitch Broadcasting SDK
*
* This software is supplied under the terms of a license agreement with Justin.tv Inc. and
* may not be copied or used except in accordance with the terms of that agreement
* Copyright (c) 2012-2013 Justin.tv Inc.
*********************************************************************************************/

#ifndef TTVSDK_TWITCH_SDK_TYPES_H
#define TTVSDK_TWITCH_SDK_TYPES_H

#include <stddef.h>
#include <stdint.h>
#include "errno.h"

#define TTVSDK_API


/**
 * Specifies that the string is encoded as UTF-8.
 */
typedef char utf8char;
typedef unsigned int uint;

/**
* TTV_MessageLevel - Level of debug messages to be printed
*/
typedef enum
{
	TTV_ML_DEBUG,
	TTV_ML_INFO,
	TTV_ML_WARNING,
	TTV_ML_ERROR,
	TTV_ML_CHAT,

	TTV_ML_NONE // this must appear last (since a given value and the values greater are the ones logged - "none" at the highest value effectively disables logging)
} TTV_MessageLevel;

/**
 * Memory allocation callback function signatures
 */
typedef void* (*TTV_AllocCallback) (size_t size, size_t alignment);
typedef void (*TTV_FreeCallback) (void* ptr);


/**
 * TTV_MemCallbacks - Pointers to memory allocation callback functions provided by the client
 */
typedef struct
{
	size_t size;							/* Size of the struct */
	TTV_AllocCallback allocCallback;		/* Memory allocation callback function provided by the client */
	TTV_FreeCallback  freeCallback;			/* Memory deallocation callback function provided by the client */
} TTV_MemCallbacks;


/**
 * TTV_AuthParams - Authentication parameters for broadcasting on Twitch. A broadcast key is ultimately needed; but it may be obtained by using username/password.
 */
typedef struct
{
	size_t size;						/* Size of the struct */
	const char* userName;				/* Twitch user name */
	const char* password;				/* Twitch password */
	const char* clientSecret;			/* Twitch client secret */

} TTV_AuthParams;


/**
 * TTV_IngestServer - Represents a Twitch Ingest server
 */
#define kMaxServerNameLength 255
#define kMaxServerUrlLength 255
typedef struct
{
	char serverName[kMaxServerNameLength+1];			/* The ingest server name */
	char serverUrl[kMaxServerUrlLength+1];				/* The ingest server URL */
	bool defaultServer;									/* Is this the default ingest server to use */

} TTV_IngestServer;


/**
 * TTV_IngestList - A list of ingest servers
 */
typedef struct
{
	TTV_IngestServer* ingestList;
	uint ingestCount;
} TTV_IngestList;


/**
 * TTV_PixelFormat - Supported input frame pixel formats
 */
typedef enum
{
	TTV_PF_BGRA = 0x00010203,
	TTV_PF_ABGR = 0x01020300,
	TTV_PF_RGBA = 0x02010003,
	TTV_PF_ARGB = 0x03020100

} TTV_PixelFormat;


/**
 * TTV_EncodingCpuUsage - Set CPU usage level for video encoding
 *
 */
typedef enum
{
	TTV_ECU_LOW,
	TTV_ECU_MEDIUM,
	TTV_ECU_HIGH
} TTV_EncodingCpuUsage;


/**
 * TTV_VideoEncoder - The video encoder to use
 */
typedef enum
{
	TTV_VID_ENC_DEFAULT = -1,

	TTV_VID_ENC_INTEL = 0,
	TTV_VID_ENC_X264 = 1,
	TTV_VID_ENC_APPLE = 2
} TTV_VideoEncoder;

#define TTV_MIN_BITRATE 230
#define TTV_MAX_BITRATE 3500
#define TTV_MIN_FPS		10
#define TTV_MAX_FPS		60
#define TTV_MAX_WIDTH	1920				/* Width and height must be multiples of 16 */
#define TTV_MAX_HEIGHT	1200

/**
 * TTV_VideoParams - Video parameters
 */
typedef struct
{
	size_t size;							/* Size of the struct */
	uint outputWidth;						/* Width of the output video stream in pixels */
	uint outputHeight;						/* Height of the output video stream in pixels */
	TTV_PixelFormat pixelFormat;			/* Frame pixel format */
	uint maxKbps;							/* Max bit rate */
	uint targetFps;							/* Target frame rate */
	TTV_EncodingCpuUsage encodingCpuUsage;	/* CPU usage level for video encoding */
	bool disableAdaptiveBitrate;			/* By default the SDK attempts to match the targetBitrate (set in maxKbps above) in cases where frames are submitted at a lower rate than target FPS. You can disable this feature */
	bool verticalFlip;						/* Flip the frames vertically - This incurs a performance penalty on some platforms (e.g. iOS) */
} TTV_VideoParams;


/**
 * TTV_AudioEncoder - The audio encoder to use
 */
typedef enum
{
	TTV_AUD_ENC_DEFAULT = -1,

	TTV_AUD_ENC_LAMEMP3 = 0,
	TTV_AUD_ENC_APPLEAAC = 1,
} TTV_AudioEncoder;


/**
 * TTV_AudioSampleFormat - Supported input audio sample formats
 */
typedef enum
{
	TTV_ASF_PCM_S16							/* PCM signed 16-bit */

} TTV_AudioSampleFormat;


/**
 * TTV_AudioParams - Audio parameters
 */
typedef struct
{
	size_t size;						/* Size of the struct */
	bool audioEnabled;					/* Is audio enabled? */
} TTV_AudioParams;


typedef enum TTV_AudioDeviceType
{
	TTV_PLAYBACK_DEVICE,
	TTV_RECORDER_DEVICE,

	TTV_DEVICE_NUM
} TTV_AudioDeviceType;


/**
 * TTV_ErrorCode
 */
typedef enum
{
	// Warnings //////////////////////////////////////////////////

	TTV_EC_WARNING_START = -1000,
	TTV_WRN_MUTEX_LOCKED,
	TTV_WRN_FAILED_TO_INIT_MIC_CAPTURE,
	TTV_WRN_NOMOREDATA,
	TTV_WRN_FRAMES_QUEUEING,
	TTV_WRN_MUTEX_NOT_AQUIRED,
	TTV_WRN_PREV_GAME_NAME_MATCH_REQUEST_DROPPED,	//!< A previously pending request for matching game names was dropped in favor of the most recent request.
	TTV_WRN_DEPRECATED,
	TTV_WRN_CHAT_MESSAGE_SPAM_DISCARDED, //!< The client is submitting chat messages too quickly and the message was discarded.
	TTV_WRN_WAIT_TIMEOUT, //!< a thread wait timed out


	// Success //////////////////////////////////////////////////

	TTV_EC_SUCCESS = 0,


	// Errors //////////////////////////////////////////////////

	TTV_EC_UNKNOWN_ERROR,

	TTV_EC_CANNOT_OPEN_FILE,
	TTV_EC_ALREADY_INITIALIZED,
	TTV_EC_CANNOT_WRITE_TO_FILE,
	TTV_EC_CANNOT_CREATE_MUTEX,
	TTV_EC_CANNOT_DESTROY_MUTEX,
	TTV_EC_COULD_NOT_TAKE_MUTEX,
	TTV_EC_COULD_NOT_RELEASE_MUTEX,
	TTV_EC_INVALID_MUTEX,
	TTV_EC_WAIT_TIMED_OUT,
	TTV_EC_INVALID_ARG,
	TTV_EC_NOT_INITIALIZED,
	TTV_EC_AUTHENTICATION,
	TTV_EC_INVALID_AUTHTOKEN,
	TTV_EC_MEMORY,
	TTV_EC_ALIGN16_REQUIRED,
	TTV_EC_UNSUPPORTED_INPUT_FORMAT,
	TTV_EC_UNSUPPORTED_OUTPUT_FORMAT,
	TTV_EC_INVALID_RESOLUTION,
	TTV_EC_INVALID_FPS,
	TTV_EC_INVALID_BITRATE,
	TTV_EC_FAILED_TO_INIT_SPEAKER_CAPTURE,
	TTV_EC_FRAME_QUEUE_FULL,
	TTV_EC_CURL_ERROR,
	TTV_EC_INVALID_CLIENTID,
	TTV_EC_INVALID_CHANNEL_NAME,
	TTV_EC_INVALID_CACERT_BUNDLE_FILE,
	TTV_EC_API_REQUEST_FAILED,
	TTV_EC_API_REQUEST_TIMEDOUT,
	TTV_EC_INVALID_API_CURL_PARAMS,
	TTV_EC_WEBAPI_RESULT_INVALID_JSON,
	TTV_EC_WEBAPI_RESULT_NO_AUTHTOKEN,
	TTV_EC_WEBAPI_RESULT_NO_STREAMKEY,
	TTV_EC_WEBAPI_RESULT_NO_CHANNELNAME,
	TTV_EC_WEBAPI_RESULT_NO_INGESTS,
	TTV_EC_WEBAPI_RESULT_NO_RECORDING_STATUS,
	TTV_EC_WEBAPI_RESULT_NO_STREAMINFO,
	TTV_EC_WEBAPI_RESULT_INVALID_VIEWERS,
	TTV_EC_WEBAPI_RESULT_NO_USERNAME,
	TTV_EC_WEBAPI_RESULT_NO_USER_DISPLAY_NAME,
	TTV_EC_NO_STREAM_KEY,
	TTV_EC_NEED_TO_LOGIN,
	TTV_EC_INVALID_VIDEOFRAME,
	TTV_EC_INVALID_BUFFER,
	TTV_EC_INVALID_CALLBACK,
	TTV_EC_INVALID_JSON,
	TTV_EC_NO_SPSPPS,
	TTV_EC_NO_D3D_SUPPORT,
	TTV_EC_NO_INGEST_SERVER_AVAILABLE,
	TTV_EC_INVALID_INGEST_SERVER,
	TTV_EC_CANNOT_SUSPEND_THREADSYNC,
	TTV_EC_CANNOT_SIGNAL_THREADSYNC,
	TTV_EC_INVALID_ENCODER,
	TTV_EC_AUDIO_DEVICE_INIT_FAILED,
	TTV_EC_INVALID_SAMPLERATE,
	TTV_EC_X264_INVALID_PRESET,
	TTV_EC_X264_INVALID_PROFILE,

	TTV_EC_FLV_UNABLE_TO_OPEN_FILE,
	TTV_EC_FLV_FILE_NOT_OPEN,
	TTV_EC_FLV_UNSUPPORTED_AUDIO_RATE,
	TTV_EC_FLV_UNSUPPORTED_AUDIO_CODEC,

	TTV_EC_RTMP_WRONG_PROTOCOL_IN_URL,
	TTV_EC_RTMP_UNABLE_TO_SEND_DATA,
	TTV_EC_RTMP_INVALID_FLV_PACKET,
	TTV_EC_RTMP_TIMEOUT,

	// Mac audio capture
	TTV_EC_MAC_INPUT_Q_SETUP_FAILED,
	TTV_EC_MAC_INPUT_Q_BUFFER_SETUP_FAILED,
	TTV_EC_MAC_INPUT_Q_START_FAILED,


// intel encoder
	TTV_EC_INTEL_FAILED_SESSION_INIT,
	TTV_EC_INTEL_FAILED_VPP_INIT,
	TTV_EC_INTEL_FAILED_ENCODER_INIT,
	TTV_EC_INTEL_FAILED_SURFACE_ALLOCATION,
	TTV_EC_INTEL_FAILED_TASKPOLL_INIT,
	TTV_EC_INTEL_NO_FREE_TASK,
	TTV_EC_INTEL_NO_FREE_SURFACE,

// apple encoder
	TTV_EC_APPLEENCODER_FAILED_START,
	TTV_EC_APPLEENCODER_FAILED_FRAME_SUBMISSION,

// lame mp3 encoder
	TTV_EC_LAMEMP3_FAILED_INIT,
	TTV_EC_LAMEMP3_FAILED_SHUTDOWN,

// apple aac encoder
	TTV_EC_APPLEAAC_FAILED_INIT,
	TTV_EC_APPLEAAC_FAILED_ENCODING,
	TTV_EC_APPLEAAC_FAILED_SHUTDOWN,

// TODO: map mac errors onto winsock errors
	TTV_EC_REQUEST_PENDING,
	TTV_EC_WSASTARTUP_FAILED,
	TTV_EC_WSACLEANUP_FAILED,
	TTV_EC_SOCKET_GETADDRINFO_FAILED,
	TTV_EC_SOCKET_CREATE_FAILED,
	TTV_EC_SOCKET_CONNECT_FAILED,
	TTV_EC_SOCKET_SEND_ERROR,
	TTV_EC_SOCKET_RECV_ERROR,
	TTV_EC_SOCKET_IOCTL_ERROR,

	TTV_EC_SOCKET_ERR = 1000,

	TTV_EC_SOCKET_EINTR = TTV_EC_SOCKET_ERR + 4,
	TTV_EC_SOCKET_EBADF = TTV_EC_SOCKET_ERR + 9,
	TTV_EC_SOCKET_EACCES = TTV_EC_SOCKET_ERR + 13,
	TTV_EC_SOCKET_EFAULT = TTV_EC_SOCKET_ERR + 14,
	TTV_EC_SOCKET_EINVAL = TTV_EC_SOCKET_ERR + 22,
	TTV_EC_SOCKET_EMFILE = TTV_EC_SOCKET_ERR + 24,
	TTV_EC_SOCKET_EWOULDBLOCK = TTV_EC_SOCKET_ERR + 35,
	TTV_EC_SOCKET_EINPROGRESS = TTV_EC_SOCKET_ERR + 36,
	TTV_EC_SOCKET_EALREADY = TTV_EC_SOCKET_ERR + 37,
	TTV_EC_SOCKET_ENOTSOCK = TTV_EC_SOCKET_ERR + 38,
	TTV_EC_SOCKET_EDESTADDRREQ = TTV_EC_SOCKET_ERR + 39,
	TTV_EC_SOCKET_EMSGSIZE = TTV_EC_SOCKET_ERR + 40,
	TTV_EC_SOCKET_EPROTOTYPE = TTV_EC_SOCKET_ERR + 41,
	TTV_EC_SOCKET_ENOPROTOOPT = TTV_EC_SOCKET_ERR + 42,
	TTV_EC_SOCKET_EPROTONOSUPPORT = TTV_EC_SOCKET_ERR + 43,
	TTV_EC_SOCKET_ESOCKTNOSUPPORT = TTV_EC_SOCKET_ERR + 44,
	TTV_EC_SOCKET_EOPNOTSUPP = TTV_EC_SOCKET_ERR + 45,
	TTV_EC_SOCKET_EPFNOSUPPORT = TTV_EC_SOCKET_ERR + 46,
	TTV_EC_SOCKET_EAFNOSUPPORT = TTV_EC_SOCKET_ERR + 47,
	TTV_EC_SOCKET_EADDRINUSE = TTV_EC_SOCKET_ERR + 48,
	TTV_EC_SOCKET_EADDRNOTAVAIL = TTV_EC_SOCKET_ERR + 49,
	TTV_EC_SOCKET_ENETDOWN = TTV_EC_SOCKET_ERR + 50,
	TTV_EC_SOCKET_ENETUNREACH = TTV_EC_SOCKET_ERR + 51,
	TTV_EC_SOCKET_ENETRESET = TTV_EC_SOCKET_ERR + 52,
	TTV_EC_SOCKET_ECONNABORTED = TTV_EC_SOCKET_ERR + 53,
	TTV_EC_SOCKET_ECONNRESET = TTV_EC_SOCKET_ERR + 54,
	TTV_EC_SOCKET_ENOBUFS = TTV_EC_SOCKET_ERR + 55,
	TTV_EC_SOCKET_EISCONN = TTV_EC_SOCKET_ERR + 56,
	TTV_EC_SOCKET_ENOTCONN = TTV_EC_SOCKET_ERR + 57,
	TTV_EC_SOCKET_ESHUTDOWN = TTV_EC_SOCKET_ERR + 58,
	TTV_EC_SOCKET_ETOOMANYREFS = TTV_EC_SOCKET_ERR + 59,
	TTV_EC_SOCKET_ETIMEDOUT = TTV_EC_SOCKET_ERR + 60,
	TTV_EC_SOCKET_ECONNREFUSED = TTV_EC_SOCKET_ERR + 61,
	TTV_EC_SOCKET_ELOOP = TTV_EC_SOCKET_ERR + 62,
	TTV_EC_SOCKET_ENAMETOOLONG = TTV_EC_SOCKET_ERR + 63,
	TTV_EC_SOCKET_EHOSTDOWN = TTV_EC_SOCKET_ERR + 64,
	TTV_EC_SOCKET_EHOSTUNREACH = TTV_EC_SOCKET_ERR + 65,
	TTV_EC_SOCKET_ENOTEMPTY = TTV_EC_SOCKET_ERR + 66,
	TTV_EC_SOCKET_EPROCLIM = TTV_EC_SOCKET_ERR + 67,
	TTV_EC_SOCKET_EUSERS = TTV_EC_SOCKET_ERR + 68,
	TTV_EC_SOCKET_EDQUOT = TTV_EC_SOCKET_ERR + 69,
	TTV_EC_SOCKET_ESTALE = TTV_EC_SOCKET_ERR + 70,
	TTV_EC_SOCKET_EREMOTE = TTV_EC_SOCKET_ERR + 71,
	TTV_EC_SOCKET_SYSNOTREADY = TTV_EC_SOCKET_ERR + 91,
	TTV_EC_SOCKET_VERNOTSUPPORTED = TTV_EC_SOCKET_ERR + 92,
	TTV_EC_SOCKET_NOTINITIALISED = TTV_EC_SOCKET_ERR + 93,
	TTV_EC_SOCKET_EDISCON = TTV_EC_SOCKET_ERR + 101,
	TTV_EC_SOCKET_ENOMORE = TTV_EC_SOCKET_ERR + 102,
	TTV_EC_SOCKET_ECANCELLED = TTV_EC_SOCKET_ERR + 103,
	TTV_EC_SOCKET_EINVALIDPROCTABLE = TTV_EC_SOCKET_ERR + 104,
	TTV_EC_SOCKET_EINVALIDPROVIDER = TTV_EC_SOCKET_ERR + 105,
	TTV_EC_SOCKET_EPROVIDERFAILEDINIT = TTV_EC_SOCKET_ERR + 106,
	TTV_EC_SOCKET_SYSCALLFAILURE = TTV_EC_SOCKET_ERR + 107,
	TTV_EC_SOCKET_SERVICE_NOT_FOUND = TTV_EC_SOCKET_ERR + 108,
	TTV_EC_SOCKET_TYPE_NOT_FOUND = TTV_EC_SOCKET_ERR + 109,
	TTV_EC_SOCKET_E_NO_MORE = TTV_EC_SOCKET_ERR + 110,
	TTV_EC_SOCKET_E_CANCELLED = TTV_EC_SOCKET_ERR + 111,
	TTV_EC_SOCKET_EREFUSED = TTV_EC_SOCKET_ERR + 112,
	TTV_EC_SOCKET_HOST_NOT_FOUND = TTV_EC_SOCKET_ERR + 1001,
	TTV_EC_SOCKET_TRY_AGAIN = TTV_EC_SOCKET_ERR + 1002,
	TTV_EC_SOCKET_NO_RECOVERY = TTV_EC_SOCKET_ERR + 1003,
	TTV_EC_SOCKET_NO_DATA = TTV_EC_SOCKET_ERR + 1004,
	TTV_EC_SOCKET_QOS_RECEIVERS = TTV_EC_SOCKET_ERR + 1005,
	TTV_EC_SOCKET_QOS_SENDERS = TTV_EC_SOCKET_ERR + 1006,
	TTV_EC_SOCKET_QOS_NO_SENDERS = TTV_EC_SOCKET_ERR + 1007,
	TTV_EC_SOCKET_QOS_NO_RECEIVERS = TTV_EC_SOCKET_ERR + 1008,
	TTV_EC_SOCKET_QOS_REQUEST_CONFIRMED = TTV_EC_SOCKET_ERR + 1009,
	TTV_EC_SOCKET_QOS_ADMISSION_FAILURE = TTV_EC_SOCKET_ERR + 1010,
	TTV_EC_SOCKET_QOS_POLICY_FAILURE = TTV_EC_SOCKET_ERR + 1011,
	TTV_EC_SOCKET_QOS_BAD_STYLE = TTV_EC_SOCKET_ERR + 1012,
	TTV_EC_SOCKET_QOS_BAD_OBJECT = TTV_EC_SOCKET_ERR + 1013,
	TTV_EC_SOCKET_QOS_TRAFFIC_CTRL_ERROR = TTV_EC_SOCKET_ERR + 1014,
	TTV_EC_SOCKET_QOS_GENERIC_ERROR = TTV_EC_SOCKET_ERR + 1015,
	TTV_EC_SOCKET_QOS_ESERVICETYPE = TTV_EC_SOCKET_ERR + 1016,
	TTV_EC_SOCKET_QOS_EFLOWSPEC = TTV_EC_SOCKET_ERR + 1017,
	TTV_EC_SOCKET_QOS_EPROVSPECBUF = TTV_EC_SOCKET_ERR + 1018,
	TTV_EC_SOCKET_QOS_EFILTERSTYLE = TTV_EC_SOCKET_ERR + 1019,
	TTV_EC_SOCKET_QOS_EFILTERTYPE = TTV_EC_SOCKET_ERR + 1020,
	TTV_EC_SOCKET_QOS_EFILTERCOUNT = TTV_EC_SOCKET_ERR + 1021,
	TTV_EC_SOCKET_QOS_EOBJLENGTH = TTV_EC_SOCKET_ERR + 1022,
	TTV_EC_SOCKET_QOS_EFLOWCOUNT = TTV_EC_SOCKET_ERR + 1023,
	TTV_EC_SOCKET_QOS_EUNKOWNPSOBJ = TTV_EC_SOCKET_ERR + 1024,
	TTV_EC_SOCKET_QOS_EPOLICYOBJ = TTV_EC_SOCKET_ERR + 1025,
	TTV_EC_SOCKET_QOS_EFLOWDESC = TTV_EC_SOCKET_ERR + 1026,
	TTV_EC_SOCKET_QOS_EPSFLOWSPEC = TTV_EC_SOCKET_ERR + 1027,
	TTV_EC_SOCKET_QOS_EPSFILTERSPEC = TTV_EC_SOCKET_ERR + 1028,
	TTV_EC_SOCKET_QOS_ESDMODEOBJ = TTV_EC_SOCKET_ERR + 1029,
	TTV_EC_SOCKET_QOS_ESHAPERATEOBJ = TTV_EC_SOCKET_ERR + 1030,
	TTV_EC_SOCKET_QOS_RESERVED_PETYPE = TTV_EC_SOCKET_ERR + 1031,
	TTV_EC_SOCKET_SECURE_HOST_NOT_FOUND = TTV_EC_SOCKET_ERR + 1032,
	TTV_EC_SOCKET_IPSEC_NAME_POLICY_ERROR = TTV_EC_SOCKET_ERR + 1033,

	TTV_EC_SOCKET_END,

// chat
	TTV_EC_CHAT_NOT_INITIALIZED,		//!< The chat subsystem has not been initialized properly.
	TTV_EC_CHAT_ALREADY_INITIALIZED,	//!< The chat subsystem has already been initialized.
	TTV_EC_CHAT_ALREADY_IN_CHANNEL,		//!< Already in the channel.
	TTV_EC_CHAT_INVALID_LOGIN,			//!< Invalid login credentials.
	TTV_EC_CHAT_INVALID_CHANNEL,		//!< The named channel doesn't exist.
	TTV_EC_CHAT_LOST_CONNECTION,		//!< Lost connection to the chat server.
	TTV_EC_CHAT_COULD_NOT_CONNECT,		//!< Could not connect to the chat server.
	TTV_EC_CHAT_NOT_IN_CHANNEL,			//!< Not in the named channel.
	TTV_EC_CHAT_INVALID_MESSAGE,		//!< Not a valid message.
	TTV_EC_CHAT_TOO_MANY_REQUESTS,		//!< The request queue is growing too large and the request has been ignored.
	TTV_EC_CHAT_LEAVING_CHANNEL,		//!< In the middle of leaving the channel.
	TTV_EC_CHAT_SHUTTING_DOWN,			//!< The chat system is in the middle of shutting down and cannot be used currently.
	TTV_EC_CHAT_ANON_DENIED,			//!< Attempted to perform an action that anonymous connections are not allowed to do.
	TTV_EC_CHAT_EMOTICON_DATA_NOT_READY,			//!< The emoticon data is not ready to use.
	TTV_EC_CHAT_EMOTICON_DATA_DOWNLOADING,			//!< The emoticon data is already downloading.
	TTV_EC_CHAT_EMOTICON_DATA_LOCKED,				//!< The emoticon data has been locked by the client by a call to TTV_Chat_GetEmoticonData which has not been freed by TTV_Chat_FreeEmoticonData.
	TTV_EC_CHAT_EMOTICON_DOWNLOAD_FAILED,			//!< There were too many download errors while trying to fetch the emoticon data.	TTV_EC_CHAT_ANON_DENIED			//!< Attempted to perform an action that anonymous connections are not allowed to do.


//webcam
	TTV_EC_WEBCAM_NO_PLATFORM_SUPPORT,
	TTV_EC_WEBCAM_COULD_NOT_COMPLETE,
	TTV_EC_WEBCAM_OUT_OF_MEMORY,
	TTV_EC_WEBCAM_UNKNOWN_ERROR,
	TTV_EC_WEBCAM_INVALID_PARAMETER,
	TTV_EC_WEBCAM_INVALID_CAPABILITY,
	TTV_EC_WEBCAM_BUFFER_NOT_BIG_ENOUGH,
	TTV_EC_WEBCAM_DEVICE_NOT_STARTED,
	TTV_EC_WEBCAM_DEVICE_ALREADY_STARTED,
	TTV_EC_WEBCAM_DEVICE_NOT_FOUND,
	TTV_EC_WEBCAM_FRAME_NOT_AVAILABLE,
	TTV_EC_WEBCAM_NOT_INITIALIZED,
	TTV_EC_WEBCAM_FAILED_TO_START,
	TTV_EC_WEBCAM_LEFT_IN_UNSAFE_STATE,
	TTV_EC_WEBCAM_UNSUPPORTED_SOURCE_FORMAT,
	TTV_EC_WEBCAM_UNSUPPORTED_TARGET_FORMAT,


	TTV_EC_INVALID_STRUCT_SIZE, //!< The size field of the struct does not have the expected value.  Some fields may have been added to the struct and you may have set them and recompile your code.
	TTV_EC_STREAM_ALREADY_STARTED,	//!< The requested operation can't be serviced because the broadcast has already been started.
	TTV_EC_STREAM_NOT_STARTED,	//!< The requested operation can't be serviced because the broadcast not been started.
	TTV_EC_REQUEST_ABORTED, //!< The request was aborted and did not finish.
	TTV_EC_FRAME_QUEUE_TOO_LONG //!< The network is backing up because video settings are too high for the internet connection.  Stop the stream and restart with lower settings.

} TTV_ErrorCode;

typedef enum
{
	TTV_ST_RTMPSTATE,
	TTV_ST_RTMPDATASENT
} TTV_StatType;

#define TTV_RTMP_LAST_CONNECT_STATE 6

#define TTV_FAILED(err) ( (err) > TTV_EC_SUCCESS )
#define TTV_SUCCEEDED(err) ( (err) <= TTV_EC_SUCCESS)
#define TTV_RETURN_ON_NULL(ptr,err) { if ( (ptr) == nullptr) return err; }
#define ASSERT_ON_ERROR(err) {assert ( (err) <= TTV_EC_SUCCESS ); }

#define TTV_TO_WSA_ERROR(ttv_ec) (int)(ttv_ec-TTV_EC_SOCKET_ERR+WSABASEERR)
#define WSA_TO_TTV_ERROR(wsa_ec) (TTV_ErrorCode) (wsa_ec-WSABASEERR+TTV_EC_SOCKET_ERR)


/**
 * Callback signature for buffer unlock
 */
typedef void (*TTV_BufferUnlockCallback) (const uint8_t* buffer, void* userData);

/**
 * Callback signature for asynchronous tasks
 */
typedef void (*TTV_TaskCallback) (TTV_ErrorCode result, void* userData);

/**
 * Callback signature for Stats reporting
 */
typedef void (*TTV_StatCallback) (TTV_StatType type, uint64_t data);

/**
 * TTV_VideoFrame - Input video frame parameters
 */
typedef struct
{
	size_t size;							/* Size of the struct */
	uint8_t* frameBuffer;					/* Raw bytes of the frame - the frame resolution MUST match the resolution of the output video */
	TTV_BufferUnlockCallback callback;		/* callback that gets called when VideoFrame is no longer needed */
	void* userData;							/* userData passed to the callback */
	uint64_t mTimeStamp;					/* For internal use */
} TTV_VideoFrame;

/**
* TTV_AuthToken - The max length of a Twitch authentication token.
*/
const uint kAuthTokenBufferSize = 128;

/**
 * TTV_AuthToken - The authentication token returned after successfully authenticating with username and password.
 */
typedef struct
{
	char data[kAuthTokenBufferSize];		/* The token data. */
} TTV_AuthToken;

/**
 * TTV_UserInfo - The user name
 */
#define kMaxUserNameLength 63
typedef struct
{
	size_t size;				/* The size of the struct. */
	char displayName[kMaxUserNameLength+1];				/* The displayed name */
	char name[kMaxUserNameLength+1];					/* The user name */
} TTV_UserInfo;

/**
* TTV_ChannelInfo - The channel info
*/
#define kMaxChannelUrlLength 255
typedef struct
{
	size_t size;			/* The size of the struct. */
	char name[kMaxUserNameLength+1];				/* The channel name */
	char displayName[kMaxUserNameLength+1];				/* The displayed channel name (which may be different) */
	char channelUrl[kMaxChannelUrlLength+1];			/* The URL to that channel */
} TTV_ChannelInfo;

/**
 * TTV_StreamInfo - The stream info
 */
typedef struct
{
	size_t size;				/* The size of the struct. */
	int viewers;				/* The current viewer count. */
	uint64_t streamId;			/* The unique id of the stream. */
} TTV_StreamInfo;

/**
 * TTV_StreamInfoForSetting - The stream info
 */
#define kMaxStreamTitleLength 255
#define kMaxGameNameLength 255
typedef struct
{
	size_t size;				/* The size of the struct. */
	char streamTitle[kMaxStreamTitleLength+1];			/* The title of the stream. If the first character is null, this parameter is ignored. */
	char gameName[kMaxGameNameLength+1];				/* The name of the game being played. If the first character is null, this parameter is ignored. */
} TTV_StreamInfoForSetting;

/**
* TTV_RecordingStatus - The Video recording status of the channel
*/
#define kMaxCureUrlLength 255
typedef struct
{
	size_t size;				/* The size of the struct. */
	bool recordingEnabled;							/* Recording is enabled/disabled for the channel */
	char cureUrl[kMaxCureUrlLength+1];				/* The URL for where the user can go to enable video recording for the channel */
} TTV_ArchivingState;

/**
 * TTV_GameInfo - Display information about a game.
 */
typedef struct
{
	char name[kMaxGameNameLength+1]; 	/* The display name of the game. */
	int popularity; 					/* A popularity rating for the game. */
	int id; 							/* The game's unique id. */

} TTV_GameInfo;

/**
 * TTV_GameInfoList - A list of game info structs.
 */
typedef struct
{
	TTV_GameInfo* list;			/* The array of game info entries */
	unsigned int count;			/* The number of entries in the array. */

} TTV_GameInfoList;

/**
 * TTV_GameLiveStreamInfo - Information about a live stream of a particular game
 */
typedef struct
{
	char channelUrl[kMaxChannelUrlLength+1];
	char previewUrlTemplate[kMaxChannelUrlLength+1];
	char streamTitle[kMaxStreamTitleLength+1];
	char channelDisplayName[kMaxUserNameLength+1];
	unsigned int viewerCount;
} TTV_LiveGameStreamInfo;

/**
 * TTV_LiveGameStreamList - List of live streams of a given game
 */
typedef struct
{
	TTV_LiveGameStreamInfo* list;		/* The array of live game stream info entries */
	unsigned int count;					/* The number of entries in the array */
} TTV_LiveGameStreamList;

/**
 * All the valid flags for TTV_Start()
 */

#define TTV_Start_BandwidthTest 0x1

#endif	/* TTVSDK_TWITCH_SDK_TYPES_H */
