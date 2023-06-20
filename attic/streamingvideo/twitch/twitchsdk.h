/*
 *
 * Bartwe: 20130617 - Changed interface to export stdcall function pointers
 *
 */


/********************************************************************************************
* Twitch Broadcasting SDK
*
* This software is supplied under the terms of a license agreement with Justin.tv Inc. and
* may not be copied or used except in accordance with the terms of that agreement
* Copyright (c) 2012-2013 Justin.tv Inc.
*********************************************************************************************/

#ifndef TTVSDK_TWITCH_SDK_H
#define TTVSDK_TWITCH_SDK_H

#include "twitchsdktypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef TTV_CALLSPEC
#define TTV_CALLSPEC
#endif


/**
* TTV_Init - Initialize the Twitch Broadcasting SDK
*
* @param[in] memCallbacks - Memory allocation/deallocation callback functions provided by the client. If nullptr, malloc/free will be used
* @param[in] clientID - The Twitch client ID assigned to your application
* @param[in] caCertFile - Full path of the CA Cert bundle file (Strongly encourage using the bundle file provided with the SDK)
* @param[in] vidEncoder - The video encoder to use
* @param[in] audioEncoder - The audio encoder to use
* @param[in] dllPath - [Optional] Windows Only - Path to DLL's to load if no in exe folder (e.g. Intel DLL)
* @return - TTV_EC_SUCCESS if function succeeds; error code otherwise
*/
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_Init)(const TTV_MemCallbacks* memCallbacks,
								  const char* clientID,
								  const wchar_t* caCertFile,
								  TTV_VideoEncoder vidEncoder,
								  const wchar_t* dllPath);

/**
* TTV_GetDefaultParams - Fill in the video parameters with default settings based on supplied resolution and target FPS
* This function is now deprecated. You should use TTV_GetMaxResolution to work backwards from the bitrate.
* @param[in,out] vidParams - The video params to be filled in with default settings
* NOTE: The width, height and targetFps members of the vidParams MUST be set by the caller
* @return - TTV_EC_SUCCESS if function succeeds; error code otherwise
*/
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_GetDefaultParams)(TTV_VideoParams* vidParams);


/**
* TTV_GetMaxResolution - Using the provided maximum bitrate, motion factor, and aspect ratio, calculate
* the maximum resolution at which the video quality would be acceptable.
*
* @param[in] maxKbps - Maximum bitrate supported (this should be determined by running the ingest tester for a given ingest server)
* @param[in] frameRate - The desired frame rate. For a given bitrate and motion factor, a higher framerate will mean a lower resolution.
* @param[in] bitsPerPixel - The bits per pixel used in the final encoded video. A fast motion game (e.g. first person
							shooter) required more bits per pixel of encoded video avoid compression artifacting. Use 0.1 for an
							average motion game. For games without too many fast changes in the scene, you could use a value below
							0.1 but not much. For fast moving games with lots of scene changes a value as high as  0.2 would be appropriate.
* @param[in] aspectRatio - The aspect ratio of the video which we'll use for calculating width and height
* @param[out] width - Maximum recommended width
* @param[out] height - Maximum recommended height
* @return - TTV_EC_SUCCESS if function succeeds; error code otherwise
*/
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_GetMaxResolution)(uint maxKbps,
											  uint frameRate,
											  float bitsPerPixel,
											  float aspectRatio,
											  uint* width,
											  uint* height);


/**
* TTV_PollTasks - Polls all currently executing async tasks and if they've finished calls their callbacks
*/
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_PollTasks)();


/**
* TTV_RequestAuthToken - Request an authentication key based on the provided username and password.
* @param[in] authParams - Authentication parameters
* @param[in] callback - The callback function to be called when the request is completed
* @param[in] userData - Optional pointer to be passed through to the callback function
* @param[out] authToken - The authentication token to be written to
*/
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_RequestAuthToken)(const TTV_AuthParams* authParams,
											  TTV_TaskCallback callback,
											  void* userData,
											  TTV_AuthToken* authToken);


