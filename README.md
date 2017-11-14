# ESP8266 WebSocket wifiCar
Wifi Controlled Car Aruduino Sketch (ESP8266 ESP-WROOM-02)

YouTube
https://www.youtube.com/watch?v=sxLcQ3OFHZo

WiFiチップESP8266搭載のWiFiモジュールEPS-WROOM-02でWiFiラジコンカーを作ってみた。

WiFiラジコンの特徴

・スマホのWebブラウザで操作する。
　従ってAppストアやPlayストアからアプリをダウンロードする必要なし。
　（開発者としては、面倒な審査を受けなくてよい。）
　車自体がWiFiアクセスポイント&Webサーバになっており、
　WiFi接続後、WebブラウザにURLとしてIPアドレスを入力すると操作画面が表示される。
・基本的にはハンドル用スライダと速度用スライダを左手と右手の親指で操作するが、
　加速度センサーの値をもとに、スマホの前後左右の傾きでコントロールすることもできる。
・複数のスマホで１台をコントロールできる。（混信ではない）
・スマホからのハートビート信号が届かなくなったら（見通し距離５０ｍ程度）リセットがかかり、停止する。
・WiFi接続時のパスワードを設定することもできる。初期状態はオープン。
　起動時またはリセット時のLED点滅中(2秒)に設定スイッチを押すことで、設定モードに移行する。
　設定モード中にスマホでWiFi接続後、WebブラウザでURLとしてIPアドレスを入力すると
　WiFiパスワード設定画面が表示される。
　尚、設定モードにすると、WiFiパスワードが設定されていた場合はクリアする。
　（こうしておかないと、WiFiパスワードを忘れてしまった場合、２度と接続できなくなってしまうので。）
・コントロール信号、ハートビート信号はWebSocketを使って送信している。
・DCブラシモーターのノイズ対策として制御回路系電源と駆動系電源を完全分離。
　PWM信号はフォトカプラで伝達。

・主な部品
　車体はトイザらス ファストレーン エックスワイルド（2500円)
　　もとの制御基盤は取り外す
　バッテリーはBUFFALOの2600mA 2.0A BSMPB2602P1BK（Amazonで1600円)
　ESP-WROOM-02（秋月で650円）
　三端子レギュレーター3.3V500mA（秋月で100円）
　ＵＳＢシリアル変換モジュール(秋月で600円）
　フォトカプラ(TLP785)
　モータドライバ(TA7291P)
　サーボモータ(SG-90)
　LED、コンデンサー、抵抗、タクトスイッチ、USBケーブル
 
 C:\Users\koba_\Documents\Arduino\kobayashi\WebSocketServer_CarControl_softAP799
 
© 2017 GitHub, Inc.
