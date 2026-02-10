# **Navigation light and Sound Signals** **ECU**

## 2026-01-29

Ib Havn

# **Introduction**

This document is a project proposal for an ECU to control navigation lights and sound signals on a pleasure boat  under 15m..

# **Goal**

1. Control navigation lights based on the state the boat and conditions the boat is sailing under.

2. Control the signal equipment (horn/whistle) based on the state and conditions the boat is sailing under.

3. Semi-automatic control of the horn to emit different ad hoc signals (blasts) depending on a given situation that may arise between the boat and other boats.

# **Background information**

The ECU must follow the rules for lantern lighting  and sound signal set out by the International Maritime Organization IMO.

The conditions and the states in which a boat can sail are defined in *Convention on the International Regulations for Preventing Collisions at Sea, 1972 (COLREGs)*

* Day  
* Hours of darkness  
* Restricted visibility

In addition, a boat can be in the following states:

* Moored  
* Underway, Making no way  
* Underway, Making way  
* Anchorage  
* Not under command (NUC) , Making no way  
* Not under command (NUC) , Making way

The rules for lantern use and signaling are summarized in the table below.

|  | Underway |  | Not under command (NUC) |  | Moored | Anchorage | Towing |
| ----- | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
|  | ***Making no way*** | ***Making way*** | ***Making no way*** | ***Making way*** |  |  |
| **Daylight** | Nothing | Nothing | ⬤<br>⬤<br>(Two black balls) | ⬤<br>⬤<br>(Two black balls) | Nothing | ⬤<br>(One black ball) | 
| **Hours of darkness** | Sidelights, Sternlight, Masthead light | Sidelights, Sternlight, Masthead light | ⬤<br>⬤<br>Two all-round red lights in a vertical line | Sidelights, Sternlight<br>⬤<br>⬤<br>Two all-round red lights in a vertical line | Nothing | All-round white light | Sidelights, Sternlight, Masthead light,<br>A yellow towing light |
| **Restricted visibility** | Sidelights, Sternlight, Masthead light Sound signal: ▬▬ ▬▬/2 min (Rule 35b) | Sidelights, Sternlight, Masthead light<br><br>Sound signal:<br>▬▬/2min (Rule 35a) | Sidelights, Sternlight <br>⬤<br>⬤<br>Two all-round red lights in a vertical line<br><br>Sound signal:<br>▬▬ ●● (Rule 35c) | ⬤<br>⬤<br>Two all-round red lights in a vertical line<br><br>Sound signal:<br>▬▬ ●● (Rule 35c) | Nothing | All-round white lightSound signal:<br>● ▬▬ ● (Warning) | Sidelights, Sternlight, Masthead light,<br>A yellow towing light <br><br>Sound signal:<br>▬▬ ●● (Rule 35c) |

**Explanation of the different lights:**

| Term | COLREGs Definition |
| :---- | :---- |
| **Masthead light** | A white light showing an unbroken light over an arc of the horizon of 225° and so fixed as to show the light from right ahead to 22.5° abaft the beam on either side. |
| **Sidelights** | A green light on the starboard side and a red light on the port side each showing an unbroken light over an arc of the horizon of 112.5°. |
| **Sternlight** | A white light showing an unbroken light over an arc of the horizon of 135° and so fixed as to show the light 67.5° from right aft on each side. |
| **All-round light** | A light showing an unbroken light over an arc of the horizon of 360°. |
| **Not under command (NUC) light** | Two red 360° lights placed high and vertically above each other. |
| **Towing light** |  A yellow light placed above Sternlight showing an unbroken light over an arc of the horizon of 135° and so fixed as to show the light 67.5° from right aft on each side. |

**Explanation of sound signals:**

| Term | COLREGs Definition |
| ----- | :---- |
| **●** | Short blast: A blast of approximately 1 second duration |
| **▬▬**  | Prolonged blast: A blast of from 4 to 6 seconds duration. |
| **Pause** | Duration 1 second |

