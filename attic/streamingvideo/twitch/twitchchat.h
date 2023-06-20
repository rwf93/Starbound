/********************************************************************************************
* Twitch Broadcasting SDK
*
* This software is supplied under the terms of a license agreement with Justin.tv Inc. and
* may not be copied or used except in accordance with the terms of that agreement
* Copyright (c) 2012-2013 Justin.tv Inc.
*********************************************************************************************/

#ifndef TTVSDK_TWITCH_CHAT_H
#define TTVSDK_TWITCH_CHAT_H

#include "twitchsdktypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * TTV_ChatEvent - The various events coming from the chat system.
 */
typedef enum
{
	TTV_CHAT_JOINED_CHANNEL,			//!< Local user joined a channel.
	TTV_CHAT_LEFT_CHANNEL				//!< Local user left a channel.

} TTV_ChatEvent;


/**
 * TTV_ChatUserMode - A list of the mode flags a user may have.
 */
typedef enum
{
	TTV_CHAT_USERMODE_VIEWER		= 0,		//!< A regulare viewer.

	TTV_CHAT_USERMODE_MODERATOR		= 1 << 0,	//!< A moderator.
	TTV_CHAT_USERMODE_BROADCASTER	= 1 << 1,	//!< The broadcaster.
	TTV_CHAT_USERMODE_ADMINSTRATOR	= 1 << 2,	//!< An admin.
	TTV_CHAT_USERMODE_STAFF			= 1 << 3,	//!< A member of Twitch.

	TTV_CHAT_USERMODE_BANNED		= 1 << 30	//!< The user has been banned.  This flag may not always be up to date.

} TTV_ChatUserMode;


/**
 * TTV_ChatUserSubscription - A list of the subscription flags a user may have.
 */
typedef enum
{
	TTV_CHAT_USERSUB_NONE			= 0,		//!< A standard user.

	TTV_CHAT_USERSUB_SUBSCRIBER		= 1 << 0,	//!< A subscriber in the current channel.
	TTV_CHAT_USERSUB_TURBO			= 1 << 1	//!< A Twitch Turbo account.

} TTV_ChatUserSubscription;


/**
 * This is the maximum length of the user name in bytes, NOT characters.  Be careful with multi-byte characters in UTF-8.
 * However, user names are currently restricted to ASCII which is confined to the lower 7 bits so the encodings are the same.
 */
#define kMaxChatUserNameLength 63
/**
 * TTV_ChatUserInfo - The user information.
 */
typedef struct
{
	utf8char displayName[kMaxChatUserNameLength+1];		//!< The UTF-8 encoded displayed name.  Currently restricted to the ASCII subset.
	TTV_ChatUserMode modes;								//!< The mode which controls priviledges in a particular chat room.
	TTV_ChatUserSubscription subscriptions;				//!< The user's subscriptions for the channel.
	unsigned int nameColorARGB;							//!< The current ARGB color of the user's name text.
	unsigned int emoticonSet;							//!< The emoticon set id for the user.
	bool ignore;										//!< Whether or not to ignore the user.

} TTV_ChatUserInfo;


/**
 * TTV_ChatUserList - A list of chat users.
 */
typedef struct
{
	TTV_ChatUserInfo* userList;		//!< The array of user info entries.
	unsigned int userCount;			//!< The number of entries in the array.

} TTV_ChatUserList;


/**
 * This is the maximum length of the channel name in bytes, NOT characters.  Be careful with multi-byte characters in UTF-8.
 * However, user names are currently restricted to ASCII which is confined to the lower 7 bits so the encodings happen to be the same (for now).
 */
#define kMaxChatChannelNameLength 63
/**
 * TTV_ChatChannelInfo - Information about a chat channel.
 */
