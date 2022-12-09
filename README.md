<h1 align="center">Welcome to the M5 Stack (ESP32) Project 👋</h1>
<p>
<img alt="Version" src="https://img.shields.io/badge/version-1.0.0-blue.svg?cacheSeconds=2592000" />
<a href="https://github.com/TheToto318/IoT_stack/blob/main/README.md" target="\_blank">
<img alt="Documentation" src="https://img.shields.io/badge/documentation-yes-brightgreen.svg" />
</a>
<a href="https://github.com/TheToto318/IoT_stack/graphs/commit-activity" target="\_blank">
<img alt="Maintenance" src="https://img.shields.io/badge/Maintained%3F-yes-green.svg" />
</a>
<a href="https://github.com/TheToto318/IoT_stack/blob/main/LICENSE" target="\_blank">
<img alt="License: MIT" src="https://img.shields.io/github/license/TheToto318/IoT_Stack" />
</a>
</p>

> Create a layer two and three protocol to establish connection between M5 Stack nodes by air using the RadioHead library

### 🏠 [Homepage](https://github.com/TheToto318/SAE32)

## Project details

Library used :

* RadioHead : RF95 (http://www.airspayce.com/mikem/arduino/RadioHead/classRH__RF95.html)
* TFT Terminal for display scrolling
* M5Stack

## Features
* All in one script for sender and receiver mode.
* Interactive set up including :
    * Transmission power
    * Frequency
    * Modulation type
    * Receiver address in listen mode.
    * Receiver address, sender address, message template in emitter mode.
* Error detection code 
* Reed Solomon redundancy (correcting error)
* Adaptative payload
* ACK system + ALOAH

## Automate scheme

![Workflow](./Scheme/Automate%20Scheme.drawio.png)

## Frame diagram

![Workflow](./Scheme/Frame%20diagram.drawio.png)

## Frame transfer diagram

![Workflow](./Scheme/Frame%20transfer.drawio.png)