/**
* TTV_Login - Use the given auth token to log into the associated channel. This MUST be called
* before starting to stream and returns information about the channel. If the callback is called
* with a failure error code, the most likely cause is an invalid auth token and you need
* to call TTV_RequestAuthToken to obtain a new one.
* @param[in] authToken - The auth token to validate
* @param[in] callback - The callback function to be called when the request is completed
* @param[in] userData - Optional pointer to be passed through to the callback function
* @param[out] channelInfo (Optional; pass nullptr if not needed) - The channel info struct for the channel logged into
*/
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_Login)(const TTV_AuthToken* authToken,
								   TTV_TaskCallback callback,
								   void* userData,
								   TTV_ChannelInfo* channelInfo);


/**
* TTV_GetIngestServers - Get the list of available ingest servers
* @param[in] authToken - The authentication token previously obtained
* @param[in] callback - The callback function to be called when function is completed
* @param[in] userData - Optional pointer to be passed through to the callback function
* @param[out] ingestList - The list of available ingest servers to choose from
*/
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_GetIngestServers)(const TTV_AuthToken* authToken,
											  TTV_TaskCallback callback,
											  void* userData,
											  TTV_IngestList* ingestList);


/**
* TTV_FreeIngestList - Must be called after getting the ingest servers to free the allocated list of server info
* @param[in] ingestList - Pointer to the TTV_IngestList struct to be freed
*/
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_FreeIngestList)(TTV_IngestList* ingestList);


/**
* TTV_Start - Starts streaming.  This function is asynchronous if a valid callback is provided.
* @param[in] videoParams - Output stream video paramaters
* @param[in] audioParams - Output stream audio parameters
* @param[in] ingestServer - The ingest server to use
* @param[in] flags - optional flags
* @param[in] callback - The callback to call when the operation has completed.  Can be null to force this function to be synchronous.
* @param[in] userData - Client data to pass along in the callback.
*/
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_Start)(const TTV_VideoParams* videoParams,
								   const TTV_AudioParams* audioParams,
								   const TTV_IngestServer* ingestServer,
								   uint32_t flags,
								   TTV_TaskCallback callback,
								   void* userData);


/**
* TTV_SubmitVideoFrame - Submit a video frame to be added to the video stream
* @param[in] frameBuffer - Pointer to the frame buffer. This buffer will be considered "locked"
					       until the callback is called.
* @param[in] callback - The callback function to be called when buffer is no longer needed
* @param[in] userData - Optional pointer to be passed through to the callback function
* @return - TTV_EC_SUCCESS if function succeeds; error code otherwise
*/
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_SubmitVideoFrame)(const uint8_t* frameBuffer,
											  TTV_BufferUnlockCallback callback,
											  void* userData);


/**
* TTV_SendActionMetaData - Send a singular action metadata point to Twitch's metadata service
* @param[in] authToken - The streamer's auth token
* @param[in] streamId - The stream's unique ID
* @param[in] name - A specific name for an event meant to be queryable
* @param[in] streamTime - Number of milliseconds into the broadcast for when event occurs
* @param[in] humanDescription - Long form string to describe the meaning of an event. Maximum length is 1000 characters
* @param[in] data - A valid JSON object that is the payload of an event. Values in this JSON object have to be strings. Maximum of 50 keys are allowed. Maximum length for values are 255 characters.
*/
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_SendActionMetaData)(const TTV_AuthToken* authToken, uint64_t streamId,
												const char* name, uint64_t streamTime,
												const char* humanDescription, const char* data);


/**
* TTV_SendStartSpanMetaData - Send the beginning datapoint of an event that has a beginning and end
* @param[in] authToken - The streamer's auth token
* @param[in] streamId - The stream's unique ID
* @param[in] name - A specific name for an event meant to be queryable
* @param[in] streamTime - Number of milliseconds into the broadcast for when event occurs
* @param[out] sequenceId - A unique sequenceId returned that associates a start and end event together
* @param[in] humanDescription - Long form string to describe the meaning of an event. Maximum length is 1000 characters
* @param[in] data - A valid JSON object that is the payload of an event. Values in this JSON object have to be strings. Maximum of 50 keys are allowed. Maximum length for values are 255 characters.
*/
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_SendStartSpanMetaData)(const TTV_AuthToken* authToken, uint64_t streamId,
												   const char* name, uint64_t streamTime, unsigned long* sequenceId,
												   const char* humanDescription, const char* data);


