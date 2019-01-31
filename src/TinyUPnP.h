/*
  TinyUPnP.h - Library for creating UPnP rules automatically in your router.
  Created by Ofek Pearl, September 2017.
  Released into the public domain.
*/

#ifndef TinyUPnP_h
#define TinyUPnP_h

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <limits.h>

// #define UPNP_DEBUG
#define UPNP_SSDP_PORT 1900
#define TCP_CONNECTION_TIMEOUT_MS 6000
#define INTERNET_GATEWAY_DEVICE "urn:schemas-upnp-org:device:InternetGatewayDevice:1"
/*
#define PORT_MAPPING_INVALID_INDEX "<errorDescription>SpecifiedArrayIndexInvalid</errorDescription>"
#define PORT_MAPPING_INVALID_ACTION "<errorDescription>Invalid Action</errorDescription>"
*/
#define RULE_PROTOCOL_TCP "TCP"
#define RULE_PROTOCOL_UDP "UDP"

#define MAX_NUM_OF_UPDATES_WITH_NO_EFFECT 6  // after 10 tries of updatePortMapping we will execute the more extensive addPortMapping
/*
#define UDP_TX_PACKET_MAX_SIZE 512  // reduce max UDP packet size to conserve memory (by default UDP_TX_PACKET_MAX_SIZE=8192)
#define UDP_TX_RESPONSE_MAX_SIZE 512
*/

const String UPNP_SERVICE_TYPE_1 = "urn:schemas-upnp-org:service:WANPPPConnection:1";
const String UPNP_SERVICE_TYPE_2 = "urn:schemas-upnp-org:service:WANIPConnection:1";
const String UPNP_SERVICE_TYPE_TAG_START = "<serviceType>";
const String UPNP_SERVICE_TYPE_TAG_END = "</serviceType>";

const String SOAPActions [] = {
	"AddPortMapping",
	"GetSpecificPortMappingEntry",
	"DeletePortMapping",
	"GetGenericPortMappingEntry",
	"GetExternalIPAddress"
};
enum e_SOAPActions{
	AddPortMapping,
	GetSpecificPortMappingEntry,
	DeletePortMapping,
	GetGenericPortMappingEntry,
	GetExternalIPAddress
};

typedef void (*callback_function)(void);

typedef struct _gatewayInfo {
	// router info
	IPAddress host;
	int port;  // this port is used when getting router capabilities and xml files
	String path;  // this is the path that is used to retrieve router information from xml files
	
	// info for actions
	int actionPort;  // this port is used when performing SOAP API actions
	String actionPath;  // this is the path used to perform SOAP API actions
	String serviceTypeName;  // i.e "WANPPPConnection:1" or "WANIPConnection:1"
} gatewayInfo;

typedef struct _upnpRule {
	int index;
	String devFriendlyName;
	IPAddress internalAddr;
	int internalPort;
	int externalPort;
	String protocol;
	int leaseDuration;
} upnpRule;

typedef struct _upnpRuleNode {
	_upnpRule *rule_ptr;
	_upnpRuleNode *next_ptr;
} upnpRuleNode;

enum upnpResult {
	UNDEFINED_ERROR,
	UPNP_OK,
	SUCCESS,  // port mapping was added
	ALREADY_MAPPED,  // the port mapping is already found in the IGD
	NOP,  // the check is delayed
	WIFI_TIMOUT,
	TCP_TIMOUT,
	PORT_MAPPING_TIMEOUT,
	INVALID_MAPPING_RULE,
	INVALID_GATEWAY_INFO,
	INVALID_PARAMETER,
	INVALID_INDEX
};

class TinyUPnP
{
	public:
		TinyUPnP(unsigned long timeoutMs);
		~TinyUPnP();
		upnpResult addPortMapping();
		void setMappingConfig(IPAddress ruleIP, int rulePort, String ruleProtocol, int ruleLeaseDuration, String ruleFriendlyName);
		upnpResult updatePortMapping(unsigned long intervalMs, callback_function fallback = NULL/* optional */);
		boolean printAllPortMappings();
		void printAllRules();		// TODO
		upnpResult testConnectivity(unsigned long startTime = 0);
		unsigned long _lastUpdateTime;
		IPAddress getExternalIP();
	private:
		boolean connectUDP();
		void broadcastMSearch();
		boolean waitForUnicastResponseToMSearch(gatewayInfo *deviceInfo, IPAddress gatewayIP);
		boolean getGatewayInfo(gatewayInfo *deviceInfo, long startTime);
		boolean isGatewayInfoValid(gatewayInfo *deviceInfo);
		void clearGatewayInfo(gatewayInfo *deviceInfo);
		boolean connectToIGD(IPAddress host, int port);
		upnpResult postSOAPAction(gatewayInfo *deviceInfo, _upnpRule *rule_ptr, e_SOAPActions SOAPAction);
		boolean getIGDEventURLs(gatewayInfo *deviceInfo);
		boolean addPortMappingEntry(gatewayInfo *deviceInfo, _upnpRule *rule_ptr = NULL);
		boolean verifyPortMapping(gatewayInfo *deviceInfo, _upnpRule *rule_ptr = NULL);
		//IPAddress ipToAddress(String ip);
		//char* ipAddressToCharArr(IPAddress ipAddress);  // ?? not sure this is needed
		//String ipAddressToString(IPAddress ipAddress);
		String upnpRuleToString(upnpRule *rule_ptr);
		String getSpacesString(int num);
		IPAddress getHost(String url);
		int getPort(String url);
		String getPath(String url);
		String getTagContent(String line, String tagName);

		upnpRuleNode *_headRuleNode;

		unsigned long _timeoutMs;  // 0 for blocking operation
		WiFiUDP _udpClient;
		WiFiClient _wifiClient;
		gatewayInfo _gwInfo;
		unsigned long _consequtiveFails;
};

#endif
