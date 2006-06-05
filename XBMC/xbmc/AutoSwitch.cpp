
#include "stdafx.h"
#include "AutoSwitch.h"
#include "util.h"

#define METHOD_BYFOLDERS  0
#define METHOD_BYFILES   1
#define METHOD_BYTHUMBPERCENT 2
#define METHOD_BYFILECOUNT 3

CAutoSwitch::CAutoSwitch(void)
{}

CAutoSwitch::~CAutoSwitch(void)
{}

/// \brief Generic function to add a layer of transparency to the calling window
/// \param vecItems Vector of FileItems passed from the calling window
VIEW_METHOD CAutoSwitch::GetView(const CFileItemList &vecItems)
{
  int iSortMethod = -1;
  bool bBigThumbs = false;
  int iPercent = 0;
  int iCurrentWindow = m_gWindowManager.GetActiveWindow();
  bool bHideParentFolderItems = g_guiSettings.GetBool("filelists.hideparentdiritems");

  switch (iCurrentWindow)
  {
  case WINDOW_MUSIC_FILES:
    {
      iSortMethod = g_guiSettings.GetInt("musicfiles.autoswitchmethod");
      bBigThumbs = g_guiSettings.GetBool("musicfiles.autoswitchuselargethumbs");
      if ( iSortMethod == METHOD_BYTHUMBPERCENT )
      {
        iPercent = g_guiSettings.GetInt("musicfiles.autoswitchpercentage");
      }
    }
    break;

  case WINDOW_VIDEO_FILES:
    {
      iSortMethod = METHOD_BYTHUMBPERCENT;
      bBigThumbs = g_guiSettings.GetBool("videofiles.autoswitchuselargethumbs");
      iPercent = 50;  // 50% of thumbs -> use thumbs.
    }
    break;

  case WINDOW_PICTURES:
    {
      iSortMethod = METHOD_BYFILECOUNT;
      bBigThumbs = g_guiSettings.GetBool("pictures.autoswitchuselargethumbs");
    }
    break;

  case WINDOW_PROGRAMS:
    {
      iSortMethod = METHOD_BYTHUMBPERCENT;
      bBigThumbs = g_guiSettings.GetBool("programfiles.autoswitchuselargethumbs");
      iPercent = 50;  // 50% of thumbs -> use thumbs.
    }
    break;
  }
  // if this was called by an unknown window just return listtview
  if (iSortMethod < 0) return VIEW_METHOD_LIST;

  bool bThumbs = false;

  switch (iSortMethod)
  {
  case METHOD_BYFOLDERS:
    bThumbs = ByFolders(vecItems);
    break;

  case METHOD_BYFILES:
    bThumbs = ByFiles(bHideParentFolderItems, vecItems);
    break;

  case METHOD_BYTHUMBPERCENT:
    bThumbs = ByThumbPercent(bHideParentFolderItems, iPercent, vecItems);
    break;
  case METHOD_BYFILECOUNT:
    bThumbs = ByFileCount(vecItems);
    break;
  }

  if (bThumbs)
  {
    if (bBigThumbs)
      return VIEW_METHOD_LARGE_ICONS;
    return VIEW_METHOD_ICONS;
  }
  return VIEW_METHOD_LIST;
}

/// \brief Auto Switch method based on the current directory \e containing ALL folders and \e atleast one non-default thumb
/// \param vecItems Vector of FileItems
bool CAutoSwitch::ByFolders(const CFileItemList& vecItems)
{
  bool bThumbs = false;
  // is the list all folders?
  if (vecItems.GetFolderCount() == vecItems.Size())
  {
    // test for thumbs
    for (int i = 0; i < vecItems.Size(); i++)
    {
      const CFileItem* pItem = vecItems[i];
      if (pItem->HasThumbnail())
      {
        bThumbs = true;
        break;
      }
    }
  }
  return bThumbs;
}

/// \brief Auto Switch method based on the current directory \e not containing ALL files and \e atleast one non-default thumb
/// \param bHideParentDirItems - are we not counting the ".." item?
/// \param vecItems Vector of FileItems
bool CAutoSwitch::ByFiles(bool bHideParentDirItems, const CFileItemList& vecItems)
{
  bool bThumbs = false;
  int iCompare = 0;

  // parent directorys are visible, incrememt
  if (!bHideParentDirItems)
  {
    iCompare = 1;
  }

  // confirm the list is not just files and folderback
  if (vecItems.GetFolderCount() > iCompare)
  {
    // test for thumbs
    for (int i = 0; i < vecItems.Size(); i++)
    {
      const CFileItem* pItem = vecItems[i];
      if (pItem->HasThumbnail())
      {
        bThumbs = true;
        break;
      }
    }
  }
  return bThumbs;
}


/// \brief Auto Switch method based on the percentage of non-default thumbs \e in the current directory
/// \param iPercent Percent of non-default thumbs to autoswitch on
/// \param vecItems Vector of FileItems
bool CAutoSwitch::ByThumbPercent(bool bHideParentDirItems, int iPercent, const CFileItemList& vecItems)
{
  bool bThumbs = false;
  int iNumThumbs = 0;
  int iNumItems = vecItems.Size();
  if (!bHideParentDirItems)
  {
    iNumItems--;
  }

  if (iNumItems <= 0) return false;

  for (int i = 0; i < vecItems.Size(); i++)
  {
    const CFileItem* pItem = vecItems[i];
    if (pItem->HasThumbnail())
    {
      iNumThumbs++;
      float fTempPercent = ( (float)iNumThumbs / (float)iNumItems ) * (float)100;
      if (fTempPercent >= (float)iPercent)
      {
        bThumbs = true;
        break;
      }
    }
  }

  return bThumbs;
}

/// \brief Auto Switch method based on whether there is more than 25% files.
/// \param iPercent Percent of non-default thumbs to autoswitch on
bool CAutoSwitch::ByFileCount(const CFileItemList& vecItems)
{
  if (vecItems.Size() == 0) return false;
  float fPercent = (float)vecItems.GetFileCount() / vecItems.Size();
  return (fPercent > 0.25);
}
