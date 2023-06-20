#include "StarTwitchStreamingVideoController.hpp"

#include "twitchsdk.h"

#include "StarDynamicLib.hpp"
#include "StarFile.hpp"

namespace Star {

const char* twitchClientId = "cuxxi4cir240k3e95tfhdkjt1mtwe4f";
const char* twitchClientSecret = "8zdy461iyv1kiom0ejszqxq85djbqxr";

float const WorldTimestep = 1.0f / 60.0f;

enum class TwitchStreamingVideoControllerMode {
  None, Initialize, WaitingForAuth, Authenticated, LoggingIn, LoggedIn, FindingIngestServer, FoundIngestServer, Start, Streaming, Cleanup

};

class TwitchStreamingVideoControllerInternals {
public:
  DynamicLibPtr lib;

  TTV_AuthToken authToken;       // The unique key that allows the client to stream.
  TTV_ChannelInfo channelInfo;     // The information about the channel associated with the auth token
  TTV_IngestList ingestList;       // Will contain valid data the callback triggered due to TTV_GetIngestServers is called.
  TTV_UserInfo userInfo;         // Profile information about the local user.
  TTV_StreamInfo streamInfo;       // Information about the stream the user is streaming on.
  TTV_IngestServer ingestServer;     // The ingest server to use.

  std::vector<unsigned char*> freeBufferList;  // The list of free buffers.  The app needs to allocate exactly 3.
  std::vector<unsigned char*> captureBuffers;  // The list of all buffers.

  String username;
  String password;
  String clientSecret;

  TwitchStreamingVideoControllerMode mode;

  List<String> errors;
  List<String> status;

  Variant configuration;

  Vec2U outputSize;

