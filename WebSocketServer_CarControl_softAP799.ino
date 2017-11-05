//
//
// References:
// https://gist.github.com/bbx10/667e3d4f5f2c0831d00b
// https://github.com/Links2004/arduinoWebSockets
//
//

#include <Ticker.h>
#include <SPI.h>
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <ESP8266WebServer.h>
#include "FS.h"

ESP8266WiFiMulti WiFiMulti;

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(443);

Servo servo;

Ticker ticker1;

int settingSW; //設定モードスイッチ
int Ncon = 0;  //WebSocketのコネクション数
int Mterm = -1; //モーション制御できるマスター端末
int Sangle = 1500; //角度初期値
int Sspeed = 0; //スピード初期値
int Sbrake = 1; //ブレーキ初期値

int TimeOutAll = 0;   // 全端末からの信号タイムアウト
int TimeOutMaster = 0; //マスタ端末からの信号タイムアウト

int MasterTimeOutFLG;

//----------------------------------------------------------
static const char PROGMEM INDEX_HTML_SET[] = R"rawliteral(
<!DOCTYPE html>
<html>
<meta charset="UTF-8"> 
<meta name="viewport" content="width=device-width,initial-scale=0.9,minimum-scale=0.9,maximum-scale=0.9,user-scalable=no">

<head>
<title>Car Controller with WebSocket</title>
</head>
<body>
<form action="wifipasswordset" method="POST">
  WiFi Password is Cleared<br><br>
  Please input new WiFi Password:<input type='text' name='password' value='' size=25 pattern="[a-zA-Z0-9!-/:-@¥[-`{-~]{8,20}"><br>
  (using half-width English numbers and letters, using more than 8 characters but less than 20.)<br><br>
  <button type='submit' name='action' value='save'>Save</button>
</form>
</body>
</html>
)rawliteral";

static const char PROGMEM INDEX_HTML_SET_OK[] = R"rawliteral(
<!DOCTYPE html>
<html>
<meta charset="UTF-8"> 
<meta name="viewport" content="width=device-width,initial-scale=0.9,minimum-scale=0.9,maximum-scale=0.9,user-scalable=no">

<head>
<title>Car Controller with WebSocket</title>
</head>
<body>
<form>
  WiFi Password is Saved<br>
  Please reset.<br>
</form>
</body>
</html>
)rawliteral";

static const char PROGMEM INDEX_HTML_SET_NG[] = R"rawliteral(
<!DOCTYPE html>
<html>
<meta charset="UTF-8"> 
<meta name="viewport" content="width=device-width,initial-scale=0.9,minimum-scale=0.9,maximum-scale=0.9,user-scalable=no">

<head>
<title>Car Controller with WebSocket</title>
</head>
<body>
<form>
  WiFi Password is Not Saved !!<br>
  Please reset and retry.<br>
</form>
</body>
</html>
)rawliteral";

static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<meta charset="UTF-8"> 
<meta name="viewport" content="width=device-width,initial-scale=0.9,minimum-scale=0.9,maximum-scale=0.9,user-scalable=no">

<head>
<title>Car Controller with WebSocket</title>
</head>
<body>
<script type="text/javascript">

//加速度移動平均用変数
var ia=0; //加速度移動平均カウンタ
var imax=10; //加速度移動平均要素数
var full=0; //加速度配列がフルになったかどうか
var xga=new Array(imax); // 直近imax個の加速度
var yga=new Array(imax);
var zga=new Array(imax);
var xsm=0, ysm=0, zsm=0; // 加速度移動合計
var xg, yg, zg;  // 加速度移動平均

// 同じ値は送信しない
var speedSV=0;
var angleSV=1500;

// 送った値がechobackされるまでは、次を送らない
// そうしないと、サーバの処理がおいつかず、Disconnectされてしまう。
var speedRCV=0;
var angleRCV=1500;

var speedSendFLG=0;
var angleSendFLG=0;
var signalSendFLG=0;

// WebSocket生成
var connection = new WebSocket('ws://' + window.location.hostname + ':443');

