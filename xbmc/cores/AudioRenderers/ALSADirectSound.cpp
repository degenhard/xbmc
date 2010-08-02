#ifndef __APPLE__
/*
* XBMC Media Center
* Copyright (c) 2002 d7o3g4q and RUNTiME
* Portions Copyright (c) by the authors of ffmpeg and xvid
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "ALSADirectSound.h"
#include "AudioContext.h"
#include "FileSystem/SpecialProtocol.h"
#include "GUISettings.h"
#include "utils/log.h"
#include "limits.h"

#define CHECK_ALSA(l,s,e) if ((e)<0) CLog::Log(l,"%s - %s, alsa error: %d - %s",__FUNCTION__,s,e,snd_strerror(e));
#define CHECK_ALSA_RETURN(l,s,e) CHECK_ALSA((l),(s),(e)); if ((e)<0) return false;

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//***********************************************************************************************
CALSADirectSound::CALSADirectSound()
{
  m_pPlayHandle  = NULL;
  m_bIsAllocated = false;
}

bool CALSADirectSound::Initialize(IAudioCallback* pCallback, const CStdString& device, AEChLayout channelLayout, unsigned int uiSamplesPerSec, unsigned int uiBitsPerSample, bool bResample, bool bIsMusic, bool bPassthrough)
{
  /* figure out how many channels we need to open */
  static enum AEChannel ALSAChannelMap[9] =
    {AE_CH_FL, AE_CH_FR, AE_CH_BL, AE_CH_BR, AE_CH_FC, AE_CH_LFE, AE_CH_SL, AE_CH_SR, AE_CH_NULL};

  int i, c;
  int iChannels = 0;
  for(c = 0; c < 8; ++c)
    for(i = 0; channelLayout[i] != AE_CH_NULL; ++i)
      if (channelLayout[i] == ALSAChannelMap[c])
      {
        iChannels = c + 1;
        break;
      }

  if (iChannels == 0)
    return false;

  CStdString deviceuse;

  bool bAudioOnAllSpeakers(false);
  g_audioContext.SetupSpeakerConfig(iChannels, bAudioOnAllSpeakers, bIsMusic);
  g_audioContext.SetActiveDevice(CAudioContext::DIRECTSOUND_DEVICE);

  m_pPlayHandle = NULL;
  m_bPause = false;
  m_bCanPause = false;
  m_bIsAllocated = false;
  m_uiChannels = iChannels;
  m_uiSamplesPerSec = uiSamplesPerSec;
  m_uiBitsPerSample = uiBitsPerSample;
  m_bPassthrough = bPassthrough;

  snd_pcm_uframes_t dwFrameCount = 512;
  unsigned int      dwNumPackets = 16;
  snd_pcm_uframes_t dwBufferSize = 0;

  snd_pcm_hw_params_t *hw_params=NULL;
  snd_pcm_sw_params_t *sw_params=NULL;

  /* Open the device */
  int nErr;

  /* if this is first access to audio, global sound config might not be loaded */
  if(!snd_config)
    snd_config_update();

  snd_config_t *config = snd_config;
  deviceuse = device;

  nErr = snd_config_copy(&config, snd_config);
  CHECK_ALSA_RETURN(LOGERROR,"config_copy",nErr);

  if(m_bPassthrough)
  {
    /* http://www.alsa-project.org/alsa-doc/alsa-lib/group___digital___audio___interface.html */
    deviceuse += ":AES0=0x6";
    deviceuse += ",AES1=0x82";
    deviceuse += ",AES2=0x0";
    if(uiSamplesPerSec == 192000)
      deviceuse += ",AES3=0xe";
    else if(uiSamplesPerSec == 176400)
      deviceuse += ",AES3=0xc";
    else if(uiSamplesPerSec == 96000)
      deviceuse += ",AES3=0xa";
    else if(uiSamplesPerSec == 88200)
      deviceuse += ",AES3=0x8";
    else if(uiSamplesPerSec == 48000)
      deviceuse += ",AES3=0x2";
    else if(uiSamplesPerSec == 44100)
      deviceuse += ",AES3=0x0";
    else if(uiSamplesPerSec == 32000)
      deviceuse += ",AES3=0x3";
    else
      deviceuse += ",AES3=0x1";
  }
  else
  {
    if(deviceuse == "hdmi"
    || deviceuse == "iec958"
    || deviceuse == "spdif")
      deviceuse = "plug:" + deviceuse;

    if(deviceuse == "default")
      switch(iChannels)
      {
        case 8: deviceuse = "plug:surround71"; break;
        case 6: deviceuse = "plug:surround51"; break;
        case 5: deviceuse = "plug:surround50"; break;
        case 4: deviceuse = "plug:surround40"; break;
      }

    if(deviceuse != device)
    {
      snd_input_t* input;
      nErr = snd_input_stdio_open(&input, _P("special://xbmc/system/asound.conf").c_str(), "r");
      if(nErr >= 0)
      {
        nErr = snd_config_load(config, input);
        CHECK_ALSA_RETURN(LOGERROR,"config_load", nErr);

        snd_input_close(input);
        CHECK_ALSA_RETURN(LOGERROR,"input_close", nErr);
      }
      else
      {
        CLog::Log(LOGWARNING, "%s - Unable to load alsa configuration \"%s\" for device \"%s\" - %s", __FUNCTION__, "special://xbmc/system/asound.conf", deviceuse.c_str(), snd_strerror(nErr));
        deviceuse = device;
      }
    }
  }

  CLog::Log(LOGDEBUG, "%s - using alsa device %s", __FUNCTION__, deviceuse.c_str());

  nErr = snd_pcm_open_lconf(&m_pPlayHandle, deviceuse.c_str(), SND_PCM_STREAM_PLAYBACK, 0, config);

  if(nErr == -EBUSY)
  {
    // this could happen if we are in the middle of a resolution switch sometimes
    CLog::Log(LOGERROR, "%s - device %s busy retrying...", __FUNCTION__, deviceuse.c_str());
    if(m_pPlayHandle)
    {
      snd_pcm_close(m_pPlayHandle);
      m_pPlayHandle = NULL;
    }
    Sleep(200);
    nErr = snd_pcm_open_lconf(&m_pPlayHandle, deviceuse.c_str(), SND_PCM_STREAM_PLAYBACK, 0, config);
  }

  if(nErr < 0 && deviceuse != device)
  {
    CLog::Log(LOGERROR, "%s - failed to open custom device %s (error:%s), retry with default %s", __FUNCTION__, deviceuse.c_str(), snd_strerror(nErr), device.c_str());
    if(m_pPlayHandle)
    {
      snd_pcm_close(m_pPlayHandle);
      m_pPlayHandle = NULL;
    }
    nErr = snd_pcm_open_lconf(&m_pPlayHandle, device.c_str(), SND_PCM_STREAM_PLAYBACK, 0, config);

  }

  CHECK_ALSA_RETURN(LOGERROR,"pcm_open_lconf",nErr);

  snd_config_delete(config);

  /* Allocate Hardware Parameters structures and fills it with config space for PCM */
  snd_pcm_hw_params_malloc(&hw_params);

  /* Allocate Software Parameters structures and fills it with config space for PCM */
  snd_pcm_sw_params_malloc(&sw_params);

  nErr = snd_pcm_hw_params_any(m_pPlayHandle, hw_params);
  CHECK_ALSA_RETURN(LOGERROR,"hw_params_any",nErr);

  nErr = snd_pcm_hw_params_set_access(m_pPlayHandle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
  CHECK_ALSA_RETURN(LOGERROR,"hw_params_set_access",nErr);

  // always use 16 bit samples
  nErr = snd_pcm_hw_params_set_format(m_pPlayHandle, hw_params, SND_PCM_FORMAT_S16);
  CHECK_ALSA_RETURN(LOGERROR,"hw_params_set_format",nErr);

  nErr = snd_pcm_hw_params_set_rate_near(m_pPlayHandle, hw_params, &m_uiSamplesPerSec, NULL);
  CHECK_ALSA_RETURN(LOGERROR,"hw_params_set_rate",nErr);

  nErr = snd_pcm_hw_params_set_channels(m_pPlayHandle, hw_params, iChannels);
  CHECK_ALSA_RETURN(LOGERROR,"hw_params_set_channels",nErr);

  nErr = snd_pcm_hw_params_set_period_size_near(m_pPlayHandle, hw_params, &dwFrameCount, NULL);
  CHECK_ALSA_RETURN(LOGERROR,"hw_params_set_period_size",nErr);

  nErr = snd_pcm_hw_params_set_periods_near(m_pPlayHandle, hw_params, &dwNumPackets, NULL);
  CHECK_ALSA_RETURN(LOGERROR,"hw_params_set_periods",nErr);

  nErr = snd_pcm_hw_params_get_buffer_size(hw_params, &dwBufferSize);
  CHECK_ALSA_RETURN(LOGERROR,"hw_params_get_buffer_size",nErr);

  /* Assign them to the playback handle and free the parameters structure */
  nErr = snd_pcm_hw_params(m_pPlayHandle, hw_params);
  CHECK_ALSA_RETURN(LOGERROR,"snd_pcm_hw_params",nErr);

  nErr = snd_pcm_sw_params_current(m_pPlayHandle, sw_params);
  CHECK_ALSA_RETURN(LOGERROR,"sw_params_current",nErr);

  nErr = snd_pcm_sw_params_set_start_threshold(m_pPlayHandle, sw_params, INT_MAX);
  CHECK_ALSA_RETURN(LOGERROR,"sw_params_set_start_threshold",nErr);

  snd_pcm_uframes_t boundary;
  nErr = snd_pcm_sw_params_get_boundary( sw_params, &boundary );
  CHECK_ALSA_RETURN(LOGERROR,"snd_pcm_sw_params_get_boundary",nErr);

  nErr = snd_pcm_sw_params_set_silence_threshold(m_pPlayHandle, sw_params, 0 );
  CHECK_ALSA_RETURN(LOGERROR,"snd_pcm_sw_params_set_silence_threshold",nErr);

  nErr = snd_pcm_sw_params_set_silence_size( m_pPlayHandle, sw_params, boundary );
  CHECK_ALSA_RETURN(LOGERROR,"snd_pcm_sw_params_set_silence_size",nErr);

  nErr = snd_pcm_sw_params(m_pPlayHandle, sw_params);
  CHECK_ALSA_RETURN(LOGERROR,"snd_pcm_sw_params",nErr);

  snd_pcm_hw_params_free (hw_params);
  snd_pcm_sw_params_free (sw_params);


  m_bCanPause    = !!snd_pcm_hw_params_can_pause(hw_params);
  m_dwPacketSize = snd_pcm_frames_to_bytes(m_pPlayHandle, dwFrameCount);
  m_dwNumPackets = dwNumPackets;
  m_uiBufferSize = snd_pcm_frames_to_bytes(m_pPlayHandle, dwBufferSize);

  CLog::Log(LOGDEBUG, "CALSADirectSound::Initialize - packet size:%u, packet count:%u, buffer size:%u"
                    , (unsigned int)m_dwPacketSize, m_dwNumPackets, (unsigned int)dwBufferSize);

  if(m_uiSamplesPerSec != uiSamplesPerSec)
    CLog::Log(LOGWARNING, "CALSADirectSound::CALSADirectSound - requested samplerate (%d) not supported by hardware, using %d instead", uiSamplesPerSec, m_uiSamplesPerSec);


  nErr = snd_pcm_prepare (m_pPlayHandle);
  CHECK_ALSA(LOGERROR,"snd_pcm_prepare",nErr);

  switch(uiBitsPerSample)
  {
    case  8: m_format.m_dataFormat = AE_FMT_U8   ; break;
    case 16: m_format.m_dataFormat = AE_FMT_S16LE; break;
    case 32: m_format.m_dataFormat = AE_FMT_FLOAT; break;
    default:
      return false;
  }

  AEChLayout layout = new AEChannel[iChannels+1];
  for(i = 0; i < iChannels; ++i)
    layout[i] = ALSAChannelMap[i];
  layout[iChannels] = AE_CH_NULL;

  m_format.m_sampleRate    = uiSamplesPerSec;
  m_format.m_channelCount  = iChannels;
  m_format.m_channelLayout = layout;
  m_format.m_frames        = dwFrameCount;
  m_format.m_frameSamples  = m_format.m_frames * m_format.m_channelCount;
  m_format.m_frameSize     = (uiBitsPerSample / 8) * m_format.m_frameSamples;

  m_bIsAllocated = true;
  return true;
}

