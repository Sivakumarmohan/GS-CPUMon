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

	void SetupTaskbarIcon();

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

	void OnOpen( wxCommandEvent& commandEvent );
	void OnMqttError( wxThreadEvent & mqttErrEvent );
	void OnMqttMessage( wxThreadEvent & mqttErrEvent );
	
	void OnTaskbarDblClick( wxTaskBarIconEvent & taskBarEvent );

	wxMenu* CreatePopupMenu();

	void Subscibe();
	void Publish();

	void ArgCheck();
	void BindMqttMessages();

	wxDECLARE_EVENT_TABLE();

	bool CreateDatabase();

	class CpuMonTaskbarIcon : public wxTaskBarIcon
	{
		CpuMonApp * m_app;
		void OnCommand( wxCommandEvent& commandEvent )
		{
			m_app->ProcessEvent( commandEvent );
		}
	public:
		CpuMonTaskbarIcon( CpuMonApp* app ) : m_app( app )
		{
			Bind( wxEVT_COMMAND_MENU_SELECTED, &CpuMonTaskbarIcon::OnCommand, this, wxID_ANY );
		};
		wxMenu* CreatePopupMenu()
		{
			return m_app->CreatePopupMenu();
		}
	};

	volatile bool  m_quit;
	Mqtt           m_mqtt;
	Database       m_database;
	bool           m_doPublish;
	wxString       m_server;
	wxUint32       m_port;

	wxTaskBarIcon * m_tbIcon;
};

DECLARE_APP( CpuMonApp );

