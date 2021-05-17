#include "Database.h"

wxDEFINE_EVENT( EVT_DB_ERROR, wxThreadEvent );

Database::Database() : m_dbImpl( nullptr )
{
}

Database::~Database()
{
	if ( m_dbImpl == nullptr )
	{
		sqlite3_close( m_dbImpl );
		m_dbImpl = nullptr;
	}
}

bool Database::Create( const char* connString )
{
	if ( SQLITE_OK != sqlite3_open( connString, & m_dbImpl ) )
	{
		ReportError( "db cannot be created." );
		return false;
	}

	char sqlCreateTableQuery[] =
		"CREATE TABLE IF NOT EXISTS CPU_STATE("
		"idx INT IDENTITY( 1, 1 ),"
		"RecTime DATETIME,"
		"Usage  INTEGER,"
		"Temperature INTEGER,"
		"MemUsage INTEGER"
		");";

	int sqlResult = sqlite3_exec( m_dbImpl,
		sqlCreateTableQuery,
		nullptr, nullptr, nullptr );
	if ( SQLITE_OK != sqlResult )
	{
		sqlite3_close( m_dbImpl );
		ReportError( "db cannot be created." );
		return false;
	}
	return false;
}

bool Database::ExecSql( const char* sql )
{
	if ( m_dbImpl == nullptr )
	{
		return false;
	}
	int sqlResult = sqlite3_exec( m_dbImpl,
		sql,
		nullptr, nullptr, nullptr );

	return (SQLITE_OK == sqlResult);
}

void Database::ReportError( const char* error )
{
	wxThreadEvent  errorEvent( EVT_DB_ERROR );
	errorEvent.SetString( error );
	//wxQueueEvent( m_msgHandler, errorEvent.Clone() );
}
