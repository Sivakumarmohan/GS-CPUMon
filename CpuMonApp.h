#pragma once

#include <wx/wx.h>
#include <wx/taskbar.h>

#include "Mqtt.h"
#include "Database.h"

class CpuMonApp : public wxApp , public wxThreadHelper
{
public:
	CpuMonApp();
	~CpuMonApp();
	
	void Quit()
	{
		m_quit = true;
		GetThread()->Wait();
		m_tbIcon->Destroy();
	}

protected:
	virtual bool OnInit();
	virtual int  OnExit();

	virtual wxThread::ExitCode Entry()
	{
		if ( m_doPublish )
		{
			Publish();
		}
		else
		{
			Subscibe();
		}
		return 0;
	};

	void OnMqttError( wxThreadEvent & mqttErrEvent );
	void OnMqttMessage( wxThreadEvent & mqttErrEvent );

	void Subscibe();
	void Publish();

	wxDECLARE_EVENT_TABLE();

	bool CreateDatabase();

	wxUint32  GetMemoryUsage();
	wxUint32  GetCpuUsage();

	volatile bool  m_quit;
	Mqtt           m_mqtt;
	Database       m_database;
	bool           m_doPublish;
	wxString       m_server;
	wxUint32       m_port;

	wxTaskBarIcon * m_tbIcon;
};

DECLARE_APP( CpuMonApp );