//***********************************************************************************************
CALSADirectSound::~CALSADirectSound()
{
  Deinitialize();
}


//***********************************************************************************************
bool CALSADirectSound::Deinitialize()
{
  if (m_bIsAllocated)
    delete[] m_format.m_channelLayout;

  m_bIsAllocated = false;
  if (m_pPlayHandle)
  {
    snd_pcm_drop(m_pPlayHandle);
    snd_pcm_close(m_pPlayHandle);
  }

  m_pPlayHandle=NULL;
  g_audioContext.SetActiveDevice(CAudioContext::DEFAULT_DEVICE);
  return true;
}

void CALSADirectSound::Flush()
{
  if (!m_bIsAllocated)
     return;

  int nErr = snd_pcm_drop(m_pPlayHandle);
  CHECK_ALSA(LOGERROR,"flush-drop",nErr);
  nErr = snd_pcm_prepare(m_pPlayHandle);
  CHECK_ALSA(LOGERROR,"flush-prepare",nErr);
}

//***********************************************************************************************
bool CALSADirectSound::Pause()
{
  if (!m_bIsAllocated)
     return -1;

  if (m_bPause) return true;
  m_bPause = true;

  if(m_bCanPause)
  {
    int nErr = snd_pcm_pause(m_pPlayHandle,1); // this is not supported on all devices.
    CHECK_ALSA(LOGERROR,"pcm_pause",nErr);
    if(nErr<0)
      m_bCanPause = false;
  }

  if(!m_bCanPause)
  {
    snd_pcm_sframes_t avail = snd_pcm_avail_update(m_pPlayHandle);
    snd_pcm_sframes_t delay = 0;
    if(avail > 0)
      delay = snd_pcm_bytes_to_frames(m_pPlayHandle, m_uiBufferSize) - avail;

    CLog::Log(LOGWARNING, "CALSADirectSound::CALSADirectSound - device is not able to pause playback, will flush and prefix with %d frames", (int)delay);
    Flush();

    if(delay > 0)
    {
      void* silence = calloc(snd_pcm_frames_to_bytes(m_pPlayHandle, delay), 1);
      int nErr = snd_pcm_writei(m_pPlayHandle, silence, delay);
      CHECK_ALSA(LOGERROR,"snd_pcm_writei", nErr);
      free(silence);
    }
  }

  return true;
}