// WebSocketコネクション確立
connection.onopen = function () {
  setInterval("sendSignal()", 400);
  sendSignal();
};

// WebSocketコネクションエラー発生
connection.onerror = function (error) {
  //console.log('WebSocket Error ' + error);
};

// WebSocketメッセージ処理
connection.onmessage = function (e) {
  //console.log('Server: ' + e.data);
  var msg=e.data.split("|");
  if(msg[0]=="brake") {
    document.getElementById("BrakeArea").innerHTML = msg[1];
  } else if(msg[0]=="angle") {
    document.getElementById("AngleArea").innerHTML = msg[1];
    document.getElementById("angle").value=msg[1];
    angleRCV=msg[1];
    angleSendFLG=0;
  } else if(msg[0]=="speed") {
    document.getElementById("SpeedArea").innerHTML = msg[1];
    document.getElementById("speed").value=msg[1];
    speedRCV=msg[1];
    speedSendFLG=0;
  } else if(msg[0]=="master") {
    document.getElementById("MasterTermNo").innerHTML = msg[1];
  } else if(msg[0]=="myterm") {
    document.getElementById("MyTermNo").innerHTML = msg[1];
  } else if(msg[0]=="signal") {
    signalSendFLG=0;
  } else {
    document.getElementById("StatusArea").innerHTML = e.data;
  }
};

function sendSignal() {
  if(speedSendFLG==0 && angleSendFLG==0 && signalSendFLG==0) {
    connection.send('signal|0');
    signalSendFLG=1;
  }
}

function sendBrake() {
  if(document.getElementById("BrakeArea2").innerHTML=="1") {
    document.getElementById("BrakeArea2").innerHTML="0"
    connection.send('brake|0');
  } else {
    //window.removeEventListener("devicemotion", sendMotion, false);
    resetSpeed();
    //window.addEventListener("devicemotion", sendMotion, false);
  }
}

function sendAngle() {
  var Langle=document.getElementById("angle").value;
  if(angleRCV==angleSV) {
    if(angleSV != Langle) {
      //console.log('angle: ' + Langle);
      document.getElementById("AngleArea2").innerHTML = Langle;
      connection.send('angle|'+Langle);
      angleSV=Langle;
      angleSendFLG=1;
    }
  }
}

function sendSpeed() {
  var Lspeed=document.getElementById("speed").value
  if(speedRCV==speedSV) {
    if(speedSV != Lspeed) {
      //console.log('speed: ' + Lspeed);
      document.getElementById("SpeedArea2").innerHTML = Lspeed;
      connection.send('speed|'+Lspeed);
      speedSV=Lspeed;
      speedSendFLG=1;
    }
  }
}

function resetAngle() {
  console.log('angle:1500');
  document.getElementById("angle").value = 1500;
  document.getElementById("AngleArea2").innerHTML = 1500;
  connection.send('angle|1500');
  angleSV=1500;
  angleSendFLG=1;
}

function resetSpeed() {
  // ブレーキをかけるときは一度0,0にしてからでないとだめなようだ
  //speed 0
  document.getElementById("speed").value = 0;
  document.getElementById("SpeedArea2").innerHTML = 0;
  connection.send('speed|0');
  speedSV=0;
  speedSendFLG=1;
  //brake ON
  document.getElementById("BrakeArea2").innerHTML="1"
  connection.send('brake|1');
}

// onmousedown,ontouchstartイベントでスライダーの値をとると、
// 直前の値をとってきてしまうのでsetTimeoutで一定時間(1msec)待ってから取るようにした。
function sendAngleDelay() {
  setTimeout('sendAngle();', 1);
}

function sendSpeedDelay() {
  setTimeout('sendSpeed();', 1);
}

