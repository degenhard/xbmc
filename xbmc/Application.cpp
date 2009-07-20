/*
 *      Copyright (C) 2005-2008 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "stdafx.h"
#include "Application.h"
#include "KeyboardLayoutConfiguration.h"
#ifdef HAS_XBOX_HARDWARE
#include "xbox/XKEEPROM.h"
#include "utils/LCD.h"
#include "xbox/IoSupport.h"
#include "xbox/XKHDD.h"
#endif
#include "xbox/xbeheader.h"
#include "Util.h"
#include "TextureManager.h"
#include "cores/PlayerCoreFactory.h"
#include "cores/dvdplayer/DVDFileInfo.h"
#include "PlayListPlayer.h"
#include "Autorun.h"
#include "ActionManager.h"
#ifdef HAS_LCD
#include "utils/LCDFactory.h"
#else
#include "GUILabelControl.h"  // needed for CInfoLabel
#include "guiImage.h"
#endif
#ifdef HAS_XBOX_HARDWARE
#include "utils/MemoryUnitManager.h"
#include "utils/FanController.h"
#include "utils/LED.h"
#endif
#include "XBVideoConfig.h"
#include "LangCodeExpander.h"
#include "utils/GUIInfoManager.h"
#include "PlayListFactory.h"
#include "GUIFontManager.h"
#include "GUIColorManager.h"
#include "SkinInfo.h"
#include "lib/libPython/XBPython.h"
#include "ButtonTranslator.h"
#include "GUIAudioManager.h"
#include "lib/libscrobbler/lastfmscrobbler.h"
#include "lib/libscrobbler/librefmscrobbler.h"
#include "GUIPassword.h"
#include "ApplicationMessenger.h"
#include "SectionLoader.h"
#include "cores/DllLoader/DllLoaderContainer.h"
#include "GUIUserMessages.h"
#include "FileSystem/DirectoryCache.h"
#include "FileSystem/StackDirectory.h"
#include "FileSystem/SpecialProtocol.h"
#include "FileSystem/DllLibCurl.h"
#include "FileSystem/CMythSession.h"
#include "FileSystem/PluginDirectory.h"
#ifdef HAS_FILESYSTEM_SAP
#include "FileSystem/SAPDirectory.h"
#endif
#include "FileSystem/HTSPDirectory.h"
#include "utils/TuxBoxUtil.h"
#include "utils/SystemInfo.h"
#include "ApplicationRenderer.h"
#include "GUILargeTextureManager.h"
#include "LastFmManager.h"
#include "SmartPlaylist.h"
#include "FileSystem/RarManager.h"
#include "PlayList.h"
#include "utils/DownloadQueueManager.h"

#ifdef HAS_FILESYSTEM
#include "FileSystem/FileDAAP.h"
#endif
#ifdef HAS_UPNP
#include "UPnP.h"
#include "FileSystem/UPnPDirectory.h"
#endif
#include "PartyModeManager.h"
#ifdef HAS_VIDEO_PLAYBACK
#include "cores/VideoRenderers/RenderManager.h"
#endif
#ifdef HAS_KARAOKE
#include "CdgParser.h"
#endif
#include "AudioContext.h"
#include "GUIFontTTF.h"
#include "xbox/network.h"
#include "utils/Win32Exception.h"
#include "lib/libGoAhead/XBMChttp.h"
#include "lib/libGoAhead/WebServer.h"
#ifdef HAS_FTP_SERVER
#include "lib/libfilezilla/xbfilezilla.h"
#endif
#ifdef HAS_TIME_SERVER
#include "utils/Sntp.h"
#endif
#ifdef HAS_XFONT
#include <xfont.h>  // for textout functions
#endif
#ifdef HAS_EVENT_SERVER
#include "utils/EventServer.h"
#endif

// Windows includes
#include "GUIWindowManager.h"
#include "GUIWindowHome.h"
#include "GUIStandardWindow.h"
#include "GUIWindowSettings.h"
#include "GUIWindowFileManager.h"
#include "GUIWindowSettingsCategory.h"
#include "GUIWindowMusicPlaylist.h"
#include "GUIWindowMusicSongs.h"
#include "GUIWindowMusicNav.h"
#include "GUIWindowMusicPlaylistEditor.h"
#include "GUIWindowVideoPlaylist.h"
#include "GUIWindowMusicInfo.h"
#include "GUIWindowVideoInfo.h"
#include "GUIWindowVideoFiles.h"
#include "GUIWindowVideoNav.h"
#include "GUIWindowSettingsProfile.h"
#include "GUIWindowSettingsScreenCalibration.h"
#include "GUIWindowPrograms.h"
#include "GUIWindowPictures.h"
#include "GUIWindowScripts.h"
#include "GUIWindowWeather.h"
#include "GUIWindowGameSaves.h"
#include "GUIWindowLoginScreen.h"
#include "GUIWindowVisualisation.h"
#include "GUIWindowSystemInfo.h"
#include "GUIWindowScreensaver.h"
#include "GUIWindowSlideShow.h"
#include "GUIWindowStartup.h"
#include "GUIWindowFullScreen.h"
#include "GUIWindowOSD.h"
#include "GUIWindowMusicOverlay.h"
#include "GUIWindowVideoOverlay.h"

// Dialog includes
#include "GUIDialogMusicOSD.h"
#include "GUIDialogVisualisationSettings.h"
#include "GUIDialogVisualisationPresetList.h"
#include "GUIDialogTrainerSettings.h"
#include "GUIWindowScriptsInfo.h"
#include "GUIDialogNetworkSetup.h"
#include "GUIDialogMediaSource.h"
#include "GUIDialogVideoSettings.h"
#include "GUIDialogAudioSubtitleSettings.h"
#include "GUIDialogVideoBookmarks.h"
#include "GUIDialogProfileSettings.h"
#include "GUIDialogLockSettings.h"
#include "GUIDialogContentSettings.h"
#include "GUIDialogVideoScan.h"
#include "GUIDialogBusy.h"

#include "GUIDialogKeyboard.h"
#include "GUIDialogYesNo.h"
#include "GUIDialogOK.h"
#include "GUIDialogProgress.h"
#include "GUIDialogSelect.h"
#include "GUIDialogFileStacking.h"
#include "GUIDialogNumeric.h"
#include "GUIDialogGamepad.h"
#include "GUIDialogSubMenu.h"
#include "GUIDialogFavourites.h"
#include "GUIDialogButtonMenu.h"
#include "GUIDialogContextMenu.h"
#include "GUIDialogMusicScan.h"
#include "GUIDialogPlayerControls.h"
#include "GUIDialogSongInfo.h"
#include "GUIDialogSmartPlaylistEditor.h"
#include "GUIDialogSmartPlaylistRule.h"
#include "GUIDialogPictureInfo.h"
#include "GUIDialogPluginSettings.h"
#include "GUIDialogFullScreenInfo.h"
#include "GUIDialogSlider.h"
#include "cores/dlgcache.h"

#ifdef _LINUX
#include "XHandle.h"
#endif

using namespace std;
using namespace XFILE;
using namespace DIRECTORY;
using namespace MEDIA_DETECT;
using namespace PLAYLIST;
using namespace VIDEO;
using namespace MUSIC_INFO;
#ifdef HAS_EVENT_SERVER
using namespace EVENTSERVER;
#endif

// uncomment this if you want to use release libs in the debug build.
// Atm this saves you 7 mb of memory
#define USE_RELEASE_LIBS

#ifdef HAS_LCD
#pragma comment (lib,"xbmc/lib/libXenium/XeniumSPIg.lib")
#endif

#if defined(_DEBUG) && !defined(USE_RELEASE_LIBS)
 #ifdef HAS_FILESYSTEM
  #pragma comment (lib,"xbmc/lib/libXBMS/libXBMSd.lib")    // SECTIONNAME=LIBXBMS
  #pragma comment (lib,"xbmc/lib/libsmb/libsmbd.lib")      // SECTIONNAME=LIBSMB
  #pragma comment (lib,"xbmc/lib/libxdaap/libxdaapd.lib") // SECTIONNAME=LIBXDAAP
  #pragma comment (lib,"xbmc/lib/libRTV/libRTVd.lib")    // SECTIONNAME=LIBRTV
 #endif
 #ifdef _XBOX
  #pragma comment (lib,"xbmc/lib/libGoAhead/goaheadd.lib") // SECTIONNAME=LIBHTTP
  #pragma comment (lib,"xbmc/lib/sqLite/libSQLite3d.lib")
  #pragma comment (lib,"xbmc/lib/libshout/libshoutd.lib" )
  #pragma comment (lib,"xbmc/lib/libcdio/libcdiod.lib" )
  #pragma comment (lib,"xbmc/lib/libiconv/libiconvd.lib")
  #pragma comment (lib,"xbmc/lib/libfribidi/libfribidid.lib")
  #pragma comment (lib,"xbmc/lib/libpcre/libpcred.lib")
 #else
  #pragma comment (lib,"../../xbmc/lib/libGoAhead/goahead_win32d.lib") // SECTIONNAME=LIBHTTP
  #pragma comment (lib,"../../xbmc/lib/sqLite/libSQLite3_win32d.lib")
  #pragma comment (lib,"../../xbmc/lib/libshout/libshout_win32d.lib" )
  #pragma comment (lib,"../../xbmc/lib/libcdio/libcdio_win32d.lib" )
  #pragma comment (lib,"../../xbmc/lib/libiconv/libiconvd.lib")
  #pragma comment (lib,"../../xbmc/lib/libfribidi/libfribidid.lib")
  #pragma comment (lib,"../../xbmc/lib/libpcre/libpcred.lib")
 #endif
 #ifdef HAS_MIKMOD
  #pragma comment (lib,"xbmc/lib/mikxbox/mikxboxd.lib")  // SECTIONNAME=MOD_RW,MOD_RX
 #endif
#else
 #ifdef HAS_FILESYSTEM
  #pragma comment (lib,"xbmc/lib/libXBMS/libXBMS.lib")
  #pragma comment (lib,"xbmc/lib/libsmb/libsmb.lib")
  #pragma comment (lib,"xbmc/lib/libxdaap/libxdaap.lib") // SECTIONNAME=LIBXDAAP
  #pragma comment (lib,"xbmc/lib/libRTV/libRTV.lib")
 #endif
 #ifdef _XBOX
  #pragma comment (lib,"xbmc/lib/libGoAhead/goahead.lib")
  #pragma comment (lib,"xbmc/lib/sqLite/libSQLite3.lib")
  #pragma comment (lib,"xbmc/lib/libcdio/libcdio.lib")
  #pragma comment (lib,"xbmc/lib/libshout/libshout.lib")
  #pragma comment (lib,"xbmc/lib/libiconv/libiconv.lib")
  #pragma comment (lib,"xbmc/lib/libfribidi/libfribidi.lib")
  #pragma comment (lib,"xbmc/lib/libpcre/libpcre.lib")
 #else
  #pragma comment (lib,"../../xbmc/lib/libGoAhead/goahead_win32.lib")
  #pragma comment (lib,"../../xbmc/lib/sqLite/libSQLite3_win32.lib")
  #pragma comment (lib,"../../xbmc/lib/libshout/libshout_win32.lib" )
  #pragma comment (lib,"../../xbmc/lib/libcdio/libcdio_win32.lib" )
  #pragma comment (lib,"../../xbmc/lib/libiconv/libiconv.lib")
  #pragma comment (lib,"../../xbmc/lib/libfribidi/libfribidi.lib")
  #pragma comment (lib,"../../xbmc/lib/libpcre/libpcre.lib")
 #endif
 #ifdef HAS_MIKMOD
  #pragma comment (lib,"xbmc/lib/mikxbox/mikxbox.lib")
 #endif
#endif

#define MAX_FFWD_SPEED 5

CStdString g_LoadErrorStr;

#ifdef HAS_XBOX_D3D
static void WaitCallback(DWORD flags)
{
#ifndef PROFILE
  /* if cpu is far ahead of gpu, sleep instead of yield */
  if( flags & D3DWAIT_PRESENT )
    while(D3DDevice::GetPushDistance(D3DDISTANCE_FENCES_TOWAIT) > 0)
      Sleep(1);
  else if( flags & (D3DWAIT_OBJECTLOCK | D3DWAIT_BLOCKONFENCE | D3DWAIT_BLOCKUNTILIDLE) )
    while(D3DDevice::GetPushDistance(D3DDISTANCE_FENCES_TOWAIT) > 1)
      Sleep(1);
#endif
}
#endif

//extern IDirectSoundRenderer* m_pAudioDecoder;
CApplication::CApplication(void) : m_ctrDpad(220, 220), m_itemCurrentFile(new CFileItem), m_progressTrackingItem(new CFileItem)
{
  m_iPlaySpeed = 1;
  m_bSpinDown = false;
  m_bNetworkSpinDown = false;
  m_dwSpinDownTime = timeGetTime();
  m_pWebServer = NULL;
  m_pXbmcHttp = NULL;
  m_prevMedia="";
  m_pFileZilla = NULL;
  m_pPlayer = NULL;
#ifdef HAS_XBOX_HARDWARE
  XSetProcessQuantumLength(5); //default=20msec
  XSetFileCacheSize (256*1024); //default=64kb
#endif
  m_bScreenSave = false;   // CB: SCREENSAVER PATCH
  m_iScreenSaveLock = 0;
  m_dwSkinTime = 0;
  m_bInitializing = true;
  m_eForcedNextPlayer = EPC_NONE;
  m_strPlayListFile = "";
  m_nextPlaylistItem = -1;
  m_bPlaybackStarting = false;

  //true while we in IsPaused mode! Workaround for OnPaused, which must be add. after v2.0
  m_bIsPaused = false;

  /* for now always keep this around */
#ifdef HAS_KARAOKE
  m_pCdgParser = new CCdgParser();
#endif
  m_currentStack = new CFileItemList;
  m_lastActionCode = 0;
}

CApplication::~CApplication(void)
{
  delete m_currentStack;
}

// text out routine for below
#ifdef HAS_XFONT
static void __cdecl FEH_TextOut(XFONT* pFont, int iLine, const wchar_t* fmt, ...)
{
  wchar_t buf[100];
  va_list args;
  va_start(args, fmt);
  _vsnwprintf(buf, 100, fmt, args);
  va_end(args);

  if (!(iLine & 0x8000))
    CLog::Log(LOGFATAL, "%S", buf);

  bool Center = (iLine & 0x10000) > 0;
  pFont->SetTextAlignment(Center ? XFONT_TOP | XFONT_CENTER : XFONT_TOP | XFONT_LEFT);

  iLine &= 0x7fff;

  for (int i = 0; i < 2; i++)
  {
    D3DRECT rc = { 0, 50 + 25 * iLine, 720, 50 + 25 * (iLine + 1) };
    D3DDevice::Clear(1, &rc, D3DCLEAR_TARGET, 0, 0, 0);
    pFont->TextOut(g_application.m_pBackBuffer, buf, -1, Center ? 360 : 80, 50 + 25*iLine);
    D3DDevice::Present(0, 0, 0, 0);
  }
}
#else
static void __cdecl FEH_TextOut(void* pFont, int iLine, const wchar_t* fmt, ...) {}
#endif

HWND g_hWnd = NULL;

void CApplication::InitBasicD3D()
{
  bool bPal = g_videoConfig.HasPAL();
  CLog::Log(LOGINFO, "Init display in default mode: %s", bPal ? "PAL" : "NTSC");
  // init D3D with defaults (NTSC or PAL standard res)
  m_d3dpp.BackBufferWidth = 720;
  m_d3dpp.BackBufferHeight = bPal ? 576 : 480;
  m_d3dpp.BackBufferFormat = D3DFMT_LIN_X8R8G8B8;
  m_d3dpp.BackBufferCount = 1;
  m_d3dpp.EnableAutoDepthStencil = FALSE;
  m_d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
#ifdef HAS_XBOX_D3D
  m_d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
#else
  m_d3dpp.FullScreen_PresentationInterval = 0;
  m_d3dpp.Windowed = TRUE;
  m_d3dpp.hDeviceWindow = g_hWnd;
#endif

  if (!(m_pD3D = Direct3DCreate8(D3D_SDK_VERSION)))
  {
    CLog::Log(LOGFATAL, "FATAL ERROR: Unable to create Direct3D!");
    Sleep(INFINITE); // die
  }

  // Check if we have the required modes available
  g_videoConfig.GetModes(m_pD3D);
  if (!g_graphicsContext.IsValidResolution(g_guiSettings.m_LookAndFeelResolution))
  {
    // Oh uh - doesn't look good for starting in their wanted screenmode
    CLog::Log(LOGERROR, "The screen resolution requested is not valid, resetting to a valid mode");
    g_guiSettings.m_LookAndFeelResolution = g_videoConfig.GetSafeMode();
    CLog::Log(LOGERROR, "Resetting to mode %s", g_settings.m_ResInfo[g_guiSettings.m_LookAndFeelResolution].strMode);
    CLog::Log(LOGERROR, "Done reset");
  }

  // Transfer the resolution information to our graphics context
  g_graphicsContext.SetD3DParameters(&m_d3dpp);
  g_graphicsContext.SetVideoResolution(g_guiSettings.m_LookAndFeelResolution, TRUE);

  // Create the device
#ifdef HAS_XBOX_D3D
  // Xbox MUST use HAL / Hardware Vertex Processing!
  if (m_pD3D->CreateDevice(0, D3DDEVTYPE_HAL, NULL, D3DCREATE_HARDWARE_VERTEXPROCESSING, &m_d3dpp, &m_pd3dDevice) != S_OK)
  {
    CLog::Log(LOGFATAL, "FATAL ERROR: Unable to create D3D Device!");
    Sleep(INFINITE); // die
  }
  m_pd3dDevice->GetBackBuffer(0, 0, &m_pBackBuffer);
#else
  if (m_pD3D->CreateDevice(0, D3DDEVTYPE_REF, NULL, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &m_d3dpp, &m_pd3dDevice) != S_OK)
  {
    CLog::Log(LOGFATAL, "FATAL ERROR: Unable to create D3D Device!");
    Sleep(INFINITE); // die
  }
#endif

  if (m_splash)
  {
    m_splash->Stop();
  }

  m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0, 0, 0);
  m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}

// This function does not return!
void CApplication::FatalErrorHandler(bool InitD3D, bool MapDrives, bool InitNetwork)
{
  // XBMC couldn't start for some reason...
  // g_LoadErrorStr should contain the reason
  CLog::Log(LOGWARNING, "Emergency recovery console starting...");

  bool HaveGamepad = true; // should always have the gamepad when we get here
  if (InitD3D)
    InitBasicD3D();

  if (m_splash)
  {
    m_splash->Stop();
  }

  m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0, 0, 0);
  m_pd3dDevice->Present( NULL, NULL, NULL, NULL );

  // D3D is up, load default font
#ifdef HAS_XFONT
  XFONT* pFont;
  if (XFONT_OpenDefaultFont(&pFont) != S_OK)
  {
    CLog::Log(LOGFATAL, "FATAL ERROR: Unable to open default font!");
    Sleep(INFINITE); // die
  }

  // defaults for text
  pFont->SetBkMode(XFONT_OPAQUE);
  pFont->SetBkColor(D3DCOLOR_XRGB(0, 0, 0));
  pFont->SetTextColor(D3DCOLOR_XRGB(0xff, 0x20, 0x20));
#else
  void *pFont = NULL;
#endif
  int iLine = 0;
  FEH_TextOut(pFont, iLine++, L"XBMC Fatal Error:");
  char buf[500];
  strncpy(buf, g_LoadErrorStr.c_str(), 500);
  buf[499] = 0;
  char* pNewline = strtok(buf, "\n");
  while (pNewline)
  {
    FEH_TextOut(pFont, iLine++, L"%S", pNewline);
    pNewline = strtok(NULL, "\n");
  }
  ++iLine;

#ifdef HAS_XBOX_HARDWARE
  if (MapDrives)
  {
    // map in default drives
    CIoSupport::RemapDriveLetter('C',"Harddisk0\\Partition2");
    CIoSupport::RemapDriveLetter('D',"Cdrom0");
    CIoSupport::RemapDriveLetter('E',"Harddisk0\\Partition1");

    //Add. also Drive F/G
    if (CIoSupport::PartitionExists(6)) 
      CIoSupport::RemapDriveLetter('F',"Harddisk0\\Partition6");
    if (CIoSupport::PartitionExists(7))
      CIoSupport::RemapDriveLetter('G',"Harddisk0\\Partition7");
  }
#endif
  bool Pal = g_graphicsContext.GetVideoResolution() == PAL_4x3;

  if (HaveGamepad)
    FEH_TextOut(pFont, (Pal ? 16 : 12) | 0x18000, L"Press any button to reboot");

  bool NetworkUp = false;

  // Boot up the network for FTP
  if (InitNetwork)
  {
    std::vector<int> netorder;
    if (m_bXboxMediacenterLoaded)
    {
      if (g_guiSettings.GetInt("network.assignment") == NETWORK_DHCP)
      {
        netorder.push_back(NETWORK_DHCP);
        netorder.push_back(NETWORK_STATIC);
      }
      else if (g_guiSettings.GetInt("network.assignment") == NETWORK_STATIC)
      {
        netorder.push_back(NETWORK_STATIC);
        netorder.push_back(NETWORK_DHCP);
      }
      else
      {
        netorder.push_back(NETWORK_DASH);
        netorder.push_back(NETWORK_DHCP);
        netorder.push_back(NETWORK_STATIC);
      }
    }
    else
    {
      netorder.push_back(NETWORK_DASH);
      netorder.push_back(NETWORK_DHCP);
      netorder.push_back(NETWORK_STATIC);
    }

    while(1)
    {
      std::vector<int>::iterator it;
      for( it = netorder.begin();it != netorder.end(); it++)
      {
        g_network.Deinitialize();

#ifdef HAS_XBOX_NETWORK
        if (!g_network.IsEthernetConnected())
        {
          FEH_TextOut(pFont, iLine, L"Network cable unplugged");
          break;
        }
#endif
        switch( (*it) )
        {
          case NETWORK_DASH:
            FEH_TextOut(pFont, iLine, L"Init network using dash settings...");
            g_network.Initialize(NETWORK_DASH, "","","","");
            break;
          case NETWORK_DHCP:
            FEH_TextOut(pFont, iLine, L"Init network using DHCP...");
            g_network.Initialize(NETWORK_DHCP, "","","","");
            break;
          default:
            FEH_TextOut(pFont, iLine, L"Init network using static ip...");
            if( m_bXboxMediacenterLoaded )
            {
              g_network.Initialize(NETWORK_STATIC,
                    g_guiSettings.GetString("network.ipaddress").c_str(),
                    g_guiSettings.GetString("network.subnet").c_str(),
                    g_guiSettings.GetString("network.gateway").c_str(),
                    g_guiSettings.GetString("network.dns").c_str() );
            }
            else
            {
              g_network.Initialize(NETWORK_STATIC,
                    "192.168.0.42",
                    "255.255.255.0",
                    "192.168.0.1",
                    "192.168.0.1" );
            }
            break;
        }

        int count = 0;
#ifdef HAS_XBOX_NETWORK
        DWORD dwState = XNET_GET_XNADDR_PENDING;

        while(dwState == XNET_GET_XNADDR_PENDING)
        {
          dwState = g_network.UpdateState();

          if( dwState != XNET_GET_XNADDR_PENDING )
            break;

          if (HaveGamepad && AnyButtonDown())
            g_applicationMessenger.Restart();


          Sleep(50);
          ++count;
        }

        if( dwState != XNET_GET_XNADDR_PENDING && dwState != XNET_GET_XNADDR_NONE )
        {
          /* yay, we got network */
          NetworkUp = true;
          break;
        }
#endif
        /* increment line before next attempt */
        ++iLine;
      }

      /* break out of the continous loop if we have network*/
      if( NetworkUp )
        break;
      else
      {
        int n = 10;
        while (n)
        {
          FEH_TextOut(pFont, (iLine + 1) | 0x8000, L"Unable to init network, retrying in %d seconds", n--);
          for (int i = 0; i < 20; ++i)
          {
            Sleep(50);

            if (HaveGamepad && AnyButtonDown())
              g_applicationMessenger.Restart();
          }
        }
      }
    }
  }

  if( NetworkUp )
  {
    FEH_TextOut(pFont, iLine++, L"IP Address: %S", g_network.m_networkinfo.ip);
    ++iLine;
  }

  if (NetworkUp)
  {
#ifdef HAS_FTP_SERVER
    if (!m_pFileZilla)
    {
      // Start FTP with default settings
      FEH_TextOut(pFont, iLine++, L"Starting FTP server...");
      StartFtpEmergencyRecoveryMode();
    }

    FEH_TextOut(pFont, iLine++, L"FTP server running on port %d, login: xbox/xbox", m_pFileZilla->mSettings.GetServerPort());
#endif
    ++iLine;
  }

  if (HaveGamepad)
  {
    for (;;)
    {
      Sleep(50);
      if (AnyButtonDown())
      {
        g_application.Stop();
        Sleep(200);
#ifdef _XBOX
#ifndef _DEBUG  // don't actually shut off if debug build, it hangs VS for a long time
        XKUtils::XBOXPowerCycle();
#endif
#else
        SendMessage(g_hWnd, WM_CLOSE, 0, 0);
#endif
      }
    }
  }
  else
  {
#ifdef _XBOX
    Sleep(INFINITE);
#else
    SendMessage(g_hWnd, WM_CLOSE, 0, 0);
#endif
  }
}

LONG WINAPI CApplication::UnhandledExceptionFilter(struct _EXCEPTION_POINTERS *ExceptionInfo)
{
  PCSTR pExceptionString = "Unknown exception code";

#define STRINGIFY_EXCEPTION(code) case code: pExceptionString = #code; break

  switch (ExceptionInfo->ExceptionRecord->ExceptionCode)
  {
    STRINGIFY_EXCEPTION(EXCEPTION_ACCESS_VIOLATION);
    STRINGIFY_EXCEPTION(EXCEPTION_ARRAY_BOUNDS_EXCEEDED);
    STRINGIFY_EXCEPTION(EXCEPTION_BREAKPOINT);
    STRINGIFY_EXCEPTION(EXCEPTION_FLT_DENORMAL_OPERAND);
    STRINGIFY_EXCEPTION(EXCEPTION_FLT_DIVIDE_BY_ZERO);
    STRINGIFY_EXCEPTION(EXCEPTION_FLT_INEXACT_RESULT);
    STRINGIFY_EXCEPTION(EXCEPTION_FLT_INVALID_OPERATION);
    STRINGIFY_EXCEPTION(EXCEPTION_FLT_OVERFLOW);
    STRINGIFY_EXCEPTION(EXCEPTION_FLT_STACK_CHECK);
    STRINGIFY_EXCEPTION(EXCEPTION_ILLEGAL_INSTRUCTION);
    STRINGIFY_EXCEPTION(EXCEPTION_INT_DIVIDE_BY_ZERO);
    STRINGIFY_EXCEPTION(EXCEPTION_INT_OVERFLOW);
    STRINGIFY_EXCEPTION(EXCEPTION_INVALID_DISPOSITION);
    STRINGIFY_EXCEPTION(EXCEPTION_NONCONTINUABLE_EXCEPTION);
    STRINGIFY_EXCEPTION(EXCEPTION_SINGLE_STEP);
  }
#undef STRINGIFY_EXCEPTION

  g_LoadErrorStr.Format("%s (0x%08x)\n at 0x%08x",
                        pExceptionString, ExceptionInfo->ExceptionRecord->ExceptionCode,
                        ExceptionInfo->ExceptionRecord->ExceptionAddress);

  CLog::Log(LOGFATAL, "%s", g_LoadErrorStr.c_str());

  return ExceptionInfo->ExceptionRecord->ExceptionCode;
}

