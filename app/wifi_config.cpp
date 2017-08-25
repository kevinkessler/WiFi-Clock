/*
 * wifi_config.cpp
 *
 *  Created on: Mar 14, 2016
 *      Author: kevin
 */

#include <SmingCore/SmingCore.h>
#include <rboot/rboot.h>
#include "display.h"
#include "wifi_config.h"
#include "Configurator.h"

BssList networks;
HttpServer *server;
String newNetwork;
String newPassword;
Timer connectionTimer;
bool toldToConnect=false;
bool web_running=false;

extern Timer pubTimer;
extern ClockDisplay led_display;
extern Configurator *config;

void enable_web_server(uint8_t errCode)
{
	if (web_running)
		return;
	web_running=true;
	pubTimer.stop();

	led_display.showError(errCode);

/*	if(!WifiStation.isConnected())
	{
		WifiStation.config("","",false);
//		WifiStation.enable(true);
	}*/




	//WifiAccessPoint.enable(true);
	bool succ=wifi_set_opmode_current(0x03);
	// Unreliable, often fails on wifi_softap_set_config unless started from a reboot
	WifiAccessPoint.config("Wifi Clock Configuration", "", AUTH_OPEN);

	WifiStation.startScan(networkScanCompleted);

	int slot = rboot_get_current_rom();
	if (slot == 0) {
	    //debugf("trying to mount spiffs at %x, length %d", RBOOT_SPIFFS_0 + 0x40200000, SPIFF_SIZE);
	    spiffs_mount_manual(RBOOT_SPIFFS_0 + 0x40200000, SPIFF_SIZE);
	} else {
	    //debugf("trying to mount spiffs at %x, length %d", RBOOT_SPIFFS_1 + 0x40200000, SPIFF_SIZE);
	    spiffs_mount_manual(RBOOT_SPIFFS_1 + 0x40200000, SPIFF_SIZE);
	}


	server=new HttpServer();
	server->listen(80);
	server->addPath("/",onIndex);
	server->addPath("/settings", onSettings);
	server->addPath("/ajax/get-networks", onAjaxNetworkList);
	server->addPath("/ajax/connect", onAjaxConnect);
	server->setDefaultHandler(onFile);
}

void disable_web_server()
{
	if(web_running)
		restart();
}

void onFile(HttpRequest &request, HttpResponse &response)
{
	String file = request.getPath();
	if (file[0] == '/')
		file = file.substring(1);

	if (file[0] == '.')
		response.forbidden();
	else
	{
		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile(file);
	}
}

void onIndex(HttpRequest &request, HttpResponse &response)
{
	TemplateFileStream *tmpl = new TemplateFileStream("index.html");
	auto &vars = tmpl->variables();
	response.sendTemplate(tmpl);
}

void onAjaxNetworkList(HttpRequest &request, HttpResponse &response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	json["status"] = (bool)true;

	bool connected = WifiStation.isConnected();
	json["connected"] = connected;
	if (connected)
	{
		// Copy full string to JSON buffer memory
		json["network"]= WifiStation.getSSID();
	}

	JsonArray& netlist = json.createNestedArray("available");
	debugf("Display Nets");
	for (int i = 0; i < networks.count(); i++)
	{
		debugf("%s %d",networks[i].ssid.c_str(),networks[i].rssi);
		if (networks[i].hidden) continue;
		JsonObject &item = netlist.createNestedObject();
		item["id"] = (int)networks[i].getHashId();
		// Copy full string to JSON buffer memory
		item["title"] = networks[i].ssid;
		item["signal"] = networks[i].rssi;
		item["encryption"] = networks[i].getAuthorizationMethodName();
	}

	response.setAllowCrossDomainOrigin("*");
	response.sendJsonObject(stream);

	WifiStation.startScan(networkScanCompleted);
}

