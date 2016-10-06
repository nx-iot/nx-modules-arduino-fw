/*
   UIPEthernet TcpClient example.

   UIPEthernet is a TCP/IP stack that can be used with a enc28j60 based
   Ethernet-shield.

   UIPEthernet uses the fine uIP stack by Adam Dunkels <adam@sics.se>

        -----------------

   This TcpClient example gets its local ip-address via dhcp and sets
   up a tcp socket-connection to 192.168.0.1 port 5000 every 5 Seconds.
   After sending a message it waits for a response. After receiving the
   response the client disconnects and tries to reconnect after 5 seconds.

   Copyright (C) 2013 by Norbert Truchsess <norbert.truchsess@t-online.de>
*/

#include <UIPEthernet.h>
#include <EEPROM.h>

#define LAN_MAX_BUFFER      64
#define UART_MAX_BUFFER     64

#define TAG_DHCP            0x10
#define TAG_LOCAL_IP        0x20
#define TAG_LOCAL_SN        0x30
#define TAG_LOCAL_GW        0x40
#define TAG_LOCAL_DNS       0x50
#define TAG_SERVER_IP       0x60
#define TAG_SERVER_PORT     0x70
#define TAG_SHOW_CON        0x80

#define DHCP_ENABLE         0x10
#define DHCP_DISABLE        0x20  //STATIC


EthernetClient client;
EthernetServer server = EthernetServer(1000);
EthernetClient clientConf;

typedef struct {
  byte tag;
  int length;
  byte value[10];
} data_cmd_t;

data_cmd_t data_cmd, data_cmd_buff;

byte lan_buff[LAN_MAX_BUFFER];
unsigned int lan_index_read = 0;
unsigned int lan_index_write = 0;
unsigned int lan_index_count = 0;

byte uart_buff[UART_MAX_BUFFER];
unsigned int uart_index_read = 0;
unsigned int uart_index_write = 0;
unsigned int uart_index_count = 0;

int len_update = 0;
int uart_update = 0;

int Config = 0;
int message_id = 0;

byte cmd_config_mode[3], buff_cmd[5];
//int i, j, k;
int cmd_count = 0;

void ee_get_dhcp(data_cmd_t *data);
void ee_get_ip(data_cmd_t *data);
void ee_get_subnet(data_cmd_t *data);
void ee_get_gateway(data_cmd_t *data);
void ee_get_dns_serv(data_cmd_t *data);
void ee_get_ip_serv(data_cmd_t *data);
void ee_get_port_serv(data_cmd_t *data);

void print_conf() ;

void setup() {

  Serial.begin(9600);

  //set_default_conf();
  //print_conf();

  uint8_t mac[6] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
  Ethernet.begin(mac);

  //  Serial.print("LIP: ");
  //  Serial.println(Ethernet.localIP());
  //  Serial.print("SN: ");
  //  Serial.println(Ethernet.subnetMask());
  //  Serial.print("GW: ");
  //  Serial.println(Ethernet.gatewayIP());
  //  Serial.print("DNS: ");
  //  Serial.println(Ethernet.dnsServerIP());

  server.begin();


  memset(lan_buff, 0, sizeof(lan_buff));
  memset(uart_buff, 0, sizeof(uart_buff));
  cmd_config_mode[0] = (const byte)'+';
  cmd_config_mode[1] = (const byte)'+';
  cmd_config_mode[2] = (const byte)'+';


}

/////////////////Set Network Config/////////////////
/**
 * @Function ee_set_dhcp
 * @Param dhcp
 * @ParamType byte
 * @Return none void
 */
void ee_set_dhcp(byte dhcp) {
  memset(&data_cmd, 0, sizeof(data_cmd_t));
  data_cmd.tag = 0x10;
  data_cmd.length = 1;
  data_cmd.value[0] = dhcp;//0c10=DHCP,0x20=Static

  EEPROM.put( 0, data_cmd );
}
////////////////////////////////////////////////////
/**
 * @Function ee_set_ip
 * @Param ip1 ip2 ip3 ip4
 * @ParamType byte byte byte byte
 * @Return none void
 */
void ee_set_ip(byte ip1, byte ip2, byte ip3, byte ip4) {
  memset(&data_cmd, 0, sizeof(data_cmd_t));
  data_cmd.tag = 0x20;
  data_cmd.length = 4;
  data_cmd.value[0] = ip1;
  data_cmd.value[1] = ip2;
  data_cmd.value[2] = ip3;
  data_cmd.value[3] = ip4;

  EEPROM.put( sizeof(data_cmd_t) * 1, data_cmd );
}
////////////////////////////////////////////////////
/**
 * @Function ee_set_subnet
 * @Param ip1 ip2 ip3 ip4
 * @ParamType byte byte byte byte
 * @Return none void
 */
