#include <mosquitto.h>
#include <wx/tokenzr.h>

#ifdef __WXMSW__
#include "sqlite/sqlite3.h"
#else
#include <sqlite3.h>
#endif

#include "CpuMonApp.h"
#include "Platform.h"

#ifdef __WXGTK__
#include "tbicon.xpm"
#endif

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
	EVT_MENU( wxID_OPEN, CpuMonApp::OnOpen )
	//EVT_THREAD( EVT_MQTT_ERROR, CpuMonApp::OnMqttError )
	//EVT_THREAD( EVT_MQTT_MESSG, CpuMonApp::OnMqttMessage )
wxEND_EVENT_TABLE()

CpuMonApp::CpuMonApp() : wxThreadHelper( wxTHREAD_JOINABLE ),
	m_quit( false ),
	m_mqtt( this ),
	m_doPublish( false ),
	m_server( _T("127.0.0.1") ),
	m_port( 1884 ),
	m_tbIcon( nullptr )
{
}

CpuMonApp::~CpuMonApp()
{
}

bool CpuMonApp::OnInit()
{
	ArgCheck();

	tmpFrame * frame = new tmpFrame;
	frame->Show();
	SetTopWindow( frame );

	SetupTaskbarIcon();

	CreateThread();
	GetThread()->Run();

	return true;
}

int CpuMonApp::OnExit()
{
	return 0;
}

void CpuMonApp::SetupTaskbarIcon()
{
	m_tbIcon = new CpuMonTaskbarIcon( this );
#ifdef __WXMSW__
	m_tbIcon->SetIcon( wxIcon( _T( "TBICON" ) ) );
#else
	m_tbIcon->SetIcon( wxIcon( tbicon ) );
#endif

	m_tbIcon->Bind( wxEVT_TASKBAR_LEFT_DCLICK, &CpuMonApp::OnTaskbarDblClick, this );
}

void CpuMonApp::OnOpen( wxCommandEvent& commandEvent )
{
	wxMessageBox( "open" );
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

void CpuMonApp::OnTaskbarDblClick( wxTaskBarIconEvent& taskBarEvent )
{
	wxMessageBox( "click" );
}

wxMenu* CpuMonApp::CreatePopupMenu()
{
	wxMenu* menu = new wxMenu;
	menu->Append( wxID_OPEN, _("Open") );
	menu->Append( wxID_EXIT, _("Close") );
    return menu;
}

void CpuMonApp::Subscibe()
{
	CreateDatabase();
	BindMqttMessages();

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
	BindMqttMessages();
	if ( !m_mqtt.Init() || !m_mqtt.Connect( m_server, m_port ) )
		return;

	char message[100] = { 0 };
	while ( ! m_quit )
	{
		if ( ! m_mqtt.Loop() )
			break;

		wxSleep( 10 );
		wxUint32  memUsage = Platform::GetMemoryUsage();
		wxUint32  cpuUsage = Platform::GetCpuUsage();
		wxUint32  cpuTemp  = 0;

		wxDateTime msgDate = wxDateTime::Now();
		sprintf( message, "%d/%02d/%02d %02d:%02d:%02d,%02d,%02d,%02d", 
			msgDate.GetYear(), msgDate.GetMonth(), msgDate.GetDay(),
			msgDate.GetHour(), msgDate.GetMinute(), msgDate.GetSecond(),
			cpuUsage,
			cpuTemp,
			memUsage );
		if ( ! m_mqtt.Publish( m_server, m_port, g_Topic, message ) )
			break;
	}
}

void CpuMonApp::ArgCheck()
{
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
}

void CpuMonApp::BindMqttMessages()
{
	Bind( EVT_MQTT_ERROR, &CpuMonApp::OnMqttError,   this );
	Bind( EVT_MQTT_MESSG, &CpuMonApp::OnMqttMessage, this );
}

bool CpuMonApp::CreateDatabase()
{
	return m_database.Create( "macstat.db" );
}
