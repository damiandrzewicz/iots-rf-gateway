<template><h1 id="iots-rf-core-library" tabindex="-1"><a class="header-anchor" href="#iots-rf-core-library" aria-hidden="true">#</a> IoTs RF Core Library</h1>
<h2 id="requirements" tabindex="-1"><a class="header-anchor" href="#requirements" aria-hidden="true">#</a> Requirements</h2>
<ul>
<li>low power consumption (battery powered devices):
<ul>
<li>voltage range is from 2.4V to 3.3V,</li>
<li>BOD below 1.8V,</li>
<li>deep sleep modes:
<ul>
<li>for some amount of time,</li>
<li>forever until interrupt,</li>
</ul>
</li>
</ul>
</li>
<li>possibility of connecting various peripheral devices,</li>
<li>send data to <code>gateway</code>,</li>
<li>receive data from <code>gateway</code>,</li>
<li>measure battery percent</li>
</ul>
<h2 id="hardware" tabindex="-1"><a class="header-anchor" href="#hardware" aria-hidden="true">#</a> Hardware</h2>
<h3 id="circuit" tabindex="-1"><a class="header-anchor" href="#circuit" aria-hidden="true">#</a> Circuit</h3>
<p>Hadrware configuration is based on <code>Moteino R6</code> board referenced <a href="https://lowpowerlab.com/shop/product/99" target="_blank" rel="noopener noreferrer">here<ExternalLinkIcon/></a>. It is <code>Atmega328p</code> onboard. Board integrates <code>RFM69</code> radio modules. There are two variations of these modules: <code>5V/16MHz</code> and <code>3.3V/8MHz</code>. Unfortunately internal <code>8MHz</code> oscillator is inaccurate and sometimes serial communication issues occurs, especially with bootloader data exchange.</p>
<p><img src="@source/hardware/iots-rf-core/MoteinoR6_front.jpg" alt="moteino R6">
<img src="@source/hardware/iots-rf-core/MoteinoR6_Schematic.png" alt="moteino R6"></p>
<p>Solution for above issues is to remove voltage regulator pointed in circle 1 and replace external 16MHz oscillator to <code>7,3628MHz</code> UART friendly frequency. For <code>8MHz</code> frequency with baud rate <code>57600</code> which is default value of <code>Optiboot</code> bootloader, error is about 3.7%. When baud is switched to <code>38400</code> situation is better but still not perfect. Error is about 0.2% and it depends on temperature of AVR controller. When external 7,3628MHz is used then frequency is really stable and UART error is 0%. That is the best situation.</p>
<p><img src="@source/hardware/iots-rf-core/avr_uart_freq.png" alt="moteino R6"></p>
<p>For IoTs RF Core external oscillator is replaced to <code>7,3628MHz</code> and voltage regulator is removed and missing connections are jumped. Before this modification (with 8MHz internal oscillator) programming by FTDI adapter wasn't possible on 4 from 20 boards. Communication wasn't reliable. After misidifactions communication is 100% reliable and all boards were flashed sucessfully.</p>
<h3 id="bootloader" tabindex="-1"><a class="header-anchor" href="#bootloader" aria-hidden="true">#</a> Bootloader</h3>
<p>As Bootloader default <code>Arduino</code> <a href="https://github.com/Optiboot/optiboot" target="_blank" rel="noopener noreferrer">optiboot<ExternalLinkIcon/></a> is used. Binary needs to be build referring to previous frequency changes by following command:</p>
<div class="language-text ext-text line-numbers-mode"><pre v-pre class="language-text"><code>make AVR_FREQ=7372800L BAUD_RATE=115200 atmega328
</code></pre><div class="line-numbers"><span class="line-number">1</span><br></div></div><p>In the end following <code>platformio</code> configuration is used for flash fuses and bootloader:</p>
<div class="language-text ext-text line-numbers-mode"><pre v-pre class="language-text"><code>[env:bootloader]
platform = atmelavr
board = moteino8mhz
framework = arduino
upload_protocol = usbasp
;Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms; [CKSEL=1100 SUT=11] 
board_fuses.lfuse = 0xFC
board_fuses.hfuse = 0xDC
board_fuses.efuse = 0xFE
board_bootloader.lfuse = 0xFC
board_bootloader.hfuse = 0xDC
board_bootloader.efuse = 0xFE
;Bootloader file HEX
board_bootloader.file = bootloaders/optiboot_7370000hz_115200b_atmega328.hex
</code></pre><div class="line-numbers"><span class="line-number">1</span><br><span class="line-number">2</span><br><span class="line-number">3</span><br><span class="line-number">4</span><br><span class="line-number">5</span><br><span class="line-number">6</span><br><span class="line-number">7</span><br><span class="line-number">8</span><br><span class="line-number">9</span><br><span class="line-number">10</span><br><span class="line-number">11</span><br><span class="line-number">12</span><br><span class="line-number">13</span><br><span class="line-number">14</span><br></div></div><h2 id="configuration" tabindex="-1"><a class="header-anchor" href="#configuration" aria-hidden="true">#</a> Configuration</h2>
<p>Configuration is an interface class of following pure virtual operations:</p>
<p>@startuml
interface Config {
+{abstract} ~Config()</p>
<p>+{abstract} void save()
+{abstract} void read()
+{abstract} void clear()</p>
<p>+{abstract} bool isEmpty()
+{abstract} void setEmpty()
+{abstract} void setDefaults()
}
@enduml</p>
<h3 id="eeprom-configuration" tabindex="-1"><a class="header-anchor" href="#eeprom-configuration" aria-hidden="true">#</a> EEPROM Configuration</h3>
<p>Every node can save its data to <code>EEPROM</code> memory. It is necessary due to keep data when power is down and for &quot;cacheing&quot; objects. &quot;Cacheing&quot; means when data are saved in unvolatile 'EEPROM' memory it's not necessary to ask Gateway for data. They can be read from &quot;memory&quot;. Base abstract class is <code>EEPROMConfig</code> which is a template class.</p>
<p>@startuml
class EEPROMConfig<T> {
+EEPROMConfig(int address)
+{abstract} ~EEPROMConfig()</p>
<p>+void save() override
+void read() override
+void clear() override</p>
<p>+T &amp;data()
+size_t dataSize()</p>
<p>-T data_
-int address_;
}</p>
<p>interface Config{}
class EEPROMConfig<T> implements Config
@enduml</p>
<p>Template has 3 methods for saving, reading and clearing data directly in <code>EEPROM</code> memory. It contains three additional ethods for checking if structure of type <code>T</code> is empty, setting it empty and setting defaults. Constructor argument is starting memory address of initialized structure.</p>
<div class="custom-container warning"><p class="custom-container-title">WARNING</p>
<p>Remember to do not allocate another config class with address which conflicts previously allocated <code>EEPROM</code> memory. For example if first class is allocated at address 0 and has size 12 bytes and second config is allocated at address 8, then it will due to undefined behaviour. That's a reason why method <code>dataSize</code> is introduced.</p>
</div>
<div class="custom-container warning"><p class="custom-container-title">WARNING</p>
<p>Structure cannot has <code>virtual</code> methods, because this kind of objects cannot be restored from <code>EEPROM</code> memory.</p>
</div>
<h4 id="radioconfig-eeprom-implementation" tabindex="-1"><a class="header-anchor" href="#radioconfig-eeprom-implementation" aria-hidden="true">#</a> RadioConfig EEPROM implementation</h4>
<p>@startuml
class EEPROMConfig<RadioConfigData></p>
<p>class RadioConfig{
bool isEmpty() override
void setEmpty() override
void setDefaults() override
}</p>
<p>class RadioConfigData{
+networkId: uint8_t
+gatewayId: uint8_t
+customFrequency: unsigned long
+encryptKey[17]: char
+powerLevel: int8_t
}</p>
<p>EEPROMConfig &lt;|-- RadioConfig
EEPROMConfig *-- RadioConfigData
@enduml</p>
<h4 id="uuidconfig-eeprom-implementation" tabindex="-1"><a class="header-anchor" href="#uuidconfig-eeprom-implementation" aria-hidden="true">#</a> UUIDConfig EEPROM implementation</h4>
<p>@startuml
class EEPROMConfig<UUIDConfigData></p>
<p>class UUIDConfig{
bool isEmpty() override
void setEmpty() override
void setDefaults() override
}</p>
<p>class UUIDConfigData{
+uuid[16]: byte
}</p>
<p>EEPROMConfig &lt;|-- UUIDConfig
EEPROMConfig *-- UUIDConfigData
@enduml</p>
<h2 id="node-modes" tabindex="-1"><a class="header-anchor" href="#node-modes" aria-hidden="true">#</a> Node Modes</h2>
<h3 id="state-mode" tabindex="-1"><a class="header-anchor" href="#state-mode" aria-hidden="true">#</a> State Mode</h3>
<p>@startuml
[*] -&gt; Awoken
VerifyConfig : before - read config
VerifyConfig --&gt; SleepUntilExtInt : failed\n&amp;&amp;\nstateBtn released
VerifyConfig --&gt; Awoken : success\n&amp;&amp;\nstateBtn pressed
SendRadio --&gt; ListenRadio : success
SendRadio --&gt; SleepForOrUntilExtInt : failed
SendRadio --&gt; SendRadio : repeat few times
ListenRadio : for 50 ms
ListenRadio --&gt; SleepForOrUntilExtInt : failed
ListenRadio --&gt; MessageHandle : success
MessageHandle --&gt; SleepForOrUntilExtInt : success/failed
SleepForOrUntilExtInt : choose sleep option for node
SleepForOrUntilExtInt --&gt; Awoken : success
Awoken --&gt; VerifyConfig : stateBtn released
MessageBuild --&gt; SendRadio : success
Awoken --&gt; Pairing : btn pressed for 1s
Awoken --&gt; FactoryReset : btn pressed for 15s
Awoken --&gt; VerifyConfig
VerifyConfig --&gt; MessageBuild : success
FactoryReset --&gt; VerifyConfig : success
Awoken --&gt; RadioReset : btn pressed for 10s
RadioReset -&gt; VerifyConfig : success
Pairing : enter: start blink\non:pair
Pairing --&gt; VerifyConfig : success
Pairing --&gt; SleepUntilExtInt : failed
SleepUntilExtInt --&gt; Awoken : stateBtn pressed
SleepUntilExtInt --&gt; SleepUntilExtInt : stateBtn released\nfake wakeup
@enduml</p>
<p>@startuml</p>
<p>state Configuring {
[*] --&gt; VerifyConfig
VerifyConfig --&gt; SleepUntilExtInt : failed</p>
<p>RadioPairing --&gt; VerifyConfig : success
RadioPairing --&gt; SleepUntilExtInt : failed</p>
<p>RadioReset --&gt; VerifyConfig : success
RadioReset --&gt; SleepUntilExtInt : failed</p>
<p>FactoryReset --&gt; VerifyConfig : success
FactoryReset --&gt; SleepUntilExtInt : failed</p>
<p>}</p>
<p>SleepUntilExtInt --&gt; CheckStateBtn : success
CheckStateBtn --&gt; VerifyConfig : stateBtn HIGH\n&amp;&amp;\ndebounce timeout
CheckStateBtn --&gt; RadioPairing : stateBtn min. 3s\n&amp;&amp;\ndebounce timeout
CheckStateBtn --&gt; FactoryReset : stateBtn min. 15s\n&amp;&amp;\ndebounce timeout
CheckStateBtn --&gt; RadioReset : stateBtn min. 10s\n&amp;&amp;\ndebounce timeout</p>
<p>state Messaging {
VerifyConfig --&gt; MessageBuild : success
MessageBuild --&gt; SendToRadio : success
MessageBuild --&gt; SleepForOrUntilExtInt : failed
SendToRadio --&gt; ListenFromRadio : success
ListenFromRadio : for 50 ms
SendToRadio --&gt; SleepForOrUntilExtInt : failed
ListenFromRadio --&gt; MessageHandle : success
ListenFromRadio --&gt; SleepForOrUntilExtInt : failed
MessageHandle --&gt; SleepForOrUntilExtInt : success/failed
SleepForOrUntilExtInt --&gt; CheckStateBtn : success
}</p>
<p>@enduml</p>
<h3 id="work-mode" tabindex="-1"><a class="header-anchor" href="#work-mode" aria-hidden="true">#</a> Work Mode</h3>
</template>
