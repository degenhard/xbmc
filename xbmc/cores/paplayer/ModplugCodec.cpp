/*
 *      Copyright (C) 2009 Team XBMC
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

#include "ModplugCodec.h"
#include "Util.h"
#include "FileSystem/File.h"
#include "utils/log.h"

#include <cstdlib>
#include <cstdio>

using namespace XFILE;

ModplugCodec::ModplugCodec()
{
  m_CodecName = "MOD";
  m_module = NULL;
}

ModplugCodec::~ModplugCodec()
{
  DeInit();
}

bool ModplugCodec::Init(const CStdString &strFile, unsigned int filecache)
{
  DeInit();

  if (!m_dll.Load())
    return false;

  // set correct codec name
  CUtil::GetExtension(strFile,m_CodecName);
  m_CodecName.erase(0,1);
  m_CodecName.ToUpper();

  // Read our file to memory so it can be passed to ModPlug_Load()
  CFile file;
  if (!file.Open(strFile))
  {
    CLog::Log(LOGERROR,"ModplugCodec: error opening file %s!",strFile.c_str());
    return false;
  }
  char *data = new char[file.GetLength()];
  file.Read(data,file.GetLength());

  // Now load the module
  m_module = m_dll.ModPlug_Load(data,file.GetLength());
  delete[] data;
  if (!m_module)
  {
    CLog::Log(LOGERROR,"ModplugCodec: error loading module file %s!",strFile.c_str());
    return false;
  }
  m_Channels = 2;
  m_SampleRate = 44100;
  m_BitsPerSample = 16;
  m_TotalTime = (__int64)(m_dll.ModPlug_GetLength(m_module))/1000;

  return true;
}

void ModplugCodec::DeInit()
{
  if (m_module)
    m_dll.ModPlug_Unload(m_module);
  m_module = NULL;
}

__int64 ModplugCodec::Seek(__int64 iSeekTime)
{
  m_dll.ModPlug_Seek(m_module, (int)(iSeekTime/1000));
  return iSeekTime;
}

int ModplugCodec::ReadPCM(BYTE *pBuffer, int size, int *actualsize)
{
  if (!m_module)
    return READ_ERROR;

  if ((*actualsize = m_dll.ModPlug_Read(m_module, pBuffer, size)) == size)
    return READ_SUCCESS;
  
  return READ_ERROR;
}

bool ModplugCodec::CanInit()
{
  return m_dll.CanLoad();
}