#ifdef _XBOX
#include "xbox/Undocumented.h"
extern "C" HANDLE __stdcall KeGetCurrentThread(VOID);
#endif
extern "C" void __stdcall init_emu_environ();
extern "C" void __stdcall update_emu_environ();

HRESULT CApplication::Create(HWND hWnd)
{
  g_guiSettings.Initialize();  // Initialize default Settings
  g_settings.Initialize(); //Initialize default AdvancedSettings

  g_hWnd = hWnd;

  HRESULT hr;
  //grab a handle to our thread to be used later in identifying the render thread
  m_threadID = GetCurrentThreadId();

  //floating point precision to 24 bits (faster performance)
  _controlfp(_PC_24, _MCW_PC);

  init_emu_environ();

  /* install win32 exception translator, win32 exceptions
   * can now be caught using c++ try catch */
  win32_exception::install_handler();

  CStdString strExecutablePath;
  char szDevicePath[MAX_PATH];

  // map Q to home drive of xbe to load the config file
  CUtil::GetHomePath(strExecutablePath);
  CIoSupport::GetPartition(strExecutablePath.c_str()[0], szDevicePath);
  strcat(szDevicePath, &strExecutablePath.c_str()[2]);
  CIoSupport::RemapDriveLetter('Q', szDevicePath);
 
  // Do all the special:// & driveletter mapping & setup profiles
  InitDirectoriesXbox();
  
  CLog::Log(LOGNOTICE, "-----------------------------------------------------------------------");
#if defined(_LINUX) && !defined(__APPLE__)
  CLog::Log(LOGNOTICE, "Starting XBMC, Platform: GNU/Linux.  Built on %s (SVN:%s)", __DATE__, SVN_REV);
#elif defined(__APPLE__)
  CLog::Log(LOGNOTICE, "Starting XBMC, Platform: Mac OS X.  Built on %s", __DATE__);
#elif defined(_WIN32) && !defined(HAS_XBOX_HARDWARE)
  CLog::Log(LOGNOTICE, "Starting XBMC, Platform: %s.  Built on %s (SVN:%s, compiler %i)",g_sysinfo.GetKernelVersion().c_str(), __DATE__, SVN_REV, _MSC_VER);
#else
  CLog::Log(LOGNOTICE, "Starting XBMC, Platform: Xbox.  Built on %s (SVN:%s, compiler %i)", __DATE__, SVN_REV, _MSC_VER);
#endif
  CSpecialProtocol::LogPaths();

  char szXBEFileName[1024];
  CIoSupport::GetXbePath(szXBEFileName);
  CLog::Log(LOGNOTICE, "The executable running is: %s", szXBEFileName);
  CLog::Log(LOGNOTICE, "Log File is located: %sxbmc.log", g_stSettings.m_logFolder.c_str());
  CLog::Log(LOGNOTICE, "-----------------------------------------------------------------------");

  // if we are running from DVD our UserData location will be TDATA
  if (CUtil::IsDVD(strExecutablePath))
  {
    // TODO: Should we copy over any UserData folder from the DVD?
    if (!CFile::Exists("special://masterprofile/guisettings.xml")) // first run - cache userdata folder
    {
      CFileItemList items;
      CUtil::GetRecursiveListing("special://xbmc/userdata",items,"");
      for (int i=0;i<items.Size();++i)
          CFile::Cache(items[i]->m_strPath,"special://masterprofile/"+CUtil::GetFileName(items[i]->m_strPath));
    }
    g_settings.m_vecProfiles[0].setDirectory("special://masterprofile/");
    g_stSettings.m_logFolder = "special://masterprofile/";
  }
  else
  {
    CStdString strMnt = _P(g_settings.GetUserDataFolder());
    if (strMnt.Left(2).Equals("Q:"))
    {
      CUtil::GetHomePath(strMnt);
      strMnt += _P(g_settings.GetUserDataFolder()).substr(2);
    }

    CIoSupport::GetPartition(strMnt.c_str()[0], szDevicePath);
    strcat(szDevicePath, &strMnt.c_str()[2]);
    CIoSupport::RemapDriveLetter('T', szDevicePath);
  }

  CLog::Log(LOGNOTICE, "Setup DirectX");
  // Create the Direct3D object
  if ( NULL == ( m_pD3D = Direct3DCreate8(D3D_SDK_VERSION) ) )
  {
    CLog::Log(LOGFATAL, "XBAppEx: Unable to create Direct3D!" );
    return E_FAIL;
  }

  //list available videomodes
  g_videoConfig.GetModes(m_pD3D);
  //init the present parameters with values that are supported
  RESOLUTION initialResolution = g_videoConfig.GetInitialMode(m_pD3D, &m_d3dpp);
  // Transfer the resolution information to our graphics context
  g_graphicsContext.SetD3DParameters(&m_d3dpp);
  g_graphicsContext.SetVideoResolution(initialResolution, TRUE);

  // Initialize core peripheral port support. Note: If these parameters
  // are 0 and NULL, respectively, then the default number and types of
  // controllers will be initialized.
#ifdef HAS_XBOX_HARDWARE
  XInitDevices( m_dwNumInputDeviceTypes, m_InputDeviceTypes );

  // Create the gamepad devices
  if ( FAILED(hr = XBInput_CreateGamepads(&m_Gamepad)) )
  {
    CLog::Log(LOGERROR, "XBAppEx: Call to CreateGamepads() failed!" );
    return hr;
  }

  if ( FAILED(hr = XBInput_CreateIR_Remotes()) )
  {
    CLog::Log(LOGERROR, "XBAppEx: Call to CreateIRRemotes() failed!" );
    return hr;
  }
#endif

  // Create the Mouse and Keyboard devices
  g_Mouse.Initialize(&hWnd);
  g_Keyboard.Initialize(hWnd);

#ifdef HAS_XBOX_HARDWARE
  // Wait for controller polling to finish. in an elegant way, instead of a Sleep(1000)
  while (XGetDeviceEnumerationStatus() == XDEVICE_ENUMERATION_BUSY)
  {
    ReadInput();
  }
  Sleep(10); // needed or the readinput doesnt fetch anything
  ReadInput();
#endif
#ifdef HAS_GAMEPAD
  //Check for LTHUMBCLICK+RTHUMBCLICK and BLACK+WHITE, no LTRIGGER+RTRIGGER
  if (((m_DefaultGamepad.wButtons & (XINPUT_GAMEPAD_LEFT_THUMB + XINPUT_GAMEPAD_RIGHT_THUMB)) && !(m_DefaultGamepad.wButtons & (KEY_BUTTON_LEFT_TRIGGER+KEY_BUTTON_RIGHT_TRIGGER))) ||
      ((m_DefaultGamepad.bAnalogButtons[XINPUT_GAMEPAD_BLACK] && m_DefaultGamepad.bAnalogButtons[XINPUT_GAMEPAD_WHITE]) && !(m_DefaultGamepad.wButtons & KEY_BUTTON_LEFT_TRIGGER+KEY_BUTTON_RIGHT_TRIGGER)))
  {
    CLog::Log(LOGINFO, "Key combination detected for userdata deletion (LTHUMB+RTHUMB or BLACK+WHITE)");
    InitBasicD3D();
    // D3D is up, load default font
    XFONT* pFont;
    if (XFONT_OpenDefaultFont(&pFont) != S_OK)
    {
      CLog::Log(LOGFATAL, "FATAL ERROR: Unable to open default font!");
      Sleep(INFINITE); // die
    }
    // defaults for text
    pFont->SetBkMode(XFONT_OPAQUE);
    pFont->SetBkColor(D3DCOLOR_XRGB(0, 0, 0));
    pFont->SetTextColor(D3DCOLOR_XRGB(0xff, 0x20, 0x20));
    int iLine = 0;
    FEH_TextOut(pFont, iLine++, L"Key combination for userdata deletion detected!");
    FEH_TextOut(pFont, iLine++, L"Are you sure you want to proceed?");
    iLine++;
    FEH_TextOut(pFont, iLine++, L"A for yes, any other key for no");
    bool bAnyAnalogKey = false;
    while (m_DefaultGamepad.wPressedButtons != XBGAMEPAD_NONE) // wait for user to let go of lclick + rclick
    {
      ReadInput();
    }
    while (m_DefaultGamepad.wPressedButtons == XBGAMEPAD_NONE && !bAnyAnalogKey)
    {
      ReadInput();
      bAnyAnalogKey = m_DefaultGamepad.bPressedAnalogButtons[0] || m_DefaultGamepad.bPressedAnalogButtons[1] || m_DefaultGamepad.bPressedAnalogButtons[2] || m_DefaultGamepad.bPressedAnalogButtons[3] || m_DefaultGamepad.bPressedAnalogButtons[4] || m_DefaultGamepad.bPressedAnalogButtons[5] || m_DefaultGamepad.bPressedAnalogButtons[6] || m_DefaultGamepad.bPressedAnalogButtons[7];
    }
    if (m_DefaultGamepad.bPressedAnalogButtons[XINPUT_GAMEPAD_A])
    {
      CUtil::DeleteGUISettings();
      CUtil::WipeDir(CUtil::AddFileToFolder(g_settings.GetUserDataFolder(),"database\\"));
      CUtil::WipeDir(CUtil::AddFileToFolder(g_settings.GetUserDataFolder(),"thumbnails\\"));
      CUtil::WipeDir(CUtil::AddFileToFolder(g_settings.GetUserDataFolder(),"playlists\\"));
      CUtil::WipeDir(CUtil::AddFileToFolder(g_settings.GetUserDataFolder(),"cache\\"));
      CUtil::WipeDir(CUtil::AddFileToFolder(g_settings.GetUserDataFolder(),"profiles\\"));
      CUtil::WipeDir(CUtil::AddFileToFolder(g_settings.GetUserDataFolder(),"visualisations\\"));
      CFile::Delete(CUtil::AddFileToFolder(g_settings.GetUserDataFolder(),"avpacksettings.xml"));
      g_settings.m_vecProfiles.erase(g_settings.m_vecProfiles.begin()+1,g_settings.m_vecProfiles.end());

      g_settings.SaveProfiles("q:\\system\\profiles.xml");

      char szXBEFileName[1024];

      CIoSupport::GetXbePath(szXBEFileName);
      CUtil::RunXBE(szXBEFileName);
    }
    m_pd3dDevice->Release();
  }
#endif

  CIoSupport::RemapDriveLetter('C', "Harddisk0\\Partition2");
  CIoSupport::RemapDriveLetter('E', "Harddisk0\\Partition1");

  CIoSupport::Dismount("Cdrom0");
  CIoSupport::RemapDriveLetter('D', "Cdrom0");

  // Attempt to read the LBA48 v3 patch partition table, if kernel supports the command and it exists.
  CIoSupport::ReadPartitionTable();
  if (CIoSupport::HasPartitionTable())
  {
    // Mount up to Partition15 (drive O:) if they are available.
    for (int i=EXTEND_PARTITION_BEGIN; i <= EXTEND_PARTITION_END; i++)
    {
      char szDevice[32];
      if (CIoSupport::PartitionExists(i))
      {
        char cDriveLetter = 'A' + i - 1;
        sprintf(szDevice, "Harddisk0\\Partition%u", i);

        CIoSupport::RemapDriveLetter(cDriveLetter, szDevice);
      }
    }
  }
  else
  {
    if (CIoSupport::DriveExists('F'))
      CIoSupport::RemapDriveLetter('F', "Harddisk0\\Partition6");
    if (CIoSupport::DriveExists('G'))
      CIoSupport::RemapDriveLetter('G', "Harddisk0\\Partition7");
  }

  CIoSupport::RemapDriveLetter('X',"Harddisk0\\Partition3");
  CIoSupport::RemapDriveLetter('Y',"Harddisk0\\Partition4");
#ifdef HAS_XBOX_HARDWARE
  CIoSupport::RemapDriveLetter('Z',"Harddisk0\\Partition5");
#endif

  CLog::Log(LOGINFO, "Drives are mapped");

  CLog::Log(LOGNOTICE, "load settings...");
  g_LoadErrorStr = "Unable to load settings";
  g_settings.m_iLastUsedProfileIndex = g_settings.m_iLastLoadedProfileIndex;
  if (g_settings.bUseLoginScreen && g_settings.m_iLastLoadedProfileIndex != 0)
    g_settings.m_iLastLoadedProfileIndex = 0;

  m_bAllSettingsLoaded = g_settings.Load(m_bXboxMediacenterLoaded, m_bSettingsLoaded);
  if (!m_bAllSettingsLoaded)
    FatalErrorHandler(true, true, true);

  update_emu_environ();//apply the GUI settings

  // Check for WHITE + Y for forced Error Handler (to recover if something screwy happens)
#ifdef HAS_GAMEPAD
  if (m_DefaultGamepad.bAnalogButtons[XINPUT_GAMEPAD_Y] && m_DefaultGamepad.bAnalogButtons[XINPUT_GAMEPAD_WHITE])
  {
    g_LoadErrorStr = "Key code detected for Error Recovery mode";
    FatalErrorHandler(true, true, true);
  }
#endif

  //Check for X+Y - if pressed, set debug log mode and mplayer debuging on
  CheckForDebugButtonCombo();

#ifdef HAS_XBOX_HARDWARE
  bool bNeedReboot = false;
  char temp[1024];
  CIoSupport::GetXbePath(temp);
  char temp2[1024];
  char temp3;
  temp3 = temp[0];
  CIoSupport::GetPartition(temp3,temp2);
  CStdString strTemp(temp+2);
  int iLastSlash = strTemp.rfind('\\');
  strcat(temp2,strTemp.substr(0,iLastSlash).c_str());
  F_VIDEO ForceVideo = VIDEO_NULL;
  F_COUNTRY ForceCountry = COUNTRY_NULL;

  if (CUtil::RemoveTrainer())
    bNeedReboot = true;

// now check if we are switching video modes. if, are we in the wrong mode according to eeprom?
  if (g_guiSettings.GetBool("myprograms.gameautoregion"))
  {
    bool fDoPatchTest = false;

    // should use xkeeprom.h :/
    EEPROMDATA EEPROM;
    ZeroMemory(&EEPROM, sizeof(EEPROMDATA));

    if( XKUtils::ReadEEPROMFromXBOX((LPBYTE)&EEPROM))
    {
      DWORD DWVideo = *(LPDWORD)(&EEPROM.VideoStandard[0]);
      char temp[1024];
      CIoSupport::GetXbePath(temp);
      char temp2[1024];
      char temp3;
      temp3 = temp[0];
      CIoSupport::GetPartition(temp3,temp2);
      CStdString strTemp(temp+2);
      int iLastSlash = strTemp.rfind('\\');
      strcat(temp2,strTemp.substr(0,iLastSlash).c_str());

      if ((DWVideo == XKEEPROM::VIDEO_STANDARD::NTSC_M) && ((XGetVideoStandard() == XC_VIDEO_STANDARD_PAL_I) || (XGetVideoStandard() == XC_VIDEO_STANDARD_NTSC_J) || initialResolution > 5))
      {
        CLog::Log(LOGINFO, "Rebooting to change resolution from %s back to NTSC_M", (XGetVideoStandard() == XC_VIDEO_STANDARD_PAL_I) ? "PAL" : "NTSC_J");
        ForceVideo = VIDEO_NTSCM;
        ForceCountry = COUNTRY_USA;
        bNeedReboot = true;
        fDoPatchTest = true;
      }
      else if ((DWVideo == XKEEPROM::VIDEO_STANDARD::PAL_I) && ((XGetVideoStandard() == XC_VIDEO_STANDARD_NTSC_M) || (XGetVideoStandard() == XC_VIDEO_STANDARD_NTSC_J) || initialResolution < 6))
      {
        CLog::Log(LOGINFO, "Rebooting to change resolution from %s back to PAL_I", (XGetVideoStandard() == XC_VIDEO_STANDARD_NTSC_M) ? "NTSC_M" : "NTSC_J");
        ForceVideo = VIDEO_PAL50;
        ForceCountry = COUNTRY_EUR;
        bNeedReboot = true;
        fDoPatchTest = true;
      }
      else if ((DWVideo == XKEEPROM::VIDEO_STANDARD::NTSC_J) && ((XGetVideoStandard() == XC_VIDEO_STANDARD_NTSC_M) || (XGetVideoStandard() == XC_VIDEO_STANDARD_PAL_I) || initialResolution > 5))
      {
        CLog::Log(LOGINFO, "Rebooting to change resolution from %s back to NTSC_J", (XGetVideoStandard() == XC_VIDEO_STANDARD_PAL_I) ? "PAL" : "NTSC_M");
        ForceVideo = VIDEO_NTSCJ;
        ForceCountry = COUNTRY_JAP;
        bNeedReboot = true;
        fDoPatchTest = true;
      }
      else
        CUtil::RemoveKernelPatch(); // This removes the Resolution patch from the kernel if it is not needed (if actual resolution matches eeprom setting)

      if (fDoPatchTest) // Is set if we have to test whether our patch is in the kernel & therefore responsible for the mismatch of resolution & eeprom setting
      {
        if (!CUtil::LookForKernelPatch()) // If our patch is not present we are not responsible for the mismatch of current resolution & eeprom setting
        {
          // We do a hard reset to come back to default resolution and avoid infinite reboots
          CLog::Log(LOGINFO, "No infinite reboot loop...");
          g_applicationMessenger.Reset();
        }
      }
    }
  }

  if (bNeedReboot)
  {
    Destroy();
    CUtil::LaunchXbe(temp2,("D:\\"+strTemp.substr(iLastSlash+1)).c_str(),NULL,ForceVideo,ForceCountry);
  }
#endif

  CStdString strHomePath = "Q:";
  CLog::Log(LOGINFO, "Checking skinpath existence, and existence of keymap.xml:%s...", (strHomePath + "\\skin").c_str());

  CStdString systemKeymapPath = "Q:\\system\\Keymap.xml";
  CStdString userKeymapPath = g_settings.GetUserDataItem("Keymap.xml");
#ifdef _XBOX
  if (access(strHomePath + "\\skin", 0) || (access(systemKeymapPath.c_str(), 0) && access(userKeymapPath.c_str(), 0)))
  {
    g_LoadErrorStr = "Unable to find skin or Keymap.xml.  Make sure you have System/Keymap.xml and Skin/ folder";
    FatalErrorHandler(true, false, true);
  }
#endif

  if (!g_graphicsContext.IsValidResolution(g_guiSettings.m_LookAndFeelResolution))
  {
    // Oh uh - doesn't look good for starting in their wanted screenmode
    CLog::Log(LOGERROR, "The screen resolution requested is not valid, resetting to a valid mode");
    g_guiSettings.m_LookAndFeelResolution = initialResolution;
  }
  // Transfer the new resolution information to our graphics context
#ifndef HAS_XBOX_D3D
  m_d3dpp.Windowed = TRUE;
  m_d3dpp.hDeviceWindow = g_hWnd;
#else
#define D3DCREATE_MULTITHREADED 0
#endif

  g_graphicsContext.SetD3DParameters(&m_d3dpp);
  g_graphicsContext.SetVideoResolution(g_guiSettings.m_LookAndFeelResolution, TRUE);
  
  if ( FAILED( hr = m_pD3D->CreateDevice(0, D3DDEVTYPE_HAL, NULL,
                                         D3DCREATE_MULTITHREADED | D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                         &m_d3dpp, &m_pd3dDevice ) ) )
  {
    // try software vertex processing
    if ( FAILED( hr = m_pD3D->CreateDevice(0, D3DDEVTYPE_HAL, NULL,
                                          D3DCREATE_MULTITHREADED | D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                          &m_d3dpp, &m_pd3dDevice ) ) )
    {
      // and slow as arse reference processing
      if ( FAILED( hr = m_pD3D->CreateDevice(0, D3DDEVTYPE_REF, NULL,
                                            D3DCREATE_MULTITHREADED | D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                            &m_d3dpp, &m_pd3dDevice ) ) )
      {

        CLog::Log(LOGFATAL, "XBAppEx: Could not create D3D device!" );
        CLog::Log(LOGFATAL, " width/height:(%ix%i)" , m_d3dpp.BackBufferWidth, m_d3dpp.BackBufferHeight);
        CLog::Log(LOGFATAL, " refreshrate:%i" , m_d3dpp.FullScreen_RefreshRateInHz);
        if (m_d3dpp.Flags & D3DPRESENTFLAG_WIDESCREEN)
          CLog::Log(LOGFATAL, " 16:9 widescreen");
        else
          CLog::Log(LOGFATAL, " 4:3");

        if (m_d3dpp.Flags & D3DPRESENTFLAG_INTERLACED)
          CLog::Log(LOGFATAL, " interlaced");
        if (m_d3dpp.Flags & D3DPRESENTFLAG_PROGRESSIVE)
          CLog::Log(LOGFATAL, " progressive");
        return hr;
      }
    }
  }
  g_graphicsContext.SetD3DDevice(m_pd3dDevice);
  g_graphicsContext.CaptureStateBlock();
  // set filters
  g_graphicsContext.Get3DDevice()->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR /*g_stSettings.m_minFilter*/ );
  g_graphicsContext.Get3DDevice()->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR /*g_stSettings.m_maxFilter*/ );
  CUtil::InitGamma();
  
  // set GUI res and force the clear of the screen
  g_graphicsContext.SetVideoResolution(g_guiSettings.m_LookAndFeelResolution, TRUE, true);

  // initialize our charset converter
  g_charsetConverter.reset();

  // Load the langinfo to have user charset <-> utf-8 conversion
  CStdString strLanguage = g_guiSettings.GetString("locale.language");
  strLanguage[0] = toupper(strLanguage[0]);

  // Load the langinfo to have user charset <-> utf-8 conversion
  CStdString strLangInfoPath;
  strLangInfoPath.Format("special://xbmc/language/%s/langinfo.xml", strLanguage.c_str());

  CLog::Log(LOGINFO, "load language info file:%s", strLangInfoPath.c_str());
  g_langInfo.Load(strLangInfoPath);

  CStdString strKeyboardLayoutConfigurationPath;
  strKeyboardLayoutConfigurationPath.Format("Q:\\language\\%s\\keyboardmap.xml", g_guiSettings.GetString("locale.language"));
  CLog::Log(LOGINFO, "load keyboard layout configuration info file: %s", strKeyboardLayoutConfigurationPath.c_str());
  g_keyboardLayoutConfiguration.Load(strKeyboardLayoutConfigurationPath);

  m_splash = new CSplash("Q:\\media\\splash.png");
  m_splash->Start();

  CStdString strLanguagePath;
  strLanguagePath.Format("special://xbmc/language/%s/strings.xml", strLanguage.c_str());

  CLog::Log(LOGINFO, "load language file:%s", strLanguagePath.c_str());
  if (!g_localizeStrings.Load(strLanguagePath))
    FatalErrorHandler(false, false, true);

  CLog::Log(LOGINFO, "load keymapping");
  if (!CButtonTranslator::GetInstance().Load())
    FatalErrorHandler(false, false, true);

  // check the skin file for testing purposes
  CStdString strSkinBase = "special://xbmc/skin/";
  CStdString strSkinPath = strSkinBase + g_guiSettings.GetString("lookandfeel.skin");
  CLog::Log(LOGINFO, "Checking skin version of: %s", g_guiSettings.GetString("lookandfeel.skin").c_str());
  if (!g_SkinInfo.Check(strSkinPath))
  {
    // reset to the default skin (DEFAULT_SKIN)
    CLog::Log(LOGINFO, "The above skin isn't suitable - checking the version of the default: %s", DEFAULT_SKIN);
    strSkinPath = strSkinBase + DEFAULT_SKIN;
    if (!g_SkinInfo.Check(strSkinPath))
    {
      g_LoadErrorStr.Format("No suitable skin version found.\nWe require at least version %5.4f \n", g_SkinInfo.GetMinVersion());
      FatalErrorHandler(false, false, true);
    }
  }
  int iResolution = g_graphicsContext.GetVideoResolution();
  CLog::Log(LOGINFO, "GUI format %ix%i %s",
            g_settings.m_ResInfo[iResolution].iWidth,
            g_settings.m_ResInfo[iResolution].iHeight,
            g_settings.m_ResInfo[iResolution].strMode);
  m_gWindowManager.Initialize();

  g_actionManager.SetScriptActionCallback(&g_pythonParser);

  // show recovery console on fatal error instead of freezing
  CLog::Log(LOGINFO, "install unhandled exception filter");
  SetUnhandledExceptionFilter(UnhandledExceptionFilter);

#ifdef HAS_XBOX_D3D
  D3DDevice::SetWaitCallback(WaitCallback);
#endif

  g_Mouse.SetEnabled(g_guiSettings.GetBool("lookandfeel.enablemouse"));

  CUtil::InitRandomSeed();

  return CXBApplicationEx::Create(hWnd);
}


