#include <mosquitto.h>
#include <wx/tokenzr.h>
#include "version.h"

#ifdef __WXMSW__
#include "sqlite/sqlite3.h"
#else
#include <sqlite3.h>
#endif

#include "CpuMonApp.h"
#ifdef __WXGTK__
#include "tbicon.xpm"
#endif

using namespace std;

const char g_Topic[] = "MACHINE_STATUS";

class tmpFrame : public wxFrame
{
public:
	tmpFrame() : wxFrame( nullptr, wxID_ANY, _T( "MQTT" ) )
	{
		Bind( wxEVT_CLOSE_WINDOW, & tmpFrame::OnClose, this );
	}
	void OnClose( wxCloseEvent & closeEvent )
	{
		wxGetApp().Quit();
		closeEvent.Skip();
	}
};

IMPLEMENT_APP( CpuMonApp );

wxBEGIN_EVENT_TABLE( CpuMonApp, wxApp )
	//EVT_THREAD( EVT_MQTT_ERROR, CpuMonApp::OnMqttError )
	//EVT_THREAD( EVT_MQTT_MESSG, CpuMonApp::OnMqttMessage )
wxEND_EVENT_TABLE()

CpuMonApp::CpuMonApp() : wxThreadHelper( wxTHREAD_JOINABLE ),
	m_quit( false ),
	m_mqtt( this ),
	m_doPublish( false ),
	m_server( _T("127.0.0.1") ),
	m_port( 1884 )
{
}

CpuMonApp::~CpuMonApp()
{
}

bool CpuMonApp::OnInit()
{
	m_tbIcon = new wxTaskBarIcon;
#ifdef __WXMSW__
	m_tbIcon->SetIcon( wxIcon( _T("TBICON") ) );
#else
	m_tbIcon->SetIcon( wxIcon( tbicon ) );
#endif
	tmpFrame * frame = new tmpFrame;
	frame->Show();
	SetTopWindow( frame );

	if ( argc > 1 )
	{
		const wxArrayString& args = argv.GetArguments();
		if ( wxStrcmp( args[1], "/pub" ) == 0 )
		{
			m_doPublish = true;
		}
		if ( argc > 2 )
			m_server = args[2];
		if ( argc > 3 )
			m_port = wxAtoi( args[3] );
	}

	CreateThread();
	GetThread()->Run();

	return true;
}

int CpuMonApp::OnExit()
{
	return 0;
}

void CpuMonApp::OnMqttError( wxThreadEvent & mqttErrEvent )
{
	wxMessageBox( mqttErrEvent.GetString() );
}

void CpuMonApp::OnMqttMessage( wxThreadEvent & mqttMesgEvent )
{
	wxString message = mqttMesgEvent.GetString();

	wxArrayString tokens = wxStringTokenize( message, _T( "," ) );
	if ( tokens.size() < 4 )
	{
		return;
	}
	const char sqlInsertFormat[] = "INSERT INTO MACHINE_STATUS "
		"( RecDateTime, CpuUsage, CpuTemperature, MemUsage )"
		" VALUES "
		"( \'%s\', \'%s\', \'%s\', \'%s\' );";

	wxString  sql;
	sql.Printf( sqlInsertFormat, tokens[0], tokens[1], tokens[2], tokens[3] );
	m_database.ExecSql( sql.mb_str() );
}

void CpuMonApp::Subscibe()
{
	CreateDatabase();
	Bind( EVT_MQTT_ERROR, &CpuMonApp::OnMqttError, this );
	Bind( EVT_MQTT_MESSG, &CpuMonApp::OnMqttMessage, this );

	m_mqtt.Init();
	m_mqtt.Subscribe( m_server, m_port, g_Topic );

	while ( !m_quit )
	{
		if ( !m_mqtt.Loop() )
			break;
	}
}

void CpuMonApp::Publish()
{
	if ( !m_mqtt.Init() || !m_mqtt.Connect( m_server, m_port ) )
		return;

	char message[100] = { 0 };
	while ( ! m_quit )
	{
		if ( ! m_mqtt.Loop() )
			break;

		wxSleep( 10 );
		wxUint32  memUsage = GetMemoryUsage();
		wxUint32  cpuUsage = GetCpuUsage();
		wxUint32  cpuTemp  = 0;

		wxDateTime msgDate = wxDateTime::Now();
		sprintf( message, "20%d/%02d/%02d %02d:%02d:%02d,%02d,%02d,%02d", 
			msgDate.GetYear(), msgDate.GetMonth(), msgDate.GetDay(),
			msgDate.GetHour(), msgDate.GetMinute(), msgDate.GetSecond(),
			cpuUsage,
			cpuTemp,
			memUsage );
		if ( ! m_mqtt.Publish( m_server, m_port, g_Topic, message ) )
			break;
	}
}

bool CpuMonApp::CreateDatabase()
{
	return m_database.Create( "macstat.db" );
}

wxUint32 CpuMonApp::GetMemoryUsage()
{
#ifdef __WXMSW__
	MEMORYSTATUSEX memStat;
	memStat.dwLength = sizeof( memStat );
	::GlobalMemoryStatusEx( & memStat );
	return wxUint32( memStat.dwMemoryLoad );
#else
	return 2300;
#endif
}

wxUint32 CpuMonApp::GetCpuUsage()
{
#ifdef __WXMSW__
	static FILETIME prevIdleTime;
	static FILETIME prevKernelTime;
	static FILETIME prevUserTime;

	FILETIME idleTime;
	FILETIME kernelTime;
	FILETIME userTime;
	BOOL ret = GetSystemTimes( & idleTime, & kernelTime, & userTime );

	__int64 idle   = CompareFileTime( & prevIdleTime,  & idleTime );
	__int64 kernel = CompareFileTime( & prevKernelTime, & kernelTime );
	__int64 user   = CompareFileTime( & prevUserTime,  & userTime );

	float rate = (kernel + user - idle) / (1.0 * (kernel + user));

	prevIdleTime   = idleTime;
	prevKernelTime = kernelTime;
	prevUserTime   = userTime;
	
	return wxUint32( rate );
#else
	return 0;
#endif
}
wxString CpuMonApp::getVersion()
{
      wxString versionStr;
#ifdef __WXMSW__
      versionStr =  std::to_string(MAJOR_VERSION) + '.' + std::to_string(MINOR_VERSION);
#else
      versionStr =  std::to_string(MAJOR_VERSION) + '.' + std::to_string(MINOR_VERSION);
#endif
      return versionStr;
}