typedef struct
{
	utf8char name[kMaxChatChannelNameLength+1];			//!< The UTF-8 encoded name of the channel.  Currently restricted to the ASCII subset.
	TTV_ChatUserInfo broadcasterUserInfo;				//!< Information about the broadcaster of the stream.

} TTV_ChatChannelInfo;


/**
 * This is the maximum length of the message in bytes, NOT characters.  Be careful with multi-byte characters in UTF-8.
 */
#define kMaxChatMessageLength 510
/**
 * TTV_ChatMessage - A message from a user in a chat channel.
 */
typedef struct
{
	utf8char userName[kMaxChatUserNameLength+1];		//!< The UTF-8 encoded user that sent the message.  Currently restricted to the ASCII subset.
	utf8char message[kMaxChatMessageLength+1];			//!< The UTF-8 encoded message text.
	TTV_ChatUserMode modes;								//!< The mode which controls priviledges in a particular chat room.
	TTV_ChatUserSubscription subscriptions;				//!< The user's subscriptions for the channel.
	unsigned int nameColorARGB;							//!< The ARGB color of the name text.
	unsigned int emoticonSet;							//!< The emoticon set to use when rendering emoticons.  0 is the default.
	bool action;										//!< Whether or not the message is an action.  If true, it should be displayed entirely in the name text color and of the form "<userName> <message>".

} TTV_ChatMessage;


/**
 * TTV_ChatMessageList - A list of chat messages.
 */
typedef struct
{
	TTV_ChatMessage* messageList;					//!< The ordered array of chat messages.
	unsigned int messageCount;						//!< The number of messages in the list.

} TTV_ChatMessageList;


/**
 * TTV_ChatMessageTokenType - The types of tokens that can be generated from a chat message.
 */
typedef enum
{
	TTV_CHAT_MSGTOKEN_TEXT,
	TTV_CHAT_MSGTOKEN_IMAGE

} TTV_ChatMessageTokenType;


/**
 * TTV_ChatTextMessageToken - Information about a text token.
 */
typedef struct
{
	utf8char buffer[kMaxChatMessageLength];

} TTV_ChatTextMessageToken;


/**
 * TTV_ChatImageMessageToken - Information about an image token.
 */
typedef struct
{
	int sheetIndex;			//!< The index of the sheet to use when rendering.  -1 means no image.
	unsigned short x1;		//!< The left edge in pixels on the sheet.
	unsigned short y1;		//!< The top edge in pixels on the sheet.
	unsigned short x2;		//!< The right edge in pixels on the sheet.
	unsigned short y2;		//!< The bottom edge in pixels on the sheet.

} TTV_ChatImageMessageToken;


/**
 * TTV_ChatMessageToken - Information about an image token.
 */
typedef struct
{
	TTV_ChatMessageTokenType type;	//!< The way to interpret the data.

	union
	{
		TTV_ChatTextMessageToken text;
		TTV_ChatImageMessageToken image;
	} data;

} TTV_ChatMessageToken;


/**
 * TTV_ChatTokenizedMessage - A list of tokens parsed from a text message.
 */
typedef struct
{
	utf8char displayName[kMaxChatUserNameLength+1];		//!< The UTF-8 encoded displayed name.  Currently restricted to the ASCII subset.
	TTV_ChatUserMode modes;								//!< The modes of the user who sent the message.
	TTV_ChatUserSubscription subscriptions;				//!< The subscriptions of the user who sent the message.
	unsigned int nameColorARGB;							//!< The current ARGB color of the user's name text.
 	TTV_ChatMessageToken* tokenList;					//!< The array of message tokens.
	unsigned int tokenCount;							//!< The number of entries in tokenList.

} TTV_ChatTokenizedMessage;


/**
 * TTV_ChatSpriteSheet - The texture data which can be loaded into a texture and used for rendering emoticons and badges.
 */
typedef struct
{
	unsigned int sheetIndex;		//!< The index of the sprite sheet.
	const uint8_t* buffer;			//!< The RGBA buffer data, 8 bits per channel.
	unsigned int width;				//!< The width of the buffer in pixels.
	unsigned int height;			//!< The height of the buffer in pixels.

} TTV_ChatTextureSheet;


