# ESP8266 WebSocket wifiCar
Wifi Controlled Car Aruduino Sketch (ESP8266 ESP-WROOM-02)

YouTube
https://www.youtube.com/watch?v=sxLcQ3OFHZo

WiFiチップESP8266搭載のWiFiモジュールEPS-WROOM-02でWiFiラジコンカーを作ってみた。<BR>
##WiFiラジコンの特徴<BR>
・スマホのWebブラウザで操作する。<BR>
　従ってAppストアやPlayストアからアプリをダウンロードする必要なし。<BR>
　（開発者としては、面倒な審査を受けなくてよい。）<BR>
　車自体がWiFiアクセスポイント&Webサーバになっており、<BR>
　WiFi接続後、WebブラウザにURLとしてIPアドレスを入力すると操作画面が表示される。<BR>
・基本的にはハンドル用スライダと速度用スライダを左手と右手の親指で操作するが、<BR>
　加速度センサーの値をもとに、スマホの前後左右の傾きでコントロールすることもできる。<BR>
・複数のスマホで１台をコントロールできる。（混信ではない）<BR>
・スマホからのハートビート信号が届かなくなったら（見通し距離５０ｍ程度）リセットがかかり、停止する。<BR>
・WiFi接続時のパスワードを設定することもできる。初期状態はオープン。<BR>
　起動時またはリセット時のLED点滅中(2秒)に設定スイッチを押すことで、設定モードに移行する。<BR>
　設定モード中にスマホでWiFi接続後、WebブラウザでURLとしてIPアドレスを入力すると<BR>
　WiFiパスワード設定画面が表示される。<BR>
　尚、設定モードにすると、WiFiパスワードが設定されていた場合はクリアする。<BR>
　（こうしておかないと、WiFiパスワードを忘れてしまった場合、２度と接続できなくなってしまうので。）<BR>
・コントロール信号、ハートビート信号はWebSocketを使って送信している。<BR>
・DCブラシモーターのノイズ対策として制御回路系電源と駆動系電源を完全分離。<BR>
　PWM信号はフォトカプラで伝達。<BR>

・主な部品<BR>
　車体はトイザらス ファストレーン エックスワイルド（2500円)<BR>
　　もとの制御基盤は取り外す<BR>
　バッテリーはBUFFALOの2600mA 2.0A BSMPB2602P1BK（Amazonで1600円)<BR>
　ESP-WROOM-02（秋月で650円）<BR>
　三端子レギュレーター3.3V500mA（秋月で100円）<BR>
　ＵＳＢシリアル変換モジュール(秋月で600円）<BR>
　フォトカプラ(TLP785)<BR>
　モータドライバ(TA7291P)<BR>
　サーボモータ(SG-90)<BR>
　LED、コンデンサー、抵抗、タクトスイッチ、USBケーブル<BR>
 <BR>
 C:\Users\koba_\Documents\Arduino\kobayashi\WebSocketServer_CarControl_softAP799<BR>
 
© 2017 GitHub, Inc.
