#pragma once
#include <mosquitto.h>
#include <wx/wx.h>

wxDECLARE_EVENT( EVT_MQTT_ERROR, wxThreadEvent );
wxDECLARE_EVENT( EVT_MQTT_MESSG, wxThreadEvent );

class Mqtt/* : public wxThreadHelper*/
{
public:
	Mqtt( wxEvtHandler * handler );
	~Mqtt();

	bool Init();
	bool Connect( const char* host, int port, int keepAlive = 10 );
	bool Loop( int timeout = 10 );
	bool Subscribe( const char * host, int port,
		const char * topic,
		int keepAlive = 10, int qos = 2 );
	bool Publish( const char * host, int port,
		const char * topic, const char * message,
		int keepAlive = 10, int qos = 2 );

protected:
	wxEvtHandler *  m_msgHandler;
	struct mosquitto * m_mqttSession;

	void Start();

	static void  OnSubscribe( struct mosquitto * mosq, void * obj,
		int mid, int qos_count, const int * granted_qos );

	static void  OnConnect( struct mosquitto * mosq, void * obj,
		int result, int flags, const mosquitto_property * properties );

	static void  OnMessage( struct mosquitto * mosq, void * obj,
		const struct mosquitto_message * message, const mosquitto_property * properties );

	void ReportError( const char * error );
	void SendMqttMessage( const char * message );
};

