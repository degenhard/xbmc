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


#include "AddonsDirectory.h"
#include "AddonDatabase.h"
#include "FactoryDirectory.h"
#include "Directory.h"
#include "DirectoryCache.h"
#include "FileItem.h"
#include "addons/Repository.h"
#include "addons/PluginSource.h"
#include "StringUtils.h"

using namespace ADDON;

namespace XFILE
{

CAddonsDirectory::CAddonsDirectory(void)
{
  m_allowPrompting = true;
  m_cacheDirectory = DIR_CACHE_ONCE;
}

CAddonsDirectory::~CAddonsDirectory(void)
{}

bool CAddonsDirectory::GetDirectory(const CStdString& strPath, CFileItemList &items)
{
  CStdString path1(strPath);
  CUtil::RemoveSlashAtEnd(path1);
  CURL path(path1);
  items.ClearProperties();

  VECADDONS addons;
  // get info from repository
  if (path.GetHostName().Equals("enabled") || path.GetHostName().Equals("disabled"))
  {
    CAddonMgr::Get().GetAllAddons(addons, path.GetHostName().Equals("enabled"));
    items.SetProperty("reponame",g_localizeStrings.Get(24062));
  }
  else if (path.GetHostName().Equals("repos"))
  {
    CAddonMgr::Get().GetAddons(ADDON_REPOSITORY,addons,true);
  }
  else if (path.GetHostName().Equals("sources"))
  {
    return GetScriptsAndPlugins(path.GetFileName(), items);
  }
  else if (path.GetHostName().Equals("all"))
  {
    CAddonDatabase database;
    database.Open();
    database.GetAddons(addons);
    items.SetProperty("reponame",g_localizeStrings.Get(24032));
  }
  else
  {
    AddonPtr addon;
    CAddonMgr::Get().GetAddon(path.GetHostName(),addon);
    if (!addon)
      return false;
    CAddonDatabase database;
    database.Open();
    if (!database.GetRepository(addon->ID(),addons))
    {
      RepositoryPtr repo = boost::dynamic_pointer_cast<CRepository>(addon);
      addons = CRepositoryUpdateJob::GrabAddons(repo,false);
    }
    items.SetProperty("reponame",addon->Name());
  }

  if (path.GetFileName().IsEmpty())
  {
    if (!path.GetHostName().Equals("repos"))
    {
      for (int i=ADDON_UNKNOWN+1;i<ADDON_VIZ_LIBRARY;++i)
      {
        for (unsigned int j=0;j<addons.size();++j)
        {
          if (addons[j]->Type() == (TYPE)i)
          {
            CFileItemPtr item(new CFileItem(TranslateType((TYPE)i,true)));
            item->m_strPath = CUtil::AddFileToFolder(strPath,TranslateType((TYPE)i,false));
            item->m_bIsFolder = true;
            items.Add(item);
            break;
          }
        }
      }
      items.m_strPath = strPath;
      return true;
    }
  }
  else
  {
    TYPE type = TranslateType(path.GetFileName());
    items.SetProperty("addoncategory",TranslateType(type, true));
    items.m_strPath = strPath;

    // FIXME: Categorisation of addons needs adding here
    for (unsigned int j=0;j<addons.size();++j)
    {
      if (addons[j]->Type() != type)
        addons.erase(addons.begin()+j--);
    }
  }

  items.m_strPath = strPath;
  GenerateListing(path, addons, items);
  // check for available updates
  if (path.GetHostName().Equals("enabled"))
  {
    CAddonDatabase database;
    database.Open();
    for (int i=0;i<items.Size();++i)
    {
      AddonPtr addon2;
      database.GetAddon(items[i]->GetProperty("Addon.ID"),addon2);
      if (addon2 && addon2->Version() > AddonVersion(items[i]->GetProperty("Addon.Version")))
      {
        items[i]->SetProperty("Addon.Status",g_localizeStrings.Get(24068));
        items[i]->SetProperty("Addon.UpdateAvail","true");
      }
    }
  }
  if (path.GetHostName().Equals("repos"))
  {
    CFileItemPtr item(new CFileItem("addons://all/",true));
    item->SetLabel(g_localizeStrings.Get(24032));
    items.Add(item);
  }

  return true;
}

void CAddonsDirectory::GenerateListing(CURL &path, VECADDONS& addons, CFileItemList &items)
{
  items.ClearItems();
  for (unsigned i=0; i < addons.size(); i++)
  {
    AddonPtr addon = addons[i];
    CFileItemPtr pItem = (addon->Type() == ADDON_REPOSITORY) ? FileItemFromAddon(addon, "addons://", true)
                                                             : FileItemFromAddon(addon, path.Get(), false);
    AddonPtr addon2;
    if (CAddonMgr::Get().GetAddon(addon->ID(),addon2))
      pItem->SetProperty("Addon.Status",g_localizeStrings.Get(305));
    else if (pItem->GetProperty("Addon.Path").Left(22).Equals("special://xbmc/addons/"))
      pItem->SetProperty("Addon.Status",g_localizeStrings.Get(24095));
    items.Add(pItem);
  }
}

CFileItemPtr CAddonsDirectory::FileItemFromAddon(AddonPtr &addon, const CStdString &basePath, bool folder)
{
  if (!addon)
    return CFileItemPtr();

  // TODO: This can probably be done more efficiently
  CURL url(basePath);
  url.SetFileName(addon->ID());
  CStdString path(url.Get());
  if (folder)
    CUtil::AddSlashAtEnd(path);

  CFileItemPtr item(new CFileItem(path, folder));
  item->SetLabel(addon->Name());
  item->SetLabel2(addon->Summary());
  item->SetThumbnailImage(addon->Icon());
  item->SetIconImage("DefaultAddon.png");
  item->SetProperty("fanart_image", addon->FanArt());
  CAddonDatabase::SetPropertiesFromAddon(addon, item);
  return item;
}

bool CAddonsDirectory::GetScriptsAndPlugins(const CStdString &content, CFileItemList &items)
{
  items.Clear();

  CPluginSource::Content type = CPluginSource::Translate(content);
  if (type == CPluginSource::UNKNOWN)
    return false;

  VECADDONS addons;
  CAddonMgr::Get().GetAddons(ADDON_PLUGIN, addons);
  for (unsigned i=0; i<addons.size(); i++)
  {
    PluginPtr plugin = boost::dynamic_pointer_cast<CPluginSource>(addons[i]);
    if (!plugin || !plugin->Provides(type))
      continue;
    items.Add(FileItemFromAddon(addons[i], "plugin://", true));
  }

  addons.clear();
  CAddonMgr::Get().GetAddons(ADDON_SCRIPT, addons);
  for (unsigned i=0; i<addons.size(); i++)
  {
    PluginPtr plugin = boost::dynamic_pointer_cast<CPluginSource>(addons[i]);
    if (!plugin || !plugin->Provides(type))
      continue;
    items.Add(FileItemFromAddon(addons[i], "script://", false));
  }
  if (items.Size() == 0)
  {
    CFileItemPtr item(new CFileItem("addons://more",false));
    item->SetLabelPreformated(true);
    item->SetLabel(g_localizeStrings.Get(21452));
    item->SetThumbnailImage("DefaultAddon.png");
    items.Add(item);
  }
  return items.Size() > 0;
}

}