  TTV_ErrorToString fTTV_ErrorToString;
  TTV_Init fTTV_Init;
  TTV_RequestAuthToken fTTV_RequestAuthToken;
  TTV_Login fTTV_Login;
  TTV_FreeIngestList fTTV_FreeIngestList;
  TTV_GetIngestServers fTTV_GetIngestServers;
  TTV_GetUserInfo fTTV_GetUserInfo;
  TTV_GetStreamInfo fTTV_GetStreamInfo;
  TTV_GetDefaultParams fTTV_GetDefaultParams;
  TTV_Start fTTV_Start;
  TTV_Stop fTTV_Stop;
  TTV_Shutdown fTTV_Shutdown;
  TTV_SubmitVideoFrame fTTV_SubmitVideoFrame;
  TTV_PollTasks fTTV_PollTasks;
  TTV_PollStats fTTV_PollStats;
  TTV_SetTraceLevel fTTV_SetTraceLevel;
  TTV_SetTraceOutput fTTV_SetTraceOutput;
};

shared_ptr<TwitchStreamingVideoControllerInternals> g_internals;

void AuthDoneCallback(TTV_ErrorCode result, void* userData) {
  _unused(userData);
  if ( TTV_SUCCEEDED(result)) {
    g_internals->mode = TwitchStreamingVideoControllerMode::Authenticated;
  } else {
    const char* err = g_internals->fTTV_ErrorToString(result);
    g_internals->errors.append(strf("AuthDoneCallback got failure: %s", err));
    g_internals->mode = TwitchStreamingVideoControllerMode::Cleanup;
  }
}

void LoginCallback(TTV_ErrorCode result, void* userData) {
  _unused(userData);
  if ( TTV_SUCCEEDED(result)) {
    g_internals->mode = TwitchStreamingVideoControllerMode::LoggedIn;
  } else {
    const char* err = g_internals->fTTV_ErrorToString(result);
    g_internals->errors.append(strf("LoginCallback got failure: %s", err));
    g_internals->mode = TwitchStreamingVideoControllerMode::Cleanup;
  }
}

void IngestListCallback(TTV_ErrorCode result, void* userData) {
  _unused(userData);
  if ( TTV_SUCCEEDED(result)) {
    uint serverIndex = 0;
    for (uint i = 0; i < g_internals->ingestList.ingestCount; ++i) {
      // Use the default server for now
      if (g_internals->ingestList.ingestList[i].defaultServer) {
        serverIndex = i;
        break;
      }
    }

    g_internals->ingestServer = g_internals->ingestList.ingestList[serverIndex];
    g_internals->mode = TwitchStreamingVideoControllerMode::FoundIngestServer;
  } else {
    const char* err = g_internals->fTTV_ErrorToString(result);
    g_internals->errors.append(strf("IngestListCallback got failure: %s", err));
    g_internals->mode = TwitchStreamingVideoControllerMode::Cleanup;
  }

  g_internals->fTTV_FreeIngestList(&g_internals->ingestList);
}

void UserInfoDoneCallback(TTV_ErrorCode result, void* /*userData*/) {
  if ( TTV_FAILED(result)) {
    const char* err = g_internals->fTTV_ErrorToString(result);
    g_internals->errors.append(strf("UserInfoDoneCallback got failure: %s", err));
    g_internals->mode = TwitchStreamingVideoControllerMode::Cleanup;
  }
}

void StreamInfoDoneCallback(TTV_ErrorCode result, void* /*userData*/) {
  if ( TTV_FAILED(result) && result != TTV_EC_WEBAPI_RESULT_NO_STREAMINFO) {
    const char* err = g_internals->fTTV_ErrorToString(result);
    g_internals->errors.append(strf("StreamInfoDoneCallback got failure: %s", err));
    g_internals->mode = TwitchStreamingVideoControllerMode::Cleanup;
  }
}

void FrameUnlockCallback(const uint8_t* buffer, void* /*userData*/) {
  unsigned char* p = const_cast<unsigned char*>(buffer);
  g_internals->freeBufferList.push_back(p);
}

TwitchStreamingVideoController::TwitchStreamingVideoController() {
  m_internals = make_shared<TwitchStreamingVideoControllerInternals>();
  g_internals = m_internals;
  m_internals->mode = TwitchStreamingVideoControllerMode::None;
  m_timer = 0;

#ifdef WIN32
  m_internals->lib = DynamicLib::loadLibrary("twitchsdk.dll");
#endif

  m_internals->fTTV_ErrorToString = (TTV_ErrorToString)m_internals->lib->funcPtr("TTV_ErrorToString");
  m_internals->fTTV_Init = (TTV_Init)m_internals->lib->funcPtr("TTV_Init");
  m_internals->fTTV_RequestAuthToken = (TTV_RequestAuthToken)m_internals->lib->funcPtr("TTV_RequestAuthToken");
  m_internals->fTTV_Login = (TTV_Login)m_internals->lib->funcPtr("TTV_Login");
  m_internals->fTTV_FreeIngestList = (TTV_FreeIngestList)m_internals->lib->funcPtr("TTV_FreeIngestList");
  m_internals->fTTV_GetIngestServers = (TTV_GetIngestServers)m_internals->lib->funcPtr("TTV_GetIngestServers");
  m_internals->fTTV_GetUserInfo = (TTV_GetUserInfo)m_internals->lib->funcPtr("TTV_GetUserInfo");
  m_internals->fTTV_GetStreamInfo = (TTV_GetStreamInfo)m_internals->lib->funcPtr("TTV_GetStreamInfo");
  m_internals->fTTV_GetDefaultParams = (TTV_GetDefaultParams)m_internals->lib->funcPtr("TTV_GetDefaultParams");
  m_internals->fTTV_Start = (TTV_Start)m_internals->lib->funcPtr("TTV_Start");
  m_internals->fTTV_Stop = (TTV_Stop)m_internals->lib->funcPtr("TTV_Stop");
  m_internals->fTTV_Shutdown = (TTV_Shutdown)m_internals->lib->funcPtr("TTV_Shutdown");
  m_internals->fTTV_SubmitVideoFrame = (TTV_SubmitVideoFrame)m_internals->lib->funcPtr("TTV_SubmitVideoFrame");
  m_internals->fTTV_PollTasks = (TTV_PollTasks)m_internals->lib->funcPtr("TTV_PollTasks");
  m_internals->fTTV_PollStats = (TTV_PollStats)m_internals->lib->funcPtr("TTV_PollStats");
  m_internals->fTTV_SetTraceLevel = (TTV_SetTraceLevel)m_internals->lib->funcPtr("TTV_SetTraceLevel");
  m_internals->fTTV_SetTraceOutput = (TTV_SetTraceOutput)m_internals->lib->funcPtr("TTV_SetTraceOutput");
}

TwitchStreamingVideoController::~TwitchStreamingVideoController() {}

void TwitchStreamingVideoController::setStreamConfiguration(Variant const& configuration) {
  m_internals->configuration = configuration;
}

bool TwitchStreamingVideoController::active() const {
  return (m_internals->mode != TwitchStreamingVideoControllerMode::None);
}

void TwitchStreamingVideoController::update() {
  TTV_ErrorCode ret;
  if ((m_internals->mode != TwitchStreamingVideoControllerMode::None) && (m_internals->mode !=TwitchStreamingVideoControllerMode::Initialize)) {
    ret = m_internals->fTTV_PollTasks();
    if (TTV_FAILED(ret)) {
      const char* err = g_internals->fTTV_ErrorToString(ret);
      m_internals->errors.append(strf("Error while polling tasks: %s", err));
//      return;
    }
    ret = m_internals->fTTV_PollStats();
    if (TTV_FAILED(ret)) {
      const char* err = g_internals->fTTV_ErrorToString(ret);
      m_internals->errors.append(strf("Error while polling stats: %s", err));
//      return;
    }
  }
  switch (m_internals->mode) {
    case TwitchStreamingVideoControllerMode::None: {
      break;
    }
    case TwitchStreamingVideoControllerMode::Initialize: {

      String clientId = twitchClientId;
      String caCertPath = File::fullPath(m_internals->configuration.getString("caCertPath"));
      String dllLoadPath = File::fullPath(m_internals->configuration.getString("dllLoadPath"));

      // Initialize the SDK
      std::wstring caCertPathW = caCertPath.wstring();
      std::wstring dllLoadPathW = dllLoadPath.wstring();

      wchar_t* caCertPathWptr = (wchar_t*)caCertPathW.c_str();
      wchar_t* dllLoadPathWptr = (wchar_t*)dllLoadPathW.c_str();
      ret = g_internals->fTTV_Init(nullptr, clientId.utf8Ptr(), caCertPathWptr, TTV_VID_ENC_DEFAULT, dllLoadPathWptr);

      if ( TTV_FAILED(ret)) {
        const char* err = g_internals->fTTV_ErrorToString(ret);
        m_internals->errors.append(strf("Error while initializing the Twitch SDK: %s", err));
        m_internals->mode = TwitchStreamingVideoControllerMode::None;
        return;
      }


      if (m_internals->configuration.getBool("tracing", false)) {
        auto trace = File::fullPath(m_internals->configuration.getString("traceFile"));
        std::wstring traceFileW = trace.wstring();
        wchar_t* traceFileWPtr = (wchar_t*)traceFileW.c_str();
        m_internals->fTTV_SetTraceOutput(traceFileWPtr);
        m_internals->fTTV_SetTraceLevel(TTV_MessageLevel::TTV_ML_DEBUG);
      }

      m_internals->username = m_internals->configuration.getString("username");
      m_internals->password = m_internals->configuration.getString("password");
      m_internals->clientSecret = twitchClientSecret;

      // Obtain the AuthToken which will allow the user to stream
      TTV_AuthParams authParams;
      authParams.size = sizeof(TTV_AuthParams);
      authParams.userName = m_internals->username.utf8Ptr();
      authParams.password = m_internals->password.utf8Ptr();
      authParams.clientSecret = m_internals->clientSecret.utf8Ptr();

      m_internals->mode = TwitchStreamingVideoControllerMode::WaitingForAuth;

      ret = g_internals->fTTV_RequestAuthToken(&authParams, AuthDoneCallback, nullptr, &m_internals->authToken);
      if ( TTV_FAILED(ret)) {
        const char* err = g_internals->fTTV_ErrorToString(ret);
        m_internals->errors.append(strf("Error while requesting auth token: %s", err));
        m_internals->mode = TwitchStreamingVideoControllerMode::Cleanup;
        return;
      }
      break;
    }
    case TwitchStreamingVideoControllerMode::WaitingForAuth: {
      break;
    }
    case TwitchStreamingVideoControllerMode::Authenticated: {
      m_internals->mode = TwitchStreamingVideoControllerMode::LoggingIn;
      m_internals->channelInfo.size = sizeof(m_internals->channelInfo);
      ret = g_internals->fTTV_Login(&m_internals->authToken, LoginCallback, nullptr, &m_internals->channelInfo);
      if (TTV_FAILED(ret)) {
        const char* err = g_internals->fTTV_ErrorToString(ret);
        m_internals->errors.append(strf("Error while trying to login: %s", err));
        m_internals->mode = TwitchStreamingVideoControllerMode::Cleanup;
        return;
      }
      break;
    }
    case TwitchStreamingVideoControllerMode::LoggingIn: {
      break;
    }
    case TwitchStreamingVideoControllerMode::LoggedIn: {
      m_internals->mode = TwitchStreamingVideoControllerMode::FindingIngestServer;
      ret = g_internals->fTTV_GetIngestServers(&m_internals->authToken, IngestListCallback, nullptr, &m_internals->ingestList);
      if (TTV_FAILED(ret)) {
        const char* err = g_internals->fTTV_ErrorToString(ret);
        m_internals->errors.append(strf("Error while trying to fetch ingest servers: %s", err));
        m_internals->mode = TwitchStreamingVideoControllerMode::Cleanup;
        return;
      }
      break;
    }
    case TwitchStreamingVideoControllerMode::FindingIngestServer: {
      break;
    }
    case TwitchStreamingVideoControllerMode::FoundIngestServer: {
      m_internals->mode = TwitchStreamingVideoControllerMode::Start;
      m_internals->userInfo.size = sizeof(m_internals->userInfo);
      m_internals->streamInfo.size = sizeof(m_internals->streamInfo);

      // Kick off requests for the user and stream information that aren't 100% essential to be ready before streaming starts
      ret = g_internals->fTTV_GetUserInfo(&m_internals->authToken, UserInfoDoneCallback, nullptr, &m_internals->userInfo);
      if (TTV_FAILED(ret)) {
        const char* err = g_internals->fTTV_ErrorToString(ret);
        m_internals->errors.append(strf("Error while trying to fetch user info: %s", err));
        m_internals->mode = TwitchStreamingVideoControllerMode::Cleanup;
        return;
      }
      ret = g_internals->fTTV_GetStreamInfo(&m_internals->authToken, StreamInfoDoneCallback, nullptr, m_internals->username.utf8Ptr(), &m_internals->streamInfo);
      if (TTV_FAILED(ret)) {
        const char* err = g_internals->fTTV_ErrorToString(ret);
        m_internals->errors.append(strf("Error while trying to fetch stream info: %s", err));
        m_internals->mode = TwitchStreamingVideoControllerMode::Cleanup;
        return;
      }
      break;
    }
    case TwitchStreamingVideoControllerMode::Start: {
      TTV_ErrorCode ret;
      TTV_VideoParams videoParams;
      memset(&videoParams, 0, sizeof(TTV_VideoParams));
      videoParams.size = sizeof(TTV_VideoParams);
      videoParams.outputWidth = m_internals->outputSize[0];
      videoParams.outputHeight = m_internals->outputSize[1];
      videoParams.targetFps = m_internals->configuration.getUInt("targetFps");
      videoParams.verticalFlip = m_internals->configuration.getBool("verticalFlip");

      // Compute the rest of the fields based on the given parameters
      ret = g_internals->fTTV_GetDefaultParams(&videoParams);
      if ( TTV_FAILED(ret)) {
        const char* err = g_internals->fTTV_ErrorToString(ret);
        m_internals->errors.append(strf("Error while preparing to start the stream: %s", err));
        m_internals->mode = TwitchStreamingVideoControllerMode::Cleanup;
        return;
      }
      videoParams.pixelFormat = TTV_PF_BGRA;

      // Setup the audio parameters
      TTV_AudioParams audioParams;
      memset(&audioParams, 0, sizeof(TTV_AudioParams));
      audioParams.size = sizeof(TTV_AudioParams);
      audioParams.audioEnabled = m_internals->configuration.getBool("audioEnabled");

      ret = g_internals->fTTV_Start(&videoParams, &audioParams, &m_internals->ingestServer, 0, nullptr, nullptr);
      if ( TTV_FAILED(ret)) {
        const char* err = g_internals->fTTV_ErrorToString(ret);
        m_internals->errors.append(strf("Error while starting to stream: %s", err));
        m_internals->mode = TwitchStreamingVideoControllerMode::Cleanup;
        return;
      }

      m_internals->mode = TwitchStreamingVideoControllerMode::Streaming;

      // Allocate exactly 3 buffers to use as the capture destination while streaming.
      // These buffers are passed to the SDK.
      for (unsigned int i = 0; i < 3; ++i) {
        unsigned char* pBuffer = new unsigned char[videoParams.outputWidth * videoParams.outputHeight * 4];
        m_internals->captureBuffers.push_back(pBuffer);
        m_internals->freeBufferList.push_back(pBuffer);
      }

      break;
    }
    case TwitchStreamingVideoControllerMode::Streaming: {
      m_timer -= WorldTimestep;
      if (m_timer < 0) {
        ret = g_internals->fTTV_GetStreamInfo(&m_internals->authToken, StreamInfoDoneCallback, nullptr, m_internals->username.utf8Ptr(), &m_internals->streamInfo);
        if (TTV_FAILED(ret)) {
          const char* err = g_internals->fTTV_ErrorToString(ret);
          m_internals->errors.append(strf("Error while trying to fetch stream info on timer: %s", err));
          m_internals->mode = TwitchStreamingVideoControllerMode::Cleanup;
          return;
        }
        m_timer = m_internals->configuration.getFloat("refreshTimer");
      }
      break;
    }
    case TwitchStreamingVideoControllerMode::Cleanup: {
      m_internals->mode = TwitchStreamingVideoControllerMode::None;
      TTV_ErrorCode ret = g_internals->fTTV_Stop(nullptr, nullptr);
      if ( TTV_FAILED(ret)) {
        const char* err = g_internals->fTTV_ErrorToString(ret);
        m_internals->errors.append(strf("Error while stopping the stream: %s", err));
      }

      // Delete the capture buffers
      for (unsigned int i = 0; i < m_internals->captureBuffers.size(); ++i) {
        delete[] m_internals->captureBuffers[i];
      }
      m_internals->freeBufferList.clear();
      m_internals->captureBuffers.clear();

      ret = g_internals->fTTV_Shutdown();
      if ( TTV_FAILED(ret)) {
        const char* err = g_internals->fTTV_ErrorToString(ret);
        m_internals->errors.append(strf("Error while shutting down the Twitch SDK: %s", err));
        return;
      }
      break;
    }
  }
}

void TwitchStreamingVideoController::start() {
  if (m_internals->mode != TwitchStreamingVideoControllerMode::None) {
    m_internals->errors.append("Already started.");
  }
  else {
    m_internals->mode = TwitchStreamingVideoControllerMode::Initialize;
  }
}

void TwitchStreamingVideoController::stop() {
  if (active())
    m_internals->mode = TwitchStreamingVideoControllerMode::Cleanup;
}

bool TwitchStreamingVideoController::hasError() {
  return m_internals->errors.size() != 0;
}

String TwitchStreamingVideoController::nextError() {
  return m_internals->errors.takeFirst();
}

bool TwitchStreamingVideoController::hasStatus() {
  return m_internals->status.size() != 0;
}

String TwitchStreamingVideoController::nextStatus() {
  return m_internals->status.takeFirst();
}

void TwitchStreamingVideoController::setFrameBufferSize(Vec2U const& size) {
  auto outputSize = Vec2U((size[0] /32) *32, (size[1] /32) *32);
  if (m_internals->outputSize != outputSize) {
    m_internals->outputSize = outputSize;
    if (active())
      stop();
  }
}

void TwitchStreamingVideoController::setStreamMetadata(Variant const& metadata) {
  _unused(metadata);
}

bool TwitchStreamingVideoController::nextFrameExpected() {
  return m_internals->freeBufferList.size() != 0;
}

StreamingVideoFrameBuffer TwitchStreamingVideoController::acquireFrameBuffer() {
  StreamingVideoFrameBuffer result;
  result.width = m_internals->outputSize[0];
  result.height = m_internals->outputSize[1];
  result.buffer = m_internals->freeBufferList.back();
  m_internals->freeBufferList.pop_back();
  return result;
}

void TwitchStreamingVideoController::submitFrameBuffer(StreamingVideoFrameBuffer buffer) {
  TTV_ErrorCode ret = g_internals->fTTV_SubmitVideoFrame((uint8_t*)buffer.buffer, FrameUnlockCallback, 0);
  if ( TTV_FAILED(ret)) {
    m_internals->mode = TwitchStreamingVideoControllerMode::Cleanup;
    const char* err = g_internals->fTTV_ErrorToString(ret);
    m_internals->errors.append(strf("Error while submitting frame to stream: %s", err));
  }
}

String TwitchStreamingVideoController::kind() const {
  return "twitch";
}

}