/**
 * TTV_ChatTextureSheetList - A list of texture sheets.
 */
typedef struct
{
	TTV_ChatTextureSheet* list;			//!< The array of sheets.
	unsigned int count;					//!< The number of entries in the array.

} TTV_ChatTextureSheetList;


/**
 * TTV_ChatBadgeData - Information about how to render badges based on the subscriptions and user modes.
 */
typedef struct
{
	TTV_ChatImageMessageToken turboIcon;
	TTV_ChatImageMessageToken channelSubscriberIcon;
	TTV_ChatImageMessageToken broadcasterIcon;
	TTV_ChatImageMessageToken staffIcon;
	TTV_ChatImageMessageToken adminIcon;
	TTV_ChatImageMessageToken moderatorIcon;

} TTV_ChatBadgeData;


/**
 * TTV_ChatEmoticonData - Information to use when rendering emoticons and badges.
 */
typedef struct
{
	utf8char channel[kMaxChatChannelNameLength+1];
	TTV_ChatTextureSheetList textures;
	TTV_ChatBadgeData badges;

} TTV_ChatEmoticonData;


/**
 * Callback signature for connection and disconnection events from the chat service.  Once a connecion is successful, this may be
 * called at any time to indicate a disconnect from the server.  If a disconnection occurs, TTV_ChatConnect must be called again
 * to connect to the server and all channels must be rejoined.
 *
 * When a connection is attempted via TTV_ChatConnect, a successful connection will result in the callback being called with TTV_EC_SUCCESS.
 * Otherwise, an error will be returned which can be one of TTV_EC_CHAT_INVALID_LOGIN, TTV_EC_CHAT_LOST_CONNECTION, TTV_EC_CHAT_COULD_NOT_CONNECT.
 */
typedef void (*TTV_ChatStatusCallback) (TTV_ErrorCode result);

/**
 * Callback signature for result of the local user joining or leaving channels.
 *
 * Expected evt values are TTV_CHAT_JOINED_CHANNEL, TTV_CHAT_LEFT_CHANNEL.
 */
typedef void (*TTV_ChatChannelMembershipCallback) (TTV_ChatEvent evt, const TTV_ChatChannelInfo* channelInfo);

/**
 * Callback signature for notifications of users joining, leaving or changing their attributes in a chat channel.  The lists returned in this callback must be freed by
 * calling TTV_Chat_FreeUserList on each one when the application is done with them.
 */
typedef void (*TTV_ChatChannelUserChangeCallback) (const TTV_ChatUserList* joinList, const TTV_ChatUserList* leaveList, const TTV_ChatUserList* infoChangeList);

/**
 * Callback signature for a reply to TTV_ChatGetChannelUsers for a request for the user list for a chat channel.  The list returned in the callback must be freed by
 * calling TTV_Chat_FreeUserList when the application is done with it or it will leak each time the callback is called.
 */
typedef void (*TTV_ChatQueryChannelUsersCallback) (const TTV_ChatUserList* userList);

/**
 * Callback signature for notifications of a message event occurring in chat.  This list is freed automatically when the call to the callback returns
 * so be sure not to retain a reference to the list.
 */
typedef void (*TTV_ChatChannelMessageCallback) (const TTV_ChatMessageList* messageList);

/**
 * Callback signature for notifications that the chatroom should be cleared.
 */
typedef void (*TTV_ChatClearCallback) (const utf8char* channelName);

/**
 * Callback to indicate the result of the request to fetch emoticon data.
 *
 * @param error Specifies any error or warning that may have occurred while preparing the emoticon data.
 */
typedef void (*TTV_EmoticonDataDownloadCallback) (TTV_ErrorCode error);