HRESULT CApplication::Initialize()
{
  CLog::Log(LOGINFO, "creating subdirectories");

  CLog::Log(LOGINFO, "userdata folder: %s", g_settings.GetProfileUserDataFolder().c_str());
  CLog::Log(LOGINFO, "recording folder:%s", g_guiSettings.GetString("mymusic.recordingpath",false).c_str());
  CLog::Log(LOGINFO, "screenshots folder:%s", g_guiSettings.GetString("pictures.screenshotpath",false).c_str());

  // UserData folder layout:
  // UserData/
  //   Database/
  //     CDDb/
  //   Thumbnails/
  //     Music/
  //       temp/
  //     0 .. F/

  CDirectory::Create(g_settings.GetUserDataFolder());
  CDirectory::Create(g_settings.GetProfileUserDataFolder());
  g_settings.CreateProfileFolders();

  CDirectory::Create(g_settings.GetProfilesThumbFolder());

  CreateDirectory("Z:\\temp", NULL); // temp directory for python and dllGetTempPathA
  CreateDirectory("Q:\\scripts", NULL);
  CreateDirectory("Q:\\plugins", NULL);
  CreateDirectory("Q:\\plugins\\music", NULL);
  CreateDirectory("Q:\\plugins\\video", NULL);
  CreateDirectory("Q:\\plugins\\pictures", NULL);
  CreateDirectory("Q:\\plugins\\programs", NULL);
  CreateDirectory("Q:\\language", NULL);
  CreateDirectory("Q:\\visualisations", NULL);
  CreateDirectory("Q:\\sounds", NULL);
  CreateDirectory(g_settings.GetUserDataFolder()+"\\visualisations",NULL);

  // initialize network
  if (!m_bXboxMediacenterLoaded)
  {
    CLog::Log(LOGINFO, "using default network settings");
    g_guiSettings.SetString("network.ipaddress", "192.168.0.100");
    g_guiSettings.SetString("network.subnet", "255.255.255.0");
    g_guiSettings.SetString("network.gateway", "192.168.0.1");
    g_guiSettings.SetString("network.dns", "192.168.0.1");
    g_guiSettings.SetBool("servers.ftpserver", true);
    g_guiSettings.SetBool("servers.webserver", false);
    g_guiSettings.SetBool("locale.timeserver", false);
  }

  /* setup network based on our settings */
  /* network will start it's init procedure */
  g_network.SetupNetwork();

  StartServices();

  m_gWindowManager.Add(new CGUIWindowHome);                     // window id = 0

  CLog::Log(LOGNOTICE, "load default skin:[%s]", g_guiSettings.GetString("lookandfeel.skin").c_str());
  LoadSkin(g_guiSettings.GetString("lookandfeel.skin"));

  m_gWindowManager.Add(new CGUIWindowPrograms);                 // window id = 1
  m_gWindowManager.Add(new CGUIWindowPictures);                 // window id = 2
  m_gWindowManager.Add(new CGUIWindowFileManager);      // window id = 3
  m_gWindowManager.Add(new CGUIWindowVideoFiles);          // window id = 6
  m_gWindowManager.Add(new CGUIWindowSettings);                 // window id = 4
  m_gWindowManager.Add(new CGUIWindowSystemInfo);               // window id = 7
  m_gWindowManager.Add(new CGUIWindowSettingsScreenCalibration); // window id = 11
  m_gWindowManager.Add(new CGUIWindowSettingsCategory);         // window id = 12 slideshow:window id 2007
  m_gWindowManager.Add(new CGUIWindowScripts);                  // window id = 20
  m_gWindowManager.Add(new CGUIWindowVideoNav);                 // window id = 36
  m_gWindowManager.Add(new CGUIWindowVideoPlaylist);            // window id = 28
  m_gWindowManager.Add(new CGUIWindowLoginScreen);            // window id = 29
  m_gWindowManager.Add(new CGUIWindowSettingsProfile);          // window id = 34
  m_gWindowManager.Add(new CGUIWindowGameSaves);               // window id = 35
  m_gWindowManager.Add(new CGUIDialogYesNo);              // window id = 100
  m_gWindowManager.Add(new CGUIDialogProgress);           // window id = 101
  m_gWindowManager.Add(new CGUIDialogKeyboard);           // window id = 103
  m_gWindowManager.Add(&m_guiDialogVolumeBar);          // window id = 104
  m_gWindowManager.Add(&m_guiDialogSeekBar);            // window id = 115
  m_gWindowManager.Add(new CGUIDialogSubMenu);            // window id = 105
  m_gWindowManager.Add(new CGUIDialogContextMenu);        // window id = 106
  m_gWindowManager.Add(&m_guiDialogKaiToast);           // window id = 107
  m_gWindowManager.Add(new CGUIDialogNumeric);            // window id = 109
  m_gWindowManager.Add(new CGUIDialogGamepad);            // window id = 110
  m_gWindowManager.Add(new CGUIDialogButtonMenu);         // window id = 111
  m_gWindowManager.Add(new CGUIDialogMusicScan);          // window id = 112
  m_gWindowManager.Add(new CGUIDialogPlayerControls);     // window id = 113
  m_gWindowManager.Add(new CGUIDialogSlider);             // window id = 145
  m_gWindowManager.Add(new CGUIDialogMusicOSD);           // window id = 120
  m_gWindowManager.Add(new CGUIDialogVisualisationSettings);     // window id = 121
  m_gWindowManager.Add(new CGUIDialogVisualisationPresetList);   // window id = 122
  m_gWindowManager.Add(new CGUIDialogVideoSettings);             // window id = 123
  m_gWindowManager.Add(new CGUIDialogAudioSubtitleSettings);     // window id = 124
  m_gWindowManager.Add(new CGUIDialogVideoBookmarks);      // window id = 125
  // Don't add the filebrowser dialog - it's created and added when it's needed
  m_gWindowManager.Add(new CGUIDialogTrainerSettings);  // window id = 127
  m_gWindowManager.Add(new CGUIDialogNetworkSetup);  // window id = 128
  m_gWindowManager.Add(new CGUIDialogMediaSource);   // window id = 129
  m_gWindowManager.Add(new CGUIDialogProfileSettings); // window id = 130
  m_gWindowManager.Add(new CGUIDialogVideoScan);      // window id = 133
  m_gWindowManager.Add(new CGUIDialogFavourites);     // window id = 134
  m_gWindowManager.Add(new CGUIDialogSongInfo);       // window id = 135
  m_gWindowManager.Add(new CGUIDialogSmartPlaylistEditor);       // window id = 136
  m_gWindowManager.Add(new CGUIDialogSmartPlaylistRule);       // window id = 137
  m_gWindowManager.Add(new CGUIDialogBusy);      // window id = 138
  m_gWindowManager.Add(new CGUIDialogPictureInfo);      // window id = 139
  m_gWindowManager.Add(new CGUIDialogPluginSettings);      // window id = 140

  m_gWindowManager.Add(new CGUIDialogLockSettings); // window id = 131

  m_gWindowManager.Add(new CGUIDialogContentSettings);        // window id = 132

  m_gWindowManager.Add(new CGUIWindowMusicPlayList);          // window id = 500
  m_gWindowManager.Add(new CGUIWindowMusicSongs);             // window id = 501
  m_gWindowManager.Add(new CGUIWindowMusicNav);               // window id = 502
  m_gWindowManager.Add(new CGUIWindowMusicPlaylistEditor);    // window id = 503

  m_gWindowManager.Add(new CGUIDialogSelect);             // window id = 2000
  m_gWindowManager.Add(new CGUIWindowMusicInfo);                // window id = 2001
  m_gWindowManager.Add(new CGUIDialogOK);                 // window id = 2002
  m_gWindowManager.Add(new CGUIWindowVideoInfo);                // window id = 2003
  m_gWindowManager.Add(new CGUIWindowScriptsInfo);              // window id = 2004
  m_gWindowManager.Add(new CGUIWindowFullScreen);         // window id = 2005
  m_gWindowManager.Add(new CGUIWindowVisualisation);      // window id = 2006
  m_gWindowManager.Add(new CGUIWindowSlideShow);          // window id = 2007
  m_gWindowManager.Add(new CGUIDialogFileStacking);       // window id = 2008

  m_gWindowManager.Add(new CGUIWindowOSD);                // window id = 2901
  m_gWindowManager.Add(new CGUIWindowMusicOverlay);       // window id = 2903
  m_gWindowManager.Add(new CGUIWindowVideoOverlay);       // window id = 2904
  m_gWindowManager.Add(new CGUIWindowScreensaver);        // window id = 2900 Screensaver
  m_gWindowManager.Add(new CGUIWindowWeather);            // window id = 2600 WEATHER
  m_gWindowManager.Add(new CGUIWindowStartup);            // startup window (id 2999)

  /* window id's 3000 - 3100 are reserved for python */
  g_DownloadManager.Initialize();

  m_ctrDpad.SetDelays(100, 500); //g_stSettings.m_iMoveDelayController, g_stSettings.m_iRepeatDelayController);

  SAFE_DELETE(m_splash);

  if (g_guiSettings.GetBool("masterlock.startuplock") && 
      g_settings.m_vecProfiles[0].getLockMode() != LOCK_MODE_EVERYONE && 
     !g_settings.m_vecProfiles[0].getLockCode().IsEmpty())
  {
     g_passwordManager.CheckStartUpLock();
  }

  // check if we should use the login screen
  if (g_settings.bUseLoginScreen)
  {
    m_gWindowManager.ActivateWindow(WINDOW_LOGIN_SCREEN);
  }
  else
  {
    RESOLUTION res = INVALID;
    CStdString startupPath = g_SkinInfo.GetSkinPath("startup.xml", &res);
    int startWindow = g_guiSettings.GetInt("lookandfeel.startupwindow");
    // test for a startup window, and activate that instead of home
    if (CFile::Exists(startupPath) && (!g_SkinInfo.OnlyAnimateToHome() || startWindow == WINDOW_HOME))
    {
      m_gWindowManager.ActivateWindow(WINDOW_STARTUP);
    }
    else
    {
      // We need to Popup the WindowHome to initiate the GUIWindowManger for MasterCode popup dialog!
      // Then we can start the StartUpWindow! To prevent BlackScreen if the target Window is Protected with MasterCode!
      m_gWindowManager.ActivateWindow(WINDOW_HOME);
      if (startWindow != WINDOW_HOME)
        m_gWindowManager.ActivateWindow(startWindow);
    }
  }

  g_pythonParser.bStartup = true;
  //g_sysinfo.Refresh();

  CLog::Log(LOGINFO, "removing tempfiles");
  CUtil::RemoveTempFiles();

  if (!m_bAllSettingsLoaded)
  {
    CLog::Log(LOGWARNING, "settings not correct, show dialog");
    CStdString test;
    CUtil::GetHomePath(test);
    CGUIDialogOK *dialog = (CGUIDialogOK *)m_gWindowManager.GetWindow(WINDOW_DIALOG_OK);
    if (dialog)
    {
      dialog->SetHeading(279);
      dialog->SetLine(0, "Error while loading settings");
      dialog->SetLine(1, test);
      dialog->SetLine(2, "");;
      dialog->DoModal();
    }
  }

  //  Show mute symbol
  if (g_stSettings.m_nVolumeLevel == VOLUME_MINIMUM)
    Mute();

  // if the user shutoff the xbox during music scan
  // restore the settings
  if (g_stSettings.m_bMyMusicIsScanning)
  {
    CLog::Log(LOGWARNING,"System rebooted during music scan! ... restoring UseTags and FindRemoteThumbs");
    RestoreMusicScanSettings();
  }

  if (g_guiSettings.GetBool("videolibrary.updateonstartup"))
  {
    CLog::Log(LOGNOTICE, "Updating video library on startup");
    CGUIDialogVideoScan *scanner = (CGUIDialogVideoScan *)m_gWindowManager.GetWindow(WINDOW_DIALOG_VIDEO_SCAN);
    SScraperInfo info;
    VIDEO::SScanSettings settings;
    if (scanner && !scanner->IsScanning())
      scanner->StartScanning("",info,settings,false);
  }

  if (g_guiSettings.GetBool("musiclibrary.updateonstartup"))
  {
    CLog::Log(LOGNOTICE, "Updating music library on startup");
    CGUIDialogMusicScan *scanner = (CGUIDialogMusicScan *)m_gWindowManager.GetWindow(WINDOW_DIALOG_MUSIC_SCAN);
    if (scanner && !scanner->IsScanning())
      scanner->StartScanning("");
  }

  m_slowTimer.StartZero();

#ifdef __APPLE__
  g_xbmcHelper.CaptureAllInput();
#endif

  CLog::Log(LOGNOTICE, "initialize done");

  m_bInitializing = false;

  // final check for debugging combo
  CheckForDebugButtonCombo();

  // reset our screensaver (starts timers etc.)
  ResetScreenSaver();
  return S_OK;
}

void CApplication::PrintXBEToLCD(const char* xbePath)
{
#ifdef HAS_LCD
  int pLine = 0;
  CStdString strXBEName;
  if (!CUtil::GetXBEDescription(xbePath, strXBEName))
  {
    CUtil::GetDirectoryName(xbePath, strXBEName);
    CUtil::ShortenFileName(strXBEName);
    CUtil::RemoveIllegalChars(strXBEName);
  }
  // crop to LCD screen size
  if ((int)strXBEName.size() > g_advancedSettings.m_lcdColumns)
    strXBEName = strXBEName.Left(g_advancedSettings.m_lcdColumns);
  if (g_lcd)
  {
    g_infoManager.SetLaunchingXBEName(strXBEName);
    g_lcd->Render(ILCD::LCD_MODE_XBE_LAUNCH);
  }
#endif
}

void CApplication::StartIdleThread()
{
  m_idleThread.Create(false, 0x100);
}

void CApplication::StopIdleThread()
{
  m_idleThread.StopThread();
}

void CApplication::StartWebServer()
{
  if (g_guiSettings.GetBool("servers.webserver") && g_network.IsAvailable() )
  {
    CLog::Log(LOGNOTICE, "Webserver: Starting...");
    CSectionLoader::Load("LIBHTTP");
    m_pWebServer = new CWebServer();
    m_pWebServer->Start(g_network.m_networkinfo.ip, atoi(g_guiSettings.GetString("servers.webserverport")), "Q:\\web", false);
    if (m_pWebServer)
    {
      m_pWebServer->SetUserName(g_guiSettings.GetString("servers.webserverusername").c_str());
       m_pWebServer->SetPassword(g_guiSettings.GetString("servers.webserverpassword").c_str());
    }
    if (m_pWebServer && m_pXbmcHttp && g_stSettings.m_HttpApiBroadcastLevel>=1)
      g_applicationMessenger.HttpApi("broadcastlevel; StartUp;1");
  }
}

void CApplication::StopWebServer()
{
  if (m_pWebServer)
  {
    CLog::Log(LOGNOTICE, "Webserver: Stopping...");
    m_pWebServer->Stop();
    delete m_pWebServer;
    m_pWebServer = NULL;
    CSectionLoader::Unload("LIBHTTP");
    CLog::Log(LOGNOTICE, "Webserver: Stopped...");
  }
}

void CApplication::StartFtpServer()
{
#ifdef HAS_FTP_SERVER
  if ( g_guiSettings.GetBool("servers.ftpserver") && g_network.IsAvailable() )
  {
    CLog::Log(LOGNOTICE, "XBFileZilla: Starting...");
    if (!m_pFileZilla)
    {
      CStdString xmlpath = "special://xbmc/system/";
      // if user didn't upgrade properly,
      // check whether UserData/FileZilla Server.xml exists
      if (CFile::Exists(g_settings.GetUserDataItem("FileZilla Server.xml")))
        xmlpath = g_settings.GetUserDataFolder();

      // check file size and presence
      CFile xml;
      if (xml.Open(xmlpath+"FileZilla Server.xml",true) && xml.GetLength() > 0)
      {
        m_pFileZilla = new CXBFileZilla(_P(xmlpath));
        m_pFileZilla->Start(false);
      }
      else
      {
        // 'FileZilla Server.xml' does not exist or is corrupt, 
        // falling back to ftp emergency recovery mode
        CLog::Log(LOGNOTICE, "XBFileZilla: 'FileZilla Server.xml' is missing or is corrupt!");
        CLog::Log(LOGNOTICE, "XBFileZilla: Starting ftp emergency recovery mode");
        StartFtpEmergencyRecoveryMode();
      }
      xml.Close();
    }
  }
#endif
}

void CApplication::StopFtpServer()
{
#ifdef HAS_FTP_SERVER
  if (m_pFileZilla)
  {
    CLog::Log(LOGINFO, "XBFileZilla: Stopping...");
/*
    std::vector<SXFConnection> mConnections;
    std::vector<SXFConnection>::iterator it;

    m_pFileZilla->GetAllConnections(mConnections);

    for(it = mConnections.begin();it != mConnections.end();it++)
    {
      CLog(LOGDEBUG, "%s - Closing FTP connection %i", it->mId);
      m_pFileZilla->CloseConnection(it->mId);
    }
*/
    m_pFileZilla->Stop();
    delete m_pFileZilla;
    m_pFileZilla = NULL;

    CLog::Log(LOGINFO, "XBFileZilla: Stopped");
  }
#endif
}

void CApplication::StartTimeServer()
{
#ifdef HAS_TIME_SERVER
  if (g_guiSettings.GetBool("locale.timeserver") && g_network.IsAvailable() )
  {
    if( !m_psntpClient )
    {
      CSectionLoader::Load("SNTP");
      CLog::Log(LOGNOTICE, "start timeserver client");

      m_psntpClient = new CSNTPClient();
      m_psntpClient->Update();
    }
  }
#endif
}

void CApplication::StopTimeServer()
{
#ifdef HAS_TIME_SERVER
  if( m_psntpClient )
  {
    CLog::Log(LOGNOTICE, "stop time server client");
    SAFE_DELETE(m_psntpClient);
    CSectionLoader::Unload("SNTP");
  }
#endif
}

void CApplication::StartUPnP()
{
#ifdef HAS_UPNP
    StartUPnPClient();
    StartUPnPServer();
    StartUPnPRenderer();
#endif
}

void CApplication::StopUPnP()
{
#ifdef HAS_UPNP
  if (CUPnP::IsInstantiated())
  {
    CLog::Log(LOGNOTICE, "stopping upnp");
    CUPnP::ReleaseInstance();
  }
#endif
}

void CApplication::StartEventServer()
{
#ifdef HAS_EVENT_SERVER
  CEventServer* server = CEventServer::GetInstance();
  if (!server)
  {
    CLog::Log(LOGERROR, "ES: Out of memory");
    return;
  }
  if (g_guiSettings.GetBool("remoteevents.enabled"))
  {
    CLog::Log(LOGNOTICE, "ES: Starting event server");
    server->StartServer();
  }
#endif
}

bool CApplication::StopEventServer(bool promptuser)
{
#ifdef HAS_EVENT_SERVER
  CEventServer* server = CEventServer::GetInstance();
  if (!server)
  {
    CLog::Log(LOGERROR, "ES: Out of memory");
    return false;
  }
  if (promptuser)
  {
    if (server->GetNumberOfClients() > 0)
    {
      bool cancelled = false;
      if (!CGUIDialogYesNo::ShowAndGetInput(13140, 13141, 13142, 20022,
                                            -1, -1, cancelled, 10000)
          || cancelled)
      {
        CLog::Log(LOGNOTICE, "ES: Not stopping event server");
        return false;
      }
    }
    CLog::Log(LOGNOTICE, "ES: Stopping event server with confirmation");
  }
  else
  {
    CLog::Log(LOGNOTICE, "ES: Stopping event server");
  }
  CEventServer::GetInstance()->StopServer();
#endif
  return true;
}

void CApplication::RefreshEventServer()
{
#ifdef HAS_EVENT_SERVER
  if (g_guiSettings.GetBool("remoteevents.enabled"))
  {
    CEventServer::GetInstance()->RefreshSettings();
  }
#endif
}

void CApplication::StartUPnPRenderer()
{
#ifdef HAS_UPNP
  if (g_guiSettings.GetBool("upnp.renderer"))
  {
    CLog::Log(LOGNOTICE, "starting upnp renderer");
    CUPnP::GetInstance()->StartRenderer();
  }
#endif
}

void CApplication::StopUPnPRenderer()
{
#ifdef HAS_UPNP
  if (CUPnP::IsInstantiated())
  {
    CLog::Log(LOGNOTICE, "stopping upnp renderer");
    CUPnP::GetInstance()->StopRenderer();
  }
#endif
}

void CApplication::StartUPnPClient()
{
#ifdef HAS_UPNP
  if (g_guiSettings.GetBool("upnp.client"))
  {
    CLog::Log(LOGNOTICE, "starting upnp client");
    CUPnP::GetInstance()->StartClient();
  }
#endif
}

void CApplication::StopUPnPClient()
{
#ifdef HAS_UPNP
  if (CUPnP::IsInstantiated())
  {
    CLog::Log(LOGNOTICE, "stopping upnp client");
    CUPnP::GetInstance()->StopClient();
  }
#endif
}

void CApplication::StartUPnPServer()
{
#ifdef HAS_UPNP
  if (g_guiSettings.GetBool("upnp.server"))
  {
    CLog::Log(LOGNOTICE, "starting upnp server");
    CUPnP::GetInstance()->StartServer();
  }
#endif
}

void CApplication::StopUPnPServer()
{
#ifdef HAS_UPNP
  if (CUPnP::IsInstantiated())
  {
    CLog::Log(LOGNOTICE, "stopping upnp server");
    CUPnP::GetInstance()->StopServer();
  }
#endif
}

void CApplication::StartLEDControl(bool switchoff)
{
#ifdef HAS_XBOX_HARDWARE
  if (switchoff && g_guiSettings.GetInt("system.ledcolour") != LED_COLOUR_NO_CHANGE)
  {
    if ( IsPlayingVideo() && (g_guiSettings.GetInt("system.leddisableonplayback") == LED_PLAYBACK_VIDEO))
      ILED::CLEDControl(LED_COLOUR_OFF);
    if ( IsPlayingAudio() && (g_guiSettings.GetInt("system.leddisableonplayback") == LED_PLAYBACK_MUSIC))
      ILED::CLEDControl(LED_COLOUR_OFF);
    if ( ((IsPlayingVideo() || IsPlayingAudio())) && (g_guiSettings.GetInt("system.leddisableonplayback") == LED_PLAYBACK_VIDEO_MUSIC))
      ILED::CLEDControl(LED_COLOUR_OFF);
  }
  else if (!switchoff)
    ILED::CLEDControl(g_guiSettings.GetInt("system.ledcolour"));
#endif
}

void CApplication::DimLCDOnPlayback(bool dim)
{
#ifdef HAS_LCD
  if(g_lcd && dim && (g_guiSettings.GetInt("lcd.disableonplayback") != LED_PLAYBACK_OFF) && (g_guiSettings.GetInt("lcd.type") != LCD_TYPE_NONE))
  {
    if ( (IsPlayingVideo()) && g_guiSettings.GetInt("lcd.disableonplayback") == LED_PLAYBACK_VIDEO)
      g_lcd->SetBackLight(0);
    if ( (IsPlayingAudio()) && g_guiSettings.GetInt("lcd.disableonplayback") == LED_PLAYBACK_MUSIC)
      g_lcd->SetBackLight(0);
    if ( ((IsPlayingVideo() || IsPlayingAudio())) && g_guiSettings.GetInt("lcd.disableonplayback") == LED_PLAYBACK_VIDEO_MUSIC)
      g_lcd->SetBackLight(0);
  }
  else if(!dim)
    g_lcd->SetBackLight(g_guiSettings.GetInt("lcd.backlight"));
#endif
}

void CApplication::StartServices()
{
#ifdef HAS_XBOX_HARDWARE
  StartIdleThread();
#endif

  CheckDate();
  StartLEDControl(false);

  // Start Thread for DVD Mediatype detection
  CLog::Log(LOGNOTICE, "start dvd mediatype detection");
  m_DetectDVDType.Create(false, THREAD_MINSTACKSIZE);

  CLog::Log(LOGNOTICE, "initializing playlistplayer");
  g_playlistPlayer.SetRepeat(PLAYLIST_MUSIC, g_stSettings.m_bMyMusicPlaylistRepeat ? PLAYLIST::REPEAT_ALL : PLAYLIST::REPEAT_NONE);
  g_playlistPlayer.SetShuffle(PLAYLIST_MUSIC, g_stSettings.m_bMyMusicPlaylistShuffle);
  g_playlistPlayer.SetRepeat(PLAYLIST_VIDEO, g_stSettings.m_bMyVideoPlaylistRepeat ? PLAYLIST::REPEAT_ALL : PLAYLIST::REPEAT_NONE);
  g_playlistPlayer.SetShuffle(PLAYLIST_VIDEO, g_stSettings.m_bMyVideoPlaylistShuffle);
  CLog::Log(LOGNOTICE, "DONE initializing playlistplayer");

#ifdef HAS_LCD
  CLCDFactory factory;
  g_lcd = factory.Create();
  if (g_lcd)
  {
    g_lcd->Initialize();
  }
#endif

#ifdef HAS_XBOX_HARDWARE
  if (g_guiSettings.GetBool("system.autotemperature"))
  {
    CLog::Log(LOGNOTICE, "start fancontroller");
    CFanController::Instance()->Start(g_guiSettings.GetInt("system.targettemperature"), g_guiSettings.GetInt("system.minfanspeed"));
  }
  else if (g_guiSettings.GetBool("system.fanspeedcontrol"))
  {
    CLog::Log(LOGNOTICE, "setting fanspeed");
    CFanController::Instance()->SetFanSpeed(g_guiSettings.GetInt("system.fanspeed"));
  }
  int setting_level = g_guiSettings.GetInt("harddisk.aamlevel");
  if (setting_level == AAM_QUIET)
    XKHDD::SetAAMLevel(0x80);
  else if (setting_level == AAM_FAST)
    XKHDD::SetAAMLevel(0xFE);
  setting_level = g_guiSettings.GetInt("harddisk.apmlevel");
  switch(setting_level)
  {
  case APM_LOPOWER:
    XKHDD::SetAPMLevel(0x80);
    break;
  case APM_HIPOWER:
    XKHDD::SetAPMLevel(0xFE);
    break;
  case APM_LOPOWER_STANDBY:
    XKHDD::SetAPMLevel(0x01);
    break;
  case APM_HIPOWER_STANDBY:
    XKHDD::SetAPMLevel(0x7F);
    break;
  }
#endif
}

void CApplication::CheckDate()
{
  CLog::Log(LOGNOTICE, "Checking the Date!");
  // Check the Date: Year, if it is  above 2099 set to 2004!
  SYSTEMTIME CurTime;
  SYSTEMTIME NewTime;
  GetLocalTime(&CurTime);
  GetLocalTime(&NewTime);
  CLog::Log(LOGINFO, "Current Date is: %i-%i-%i", CurTime.wDay, CurTime.wMonth, CurTime.wYear);
  if ((CurTime.wYear > 2099) || (CurTime.wYear < 2001) )        // XBOX MS Dashboard also uses min/max DateYear 2001/2099 !!
  {
    CLog::Log(LOGNOTICE, "- The Date is Wrong: Setting New Date!");
    NewTime.wYear       = 2004; // 2004
    NewTime.wMonth      = 1;  // January
    NewTime.wDayOfWeek  = 1;  // Monday
    NewTime.wDay        = 5;  // Monday 05.01.2004!!
    NewTime.wHour       = 12;
    NewTime.wMinute     = 0;

    FILETIME stNewTime, stCurTime;
    SystemTimeToFileTime(&NewTime, &stNewTime);
    SystemTimeToFileTime(&CurTime, &stCurTime);
#ifdef HAS_XBOX_HARDWARE
    NtSetSystemTime(&stNewTime, &stCurTime);    // Set a Default Year 2004!
#endif
    CLog::Log(LOGNOTICE, "- New Date is now: %i-%i-%i",NewTime.wDay, NewTime.wMonth, NewTime.wYear);
  }
  return ;
}

void CApplication::StopServices()
{
  g_network.NetworkMessage(CNetwork::SERVICES_DOWN, 0);

  CLog::Log(LOGNOTICE, "stop dvd detect media");
  m_DetectDVDType.StopThread();

#ifdef HAS_XBOX_HARDWARE
  CLog::Log(LOGNOTICE, "stop fancontroller");
  CFanController::Instance()->Stop();
  CFanController::RemoveInstance();
  StopIdleThread();
#endif  
}

void CApplication::DelayLoadSkin()
{
  m_dwSkinTime = timeGetTime() + 2000;
  return ;
}

void CApplication::CancelDelayLoadSkin()
{
  m_dwSkinTime = 0;
}

void CApplication::ReloadSkin()
{
  CGUIMessage msg(GUI_MSG_LOAD_SKIN, (DWORD) -1, m_gWindowManager.GetActiveWindow());
  g_graphicsContext.SendMessage(msg);
  // Reload the skin, restoring the previously focused control.  We need this as
  // the window unload will reset all control states.
  CGUIWindow* pWindow = m_gWindowManager.GetWindow(m_gWindowManager.GetActiveWindow());
  unsigned iCtrlID = pWindow->GetFocusedControlID();
  g_application.LoadSkin(g_guiSettings.GetString("lookandfeel.skin"));
  pWindow = m_gWindowManager.GetWindow(m_gWindowManager.GetActiveWindow());
  if (pWindow && pWindow->HasSaveLastControl())
  {
    CGUIMessage msg3(GUI_MSG_SETFOCUS, m_gWindowManager.GetActiveWindow(), iCtrlID, 0);
    pWindow->OnMessage(msg3);
  }
}

