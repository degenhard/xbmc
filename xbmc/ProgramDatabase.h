#pragma once
#include "lib/sqlLite/sqlitedataset.h"
#include "StdString.h"
#include "FileItem.h"
#include <vector>
#include <memory>
using namespace std;
using namespace dbiplus;

typedef vector<CStdString> VECPROGRAMPATHS;

#define COMPARE_PERCENTAGE     0.90f // 90%
#define COMPARE_PERCENTAGE_MIN 0.50f // 50%


class CProgramDatabase
{
public:
  CProgramDatabase(void);
  virtual ~CProgramDatabase(void);
  bool    Open() ;
  void	  Close() ;
  long    AddProgram(const CStdString& strFilenameAndPath, const CStdString& strDescription, const CStdString& strBookmark);
  long	  GetFile(const CStdString& strFilenameAndPath, VECFILEITEMS& programs);
  void	  GetProgramsByBookmark(CStdString& strBookmark, VECFILEITEMS& programs, bool bOnlyDefaultXBE);
  void	  GetProgramsByPath(const CStdString& strPath, VECFILEITEMS& programs, int idepth, bool bOnlyDefaultXBE);
  bool    IncTimesPlayed(const CStdString& strFileName1);
  bool	  EntryExists(const CStdString& strPath, const CStdString& strBookmark);
protected:
  auto_ptr<SqliteDatabase>  m_pDB;
	auto_ptr<Dataset>				  m_pDS;


  long    AddPath(const CStdString& strPath);
  long    AddFile(long lPathId, const CStdString& strFileName, const CStdString& strDescription);

  long	  AddBookMark(const CStdString& strBookmark);
  long    GetProgram(long lPathId);
  long    GetPath(const CStdString& strPath);

  void    Split(const CStdString& strFileNameAndPath, CStdString& strPath, CStdString& strFileName);
  void    RemoveInvalidChars(CStdString& strTxt);
  bool	  CreateTables();
  void    DeleteProgram(const CStdString& strPath);
  void	  DeleteFile(long lFileId);
};
