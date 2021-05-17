#pragma once

#include <wx/wx.h>
#include <wx/taskbar.h>

#include "Mqtt.h"
#include "Database.h"

class CpuMonApp : public wxApp, public wxThreadHelper
{
public:
	CpuMonApp();
	~CpuMonApp();

protected:
	virtual bool OnInit();
	virtual int  OnExit();

	virtual wxThread::ExitCode Entry()
	{
		Start();
		return 0;
	};

	void OnMqttError( wxThreadEvent & mqttErrEvent );
	void OnMqttMessage( wxThreadEvent & mqttErrEvent );

	void Start();

	wxDECLARE_EVENT_TABLE();

	bool CreateDatabase();
	bool MqttSubscribe();

	Mqtt  m_mqtt;
	Database  m_database;

	wxTaskBarIcon  m_tbIcon;
};

DECLARE_APP( CpuMonApp );