//***********************************************************************************************
bool CALSADirectSound::Resume()
{
  if (!m_bIsAllocated)
     return -1;

  snd_pcm_state_t state = snd_pcm_state(m_pPlayHandle);
  if(state == SND_PCM_STATE_PAUSED)
    snd_pcm_pause(m_pPlayHandle,0);

  if(state == SND_PCM_STATE_PREPARED)
  {
    snd_pcm_sframes_t avail = snd_pcm_avail_update(m_pPlayHandle);
    snd_pcm_sframes_t delay = 0;
    if(avail > 0)
      delay = snd_pcm_bytes_to_frames(m_pPlayHandle, m_uiBufferSize) - avail;
    if(delay > 0)
      snd_pcm_start(m_pPlayHandle);
  }

  m_bPause = false;

  return true;
}

//***********************************************************************************************
bool CALSADirectSound::Stop()
{
  if (!m_bIsAllocated)
     return -1;

  Flush();

  m_bPause = false;

  return true;
}

//***********************************************************************************************
unsigned int CALSADirectSound::GetSpace()
{
  if (!m_bIsAllocated) return 0;

  int nSpace = snd_pcm_avail_update(m_pPlayHandle);
  if (nSpace == 0)
  {
    snd_pcm_state_t state = snd_pcm_state(m_pPlayHandle);
    if(state != SND_PCM_STATE_RUNNING && !m_bPause)
    {
      CLog::Log(LOGWARNING,"CALSADirectSound::GetSpace - buffer underun (%d)", state);
      Flush();
    }
  }
  if (nSpace < 0)
  {
     CLog::Log(LOGWARNING,"CALSADirectSound::GetSpace - get space failed. err: %d (%s)", nSpace, snd_strerror(nSpace));
     nSpace = 0;
     Flush();
  }
  return snd_pcm_frames_to_bytes(m_pPlayHandle, nSpace);
}

