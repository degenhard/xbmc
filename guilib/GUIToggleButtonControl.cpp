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

#include "include.h"
#include "GUIToggleButtonControl.h"
#include "GUIWindowManager.h"
#include "GUIDialog.h"
#include "utils/CharsetConverter.h"
#include "utils/GUIInfoManager.h"

using namespace std;

CGUIToggleButtonControl::CGUIToggleButtonControl(DWORD dwParentID, DWORD dwControlId, float posX, float posY, float width, float height, const CImage& textureFocus, const CImage& textureNoFocus, const CImage& altTextureFocus, const CImage& altTextureNoFocus, const CLabelInfo &labelInfo)
    : CGUIButtonControl(dwParentID, dwControlId, posX, posY, width, height, textureFocus, textureNoFocus, labelInfo)
    , m_selectButton(dwParentID, dwControlId, posX, posY, width, height, altTextureFocus, altTextureNoFocus, labelInfo)
{
  m_toggleSelect = 0;
  ControlType = GUICONTROL_TOGGLEBUTTON;
}

CGUIToggleButtonControl::~CGUIToggleButtonControl(void)
{
}

void CGUIToggleButtonControl::Render()
{
  // ask our infoManager whether we are selected or not...
  if (m_toggleSelect)
    m_bSelected = g_infoManager.GetBool(m_toggleSelect, m_dwParentID);

  if (m_bSelected)
  {
    // render our Alternate textures...
    m_selectButton.SetFocus(HasFocus());
    m_selectButton.SetVisible(IsVisible());
    m_selectButton.SetEnabled(!IsDisabled());
    m_selectButton.SetPulseOnSelect(m_pulseOnSelect);
    m_selectButton.Render();
    CGUIControl::Render();
  }
  else
  { // render our Normal textures...
    CGUIButtonControl::Render();
  }
}

bool CGUIToggleButtonControl::OnAction(const CAction &action)
{
  if (action.wID == ACTION_SELECT_ITEM)
  {
    m_bSelected = !m_bSelected;
  }
  return CGUIButtonControl::OnAction(action);
}

void CGUIToggleButtonControl::PreAllocResources()
{
  CGUIButtonControl::PreAllocResources();
  m_selectButton.PreAllocResources();
}

void CGUIToggleButtonControl::AllocResources()
{
  CGUIButtonControl::AllocResources();
  m_selectButton.AllocResources();
}

void CGUIToggleButtonControl::FreeResources()
{
  CGUIButtonControl::FreeResources();
  m_selectButton.FreeResources();
}

void CGUIToggleButtonControl::DynamicResourceAlloc(bool bOnOff)
{
  CGUIButtonControl::DynamicResourceAlloc(bOnOff);
  m_selectButton.DynamicResourceAlloc(bOnOff);
}

void CGUIToggleButtonControl::Update()
{
  CGUIButtonControl::Update();
  m_selectButton.Update();
}

void CGUIToggleButtonControl::SetPosition(float posX, float posY)
{
  CGUIButtonControl::SetPosition(posX, posY);
  m_selectButton.SetPosition(posX, posY);
}

void CGUIToggleButtonControl::SetWidth(float width)
{
  CGUIButtonControl::SetWidth(width);
  m_selectButton.SetWidth(width);
}

void CGUIToggleButtonControl::SetHeight(float height)
{
  CGUIButtonControl::SetHeight(height);
  m_selectButton.SetHeight(height);
}

void CGUIToggleButtonControl::SetColorDiffuse(const CGUIInfoColor &color)
{
  CGUIButtonControl::SetColorDiffuse(color);
  m_selectButton.SetColorDiffuse(color);
}

void CGUIToggleButtonControl::SetLabel(const string &strLabel)
{
  CGUIButtonControl::SetLabel(strLabel);
  m_selectButton.SetLabel(strLabel);
}

void CGUIToggleButtonControl::SetAltLabel(const string &label)
{
  if (label.size())
    m_selectButton.SetLabel(label);
}

CStdString CGUIToggleButtonControl::GetLabel() const
{
  if (m_bSelected)
    return m_selectButton.GetLabel();
  return CGUIButtonControl::GetLabel();
}

void CGUIToggleButtonControl::SetAltClickActions(const vector<CStdString> &clickActions)
{
  m_selectButton.SetClickActions(clickActions);
}

void CGUIToggleButtonControl::OnClick()
{
  // the ! is here as m_bSelected gets updated before this is called
  if (!m_bSelected && m_selectButton.HasClickActions())
    m_selectButton.OnClick();
  else
    CGUIButtonControl::OnClick();
}
