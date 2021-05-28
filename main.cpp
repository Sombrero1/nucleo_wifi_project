#include "mbed.h"
#include "ESP8266Interface.h"


#include "picojson.h"


#include "Websocket.h"

#include "TextLCD.h"
#include <cstdint>
#include <cstdio>
#include <string>


TextLCD lcd(D11, D12, D7, D6, D5, D4); // rs, e, d4-d7

 
int arrivedcount = 0;
WiFiInterface *wifi;

void write_msg_lcd(string m){
    lcd.cls();
    lcd.printf(m.c_str());
}
 
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


bool get_msg(Websocket *ws){
    char recv [100];
    bool receive = ws->read(recv);
    printf("hier:%d\n",receive);
    if (receive) {
            printf("rcv: %s\r\n", recv);

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
        }
    return receive;
}

void open_websocket()
{
    Websocket ws("ws://192.168.43.187:8080/websocket/30", wifi);
    bool err = ws.connect();
    printf("state: %d\n",err);

    while(err==false) {
        return;
    }
  

    wait(1);
    ws.send("success");
    wait(1);
    while (get_msg(&ws)){
        wait(21);
    };

    return;//не смогли по какой-то причине считать
}

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

RawSerial  pc(USBTX, USBRX);
RawSerial  dev(D8, D2);
 
bool get_ssid_and_password(string *ssid, string*pass){
    dev.baud(115200);

    wait(1);
    dev.puts("AT+RST\r\n");
    wait(0.5);
    dev.puts("AT+CWMODE_CUR=2\r\n");
    wait(0.5);
    dev.puts("AT+CWSAP_CUR=\"ESP8266\",\"1234567890\",5,3\r\n");
    wait(0.5);
    dev.puts("AT+CIPMUX=1\r\n");
    wait(0.5);
    dev.puts("AT+CIPAP_CUR=\"192.168.5.1\"\r\n");
    wait(0.5);
    dev.puts("AT+CIPSERVER=1,1001\r\n");

    write_msg_lcd("enter SSID and\n password");

    string t = "";
    while(t.find("+IPD") == -1){
        t = "";
        char chr;
        if(dev.readable()){
            do{
                chr = dev.getc();
                t = t + chr;
            }
            while(chr!='\n');
        }
    }
    printf("read : %s\n",t.c_str());
    
    if(t.find('{') != -1){
        picojson::value v;
        
        string m = t.substr(t.find('{'), t.find('}')-t.find('{')+1);
        const char *json = m.c_str();

        picojson::parse(v, json, json + strlen(json));
                //printf("res error? %s\r\n", err.c_str());
        *ssid =  v.get("ssid").get<string>();
        *pass =  v.get("password").get<string>();

        printf("ssid and password : %s,%s\n",ssid->c_str(),pass->c_str());

        return true;
    }
    else{
        return false;
    }
}

int main()
{  
#ifdef MBED_MAJOR_VERSION
    printf("Mbed OS version %d.%d.%d\n\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
#endif

    wifi = WiFiInterface::get_default_instance();
    if (!wifi) {
        printf("ERROR: No WiFiInterface found.\n");
        return -1;
    }
    
    // write_msg_lcd("search "+ssid);
    // int count = scan_demo(wifi);
    // if (count == 0) {
    //     printf("No WIFI APs found - can't continue further.\n");
    //     return -1;
    // }
    
    string ssid,pass;
    int ret = 0;
    do{
        while(!get_ssid_and_password(&ssid, &pass)){
            write_msg_lcd("try again");
        }
        ret = wifi->connect(ssid.c_str(),pass.c_str(), NSAPI_SECURITY_WPA_WPA2);
        write_msg_lcd("Connecting to\n"+ssid);
        printf("\nConnecting to %s...\n", ssid.c_str());
        if (ret != 0) {
            if (ret == -3008 || ret == -3006 ||ret==-3003){
                write_msg_lcd("Wi-Fi: %s. Conn error.\n Check pass");
            }
            else{
                write_msg_lcd("Wi-Fi: %s. Error. Reset device");
                printf("\nConnection error: %d\n", ret);
            }
            wait(2);
        }
    }while(ret!=0);

    printf("Success\n\n");
    printf("MAC: %s\n", wifi->get_mac_address());
    printf("IP: %s\n", wifi->get_ip_address());
    printf("Netmask: %s\n", wifi->get_netmask());
    printf("Gateway: %s\n", wifi->get_gateway());
    printf("RSSI: %d\n\n", wifi->get_rssi());
    
    write_msg_lcd("success conn");
    while(true){
        open_websocket();
        write_msg_lcd("reconnect to \nserv");
    }
    wifi->disconnect();

    printf("\nDone\n");
}
