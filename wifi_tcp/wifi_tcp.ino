#include <ESP8266WiFi.h>


const char* ssid = "slabs";
const char* password = "huramamyinternety";

String arg_tab[4];
unsigned char buff[512];
unsigned char buff_r[512];
unsigned int rsize = 0;
unsigned int rtail = 0;

WiFiClient client;

void setup() {
  Serial.begin(115200);
  delay(10);

//  // We start by connecting to a WiFi network
//
//  Serial.println();
//  Serial.println();
//  Serial.print("Connecting to ");
//  Serial.println(ssid);
//
//  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
//     would try to act as both a client and an access-point and could cause
//     network-issues with your other WiFi-devices on your WiFi-network. */
//  WiFi.mode(WIFI_STA);
//  WiFi.begin(ssid, password);
//
//  while (WiFi.status() != WL_CONNECTED) {
//    delay(500);
//    Serial.print(".");
//  }
//
//  Serial.println("");
//  Serial.println("WiFi connected");
//  Serial.println("IP address: ");
//  Serial.println(WiFi.localIP());
}

bool read_cmd(String *ptr) {
  int rbyte = 0;
  bool cr = false;

  while(true) {
    if(Serial.available() > 0) {
         rbyte = Serial.read();
         if(rbyte == '\r') {
          cr = true;
          continue;
         }
         if(rbyte == '\n' && cr == true) {
          return true;
         } else {
          cr = false;
         }
         *ptr += (char) rbyte;
    }
  }
}

bool read_stream(unsigned char *data, unsigned int const len) {

unsigned int l = 0;

  while(true) {
    if(Serial.available() > 0) {
      data[l] = Serial.read();
      l++;
    }
    if(l >= len) return true;
  }
}

// connect_ap slabs huramamyinternety
// connect_tcp 192.168.1.72 1883
// send_stream 10
// 0123456789
String* parse_cmd(String *cmd) {
  int index = 0;
  int start_index = 0;
  int i = 0;
  int ends = 0;
  while(true) {
    index = cmd->indexOf(' ', start_index);
    if(index < 0) break;
    arg_tab[i] = cmd->substring(start_index, index);
    start_index = index + 1;
    i++;
  }
  arg_tab[i] = cmd->substring(start_index);
  return arg_tab;
}

bool connect_ap(String *argvector) {
//  Serial.println("mode");
  WiFi.mode(WIFI_STA);
//  Serial.println("begin");
  
  WiFi.begin(argvector[1].c_str(), argvector[2].c_str());
//  Serial.println("begined");

  unsigned int timeout = 20;

  while (timeout) {
    if(WiFi.status() == WL_CONNECTED) {
      return true;
    }
    delay(500);
    timeout--;
  }
  return false;  
}

bool connect_tcp(String *argvector) {

  return client.connect(argvector[1].c_str(), argvector[2].toInt());
  
}

bool disconnect_tcp(String *argvector) {
  (void) argvector;
  client.stop();
  return true;
}

bool clear(String *argvector) {
  (void) argvector;
  memset(buff_r, 0, 512);
  rsize = 0;
  rtail = 0;
  return true;
}

bool receive_stream(String *argvector) {
  unsigned int len = argvector[1].toInt();
  if(len == 0) {
    len = rsize;
    rsize = 0;
    rtail = 0;
    return true;
  }
  if(len == Serial.write(&buff_r[rtail], len)) {
    rtail += len;
    return true;
  }

  return false;
}

bool send_stream(String *argvector) {
  unsigned int len = argvector[1].toInt();
  if(read_stream(buff, len)) {
    clear(NULL);
    int written = client.write(buff, len);
//    Serial.print("Written: ");
//    Serial.println(written);
    if(len == written){
      return true;
    }
  }
  return false;
}


bool execute_cmd(String *argvector) {
//  Serial.print("Execute_cmd: ");
//  Serial.print(argvector[0]);
//  Serial.print(',');
//  Serial.print(argvector[1]);
//  Serial.print(',');
//  Serial.print(argvector[2]);
//  Serial.print(',');
  if(argvector[0].equals("connect_ap")) {
    return connect_ap(argvector);
  }
  else if(argvector[0].equals("connect_tcp")) {
    return connect_tcp(argvector);
  }
  else if(argvector[0].equals("disconnect_tcp")) {
    return disconnect_tcp(argvector);
  }
  else if(argvector[0].equals("send_stream")) {
    return send_stream(argvector);
  }
  else if(argvector[0].equals("receive_stream")) {
    return receive_stream(argvector);
  }
  else if(argvector[0].equals("clear")) {
    return clear(argvector);
  }
  else {
    return false;
  }
}

String *str = new String();
String *cmd_vector;

int rbyte = 0;
bool cr = false;
bool special_char = false;
long int s=0;

void loop() {

  if(Serial.available() > 0) {
    rbyte = Serial.read();
    if(rbyte == '\r') {
      cr = true;
      special_char = true;
    }
    else if(rbyte == '\n' && cr == true) {
      // has received complete command -> invoke it
      cmd_vector = parse_cmd(str);

      if(execute_cmd(cmd_vector)) {
        Serial.print("OK\r\n");
      }
      else {
        Serial.print("ERROR\r\n");
      }
      delete str;
      str = new String();
      special_char = true;
    } else {
      cr = false;
      special_char = false;
    }
    if(!special_char) {
      *str += (char) rbyte;
    }
  }

  s = client.available();
  if (s > 0) {
    if (s + rsize < 512) {
      client.read(&buff_r[rsize], s);
      rsize += s;
    } else {
      // ugly clean
      memset(buff_r, 0, 512);
      rsize = 0;
      rtail = 0;
    }
  }


}