void ee_set_subnet(byte ip1, byte ip2, byte ip3, byte ip4) {
  memset(&data_cmd, 0, sizeof(data_cmd_t));
  data_cmd.tag = 0x30;
  data_cmd.length = 4;
  data_cmd.value[0] = ip1;
  data_cmd.value[1] = ip2;
  data_cmd.value[2] = ip3;
  data_cmd.value[3] = ip4;

  EEPROM.put( sizeof(data_cmd_t) * 2, data_cmd );
}
////////////////////////////////////////////////////
/**
 * @Function ee_set_gateway
 * @Param ip1 ip2 ip3 ip4
 * @ParamType byte byte byte byte
 * @Return none void
 */
void ee_set_gateway(byte ip1, byte ip2, byte ip3, byte ip4) {
  memset(&data_cmd, 0, sizeof(data_cmd_t));
  data_cmd.tag = 0x40;
  data_cmd.length = 4;
  data_cmd.value[0] = ip1;
  data_cmd.value[1] = ip2;
  data_cmd.value[2] = ip3;
  data_cmd.value[3] = ip4;

  EEPROM.put( sizeof(data_cmd_t) * 3, data_cmd );
}
////////////////////////////////////////////////////
/**
 * @Function ee_set_dns_serv
 * @Param ip1 ip2 ip3 ip4
 * @ParamType byte byte byte byte
 * @Return none void
 */
void ee_set_dns_serv(byte ip1, byte ip2, byte ip3, byte ip4) {
  memset(&data_cmd, 0, sizeof(data_cmd_t));
  data_cmd.tag = 0x50;
  data_cmd.length = 4;
  data_cmd.value[0] = ip1;
  data_cmd.value[1] = ip2;
  data_cmd.value[2] = ip3;
  data_cmd.value[3] = ip4;

  EEPROM.put( sizeof(data_cmd_t) * 4, data_cmd );
}
////////////////////////////////////////////////////
/**
 * @Function ee_set_ip_serv
 * @Param ip1 ip2 ip3 ip4
 * @ParamType byte byte byte byte
 * @Return none void
 */
void ee_set_ip_serv(byte ip1, byte ip2, byte ip3, byte ip4) {
  memset(&data_cmd, 0, sizeof(data_cmd_t));
  data_cmd.tag = 0x60;
  data_cmd.length = 4;
  data_cmd.value[0] = ip1;
  data_cmd.value[1] = ip2;
  data_cmd.value[2] = ip3;
  data_cmd.value[3] = ip4;

  EEPROM.put( sizeof(data_cmd_t) * 5, data_cmd );
}
////////////////////////////////////////////////////
/**
 * @Function ee_set_port_serv
 * @Param port
 * @ParamType int
 * @Return none void
 */
void ee_set_port_serv(int port) {
  memset(&data_cmd, 0, sizeof(data_cmd_t));
  data_cmd.tag = 0x70;
  data_cmd.length = 2;
  memcpy(&data_cmd.value[0], &port, 2);

  EEPROM.put( sizeof(data_cmd_t) * 6, data_cmd );
}


/////////////////Get Network Config/////////////////
/**
 * @Function ee_get_dhcp
 * @Param data
 * @ParamType data_cmd_t*
 * @Return none void
 */
void ee_get_dhcp(data_cmd_t *data) {
  EEPROM.get( 0, *data );
}
////////////////////////////////////////////////////
/**
 * @Function ee_get_ip
 * @Param data
 * @ParamType data_cmd_t*
 * @Return none void
 */
void ee_get_ip(data_cmd_t *data) {
  EEPROM.get( sizeof(data_cmd_t) * 1, *data );
}
////////////////////////////////////////////////////
/**
 * @Function ee_get_subnet
 * @Param data
 * @ParamType data_cmd_t*
 * @Return none void
 */
void ee_get_subnet(data_cmd_t *data) {
  EEPROM.get( sizeof(data_cmd_t) * 2, *data );
}
////////////////////////////////////////////////////
/**
 * @Function ee_get_gateway
 * @Param data
 * @ParamType data_cmd_t*
 * @Return none void
 */
void ee_get_gateway(data_cmd_t *data) {
  EEPROM.get( sizeof(data_cmd_t) * 3, *data );
}
////////////////////////////////////////////////////
/**
 * @Function ee_get_dns_serv
 * @Param data
 * @ParamType data_cmd_t*
 * @Return none void
 */
void ee_get_dns_serv(data_cmd_t *data) {
  EEPROM.get( sizeof(data_cmd_t) * 4, *data );
}
////////////////////////////////////////////////////
/**
 * @Function ee_get_ip_serv
 * @Param data
 * @ParamType data_cmd_t*
 * @Return none void
 */