function setMaster() {
  var wterm=document.getElementById("MyTermNo").innerHTML;
  if(document.getElementById("MasterTermNo").innerHTML == "-1") {
    speedSV=speedRCV;
    angleSV=angleRCV;
    document.getElementById("SpeedArea2").innerHTML = document.getElementById("SpeedArea").innerHTML;
    document.getElementById("AngleArea2").innerHTML = document.getElementById("AngleArea").innerHTML;
    document.getElementById("BrakeArea2").innerHTML = document.getElementById("BrakeArea").innerHTML;
    connection.send('master|'+wterm);
  } else if(document.getElementById("MasterTermNo").innerHTML == wterm) {
    connection.send('master|-1');
  }
}

function sendMotion(event) {
  //加速度取得
  var xgw = event.accelerationIncludingGravity.x;
  var ygw = event.accelerationIncludingGravity.y;
  var zgw = event.accelerationIncludingGravity.z;

  //加速度移動平均
  if(full==0) {
    xga[ia]=xgw;
    yga[ia]=ygw;
    zga[ia]=zgw;
    xsm=xsm+xga[ia];
    ysm=ysm+yga[ia];
    zsm=zsm+zga[ia];
    ia=ia+1;
    xg=xsm/ia;
    yg=ysm/ia;
    zg=zsm/ia;
    if(ia==imax) {
      ia=0;
      full=1;
    }
  } else {
    xsm=xsm-xga[ia];
    ysm=ysm-yga[ia];
    zsm=zsm-zga[ia];
    xga[ia]=xgw;
    yga[ia]=ygw;
    zga[ia]=zgw;
    xsm=xsm+xga[ia];
    ysm=ysm+yga[ia];
    zsm=zsm+zga[ia];
    ia=ia+1;
    xg=xsm/imax;
    yg=ysm/imax;
    zg=zsm/imax;
    if(ia==imax) {
      ia=0;    
    }
  }
  //加速度の絶対値を求める
  var xyzg = Math.sqrt(Math.pow(xg,2)+Math.pow(yg,2)+Math.pow(zg,2));

  //加速度を表示
  document.getElementById("xg").innerHTML = "x="+xg.toFixed(2);
  document.getElementById("yg").innerHTML = "y="+yg.toFixed(2);
  document.getElementById("zg").innerHTML = "z="+zg.toFixed(2);
  document.getElementById("xyzg").innerHTML = "xyz="+xyzg.toFixed(2);

  //自端末がマスタでなければモーションコントロール不可
  if(document.getElementById("MyTermNo").innerHTML!=document.getElementById("MasterTermNo").innerHTML) {
    return;
  }

  //角度を送信
  var Langle=parseInt(1500.0-xg*333.0/4.0);
  if(Langle>1833) Langle=1833;
  if(Langle<1167 ) Langle=1167;
  if(angleRCV==angleSV) {
    if(angleSV != Langle) {
      //console.log('angle: ' + Langle);
      document.getElementById("AngleArea2").innerHTML = Langle;
      connection.send('angle|'+Langle);
      angleSV=Langle;
      angleSendFLG=1;
    }
  }

  //ブレーキがONならスピードコントロール不可
  if(document.getElementById("BrakeArea2").innerHTML=="1") {
    return;
  }

  //スピードを送信
  var Lspeed;
  if(yg>5.5) {
    Lspeed=-1023;
  } else if(yg>0.5) {  
    Lspeed=-parseInt((yg-0.5)*1023.0/5.0);
  } else if(yg>-0.5) {
    Lspeed=0;
  } else if(yg>-3.5) {
    Lspeed=-parseInt((yg+0.5)*1023.0/3.0);
  } else {
    Lspeed=1023;
  }
  if(speedRCV==speedSV) {
    if(speedSV != Lspeed) {
      //console.log('speed: ' + Lspeed);
      document.getElementById("SpeedArea2").innerHTML = Lspeed;
      connection.send('speed|'+Lspeed);
      speedSV=Lspeed;
      speedSendFLG=1;
    }
  }
}

