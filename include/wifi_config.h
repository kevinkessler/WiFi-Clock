/*
 * wifi_config.h
 *
 *  Created on: Mar 14, 2016
 *      Author: kevin
 */

#ifndef INCLUDE_WIFI_CONFIG_H_
#define INCLUDE_WIFI_CONFIG_H_

void enable_web_server(uint8);
void networkScanCompleted(bool, BssList);
void onIndex(HttpRequest &request, HttpResponse &response);
void onAjaxNetworkList(HttpRequest &request, HttpResponse &response);
void onAjaxConnect(HttpRequest &request, HttpResponse &response);
void onSettings(HttpRequest &request, HttpResponse &response);
void onFile(HttpRequest &request, HttpResponse &response);
void makeConnection(void);
void restart(void);
void disable_web_server();

#endif /* INCLUDE_WIFI_CONFIG_H_ */