//***********************************************************************************************
unsigned int CALSADirectSound::AddPackets(const void* data, unsigned int len)
{
  if (!m_bIsAllocated)
  {
    CLog::Log(LOGERROR,"CALSADirectSound::AddPackets - sanity failed. no valid play handle!");
    return len;
  }
  // if we are paused we don't accept any data as pause doesn't always
  // work, and then playback would start again
  if(m_bPause)
    return 0;

  int framesToWrite;

  framesToWrite  = len / m_dwPacketSize;
  framesToWrite *= m_dwPacketSize;
  framesToWrite  = snd_pcm_bytes_to_frames(m_pPlayHandle, framesToWrite);

  if(framesToWrite == 0)
  {
    // if we haven't started playback, do so now
    if(snd_pcm_state(m_pPlayHandle) == SND_PCM_STATE_PREPARED && !m_bPause)
      snd_pcm_start(m_pPlayHandle);
    return 0;
  }

  int writeResult = snd_pcm_writei(m_pPlayHandle, data, framesToWrite);

  if (  writeResult == -EPIPE  )
  {
    CLog::Log(LOGDEBUG, "CALSADirectSound::AddPackets - buffer underun (tried to write %d frames)",
            framesToWrite);
    Flush();
    return 0;
  }
  else if (writeResult != framesToWrite)
  {
    CLog::Log(LOGERROR, "CALSADirectSound::AddPackets - failed to write %d frames. "
            "bad write (err: %d) - %s",
            framesToWrite, writeResult, snd_strerror(writeResult));
    Flush();
  }

  if (writeResult > 0)
  {
    if(snd_pcm_state(m_pPlayHandle) == SND_PCM_STATE_PREPARED && !m_bPause)
      snd_pcm_start(m_pPlayHandle);
    
    return snd_pcm_frames_to_bytes(m_pPlayHandle, writeResult);
  }

  return 0;
}

