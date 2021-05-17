#include <mosquitto.h>
#include <wx/tokenzr.h>
#include "sqlite/sqlite3.h"

#include "CpuMonApp.h"

class tmpFrame : public wxFrame
{
public:
	tmpFrame() : wxFrame( nullptr, wxID_ANY, _T( "MQTT" ) ) {}
};

IMPLEMENT_APP( CpuMonApp );

wxBEGIN_EVENT_TABLE( CpuMonApp, wxApp )
	EVT_THREAD( EVT_MQTT_ERROR, CpuMonApp::OnMqttError )
	EVT_THREAD( EVT_MQTT_MESSG, CpuMonApp::OnMqttMessage )
wxEND_EVENT_TABLE()

CpuMonApp::CpuMonApp() : m_mqtt( this )
{
}

CpuMonApp::~CpuMonApp()
{
}

bool CpuMonApp::OnInit()
{
	tmpFrame * frame = new tmpFrame;
	frame->Show();

	m_tbIcon.SetIcon( wxIcon( _T( "TBICON" ) ) );

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
	const char sqlInsertFormat[] = "INSERT INTO CPU_STATE "
		"( RecTime, Usage, Temperature, MemUsage )"
		" VALUES "
		"( \'%s\', \'%s\', \'%s\', \'%s\' );";

	wxString  sql;
	sql.Printf( sqlInsertFormat, tokens[0], tokens[1], tokens[2], tokens[3] );
	m_database.ExecSql( sql.mb_str() );
}

void CpuMonApp::Start()
{
	CreateDatabase();
	MqttSubscribe();
}

bool CpuMonApp::CreateDatabase()
{
	return m_database.Create( "cpu.db" );
}

bool CpuMonApp::MqttSubscribe()
{
	Bind( EVT_MQTT_ERROR, & CpuMonApp::OnMqttError, this );
	Bind( EVT_MQTT_MESSG, & CpuMonApp::OnMqttMessage, this );

	m_mqtt.Init();
	m_mqtt.Subscribe( "127.0.0.1", 1884, "CPU_STAT" );

	return true;
}