void CApplication::LoadSkin(const CStdString& strSkin)
{
  bool bPreviousPlayingState=false;
  bool bPreviousRenderingState=false;
  if (g_application.m_pPlayer && g_application.IsPlayingVideo())
  {
    bPreviousPlayingState = !g_application.m_pPlayer->IsPaused();
    if (bPreviousPlayingState)
      g_application.m_pPlayer->Pause();
#ifdef HAS_VIDEO_PLAYBACK
    if (!g_renderManager.Paused())
    {
      if (m_gWindowManager.GetActiveWindow() == WINDOW_FULLSCREEN_VIDEO)
     {
        m_gWindowManager.ActivateWindow(WINDOW_HOME);
        bPreviousRenderingState = true;
      }
    }
#endif
  }
  //stop the busy renderer if it's running before we lock the graphiccontext or we could deadlock.
  g_ApplicationRenderer.Stop();
  // close the music and video overlays (they're re-opened automatically later)
  CSingleLock lock(g_graphicsContext);

  m_dwSkinTime = 0;

  CStdString strHomePath;
  CStdString strSkinPath = "Q:\\skin\\" + strSkin;

  CLog::Log(LOGINFO, "  load skin from:%s", strSkinPath.c_str());

  // save the current window details
  int currentWindow = m_gWindowManager.GetActiveWindow();
  vector<DWORD> currentModelessWindows;
  m_gWindowManager.GetActiveModelessWindows(currentModelessWindows);

  CLog::Log(LOGINFO, "  delete old skin...");
  UnloadSkin();

  // Load in the skin.xml file if it exists
  g_SkinInfo.Load(strSkinPath);

  CLog::Log(LOGINFO, "  load fonts for skin...");
  g_graphicsContext.SetMediaDir(strSkinPath);
  g_directoryCache.ClearSubPaths(strSkinPath);
  if (g_langInfo.ForceUnicodeFont() && !g_fontManager.IsFontSetUnicode(g_guiSettings.GetString("lookandfeel.font")))
  {
    CLog::Log(LOGINFO, "    language needs a ttf font, loading first ttf font available");
    CStdString strFontSet;
    if (g_fontManager.GetFirstFontSetUnicode(strFontSet))
    {
      CLog::Log(LOGINFO, "    new font is '%s'", strFontSet.c_str());
      g_guiSettings.SetString("lookandfeel.font", strFontSet);
      g_settings.Save();
    }
    else
      CLog::Log(LOGERROR, "    no ttf font found, but needed for the language %s.", g_guiSettings.GetString("locale.language").c_str());
  }
  g_colorManager.Load(g_guiSettings.GetString("lookandfeel.skincolors"));

  g_fontManager.LoadFonts(g_guiSettings.GetString("lookandfeel.font"));

  // load in the skin strings
  CStdString skinPath, skinEnglishPath;
  CUtil::AddFileToFolder(strSkinPath, "language", skinPath);
  CUtil::AddFileToFolder(skinPath, g_guiSettings.GetString("locale.language"), skinPath);
  CUtil::AddFileToFolder(skinPath, "strings.xml", skinPath);

  CUtil::AddFileToFolder(strSkinPath, "language", skinEnglishPath);
  CUtil::AddFileToFolder(skinEnglishPath, "English", skinEnglishPath);
  CUtil::AddFileToFolder(skinEnglishPath, "strings.xml", skinEnglishPath);

  g_localizeStrings.LoadSkinStrings(skinPath, skinEnglishPath);

  LARGE_INTEGER start;
  QueryPerformanceCounter(&start);

  CLog::Log(LOGINFO, "  load new skin...");
  CGUIWindowHome *pHome = (CGUIWindowHome *)m_gWindowManager.GetWindow(WINDOW_HOME);
  if (!g_SkinInfo.Check(strSkinPath) || !pHome || !pHome->Load("Home.xml"))
  {
    // failed to load home.xml
    // fallback to default skin
    if ( strcmpi(strSkin.c_str(), DEFAULT_SKIN) != 0)
    {
      CLog::Log(LOGERROR, "failed to load home.xml for skin:%s, fallback to \"%s\" skin", strSkin.c_str(), DEFAULT_SKIN);
      g_guiSettings.SetString("lookandfeel.skin", DEFAULT_SKIN);
      LoadSkin(g_guiSettings.GetString("lookandfeel.skin"));
      return ;
    }
  }

  // Load the user windows
  LoadUserWindows(strSkinPath);

  LARGE_INTEGER end, freq;
  QueryPerformanceCounter(&end);
  QueryPerformanceFrequency(&freq);
  CLog::Log(LOGDEBUG,"Load Skin XML: %.2fms", 1000.f * (end.QuadPart - start.QuadPart) / freq.QuadPart);

  CLog::Log(LOGINFO, "  initialize new skin...");
  m_guiPointer.AllocResources(true);
  m_guiDialogVolumeBar.AllocResources(true);
  m_guiDialogSeekBar.AllocResources(true);
  m_guiDialogKaiToast.AllocResources(true);
  m_guiDialogMuteBug.AllocResources(true);
  m_gWindowManager.AddMsgTarget(this);
  m_gWindowManager.AddMsgTarget(&g_playlistPlayer);
  m_gWindowManager.AddMsgTarget(&g_infoManager);
  m_gWindowManager.SetCallback(*this);
  m_gWindowManager.Initialize();
  g_audioManager.Initialize(CAudioContext::DEFAULT_DEVICE);
  g_audioManager.Load();

  CGUIDialogFullScreenInfo* pDialog = NULL;
  RESOLUTION res;
  CStdString strPath = g_SkinInfo.GetSkinPath("DialogFullScreenInfo.xml", &res);
  if (CFile::Exists(strPath))
    pDialog = new CGUIDialogFullScreenInfo;
   
  if (pDialog)
    m_gWindowManager.Add(pDialog); // window id = 142

  CLog::Log(LOGINFO, "  skin loaded...");

  // leave the graphics lock
  lock.Leave();
  g_ApplicationRenderer.Start();

  // restore windows
  if (currentWindow != WINDOW_INVALID)
  {
    m_gWindowManager.ActivateWindow(currentWindow);
    for (unsigned int i = 0; i < currentModelessWindows.size(); i++)
    {
      CGUIDialog *dialog = (CGUIDialog *)m_gWindowManager.GetWindow(currentModelessWindows[i]);
      if (dialog) dialog->Show();
    }
  }

  if (g_application.m_pPlayer && g_application.IsPlayingVideo())
  {
    if (bPreviousPlayingState)
      g_application.m_pPlayer->Pause();
    if (bPreviousRenderingState)
      m_gWindowManager.ActivateWindow(WINDOW_FULLSCREEN_VIDEO);
  }
}

void CApplication::UnloadSkin()
{
  g_ApplicationRenderer.Stop();
  g_audioManager.DeInitialize(CAudioContext::DEFAULT_DEVICE);

  m_gWindowManager.DeInitialize();

  //These windows are not handled by the windowmanager (why not?) so we should unload them manually
  CGUIMessage msg(GUI_MSG_WINDOW_DEINIT, 0, 0);
  m_guiPointer.OnMessage(msg);
  m_guiPointer.ResetControlStates();
  m_guiPointer.FreeResources(true);
  m_guiDialogMuteBug.OnMessage(msg);
  m_guiDialogMuteBug.ResetControlStates();
  m_guiDialogMuteBug.FreeResources(true);

  // remove the skin-dependent window
  m_gWindowManager.Delete(WINDOW_DIALOG_FULLSCREEN_INFO);

  g_TextureManager.Cleanup();

  g_fontManager.Clear();

  g_colorManager.Clear();

  g_charsetConverter.reset();

  g_infoManager.Clear();
}

bool CApplication::LoadUserWindows(const CStdString& strSkinPath)
{
  WIN32_FIND_DATA FindFileData;
  WIN32_FIND_DATA NextFindFileData;
  HANDLE hFind;
  TiXmlDocument xmlDoc;
  RESOLUTION resToUse = INVALID;

  // Start from wherever home.xml is
  g_SkinInfo.GetSkinPath("Home.xml", &resToUse);
  std::vector<CStdString> vecSkinPath;
  if (resToUse == HDTV_1080i)
    vecSkinPath.push_back(CUtil::AddFileToFolder(strSkinPath, g_SkinInfo.GetDirFromRes(HDTV_1080i)));
  if (resToUse == HDTV_720p)
    vecSkinPath.push_back(CUtil::AddFileToFolder(strSkinPath, g_SkinInfo.GetDirFromRes(HDTV_720p)));
  if (resToUse == PAL_16x9 || resToUse == NTSC_16x9 || resToUse == HDTV_480p_16x9 || resToUse == HDTV_720p || resToUse == HDTV_1080i)
    vecSkinPath.push_back(CUtil::AddFileToFolder(strSkinPath, g_SkinInfo.GetDirFromRes(g_SkinInfo.GetDefaultWideResolution())));
  vecSkinPath.push_back(CUtil::AddFileToFolder(strSkinPath, g_SkinInfo.GetDirFromRes(g_SkinInfo.GetDefaultResolution())));
  for (unsigned int i = 0;i < vecSkinPath.size();++i)
  {
    CStdString strPath = CUtil::AddFileToFolder(vecSkinPath[i], "custom*.xml");
    CLog::Log(LOGINFO, "Loading user windows, path %s", vecSkinPath[i].c_str());
    hFind = FindFirstFile(_P(strPath).c_str(), &NextFindFileData);

    CStdString strFileName;
    while (hFind != INVALID_HANDLE_VALUE)
    {
      FindFileData = NextFindFileData;

      if (!FindNextFile(hFind, &NextFindFileData))
      {
        FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
      }

      // skip "up" directories, which come in all queries
      if (!strcmp(FindFileData.cFileName, ".") || !strcmp(FindFileData.cFileName, ".."))
        continue;

      strFileName = CUtil::AddFileToFolder(vecSkinPath[i], FindFileData.cFileName);
      CLog::Log(LOGINFO, "Loading skin file: %s", strFileName.c_str());
      CStdString strLower(FindFileData.cFileName);
      strLower.MakeLower();
      strLower = CUtil::AddFileToFolder(vecSkinPath[i], strLower);
      if (!xmlDoc.LoadFile(strFileName) && !xmlDoc.LoadFile(strLower))
      {
        CLog::Log(LOGERROR, "unable to load:%s, Line %d\n%s", strFileName.c_str(), xmlDoc.ErrorRow(), xmlDoc.ErrorDesc());
        continue;
      }

      // Root element should be <window>
      TiXmlElement* pRootElement = xmlDoc.RootElement();
      CStdString strValue = pRootElement->Value();
      if (!strValue.Equals("window"))
      {
        CLog::Log(LOGERROR, "file :%s doesnt contain <window>", strFileName.c_str());
        continue;
      }

      // Read the <type> element to get the window type to create
      // If no type is specified, create a CGUIWindow as default
      CGUIWindow* pWindow = NULL;
      CStdString strType;
      if (pRootElement->Attribute("type"))
        strType = pRootElement->Attribute("type");
      else
      {
        const TiXmlNode *pType = pRootElement->FirstChild("type");
        if (pType && pType->FirstChild())
          strType = pType->FirstChild()->Value();
      }
      if (strType.Equals("dialog"))
        pWindow = new CGUIDialog(0, "");
      else if (strType.Equals("submenu"))
        pWindow = new CGUIDialogSubMenu();
      else if (strType.Equals("buttonmenu"))
        pWindow = new CGUIDialogButtonMenu();
      else
        pWindow = new CGUIStandardWindow();

      int id = WINDOW_INVALID;
      if (!pRootElement->Attribute("id", &id))
      {
        const TiXmlNode *pType = pRootElement->FirstChild("id");
        if (pType && pType->FirstChild())
          id = atol(pType->FirstChild()->Value());
      }
      // Check to make sure the pointer isn't still null
      if (pWindow == NULL || id == WINDOW_INVALID)
      {
        CLog::Log(LOGERROR, "Out of memory / Failed to create new object in LoadUserWindows");
        return false;
      }
      if (m_gWindowManager.GetWindow(WINDOW_HOME + id))
      {
        delete pWindow;
        continue;
      }
      // set the window's xml file, and add it to the window manager.
      pWindow->SetXMLFile(FindFileData.cFileName);
      pWindow->SetID(WINDOW_HOME + id);
      m_gWindowManager.AddCustomWindow(pWindow);
    }
    CloseHandle(hFind);
  }
  return true;
}