void ee_get_ip_serv(data_cmd_t *data) {
  EEPROM.get( sizeof(data_cmd_t) * 5, *data );
}
////////////////////////////////////////////////////
/**
 * @Function ee_get_port_serv
 * @Param data
 * @ParamType data_cmd_t*
 * @Return none void
 */
void ee_get_port_serv(data_cmd_t *data) {
  EEPROM.get( sizeof(data_cmd_t) * 6, *data );
}

void set_default_conf() {
  ee_set_dhcp((byte)0x10);
  ee_set_ip((byte)192, (byte)168, (byte)1, (byte)10);
  ee_set_subnet((byte)255, (byte)255, (byte)255, (byte)0);
  ee_set_gateway((byte)192, (byte)168, (byte)1, (byte)1);
  ee_set_dns_serv((byte)8, (byte)8, (byte)8, (byte)8);
  ee_set_ip_serv((byte)192, (byte)168, (byte)10, (byte)123);
  ee_set_port_serv((int)5000);
}

void print_conf() {
  ee_get_dhcp(&data_cmd_buff);
  //  Serial.write(data_cmd_buff.tag);
  //  Serial.write((int)data_cmd_buff.length);
  for (int d = 0; d < data_cmd_buff.length; d++) {
    Serial.write(data_cmd_buff.value[d]);
  }

  ee_get_ip(&data_cmd_buff);
  for (int d = 0; d < data_cmd_buff.length; d++) {
    Serial.write(data_cmd_buff.value[d]);
  }

  ee_get_subnet(&data_cmd_buff);
  for (int d = 0; d < data_cmd_buff.length; d++) {
    Serial.write(data_cmd_buff.value[d]);
  }

  ee_get_gateway(&data_cmd_buff);
  for (int d = 0; d < data_cmd_buff.length; d++) {
    Serial.write(data_cmd_buff.value[d]);
  }

  ee_get_dns_serv(&data_cmd_buff);
  for (int d = 0; d < data_cmd_buff.length; d++) {
    Serial.write(data_cmd_buff.value[d]);
  }

  ee_get_ip_serv(&data_cmd_buff);
  for (int d = 0; d < data_cmd_buff.length; d++) {
    Serial.write(data_cmd_buff.value[d]);
  }

  ee_get_port_serv(&data_cmd_buff);
  for (int d = 0; d < data_cmd_buff.length; d++) {
    Serial.write(data_cmd_buff.value[d]);
  }
}

void print_conf_port() {
  ee_get_dhcp(&data_cmd_buff);
  //  Serial.write(data_cmd_buff.tag);
  //  Serial.write((int)data_cmd_buff.length);
  for (int d = 0; d < data_cmd_buff.length; d++) {
    clientConf.write(data_cmd_buff.value[d]);
  }

  ee_get_ip(&data_cmd_buff);
  for (int d = 0; d < data_cmd_buff.length; d++) {
    clientConf.write(data_cmd_buff.value[d]);
  }

  ee_get_subnet(&data_cmd_buff);
  for (int d = 0; d < data_cmd_buff.length; d++) {
    clientConf.write(data_cmd_buff.value[d]);
  }

  ee_get_gateway(&data_cmd_buff);
  for (int d = 0; d < data_cmd_buff.length; d++) {
    Serial.write(data_cmd_buff.value[d]);
  }

  ee_get_dns_serv(&data_cmd_buff);
  for (int d = 0; d < data_cmd_buff.length; d++) {
    clientConf.write(data_cmd_buff.value[d]);
  }

  ee_get_ip_serv(&data_cmd_buff);
  for (int d = 0; d < data_cmd_buff.length; d++) {
    clientConf.write(data_cmd_buff.value[d]);
  }

  ee_get_port_serv(&data_cmd_buff);
  for (int d = 0; d < data_cmd_buff.length; d++) {
    clientConf.write(data_cmd_buff.value[d]);
  }
}