/**
* TTV_SendEndSpanMetaData - Send the ending datapoint of an event that has a beginning and end.
* @param[in] authToken - The streamer's auth token
* @param[in] streamId - The stream's unique ID
* @param[in] name - A specific name for an event meant to be queryable
* @param[in] streamTime - Number of milliseconds into the broadcast for when event occurs
* @param[in] sequenceId - Associates a start and end event together. Use the correspoding sequenceId returned in TTV_SendStartSpanMetaData
* @param[in] humanDescription - Long form string to describe the meaning of an event. Maximum length is 1000 characters
* @param[in] data - A valid JSON object that is the payload of an event. Values in this JSON object have to be strings. Maximum of 50 keys are allowed. Maximum length for values are 255 characters.
*/
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_SendEndSpanMetaData)(const TTV_AuthToken* authToken, uint64_t streamId,
												 const char* name, uint64_t streamTime, unsigned long sequenceId,
												 const char* humanDescription, const char* data);


/**
* TTV_SetStreamInfo - Send stream-related information to Twitch
* @param[in] authToken - The authentication token previously obtained
* @param[in] channel - The channel name that will have its stream info set.
* @param[in] streamInfoToBeSet - The stream-related information to send.
*/
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_SetStreamInfo)(const TTV_AuthToken* authToken, const char* channel, const TTV_StreamInfoForSetting* streamInfoToSet);


/**
* TTV_GetUserInfo - Returns user-related information from Twitch
* @param[in] authToken - The authentication token previously obtained
* @param[in] callback - The callback function to be called when user info is retrieved
* @param[in] userData - Optional pointer to be passed through to the callback function
* @param[out] userInfo - The user-related information.
*/
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_GetUserInfo)(const TTV_AuthToken* authToken,
                                         TTV_TaskCallback callback,
                                         void* userData,
                                         TTV_UserInfo* userInfo);

/**
* TTV_StreamInfo - Returns stream-related information from Twitch
* @param[in] authToken - The authentication token previously obtained
* @param[in] callback - The callback function to be called when user info is retrieved
* @param[in] userData - Optional pointer to be passed through to the callback function
* @param[in] channel - The channel name that will have its stream info set.
* @param[out] streamInfo - The stream-related information.
*/
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_GetStreamInfo)(const TTV_AuthToken* authToken,
                                           TTV_TaskCallback callback,
                                           void* userData,
                                           const char* channel,
                                           TTV_StreamInfo* streamInfo);


/**
* TTV_GetArchivingState - Returns whether video recording is enabled for the channel and
* if not, a cure URL that lets the user to enable recording for the channel
* @param[in] authToken - The authentication token previously obtained
* @param[in] callback - The callback function to be called when user info is retrieved
* @param[in] userData - Optional pointer to be passed through to the callback function
* @param[out] archivingState - The channel archiving state
*/
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_GetArchivingState)(const TTV_AuthToken* authToken,
											   TTV_TaskCallback callback,
											   void* userData,
											   TTV_ArchivingState* archivingState);


/**
* TTV_RunCommercial - Runs a commercial on the channel
* @param[in] authToken - The authentication token previously obtained
* @param[in] callback - The callback function to be called when the web API has been called
* @param[in] userData - Optional pointer to be passed through to the callback function
*/
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_RunCommercial)(const TTV_AuthToken* authToken,
										   TTV_TaskCallback callback,
										   void* userData);

/**
* TTV_GetGameLiveStreams - Gets 25 of the current live streams of the given game
* @param[in] gameName - Name of the game to get the streams for
* @param[in] callback - The callback function to be called when the web API has been called
* @param[in] userData - Optional pointer to be passed through to the callback function
* @param[out] gameStreamList - List of the current live streams for the game
*/
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_GetGameLiveStreams)(const char* gameName,
												TTV_TaskCallback callback,
												void* userData,
												TTV_LiveGameStreamList* gameStreamList);