window.onload=init;
function init(){
  document.form1.brake.ontouchstart=sendBrake;
  document.form1.steering.ontouchstart=sendAngleDelay;
  document.form1.steering.ontouchmove=sendAngleDelay;
  document.form1.steering.ontouchend=resetAngle;
  document.form1.axel.ontouchstart=sendSpeedDelay;
  document.form1.axel.ontouchmove=sendSpeedDelay;
  document.form1.axel.ontouchend=resetSpeed;
  document.form1.master.ontouchstart=setMaster;

  window.addEventListener("devicemotion", sendMotion, false);
}

</script>

<form name="form1">
<span id="StatusArea">not connected</span>
<span id="MyTermNo">MyTermNo</span>
<span id="MasterTermNo">MasterTermNo</span>

<br>
<span id="AngleArea" style="background-color: #f8e0e0; margin: 0px 0px 0px 180px; padding: 5px;">angle</span>
<span id="AngleArea2" style="background-color: #f8e0e0; margin: 0px 0px 0px 10px; padding: 5px;">1500</span>
<br>
<input id="angle" type="range" min="1167" max="1833" step="1" value="1500" name="steering" style="background-color: #f8e0e0; height: 60px; width: 200px; margin: 0px 5px 5px 20px; padding: 10px;"/>
<br>
<br>
<span id="BrakeArea" style="background-color: #ffff00; margin: 0px 0px 0px 100px; padding: 5px;">brake</span>
<span id="BrakeArea2" style="background-color: #ffff00; margin: 0px 0px 0px 10px; padding: 5px;">1</span>
<br>
<span id="SpeedArea" style="background-color: #0fe0e0; margin: 0px 0px 0px 200px; padding: 5px;">speed</span>
<span id="SpeedArea2" style="background-color: #0fe0e0; margin: 0px 0px 0px 0px; padding: 5px;">0</span>
<br>
<input type="button" value="brake" name="brake" style="background-color: #ffff00; height: 80px; width: 80px; margin: 0px 0px 0px 100px; padding: 10px;"/>
<br>
<input id="speed" type="range" min="-1023" max="1023" step="1" value="0" name="axel" style="background-color: #0fe0e0; height: 60px; width: 200px; margin: 0px 0px 0px 180px; padding: 10px; -webkit-transform: rotate(-90deg); transform: rotate(-90deg);"/>
<br>
<input type="button" value="master" name="master" style="background-color: #ff00ff; height: 50px; width: 80px; margin: 0px 0px 0px 100px; padding: 10px;"/>
<br>
<br>
<span id="xyzg">XYZA</span> <span id="xg">XA</span> <span id="yg">YA</span> <span id="zg">ZA</span>
</form>
</body>
</html>
)rawliteral";