void onAjaxConnect(HttpRequest &request, HttpResponse &response)
{
	JsonObjectStream* stream = new JsonObjectStream();
	JsonObject& json = stream->getRoot();

	String curNet = request.getPostParameter("network");
	String curPass = request.getPostParameter("password");

	bool updating = curNet.length() > 0 && (WifiStation.getSSID() != curNet || WifiStation.getPassword() != curPass);
	bool connectingNow = WifiStation.getConnectionStatus() == eSCS_Connecting || toldToConnect;

	if (updating && connectingNow)
	{
		debugf("updating: %d, connectingNow: %d", updating, connectingNow);
		json["status"] = (bool)false;
		json["connected"] = (bool)false;
	}
	else
	{
		json["status"] = (bool)true;
		if (updating)
		{
			newNetwork = curNet;
			newPassword = curPass;
			debugf("CONNECT TO: %s %s", newNetwork.c_str(), newPassword.c_str());
			json["connected"] = false;
			//connectionTimer.initializeMs(1200, makeConnection).startOnce();
			makeConnection();
		}
		else
		{
			json["connected"] = WifiStation.isConnected();
			if(WifiStation.isConnected()==true)
			{
				//WifiStation.config(newNetwork,newPassword,true);
				connectionTimer.initializeMs(5000,restart).startOnce();
			}
			debugf("Network already selected. Current status: %s", WifiStation.getConnectionStatusName());
			if((WifiStation.getConnectionStatus()==eSCS_WrongPassword)||
					(WifiStation.getConnectionStatus()==eSCS_AccessPointNotFound)||
					(WifiStation.getConnectionStatus()==eSCS_ConnectionFailed))
			{
				json["error"]= WifiStation.getConnectionStatusName();
				toldToConnect=false;
			}
		}
	}

	if (!updating && !connectingNow && WifiStation.isConnectionFailed())
	{
		json["error"] = WifiStation.getConnectionStatusName();
		toldToConnect=false;
	}

	response.setAllowCrossDomainOrigin("*");
	response.sendJsonObject(stream);
}

void onSettings(HttpRequest &request, HttpResponse &response)
{

	if (request.getRequestMethod() == RequestMethod::POST)
	{
		if((request.getPostParameter("mqtt_server").compareTo(config->getMQTTServer())!=0)||
				(request.getPostParameter("firmware_server").compareTo(config->getFirmwareServer())!=0)||
				(request.getPostParameter("mqtt_port").toInt()!=config->getMQTTPort())||
				(request.getPostParameter("firmware_port").toInt()!=config->getFirmwarePort()))
		{
			config->setMQTTServer(request.getPostParameter("mqtt_server").c_str());
			config->setFirmwareServer(request.getPostParameter("firmware_server").c_str());
			config->setMQTTPort(request.getPostParameter("mqtt_port").toInt());
			config->setFirmwarePort(request.getPostParameter("firmware_port").toInt());

			debugf("Saving %s %d %s %d to config",config->getMQTTServer(),config->getMQTTPort(),
					config->getFirmwareServer(),config->getFirmwarePort());
			config->saveConfiguration();
			connectionTimer.initializeMs(5000,restart).startOnce();
		}
	}

	TemplateFileStream *tmpl = new TemplateFileStream("settings.html");
	auto &vars = tmpl->variables();

	vars["mqtt_server"]=config->getMQTTServer();
	vars["mqtt_port"]=config->getMQTTPort();
	vars["firmware_server"]=config->getFirmwareServer();
	vars["firmware_port"]=config->getFirmwarePort();

	response.sendTemplate(tmpl);
}

void makeConnection()
{
	WifiStation.enable(true);
	WifiStation.config(newNetwork, newPassword,true);


	toldToConnect=true;
}

void networkScanCompleted(bool succeeded, BssList list)
{
	if (succeeded)
	{
		debugf("Network List");
		networks.clear();
		for (int i = 0; i < list.count(); i++)
		{
			debugf("%s %d",list[i].ssid.c_str(),list[i].rssi);
			if (!list[i].hidden && list[i].ssid.length() > 0)
				networks.add(list[i]);
		}
	}
	networks.sort([](const BssInfo& a, const BssInfo& b){ return b.rssi - a.rssi; } );
}

void restart()
{
	System.restart();
}