/**
 * TTV_ChatCallbacks - The set of callbacks to call for notifications from the chat subsystem.
 */
typedef struct
{
	TTV_ChatStatusCallback				statusCallback;			//!< The callback to call for connection and disconnection events from the chat service. Cannot be NULL.
	TTV_ChatChannelMembershipCallback	membershipCallback;		//!< The callback to call when the local user joins or leaves a channel. Cannot be NULL.
	TTV_ChatChannelUserChangeCallback	userCallback;			//!< The callback to call when other users join or leave a channel. This may be NULL.
	TTV_ChatChannelMessageCallback		messageCallback;		//!< The callback to call when a message is received on a channel. Cannot be NULL.
	TTV_ChatClearCallback				clearCallback;			//!< The callback to call when the chatroom should be cleared. Can be NULL.

} TTV_ChatCallbacks;


/**
 * TTV_ChatInit - Sets the callbacks for receiving chat events.  This may be NULL to stop receiving chat events but this is not recommended
 *                since you may miss connection and disconnection events.
 *
 * @param[in] channelName - The UTF-8 encoded name of the channel to connect to. See #kMaxChatChannelNameLength for details.
 * @param[in] caCertFile - Full path of the CA Cert bundle file (Strongly encourage using the bundle file provided with the SDK).
 * @param[in] chatCallbacks - The set of callbacks for receiving chat events.
 * @return - TTV_EC_SUCCESS.
 */
TTVSDK_API TTV_ErrorCode TTV_Chat_Init(const utf8char* channelName, const wchar_t* caCertFile, const TTV_ChatCallbacks* chatCallbacks);

/**
 * TTV_Chat_Shutdown - Tear down the chat subsystem. Be sure to have freed all outstanding lists before calling this.
 *
 * @return - TTV_EC_SUCCESS.
 */
TTVSDK_API TTV_ErrorCode TTV_Chat_Shutdown();

/**
 * TTV_ChatConnect - Connects to the chat service.  This is an asynchronous call and notification of
 *					 connection success or fail will come in the callback.   TTV_Chat_Init should be called first with
 *                   valid callbacks for receiving connection and disconnection events.  The actual result of the connection attempt will
 *					 come in the statusCallback.
 *
 * @param[in] username - The UTF-8 encoded account username to use for logging in to chat.  See #kMaxChatUserNameLength for details.
 * @param[in] authToken - The auth token for the account.
 * @return - TTV_EC_SUCCESS if the request to connect is valid (does not guarantee connection, wait for a response from statusCallback).
 *			 TTV_EC_CHAT_NOT_INITIALIZED if system not initialized.
 *			 TTV_EC_CHAT_ALREADY_IN_CHANNEL if already in channel.
 *			 TTV_EC_CHAT_LEAVING_CHANNEL if still leaving a channel.
 */
TTVSDK_API TTV_ErrorCode TTV_Chat_Connect(const utf8char* username, const TTV_AuthToken* authToken);

/**
 * TTV_Chat_ConnectAnonymous - Connects to the chat service anonymously allowing chat messages to be received but not sent.  This is an asynchronous
 *					call and notification of connection success or fail will come in the callback.   TTV_Chat_Init should be called first with
 *                   valid callbacks for receiving connection and disconnection events.  The actual result of the connection attempt will
 *					 come in the statusCallback.
 *
 * @return - TTV_EC_SUCCESS if the request to connect is valid (does not guarantee connection, wait for a response from statusCallback).
 *			 TTV_EC_CHAT_NOT_INITIALIZED if system not initialized.
 *			 TTV_EC_CHAT_ALREADY_IN_CHANNEL if already in channel.
 *			 TTV_EC_CHAT_LEAVING_CHANNEL if still leaving a channel.
 */
TTVSDK_API TTV_ErrorCode TTV_Chat_ConnectAnonymous();

