#pragma once

/*
 *      Copyright (C) 2005-2009 Team XBMC
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

#include <string>
#include <set>
#include <vector>
#include "URL.h"

//forwards
class CCriticalSection;

/// this class provides support for zeroconf browsing
class CZeroconfBrowser
{
public:
  class ZeroconfService
  {
    public:
      ZeroconfService();
      ZeroconfService(const CStdString& fcr_name, const CStdString& fcr_type, const CStdString& fcr_domain);

      /// easy conversion to string and back (used in czeronfdiretory to store this service)
      ///@{
      static CStdString toPath(const ZeroconfService& fcr_service);
      static ZeroconfService fromPath(const CStdString& fcr_string);
      ///@}

      /// general access methods
      ///@{
      void SetName(const CStdString& fcr_name);
      const CStdString& GetName() const {return m_name;}

      void SetType(const CStdString& fcr_type);
      const CStdString& GetType() const {return m_type;}

      void SetDomain(const CStdString& fcr_domain);
      const CStdString& GetDomain() const {return m_domain;}
      ///@}

      /// access methods needed during resolve
      ///@{
      void SetIP(const CStdString& fcr_ip);
      const CStdString& GetIP() const {return m_ip;}

      void SetPort(int f_port);
      int GetPort() const {return m_port;}
      ///@}
    private:
      //3 entries below identify a service
      CStdString m_name;
      CStdString m_type;
      CStdString m_domain;

      //2 entries below store 1 ip:port pair for this service
      CStdString m_ip;
      int        m_port;
  };

  // starts browsing
  void Start();

  // stops browsing
  void Stop();
  
  /// methods for browsing and getting results of it
  ///@{
  /// adds a service type for browsing
  /// @param fcr_service_type the service type as string, e.g. _smb._tcp.
  /// @return false if it was already there
  bool AddServiceType(const CStdString& fcr_service_type);

  /// remove the specified service from discovery
  /// @param fcr_service_type the service type as string, e.g. _smb._tcp.
  /// @return if it was not found
  bool RemoveServiceType(const CStdString& fcr_service_type);

  ///returns the list of found services
  /// if this is updated, the following message with "zeroconf://" as path is sent:
  /// CGUIMessage message(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_UPDATE_PATH);
  std::vector<ZeroconfService> GetFoundServices();
  ///@}
  
  // resolves a ZeroconfService to ip + port
  // @param fcr_service the service to resolve
  // @param f_timeout timeout in seconds for resolving
  //   the protocol part of CURL is the raw zeroconf service type
  //   added with AddServiceType (== needs further processing! e.g. _smb._tcp -> smb)
  // @return true if it was successfully resolved (or scheduled), false if resolve
  //         failed (async or not)
  bool ResolveService(ZeroconfService& fr_service, double f_timeout = 1.0);

  // class methods
  // access to singleton; singleton gets created on call if not existent
  // if zeroconf is disabled (!HAS_ZEROCONF), this will return a dummy implementation that
  // just does nothings, otherwise the platform specific one
  static CZeroconfBrowser* GetInstance();
  // release the singleton; (save to call multiple times)
  static void ReleaseInstance();
  // returns false if ReleaseInstance() was called befores
  static bool IsInstantiated() { return  smp_instance != 0; }

protected:
  // pure virtual methods to implement for OS specific implementations
  virtual bool doAddServiceType(const CStdString& fcr_service_type) = 0;
  virtual bool doRemoveServiceType(const CStdString& fcr_service_type) = 0;
  virtual std::vector<ZeroconfService> doGetFoundServices() = 0;
  virtual bool doResolveService(ZeroconfService& fr_service, double f_timeout) = 0;

protected:
  //singleton: we don't want to get instantiated nor copied or deleted from outside
  CZeroconfBrowser();
  CZeroconfBrowser(const CZeroconfBrowser&);
  virtual ~CZeroconfBrowser();

private:
  struct ServiceInfo
  {
    CStdString type;
  };

  //protects data
  CCriticalSection* mp_crit_sec;
  typedef std::set<CStdString> tServices;
  tServices m_services;
  bool m_started;

  //protects singleton creation/destruction
  static long sm_singleton_guard;
  static CZeroconfBrowser* smp_instance;
};


//inline methods

inline bool operator<(CZeroconfBrowser::ZeroconfService const& fcr_lhs, CZeroconfBrowser::ZeroconfService const& fcr_rhs)
{
  if(fcr_lhs.GetName() < fcr_rhs.GetName() )
    return true;
  else if(fcr_lhs.GetType() < fcr_rhs.GetType() )
    return true;
  else if(fcr_lhs.GetDomain() < fcr_rhs.GetDomain() )
    return true;
  else
    return false;
}

inline bool operator==(CZeroconfBrowser::ZeroconfService const& fcr_lhs, CZeroconfBrowser::ZeroconfService const& fcr_rhs)
{
  return (fcr_lhs.GetName() == fcr_rhs.GetName() && fcr_lhs.GetType() == fcr_rhs.GetType() && fcr_lhs.GetDomain() == fcr_rhs.GetDomain() );
}