/**
* TTV_FreeGameLiveStreamList - Must be called after getting the list of live streams for a game to free the list
* @param[in] gameStreamList - Pointer to the TTV_LiveGameStreamList struct to be freed
*/
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_FreeGameLiveStreamList)(TTV_LiveGameStreamList* gameStreamList);


/**
* TTV_GetVolume - Get the volume level for the given device
* @param[in] device - The device to get the volume for
* @param[out] volume - The volume level (0.0 to 1.0)
*/
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_GetVolume)(TTV_AudioDeviceType device, float* volume);


/**
* TTV_SetVolume - Set the volume level for the given device
* @param[in] device - The device to set the volume for
* @param[in] volume - The volume level (0.0 to 1.0)
*/
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_SetVolume)(TTV_AudioDeviceType device, float volume);


/**
* TTV_Pause - Pause the video. The SDK will generate frames (e.g. a pause animation)
* util TTV_Unpause is called. Audio will continue to be captured/streamed; you can
* mute audio by setting the volume to <= 0. Submitting a video frame will unpause the stream.
*/
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_PauseVideo)();


/**
* TTV_Stop - Stops the stream.  This function is asynchronous if a valid callback is provided.
* @param[in] callback The callback to call when the operation has completed.  Can be null to force this function to be synchronous.
* @param[in] userData - Client data to pass along in the callback.
* @return - TTV_EC_SUCCESS if function succeeds; error code otherwise
*/
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_Stop)(TTV_TaskCallback callback, void* userData);


/**
* TTV_Shutdown - Shut down the Twitch Broadcasting SDK
* @return - TTV_EC_SUCCESS if function succeeds; error code otherwise
*/
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_Shutdown)();


/**
* TTV_SetTraceLevel - Sets the minimum threshold for what type of trace messages should be included into the log.
*		For example, if TTV_EC_ERROR is set, only error messages will be added to the log.
*		If TTV_EC_WARNING is set, both warning messages and error messages will be added to the log (since TTV_EC_ERROR passes the minimum threshold set by TTV_EC_WARNING).
* @return - TTV_EC_SUCCESS
*/
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_SetTraceLevel)(TTV_MessageLevel traceLevel);


/**
* TTV_SetTraceOutput - Sets the file path into which to log tracing.
* @return - TTV_EC_SUCCESS if function succeeds; TTV_EC_CANNOT_OPEN_FILE if the file cannot be opened.
*/
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_SetTraceOutput)(const wchar_t* outputFileName);

typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_RegisterStatsCallback)(TTV_StatCallback callback);
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_RemoveStatsCallback)(TTV_StatCallback callback);
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_PollStats)();


/**
* TTV_ErrorToString - Converts an error code into a human-readable UTF8 string.
* @param[in] err - The error code to translate.
* @return - The string for the error code.
*/
typedef const char* TTV_CALLSPEC (*TTV_ErrorToString)(TTV_ErrorCode err);


/**
* TTV_GetGameNameList - Retrieves the list of games which match the given string.  All arguments except userData must be non-null and str must not be empty.
*						When the callback is called and is successful gameList will be filled with the game information.  Be sure to free the game list
*						via TTV_FreeGameNameList.
*
* @param[in] str - The string to match.
* @param[in] callback - The callback which will be called when the result is ready.
* @param[in] games - The location the result will be written to.
* @param[in] userData - Data passed along in the callback.
* @return - TTV_EC_SUCCESS is the request is issued correctly.
*			TTV_WRN_PREV_GAME_NAME_MATCH_REQUEST_DROPPED if a previous request was dropped because they're being issues too quickly.
*			TTV_EC_INVALID_ARG if any arguments are invalid.
*/
typedef TTV_ErrorCode TTV_CALLSPEC (* TTV_GetGameNameList)(const char* str, TTV_TaskCallback callback, TTV_GameInfoList* gameList, void* userData);


/**
* TTV_FreeGameNameList - Must be called after a reply from TTV_GetGameNameList.
* @param[in] gameList - Pointer to the TTV_GameInfoList struct to be freed.
*/
typedef TTV_ErrorCode TTV_CALLSPEC (*TTV_FreeGameNameList)(TTV_GameInfoList* gameList);

#ifdef __cplusplus
}
#endif

#endif	/* TTVSDK_TWITCH_SDK_H */
