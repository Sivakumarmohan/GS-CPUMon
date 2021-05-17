#pragma once
#include "sqlite/sqlite3.h"
#include <wx/wx.h>

wxDECLARE_EVENT( EVT_DB_ERROR, wxThreadEvent );

class Database
{
public:
	Database();
	~Database();

	bool Create( const char * connString );
	bool ExecSql( const char * sql );

protected:
	sqlite3 * m_dbImpl;

	void ReportError( const char* error );
};