//***********************************************************************************************
float CALSADirectSound::GetDelay()
{
  if (!m_bIsAllocated)
    return 0.0;

  snd_pcm_sframes_t frames = 0;

  int nErr = snd_pcm_delay(m_pPlayHandle, &frames);
  CHECK_ALSA(LOGERROR,"snd_pcm_delay",nErr);
  if (nErr < 0)
  {
    frames = 0;
    Flush();
  }

  if (frames < 0)
  {
#if SND_LIB_VERSION >= 0x000901 /* snd_pcm_forward() exists since 0.9.0rc8 */
    snd_pcm_forward(m_pPlayHandle, -frames);
#endif
    frames = 0;
  }

  return (double)frames / m_uiSamplesPerSec;
}

float CALSADirectSound::GetCacheTime()
{
  return (float)(m_uiBufferSize - GetSpace()) / (float)m_uiBytesPerSecond;
}

float CALSADirectSound::GetCacheTotal()
{
  return (float)m_uiBufferSize / (float)m_uiBytesPerSecond;
}

//***********************************************************************************************
unsigned int CALSADirectSound::GetChunkLen()
{
  return m_dwPacketSize;
}
//***********************************************************************************************
int CALSADirectSound::SetPlaySpeed(int iSpeed)
{
  return 0;
}