void loop() {
  byte b_buff;
  ee_get_ip(&data_cmd_buff);
  if (!client.connected()) {
    if (client.connect(IPAddress(192, 168, 10, 123), 5000))
    {
      //Serial.println("Client connected to server.");
      //      Serial.write(LAN_INFO);
      //      Serial.write((byte)2);
      //      Serial.write((byte)1);
      //      Serial.write(LAN_CONNECTED);
    } else {
      //Serial.println("Client connect failed");
      //      Serial.write(LAN_ERROR);//control packet
      //      Serial.write((byte)2);//lenght remain
      //      Serial.write((byte)1);//langth payload
      //      Serial.write(LAN_CONNECT_FAIL);//payload
    }
  }

  if (client.connected()) {
    lanUpdate();
    if (lan_index_count > 0) {
      len_update = lan_index_count;
      while (len_update--) {
        Serial.write(lan_buff[lan_index_read++]);
        lan_index_count--;
        if (lan_index_read == lan_index_write || (lan_index_count <= 0)) {
          lan_index_write = 0;
          lan_index_read = 0;
        }
      }
    }

    if (uart_index_count > 0) {
      uart_update = uart_index_count;
      while (uart_update--) {
        b_buff = (byte)uart_buff[uart_index_read++];
        uart_index_count--;
        if (uart_index_read == uart_index_write || (uart_index_count <= 0)) {
          uart_index_write = 0;
          uart_index_read = 0;
        }

        buff_cmd[cmd_count++] = (byte)b_buff;
        
        if(TAG_SHOW_CON == (byte)buff_cmd[0]){
              print_conf();
              cmd_count = 0;            
        }
        if((byte)buff_cmd[0] == '-' && (byte)buff_cmd[1] == '-' && (byte)buff_cmd[2] == '-'){
          Config = 0;
          Serial.println("EC");
        }
        if (Config != 1) {
          if (cmd_count >= 3) {
            if (((byte)buff_cmd[0] == (byte)cmd_config_mode[0]) && ((byte)buff_cmd[1] == (byte)cmd_config_mode[1]) && ((byte)buff_cmd[2] == (byte)cmd_config_mode[2])) {
              Serial.println("C");
              cmd_count = 0;
              Config = 1;
              //break;
            }
            cmd_count = 0;
          }
        }
        else {
            if (TAG_DHCP == (byte)buff_cmd[0]) {
              //Serial.println("DHCP mode.");
            }
            else if (TAG_LOCAL_IP == (byte)buff_cmd[0]) { 

              ee_set_ip((byte)buff_cmd[1],(byte)buff_cmd[2],(byte)buff_cmd[3],(byte)buff_cmd[4]);
              cmd_count = 0;
            }
            else if (TAG_LOCAL_SN == (byte)buff_cmd[0]) {
              ee_set_subnet((byte)buff_cmd[1],(byte)buff_cmd[2],(byte)buff_cmd[3],(byte)buff_cmd[4]);
              cmd_count = 0;
            }
            else if (TAG_LOCAL_GW == (byte)buff_cmd[0]) {
              ee_set_gateway((byte)buff_cmd[1],(byte)buff_cmd[2],(byte)buff_cmd[3],(byte)buff_cmd[4]);
              cmd_count = 0;
            }
            else if (TAG_LOCAL_DNS == (byte)buff_cmd[0]) {
              ee_set_dns_serv((byte)buff_cmd[1],(byte)buff_cmd[2],(byte)buff_cmd[3],(byte)buff_cmd[4]);
              cmd_count = 0;
            }
            else if (TAG_SERVER_IP ==  (byte)buff_cmd[0]) {
              ee_set_ip_serv((byte)buff_cmd[1],(byte)buff_cmd[2],(byte)buff_cmd[3],(byte)buff_cmd[4]);
              cmd_count = 0;
            }
            else if (TAG_SERVER_PORT ==  (byte)buff_cmd[0]) {
              ee_set_port_serv((byte)buff_cmd[1]);
              cmd_count = 0;
            }
            
            
        }
        client.write(b_buff);
      }
     
  
    }


  }


  ///////////////////Server Part///////////////////////
  size_t size;

  if (clientConf = server.available())
  {
    while ((size = clientConf.available()) > 0)
    {
      uint8_t* msg = (uint8_t*)malloc(size);
      size = clientConf.read(msg, size);
      //Serial.write(msg, size);
      print_conf_port();
      free(msg);
    }
    //clientConf.println("DATA from Server!");
    //clientConf.stop();
  }
}

void serialEvent() {

  while (Serial.available()) {
    uart_buff[uart_index_write++] = (byte)Serial.read();
    uart_index_count++;
    if (uart_index_write >= UART_MAX_BUFFER) {
      uart_index_read = 0;
      uart_index_write = 0;
    }
  }
}


void lanUpdate() {
  unsigned char b_data;
  while (client.available()) {
    client.read(&b_data, 1);
    //Serial.write(b_data);
    lan_buff[lan_index_write++] = (byte)b_data;
    lan_index_count++;
    if (lan_index_write >= LAN_MAX_BUFFER) {
      lan_index_read = 0;
      lan_index_write = 0;
    }

    if (lan_index_read >= LAN_MAX_BUFFER) {
      lan_index_read = 0;
      lan_index_write = 0;
    }
  }
}

