/* WiFi Example
 * Copyright (c) 2016 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed.h"
#include "ESP8266Interface.h"
// #include <cstdint>
// #include <cstdio>
// #include <cstring>

#include "picojson.h"

// #define MQTTCLIENT_QOS2 1
 
// #include "MQTTmbed.h"
// #include "MQTTClientMbedOs.h"

// #include "http_request.h"
#include "Websocket.h"

#include "TextLCD.h"
#include <cstdio>
#include <string>


TextLCD lcd(D11, D12, D7, D6, D5, D4); // rs, e, d4-d7

 
int arrivedcount = 0;
WiFiInterface *wifi;

//#include <typeinfo>	

// #define MQTT_MAX_PACKET_SIZE 400   
// #define MQTT_MAX_PAYLOAD_SIZE 300 
 
void set_notification(int states[]){
    lcd.cls();
    string modes[3]={"legs","public","car"};
    string recv = "";
    for(int i = 0; i < 3; i++){
        if(states[i]==0)recv += modes[i]+":"+"late ";
        else recv += modes[i]+":"+to_string(states[i])+"m ";
    }
    lcd.printf(recv.c_str());
}

void set_time(char time[5]){
    printf("set time: %s\n",time);
}

char* next(){
    return strtok(NULL, " {}:,\"");
}

void send_websocket()
{
    wait(3);
    Websocket ws("ws://192.168.43.187:8080/websocket/30", wifi);
    wait(3);
    int err = ws.connect();
    printf("state: %d\n",err);

    while(err==0) send_websocket();
  
    char recv [100];
    wait(5);

    ws.send("success");
    wait(5);

  while (1) { 

        if (ws.read(recv)) {
        printf("rcv: %s\r\n", recv);
         //   char *msg = strtok(recv, " {}:,\"");

            picojson::value v;
            const char *json = recv;
            picojson::parse(v, json, json + strlen(json));

            int states[3];
            //printf("res error? %s\r\n", err.c_str());
            states[0] = (int) v.get("walking").get<double>();
            states[1] = (int) v.get("transit").get<double>();
            states[2] = (int) v.get("driving").get<double>();
            if(states[0]==-1 && states[1] == -1 && states[2] == -1) {
                lcd.cls();
                lcd.printf("No routes");
                }
            else set_notification(states);
            
            
            // while(msg!=NULL){
            //     printf("msg: %s\n",msg);
            
            //     if (strcmp(msg, "walking") == 0){
            //         printf("Hello:");
            //         printf(next());


            //         printf("\n");
            //     }
            //      msg = strtok(NULL, " {}:,\"");
            // }

        }
        wait(5);
    }
}



// void http_request_Post_json(){
//     const char body[] = "{\"hello\":\"world\"}";
    
//     HttpRequest* request = new HttpRequest(wifi, HTTP_POST, "http://192.168.43.187:8080/put_clock");
//     request->set_header("Content-Type", "application/json");
//     HttpResponse* response = request->send(body, strlen(body));
//     printf("body is:\n%s\n", response->get_body_as_string().c_str());
//     printf("body is:\n%s\n", response->get_status_message().c_str());
// // if response is NULL, check response->get_error()
// }
 
// void ibm_cloud_demo(NetworkInterface *net)
// {
//     TCPSocket socket;
//     MQTTClient client(&socket);
 
//     SocketAddress a;
//     char* hostname = "quickstart.messaging.internetofthings.ibmcloud.com";
//     net->gethostbyname(hostname, &a);
//     int port = 1883;
//     a.set_port(port);
 
//     printf("Connecting to %s:%d\r\n", hostname, port);
 
//     socket.open(net);
//     printf("Opened socket\n\r");
//     int rc = socket.connect(a);
//     if (rc != 0)
//         printf("rc from TCP connect is %d\r\n", rc);
//     printf("Connected socket\n\r");
 
 
//     MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
//     data.MQTTVersion = 3;
//     data.clientID.cstring = "d:quickstart:my_test:gg_ww";
//     data.username.cstring = "testuser";
//     data.password.cstring = "testpassword";
//     if ((rc = client.connect(data)) != 0)
//         printf("rc from MQTT connect is %d\r\n", rc);
 
//     MQTT::Message message;
 
//     char buf[MQTT_MAX_PAYLOAD_SIZE];
//     float temp = 1;
//     float press = 2; 
//     float hum = 3;
 
//     sprintf(buf,
//      "{\"d\":{\"ST\":\"Nucleo-IoT-mbed\",\"Temp\":%0.4f,\"Pressure\":%0.4f,\"Humidity\":%0.4f}}",
//               temp, press, hum);
//     message.qos = MQTT::QOS0;
//     message.retained = false;
//     message.dup = false;
//     message.payload = (void*)buf;
//     message.payloadlen = strlen(buf);
 
//     char* topic = "iot-2/evt/my_event/fmt/json";
 
//     if( (message.payloadlen + strlen(topic)+1) >= MQTT_MAX_PACKET_SIZE )
//         printf("message too long!\r\n");
    
//     rc = client.publish(topic, message);
//     return;       
// }






// void messageArrived(MQTT::MessageData& md)
// {
//     MQTT::Message &message = md.message;
//     printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
//     printf("Payload %.*s\r\n", message.payloadlen, (char*)message.payload);
//     ++arrivedcount;
// }
 
// void mqtt_demo(NetworkInterface *net)
// {
//     float version = 0.6;
//     char* topic = "mbed-sample";
 
//     TCPSocket socket;
//     MQTTClient client(&socket);
 
//     SocketAddress a;
//     char* hostname = "broker.hivemq.com";
//     net->gethostbyname(hostname, &a);
//     int port = 1883;
//     a.set_port(port);
 
//     printf("Connecting to %s:%d\r\n", hostname, port);
 
//     socket.open(net);
//     printf("Opened socket\n\r");
//     int rc = socket.connect(a);
//     if (rc != 0)
//         printf("rc from TCP connect is %d\r\n", rc);
//     printf("Connected socket\n\r");
 
//     MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
//     data.MQTTVersion = 3;
//     data.clientID.cstring = "mbed-sample";
//     data.username.cstring = "testuser";
//     data.password.cstring = "testpassword";
//     if ((rc = client.connect(data)) != 0)
//         printf("rc from MQTT connect is %d\r\n", rc);
 
//     if ((rc = client.subscribe(topic, MQTT::QOS2, messageArrived)) != 0)
//         printf("rc from MQTT subscribe is %d\r\n", rc);
 
//     MQTT::Message message;
 
//     // QoS 0
//     char buf[100];
//     sprintf(buf, "Я бэтмен\r\n");
//     message.qos = MQTT::QOS0;
//     message.retained = false;
//     message.dup = false;
//     message.payload = (void*)buf;
//     message.payloadlen = strlen(buf)+1;
//     rc = client.publish(topic, message);
//     while (arrivedcount < 1)
//         client.yield(100);
 
//     // QoS 1
//     sprintf(buf, "Пора восстановить справедливость\r\n");
//     message.qos = MQTT::QOS1;
//     message.payloadlen = strlen(buf)+1;
//     rc = client.publish(topic, message);
//     while (arrivedcount < 2)
//         client.yield(100);
 
//     while (arrivedcount < 3)
//         client.yield(100);
 
 
//     if ((rc = client.unsubscribe(topic)) != 0)
//         printf("rc from unsubscribe was %d\r\n", rc);
 
//     if ((rc = client.disconnect()) != 0)
//         printf("rc from disconnect was %d\r\n", rc);
 
//     socket.close();
 
//     printf("Version %.2f: finish %d msgs\r\n", version, arrivedcount);
 
//     return;
// }
const char *sec2str(nsapi_security_t sec)
{
    switch (sec) {
        case NSAPI_SECURITY_NONE:
            return "None";
        case NSAPI_SECURITY_WEP:
            return "WEP";
        case NSAPI_SECURITY_WPA:
            return "WPA";
        case NSAPI_SECURITY_WPA2:
            return "WPA2";
        case NSAPI_SECURITY_WPA_WPA2:
            return "WPA/WPA2";
        case NSAPI_SECURITY_UNKNOWN:
        default:
            return "Unknown";
    }
}

int scan_demo(WiFiInterface *wifi)
{
    WiFiAccessPoint *ap;
 
    printf("Scan:\n");
 
    int count = wifi->scan(NULL,0);
 
    if (count <= 0) {
        printf("scan() failed with return value: %d\n", count);
        return 0;
    }
 
    /* Limit number of network arbitrary to 15 */
    count = count < 15 ? count : 15;
 
    ap = new WiFiAccessPoint[count];
    count = wifi->scan(ap, count);
 
    if (count <= 0) {
        printf("scan() failed with return value: %d\n", count);
        return 0;
    }
 
    for (int i = 0; i < count; i++) {
        printf("Network: %s secured: %s BSSID: %hhX:%hhX:%hhX:%hhx:%hhx:%hhx RSSI: %hhd Ch: %hhd\n", ap[i].get_ssid(),
               sec2str(ap[i].get_security()), ap[i].get_bssid()[0], ap[i].get_bssid()[1], ap[i].get_bssid()[2],
               ap[i].get_bssid()[3], ap[i].get_bssid()[4], ap[i].get_bssid()[5], ap[i].get_rssi(), ap[i].get_channel());
    }
    printf("%d networks available.\n", count);
 
    delete[] ap;
    return count;
}