//==========================================================

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  int i, ix, len, val, in1, in2;
  String str[2], command, strNcon, strMterm, strMyTerm;
  String strSangle, strSspeed, strSbrake;
  
  //Serial.printf("webSocketEvent(%d, %d, ...)\r\n", num, type);
  switch(type) {
    case WStype_DISCONNECTED:
      {
        Serial.printf("[%u] Disconnected!\r\n", num);
        Ncon=Ncon-1; //コネクション数カウントダウン
        strNcon="Connections="+String(Ncon);
        len=strNcon.length();
        Serial.printf("DIS:%s,%d\n", strNcon.c_str(),len);

        if(Ncon==0) {
          // 接続数がゼロなので停止させる
          analogWrite(5,0);
          analogWrite(12,0);
          analogWrite(13,0);
          delay(10);
          digitalWrite(12,HIGH);
          digitalWrite(13,HIGH);
          Mterm=-1;
          Sspeed=0;
          Sbrake=1;
          TimeOutAll=0;
          TimeOutMaster=0;
        } else {
          webSocket.broadcastTXT(strNcon.c_str(), len);
          if(Mterm==num) { //ディスコネクト端末がマスター端末
            Mterm=-1;
            strMterm="master|"+String(Mterm);
            len=strMterm.length();
            webSocket.broadcastTXT(strMterm.c_str(), len);
            TimeOutMaster=0;
          }
        }
      }
      break;

    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        strMyTerm="myterm|"+String(num); //自端末番号
        len=strMyTerm.length();
        webSocket.sendTXT(num, strMyTerm.c_str(), len);

        Ncon=Ncon+1; //コネクション数カウントアップ
        strNcon="Connections="+String(Ncon);
        len=strNcon.length();
        Serial.printf("CON:%s,%d\n", strNcon.c_str(),len);
        webSocket.broadcastTXT(strNcon.c_str(), len);

        strMterm="master|"+String(Mterm); //マスター端末
        len=strMterm.length();
        webSocket.broadcastTXT(strMterm.c_str(), len);

        strSangle="angle|"+String(Sangle);
        len=strSangle.length();
        webSocket.broadcastTXT(strSangle.c_str(), len);

        strSspeed="speed|"+String(Sspeed);
        len=strSspeed.length();
        webSocket.broadcastTXT(strSspeed.c_str(), len);
          
        strSbrake="brake|"+String(Sbrake);
        len=strSbrake.length();
        webSocket.broadcastTXT(strSbrake.c_str(), len);

        TimeOutAll=0;
        if(Mterm!=num) TimeOutMaster=0;
      }
      break;

    case WStype_TEXT:
      TimeOutAll=0;
      if(Mterm==num) TimeOutMaster=0;

      Serial.printf("[%u] get Text: %s\r\n", num, payload);

      //  コマンドとその値の取り出し
      len=strlen((const char*)payload);
      for(i=0;i<2;i++){
        str[i]="";
      }
      ix=0;
      for(i=0;i<len;i++) {
        if(memcmp((const char*)payload+i,"|",1)==0) {
          ix += 1;
        } else {
          str[ix] = str[ix]+String((char)payload[i]);
        }
      }
      command=str[0];
      val=str[1].toInt();

      // スピード
      if(command=="speed") {
        //Serial.printf("[%u] get Text: %s\r\n", num, payload);
        analogWrite(5,abs(val));
        Sspeed=val;
        if(val==0) {
          in1=0;
          in2=0;
        } else if(val>0) {
          in1=val;
          in2=0;
        } else {    
          in1=0;
          in2=-val;
        }
        analogWrite(12,in1);
        analogWrite(13,in2);

        webSocket.broadcastTXT(payload, length);

      // 方向
      } else if(command=="angle") {
        Sangle=val;
        //servo.write(val);
        servo.writeMicroseconds(val);
        webSocket.broadcastTXT(payload, length);

      // ブレーキ
      } else if(command=="brake") {
        //Serial.printf("[%u] get Text: %s\r\n", num, payload);
        Sbrake=val;
        if(Sbrake==1) {
          digitalWrite(12,HIGH);
          digitalWrite(13,HIGH);
        }
        webSocket.broadcastTXT(payload, length);

      // マスター端末
      } else if(command=="master") {
        Mterm=val;
        if(Mterm!=-1) TimeOutMaster=0;
        webSocket.broadcastTXT(payload, length);

      // シグナル
      } else if(command=="signal") {
        if(Mterm==num) {
          TimeOutMaster=0;
        } else if(MasterTimeOutFLG==1) {
          Serial.printf("TimeOutMaster==1\n");
          MasterTimeOutFLG=0;
          Mterm=-1;
          strMterm="master|"+String(Mterm);
          len=strMterm.length();
          webSocket.broadcastTXT(strMterm.c_str(), len);
        }
        webSocket.sendTXT(num, payload, length);

      // その他
      } else {
        webSocket.broadcastTXT(payload, length);
      }
      break;

    case WStype_BIN:
      // Serial.printf("[%u] get binary length: %u\r\n", num, length);
      // hexdump(payload, length);
      // // echo data back to browser
      // webSocket.sendBIN(num, payload, length);
      break;

    default:
      Serial.printf("Invalid WStype [%d]\r\n", type);
      break;
  }
}

//==========================================================

void signalCheck() {
    //pin16をresetにつないでいるのでpin16をpinModeでOUTPUTにしただけでresetがかかる。
    //これを利用してresetするようにした。
    if(Ncon!=0 && TimeOutAll==1) {
      pinMode(16,OUTPUT);
    }

    if(Mterm!=-1 && TimeOutMaster==1) {
      MasterTimeOutFLG=1;
    }

    TimeOutAll=1;
    if(Mterm!=-1) TimeOutMaster=1;
}