**Other audio signals** that are not dependent on the conditions, but which may be desired to be given in a given situation. These signals are hereinafter referred to as semi-automatic signals. These can be given ad hoc and will not automatically repeat.

**Ad-hoc Signal Queueing**: If an ad-hoc signal is requested while another signal (periodic or ad-hoc) is currently playing, the new signal will be queued and automatically played after a 2-second delay once the current signal completes. This prevents overlapping horn signals and maintains clear, distinct COLREGs-compliant signaling. Only one ad-hoc signal can be queued at a time.

| Situation/Cause | Sound signal |
| ----- | :---: |
| **I turn to Starboard** | **●** |
| **I turn to port** | **●●** |
| **I am operating astern propulsion** | **●●●** |
| **I don't understand you/Danger** | **●●●●●** |
| **Pay Attention** | **▬▬**  |
| **Wants to overtake on starboard** | **▬▬ ▬▬ ●** |
| **Want to overtake to port** | **▬▬ ▬▬ ●●** |
| **Agreement to be overtaken** | **▬▬ ● ▬▬ ●** |

# 

# **Specifications**

## Hardware Platform

The platform consists of an AZ-Delivery ESP32 Dev Kit C V4 and a relay module with the number of relays necessary to control outputs for lights and horns.

It is intended that the relay contacts will later be mounted in parallel with the contacts already present in boats to control lights and horns.

## Software Platform

The system must be built on the SensEsp platform to be able to use the SignalK protocol as the normal way to communicate with the ECA.

## Communication

The ECU should normally be controlled via SignalK signals. This is done via WIFI.

It must be possible to see the status of what state the system, navigation lights and sound signals are in. This must be done by the ECU issuing SignalK telegrams with status.

On startup and when the SignalK connection is established the complete status of the ECU must be sent to the SignalK server.

## User interface

The system must be controllable by selecting the conditions under which the boat is sailing and the state of the boat (see Background Description). When this is selected, the system must automatically turn on the specified light and emit the correct sound signals according to the above table. The periodic sound signals must initially be muted when they are started. This means that there must be a function to mute and unmute the sound signals.

It is desirable to be able to track how much time is left until the periodic signals are sounded next time. This status must be a simple count down 1 sec counter that counts  down until the next period starts. This status should be accessible via SignalK telegram and the local interface.

The other ad hoc sound signals are described in the table Other sound signals must be able to be activated separately and only emitted once for each activation. If an ad-hoc signal is triggered while another signal is playing, it will be queued and automatically played after a 2-second delay when the current signal completes.

### Main UI

The main user interface consists of SignalK telegrams sent from a SignalK Server. Via this main UI it must be possible to control the different states, conditions  and sound signals. 

Every time the conditions or the bots state change this must be announced with SignalK telegrams to the server. 

### Fallback UI

A local web-based user interface running on the ECU itself must be provided for controlling the ECU when the SignalK connection is down. This fallback UI must be accessible from any device on the boat's WiFi network via web browser.

**Implementation**: Custom HTTP server endpoints on the ESP32 (via SensESP's built-in AsyncWebServer) serving a responsive HTML/JavaScript control page. The fallback UI provides the same functionality as the SignalK interface:
- Condition selection (Day / Hours of darkness / Restricted visibility)
- Boat state selection (Moored / Underway / Anchorage / NUC)
- Mute/unmute periodic signals
- Ad-hoc signal triggering (semi-automatic sound signals)
- Real-time status display (countdown timer, active lights, horn status)

**Access modes**:
- Normal: `http://nav-lights-ecu.local/lights` (when connected to boat WiFi)
- Emergency AP: `http://192.168.4.1/lights` (when ESP32 creates its own access point)

This approach provides:
- Universal access (any browser-capable device: phones, tablets, laptops)
- No app installation required
- Parallel operation with standard SensESP configuration UI
- Scalable pattern for other boat ECUs

## Security

When the system is started/booted, none of the relay outputs must be activated. This is ensured by ensuring that all control signals to the relays must be active low.

There must be a heartbeat controlling that the ECU is running as it should.