int main()
{  
#ifdef MBED_MAJOR_VERSION
    printf("Mbed OS version %d.%d.%d\n\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
#endif
    wait(1);
    // RawSerial  esp(D8, D2);
    // esp.baud(115200);
    // pc.baud(115200);
    //esp.printf("AT\n\r");
    // esp.printf("AT+CWMODE_CUR=2\n\r");
    // esp.printf("AT+CIPSTA_CUR=\"192.168.6.100\"\n\r");
    // ESP8266Interface net(D8,D2, true);
    // wifi = net.get_default_instance();
    wifi = WiFiInterface::get_default_instance();
    if (!wifi) {
        printf("ERROR: No WiFiInterface found.\n");
        return -1;
    }

    int count = scan_demo(wifi);
    if (count == 0) {
        printf("No WIFI APs found - can't continue further.\n");
        return -1;
    }

    //ниже подключение к вай фаю

    printf("\nConnecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
    int ret = wifi->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) {
        printf("\nConnection error: %d\n", ret);
        return -1;
    }

    printf("Success\n\n");
    printf("MAC: %s\n", wifi->get_mac_address());
    printf("IP: %s\n", wifi->get_ip_address());
    printf("Netmask: %s\n", wifi->get_netmask());
    printf("Gateway: %s\n", wifi->get_gateway());
    printf("RSSI: %d\n\n", wifi->get_rssi());


    TCPSocket tcpserver(wifi);
    int open,bind,listen,accept;
    tcpserver.open(wifi);
    bind = tcpserver.bind(wifi->get_ip_address(),7);
    listen = tcpserver.listen();
    tcpserver.accept();
    printf("%d; %d; %d \n",open,bind,listen);


    
    //вывести ip адрес на экранчик


  // send_websocket();
    send_websocket();
    wifi->disconnect();

    printf("\nDone\n");
}