void CALSADirectSound::RegisterAudioCallback(IAudioCallback *pCallback)
{
  m_pCallback = pCallback;
}

void CALSADirectSound::UnRegisterAudioCallback()
{
  m_pCallback = NULL;
}

void CALSADirectSound::WaitCompletion()
{
  if (!m_bIsAllocated || m_bPause)
    return;

  snd_pcm_wait(m_pPlayHandle, -1);
}

void CALSADirectSound::SwitchChannels(int iAudioStream, bool bAudioOnAllSpeakers)
{
    return ;
}

void CALSADirectSound::EnumerateAudioSinks(AudioSinkList& vAudioSinks, bool passthrough)
{
  if (!passthrough)
  {
    vAudioSinks.push_back(AudioSink("default", "alsa:default"));
    vAudioSinks.push_back(AudioSink("iec958" , "alsa:plug:iec958"));
    vAudioSinks.push_back(AudioSink("hdmi"   , "alsa:plug:hdmi"));
  }
  else
  {
    vAudioSinks.push_back(AudioSink("iec958" , "alsa:iec958"));
    vAudioSinks.push_back(AudioSink("hdmi"   , "alsa:hdmi"));
  }

  int n_cards = -1;
  int numberCards = 0;
  while ( snd_card_next( &n_cards ) == 0 && n_cards >= 0 )
    numberCards++;

  if (numberCards <= 1)
    return;

  snd_ctl_t *handle;
  snd_ctl_card_info_t *info;
  snd_ctl_card_info_alloca( &info );
  CStdString strHwName;
  n_cards = -1;

  while ( snd_card_next( &n_cards ) == 0 && n_cards >= 0 )
  {
    strHwName.Format("hw:%d", n_cards);
    if ( snd_ctl_open( &handle, strHwName.c_str(), 0 ) == 0 )
    {
      if ( snd_ctl_card_info( handle, info ) == 0 )
      {
        CStdString strReadableCardName = snd_ctl_card_info_get_name( info );
        CStdString strCardName = snd_ctl_card_info_get_id( info );

        if (!passthrough)
          GenSoundLabel(vAudioSinks, "default", strCardName, strReadableCardName);
        GenSoundLabel(vAudioSinks, "iec958", strCardName, strReadableCardName);
        GenSoundLabel(vAudioSinks, "hdmi", strCardName, strReadableCardName);
      }
      else
        CLog::Log(LOGERROR,"((ALSAENUM))control hardware info (%i): failed.\n", n_cards );
      snd_ctl_close( handle );
    }
    else
      CLog::Log(LOGERROR,"((ALSAENUM))control open (%i) failed.\n", n_cards );
  }
}

bool CALSADirectSound::SoundDeviceExists(const CStdString& device)
{
  void **hints, **n;
  char *name;
  bool retval = false;

  if (snd_device_name_hint(-1, "pcm", &hints) == 0)
  {
    for (n = hints; *n; n++)
    {
      if ((name = snd_device_name_get_hint(*n, "NAME")) != NULL)
      {
        CStdString strName = name;
        free(name);
        if (strName.find(device) != string::npos)
        {
          retval = true;
          break;
        }
      }
    }
    snd_device_name_free_hint(hints);
  }
  return retval;
}

void CALSADirectSound::GenSoundLabel(AudioSinkList& vAudioSinks, CStdString sink, CStdString card, CStdString readableCard)
{
  CStdString deviceString;
  deviceString.Format("%s:CARD=%s", sink, card.c_str());
  if (sink.Equals("default") || SoundDeviceExists(deviceString.c_str()))
  {
    CStdString finalSink;
    finalSink.Format("alsa:%s", deviceString.c_str());
    CStdString label = readableCard + " " + sink;
    vAudioSinks.push_back(AudioSink(label, finalSink));
  }
}
#endif
