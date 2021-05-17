#include "Mqtt.h"

wxDEFINE_EVENT( EVT_MQTT_ERROR, wxThreadEvent );
wxDEFINE_EVENT( EVT_MQTT_MESSG, wxThreadEvent );

Mqtt::Mqtt( wxEvtHandler * handler ) :
	m_msgHandler( handler ),
	m_mqttSession( nullptr )
{
	wxASSERT( m_msgHandler != nullptr );
}

Mqtt::~Mqtt()
{
	if ( m_mqttSession != nullptr )
	{
		mosquitto_disconnect( m_mqttSession );
		mosquitto_destroy( m_mqttSession );
		m_mqttSession = nullptr;
	}
}

bool Mqtt::Init()
{
	if ( MOSQ_ERR_SUCCESS != mosquitto_lib_init() )
	{
		ReportError( "mosquitto_lib_init failed." );
		return false;
	}

	m_mqttSession = mosquitto_new( nullptr, true, this );
	if ( m_mqttSession == nullptr )
	{
		ReportError( "mosquitto_new failed." );
		return false;
	}
	return true;
}

bool Mqtt::Subscribe( const char * host, int port, const char * topic, int keepAlive, int qos )
{
	mosquitto_subscribe_callback_set( m_mqttSession,  OnSubscribe );
	mosquitto_connect_v5_callback_set( m_mqttSession, OnConnect   );
	mosquitto_message_v5_callback_set( m_mqttSession, OnMessage   );

	bool ready = true;
	if ( MOSQ_ERR_SUCCESS != mosquitto_connect( m_mqttSession, host, port, keepAlive ) )
	{
		int errCode = errno;
		ReportError( "mosquitto_connect failed." );
		ready = false;
	}
	else if ( MOSQ_ERR_SUCCESS != mosquitto_subscribe( m_mqttSession, nullptr, topic, qos ) )
	{
		int errCode = errno;
		ReportError( "mosquitto_subscribe failed." );
		ready = false;
	}

	if ( ready )
	{
		mosquitto_loop_forever( m_mqttSession, -1, 1 );
		return true;
	}
	return false;
}

void Mqtt::Start()
{
}

void Mqtt::OnSubscribe( mosquitto* mosq, void* obj, int mid, int qos_count, const int* granted_qos )
{
}

void Mqtt::OnConnect( mosquitto* mosq, void* obj, int result, int flags, const mosquitto_property* properties )
{
}

void Mqtt::OnMessage( mosquitto* mosq, void* obj, const mosquitto_message* message, const mosquitto_property* properties )
{
	Mqtt* self = reinterpret_cast<Mqtt*>(obj);
	char* payload = new char[message->payloadlen + 1];
	memcpy( payload, reinterpret_cast<const char*>(message->payload), message->payloadlen );
	payload[message->payloadlen] = 0;
	self->SendMqttMessage( payload );
}

void Mqtt::ReportError( const char * error )
{
	wxThreadEvent  errorEvent( EVT_MQTT_ERROR );
	errorEvent.SetString( error );
	wxQueueEvent( m_msgHandler, errorEvent.Clone() );
}

void Mqtt::SendMqttMessage( const char * message )
{
	wxThreadEvent  msgEvent( EVT_MQTT_MESSG );
	msgEvent.SetString( message );
	delete[] message;
	wxQueueEvent( m_msgHandler, msgEvent.Clone() );
}