/**
 * TTV_ChatDisconnect - Disconnects from the chat server.  This will automatically remove the user from
 *						all channels that the user is in.  A notification will come in statusCallback to indicate
 *						that the disconnection was successful but you can safely assume this will succeed.
 *
 * @return - TTV_EC_SUCCESS if disconnection successful.
 *			 TTV_EC_CHAT_NOT_INITIALIZED if system not initialized.
 */
TTVSDK_API TTV_ErrorCode TTV_Chat_Disconnect();

/**
 * TTV_ChatGetChannelUsers - Retrieves the current users for the named channel.  This is used by both broadcasters and viewers.
 *							 The list returned in the callback must be freed by calling TTV_Chat_FreeUserList when the application is done with it.
 *
 * @param[in] callback - The callback to call with the result of the query.
 * @return - TTV_EC_SUCCESS if function succeeds.
 *			 TTV_EC_CHAT_NOT_IN_CHANNEL if not in the channel.
 *			 TTV_EC_CHAT_NOT_INITIALIZED if system not initialized.
 */
TTVSDK_API TTV_ErrorCode TTV_Chat_GetChannelUsers(TTV_ChatQueryChannelUsersCallback callback);

/**
 * TTV_Chat_SendMessage - Sends the given message to the channel.  The user must have joined the channel first.
 *						  This is used by both broadcasters and viewers.
 *
 *						  The game/user may also send some commands using a message to take some action in the channel.  The valid commands are:
 *
 *						  "/disconnect":			Disconnects from the chat channel.
 *						  "/commercial":			Make all viewers who normally watch commercials see a commercial right now.
 *						  "/mods":					Get a list of all of the moderators in the current channel.
 *						  "/mod <username>":		Grant moderator status to the given user.
 *						  "/unmod <username>":		Revoke moderator status from the given user.
 *						  "/ban: <username>":		Ban the given user from your channel.
 *						  "/unban <username>":		Lift a ban or a time-out that has been given to the given user.
 *						  "/clear":					Clear the current chat history.  This clears the chat room for all viewers.
 *						  "/timeout <username>":	Give a time-out to the given user.
 *						  "/subscribers":			Turn on subscribers-only mode, which keeps people who have not purchased channel subscriptions to this channel from talking in chat.
 *						  "/subscribersoff":		Turn off subscribers-only mode.
 *						  "/slow <interval>":		Require that chatters wait <interval> seconds between lines of chat.
 *						  "/slowoff":				Don't require that chatters wait between lines of chat anymore.
 *						  "/fastsubs <on|off>":		Makes subscribers exempt from /slow.
 *						  "/me":					Speak in the third person. Ex: /me want smash -> <my_username> want smash.  The entire message should also be colored with the user color.
 *						  "/color":					Change the color of your username.
 *						  "/ignore <username>":		Ignores the named user.
 *						  "/unignore <username>":	Unignores the named user.
 *
 * @param[in] message - The UTF-8 encoded message to send to the channel.
 * @return - TTV_EC_SUCCESS if function succeeds.
 *			 TTV_EC_CHAT_NOT_INITIALIZED if system not initialized.
 *			 TTV_EC_CHAT_ANON_DENIED if connected anonymously.
 */
TTVSDK_API TTV_ErrorCode TTV_Chat_SendMessage(const utf8char* message);

/**
 * TTV_ChatFlushEvents - Calls callbacks for all events which has accumulated since the last flush.  This include connects, disconnects,
 *						 user changes and received messages.
 *
 * @return - TTV_EC_SUCCESS if function succeeds.
 *			 TTV_EC_CHAT_NOT_INITIALIZED if system not initialized.
 */
TTVSDK_API TTV_ErrorCode TTV_Chat_FlushEvents();

/**
 * TTV_Chat_FreeUserList - Frees the memory for the given user list which was passed to the application during a callback.
 *
 * @param[in] userList - The user list to free.
 * @return - TTV_EC_SUCCESS if function succeeds.
 */
