
#pragma once

#include "utils/Thread.h"
#include "DVDMessageQueue.h"
#include "DVDDemuxers/DVDDemuxUtils.h"
#include "DVDCodecs/Video/DVDVideoCodec.h"
#include "DVDClock.h"
#include "DVDOverlayContainer.h"
#ifdef HAS_XBOX_HARDWARE
#include "../VideoRenderers/RenderManager.h"
#else
#include "../VideoRenderers/WinRenderManager.h"
#endif

enum CodecID;
class CDemuxStreamVideo;
class CDVDOverlayCodecCC;

#define VIDEO_PICTURE_QUEUE_SIZE 1

class CDVDPlayerVideo : public CThread
{
public:
  CDVDPlayerVideo(CDVDClock* pClock, CDVDOverlayContainer* pOverlayContainer);
  virtual ~CDVDPlayerVideo();

  bool OpenStream(CDVDStreamInfo &hint);
  void CloseStream(bool bWaitForBuffers);

  void StepFrame();
  void Flush();

  // waits untill all available data has been rendered
  // just waiting for packetqueue should be enough for video
  void WaitForBuffers()                             { m_messageQueue.WaitUntilEmpty(); }
  bool AcceptsData()                                { return !m_messageQueue.IsFull(); }
  void SendMessage(CDVDMsg* pMsg)                   { m_messageQueue.Put(pMsg); }

#ifdef HAS_VIDEO_PLAYBACK
  void Update(bool bPauseDrawing)                   { g_renderManager.Update(bPauseDrawing); }
#else
  void Update(bool bPauseDrawing)                   { }
#endif
  void UpdateMenuPicture();
 
  void EnableSubtitle(bool bEnable)                 { m_bRenderSubs = bEnable; }
  bool IsSubtitleEnabled()                          { return m_bRenderSubs; }

  void EnableFrameDrop(bool bEnabled)               { m_bDropFrames = bEnabled; }
  bool IsFrameDropEnabled()                         { return m_bDropFrames; }

  void EnableFullscreen(bool bEnable)               { m_bAllowFullscreen = bEnable; }
  void SetAspectRatio(float aspect)                 { m_fForcedAspectRatio = aspect; }

#ifdef HAS_VIDEO_PLAYBACK
  void GetVideoRect(RECT& SrcRect, RECT& DestRect)  { g_renderManager.GetVideoRect(SrcRect, DestRect); }
  float GetAspectRatio()                            { return g_renderManager.GetAspectRatio(); }
#else
  void GetVideoRect(RECT& SrcRect, RECT& DestRect)  { }
  float GetAspectRatio()                            { return 4.0f / 3.0f; }
#endif

  __int64 GetDelay()                                { return m_iVideoDelay; }
  void SetDelay(__int64 delay)                      { m_iVideoDelay = delay; }

  bool IsStalled()                                  { return m_DetectedStill;  }
  int GetNrOfDroppedFrames()                        { return m_iDroppedFrames; }

  bool InitializedOutputDevice();
  
  __int64 GetCurrentPts()                           { return m_iCurrentPts; }

  __int64 GetOutputDelay(); /* returns the expected delay, from that a packet is put in queue */
  string GetPlayerInfo();

  void SetSpeed(int iSpeed);

  // classes
  CDVDMessageQueue m_messageQueue;
  CDVDOverlayContainer* m_pOverlayContainer;
  
  CDVDClock* m_pClock;

protected:  
  virtual void OnStartup();
  virtual void OnExit();
  virtual void Process();

#define EOS_ABORT 1
#define EOS_DROPPED 2
#define EOS_VERYLATE 4

  int OutputPicture(DVDVideoPicture* pPicture, __int64 pts);
  void ProcessOverlays(DVDVideoPicture* pSource, YV12Image* pDest, __int64 pts);
  void ProcessVideoUserData(DVDVideoUserData* pVideoUserData, __int64 pts);
  
  __int64 m_iCurrentPts; // last pts displayed
  __int64 m_iVideoDelay; // not really needed to be an __int64  
  __int64 m_iFlipTimeStamp; // time stamp of last flippage. used to play at a forced framerate

  int m_iDroppedFrames;
  bool m_bDropFrames;

  float m_fFrameRate;

  struct SOutputConfiguration
  {
    unsigned int width;
    unsigned int height;
    unsigned int dwidth;
    unsigned int dheight;
    unsigned int color_matrix : 4;
    unsigned int color_range  : 1;
    float framerate;
  } m_output; //holds currently configured output

  bool m_bAllowFullscreen;
  bool m_bRenderSubs;
  
  float m_fForcedAspectRatio;
  
  int m_iNrOfPicturesNotToSkip;
  int m_speed;

  bool m_DetectedStill;

  /* autosync decides on how much of clock we should use when deciding sleep time */
  /* the value is the same as 63% timeconstant, ie that the step response of */
  /* iSleepTime will be at 63% of iClockSleep after autosync frames */
  unsigned int m_autosync;
  
  // classes
  CDVDVideoCodec* m_pVideoCodec;
  CDVDOverlayCodecCC* m_pOverlayCodecCC;
  
  DVDVideoPicture* m_pTempOverlayPicture;
  
  CRITICAL_SECTION m_critCodecSection;
};