void handleRoot() {
  if(settingSW==LOW) {
    server.send(200, "text/html", INDEX_HTML_SET);
  } else {
    server.send(200, "text/html", INDEX_HTML);
  }
}

//==========================================================

void handleRoot2() {
  String passwd= server.arg("password");
  Serial.print("passwd=");
  Serial.println(passwd);
  File fd = SPIFFS.open("/WiFiPassword.txt", "w");
  if (!fd) {
    Serial.println("/WiFiPassword.txt open error");
    server.send(200, "text/html", INDEX_HTML_SET_NG);
  } else {
    fd.println(passwd);    
    fd.close();
    Serial.println("saved password is " + passwd);
    server.send(200, "text/html", INDEX_HTML_SET_OK);
  } 
}

//==========================================================

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send_P(404, "text/plain", message.c_str());
}

//==========================================================

void setup() {
  int i;

  Serial.begin(115200);
  delay(10);

  SPIFFS.begin();

  pinMode(0,INPUT);
  pinMode(5,OUTPUT);  // LED
  pinMode(12,OUTPUT); // TA7291 IN1
  pinMode(13,OUTPUT); // TA7291 IN2

  for(i=0; i<2000; i++) {  //resetされたことを示すためにLEDを数回点滅させる
    //reset後、2秒以内に設定スイッチを入れると設定モードになる
    settingSW=digitalRead(0);
    if(settingSW==LOW) break;

    if((i/100)%2==0) {
      digitalWrite(5,HIGH);
    } else {
      digitalWrite(5,LOW);
    }
    delay(1);
  }

  if(settingSW==LOW) {
    for(i=0; i<6; i++) {  //設定モードになったのでLEDを長周期で数回点滅させる
      if(i%2==0) {
        digitalWrite(5,HIGH);
      } else {
        digitalWrite(5,LOW);
      }
      delay(400);
    }
    SPIFFS.remove("/WiFiPassword.txt"); // Wifiパスワードを削除する
  }

  digitalWrite(5,HIGH); //LED点燈

  digitalWrite(12,HIGH);
  digitalWrite(13,HIGH);
  servo.attach(4);
  servo.write(1500);

  //WiFi.mode(WIFI_AP_STA);
  WiFi.mode(WIFI_AP);
  //***  
  Serial.println();
  Serial.println("Configuring access point...");

  //アクセスポイントのssidを'ESP8266==MACアドレス'にする
  byte mac[6];
  char macHEX[13];
  String prefix="ESP8266-";
  WiFi.macAddress(mac);
  //String APssid="ESP8266-"+String(mac[0],HEX)+String(mac[1],HEX)+String(mac[2],HEX)+String(mac[3],HEX)+String(mac[4],HEX)+String(mac[5],HEX);
  //上記のコードだとゼロサプレスされてしまう
  sprintf(macHEX,"%02x%02x%02x%02x%02x%02x\0",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
  String APssid=prefix+macHEX;
  Serial.println("SSID="+APssid);

  //アクセスポイントのpasswordはいまのところなし、設定からとってくるようにする
  String APpassword="";
  File fd = SPIFFS.open("/WiFiPassword.txt", "r");
  APpassword = fd.readString();
  Serial.println("Password="+APpassword);
  fd.close();

  APpassword.trim(); //改行を取り除く

  WiFi.softAP(APssid.c_str(), APpassword.c_str());
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  server.on("/", handleRoot);
  server.on("/wifipasswordset", handleRoot2);
  server.onNotFound(handleNotFound);
  server.begin();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  // ハートビートチェック
  ticker1.attach_ms(1000,signalCheck);

  digitalWrite(5,LOW); //LED消灯
}

//==========================================================

void loop() {
  webSocket.loop();
  server.handleClient();
  //signalCheck();
}