TTVSDK_API TTV_ErrorCode TTV_Chat_FreeUserList(const TTV_ChatUserList* userList);

/**
 * TTV_Chat_TokenizeMessage - Takes a raw text message, parses it and breaks it into tokens which makes it easier to render messages with embedded emoticons.
 *							  Badges for the sending user can be rendered using TTV_ChatBadgeData obtained from a call to TTV_Chat_GetBadgeData.
 *
 * @param[in] message - The message to tokenize.
 * @param[out] tokenizedMessage - The data for the tokenized message.  This must be freed by calling TTV_Chat_FreeTokenizeMessage.
 * @return - TTV_EC_SUCCESS if function succeeds, TTV_EC_INVALID_ARG if the message is not valid.
 */
TTVSDK_API TTV_ErrorCode TTV_Chat_TokenizeMessage(const TTV_ChatMessage* message, TTV_ChatTokenizedMessage** tokenizedMessage);

/**
 * TTV_Chat_FreeTokenizedMessage - Frees the memory allocated during a call to TTV_Chat_TokenizeMessage.
 *
 * @param[in] tokenizedMessage - The data to free.
 * @return - TTV_EC_SUCCESS if function succeeds, TTV_EC_INVALID_ARG if the message is not expected.
 */
TTVSDK_API TTV_ErrorCode TTV_Chat_FreeTokenizedMessage(TTV_ChatTokenizedMessage* tokenizedMessage);

/**
 * TTV_Chat_DownloadEmoticonData - Initiates a download of the emoticon data.  This will trigger a redownload if called a second time.  The callback will be called
 *								   to indicate the success of the download.  Call TTV_Chat_GetEmoticonDatato retrieve the data after it has
 *								   been downloaded.
 *
 * @param[in] callback - The callback to call when the emoticon data has finished downloading and is prepared for use.
 * @return - TTV_EC_SUCCESS if function succeeds
 *			 TTV_EC_CHAT_EMOTICON_DATA_DOWNLOADING if the data is still downloading.
 *			 TTV_EC_CHAT_EMOTICON_DATA_LOCKED if the data has been locked by a call to TTV_Chat_GetEmoticonData and has not yet been freed by TTV_Chat_FreeEmoticonData.
 *			 TTV_EC_INVALID_ARG if an invalid callback.
 */
TTVSDK_API TTV_ErrorCode TTV_Chat_DownloadEmoticonData(TTV_EmoticonDataDownloadCallback callback);

/**
 * TTV_Chat_GetEmoticonData - Retrieves the texture information and badge info after it has been downloaded and prepared. When done with this data be sure
 *							  to call TTV_Chat_FreeEmoticonData to free the memory.  Initiate the download by calling TTV_Chat_DownloadEmoticonData.
 *
 * @param[out] textureSheetList - The texture information that will be returned.
 * @return - TTV_EC_SUCCESS if function succeeds.
 *			 TTV_EC_CHAT_EMOTICON_DATA_DOWNLOADING if the data is still downloading.
 *			 TTV_EC_CHAT_EMOTICON_DATA_NOT_READY if the data is not yet ready to be retrieved.
 */
TTVSDK_API TTV_ErrorCode TTV_Chat_GetEmoticonData(TTV_ChatEmoticonData** data);

/**
 * TTV_Chat_FreeEmoticonData - Frees the data previously obtained from TTV_Chat_GetEmoticonData.
 *
 * @param[in] textureSheetList - The texture information that will be returned.
 * @return - TTV_EC_SUCCESS if function succeeds.
 *			 TTV_EC_INVALID_ARG if not a previously retrieved list.
 */
TTVSDK_API TTV_ErrorCode TTV_Chat_FreeEmoticonData(TTV_ChatEmoticonData* data);


#ifdef __cplusplus
}
#endif

#endif	/* TTVSDK_TWITCH_CHAT_H */
