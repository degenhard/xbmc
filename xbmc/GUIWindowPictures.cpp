/*
 *      Copyright (C) 2005-2008 Team XBMC
 *      http://www.xbmc.org
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
#include "GUIWindowPictures.h"
#include "Util.h"
#include "Picture.h"
#include "Application.h"
#include "GUIPassword.h"
#include "GUIDialogMediaSource.h"
#include "GUIDialogPictureInfo.h"
#include "GUIDialogProgress.h"
#include "PlayListFactory.h"
#include "FileSystem/MultiPathDirectory.h"
#include "PictureInfoLoader.h"
#include "GUIWindowManager.h"
#include "GUIDialogOK.h"
#include "FileSystem/Directory.h"
#include "FileSystem/File.h"
#include "PlayList.h"

#define CONTROL_BTNVIEWASICONS      2
#define CONTROL_BTNSORTBY           3
#define CONTROL_BTNSORTASC          4
#define CONTROL_LIST               50
#define CONTROL_THUMBS             51
#define CONTROL_LABELFILES         12

using namespace std;
using namespace MEDIA_DETECT;
using namespace XFILE;
using namespace DIRECTORY;
using namespace PLAYLIST;

#define CONTROL_BTNSLIDESHOW   6
#define CONTROL_BTNSLIDESHOW_RECURSIVE   7
#define CONTROL_SHUFFLE      9

CGUIWindowPictures::CGUIWindowPictures(void)
    : CGUIMediaWindow(WINDOW_PICTURES, "MyPics.xml")
{
  m_thumbLoader.SetObserver(this);
}

CGUIWindowPictures::~CGUIWindowPictures(void)
{
}

bool CGUIWindowPictures::OnMessage(CGUIMessage& message)
{
  switch ( message.GetMessage() )
  {
  case GUI_MSG_WINDOW_DEINIT:
    {
      if (m_thumbLoader.IsLoading())
        m_thumbLoader.StopThread();

      if (message.GetParam1() != WINDOW_SLIDESHOW)
      {
        m_ImageLib.Unload();
      }
    }
    break;

  case GUI_MSG_WINDOW_INIT:
    {
      // check for a passed destination path
      CStdString strDestination = message.GetStringParam();
      if (!strDestination.IsEmpty())
      {
        message.SetStringParam("");
        CLog::Log(LOGINFO, "Attempting to quickpath to: %s", strDestination.c_str());
        m_history.ClearPathHistory();
      }
      // otherwise, is this the first time accessing this window?
      else if (m_vecItems->m_strPath == "?")
      {
        m_vecItems->m_strPath = strDestination = g_settings.m_defaultPictureSource;
        CLog::Log(LOGINFO, "Attempting to default to: %s", strDestination.c_str());
      }

      // try to open the destination path
      if (!strDestination.IsEmpty())
      {
        // open root
        if (strDestination.Equals("$ROOT"))
        {
          m_vecItems->m_strPath = "";
          CLog::Log(LOGINFO, "  Success! Opening root listing.");
        }
        else
        {
          // default parameters if the jump fails
          m_vecItems->m_strPath = "";

          bool bIsSourceName = false;

          SetupShares();
          VECSOURCES shares;
          m_rootDir.GetSources(shares);
          int iIndex = CUtil::GetMatchingSource(strDestination, shares, bIsSourceName);
          if (iIndex > -1)
          {
            bool bDoStuff = true;
            if (iIndex < (int)shares.size() && shares[iIndex].m_iHasLock == 2)
            {
              CFileItem item(shares[iIndex]);
              if (!g_passwordManager.IsItemUnlocked(&item,"pictures"))
              {
                m_vecItems->m_strPath = ""; // no u don't
                bDoStuff = false;
                CLog::Log(LOGINFO, "  Failure! Failed to unlock destination path: %s", strDestination.c_str());
              }
            }
            // set current directory to matching share
            if (bDoStuff)
            {
              if (bIsSourceName)
                m_vecItems->m_strPath=shares[iIndex].strPath;
              else
                m_vecItems->m_strPath=strDestination;
              CUtil::RemoveSlashAtEnd(m_vecItems->m_strPath);
              CLog::Log(LOGINFO, "  Success! Opened destination path: %s", strDestination.c_str());
            }
          }
          else
          {
            CLog::Log(LOGERROR, "  Failed! Destination parameter (%s) does not match a valid share!", strDestination.c_str());
          }
        }

        // check for network up
        if (CUtil::IsRemote(m_vecItems->m_strPath) && !WaitForNetwork())
          m_vecItems->m_strPath.Empty();

        SetHistoryForPath(m_vecItems->m_strPath);
      }

      m_dlgProgress = (CGUIDialogProgress*)g_windowManager.GetWindow(WINDOW_DIALOG_PROGRESS);

      if (message.GetParam1() != WINDOW_SLIDESHOW)
      {
        m_ImageLib.Load();
      }

      if (!CGUIMediaWindow::OnMessage(message))
        return false;

      return true;
    }
    break;

  case GUI_MSG_CLICKED:
    {
      int iControl = message.GetSenderId();
      if (iControl == CONTROL_BTNSLIDESHOW) // Slide Show
      {
        OnSlideShow();
      }
      else if (iControl == CONTROL_BTNSLIDESHOW_RECURSIVE) // Recursive Slide Show
      {
        OnSlideShowRecursive();
      }
      else if (iControl == CONTROL_SHUFFLE)
      {
        g_guiSettings.ToggleBool("slideshow.shuffle");
        g_settings.Save();
      }
      else if (m_viewControl.HasControl(iControl))  // list/thumb control
      {
        int iItem = m_viewControl.GetSelectedItem();
        int iAction = message.GetParam1();

        // iItem is checked for validity inside these routines
        if (iAction == ACTION_DELETE_ITEM)
        {
          // is delete allowed?
          if (g_guiSettings.GetBool("filelists.allowfiledeletion"))
            OnDeleteItem(iItem);
          else
            return false;
        }
        else if (iAction == ACTION_PLAYER_PLAY)
        {
          ShowPicture(iItem, true);
          return true;
        }
        else if (iAction == ACTION_SHOW_INFO)
        {
          OnInfo(iItem);
          return true;
        }
      }
    }
    break;
  }
  return CGUIMediaWindow::OnMessage(message);
}

void CGUIWindowPictures::UpdateButtons()
{
  CGUIMediaWindow::UpdateButtons();

  // Update the shuffle button
  if (g_guiSettings.GetBool("slideshow.shuffle"))
  {
    CGUIMessage msg2(GUI_MSG_SELECTED, GetID(), CONTROL_SHUFFLE);
    g_windowManager.SendMessage(msg2);
  }
  else
  {
    CGUIMessage msg2(GUI_MSG_DESELECTED, GetID(), CONTROL_SHUFFLE);
    g_windowManager.SendMessage(msg2);
  }

  // check we can slideshow or recursive slideshow
  int nFolders = m_vecItems->GetFolderCount();
  if (nFolders == m_vecItems->Size())
  {
    CONTROL_DISABLE(CONTROL_BTNSLIDESHOW);
  }
  else
  {
    CONTROL_ENABLE(CONTROL_BTNSLIDESHOW);
  }
  if (m_guiState.get() && !m_guiState->HideParentDirItems())
    nFolders--;
  if (m_vecItems->Size() == 0 || nFolders == 0)
  {
    CONTROL_DISABLE(CONTROL_BTNSLIDESHOW_RECURSIVE);
  }
  else
  {
    CONTROL_ENABLE(CONTROL_BTNSLIDESHOW_RECURSIVE);
  }
}

void CGUIWindowPictures::OnPrepareFileItems(CFileItemList& items)
{
  for (int i=0;i<items.Size();++i )
    if (items[i]->GetLabel().Equals("folder.jpg"))
      items.Remove(i);

  if (items.GetFolderCount()==items.Size() || !g_guiSettings.GetBool("pictures.usetags"))
    return;

  // Start the music info loader thread
  CPictureInfoLoader loader;
  loader.SetProgressCallback(m_dlgProgress);
  loader.Load(items);

  bool bShowProgress=!g_windowManager.HasModalDialog();
  bool bProgressVisible=false;

  DWORD dwTick=timeGetTime();

  while (loader.IsLoading() && m_dlgProgress && !m_dlgProgress->IsCanceled())
  {
    if (bShowProgress)
    { // Do we have to init a progress dialog?
      DWORD dwElapsed=timeGetTime()-dwTick;

      if (!bProgressVisible && dwElapsed>1500 && m_dlgProgress)
      { // tag loading takes more then 1.5 secs, show a progress dialog
        CURL url(items.m_strPath);
        
        m_dlgProgress->SetHeading(189);
        m_dlgProgress->SetLine(0, 505);
        m_dlgProgress->SetLine(1, "");
        m_dlgProgress->SetLine(2, url.GetWithoutUserDetails());
        m_dlgProgress->StartModal();
        m_dlgProgress->ShowProgressBar(true);
        bProgressVisible = true;
      }

      if (bProgressVisible && m_dlgProgress)
      { // keep GUI alive
        m_dlgProgress->Progress();
      }
    } // if (bShowProgress)
    Sleep(1);
  } // while (loader.IsLoading())

  if (bProgressVisible && m_dlgProgress)
    m_dlgProgress->Close();
}

bool CGUIWindowPictures::Update(const CStdString &strDirectory)
{
  if (m_thumbLoader.IsLoading())
    m_thumbLoader.StopThread();

  if (!CGUIMediaWindow::Update(strDirectory))
    return false;

  m_vecItems->SetThumbnailImage("");
  if (g_guiSettings.GetBool("pictures.generatethumbs"))
    m_thumbLoader.Load(*m_vecItems);
  m_vecItems->SetCachedPictureThumb();

  return true;
}

bool CGUIWindowPictures::OnClick(int iItem)
{
  if ( iItem < 0 || iItem >= (int)m_vecItems->Size() ) return true;
  CFileItemPtr pItem = m_vecItems->Get(iItem);

  if (pItem->IsCBZ() || pItem->IsCBR())
  {
    CStdString strComicPath;
    if (pItem->IsCBZ())
      CUtil::CreateArchivePath(strComicPath, "zip", pItem->m_strPath, "");
    else
      CUtil::CreateArchivePath(strComicPath, "rar", pItem->m_strPath, "");

    OnShowPictureRecursive(strComicPath);
    return true;
  }
  else if (CGUIMediaWindow::OnClick(iItem))
    return true;

  return false;
}

bool CGUIWindowPictures::OnPlayMedia(int iItem)
{
  if (m_vecItems->Get(iItem)->IsVideo())
    return CGUIMediaWindow::OnPlayMedia(iItem);

  return ShowPicture(iItem, false);
}

bool CGUIWindowPictures::ShowPicture(int iItem, bool startSlideShow)
{
  if ( iItem < 0 || iItem >= (int)m_vecItems->Size() ) return false;
  CFileItemPtr pItem = m_vecItems->Get(iItem);
  CStdString strPicture = pItem->m_strPath;

  if (pItem->m_strPath == "add") // 'add source button' in empty root
  {
    if (CGUIDialogMediaSource::ShowAndAddMediaSource("pictures"))
    {
      Update("");
      return true;
    }
    return false;
  }

  if (pItem->IsDVD())
    return CAutorun::PlayDisc();

  if (pItem->m_bIsShareOrDrive)
    return false;

  CGUIWindowSlideShow *pSlideShow = (CGUIWindowSlideShow *)g_windowManager.GetWindow(WINDOW_SLIDESHOW);
  if (!pSlideShow)
    return false;
  if (g_application.IsPlayingVideo())
    g_application.StopPlaying();

  pSlideShow->Reset();
  for (int i = 0; i < (int)m_vecItems->Size();++i)
  {
    CFileItemPtr pItem = m_vecItems->Get(i);
    if (!pItem->m_bIsFolder && !(CUtil::IsRAR(pItem->m_strPath) || CUtil::IsZIP(pItem->m_strPath)) && pItem->IsPicture())
    {
      pSlideShow->Add(pItem.get());
    }
  }
     
  if (pSlideShow->NumSlides() == 0)
    return false; 

  pSlideShow->Select(strPicture); 

  if (startSlideShow)
    pSlideShow->StartSlideShow(false);

  g_windowManager.ActivateWindow(WINDOW_SLIDESHOW);

  return true;
}

void CGUIWindowPictures::OnShowPictureRecursive(const CStdString& strPath)
{
  CGUIWindowSlideShow *pSlideShow = (CGUIWindowSlideShow *)g_windowManager.GetWindow(WINDOW_SLIDESHOW);
  if (pSlideShow)
  {
    // stop any video
    if (g_application.IsPlayingVideo())
      g_application.StopPlaying();
    pSlideShow->AddFromPath(strPath, true);
    if (pSlideShow->NumSlides())
      g_windowManager.ActivateWindow(WINDOW_SLIDESHOW);
  }
}

void CGUIWindowPictures::OnSlideShowRecursive(const CStdString &strPicture)
{
  CGUIWindowSlideShow *pSlideShow = (CGUIWindowSlideShow *)g_windowManager.GetWindow(WINDOW_SLIDESHOW);
  if (pSlideShow)
    pSlideShow->RunSlideShow(strPicture, true, g_guiSettings.GetBool("slideshow.shuffle"));
}

void CGUIWindowPictures::OnSlideShowRecursive()
{
  CStdString strEmpty = "";
  OnSlideShowRecursive(m_vecItems->m_strPath);
}

void CGUIWindowPictures::OnSlideShow()
{
  OnSlideShow(m_vecItems->m_strPath);
}

void CGUIWindowPictures::OnSlideShow(const CStdString &strPicture)
{
  CGUIWindowSlideShow *pSlideShow = (CGUIWindowSlideShow *)g_windowManager.GetWindow(WINDOW_SLIDESHOW);
  if (pSlideShow)
    pSlideShow->RunSlideShow(strPicture);
}

void CGUIWindowPictures::OnRegenerateThumbs()
{
  if (m_thumbLoader.IsLoading()) return;
  m_thumbLoader.SetRegenerateThumbs(true);
  m_thumbLoader.Load(*m_vecItems);
}

void CGUIWindowPictures::GetContextButtons(int itemNumber, CContextButtons &buttons)
{
  CFileItemPtr item;
  if (itemNumber >= 0 && itemNumber < m_vecItems->Size())
    item = m_vecItems->Get(itemNumber);

  if (item && !item->GetPropertyBOOL("pluginreplacecontextitems"))
  {
    if ( m_vecItems->IsVirtualDirectoryRoot() && item)
    {
      CGUIDialogContextMenu::GetContextButtons("pictures", item, buttons);
    }
    else
    {
      if (item)
      {
        if (!(item->m_bIsFolder || item->IsZIP() || item->IsRAR() || item->IsCBZ() || item->IsCBR()))
          buttons.Add(CONTEXT_BUTTON_INFO, 13406); // picture info
        buttons.Add(CONTEXT_BUTTON_VIEW_SLIDESHOW, item->m_bIsFolder ? 13317 : 13422);      // View Slideshow
        if (item->m_bIsFolder)
          buttons.Add(CONTEXT_BUTTON_RECURSIVE_SLIDESHOW, 13318);     // Recursive Slideshow

        if (!m_thumbLoader.IsLoading())
          buttons.Add(CONTEXT_BUTTON_REFRESH_THUMBS, 13315);         // Create Thumbnails
        if (g_guiSettings.GetBool("filelists.allowfiledeletion") && !item->IsReadOnly())
        {
          buttons.Add(CONTEXT_BUTTON_DELETE, 117);
          buttons.Add(CONTEXT_BUTTON_RENAME, 118);
        }
      }
      buttons.Add(CONTEXT_BUTTON_GOTO_ROOT, 20128);
      buttons.Add(CONTEXT_BUTTON_SWITCH_MEDIA, 523);
    }
  }
  CGUIMediaWindow::GetContextButtons(itemNumber, buttons);
  if (item && !item->GetPropertyBOOL("pluginreplacecontextitems"))
    buttons.Add(CONTEXT_BUTTON_SETTINGS, 5);                  // Settings
}

bool CGUIWindowPictures::OnContextButton(int itemNumber, CONTEXT_BUTTON button)
{
  CFileItemPtr item = (itemNumber >= 0 && itemNumber < m_vecItems->Size()) ? m_vecItems->Get(itemNumber) : CFileItemPtr();
  if (m_vecItems->IsVirtualDirectoryRoot() && item)
  {
    if (CGUIDialogContextMenu::OnContextButton("pictures", item, button))
    {
      Update("");
      return true;
    }
  }
  switch (button)
  {
  case CONTEXT_BUTTON_VIEW_SLIDESHOW:
    if (item && item->m_bIsFolder)
    OnSlideShow(item->m_strPath);
    else
      ShowPicture(itemNumber, true);
    return true;
  case CONTEXT_BUTTON_RECURSIVE_SLIDESHOW:
    if (item)
    OnSlideShowRecursive(item->m_strPath);
    return true;
  case CONTEXT_BUTTON_INFO:
    OnInfo(itemNumber);
    return true;
  case CONTEXT_BUTTON_REFRESH_THUMBS:
    OnRegenerateThumbs();
    return true;
  case CONTEXT_BUTTON_DELETE:
    OnDeleteItem(itemNumber);
    return true;
  case CONTEXT_BUTTON_RENAME:
    OnRenameItem(itemNumber);
    return true;
  case CONTEXT_BUTTON_SETTINGS:
    g_windowManager.ActivateWindow(WINDOW_SETTINGS_MYPICTURES);
    return true;
  case CONTEXT_BUTTON_GOTO_ROOT:
    Update("");
    return true;
  case CONTEXT_BUTTON_SWITCH_MEDIA:
    CGUIDialogContextMenu::SwitchMedia("pictures", m_vecItems->m_strPath);
    return true;
  default:
    break;
  }
  return CGUIMediaWindow::OnContextButton(itemNumber, button);
}

void CGUIWindowPictures::OnItemLoaded(CFileItem *pItem)
{
  if (pItem->IsCBR() || pItem->IsCBZ())
  {
    CStdString strTBN(CUtil::ReplaceExtension(pItem->m_strPath,".tbn"));
    if (CFile::Exists(strTBN))
    {
      CPicture pic;
      if (pic.CreateThumbnail(strTBN, pItem->GetCachedPictureThumb(),true))
      {
        pItem->SetCachedPictureThumb();
        pItem->FillInDefaultIcon();
        return;
      }
    }
  }
  if ((pItem->m_bIsFolder || pItem->IsCBR() || pItem->IsCBZ()) && !pItem->m_bIsShareOrDrive && !pItem->HasThumbnail() && !pItem->IsParentFolder())
  {
    // first check for a folder.jpg
    CStdString thumb = "folder.jpg";
    CStdString strPath = pItem->m_strPath;
    if (pItem->IsCBR())
    {
      CUtil::CreateArchivePath(strPath,"rar",pItem->m_strPath,"");
      thumb = "cover.jpg";
    }
    if (pItem->IsCBZ())
    {
      CUtil::CreateArchivePath(strPath,"zip",pItem->m_strPath,"");
      thumb = "cover.jpg";
    }
    if (pItem->IsMultiPath())
      strPath = CMultiPathDirectory::GetFirstPath(pItem->m_strPath);
    thumb = CUtil::AddFileToFolder(strPath, thumb);
    if (CFile::Exists(thumb))
    {
      CPicture pic;
      pic.CreateThumbnail(thumb, pItem->GetCachedPictureThumb(),true);
    }
    else if (!pItem->IsPlugin())
    {
      // we load the directory, grab 4 random thumb files (if available) and then generate
      // the thumb.

      CFileItemList items;

      CDirectory::GetDirectory(strPath, items, g_stSettings.m_pictureExtensions, false, false);

      // create the folder thumb by choosing 4 random thumbs within the folder and putting
      // them into one thumb.
      // count the number of images
      for (int i=0; i < items.Size();)
      {
        if (!items[i]->IsPicture() || items[i]->IsZIP() || items[i]->IsRAR() || items[i]->IsPlayList())
        {
          items.Remove(i);
        }
        else
          i++;
      }

      if (items.IsEmpty())
      {
        if (pItem->IsCBZ() || pItem->IsCBR())
        {
          CDirectory::GetDirectory(strPath, items, g_stSettings.m_pictureExtensions, false, false);
          for (int i=0;i<items.Size();++i)
          {
            CFileItemPtr item = items[i];
            if (item->m_bIsFolder)
            {
              OnItemLoaded(item.get());
              pItem->SetThumbnailImage(items[i]->GetThumbnailImage());
              pItem->SetIconImage(items[i]->GetIconImage());
              return;
            }
          }
        }
        return; // no images in this folder
      }

      // randomize them
      items.Randomize();

      if (items.Size() < 4 || pItem->IsCBR() || pItem->IsCBZ())
      { // less than 4 items, so just grab the first thumb
        CStdString folderThumb(pItem->GetCachedPictureThumb());
        items.Sort(SORT_METHOD_LABEL, SORT_ORDER_ASC);
        CPicture pic;
        pic.CreateThumbnail(items[0]->m_strPath, folderThumb);
      }
      else
      {
        // ok, now we've got the files to get the thumbs from, lets create it...
        // we basically load the 4 thumbs, resample to 62x62 pixels, and add them
        CStdString strFiles[4];
        for (int thumb = 0; thumb < 4; thumb++)
          strFiles[thumb] = items[thumb]->m_strPath;
        CPicture pic;
        pic.CreateFolderThumb(strFiles, pItem->GetCachedPictureThumb());
      }
    }
    // refill in the icon to get it to update
    pItem->SetCachedPictureThumb();
    pItem->FillInDefaultIcon();
  }
}

void CGUIWindowPictures::LoadPlayList(const CStdString& strPlayList)
{
  CLog::Log(LOGDEBUG,"CGUIWindowPictures::LoadPlayList()... converting playlist into slideshow: %s", strPlayList.c_str());
  auto_ptr<CPlayList> pPlayList (CPlayListFactory::Create(strPlayList));
  if ( NULL != pPlayList.get())
  {
    if (!pPlayList->Load(strPlayList))
    {
      CGUIDialogOK::ShowAndGetInput(6, 0, 477, 0);
      return ; //hmmm unable to load playlist?
    }
  }

  CPlayList playlist = *pPlayList;
  if (playlist.size() > 0)
  {
    // set up slideshow
    CGUIWindowSlideShow *pSlideShow = (CGUIWindowSlideShow *)g_windowManager.GetWindow(WINDOW_SLIDESHOW);
    if (!pSlideShow)
      return;
    if (g_application.IsPlayingVideo())
      g_application.StopPlaying();

    // convert playlist items into slideshow items
    pSlideShow->Reset();
    for (int i = 0; i < (int)playlist.size(); ++i)
    {
      CFileItemPtr pItem = playlist[i];
      //CLog::Log(LOGDEBUG,"-- playlist item: %s", pItem->m_strPath.c_str());
      if (pItem->IsPicture() && !(pItem->IsZIP() || pItem->IsRAR() || pItem->IsCBZ() || pItem->IsCBR()))
        pSlideShow->Add(pItem.get());
    }

    // start slideshow if there are items
    pSlideShow->StartSlideShow();
    if (pSlideShow->NumSlides())
      g_windowManager.ActivateWindow(WINDOW_SLIDESHOW);
  }
}

void CGUIWindowPictures::OnInfo(int itemNumber)
{
  CFileItemPtr item = (itemNumber >= 0 && itemNumber < m_vecItems->Size()) ? m_vecItems->Get(itemNumber) : CFileItemPtr();
  if (!item || item->m_bIsFolder || item->IsZIP() || item->IsRAR() || item->IsCBZ() || item->IsCBR())
    return;
  CGUIDialogPictureInfo *pictureInfo = (CGUIDialogPictureInfo *)g_windowManager.GetWindow(WINDOW_DIALOG_PICTURE_INFO);
  if (pictureInfo)
  {
    pictureInfo->SetPicture(item.get());
    pictureInfo->DoModal();
  }
}