#ifdef HAS_XBOX_D3D  // needed for screenshot
void CApplication::Render()
{
#else
void CApplication::RenderNoPresent()
{
#endif
  // don't do anything that would require graphiccontext to be locked before here in fullscreen.
  // that stuff should go into renderfullscreen instead as that is called from the renderin thread
#ifdef HAS_XBOX_HARDWARE  // Win32 renders from the main thread, not from the player thread
  // dont show GUI when playing full screen video
  if (g_graphicsContext.IsFullScreenVideo() && IsPlaying() && !IsPaused())
  {
    Sleep(50);
    ResetScreenSaver();
    g_infoManager.ResetCache();
    return;
  }
#endif

  g_ApplicationRenderer.Render();
}

void CApplication::DoRender()
{
  if(!m_pd3dDevice)
    return;

  g_graphicsContext.Lock();

  m_pd3dDevice->BeginScene();

  m_gWindowManager.UpdateModelessVisibility();

  // draw GUI
  g_graphicsContext.Clear();
  //SWATHWIDTH of 4 improves fillrates (performance investigator)
#ifdef HAS_XBOX_D3D
  m_pd3dDevice->SetRenderState(D3DRS_SWATHWIDTH, 4);
#endif
  m_gWindowManager.Render();


  // if we're recording an audio stream then show blinking REC
  if (!g_graphicsContext.IsFullScreenVideo())
  {
    if (m_pPlayer && m_pPlayer->IsRecording() )
    {
      static int iBlinkRecord = 0;
      iBlinkRecord++;
      if (iBlinkRecord > 25)
      {
        CGUIFont* pFont = g_fontManager.GetFont("font13");
        CGUITextLayout::DrawText(pFont, 60, 50, 0xffff0000, 0, "REC", 0);
      }

      if (iBlinkRecord > 50)
        iBlinkRecord = 0;
    }
  }

  // Now render any dialogs
  m_gWindowManager.RenderDialogs();

  // Render the mouse pointer
  if (g_Mouse.IsActive())
  {
    m_guiPointer.Render();
  }

  {
    // free memory if we got les then 10megs free ram
    MEMORYSTATUS stat;
    GlobalMemoryStatus(&stat);
    DWORD dwMegFree = (DWORD)(stat.dwAvailPhys / (1024 * 1024));
    if (dwMegFree <= 10)
    {
      g_TextureManager.Flush();
    }

    // reset image scaling and effect states
    g_graphicsContext.SetRenderingResolution(g_graphicsContext.GetVideoResolution(), 0, 0, false);

    // If we have the remote codes enabled, then show them
    if (g_advancedSettings.m_displayRemoteCodes)
    {
#ifdef HAS_IR_REMOTE
      XBIR_REMOTE* pRemote = &m_DefaultIR_Remote;
      static iRemoteCode = 0;
      static iShowRemoteCode = 0;
      if (pRemote->wButtons)
      {
        iRemoteCode = 255 - pRemote->wButtons; // remote OBC code is 255-wButtons
        iShowRemoteCode = 50;
      }
      if (iShowRemoteCode > 0)
      {
        CStdStringW wszText;
        wszText.Format(L"Remote Code: %i", iRemoteCode);
        float x = 0.08f * g_graphicsContext.GetWidth();
        float y = 0.12f * g_graphicsContext.GetHeight();
#ifndef _DEBUG
        if (LOG_LEVEL_DEBUG_FREEMEM > g_advancedSettings.m_logLevel)
          y = 0.08f * g_graphicsContext.GetHeight();
#endif
        CGUITextLayout::DrawOutlineText(g_fontManager.GetFont("font13"), x, y, 0xffffffff, 0xff000000, 2, wszText);
        iShowRemoteCode--;
      }
#endif
    }

    RenderMemoryStatus();
  }

  m_pd3dDevice->EndScene();
#ifdef HAS_XBOX_D3D
  m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
#endif
  g_graphicsContext.Unlock();

  // reset our info cache - we do this at the end of Render so that it is
  // fresh for the next process(), or after a windowclose animation (where process()
  // isn't called)
  g_infoManager.ResetCache();
}

#ifndef HAS_XBOX_D3D
void CApplication::Render()
{
  g_graphicsContext.Lock();
  { // frame rate limiter (really bad, but it does the trick :p)
    const static unsigned int singleFrameTime = 10;
    static unsigned int lastFrameTime = 0;
    unsigned int currentTime = timeGetTime();
    if (lastFrameTime + singleFrameTime > currentTime)
      Sleep(lastFrameTime + singleFrameTime - currentTime);
    lastFrameTime = timeGetTime();
  }
  RenderNoPresent();
  // Present the backbuffer contents to the display
  if (m_pd3dDevice) m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
  g_graphicsContext.Unlock();
}
#endif

void CApplication::RenderMemoryStatus()
{
  g_infoManager.UpdateFPS();
#if !defined(_DEBUG) && !defined(PROFILE)
  if (LOG_LEVEL_DEBUG_FREEMEM <= g_advancedSettings.m_logLevel)
#endif
  {
    // reset the window scaling and fade status
    RESOLUTION res = g_graphicsContext.GetVideoResolution();
    g_graphicsContext.SetRenderingResolution(res, 0, 0, false);

    CStdStringW wszText;
    MEMORYSTATUS stat;
    GlobalMemoryStatus(&stat);
    wszText.Format(L"FreeMem %d/%d KB, FPS %2.1f, CPU %2.0f%%", stat.dwAvailPhys/1024, stat.dwTotalPhys/1024, g_infoManager.GetFPS(), (1.0f - m_idleThread.GetRelativeUsage())*100);

    float x = 0.04f * g_graphicsContext.GetWidth() + g_settings.m_ResInfo[res].Overscan.left;
    float y = 0.04f * g_graphicsContext.GetHeight() + g_settings.m_ResInfo[res].Overscan.top;
    CGUITextLayout::DrawOutlineText(g_fontManager.GetFont("font13"), x, y, 0xffffffff, 0xff000000, 2, wszText);
  }
}

// OnKey() translates the key into a CAction which is sent on to our Window Manager.
// The window manager will return true if the event is processed, false otherwise.
// If not already processed, this routine handles global keypresses.  It returns
// true if the key has been processed, false otherwise.

bool CApplication::OnKey(CKey& key)
{
  // Turn the mouse off, as we've just got a keypress from controller or remote
  g_Mouse.SetInactive();
  CAction action;
  
  // get the current active window
  int iWin = m_gWindowManager.GetActiveWindow() & WINDOW_ID_MASK;

  // this will be checked for certain keycodes that need
  // special handling if the screensaver is active
  CButtonTranslator::GetInstance().GetAction(iWin, key, action);

  // a key has been pressed.
  // Reset the screensaver timer
  // but not for the analog thumbsticks/triggers
  if (!key.IsAnalogButton())
  {
    // reset harddisk spindown timer
    m_bSpinDown = false;
    m_bNetworkSpinDown = false;

    // reset Idle Timer
    m_idleTimer.StartZero();

    ResetScreenSaver();

    // allow some keys to be processed while the screensaver is active
    if (ResetScreenSaverWindow())
    {
      return true;
    }  
  }

  // change this if we have a dialog up
  if (m_gWindowManager.HasModalDialog())
  {
    iWin = m_gWindowManager.GetTopMostModalDialogID() & WINDOW_ID_MASK;
  }
  if (iWin == WINDOW_DIALOG_FULLSCREEN_INFO)
  { // fullscreen info dialog - special case
    CButtonTranslator::GetInstance().GetAction(iWin, key, action);

#ifdef HAS_SDL
    g_Keyboard.Reset();
#endif
    if (OnAction(action))
      return true;

    // fallthrough to the main window
    iWin = WINDOW_FULLSCREEN_VIDEO;
  }
  if (iWin == WINDOW_FULLSCREEN_VIDEO)
  {
    // current active window is full screen video.
    if (g_application.m_pPlayer && g_application.m_pPlayer->IsInMenu())
    {
      // if player is in some sort of menu, (ie DVDMENU) map buttons differently
      CButtonTranslator::GetInstance().GetAction(WINDOW_VIDEO_MENU, key, action);
    }
    else
    {
      // no then use the fullscreen window section of keymap.xml to map key->action
      CButtonTranslator::GetInstance().GetAction(iWin, key, action);
    }
  }
  else
  {
    // current active window isnt the fullscreen window
    // just use corresponding section from keymap.xml
    // to map key->action

    // first determine if we should use keyboard input directly
    bool useKeyboard = key.FromKeyboard() && (iWin == WINDOW_DIALOG_KEYBOARD || iWin == WINDOW_DIALOG_NUMERIC);
    CGUIWindow *window = m_gWindowManager.GetWindow(iWin);
    if (window)
    {
      CGUIControl *control = window->GetFocusedControl();
      if (control)
      {
        if (control->GetControlType() == CGUIControl::GUICONTROL_EDIT ||
            (control->IsContainer() && g_Keyboard.GetShift()))
          useKeyboard = true;
      }
    }
    if (useKeyboard)
    {
      if (key.GetFromHttpApi())
      {
        if (key.GetButtonCode() != KEY_INVALID)
          action.wID = (WORD) key.GetButtonCode();
        action.unicode = (WCHAR)key.GetUnicode();
      }
      else
      { // see if we've got an ascii key
        if (g_Keyboard.GetUnicode())
        {
          action.wID = (WORD)g_Keyboard.GetAscii() | KEY_ASCII; // Only for backwards compatibility
          action.unicode = g_Keyboard.GetUnicode();
        }
        else
        {
          action.wID = (WORD)g_Keyboard.GetKey() | KEY_VKEY;
          action.unicode = 0;
        }
      }
#ifdef HAS_SDL
      g_Keyboard.Reset();
#endif
      if (OnAction(action))
        return true;
      // failed to handle the keyboard action, drop down through to standard action
    }
    if (key.GetFromHttpApi())
    {
      if (key.GetButtonCode() != KEY_INVALID)
      {
        action.wID = (WORD) key.GetButtonCode();
        CButtonTranslator::GetInstance().GetAction(iWin, key, action);
      }
    }
    else
      CButtonTranslator::GetInstance().GetAction(iWin, key, action);
  }
  if (!key.IsAnalogButton())
    CLog::Log(LOGDEBUG, "%s: %i pressed, action is %i", __FUNCTION__, (int) key.GetButtonCode(), action.wID);

  //  Play a sound based on the action
  g_audioManager.PlayActionSound(action);

  return OnAction(action);
}

bool CApplication::OnAction(CAction &action)
{
  // Let's tell the outside world about this action
  if (m_pXbmcHttp && g_stSettings.m_HttpApiBroadcastLevel>=2)
  {
    CStdString tmp;
    tmp.Format("%i",action.wID);
    g_applicationMessenger.HttpApi("broadcastlevel; OnAction:"+tmp+";2");
  }

  if (action.wID == m_lastActionCode)
    action.holdTime = (unsigned int)m_lastActionTimer.GetElapsedMilliseconds();
  else
    m_lastActionTimer.StartZero();
  m_lastActionCode = action.wID;

  // special case for switching between GUI & fullscreen mode.
  if (action.wID == ACTION_SHOW_GUI)
  { // Switch to fullscreen mode if we can
    if (SwitchToFullScreen())
    {
      m_navigationTimer.StartZero();
      return true;
    }
  }

  // in normal case
  // just pass the action to the current window and let it handle it
  if (m_gWindowManager.OnAction(action))
  {
    m_navigationTimer.StartZero();
    return true;
  }

  // handle extra global presses

  // screenshot : take a screenshot :)
  if (action.wID == ACTION_TAKE_SCREENSHOT)
  {
    CUtil::TakeScreenshot();
    return true;
  }
  // built in functions : execute the built-in
  if (action.wID == ACTION_BUILT_IN_FUNCTION)
  {
    CUtil::ExecBuiltIn(action.strAction);
    m_navigationTimer.StartZero();
    return true;
  }

  // power down : turn off after 3 seconds of button down
  static bool PowerButtonDown = false;
  static DWORD PowerButtonCode;
  static DWORD MarkTime;
  if (action.wID == ACTION_POWERDOWN)
  {
    // Hold button for 3 secs to power down
    if (!PowerButtonDown)
    {
      MarkTime = GetTickCount();
      PowerButtonDown = true;
      PowerButtonCode = action.m_dwButtonCode;
    }
  }
  if (PowerButtonDown)
  {
    if (g_application.IsButtonDown(PowerButtonCode))
    {
      if (GetTickCount() >= MarkTime + 3000)
      {
        g_applicationMessenger.Shutdown();
        return true;
      }
    }
    else
      PowerButtonDown = false;
  }
  // show info : Shows the current video or song information
  if (action.wID == ACTION_SHOW_INFO)
  {
    g_infoManager.ToggleShowInfo();
    return true;
  }

  // codec info : Shows the current song, video or picture codec information
  if (action.wID == ACTION_SHOW_CODEC)
  {
    g_infoManager.ToggleShowCodec();
    return true;
  }

  if ((action.wID == ACTION_INCREASE_RATING || action.wID == ACTION_DECREASE_RATING) && IsPlayingAudio())
  {
    const CMusicInfoTag *tag = g_infoManager.GetCurrentSongTag();
    if (tag)
    {
      *m_itemCurrentFile->GetMusicInfoTag() = *tag;
      char rating = tag->GetRating();
      bool needsUpdate(false);
      if (rating > '0' && action.wID == ACTION_DECREASE_RATING)
      {
        m_itemCurrentFile->GetMusicInfoTag()->SetRating(rating - 1);
        needsUpdate = true;
      }
      else if (rating < '5' && action.wID == ACTION_INCREASE_RATING)
      {
        m_itemCurrentFile->GetMusicInfoTag()->SetRating(rating + 1);
        needsUpdate = true;
      }
      if (needsUpdate)
      {
        CMusicDatabase db;
        if (db.Open())      // OpenForWrite() ?
        {
          db.SetSongRating(m_itemCurrentFile->m_strPath, m_itemCurrentFile->GetMusicInfoTag()->GetRating());
          db.Close();
        }
        // send a message to all windows to tell them to update the fileitem (eg playlistplayer, media windows)
        CGUIMessage msg(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_UPDATE_ITEM, 0, m_itemCurrentFile);
        g_graphicsContext.SendMessage(msg);
      }
    }
    return true;
  }

  // stop : stops playing current audio song
  if (action.wID == ACTION_STOP)
  {
    StopPlaying();
    return true;
  }

  // previous : play previous song from playlist
  if (action.wID == ACTION_PREV_ITEM)
  {
    // first check whether we're within 3 seconds of the start of the track
    // if not, we just revert to the start of the track
    if (m_pPlayer && m_pPlayer->CanSeek() && GetTime() > 3)
    {
      SeekTime(0);
      SetPlaySpeed(1);
    }
    else
    {
      g_playlistPlayer.PlayPrevious();
    }
    return true;
  }

  // next : play next song from playlist
  if (action.wID == ACTION_NEXT_ITEM)
  {
    if (IsPlaying() && m_pPlayer->SkipNext())
      return true;

    g_playlistPlayer.PlayNext();

    return true;
  }

  if ( IsPlaying())
  {
    // OSD toggling
    if (action.wID == ACTION_SHOW_OSD)
    {
      if (IsPlayingVideo() && IsPlayingFullScreenVideo())
      {
        CGUIWindowOSD *pOSD = (CGUIWindowOSD *)m_gWindowManager.GetWindow(WINDOW_OSD);
        if (pOSD)
        {
          if (pOSD->IsDialogRunning())
            pOSD->Close();
          else
            pOSD->DoModal();
          return true;
        }
      }
    }

    // pause : pauses current audio song
    if (action.wID == ACTION_PAUSE)
    {
      m_pPlayer->Pause();
      if (!m_pPlayer->IsPaused())
      { // unpaused - set the playspeed back to normal
        SetPlaySpeed(1);
      }
      if (!g_guiSettings.GetBool("lookandfeel.soundsduringplayback"))
        g_audioManager.Enable(m_pPlayer->IsPaused());
      return true;
    }
    if (!m_pPlayer->IsPaused())
    {
      // if we do a FF/RW in my music then map PLAY action togo back to normal speed
      // if we are playing at normal speed, then allow play to pause
      if (action.wID == ACTION_PLAYER_PLAY)
      {
        if (m_iPlaySpeed != 1)
        {
          SetPlaySpeed(1);
        }
        else
        {
          m_pPlayer->Pause();
        }
        return true;
      }
      if (action.wID == ACTION_PLAYER_FORWARD || action.wID == ACTION_PLAYER_REWIND)
      {
        int iPlaySpeed = m_iPlaySpeed;
        if (action.wID == ACTION_PLAYER_REWIND && iPlaySpeed == 1) // Enables Rewinding
          iPlaySpeed *= -2;
        else if (action.wID == ACTION_PLAYER_REWIND && iPlaySpeed > 1) //goes down a notch if you're FFing
          iPlaySpeed /= 2;
        else if (action.wID == ACTION_PLAYER_FORWARD && iPlaySpeed < 1) //goes up a notch if you're RWing
          iPlaySpeed /= 2;
        else
          iPlaySpeed *= 2;

        if (action.wID == ACTION_PLAYER_FORWARD && iPlaySpeed == -1) //sets iSpeed back to 1 if -1 (didn't plan for a -1)
          iPlaySpeed = 1;
        if (iPlaySpeed > 32 || iPlaySpeed < -32)
          iPlaySpeed = 1;

        SetPlaySpeed(iPlaySpeed);
        return true;
      }
      else if ((action.fAmount1 || GetPlaySpeed() != 1) && (action.wID == ACTION_ANALOG_REWIND || action.wID == ACTION_ANALOG_FORWARD))
      {
        // calculate the speed based on the amount the button is held down
        int iPower = (int)(action.fAmount1 * MAX_FFWD_SPEED + 0.5f);
        // returns 0 -> MAX_FFWD_SPEED
        int iSpeed = 1 << iPower;
        if (iSpeed != 1 && action.wID == ACTION_ANALOG_REWIND)
          iSpeed = -iSpeed;
        g_application.SetPlaySpeed(iSpeed);
        if (iSpeed == 1)
          CLog::Log(LOGDEBUG,"Resetting playspeed");
        return true;
      }
    }
    // allow play to unpause
    else
    {
      if (action.wID == ACTION_PLAYER_PLAY)
      {
        // unpause, and set the playspeed back to normal
        m_pPlayer->Pause();
        if (!g_guiSettings.GetBool("lookandfeel.soundsduringplayback"))
          g_audioManager.Enable(m_pPlayer->IsPaused());

        g_application.SetPlaySpeed(1);
        return true;
      }
    }
  }
  if (action.wID == ACTION_MUTE)
  {
    Mute();
    return true;
  }
 
  if (action.wID == ACTION_TOGGLE_DIGITAL_ANALOG)
  { 
    if(g_guiSettings.GetInt("audiooutput.mode")==AUDIO_DIGITAL)
      g_guiSettings.SetInt("audiooutput.mode", AUDIO_ANALOG);
    else
      g_guiSettings.SetInt("audiooutput.mode", AUDIO_DIGITAL);
    g_application.Restart();
    if (m_gWindowManager.GetActiveWindow() == WINDOW_SETTINGS_SYSTEM)
    {
      CGUIMessage msg(GUI_MSG_WINDOW_INIT, 0,0,WINDOW_INVALID,m_gWindowManager.GetActiveWindow());
      m_gWindowManager.SendMessage(msg);
    }
    return true;
  }

  // Check for global volume control
  if (action.fAmount1 && (action.wID == ACTION_VOLUME_UP || action.wID == ACTION_VOLUME_DOWN))
  {
    // increase or decrease the volume
    int volume = g_stSettings.m_nVolumeLevel + g_stSettings.m_dynamicRangeCompressionLevel;

    // calculate speed so that a full press will equal 1 second from min to max
    float speed = float(VOLUME_MAXIMUM - VOLUME_MINIMUM);
    if( action.fRepeat )
      speed *= action.fRepeat;
    else
      speed /= 50; //50 fps
    if (g_stSettings.m_bMute)
    {
      // only unmute if volume is to be increased, otherwise leave muted
      if (action.wID == ACTION_VOLUME_DOWN)
        return true;
      Mute();
      return true;
    }
    if (action.wID == ACTION_VOLUME_UP)
    {
      volume += (int)((float)fabs(action.fAmount1) * action.fAmount1 * speed);
    }
    else
    {
      volume -= (int)((float)fabs(action.fAmount1) * action.fAmount1 * speed);
    }

    SetHardwareVolume(volume);
#ifndef HAS_SDL_AUDIO
    g_audioManager.SetVolume(g_stSettings.m_nVolumeLevel);
#else
    g_audioManager.SetVolume((int)(128.f * (g_stSettings.m_nVolumeLevel - VOLUME_MINIMUM) / (float)(VOLUME_MAXIMUM - VOLUME_MINIMUM)));
#endif

    // show visual feedback of volume change...
    m_guiDialogVolumeBar.Show();
    m_guiDialogVolumeBar.OnAction(action);
    return true;
  }
  // Check for global seek control
  if (IsPlaying() && action.fAmount1 && (action.wID == ACTION_ANALOG_SEEK_FORWARD || action.wID == ACTION_ANALOG_SEEK_BACK))
  {
    if (!m_pPlayer->CanSeek()) return false;
    m_guiDialogSeekBar.OnAction(action);
    return true;
  }
  return false;
}

void CApplication::UpdateLCD()
{
#ifdef HAS_LCD
  static lTickCount = 0;

  if (!g_lcd || g_guiSettings.GetInt("lcd.type") == LCD_TYPE_NONE)
    return ;
  long lTimeOut = 1000;
  if ( m_iPlaySpeed != 1)
    lTimeOut = 0;
  if ( ((long)GetTickCount() - lTickCount) >= lTimeOut)
  {
    if (g_application.NavigationIdleTime() < 5)
      g_lcd->Render(ILCD::LCD_MODE_NAVIGATION);
    else if (IsPlayingVideo())
      g_lcd->Render(ILCD::LCD_MODE_VIDEO);
    else if (IsPlayingAudio())
      g_lcd->Render(ILCD::LCD_MODE_MUSIC);
    else if (IsInScreenSaver())
      g_lcd->Render(ILCD::LCD_MODE_SCREENSAVER);
    else
      g_lcd->Render(ILCD::LCD_MODE_GENERAL);

    // reset tick count
    lTickCount = GetTickCount();
  }
#endif
}

void CApplication::FrameMove()
{
  // currently we calculate the repeat time (ie time from last similar keypress) just global as fps
  float frameTime = m_frameTime.GetElapsedSeconds();
  m_frameTime.StartZero();
  // never set a frametime less than 2 fps to avoid problems when debuggin and on breaks
  if( frameTime > 0.5 ) frameTime = 0.5;

  // check if there are notifications to display
  if (m_guiDialogKaiToast.DoWork())
  {
    if (!m_guiDialogKaiToast.IsDialogRunning())
    {
      m_guiDialogKaiToast.Show();
    }
  }

  UpdateLCD();

  // read raw input from controller, remote control, mouse and keyboard
  ReadInput();
  // process input actions
  bool didSomething = ProcessMouse();
  didSomething |= ProcessHTTPApiButtons();
  didSomething |= ProcessKeyboard();
  didSomething |= ProcessRemote(frameTime);
  didSomething |= ProcessGamepad(frameTime);
  didSomething |= ProcessEventServer(frameTime);
  // reset our previous action code
  if (!didSomething)
    m_lastActionCode = 0;
}

bool CApplication::ProcessGamepad(float frameTime)
{
#ifdef HAS_GAMEPAD
  // Handle the gamepad button presses.  We check for button down,
  // then call OnKey() which handles the translation to actions, and sends the
  // action to our window manager's OnAction() function, which filters the messages
  // to where they're supposed to end up, returning true if the message is successfully
  // processed.  If OnKey() returns false, then the key press wasn't processed at all,
  // and we can safely process the next key (or next check on the same key in the
  // case of the analog sticks which can produce more than 1 key event.)

  WORD wButtons = m_DefaultGamepad.wButtons;
  WORD wDpad = wButtons & (XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_LEFT | XINPUT_GAMEPAD_DPAD_RIGHT);

  BYTE bLeftTrigger = m_DefaultGamepad.bAnalogButtons[XINPUT_GAMEPAD_LEFT_TRIGGER];
  BYTE bRightTrigger = m_DefaultGamepad.bAnalogButtons[XINPUT_GAMEPAD_RIGHT_TRIGGER];
  BYTE bButtonA = m_DefaultGamepad.bAnalogButtons[XINPUT_GAMEPAD_A];
  BYTE bButtonB = m_DefaultGamepad.bAnalogButtons[XINPUT_GAMEPAD_B];
  BYTE bButtonX = m_DefaultGamepad.bAnalogButtons[XINPUT_GAMEPAD_X];
  BYTE bButtonY = m_DefaultGamepad.bAnalogButtons[XINPUT_GAMEPAD_Y];

  // pass them through the delay
  WORD wDir = m_ctrDpad.DpadInput(wDpad, 0 != bLeftTrigger, 0 != bRightTrigger);

  // map all controller & remote actions to their keys
  if (m_DefaultGamepad.fX1 || m_DefaultGamepad.fY1)
  {
    CKey key(KEY_BUTTON_LEFT_THUMB_STICK, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  if (m_DefaultGamepad.fX2 || m_DefaultGamepad.fY2)
  {
    CKey key(KEY_BUTTON_RIGHT_THUMB_STICK, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  // direction specific keys (for defining different actions for each direction)
  // We need to be able to know when it last had a direction, so that we can
  // post the reset direction code the next time around (to reset scrolling,
  // fastforwarding and other analog actions)

  // For the sticks, once it is pushed in one direction (eg up) it will only
  // detect movement in that direction of movement (eg up or down) - the other
  // direction (eg left and right) will not be registered until the stick has
  // been recentered for at least 2 frames.

  // first the right stick
  static lastRightStickKey = 0;
  int newRightStickKey = 0;
  if (lastRightStickKey == KEY_BUTTON_RIGHT_THUMB_STICK_UP || lastRightStickKey == KEY_BUTTON_RIGHT_THUMB_STICK_DOWN)
  {
    if (m_DefaultGamepad.fY2 > 0)
      newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_UP;
    else if (m_DefaultGamepad.fY2 < 0)
      newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_DOWN;
    else if (m_DefaultGamepad.fX2 != 0)
    {
      newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_UP;
      //m_DefaultGamepad.fY2 = 0.00001f; // small amount of movement
    }
  }
  else if (lastRightStickKey == KEY_BUTTON_RIGHT_THUMB_STICK_LEFT || lastRightStickKey == KEY_BUTTON_RIGHT_THUMB_STICK_RIGHT)
  {
    if (m_DefaultGamepad.fX2 > 0)
      newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_RIGHT;
    else if (m_DefaultGamepad.fX2 < 0)
      newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_LEFT;
    else if (m_DefaultGamepad.fY2 != 0)
    {
      newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_RIGHT;
      //m_DefaultGamepad.fX2 = 0.00001f; // small amount of movement
    }
  }
  else
  {
    if (m_DefaultGamepad.fY2 > 0 && m_DefaultGamepad.fX2*2 < m_DefaultGamepad.fY2 && -m_DefaultGamepad.fX2*2 < m_DefaultGamepad.fY2)
      newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_UP;
    else if (m_DefaultGamepad.fY2 < 0 && m_DefaultGamepad.fX2*2 < -m_DefaultGamepad.fY2 && -m_DefaultGamepad.fX2*2 < -m_DefaultGamepad.fY2)
      newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_DOWN;
    else if (m_DefaultGamepad.fX2 > 0 && m_DefaultGamepad.fY2*2 < m_DefaultGamepad.fX2 && -m_DefaultGamepad.fY2*2 < m_DefaultGamepad.fX2)
      newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_RIGHT;
    else if (m_DefaultGamepad.fX2 < 0 && m_DefaultGamepad.fY2*2 < -m_DefaultGamepad.fX2 && -m_DefaultGamepad.fY2*2 < -m_DefaultGamepad.fX2)
      newRightStickKey = KEY_BUTTON_RIGHT_THUMB_STICK_LEFT;
  }
  if (lastRightStickKey && newRightStickKey != lastRightStickKey)
  { // was held down last time - and we have a new key now
    // post old key reset message...
    CKey key(lastRightStickKey, 0, 0, 0, 0, 0, 0);
    lastRightStickKey = newRightStickKey;
    if (OnKey(key)) return true;
  }
  lastRightStickKey = newRightStickKey;
  // post the new key's message
  if (newRightStickKey)
  {
    CKey key(newRightStickKey, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }

  // now the left stick
  static lastLeftStickKey = 0;
  int newLeftStickKey = 0;
  if (lastLeftStickKey == KEY_BUTTON_LEFT_THUMB_STICK_UP || lastLeftStickKey == KEY_BUTTON_LEFT_THUMB_STICK_DOWN)
  {
    if (m_DefaultGamepad.fY1 > 0)
      newLeftStickKey = KEY_BUTTON_LEFT_THUMB_STICK_UP;
    else if (m_DefaultGamepad.fY1 < 0)
      newLeftStickKey = KEY_BUTTON_LEFT_THUMB_STICK_DOWN;
  }
  else if (lastLeftStickKey == KEY_BUTTON_LEFT_THUMB_STICK_LEFT || lastLeftStickKey == KEY_BUTTON_LEFT_THUMB_STICK_RIGHT)
  {
    if (m_DefaultGamepad.fX1 > 0)
      newLeftStickKey = KEY_BUTTON_LEFT_THUMB_STICK_RIGHT;
    else if (m_DefaultGamepad.fX1 < 0)
      newLeftStickKey = KEY_BUTTON_LEFT_THUMB_STICK_LEFT;
  }
  else
  { // check for a new control movement
    if (m_DefaultGamepad.fY1 > 0 && m_DefaultGamepad.fX1 < m_DefaultGamepad.fY1 && -m_DefaultGamepad.fX1 < m_DefaultGamepad.fY1)
      newLeftStickKey = KEY_BUTTON_LEFT_THUMB_STICK_UP;
    else if (m_DefaultGamepad.fY1 < 0 && m_DefaultGamepad.fX1 < -m_DefaultGamepad.fY1 && -m_DefaultGamepad.fX1 < -m_DefaultGamepad.fY1)
      newLeftStickKey = KEY_BUTTON_LEFT_THUMB_STICK_DOWN;
    else if (m_DefaultGamepad.fX1 > 0 && m_DefaultGamepad.fY1 < m_DefaultGamepad.fX1 && -m_DefaultGamepad.fY1 < m_DefaultGamepad.fX1)
      newLeftStickKey = KEY_BUTTON_LEFT_THUMB_STICK_RIGHT;
    else if (m_DefaultGamepad.fX1 < 0 && m_DefaultGamepad.fY1 < -m_DefaultGamepad.fX1 && -m_DefaultGamepad.fY1 < -m_DefaultGamepad.fX1)
      newLeftStickKey = KEY_BUTTON_LEFT_THUMB_STICK_LEFT;
  }

  if (lastLeftStickKey && newLeftStickKey != lastLeftStickKey)
  { // was held down last time - and we have a new key now
    // post old key reset message...
    CKey key(lastLeftStickKey, 0, 0, 0, 0, 0, 0);
    lastLeftStickKey = newLeftStickKey;
    if (OnKey(key)) return true;
  }
  lastLeftStickKey = newLeftStickKey;
  // post the new key's message
  if (newLeftStickKey)
  {
    CKey key(newLeftStickKey, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }

  // Trigger detection
  static lastTriggerKey = 0;
  int newTriggerKey = 0;
  if (bLeftTrigger)
    newTriggerKey = KEY_BUTTON_LEFT_ANALOG_TRIGGER;
  else if (bRightTrigger)
    newTriggerKey = KEY_BUTTON_RIGHT_ANALOG_TRIGGER;
  if (lastTriggerKey && newTriggerKey != lastTriggerKey)
  { // was held down last time - and we have a new key now
    // post old key reset message...
    CKey key(lastTriggerKey, 0, 0, 0, 0, 0, 0);
    lastTriggerKey = newTriggerKey;
    if (OnKey(key)) return true;
  }
  lastTriggerKey = newTriggerKey;
  // post the new key's message
  if (newTriggerKey)
  {
    CKey key(newTriggerKey, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }

  // Now the digital buttons...
  if ( wDir & DC_LEFTTRIGGER)
  {
    CKey key(KEY_BUTTON_LEFT_TRIGGER, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  if ( wDir & DC_RIGHTTRIGGER)
  {
    CKey key(KEY_BUTTON_RIGHT_TRIGGER, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  if ( wDir & DC_LEFT )
  {
    CKey key(KEY_BUTTON_DPAD_LEFT, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  if ( wDir & DC_RIGHT)
  {
    CKey key(KEY_BUTTON_DPAD_RIGHT, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  if ( wDir & DC_UP )
  {
    CKey key(KEY_BUTTON_DPAD_UP, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  if ( wDir & DC_DOWN )
  {
    CKey key(KEY_BUTTON_DPAD_DOWN, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }

  if (m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_BACK )
  {
    CKey key(KEY_BUTTON_BACK, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  if (m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_START)
  {
    CKey key(KEY_BUTTON_START, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  if (m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_LEFT_THUMB)
  {
    CKey key(KEY_BUTTON_LEFT_THUMB_BUTTON, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  if (m_DefaultGamepad.wPressedButtons & XINPUT_GAMEPAD_RIGHT_THUMB)
  {
    CKey key(KEY_BUTTON_RIGHT_THUMB_BUTTON, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  if (m_DefaultGamepad.bPressedAnalogButtons[XINPUT_GAMEPAD_A])
  {
    CKey key(KEY_BUTTON_A, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  if (m_DefaultGamepad.bPressedAnalogButtons[XINPUT_GAMEPAD_B])
  {
    CKey key(KEY_BUTTON_B, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }

  if (m_DefaultGamepad.bPressedAnalogButtons[XINPUT_GAMEPAD_X])
  {
    CKey key(KEY_BUTTON_X, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  if (m_DefaultGamepad.bPressedAnalogButtons[XINPUT_GAMEPAD_Y])
  {
    CKey key(KEY_BUTTON_Y, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  if (m_DefaultGamepad.bPressedAnalogButtons[XINPUT_GAMEPAD_BLACK])
  {
    CKey key(KEY_BUTTON_BLACK, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
  if (m_DefaultGamepad.bPressedAnalogButtons[XINPUT_GAMEPAD_WHITE])
  {
    CKey key(KEY_BUTTON_WHITE, bLeftTrigger, bRightTrigger, m_DefaultGamepad.fX1, m_DefaultGamepad.fY1, m_DefaultGamepad.fX2, m_DefaultGamepad.fY2, frameTime);
    if (OnKey(key)) return true;
  }
#endif
  return false;
}

bool CApplication::ProcessRemote(float frameTime)
{
#ifdef HAS_IR_REMOTE
  if (m_DefaultIR_Remote.wButtons)
  {
    // time depends on whether the movement is repeated (held down) or not.
    // If it is, we use the FPS timer to get a repeatable speed.
    // If it isn't, we use 20 to get repeatable jumps.
    float time = (m_DefaultIR_Remote.bHeldDown) ? frameTime : 0.020f;
    CKey key(m_DefaultIR_Remote.wButtons, 0, 0, 0, 0, 0, 0, time);
    return OnKey(key);
  }
#endif
  return false;
}

bool CApplication::ProcessMouse()
{
  if (!g_Mouse.IsActive())
    return false;
  // Reset the screensaver and idle timers
  m_idleTimer.StartZero();
  ResetScreenSaver();
  if (ResetScreenSaverWindow())
    return true;

  // call OnAction with ACTION_MOUSE
  CAction action;
  action.wID = ACTION_MOUSE;
  action.fAmount1 = (float) m_guiPointer.GetXPosition();
  action.fAmount2 = (float) m_guiPointer.GetYPosition();
  return m_gWindowManager.OnAction(action);
}

void  CApplication::CheckForTitleChange()
{ 
  if (g_stSettings.m_HttpApiBroadcastLevel>=1)
  {
    if (IsPlayingVideo())
    {
      const CVideoInfoTag* tagVal = g_infoManager.GetCurrentMovieTag();
      if (m_pXbmcHttp && tagVal && !(tagVal->m_strTitle.IsEmpty()))
      {
        CStdString msg=m_pXbmcHttp->GetOpenTag()+"MovieTitle:"+tagVal->m_strTitle+m_pXbmcHttp->GetCloseTag();
        if (m_prevMedia!=msg && g_stSettings.m_HttpApiBroadcastLevel>=1)
        {
          g_applicationMessenger.HttpApi("broadcastlevel; MediaChanged:"+msg+";1");
          m_prevMedia=msg;
        }
      }
    }
    else if (IsPlayingAudio())
    {
      const CMusicInfoTag* tagVal=g_infoManager.GetCurrentSongTag();
      if (m_pXbmcHttp && tagVal)
      {
        CStdString msg="";
        if (!tagVal->GetTitle().IsEmpty())
          msg=m_pXbmcHttp->GetOpenTag()+"AudioTitle:"+tagVal->GetTitle()+m_pXbmcHttp->GetCloseTag();
        if (!tagVal->GetArtist().IsEmpty())
          msg+=m_pXbmcHttp->GetOpenTag()+"AudioArtist:"+tagVal->GetArtist()+m_pXbmcHttp->GetCloseTag();
        if (m_prevMedia!=msg)
        {
          g_applicationMessenger.HttpApi("broadcastlevel; MediaChanged:"+msg+";1");
          m_prevMedia=msg;
        }
      }
    }
  }
}

bool CApplication::ProcessHTTPApiButtons()
{
  if (m_pXbmcHttp)
  {
    // copy key from webserver, and reset it in case we're called again before
    // whatever happens in OnKey()
    CKey keyHttp(m_pXbmcHttp->GetKey());
    m_pXbmcHttp->ResetKey();
    if (keyHttp.GetButtonCode() != KEY_INVALID)
    {
      if (keyHttp.GetButtonCode() == KEY_VMOUSE) //virtual mouse
      {
        CAction action;
        action.wID = ACTION_MOUSE;
        g_Mouse.SetLocation(CPoint(keyHttp.GetLeftThumbX(), keyHttp.GetLeftThumbY()));
        if (keyHttp.GetLeftTrigger()!=0)
          g_Mouse.bClick[keyHttp.GetLeftTrigger()-1]=true;
        if (keyHttp.GetRightTrigger()!=0)
          g_Mouse.bDoubleClick[keyHttp.GetRightTrigger()-1]=true;
        action.fAmount1 = keyHttp.GetLeftThumbX();
        action.fAmount2 = keyHttp.GetLeftThumbY();
        m_gWindowManager.OnAction(action);
      }
      else
        OnKey(keyHttp);
      return true;
    }
  }
  return false;
}

bool CApplication::ProcessEventServer(float frameTime)
{
#ifdef HAS_EVENT_SERVER
  CEventServer* es = CEventServer::GetInstance();
  if (!es || !es->Running() || es->GetNumberOfClients()==0)
    return false;

  // process any queued up actions
  if (es->ExecuteNextAction())
  {
    // reset idle timers
    m_idleTimer.StartZero();
    ResetScreenSaver();
    ResetScreenSaverWindow();
  }

  // now handle any buttons or axis
  std::string joystickName;
  bool isAxis = false;
  float fAmount = 0.0;

  WORD wKeyID = es->GetButtonCode(joystickName, isAxis, fAmount);

  if (wKeyID)
  {
    if (joystickName.length() > 0)
    {
      if (isAxis == true)
      {
        if (fabs(fAmount) >= 0.08)
          m_lastAxisMap[joystickName][wKeyID] = fAmount;
        else
          m_lastAxisMap[joystickName].erase(wKeyID);
      }

      return ProcessJoystickEvent(joystickName, wKeyID, isAxis, fAmount);
    }
    else
    {
      CKey key;
      if(wKeyID == KEY_BUTTON_LEFT_ANALOG_TRIGGER)
        key = CKey(wKeyID, (BYTE)(255*fAmount), 0, 0.0, 0.0, 0.0, 0.0, frameTime);
      else if(wKeyID == KEY_BUTTON_RIGHT_ANALOG_TRIGGER)
        key = CKey(wKeyID, 0, (BYTE)(255*fAmount), 0.0, 0.0, 0.0, 0.0, frameTime);
      else if(wKeyID == KEY_BUTTON_LEFT_THUMB_STICK_LEFT)
        key = CKey(wKeyID, 0, 0, -fAmount, 0.0, 0.0, 0.0, frameTime);
      else if(wKeyID == KEY_BUTTON_LEFT_THUMB_STICK_RIGHT)
        key = CKey(wKeyID, 0, 0,  fAmount, 0.0, 0.0, 0.0, frameTime);
      else if(wKeyID == KEY_BUTTON_LEFT_THUMB_STICK_UP)
        key = CKey(wKeyID, 0, 0, 0.0,  fAmount, 0.0, 0.0, frameTime);
      else if(wKeyID == KEY_BUTTON_LEFT_THUMB_STICK_DOWN)
        key = CKey(wKeyID, 0, 0, 0.0, -fAmount, 0.0, 0.0, frameTime);
      else if(wKeyID == KEY_BUTTON_RIGHT_THUMB_STICK_LEFT)
        key = CKey(wKeyID, 0, 0, 0.0, 0.0, -fAmount, 0.0, frameTime);
      else if(wKeyID == KEY_BUTTON_RIGHT_THUMB_STICK_RIGHT)
        key = CKey(wKeyID, 0, 0, 0.0, 0.0,  fAmount, 0.0, frameTime);
      else if(wKeyID == KEY_BUTTON_RIGHT_THUMB_STICK_UP)
        key = CKey(wKeyID, 0, 0, 0.0, 0.0, 0.0,  fAmount, frameTime);
      else if(wKeyID == KEY_BUTTON_RIGHT_THUMB_STICK_DOWN)
        key = CKey(wKeyID, 0, 0, 0.0, 0.0, 0.0, -fAmount, frameTime);
      else
        key = CKey(wKeyID);
      return OnKey(key);
    }
  }

  if (m_lastAxisMap.size() > 0)
  {
    // Process all the stored axis.
    for (map<std::string, map<int, float> >::iterator iter = m_lastAxisMap.begin(); iter != m_lastAxisMap.end(); ++iter)
    {
      for (map<int, float>::iterator iterAxis = (*iter).second.begin(); iterAxis != (*iter).second.end(); ++iterAxis)
        ProcessJoystickEvent((*iter).first, (*iterAxis).first, true, (*iterAxis).second);
    }
  }

  {
    CAction action;
    action.wID = ACTION_MOUSE;
    if (es->GetMousePos(action.fAmount1, action.fAmount2) && g_Mouse.IsEnabled())
    {
      CPoint point;
      point.x = action.fAmount1;
      point.y = action.fAmount2;
      g_Mouse.SetLocation(point, true);

      return m_gWindowManager.OnAction(action);
    }
  }
#endif
  return false;
}

bool CApplication::ProcessJoystickEvent(const std::string& joystickName, int wKeyID, bool isAxis, float fAmount)
{
#ifdef HAS_EVENT_SERVER
  m_idleTimer.StartZero();

   // Make sure to reset screen saver, mouse.
   ResetScreenSaver();
   if (ResetScreenSaverWindow())
     return true;

#ifdef HAS_SDL_JOYSTICK
   g_Joystick.Reset();
#endif
   g_Mouse.SetInactive();

   // Figure out what window we're taking the event for.
   WORD iWin = m_gWindowManager.GetActiveWindow() & WINDOW_ID_MASK;
   if (m_gWindowManager.HasModalDialog())
       iWin = m_gWindowManager.GetTopMostModalDialogID() & WINDOW_ID_MASK;

   // This code is copied from the OnKey handler, it should be factored out.
   if (iWin == WINDOW_FULLSCREEN_VIDEO &&
       g_application.m_pPlayer &&
       g_application.m_pPlayer->IsInMenu())
   {
     // If player is in some sort of menu, (ie DVDMENU) map buttons differently.
     iWin = WINDOW_VIDEO_MENU;
   }

   bool fullRange = false;
   CAction action;
   action.fAmount1 = fAmount;

   //if (action.fAmount1 < 0.0)
   // wKeyID = -wKeyID;

   // Translate using regular joystick translator.
   if (CButtonTranslator::GetInstance().TranslateJoystickString(iWin, joystickName.c_str(), wKeyID, isAxis, action.wID, action.strAction, fullRange))
   {
     action.fRepeat = 0.0f;
     g_audioManager.PlayActionSound(action);
     return OnAction(action);
   }
   else
   {
     CLog::Log(LOGDEBUG, "ERROR mapping joystick action");
   }
#endif

   return false;
}

bool CApplication::ProcessKeyboard()
{
  // process the keyboard buttons etc.
  BYTE vkey = g_Keyboard.GetKey();
  WCHAR unicode = g_Keyboard.GetUnicode();
  if (vkey || unicode)
  {
    // got a valid keypress - convert to a key code
    WORD wkeyID;
    if (vkey) // FIXME, every ascii has a vkey so vkey would always and ascii would never be processed, but fortunately OnKey uses wkeyID only to detect keyboard use and the real key is recalculated correctly.
      wkeyID = (WORD)vkey | KEY_VKEY;
    else
      wkeyID = KEY_UNICODE;
    //  CLog::Log(LOGDEBUG,"Keyboard: time=%i key=%i", timeGetTime(), vkey);
    CKey key(wkeyID);
    key.SetHeld(g_Keyboard.KeyHeld());
    return OnKey(key);
  }
  return false;
}

bool CApplication::IsButtonDown(DWORD code)
{
#ifdef HAS_GAMEPAD
  if (code >= KEY_BUTTON_A && code <= KEY_BUTTON_RIGHT_TRIGGER)
  {
    // analogue
    return (m_DefaultGamepad.bAnalogButtons[code - KEY_BUTTON_A + XINPUT_GAMEPAD_A] > XINPUT_GAMEPAD_MAX_CROSSTALK);
  }
  else if (code >= KEY_BUTTON_DPAD_UP && code <= KEY_BUTTON_RIGHT_THUMB_BUTTON)
  {
    // digital
    return (m_DefaultGamepad.wButtons & (1 << (code - KEY_BUTTON_DPAD_UP))) != 0;
  }
  else
  {
    // remote
    return m_DefaultIR_Remote.wButtons == code;
  }
#endif
  return false;
}

bool CApplication::AnyButtonDown()
{
  ReadInput();
#ifdef HAS_GAMEPAD
  if (m_DefaultGamepad.wPressedButtons || m_DefaultIR_Remote.wButtons)
    return true;

  for (int i = 0; i < 6; ++i)
  {
    if (m_DefaultGamepad.bPressedAnalogButtons[i])
      return true;
  }
#endif
  return false;
}

HRESULT CApplication::Cleanup()
{
  try
  {
    m_gWindowManager.Delete(WINDOW_MUSIC_PLAYLIST);
    m_gWindowManager.Delete(WINDOW_MUSIC_PLAYLIST_EDITOR);
    m_gWindowManager.Delete(WINDOW_MUSIC_FILES);
    m_gWindowManager.Delete(WINDOW_MUSIC_NAV);
    m_gWindowManager.Delete(WINDOW_MUSIC_INFO);
    m_gWindowManager.Delete(WINDOW_VIDEO_INFO);
    m_gWindowManager.Delete(WINDOW_VIDEO_FILES);
    m_gWindowManager.Delete(WINDOW_VIDEO_PLAYLIST);
    m_gWindowManager.Delete(WINDOW_VIDEO_NAV);
    m_gWindowManager.Delete(WINDOW_FILES);
    m_gWindowManager.Delete(WINDOW_MUSIC_INFO);
    m_gWindowManager.Delete(WINDOW_VIDEO_INFO);
    m_gWindowManager.Delete(WINDOW_DIALOG_YES_NO);
    m_gWindowManager.Delete(WINDOW_DIALOG_PROGRESS);
    m_gWindowManager.Delete(WINDOW_DIALOG_NUMERIC);
    m_gWindowManager.Delete(WINDOW_DIALOG_GAMEPAD);
    m_gWindowManager.Delete(WINDOW_DIALOG_SUB_MENU);
    m_gWindowManager.Delete(WINDOW_DIALOG_BUTTON_MENU);
    m_gWindowManager.Delete(WINDOW_DIALOG_CONTEXT_MENU);
    m_gWindowManager.Delete(WINDOW_DIALOG_MUSIC_SCAN);
    m_gWindowManager.Delete(WINDOW_DIALOG_PLAYER_CONTROLS);
    m_gWindowManager.Delete(WINDOW_DIALOG_MUSIC_OSD);
    m_gWindowManager.Delete(WINDOW_DIALOG_VIS_SETTINGS);
    m_gWindowManager.Delete(WINDOW_DIALOG_VIS_PRESET_LIST);
    m_gWindowManager.Delete(WINDOW_DIALOG_SELECT);
    m_gWindowManager.Delete(WINDOW_DIALOG_OK);
    m_gWindowManager.Delete(WINDOW_DIALOG_FILESTACKING);
    m_gWindowManager.Delete(WINDOW_DIALOG_KEYBOARD);
    m_gWindowManager.Delete(WINDOW_FULLSCREEN_VIDEO);
    m_gWindowManager.Delete(WINDOW_DIALOG_TRAINER_SETTINGS);
    m_gWindowManager.Delete(WINDOW_DIALOG_PROFILE_SETTINGS);
    m_gWindowManager.Delete(WINDOW_DIALOG_LOCK_SETTINGS);
    m_gWindowManager.Delete(WINDOW_DIALOG_NETWORK_SETUP);
    m_gWindowManager.Delete(WINDOW_DIALOG_MEDIA_SOURCE);
    m_gWindowManager.Delete(WINDOW_DIALOG_VIDEO_OSD_SETTINGS);
    m_gWindowManager.Delete(WINDOW_DIALOG_AUDIO_OSD_SETTINGS);
    m_gWindowManager.Delete(WINDOW_DIALOG_VIDEO_BOOKMARKS);
    m_gWindowManager.Delete(WINDOW_DIALOG_VIDEO_SCAN);
    m_gWindowManager.Delete(WINDOW_DIALOG_CONTENT_SETTINGS);
    m_gWindowManager.Delete(WINDOW_DIALOG_FAVOURITES);
    m_gWindowManager.Delete(WINDOW_DIALOG_SONG_INFO);
    m_gWindowManager.Delete(WINDOW_DIALOG_SMART_PLAYLIST_EDITOR);
    m_gWindowManager.Delete(WINDOW_DIALOG_SMART_PLAYLIST_RULE);
    m_gWindowManager.Delete(WINDOW_DIALOG_BUSY);
    m_gWindowManager.Delete(WINDOW_DIALOG_PICTURE_INFO);
    m_gWindowManager.Delete(WINDOW_DIALOG_PLUGIN_SETTINGS);
    m_gWindowManager.Delete(WINDOW_DIALOG_SLIDER);

    m_gWindowManager.Delete(WINDOW_STARTUP);
    m_gWindowManager.Delete(WINDOW_LOGIN_SCREEN);
    m_gWindowManager.Delete(WINDOW_VISUALISATION);
    m_gWindowManager.Delete(WINDOW_SETTINGS_MENU);
    m_gWindowManager.Delete(WINDOW_SETTINGS_PROFILES);
    m_gWindowManager.Delete(WINDOW_SETTINGS_MYPICTURES);  // all the settings categories
    m_gWindowManager.Delete(WINDOW_SCREEN_CALIBRATION);
    m_gWindowManager.Delete(WINDOW_SYSTEM_INFORMATION);
    m_gWindowManager.Delete(WINDOW_SCREENSAVER);
    m_gWindowManager.Delete(WINDOW_OSD);
    m_gWindowManager.Delete(WINDOW_MUSIC_OVERLAY);
    m_gWindowManager.Delete(WINDOW_VIDEO_OVERLAY);
    m_gWindowManager.Delete(WINDOW_SCRIPTS_INFO);
    m_gWindowManager.Delete(WINDOW_SLIDESHOW);

    m_gWindowManager.Delete(WINDOW_HOME);
    m_gWindowManager.Delete(WINDOW_PROGRAMS);
    m_gWindowManager.Delete(WINDOW_PICTURES);
    m_gWindowManager.Delete(WINDOW_SCRIPTS);
    m_gWindowManager.Delete(WINDOW_GAMESAVES);
    m_gWindowManager.Delete(WINDOW_WEATHER);

    m_gWindowManager.Delete(WINDOW_SETTINGS_MYPICTURES);
    m_gWindowManager.Remove(WINDOW_SETTINGS_MYPROGRAMS);
    m_gWindowManager.Remove(WINDOW_SETTINGS_MYWEATHER);
    m_gWindowManager.Remove(WINDOW_SETTINGS_MYMUSIC);
    m_gWindowManager.Remove(WINDOW_SETTINGS_SYSTEM);
    m_gWindowManager.Remove(WINDOW_SETTINGS_MYVIDEOS);
    m_gWindowManager.Remove(WINDOW_SETTINGS_NETWORK);
    m_gWindowManager.Remove(WINDOW_SETTINGS_APPEARANCE);
    m_gWindowManager.Remove(WINDOW_DIALOG_KAI_TOAST);

    m_gWindowManager.Remove(WINDOW_DIALOG_SEEK_BAR);
    m_gWindowManager.Remove(WINDOW_DIALOG_VOLUME_BAR);

    CLog::Log(LOGNOTICE, "unload sections");
    CSectionLoader::UnloadAll();
    // reset our d3d params before we destroy
    g_graphicsContext.SetD3DDevice(NULL);
    g_graphicsContext.SetD3DParameters(NULL);

#ifdef _DEBUG
    //  Shutdown as much as possible of the
    //  application, to reduce the leaks dumped
    //  to the vc output window before calling
    //  _CrtDumpMemoryLeaks(). Most of the leaks
    //  shown are no real leaks, as parts of the app
    //  are still allocated.

    g_localizeStrings.Clear();
    g_LangCodeExpander.Clear();
    g_charsetConverter.clear();
    g_directoryCache.Clear();
    CButtonTranslator::GetInstance().Clear();
    CLastfmScrobbler::RemoveInstance();
    CLibrefmScrobbler::RemoveInstance();
    CLastFmManager::RemoveInstance();
#ifdef HAS_EVENT_SERVER
    CEventServer::RemoveInstance();
#endif
    g_infoManager.Clear();
    DllLoaderContainer::Clear();
    g_playlistPlayer.Clear();
    g_settings.Clear();
    g_guiSettings.Clear();
#endif

#ifdef _CRTDBG_MAP_ALLOC
    _CrtDumpMemoryLeaks();
    while(1); // execution ends
#endif
    return S_OK;
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "Exception in CApplication::Cleanup()");
    return E_FAIL;
  }
}

void CApplication::Stop()
{
  try
  {
    if (m_pXbmcHttp)
    {
      if(g_stSettings.m_HttpApiBroadcastLevel>=1)
        g_applicationMessenger.HttpApi("broadcastlevel; ShutDown;1");

      m_pXbmcHttp->shuttingDown=true;
      //Sleep(100);
    }

    CLog::Log(LOGNOTICE, "Storing total System Uptime");
    g_stSettings.m_iSystemTimeTotalUp = g_stSettings.m_iSystemTimeTotalUp + (int)(timeGetTime() / 60000);

    // Update the settings information (volume, uptime etc. need saving)
    if (CFile::Exists(g_settings.GetSettingsFile()))
    {
      CLog::Log(LOGNOTICE, "Saving settings");
      g_settings.Save();
    }
    else
      CLog::Log(LOGNOTICE, "Not saving settings (settings.xml is not present)");

    m_bStop = true;
    CLog::Log(LOGNOTICE, "stop all");

    // stop scanning before we kill the network and so on
    CGUIDialogMusicScan *musicScan = (CGUIDialogMusicScan *)m_gWindowManager.GetWindow(WINDOW_DIALOG_MUSIC_SCAN);
    if (musicScan)
      musicScan->StopScanning();

    CGUIDialogVideoScan *videoScan = (CGUIDialogVideoScan *)m_gWindowManager.GetWindow(WINDOW_DIALOG_VIDEO_SCAN);
    if (videoScan)
      videoScan->StopScanning();

    StopServices();
    //Sleep(5000);

#ifdef __APPLE__
    g_xbmcHelper.ReleaseAllInput();
#endif

    if (m_pPlayer)
    {
      CLog::Log(LOGNOTICE, "stop player");
      delete m_pPlayer;
      m_pPlayer = NULL;
    }

#ifdef HAS_FILESYSTEM
    CLog::Log(LOGNOTICE, "stop daap clients");
    g_DaapClient.Release();
#endif
    //g_lcd->StopThread();
    g_applicationMessenger.Cleanup();

    CLog::Log(LOGNOTICE, "clean cached files!");
    g_RarManager.ClearCache(true);

    CLog::Log(LOGNOTICE, "unload skin");
    UnloadSkin();

    CLog::Log(LOGNOTICE, "stop python");
    g_pythonParser.FreeResources();

#ifdef HAS_LCD
    if (g_lcd)
    {
      g_lcd->Stop();
      delete g_lcd;
      g_lcd=NULL;
    }
#endif
    CLog::Log(LOGNOTICE, "stopped");
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "Exception in CApplication::Stop()");
  }

  Destroy();
}

bool CApplication::PlayMedia(const CFileItem& item, int iPlaylist)
{
  if (item.IsLastFM())
  {
    g_partyModeManager.Disable();
    return CLastFmManager::GetInstance()->ChangeStation(item.GetAsUrl());
  }
  if (item.IsSmartPlayList())
  {
    CDirectory dir;
    CFileItemList items;
    if (dir.GetDirectory(item.m_strPath, items) && items.Size())
    {
      CSmartPlaylist smartpl;
      //get name and type of smartplaylist, this will always succeed as GetDirectory also did this.
      smartpl.OpenAndReadName(item.m_strPath);
      CPlayList playlist;
      playlist.Add(items);
      return ProcessAndStartPlaylist(smartpl.GetName(), playlist, (smartpl.GetType() == "songs" || smartpl.GetType() == "albums") ? PLAYLIST_MUSIC:PLAYLIST_VIDEO);
    }
  }
  else if (item.IsPlayList() || item.IsInternetStream())
  {
    //is or could be a playlist
    auto_ptr<CPlayList> pPlayList (CPlayListFactory::Create(item));
    if (pPlayList.get() && pPlayList->Load(item.m_strPath))
    {
      if (iPlaylist != PLAYLIST_NONE)
        return ProcessAndStartPlaylist(item.m_strPath, *pPlayList, iPlaylist);
      else
      {
        CLog::Log(LOGWARNING, "CApplication::PlayMedia called to play a playlist %s but no idea which playlist to use, playing first item", item.m_strPath.c_str());
        if(pPlayList->size())
          return PlayFile(*(*pPlayList)[0], false);
      }
    }
  }

  //nothing special just play
  return PlayFile(item, false);
}

// PlayStack()
// For playing a multi-file video.  Particularly inefficient
// on startup, as we are required to calculate the length
// of each video, so we open + close each one in turn.
// A faster calculation of video time would improve this
// substantially.
bool CApplication::PlayStack(const CFileItem& item, bool bRestart)
{
  if (!item.IsStack())
    return false;

  // see if we have the info in the database
  // TODO: If user changes the time speed (FPS via framerate conversion stuff)
  //       then these times will be wrong.
  //       Also, this is really just a hack for the slow load up times we have
  //       A much better solution is a fast reader of FPS and fileLength
  //       that we can use on a file to get it's time.
  vector<long> times;
  bool haveTimes(false);
  CVideoDatabase dbs;
  if (dbs.Open())
  {
    dbs.GetVideoSettings(item.m_strPath, g_stSettings.m_currentVideoSettings);
    haveTimes = dbs.GetStackTimes(item.m_strPath, times);
    dbs.Close();
  }


  // calculate the total time of the stack
  CStackDirectory dir;
  dir.GetDirectory(item.m_strPath, *m_currentStack);
  long totalTime = 0;
  for (int i = 0; i < m_currentStack->Size(); i++)
  {
    if (haveTimes)
      (*m_currentStack)[i]->m_lEndOffset = times[i];
    else
    {
      int duration;
      if (!CDVDFileInfo::GetFileDuration((*m_currentStack)[i]->m_strPath, duration))
      {
        m_currentStack->Clear();
        return false;
      }
      totalTime += duration / 1000;
      (*m_currentStack)[i]->m_lEndOffset = totalTime;
      times.push_back(totalTime);
    }
  }

  double seconds = item.m_lStartOffset / 75.0;

  if (!haveTimes || item.m_lStartOffset == STARTOFFSET_RESUME )
  {  // have our times now, so update the dB
    if (dbs.Open())
    {
      if( !haveTimes )
        dbs.SetStackTimes(item.m_strPath, times);

      if( item.m_lStartOffset == STARTOFFSET_RESUME )
      {
        // can only resume seek here, not dvdstate
        CBookmark bookmark;
        if( dbs.GetResumeBookMark(item.m_strPath, bookmark) )
          seconds = bookmark.timeInSeconds;
        else
          seconds = 0.0f;
      }
      dbs.Close();
    }
  }

  *m_itemCurrentFile = item;
  m_currentStackPosition = 0;
  m_eCurrentPlayer = EPC_NONE; // must be reset on initial play otherwise last player will be used 

  if (seconds > 0)
  {
    // work out where to seek to
    for (int i = 0; i < m_currentStack->Size(); i++)
    {
      if (seconds < (*m_currentStack)[i]->m_lEndOffset)
      {
        CFileItem item(*(*m_currentStack)[i]);
        long start = (i > 0) ? (*m_currentStack)[i-1]->m_lEndOffset : 0;
        item.m_lStartOffset = (long)(seconds - start) * 75;
        m_currentStackPosition = i;
        return PlayFile(item, true);
      }
    }
  }

  return PlayFile(*(*m_currentStack)[0], true);
}

bool CApplication::PlayFile(const CFileItem& item, bool bRestart)
{
  if (!bRestart)
  {
    SaveCurrentFileSettings();

    OutputDebugString("new file set audiostream:0\n");
    // Switch to default options
    g_stSettings.m_currentVideoSettings = g_stSettings.m_defaultVideoSettings;
    // see if we have saved options in the database

    m_iPlaySpeed = 1;
    *m_itemCurrentFile = item;
    m_nextPlaylistItem = -1;
    m_currentStackPosition = 0;
    m_currentStack->Clear();
  }

  if (item.IsPlayList())
    return false;

  if (item.IsPlugin())
  { // we modify the item so that it becomes a real URL
    CFileItem item_new;
    if (DIRECTORY::CPluginDirectory::GetPluginResult(item.m_strPath, item_new))
      return PlayFile(item_new, false);
    return false;
  }

  // if we have a stacked set of files, we need to setup our stack routines for
  // "seamless" seeking and total time of the movie etc.
  // will recall with restart set to true
  if (item.IsStack())
    return PlayStack(item, bRestart);

  //Is TuxBox, this should probably be moved to CFileTuxBox
  if(item.IsTuxBox())
  {
    CLog::Log(LOGDEBUG, "%s - TuxBox URL Detected %s",__FUNCTION__, item.m_strPath.c_str());

    if(g_tuxboxService.IsRunning())
      g_tuxboxService.Stop();

    CFileItem item_new;
    if(g_tuxbox.CreateNewItem(item, item_new))
    {

      // Make sure it doesn't have a player
      // so we actually select one normally
      m_eCurrentPlayer = EPC_NONE;

      // keep the tuxbox:// url as playing url
      // and give the new url to the player
      if(PlayFile(item_new, true))
      {
        if(!g_tuxboxService.IsRunning())
          g_tuxboxService.Start();
        return true;
      }
    }
    return false;
  }

  CPlayerOptions options;
  PLAYERCOREID eNewCore = EPC_NONE;
  if( bRestart )
  {
    // have to be set here due to playstack using this for starting the file
    options.starttime = item.m_lStartOffset / 75.0;
    if (m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0 && m_itemCurrentFile->m_lStartOffset != 0)
      m_itemCurrentFile->m_lStartOffset = STARTOFFSET_RESUME; // to force fullscreen switching

    if( m_eForcedNextPlayer != EPC_NONE )
      eNewCore = m_eForcedNextPlayer;
    else if( m_eCurrentPlayer == EPC_NONE )
      eNewCore = CPlayerCoreFactory::GetDefaultPlayer(item);
    else
      eNewCore = m_eCurrentPlayer;
  }
  else
  {
    options.starttime = item.m_lStartOffset / 75.0;

    if (item.IsVideo())
    {
      // open the d/b and retrieve the bookmarks for the current movie
      CVideoDatabase dbs;
      dbs.Open();
      dbs.GetVideoSettings(item.m_strPath, g_stSettings.m_currentVideoSettings);

      if( item.m_lStartOffset == STARTOFFSET_RESUME )
      {
        options.starttime = 0.0f;
        CBookmark bookmark;
        if(dbs.GetResumeBookMark(item.m_strPath, bookmark))
        {
          options.starttime = bookmark.timeInSeconds;
          options.state = bookmark.playerState;
        }
      }
      else if (item.HasVideoInfoTag())
      {
        const CVideoInfoTag *tag = item.GetVideoInfoTag();

        if (tag->m_iBookmarkId != -1 && tag->m_iBookmarkId != 0)
        {
          CBookmark bookmark;
          dbs.GetBookMarkForEpisode(*tag, bookmark);
          options.starttime = bookmark.timeInSeconds;
          options.state = bookmark.playerState;
        }
      }

      dbs.Close();
    }

    if (m_eForcedNextPlayer != EPC_NONE)
      eNewCore = m_eForcedNextPlayer;
    else
      eNewCore = CPlayerCoreFactory::GetDefaultPlayer(item);
  }

  // this really aught to be inside !bRestart, but since PlayStack
  // uses that to init playback, we have to keep it outside
  int playlist = g_playlistPlayer.GetCurrentPlaylist();
  if (playlist == PLAYLIST_VIDEO && g_playlistPlayer.GetPlaylist(playlist).size() > 1)
  { // playing from a playlist by the looks
    // don't switch to fullscreen if we are not playing the first item...
    options.fullscreen = !g_playlistPlayer.HasPlayedFirstFile();
  }
  else if(m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0)
  {
    // TODO - this will fail if user seeks back to first file in stack
    if(m_currentStackPosition == 0 || m_itemCurrentFile->m_lStartOffset == STARTOFFSET_RESUME)
      options.fullscreen = true;
    else
      options.fullscreen = false;
    // reset this so we don't think we are resuming on seek
    m_itemCurrentFile->m_lStartOffset = 0;
  }
  else
    options.fullscreen = true;

  // reset any forced player
  m_eForcedNextPlayer = EPC_NONE;

#ifdef HAS_KARAOKE
  //We have to stop parsing a cdg before mplayer is deallocated
  // WHY do we have to do this????
  if(m_pCdgParser)
    m_pCdgParser->Stop();
#endif

  // tell system we are starting a file
  m_bPlaybackStarting = true;

  // We should restart the player, unless the previous and next tracks are using
  // one of the players that allows gapless playback (paplayer, dvdplayer)
  if (m_pPlayer)
  {
    if ( !(m_eCurrentPlayer == eNewCore && (m_eCurrentPlayer == EPC_DVDPLAYER || m_eCurrentPlayer  == EPC_PAPLAYER)) )
    {
      delete m_pPlayer;
      m_pPlayer = NULL;
    }
  }

  if (!m_pPlayer)
  {
    m_eCurrentPlayer = eNewCore;
    m_pPlayer = CPlayerCoreFactory::CreatePlayer(eNewCore, *this);
  }

  bool bResult;
  if (m_pPlayer)
    bResult = m_pPlayer->OpenFile(item, options);
  else
  {
    CLog::Log(LOGERROR, "Error creating player for item %s (File doesn't exist?)", item.m_strPath.c_str());
    bResult = false;
  }

  if(bResult)
  {
    if (m_iPlaySpeed != 1)
    {
      int iSpeed = m_iPlaySpeed;
      m_iPlaySpeed = 1;
      SetPlaySpeed(iSpeed);
    }

#ifdef HAS_VIDEO_PLAYBACK
    if( IsPlayingVideo() )
    {
      // if player didn't manange to switch to fullscreen by itself do it here
      if( options.fullscreen && g_renderManager.IsStarted()
       && m_gWindowManager.GetActiveWindow() != WINDOW_FULLSCREEN_VIDEO )
         SwitchToFullScreen();
    }
#endif

    if (!g_guiSettings.GetBool("lookandfeel.soundsduringplayback"))
      g_audioManager.Enable(false);
  }
  m_bPlaybackStarting = false;
  if(bResult)
  {
    // we must have started, otherwise player might send this later
    if(IsPlaying())
      OnPlayBackStarted();
  }
  else
  {
    // we send this if it isn't playlistplayer that is doing this
    int next = g_playlistPlayer.GetNextSong();
    int size = g_playlistPlayer.GetPlaylist(g_playlistPlayer.GetCurrentPlaylist()).size();
    if(next < 0
    || next >= size)
      OnPlayBackStopped();
  }

  return bResult;
}

void CApplication::OnPlayBackEnded()
{
  if(m_bPlaybackStarting)
    return;

  // informs python script currently running playback has ended
  // (does nothing if python is not loaded)
  g_pythonParser.OnPlayBackEnded();
  // Let's tell the outside world as well
  if (m_pXbmcHttp && g_stSettings.m_HttpApiBroadcastLevel>=1)
    g_applicationMessenger.HttpApi("broadcastlevel; OnPlayBackEnded;1");
  
  if (IsPlayingAudio())
  {
    CLastfmScrobbler::GetInstance()->SubmitQueue();
    CLibrefmScrobbler::GetInstance()->SubmitQueue();
  }

  CLog::Log(LOGDEBUG, "%s - Playback has finished", __FUNCTION__);

  CGUIMessage msg(GUI_MSG_PLAYBACK_ENDED, 0, 0);
  m_gWindowManager.SendThreadMessage(msg);
}

void CApplication::OnPlayBackStarted()
{
  if(m_bPlaybackStarting)
    return;

  // informs python script currently running playback has started
  // (does nothing if python is not loaded)
  g_pythonParser.OnPlayBackStarted();

  // Let's tell the outside world as well
  if (m_pXbmcHttp && g_stSettings.m_HttpApiBroadcastLevel>=1)
    g_applicationMessenger.HttpApi("broadcastlevel; OnPlayBackStarted;1");

  CLog::Log(LOGDEBUG, "%s - Playback has started", __FUNCTION__);

  CGUIMessage msg(GUI_MSG_PLAYBACK_STARTED, 0, 0);
  m_gWindowManager.SendThreadMessage(msg);
}

void CApplication::OnQueueNextItem()
{
  // informs python script currently running that we are requesting the next track
  // (does nothing if python is not loaded)
  g_pythonParser.OnQueueNextItem(); // currently unimplemented

  // Let's tell the outside world as well
  if (m_pXbmcHttp && g_stSettings.m_HttpApiBroadcastLevel>=1)
  g_applicationMessenger.HttpApi("broadcastlevel; OnQueueNextItem;1");

  CLog::Log(LOGDEBUG, "Player has asked for the next item");

  if(IsPlayingAudio())
  {
    CLastfmScrobbler::GetInstance()->SubmitQueue();
    CLibrefmScrobbler::GetInstance()->SubmitQueue();
  }

  CGUIMessage msg(GUI_MSG_QUEUE_NEXT_ITEM, 0, 0);
  m_gWindowManager.SendThreadMessage(msg);
}

void CApplication::OnPlayBackStopped()
{
  if(m_bPlaybackStarting)
    return;

  // informs python script currently running playback has ended
  // (does nothing if python is not loaded)
  g_pythonParser.OnPlayBackStopped();

  // Let's tell the outside world as well
  if (m_pXbmcHttp && g_stSettings.m_HttpApiBroadcastLevel>=1)
    g_applicationMessenger.HttpApi("broadcastlevel; OnPlayBackStopped;1");
  
  if (IsPlayingAudio())
  {
    CLastfmScrobbler::GetInstance()->SubmitQueue();
    CLibrefmScrobbler::GetInstance()->SubmitQueue();
  }

  CLog::Log(LOGDEBUG, "%s - Playback was stopped", __FUNCTION__);

  CGUIMessage msg( GUI_MSG_PLAYBACK_STOPPED, 0, 0 );
  m_gWindowManager.SendThreadMessage(msg);
}

bool CApplication::IsPlaying() const
{
  if (!m_pPlayer)
    return false;
  if (!m_pPlayer->IsPlaying())
    return false;
  return true;
}

bool CApplication::IsPaused() const
{
  if (!m_pPlayer)
    return false;
  if (!m_pPlayer->IsPlaying())
    return false;
  return m_pPlayer->IsPaused();
}

bool CApplication::IsPlayingAudio() const
{
  if (!m_pPlayer)
    return false;
  if (!m_pPlayer->IsPlaying())
    return false;
  if (m_pPlayer->HasVideo())
    return false;
  if (m_pPlayer->HasAudio())
    return true;
  return false;
}

bool CApplication::IsPlayingVideo() const
{
  if (!m_pPlayer)
    return false;
  if (!m_pPlayer->IsPlaying())
    return false;
  if (m_pPlayer->HasVideo())
    return true;
  return false;
}

bool CApplication::IsPlayingFullScreenVideo() const
{
  return IsPlayingVideo() && g_graphicsContext.IsFullScreenVideo();
}

void CApplication::SaveFileState()
{
  CStdString progressTrackingFile = m_progressTrackingItem->m_strPath;

  if (progressTrackingFile != "")
  {
    if (m_progressTrackingItem->IsVideo())
    {
      CLog::Log(LOGDEBUG, "%s - Saving file state for video file %s", __FUNCTION__, progressTrackingFile.c_str());

      CVideoDatabase videodatabase;
      if (videodatabase.Open())
      {
        if (m_progressTrackingPlayCountUpdate)
        {
          CLog::Log(LOGDEBUG, "%s - Marking video file %s as watched", __FUNCTION__, progressTrackingFile.c_str());

          // consider this item as played
          videodatabase.MarkAsWatched(*m_progressTrackingItem);
          CUtil::DeleteVideoDatabaseDirectoryCache();
        }

        if (g_stSettings.m_currentVideoSettings != g_stSettings.m_defaultVideoSettings)
        {
          videodatabase.SetVideoSettings(progressTrackingFile, g_stSettings.m_currentVideoSettings);
        }

        if (m_progressTrackingVideoResumeBookmark.timeInSeconds < 0.0f)
        {
          videodatabase.ClearBookMarksOfFile(progressTrackingFile, CBookmark::RESUME);
        }
        else
        if (m_progressTrackingVideoResumeBookmark.timeInSeconds > 0.0f)
        {
          videodatabase.AddBookMarkToFile(progressTrackingFile, m_progressTrackingVideoResumeBookmark, CBookmark::RESUME);
        }

        videodatabase.Close();
      }
    }
    else
    {
      CLog::Log(LOGDEBUG, "%s - Saving file state for audio file %s", __FUNCTION__, progressTrackingFile.c_str());

      if (m_progressTrackingPlayCountUpdate)
      {
        // Can't write to the musicdatabase while scanning for music info
        CGUIDialogMusicScan *dialog = (CGUIDialogMusicScan *)m_gWindowManager.GetWindow(WINDOW_DIALOG_MUSIC_SCAN);
        if (dialog && !dialog->IsDialogRunning())
        {
          // consider this item as played
          CLog::Log(LOGDEBUG, "%s - Marking audio file %s as listened", __FUNCTION__, progressTrackingFile.c_str());

          CMusicDatabase musicdatabase;
          if (musicdatabase.Open())
          {
            musicdatabase.IncrTop100CounterByFileName(progressTrackingFile);
            musicdatabase.Close();
          }
        }
      }
    }
  }
}

void CApplication::UpdateFileState()
{
  // Did the file change?
  if (m_progressTrackingItem->m_strPath != "" && m_progressTrackingItem->m_strPath != CurrentFile())
  {
    SaveFileState();

    // Reset tracking item
    m_progressTrackingItem->Reset();
  }
  else
  if (IsPlayingVideo() || IsPlayingAudio())
  {
    if (m_progressTrackingItem->m_strPath == "")
    {
      // Init some stuff
      *m_progressTrackingItem = CurrentFileItem();
      m_progressTrackingPlayCountUpdate = false;
    }

    if ((m_progressTrackingItem->IsAudio() && g_advancedSettings.m_audioPlayCountMinimumPercent > 0 &&
        GetPercentage() >= g_advancedSettings.m_audioPlayCountMinimumPercent) ||
        (m_progressTrackingItem->IsVideo() && g_advancedSettings.m_videoPlayCountMinimumPercent > 0 &&
        GetPercentage() >= g_advancedSettings.m_videoPlayCountMinimumPercent))
    {
      m_progressTrackingPlayCountUpdate = true;
    }

    if (m_progressTrackingItem->IsVideo())
    {
      // Update bookmark for save
      m_progressTrackingVideoResumeBookmark.player = CPlayerCoreFactory::GetPlayerName(m_eCurrentPlayer);
      m_progressTrackingVideoResumeBookmark.playerState = m_pPlayer->GetPlayerState();
      m_progressTrackingVideoResumeBookmark.thumbNailImage.Empty();

      if (g_advancedSettings.m_videoIgnoreAtEnd > 0 &&
          GetTotalTime() - GetTime() < g_advancedSettings.m_videoIgnoreAtEnd)
      {
        // Delete the bookmark
        m_progressTrackingVideoResumeBookmark.timeInSeconds = -1.0f;
      }
      else
      if (GetTime() > g_advancedSettings.m_videoIgnoreAtStart)
      {
        // Update the bookmark
        m_progressTrackingVideoResumeBookmark.timeInSeconds = GetTime();
        m_progressTrackingVideoResumeBookmark.totalTimeInSeconds = GetTotalTime();
      }
      else
      {
        // Do nothing
        m_progressTrackingVideoResumeBookmark.timeInSeconds = 0.0f;
      }
    }
  }
}

void CApplication::StopPlaying()
{
  int iWin = m_gWindowManager.GetActiveWindow();
  if ( IsPlaying() )
  {
#ifdef HAS_KARAOKE
    if( m_pCdgParser )
      m_pCdgParser->Stop();
#endif

    // turn off visualisation window when stopping
    if (iWin == WINDOW_VISUALISATION)
      m_gWindowManager.PreviousWindow();

    if (m_pPlayer)
      m_pPlayer->CloseFile();

    g_partyModeManager.Disable();
  }
}

bool CApplication::NeedRenderFullScreen()
{
  if (m_gWindowManager.GetActiveWindow() == WINDOW_FULLSCREEN_VIDEO)
  {
    m_gWindowManager.UpdateModelessVisibility();

    if (m_gWindowManager.HasDialogOnScreen()) return true;
    if (g_Mouse.IsActive()) return true;

    CGUIWindowFullScreen *pFSWin = (CGUIWindowFullScreen *)m_gWindowManager.GetWindow(WINDOW_FULLSCREEN_VIDEO);
    if (!pFSWin)
      return false;
    return pFSWin->NeedRenderFullScreen();
  }
  return false;
}

void CApplication::RenderFullScreen()
{
  g_ApplicationRenderer.Render(true);
}

void CApplication::DoRenderFullScreen()
{
  if (g_graphicsContext.IsFullScreenVideo())
  {
    // make sure our overlays are closed
    CGUIDialog *overlay = (CGUIDialog *)m_gWindowManager.GetWindow(WINDOW_VIDEO_OVERLAY);
    if (overlay) overlay->Close(true);
    overlay = (CGUIDialog *)m_gWindowManager.GetWindow(WINDOW_MUSIC_OVERLAY);
    if (overlay) overlay->Close(true);

    CGUIWindowFullScreen *pFSWin = (CGUIWindowFullScreen *)m_gWindowManager.GetWindow(WINDOW_FULLSCREEN_VIDEO);
    if (!pFSWin)
      return ;
    pFSWin->RenderFullScreen();

    if (m_gWindowManager.HasDialogOnScreen())
      m_gWindowManager.RenderDialogs();
    // Render the mouse pointer, if visible...
    if (g_Mouse.IsActive())
      g_application.m_guiPointer.Render();
  }
}

void CApplication::ResetScreenSaver()
{
  // reset our timers
  m_shutdownTimer.StartZero();

  // screen saver timer is reset only if we're not already in screensaver mode
  if (!m_bScreenSave && m_iScreenSaveLock == 0)
    m_screenSaverTimer.StartZero();
}

bool CApplication::ResetScreenSaverWindow()
{
  if (m_iScreenSaveLock == 2)
    return false;

  // if Screen saver is active
  if (m_bScreenSave)
  {
    int iProfile = g_settings.m_iLastLoadedProfileIndex;
    if (m_iScreenSaveLock == 0)
      if (g_guiSettings.GetBool("screensaver.uselock")                           &&
          g_settings.m_vecProfiles[0].getLockMode() != LOCK_MODE_EVERYONE        &&
          g_settings.m_vecProfiles[iProfile].getLockMode() != LOCK_MODE_EVERYONE &&
         !g_guiSettings.GetString("screensaver.mode").Equals("Black")            &&
        !(g_guiSettings.GetBool("screensaver.usemusicvisinstead")                &&
         !g_guiSettings.GetString("screensaver.mode").Equals("Black")            &&
          g_application.IsPlayingAudio())                                          )
      {
        m_iScreenSaveLock = 2;
        CGUIMessage msg(GUI_MSG_CHECK_LOCK,0,0);
        m_gWindowManager.GetWindow(WINDOW_SCREENSAVER)->OnMessage(msg);
      }
    if (m_iScreenSaveLock == -1)
    {
      m_iScreenSaveLock = 0;
      return true;
    }

    // disable screensaver
    m_bScreenSave = false;
    m_iScreenSaveLock = 0;
    m_screenSaverTimer.StartZero();

    float fFadeLevel = 1.0f;
    if (m_screenSaverMode == "Visualisation" || m_screenSaverMode == "Slideshow" || m_screenSaverMode == "Fanart Slideshow")
    {
      // we can just continue as usual from vis mode
      return false;
    }
    else if (m_screenSaverMode == "Dim")
    {
      fFadeLevel = (float)g_guiSettings.GetInt("screensaver.dimlevel") / 100;
    }
    else if (m_screenSaverMode == "Black")
    {
      fFadeLevel = 0;
    }
    else if (m_screenSaverMode != "None")
    { // we're in screensaver window
      if (m_gWindowManager.GetActiveWindow() == WINDOW_SCREENSAVER)
        m_gWindowManager.PreviousWindow();  // show the previous window
      return true;
    }

    // Fade to dim or black screensaver is active --> fade in
    D3DGAMMARAMP Ramp;
    for (float fade = fFadeLevel; fade <= 1; fade += 0.01f)
    {
      for (int i = 0;i < 256;i++)
      {
        Ramp.red[i] = (int)((float)m_OldRamp.red[i] * fade);
        Ramp.green[i] = (int)((float)m_OldRamp.green[i] * fade);
        Ramp.blue[i] = (int)((float)m_OldRamp.blue[i] * fade);
      }
      Sleep(5);
      m_pd3dDevice->SetGammaRamp(GAMMA_RAMP_FLAG, &Ramp); // use immediate to get a smooth fade
    }
    m_pd3dDevice->SetGammaRamp(0, &m_OldRamp); // put the old gamma ramp back in place
    return true;
  }
  else
    return false;
}

void CApplication::CheckScreenSaver()
{
  // if the screen saver window is active, then clearly we are already active
  if (m_gWindowManager.IsWindowActive(WINDOW_SCREENSAVER))
  {
    m_bScreenSave = true;
    return;
  }

  bool resetTimer = false;
  if (IsPlayingVideo() && !m_pPlayer->IsPaused()) // are we playing video and it is not paused?
    resetTimer = true;

  if (IsPlayingAudio() && m_gWindowManager.GetActiveWindow() == WINDOW_VISUALISATION) // are we playing some music in fullscreen vis?
    resetTimer = true;

  if (resetTimer)
  {
    m_screenSaverTimer.StartZero();
    return;
  }

  if (m_bScreenSave) // already running the screensaver
    return;

  if ( m_screenSaverTimer.GetElapsedSeconds() > g_guiSettings.GetInt("screensaver.time") * 60 )
    ActivateScreenSaver();
}

// activate the screensaver.
// if forceType is true, we ignore the various conditions that can alter
// the type of screensaver displayed
void CApplication::ActivateScreenSaver(bool forceType /*= false */)
{
  D3DGAMMARAMP Ramp;
  FLOAT fFadeLevel = 0;

  m_bScreenSave = true;

  // Get Screensaver Mode
  m_screenSaverMode = g_guiSettings.GetString("screensaver.mode");

  if (!forceType)
  {
    // set to Dim in the case of a dialog on screen or playing video
    if (m_gWindowManager.HasModalDialog() || (IsPlayingVideo() && g_guiSettings.GetBool("screensaver.usedimonpause")))
      m_screenSaverMode = "Dim";
    // Check if we are Playing Audio and Vis instead Screensaver!
    else if (IsPlayingAudio() && g_guiSettings.GetBool("screensaver.usemusicvisinstead") && g_guiSettings.GetString("mymusic.visualisation") != "None")
    { // activate the visualisation
      m_screenSaverMode = "Visualisation";
      m_gWindowManager.ActivateWindow(WINDOW_VISUALISATION);
      return;
    }
  }
  // Picture slideshow
  if (m_screenSaverMode == "SlideShow" || m_screenSaverMode == "Fanart Slideshow")
  {
    // reset our codec info - don't want that on screen
    g_infoManager.SetShowCodec(false);
    g_applicationMessenger.PictureSlideShow(g_guiSettings.GetString("screensaver.slideshowpath"), true);
    return;
  }
  else if (m_screenSaverMode == "Dim")
  {
    fFadeLevel = (FLOAT) g_guiSettings.GetInt("screensaver.dimlevel") / 100; // 0.07f;
  }
  else if (m_screenSaverMode == "Black")
  {
    fFadeLevel = 0;
  }
  else if (m_screenSaverMode != "None")
  {
    m_gWindowManager.ActivateWindow(WINDOW_SCREENSAVER);
    return ;
  }
  
  // Fade to fFadeLevel
  m_pd3dDevice->GetGammaRamp(&m_OldRamp); // Store the old gamma ramp
  for (float fade = 1.f; fade >= fFadeLevel; fade -= 0.01f)
  {
    for (int i = 0;i < 256;i++)
    {
      Ramp.red[i] = (int)((float)m_OldRamp.red[i] * fade);
      Ramp.green[i] = (int)((float)m_OldRamp.green[i] * fade);
      Ramp.blue[i] = (int)((float)m_OldRamp.blue[i] * fade);
    }
    Sleep(5);
    m_pd3dDevice->SetGammaRamp(GAMMA_RAMP_FLAG, &Ramp); // use immediate to get a smooth fade
  }
}

void CApplication::CheckShutdown()
{
#ifdef HAS_XBOX_HARDWARE
  CGUIDialogMusicScan *pMusicScan = (CGUIDialogMusicScan *)m_gWindowManager.GetWindow(WINDOW_DIALOG_MUSIC_SCAN);
  CGUIDialogVideoScan *pVideoScan = (CGUIDialogVideoScan *)m_gWindowManager.GetWindow(WINDOW_DIALOG_VIDEO_SCAN);

  // first check if we should reset the timer
  bool resetTimer = false;
  if (IsPlaying()) // is something playing?
    resetTimer = true;

#ifdef HAS_FTP_SERVER
  if (m_pFileZilla && m_pFileZilla->GetNoConnections() != 0) // is FTP active ?
    resetTimer = true;
#endif

  if (pMusicScan && pMusicScan->IsScanning()) // music scanning?
    resetTimer = true;

  if (pVideoScan && pVideoScan->IsScanning()) // video scanning?
    resetTimer = true;

  if (m_gWindowManager.IsWindowActive(WINDOW_DIALOG_PROGRESS)) // progress dialog is onscreen
    resetTimer = true;

  if (resetTimer)
  {
    m_shutdownTimer.StartZero();
    return;
  }

  if ( m_shutdownTimer.GetElapsedSeconds() > g_guiSettings.GetInt("system.shutdowntime") * 60 )
  {
    g_applicationMessenger.Shutdown(); // Turn off the box
  }
#endif
}

//Check if hd spindown must be blocked
bool CApplication::MustBlockHDSpinDown(bool bCheckThisForNormalSpinDown)
{
#ifdef HAS_XBOX_HARDWARE
  if (IsPlayingVideo())
  {
    //block immediate spindown when playing a video non-fullscreen (videocontrol is playing)
    if ((!bCheckThisForNormalSpinDown) && (!g_graphicsContext.IsFullScreenVideo()))
    {
      return true;
    }
    //allow normal hd spindown always if the movie is paused
    if ((bCheckThisForNormalSpinDown) && (m_pPlayer->IsPaused()))
    {
      return false;
    }
    //don't allow hd spindown when playing files with vobsub subtitles.
    CStdString strSubTitelExtension;
    if (m_pPlayer->GetSubtitleExtension(strSubTitelExtension))
    {
      return (strSubTitelExtension == ".idx");
    }
  }
#endif
  return false;
}

void CApplication::CheckNetworkHDSpinDown(bool playbackStarted)
{
  int iSpinDown = g_guiSettings.GetInt("harddisk.remoteplayspindown");
  if (iSpinDown == SPIN_DOWN_NONE)
    return ;
  if (m_gWindowManager.HasModalDialog())
    return ;
  if (MustBlockHDSpinDown(false))
    return ;

  if ((!m_bNetworkSpinDown) || playbackStarted)
  {
    int iDuration = 0;
    if (IsPlayingAudio())
    {
      //try to get duration from current tag because mplayer doesn't calculate vbr mp3 correctly
      if (m_itemCurrentFile->HasMusicInfoTag())
        iDuration = m_itemCurrentFile->GetMusicInfoTag()->GetDuration();
    }
    if (IsPlaying() && iDuration <= 0)
    {
      iDuration = (int)GetTotalTime();
    }
    //spin down harddisk when the current file being played is not on local harddrive and
    //duration is more then spindown timeoutsetting or duration is unknown (streams)
    if (
      !m_itemCurrentFile->IsHD() &&
      (
        (iSpinDown == SPIN_DOWN_VIDEO && IsPlayingVideo()) ||
        (iSpinDown == SPIN_DOWN_MUSIC && IsPlayingAudio()) ||
        (iSpinDown == SPIN_DOWN_BOTH && (IsPlayingVideo() || IsPlayingAudio()))
      ) &&
      (
        (iDuration <= 0) ||
        (iDuration > g_guiSettings.GetInt("harddisk.remoteplayhdspindownminduration")*60)
      )
    )
    {
      m_bNetworkSpinDown = true;
      if (!playbackStarted)
      { //if we got here not because of a playback start check what screen we are in
        // get the current active window
        int iWin = m_gWindowManager.GetActiveWindow();
        if (iWin == WINDOW_FULLSCREEN_VIDEO)
        {
          // check if OSD is visible, if so don't do immediate spindown
          CGUIWindowOSD *pOSD = (CGUIWindowOSD *)m_gWindowManager.GetWindow(WINDOW_OSD);
          if (pOSD)
            m_bNetworkSpinDown = !pOSD->IsDialogRunning();
        }
      }
      if (m_bNetworkSpinDown)
      {
        //do the spindown right now + delayseconds
        m_dwSpinDownTime = timeGetTime();
      }
    }
  }
  if (m_bNetworkSpinDown)
  {
    // check the elapsed time
    DWORD dwTimeSpan = timeGetTime() - m_dwSpinDownTime;
    if ( (m_dwSpinDownTime != 0) && (dwTimeSpan >= ((DWORD)g_guiSettings.GetInt("harddisk.remoteplayspindowndelay")*1000UL)) )
    {
      // time has elapsed, spin it down
#ifdef HAS_XBOX_HARDWARE
      XKHDD::SpindownHarddisk();
#endif
      //stop checking until a key is pressed.
      m_dwSpinDownTime = 0;
      m_bNetworkSpinDown = true;
    }
    else if (m_dwSpinDownTime == 0 && IsPlaying())
    {
      // we are currently spun down - let's spin back up again if we are playing media
      // and we're within 10 seconds (or 0.5*spindown time) of the end.  This should
      // make returning to the GUI a bit snappier + speed up stacked item changes.
      int iMinSpinUp = 10;
      if (iMinSpinUp > g_guiSettings.GetInt("harddisk.remoteplayspindowndelay")*0.5f)
        iMinSpinUp = (int)(g_guiSettings.GetInt("harddisk.remoteplayspindowndelay")*0.5f);
      if (g_infoManager.GetPlayTimeRemaining() == iMinSpinUp)
      { // spin back up
#ifdef HAS_XBOX_HARDWARE
        XKHDD::SpindownHarddisk(false);
#endif
      }
    }
  }
}

void CApplication::CheckHDSpindown()
{
  if (!g_guiSettings.GetInt("harddisk.spindowntime"))
    return ;
  if (m_gWindowManager.HasModalDialog())
    return ;
  if (MustBlockHDSpinDown())
    return ;

  if (!m_bSpinDown &&
      (
        !IsPlaying() ||
        (IsPlaying() && !m_itemCurrentFile->IsHD())
      )
     )
  {
    m_bSpinDown = true;
    m_bNetworkSpinDown = false; // let networkspindown override normal spindown
    m_dwSpinDownTime = timeGetTime();
  }

  //Can we do a spindown right now?
  if (m_bSpinDown)
  {
    // yes, then check the elapsed time
    DWORD dwTimeSpan = timeGetTime() - m_dwSpinDownTime;
    if ( (m_dwSpinDownTime != 0) && (dwTimeSpan >= ((DWORD)g_guiSettings.GetInt("harddisk.spindowntime")*60UL*1000UL)) )
    {
      // time has elapsed, spin it down
#ifdef HAS_XBOX_HARDWARE
      XKHDD::SpindownHarddisk();
#endif
      //stop checking until a key is pressed.
      m_dwSpinDownTime = 0;
      m_bSpinDown = true;
    }
  }
}

bool CApplication::OnMessage(CGUIMessage& message)
{
  switch ( message.GetMessage() )
  {
  case GUI_MSG_NOTIFY_ALL:
    {
      if (message.GetParam1()==GUI_MSG_REMOVED_MEDIA)
      {
        // Update general playlist: Remove DVD playlist items
        int nRemoved = g_playlistPlayer.RemoveDVDItems();
        if ( nRemoved > 0 )
        {
          CGUIMessage msg( GUI_MSG_PLAYLIST_CHANGED, 0, 0 );
          m_gWindowManager.SendMessage( msg );
        }
        // stop the file if it's on dvd (will set the resume point etc)
        if (m_itemCurrentFile->IsOnDVD())
          StopPlaying();
      }
    }
    break;

  case GUI_MSG_PLAYBACK_STARTED:
    {
      // Update our infoManager with the new details etc.
      if (m_nextPlaylistItem >= 0)
      { // we've started a previously queued item
        CFileItemPtr item = g_playlistPlayer.GetPlaylist(g_playlistPlayer.GetCurrentPlaylist())[m_nextPlaylistItem];
        // update the playlist manager
        WORD currentSong = g_playlistPlayer.GetCurrentSong();
        DWORD dwParam = ((currentSong & 0xffff) << 16) | (m_nextPlaylistItem & 0xffff);
        CGUIMessage msg(GUI_MSG_PLAYLISTPLAYER_CHANGED, 0, 0, g_playlistPlayer.GetCurrentPlaylist(), dwParam, item);
        m_gWindowManager.SendThreadMessage(msg);
        g_playlistPlayer.SetCurrentSong(m_nextPlaylistItem);
        *m_itemCurrentFile = *item;
      }
      g_infoManager.SetCurrentItem(*m_itemCurrentFile);
      CLastFmManager::GetInstance()->OnSongChange(*m_itemCurrentFile);
      g_partyModeManager.OnSongChange(true);

      CheckNetworkHDSpinDown(true);
      StartLEDControl(true);
      DimLCDOnPlayback(true);

      if (IsPlayingAudio())
      {
        // Start our cdg parser as appropriate
#ifdef HAS_KARAOKE
        if (m_pCdgParser && g_guiSettings.GetBool("karaoke.enabled") && !m_itemCurrentFile->IsInternetStream())
        {
          if (m_pCdgParser->IsRunning())
            m_pCdgParser->Stop();
          if (m_itemCurrentFile->IsMusicDb())
          {
            if (!m_itemCurrentFile->HasMusicInfoTag() || !m_itemCurrentFile->GetMusicInfoTag()->Loaded())
            {
              IMusicInfoTagLoader* tagloader = CMusicInfoTagLoaderFactory::CreateLoader(m_itemCurrentFile->m_strPath);
              tagloader->Load(m_itemCurrentFile->m_strPath,*m_itemCurrentFile->GetMusicInfoTag());
              delete tagloader;
            }
            m_pCdgParser->Start(m_itemCurrentFile->GetMusicInfoTag()->GetURL());
          }
          else
            m_pCdgParser->Start(m_itemCurrentFile->m_strPath);
        }
#endif
        // Let scrobbler know about the track
        const CMusicInfoTag* tag=g_infoManager.GetCurrentSongTag();
        if (tag)
        {
          CLastfmScrobbler::GetInstance()->AddSong(*tag, CLastFmManager::GetInstance()->IsRadioEnabled());
          CLibrefmScrobbler::GetInstance()->AddSong(*tag, CLastFmManager::GetInstance()->IsRadioEnabled());
        }
      }
      return true;
    }
    break;

  case GUI_MSG_QUEUE_NEXT_ITEM:
    {
      // Check to see if our playlist player has a new item for us,
      // and if so, we check whether our current player wants the file
      int iNext = g_playlistPlayer.GetNextSong();
      CPlayList& playlist = g_playlistPlayer.GetPlaylist(g_playlistPlayer.GetCurrentPlaylist());
      if (iNext < 0 || iNext >= playlist.size())
      {
        if (m_pPlayer) m_pPlayer->OnNothingToQueueNotify();
        return true; // nothing to do
      }
      // ok, grab the next song
      CFileItemPtr item = playlist[iNext];
      // ok - send the file to the player if it wants it
      if (m_pPlayer && m_pPlayer->QueueNextFile(*item))
      { // player wants the next file
        m_nextPlaylistItem = iNext;
      }
      return true;
    }
    break;

  case GUI_MSG_PLAYBACK_STOPPED:
  case GUI_MSG_PLAYBACK_ENDED:
  case GUI_MSG_PLAYLISTPLAYER_STOPPED:
    {
      // first check if we still have items in the stack to play
      if (message.GetMessage() == GUI_MSG_PLAYBACK_ENDED)
      {
        if (m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0 && m_currentStackPosition < m_currentStack->Size() - 1)
        { // just play the next item in the stack
          PlayFile(*(*m_currentStack)[++m_currentStackPosition], true);
          return true;
        }
      }

      // reset our spindown
      m_bNetworkSpinDown = false;
      m_bSpinDown = false;

      // reset the current playing file
      m_itemCurrentFile->Reset();
      g_infoManager.ResetCurrentItem();
      m_currentStack->Clear();

      // stop lastfm
      if (CLastFmManager::GetInstance()->IsRadioEnabled())
        CLastFmManager::GetInstance()->StopRadio();

      if (message.GetMessage() == GUI_MSG_PLAYBACK_ENDED)
      {
        // sending true to PlayNext() effectively passes bRestart to PlayFile()
        // which is not generally what we want (except for stacks, which are
        // handled above)
        g_playlistPlayer.PlayNext();
      }
      else
      {
        if (m_pPlayer)
        {
          delete m_pPlayer;
          m_pPlayer = 0;
        }
      }

      if (!IsPlaying())
      {
        g_audioManager.Enable(true);
        StartLEDControl(false);
        DimLCDOnPlayback(false);

#ifdef HAS_KARAOKE
        if(m_pCdgParser)
          m_pCdgParser->Free();
#endif
      }

      if (!IsPlayingVideo() && m_gWindowManager.GetActiveWindow() == WINDOW_FULLSCREEN_VIDEO)
      {
        m_gWindowManager.PreviousWindow();
      }

      if (!IsPlayingAudio() && g_playlistPlayer.GetCurrentPlaylist() == PLAYLIST_NONE && m_gWindowManager.GetActiveWindow() == WINDOW_VISUALISATION)
      {
        g_settings.Save();  // save vis settings
        ResetScreenSaverWindow();
        m_gWindowManager.PreviousWindow();
      }

      // reset the audio playlist on finish
      if (!IsPlayingAudio() && (g_guiSettings.GetBool("mymusic.clearplaylistsonend")) && (g_playlistPlayer.GetCurrentPlaylist() == PLAYLIST_MUSIC))
      {
        g_playlistPlayer.ClearPlaylist(PLAYLIST_MUSIC);
        g_playlistPlayer.Reset();
        g_playlistPlayer.SetCurrentPlaylist(PLAYLIST_NONE);
      }

      // DVD ejected while playing in vis ?
      if (!IsPlayingAudio() && (m_itemCurrentFile->IsCDDA() || m_itemCurrentFile->IsOnDVD()) && !CDetectDVDMedia::IsDiscInDrive() && m_gWindowManager.GetActiveWindow() == WINDOW_VISUALISATION)
      {
        // yes, disable vis
        g_settings.Save();    // save vis settings
        ResetScreenSaverWindow();
        m_gWindowManager.PreviousWindow();
      }
      return true;
    }
    break;

  case GUI_MSG_PLAYLISTPLAYER_STARTED:
  case GUI_MSG_PLAYLISTPLAYER_CHANGED:
    {
      return true;
    }
    break;
  case GUI_MSG_FULLSCREEN:
    { // Switch to fullscreen, if we can
      SwitchToFullScreen();
      return true;
    }
    break;
  case GUI_MSG_EXECUTE:
    {
      // see if it is a user set string
      CLog::Log(LOGDEBUG,"%s : Translating %s", __FUNCTION__, message.GetStringParam().c_str());
      CGUIInfoLabel info(message.GetStringParam(), "");
      message.SetStringParam(info.GetLabel(0));
      CLog::Log(LOGDEBUG,"%s : To %s", __FUNCTION__, message.GetStringParam().c_str());

      // user has asked for something to be executed
      if (CUtil::IsBuiltIn(message.GetStringParam()))
        CUtil::ExecBuiltIn(message.GetStringParam());
      else
      {
        // try translating the action from our ButtonTranslator
        WORD actionID;
        if (CButtonTranslator::TranslateActionString(message.GetStringParam().c_str(), actionID))
        {
          CAction action;
          action.wID = actionID;
          action.fAmount1 = 1.0f;
          OnAction(action);
          return true;
        }
        CFileItem item(message.GetStringParam(), false);
        if (item.IsPythonScript())
        { // a python script
          g_pythonParser.evalFile(item.m_strPath.c_str());
        }
        else if (item.IsXBE())
        { // an XBE
          int iRegion;
          if (g_guiSettings.GetBool("myprograms.gameautoregion"))
          {
            CXBE xbe;
            iRegion = xbe.ExtractGameRegion(item.m_strPath);
            if (iRegion < 1 || iRegion > 7)
              iRegion = 0;
            iRegion = xbe.FilterRegion(iRegion);
          }
          else
            iRegion = 0;
          CUtil::RunXBE(item.m_strPath.c_str(),NULL,F_VIDEO(iRegion));
        }
        else if (item.IsAudio() || item.IsVideo())
        { // an audio or video file
          PlayFile(item);
        }
        else
          return false;
      }
      return true;
    }
  }
  return false;
}

void CApplication::Process()
{
  // check if we need to load a new skin
  if (m_dwSkinTime && timeGetTime() >= m_dwSkinTime)
  {
    ReloadSkin();
  }

  // dispatch the messages generated by python or other threads to the current window
  m_gWindowManager.DispatchThreadMessages();

  // process messages which have to be send to the gui
  // (this can only be done after m_gWindowManager.Render())
  g_applicationMessenger.ProcessWindowMessages();

  // process any Python scripts
  g_pythonParser.Process();

  // process messages, even if a movie is playing
  g_applicationMessenger.ProcessMessages();

  // check for memory unit changes
#ifdef HAS_XBOX_HARDWARE
  if (g_memoryUnitManager.Update())
  { // changes have occured - update our shares
    CGUIMessage msg(GUI_MSG_NOTIFY_ALL,0,0,GUI_MSG_REMOVED_MEDIA);
    m_gWindowManager.SendThreadMessage(msg);
  }
#endif

  // check if we can free unused memory
  g_audioManager.FreeUnused();

  // check how far we are through playing the current item
  // and do anything that needs doing (lastfm submission, playcount updates etc)
  CheckPlayingProgress();

  // update sound
  if (m_pPlayer)
    m_pPlayer->DoAudioWork();

  // process karaoke
#ifdef HAS_KARAOKE
  if (m_pCdgParser)
    m_pCdgParser->ProcessVoice();
#endif

  // do any processing that isn't needed on each run
  if( m_slowTimer.GetElapsedMilliseconds() > 500 )
  {
    m_slowTimer.Reset();
    ProcessSlow();
  }
}

// We get called every 500ms
void CApplication::ProcessSlow()
{
  // check our network state every 10 seconds or when net status changes
  g_network.CheckNetwork(20);
  
  // check if we need 2 spin down the harddisk
  CheckNetworkHDSpinDown();
  if (!m_bNetworkSpinDown)
    CheckHDSpindown();

  // Store our file state for use on close()
  UpdateFileState();

  if (IsPlayingAudio())
  {
    CLastfmScrobbler::GetInstance()->UpdateStatus();
    CLibrefmScrobbler::GetInstance()->UpdateStatus();
  }

  // Check if we need to activate the screensaver (if enabled).
  if (g_guiSettings.GetString("screensaver.mode") != "None")
    CheckScreenSaver();

  // check if we need to shutdown (if enabled)
  if (g_guiSettings.GetInt("system.shutdowntime"))
    CheckShutdown();

  // check if we should restart the player
  CheckDelayedPlayerRestart();

  //  check if we can unload any unreferenced dlls or sections
  CSectionLoader::UnloadDelayed();

  // Xbox Autodetection - Send in X sec PingTime Interval
  if (m_gWindowManager.GetActiveWindow() != WINDOW_LOGIN_SCREEN) // sorry jm ;D
    CUtil::AutoDetection();

  // check for any idle curl connections
  g_curlInterface.CheckIdle();

  // check for any idle myth sessions
  CCMythSession::CheckIdle();

  // check for any idle htsp sessions
  HTSP::CHTSPDirectorySession::CheckIdle();

#ifdef HAS_TIME_SERVER
  // check for any needed sntp update
  if(m_psntpClient && m_psntpClient->UpdateNeeded())
    m_psntpClient->Update();
#endif

  // LED - LCD SwitchOn On Paused! m_bIsPaused=TRUE -> LED/LCD is ON!
  if(IsPaused() != m_bIsPaused)
  {
    if(g_guiSettings.GetBool("system.ledenableonpaused"))
      StartLEDControl(m_bIsPaused);
    if(g_guiSettings.GetBool("lcd.enableonpaused"))
      DimLCDOnPlayback(m_bIsPaused);
    m_bIsPaused = IsPaused();
  }

  g_largeTextureManager.CleanupUnusedImages();

  // checks whats in the DVD drive and tries to autostart the content (xbox games, dvd, cdda, avi files...)
  m_Autorun.HandleAutorun();

  // update upnp server/renderer states
  if(CUPnP::IsInstantiated())
    CUPnP::GetInstance()->UpdateState();

  //Check to see if current playing Title has changed and whether we should broadcast the fact
  CheckForTitleChange();
}

// Global Idle Time in Seconds
// idle time will be resetet if on any OnKey()
// int return: system Idle time in seconds! 0 is no idle!
int CApplication::GlobalIdleTime()
{
  if(!m_idleTimer.IsRunning())
  {
    m_idleTimer.Stop();
    m_idleTimer.StartZero();
  }
  return (int)m_idleTimer.GetElapsedSeconds();
}

float CApplication::NavigationIdleTime()
{
  if (!m_navigationTimer.IsRunning())
  {
    m_navigationTimer.Stop();
    m_navigationTimer.StartZero();
  }
  return m_navigationTimer.GetElapsedSeconds();
}

void CApplication::DelayedPlayerRestart()
{
  m_restartPlayerTimer.StartZero();
}

void CApplication::CheckDelayedPlayerRestart()
{
  if (m_restartPlayerTimer.GetElapsedSeconds() > 3)
  {
    m_restartPlayerTimer.Stop();
    m_restartPlayerTimer.Reset();
    Restart(true);
  }
}

void CApplication::Restart(bool bSamePosition)
{
  // this function gets called when the user changes a setting (like noninterleaved)
  // and which means we gotta close & reopen the current playing file

  // first check if we're playing a file
  if ( !IsPlayingVideo() && !IsPlayingAudio())
    return ;

  if( !m_pPlayer )
    return ;

  // do we want to return to the current position in the file
  if (false == bSamePosition)
  {
    // no, then just reopen the file and start at the beginning
    PlayFile(*m_itemCurrentFile, true);
    return ;
  }

  // else get current position
  double time = GetTime();

  // get player state, needed for dvd's
  CStdString state = m_pPlayer->GetPlayerState();

  // set the requested starttime
  m_itemCurrentFile->m_lStartOffset = (long)(time * 75.0);

  // reopen the file
  if ( PlayFile(*m_itemCurrentFile, true) && m_pPlayer )
    m_pPlayer->SetPlayerState(state);
}

const CStdString& CApplication::CurrentFile()
{
  return m_itemCurrentFile->m_strPath;
}

CFileItem& CApplication::CurrentFileItem()
{
  return *m_itemCurrentFile;
}

void CApplication::Mute(void)
{
  if (g_stSettings.m_bMute)
  { // muted - unmute.
    // check so we don't get stuck in some muted state
    if( g_stSettings.m_iPreMuteVolumeLevel == 0 )
      g_stSettings.m_iPreMuteVolumeLevel = 1;
    SetVolume(g_stSettings.m_iPreMuteVolumeLevel);
  }
  else
  { // mute
    g_stSettings.m_iPreMuteVolumeLevel = GetVolume();
    SetVolume(0);
  }
}

void CApplication::SetVolume(int iPercent)
{
  // convert the percentage to a mB (milliBell) value (*100 for dB)
  long hardwareVolume = (long)((float)iPercent * 0.01f * (VOLUME_MAXIMUM - VOLUME_MINIMUM) + VOLUME_MINIMUM);
  SetHardwareVolume(hardwareVolume);
  g_audioManager.SetVolume(g_stSettings.m_nVolumeLevel);
}

void CApplication::SetHardwareVolume(long hardwareVolume)
{
  // TODO DRC
  if (hardwareVolume >= VOLUME_MAXIMUM) // + VOLUME_DRC_MAXIMUM
    hardwareVolume = VOLUME_MAXIMUM;// + VOLUME_DRC_MAXIMUM;
  if (hardwareVolume <= VOLUME_MINIMUM)
  {
    hardwareVolume = VOLUME_MINIMUM;
  }
  // update our settings
  if (hardwareVolume > VOLUME_MAXIMUM)
  {
    g_stSettings.m_dynamicRangeCompressionLevel = hardwareVolume - VOLUME_MAXIMUM;
    g_stSettings.m_nVolumeLevel = VOLUME_MAXIMUM;
  }
  else
  {
    g_stSettings.m_dynamicRangeCompressionLevel = 0;
    g_stSettings.m_nVolumeLevel = hardwareVolume;
  }

  // update mute state
  if(!g_stSettings.m_bMute && hardwareVolume <= VOLUME_MINIMUM)
  {
    g_stSettings.m_bMute = true;
    if (!m_guiDialogMuteBug.IsDialogRunning())
      m_guiDialogMuteBug.Show();
  }
  else if(g_stSettings.m_bMute && hardwareVolume > VOLUME_MINIMUM)
  {
    g_stSettings.m_bMute = false;
    if (m_guiDialogMuteBug.IsDialogRunning())
      m_guiDialogMuteBug.Close();
  }

  // and tell our player to update the volume
  if (m_pPlayer)
  {
    m_pPlayer->SetVolume(g_stSettings.m_nVolumeLevel);
    // TODO DRC
//    m_pPlayer->SetDynamicRangeCompression(g_stSettings.m_dynamicRangeCompressionLevel);
  }
}

int CApplication::GetVolume() const
{
  // converts the hardware volume (in mB) to a percentage
  return int(((float)(g_stSettings.m_nVolumeLevel + g_stSettings.m_dynamicRangeCompressionLevel - VOLUME_MINIMUM)) / (VOLUME_MAXIMUM - VOLUME_MINIMUM)*100.0f + 0.5f);
}

int CApplication::GetSubtitleDelay() const
{
  // converts subtitle delay to a percentage
  return int(((float)(g_stSettings.m_currentVideoSettings.m_SubtitleDelay + g_advancedSettings.m_videoSubsDelayRange)) / (2 * g_advancedSettings.m_videoSubsDelayRange)*100.0f + 0.5f);
}

int CApplication::GetAudioDelay() const
{
  // converts subtitle delay to a percentage
  return int(((float)(g_stSettings.m_currentVideoSettings.m_AudioDelay + g_advancedSettings.m_videoAudioDelayRange)) / (2 * g_advancedSettings.m_videoAudioDelayRange)*100.0f + 0.5f);
}

void CApplication::SetPlaySpeed(int iSpeed)
{
  if (!IsPlayingAudio() && !IsPlayingVideo())
    return ;
  if (m_iPlaySpeed == iSpeed)
    return ;
  if (!m_pPlayer->CanSeek())
    return;
  if (m_pPlayer->IsPaused())
  {
    if (
      ((m_iPlaySpeed > 1) && (iSpeed > m_iPlaySpeed)) ||
      ((m_iPlaySpeed < -1) && (iSpeed < m_iPlaySpeed))
    )
    {
      iSpeed = m_iPlaySpeed; // from pause to ff/rw, do previous ff/rw speed
    }
    m_pPlayer->Pause();
  }
  m_iPlaySpeed = iSpeed;

  m_pPlayer->ToFFRW(m_iPlaySpeed);
  if (m_iPlaySpeed == 1)
  { // restore volume
    m_pPlayer->SetVolume(g_stSettings.m_nVolumeLevel);
  }
  else
  { // mute volume
    m_pPlayer->SetVolume(VOLUME_MINIMUM);
  }
}

int CApplication::GetPlaySpeed() const
{
  return m_iPlaySpeed;
}

// Returns the total time in seconds of the current media.  Fractional
// portions of a second are possible - but not necessarily supported by the
// player class.  This returns a double to be consistent with GetTime() and
// SeekTime().
double CApplication::GetTotalTime() const
{
  double rc = 0.0;

  if (IsPlaying() && m_pPlayer)
  {
    if (m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0)
      rc = (*m_currentStack)[m_currentStack->Size() - 1]->m_lEndOffset;
    else
      rc = m_pPlayer->GetTotalTime();
  }

  return rc;
}

void CApplication::ResetPlayTime()
{
  if (IsPlaying() && m_pPlayer)
    m_pPlayer->ResetTime();
}

// Returns the current time in seconds of the currently playing media.
// Fractional portions of a second are possible.  This returns a double to
// be consistent with GetTotalTime() and SeekTime().
double CApplication::GetTime() const
{
  double rc = 0.0;

  if (IsPlaying() && m_pPlayer)
  {
    if (m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0)
    {
      long startOfCurrentFile = (m_currentStackPosition > 0) ? (*m_currentStack)[m_currentStackPosition-1]->m_lEndOffset : 0;
      rc = (double)startOfCurrentFile + m_pPlayer->GetTime() * 0.001;
    }
    else
      rc = static_cast<double>(m_pPlayer->GetTime() * 0.001f);
  }

  return rc;
}

// Sets the current position of the currently playing media to the specified
// time in seconds.  Fractional portions of a second are valid.  The passed
// time is the time offset from the beginning of the file as opposed to a
// delta from the current position.  This method accepts a double to be
// consistent with GetTime() and GetTotalTime().
void CApplication::SeekTime( double dTime )
{
  if (IsPlaying() && m_pPlayer && (dTime >= 0.0))
  {
    if (!m_pPlayer->CanSeek()) return;
    if (m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0)
    {
      // find the item in the stack we are seeking to, and load the new
      // file if necessary, and calculate the correct seek within the new
      // file.  Otherwise, just fall through to the usual routine if the
      // time is higher than our total time.
      for (int i = 0; i < m_currentStack->Size(); i++)
      {
        if ((*m_currentStack)[i]->m_lEndOffset > dTime)
        {
          long startOfNewFile = (i > 0) ? (*m_currentStack)[i-1]->m_lEndOffset : 0;
          if (m_currentStackPosition == i)
            m_pPlayer->SeekTime((__int64)((dTime - startOfNewFile) * 1000.0));
          else
          { // seeking to a new file
            m_currentStackPosition = i;
            CFileItem item(*(*m_currentStack)[i]);
            item.m_lStartOffset = (long)((dTime - startOfNewFile) * 75.0);
            // don't just call "PlayFile" here, as we are quite likely called from the
            // player thread, so we won't be able to delete ourselves.
            g_applicationMessenger.PlayFile(item, true);
          }
          return;
        }
      }
    }
    // convert to milliseconds and perform seek
    m_pPlayer->SeekTime( static_cast<__int64>( dTime * 1000.0 ) );
  }
}

float CApplication::GetPercentage() const
{
  if (IsPlaying() && m_pPlayer)
  {
    if (IsPlayingAudio() && m_itemCurrentFile->HasMusicInfoTag())
    {
      const CMusicInfoTag& tag = *m_itemCurrentFile->GetMusicInfoTag();
      if (tag.GetDuration() > 0)
        return (float)(GetTime() / tag.GetDuration() * 100);
    } 
    
    if (m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0)
      return (float)(GetTime() / GetTotalTime() * 100);
    else
      return m_pPlayer->GetPercentage();
  }
  return 0.0f;
}

void CApplication::SeekPercentage(float percent)
{
  if (IsPlaying() && m_pPlayer && (percent >= 0.0))
  {
    if (!m_pPlayer->CanSeek()) return;
    if (m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0)
      SeekTime(percent * 0.01 * GetTotalTime());
    else
      m_pPlayer->SeekPercentage(percent);
  }
}

// SwitchToFullScreen() returns true if a switch is made, else returns false
bool CApplication::SwitchToFullScreen()
{
  // if playing from the video info window, close it first!
  if (m_gWindowManager.HasModalDialog() && m_gWindowManager.GetTopMostModalDialogID() == WINDOW_VIDEO_INFO)
  {
    CGUIWindowVideoInfo* pDialog = (CGUIWindowVideoInfo*)m_gWindowManager.GetWindow(WINDOW_VIDEO_INFO);
    if (pDialog) pDialog->Close(true);
  }

  // don't switch if there is a dialog on screen or the slideshow is active
  if (m_gWindowManager.HasModalDialog() || m_gWindowManager.GetActiveWindow() == WINDOW_SLIDESHOW)
    return false;

  // See if we're playing a video, and are in GUI mode
  if ( IsPlayingVideo() && m_gWindowManager.GetActiveWindow() != WINDOW_FULLSCREEN_VIDEO)
  {
    // then switch to fullscreen mode
    m_gWindowManager.ActivateWindow(WINDOW_FULLSCREEN_VIDEO);
    g_TextureManager.Flush();
    return true;
  }
  // special case for switching between GUI & visualisation mode. (only if we're playing an audio song)
  if (IsPlayingAudio() && m_gWindowManager.GetActiveWindow() != WINDOW_VISUALISATION)
  { // then switch to visualisation
    m_gWindowManager.ActivateWindow(WINDOW_VISUALISATION);
    g_TextureManager.Flush();
    return true;
  }
  return false;
}

PLAYERCOREID CApplication::GetCurrentPlayer()
{
  return m_eCurrentPlayer;
}

// when a scan is initiated, save current settings
// and enable tag reading and remote thums
void CApplication::SaveMusicScanSettings()
{
  CLog::Log(LOGINFO,"Music scan has started... Enabling tag reading, and remote thumbs");
  g_stSettings.m_bMyMusicIsScanning = true;
  g_settings.Save();
}

void CApplication::RestoreMusicScanSettings()
{
  g_stSettings.m_bMyMusicIsScanning = false;
  g_settings.Save();
}

void CApplication::CheckPlayingProgress()
{
  // check if we haven't rewound past the start of the file
  if (IsPlaying())
  {
    int iSpeed = g_application.GetPlaySpeed();
    if (iSpeed < 1)
    {
      iSpeed *= -1;
      int iPower = 0;
      while (iSpeed != 1)
      {
        iSpeed >>= 1;
        iPower++;
      }
      if (g_infoManager.GetPlayTime() / 1000 < iPower)
      {
        g_application.SetPlaySpeed(1);
        g_application.SeekTime(0);
      }
    }
  }
}

bool CApplication::ProcessAndStartPlaylist(const CStdString& strPlayList, CPlayList& playlist, int iPlaylist)
{
  CLog::Log(LOGDEBUG,"CApplication::ProcessAndStartPlaylist(%s, %i)",strPlayList.c_str(), iPlaylist);

  // initial exit conditions
  // no songs in playlist just return
  if (playlist.size() == 0)
    return false;

  // illegal playlist
  if (iPlaylist < PLAYLIST_MUSIC || iPlaylist > PLAYLIST_VIDEO)
    return false;

  // setup correct playlist
  g_playlistPlayer.ClearPlaylist(iPlaylist);

  // if the playlist contains an internet stream, this file will be used
  // to generate a thumbnail for musicplayer.cover
  g_application.m_strPlayListFile = strPlayList;

  // add the items to the playlist player
  g_playlistPlayer.Add(iPlaylist, playlist);

  // if we have a playlist
  if (g_playlistPlayer.GetPlaylist(iPlaylist).size())
  {
    // start playing it
    g_playlistPlayer.SetCurrentPlaylist(iPlaylist);
    g_playlistPlayer.Reset();
    g_playlistPlayer.Play();
    return true;
  }
  return false;
}

void CApplication::CheckForDebugButtonCombo()
{
#ifdef HAS_GAMEPAD
  ReadInput();
  if (m_DefaultGamepad.bAnalogButtons[XINPUT_GAMEPAD_X] && m_DefaultGamepad.bAnalogButtons[XINPUT_GAMEPAD_Y])
  {
    g_advancedSettings.m_logLevel = LOG_LEVEL_DEBUG_FREEMEM;
    CLog::Log(LOGINFO, "Key combination detected for full debug logging (X+Y)");
  }
#ifdef _DEBUG
  g_advancedSettings.m_logLevel = LOG_LEVEL_DEBUG_FREEMEM;
#endif
#endif
}

void CApplication::StartFtpEmergencyRecoveryMode()
{
#ifdef HAS_FTP_SERVER
  m_pFileZilla = new CXBFileZilla(NULL);
  m_pFileZilla->Start();

  // Default settings
  m_pFileZilla->mSettings.SetMaxUsers(0);
  m_pFileZilla->mSettings.SetWelcomeMessage("XBMC emergency recovery console FTP.");

  // default user
  CXFUser* pUser;
  m_pFileZilla->AddUser("xbox", pUser);
  pUser->SetPassword("xbox");
  pUser->SetShortcutsEnabled(false);
  pUser->SetUseRelativePaths(false);
  pUser->SetBypassUserLimit(false);
  pUser->SetUserLimit(0);
  pUser->SetIPLimit(0);
  pUser->AddDirectory("/", XBFILE_READ | XBFILE_WRITE | XBFILE_DELETE | XBFILE_APPEND | XBDIR_DELETE | XBDIR_CREATE | XBDIR_LIST | XBDIR_SUBDIRS | XBDIR_HOME);
  pUser->AddDirectory("C:\\", XBFILE_READ | XBFILE_WRITE | XBFILE_DELETE | XBFILE_APPEND | XBDIR_DELETE | XBDIR_CREATE | XBDIR_LIST | XBDIR_SUBDIRS);
  pUser->AddDirectory("D:\\", XBFILE_READ | XBDIR_LIST | XBDIR_SUBDIRS);
  pUser->AddDirectory("E:\\", XBFILE_READ | XBFILE_WRITE | XBFILE_DELETE | XBFILE_APPEND | XBDIR_DELETE | XBDIR_CREATE | XBDIR_LIST | XBDIR_SUBDIRS);
  pUser->AddDirectory("Q:\\", XBFILE_READ | XBFILE_WRITE | XBFILE_DELETE | XBFILE_APPEND | XBDIR_DELETE | XBDIR_CREATE | XBDIR_LIST | XBDIR_SUBDIRS);
  //Add. also Drive F/G
  if (CIoSupport::DriveExists('F')){
    pUser->AddDirectory("F:\\", XBFILE_READ | XBFILE_WRITE | XBFILE_DELETE | XBFILE_APPEND | XBDIR_DELETE | XBDIR_CREATE | XBDIR_LIST | XBDIR_SUBDIRS);
  }
  if (CIoSupport::DriveExists('G')){
    pUser->AddDirectory("G:\\", XBFILE_READ | XBFILE_WRITE | XBFILE_DELETE | XBFILE_APPEND | XBDIR_DELETE | XBDIR_CREATE | XBDIR_LIST | XBDIR_SUBDIRS);
  }
  pUser->CommitChanges();
#endif
}

void CApplication::SaveCurrentFileSettings()
{
  if (m_itemCurrentFile->IsVideo())
  {
    // save video settings
    if (g_stSettings.m_currentVideoSettings != g_stSettings.m_defaultVideoSettings)
    {
      CVideoDatabase dbs;
      dbs.Open();
      dbs.SetVideoSettings(m_itemCurrentFile->m_strPath, g_stSettings.m_currentVideoSettings);
      dbs.Close();
    }
  }
}

void CApplication::InitDirectoriesXbox()
{  
  // Set installation path. Use Q as ie. F doesn't exist yet!!!
  CStdString install_path = "Q:\\";

  // check logpath
  CStdString strLogFile, strLogFileOld;
  g_stSettings.m_logFolder = install_path;
  CUtil::AddSlashAtEnd(g_stSettings.m_logFolder);
  strLogFile.Format("%sxbmc.log", g_stSettings.m_logFolder);
  strLogFileOld.Format("%sxbmc.old.log", g_stSettings.m_logFolder);

  // Rotate the log (xbmc.log -> xbmc.old.log)
  ::DeleteFile(strLogFileOld.c_str());
  ::MoveFile(strLogFile.c_str(), strLogFileOld.c_str());

  // map our special drives to the correct drive letter
  CSpecialProtocol::SetXBMCPath(install_path);
  CSpecialProtocol::SetHomePath(install_path);
  CSpecialProtocol::SetTempPath("Z:\\");

  g_settings.m_vecProfiles.clear();
  g_settings.LoadProfiles("q:\\system\\profiles.xml");
  if (g_settings.m_vecProfiles.size() == 0)
  {
    //no profiles yet, make one based on the default settings
    CLog::Log(LOGDEBUG, "%s - Master profile is not defined, reverting to default", __FUNCTION__);
    
    CProfile profile;
    profile.setDirectory("q:\\UserData");
    profile.setName("Master user");
    profile.setLockMode(LOCK_MODE_EVERYONE);
    profile.setLockCode("");
    profile.setDate("");
    g_settings.m_vecProfiles.push_back(profile);
  }

  // First profile is always the Master Profile
  CSpecialProtocol::SetMasterProfilePath(g_settings.m_vecProfiles[0].getDirectory());
}
