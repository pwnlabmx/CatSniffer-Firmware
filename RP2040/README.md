# LoRa-CAD
LoRa Channel Activity Detection.

## Usage
Load the firmware into the RP2040. Once you connect to the board using a serial monitor will start listening for the preamble.
```bash
# Available commands are:
set_freqSet the frequency in range of 150/960 MHz: Default 903.9
set_sfSet the spread factor. Default: 7
set_bwSet the bandwith value. Options: (7.8, 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125, 250, 500) kHz: Default 250
set_crSet the coding rate. Default: 5
set_swSet the sync Word: Default: 0x34
set_plSet the preamble length: Default: 10
set_opSet the output power. Default: 20
get_config Show the configuration of the radio
```
## Example

To test the activity in the selected frequency we use another catsniffer loaded with the `LoRa-CLI` firmware.
Using the next configuration: `freq:915.0, bw:125.0, sf:7, cr:5, syncWord:0x12, power:20, preableLength:8`
So we update the configuration in the `LoRa-CAD` firmware:
```bash
[SX1262] Initializing ... success!
> set_freq 915
Frequency set to 915.00 MHz
> set_sw 12
Sync word set to 0x12
> set_bw 125
Bandwidth set to 125.00 kHz
```
Then running a loop sending `pwnlabs` in the sender board.
```txt
[SX1262] Received packet!
[SX1262] Data:		pwnlabs
[SX1262] RSSI:		-16.00 dBm
[SX1262] SNR:		12.25 dB
[SX1262] Frequency error:	2627.25 Hz
[SX1262] Received packet!
[SX1262] Data:		pwnlabs
[SX1262] RSSI:		-16.00 dBm
[SX1262] SNR:		13.00 dB
[SX1262] Frequency error:	2627.25 Hz
[SX1262] Received packet!
[SX1262] Data:		pwnlabs
[SX1262] RSSI:		-16.00 dBm
[SX1262] SNR:		13.00 dB
[SX1262] Frequency error:	2627.25 Hz
[SX1262] Received packet!
[SX1262] Data:		pwnlabs
[SX1262] RSSI:		-16.00 dBm
[SX1262] SNR:		12.75 dB
[SX1262] Frequency error:	2627.25 Hz
[SX1262] Received packet!
[SX1262] Data:		pwnlabs
[SX1262] RSSI:		-16.00 dBm
[SX1262] SNR:		13.00 dB
[SX1262] Frequency error:	2642.75 Hz
```
