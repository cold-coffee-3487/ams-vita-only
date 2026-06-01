# Common RF MEL VITA 49.2 Control Message Set and Tailoring

DISTRIBUTION STATEMENT A: Approved for public release: distribution unlimited.

This document details the AMS GRA VITA 49.2 tailored packet structures for the Common RF MEL.

## ControlScheduleRequestPacket

Control Packet Template

### .header (Mandatory)
| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Time Determined | Time When Payload Value Determined |
|---|---|---|---|---|---|---|---|---|
| 1 | 31:28 | .type | B | 0111 | 0111 | Packet Type: 0111 => Extension Command Packet | Fixed | N/A |
|  | 27 | .cBit | B | 1 | 1 | Indicates whether CLASS ID is included in the prologue | Fixed | N/A |
|  | 26 | .isAck | B | 0 | 0 | Indicates whether this is an acknowledge packet | Fixed | N/A |
|  | 25 | .reservedBit25 | N | 0 | 0 | Reserved | N/A | N/A |
|  | 24 | .isCancellation | B | 0 | 0 | Indicates whether this is cancellation packet | Fixed | N/A |
|  | 23:22 | .tsi | B | 11 | 11 | Time Stamp Integer indicates: 00 No int seconds time stamp, 01 => UTC, 10 => GPS, 11 => Other | Fixed per VA Definition | N/A |
|  | 21:20 | .tsf | B | 10 | 10 | Time Stamp Fractional indicates: 00 => No frac seconds field, 01 => sample count, 10 => picoseconds, 11 => free running count | Fixed per VA Definition | N/A |
|  | 19:16 | .packetCount | B | 0 | 0:15 | 4-Bit Packet sequence number for this packet type in this streamID | Dynamic | N/A |
|  | 15:0 | .packetSize | B | 32 | 32:2^16-1 | Total number of 32-bit words in this packet including the prologue | Fixed per VA Definition | N/A |


### .streamId (Stream Identifier) (Mandatory)
| Word | Bit | Designation | Bit Used | Default Value | Function | Time Determined | Time When Payload Value Determined |
|---|---|---|---|---|---|---|---|
| 2 | 31:0 | .sid | B | 0 | The Control packet stream ID uniquely identifies selection of the VA configuration instance (with transmit and receive direction) and is known by the VAA, VAS and the controlling service.  The stream ID is provided by the VAA upon allocation. | VA Initialization | N/A |


### .classId (Class Identifier) (Mandatory)
| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Time Determined | Time When Payload Value Determined |
|---|---|---|---|---|---|---|---|---|
| 3 | 31:27 | .padBitCount | E | 0 | 0 | Number of pad bits (0…31) at the end of the packet to hit a 32-bit word boundary. Not used by Base Set. | Fixed per VA Definition | N/A |
|  | 26:24 | .reservedBits26to24 | N | 0 | 0 | Reserved | N/A | N/A |
|  | 23:0 | .oui | B | 0xAAAAAA | 0xAAAAAA | Organization Number. Organizationally Unique Identifier (OUI),  "A 24-bit number that uniquely identifies a vendor, manufacturer, or other organization globally or worldwide". | Fixed per VA Definition | N/A |
| 4 | 31:24 | .infoClassCode_Type | B | 0x04 | 4:5 | 0 => Unknown; 1 => TxComm, 2 => RxComm, 3 => Sensor;  4 => TxComm base set; 5 => RxComm Base Set; 6 => TxComm extended set 1; 7 => RxComm extended set 1; 8 => TxComm  extended set 2; 9 => RxComm extended set 2; (10-255) => TBD | Fixed per VA Definition | N/A |
|  | 23:16 | .infoClassCode_Version | B | 0x00 | 0:2^8-1 | Version number of information class | Fixed per VA Definition | N/A |
|  | 15:8 | .packetClassCode_Type | B | 0x0A | 0x0A | Defined PacketTypes:  0x00 => undefined,  0x0A => Base Set Control ScheduleRequest, 0x0B => Base Set Schedule Acknowledge (AckR),  0x0C => Base Set Execution Acknowledge (AckX), 0x0D => Base Set Data, 0x0E => Base Set DataContext | Fixed per VA Definition | N/A |
|  | 7:0 | .packetClassCode_Version | B | 0x05 | 0:2^8-1 | Version of Packet Class definition for specified type | Fixed per VA Definition | N/A |


### .timeStamp (Time Stamp) (Mandatory)
| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Time Determined | Time When Payload Value Determined |
|---|---|---|---|---|---|---|---|---|
| 5 | 31:0 | .tsi | B | 0 | 0:2^32-1 | Integer time stamp, UTC seconds since midnight, Jan. 1, 2019 | Dynamic within per VA limits | N/A |
| 6 | 63:32 | .tsf | B | 0 | 0:2^64-1 | Fractional time stamp most-significant (Upper 32 bits); format indicated by header.tsf, picoseconds since last seconds update | Dynamic within per VA limits | N/A |
| 7 | 31:0 | .tsfLower | B | 0 |  | Fractional time stamp least-significant (Lower 32 bits); format indicated by header.tsf, picoseconds since last seconds update | Dynamic within per VA limits | N/A |


### .cam (Control/Ack Mode Field) (Mandatory)
| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Time Determined | Time When Payload Value Determined | Notes |
|---|---|---|---|---|---|---|---|---|---|
| 8 | 31 | .ce | N | 0 | 0 | Controllee identifier fields are not used in the AMS Base Set; set to zero | Fixed per VA Definition | N/A |  |
|  | 30 | .ie | N | 0 | 0 | Controllee identifier format selection is not used in the AMS Base Set; set to zero | Fixed per VA Definition | N/A |  |
|  | 29 | .cr | N | 0 | 0 | Controller identifier fields are not used in the AMS Base Set; set to zero | Fixed per VA Definition | N/A |  |
|  | 28 | .ir | N | 0 | 0 | Controller identifier format selection is not used in the AMS Base Set; set to zero | Fixed per VA Definition | N/A |  |
|  | 27 | .p | B | 0 | 0 | Partial packet execution permitted. 1 => parameters that did not cause errors or warnings will be exectued, and parameters that did cause errors or warnings will be exectued if they were corrected. 0 => any uncorrected errors or warnings will cause the entire packet to be rejected. The Baseset does not support warnings, and packets that cause errors will not be executed totally or in part. | Fixed per VA Definition | N/A | If [p,w,er] = [0,1,0] then schedule request is cancelled if any errors are generated. Currently warnings are not supported. If warnings are generated in a future extension [p,w,er] = [0,1,0] cancels  schedule request if warning condition was not corrected. Otherwise schedule request proceeds. See Table 8.3.1.2-1 of [2] |
|  | 26 | .w | E | 0 | 0:1 | Indicates action taken when a warning occurs. 1 =>  the MFA will try to correct parameters that caused warnings, 0 => it will not. How the MFA behaves when there are uncorrected warnings is determined by p (bit 27). The Base Set does not support warnings. | Fixed per VA Definition | N/A |  |
|  | 25 | .er | B | 0 | 0 | Indicates action taken when an error occurs. 1 =>  the MFA will try to correct parameters that caused errors, 0 =>  it will not. How the MFA behaves when there are uncorrected errors is determined by p (bit 27). The Base Set does not support attempts to correct errors, and packets that cause errors will not be executed totally or in part. | Fixed per VA Definition | N/A |  |
|  | 24:23 | .actionBits | B | 10 | 10 | Action Bit Field: 00 => no action, 01 => dry run mode, 10 => execute, 11 => reserved | Fixed: 10 (execute) - required <br> Other values optional | N/A |  |
|  | 22 | .nack | E | 0 | 0 | Provide AckV or AckX only on warnings or errors (respond only with negative acklowledements) | Fixed per VA Definition | N/A |  |
|  | 21 | .reservedBit21 | N | 0 | 0 | Reserved | N/A | N/A |  |
|  | 20 | .reqV | E | 0 | 0 | Validation Ack packet requested | Dynamic (if allowed by VA) | N/A |  |
|  | 19 | .reqX | B | 1 | 1 | Execution Ack packet requested | Dynamic (if allowed by VA) | N/A |  |
|  | 18 | .reqS | N | 0 | 0 | Query-State Ack packet requested | Dynamic (if allowed by VA) | N/A |  |
|  | 17 | .reqW | E | 0 | 0 | Request warning fields in execution and validation ack packets | Dynamic (if allowed by VA) | N/A | Base Set does not support Warning Reports |
|  | 16 | .reqEr | B | 1 | 1 | Request error fields in execution ack packets | Dynamic (if allowed by VA) | N/A |  |
|  | 15 | .reqR | B | 1 | 1 | Information-Response Ack packet requested (see Change Notes) | Dynamic (if allowed by VA) | N/A |  |
|  | 14:12 | .timingControl | E | 0 | 0:7 | Timing Control: refer to documentation for details,  000 => indicates "it is at the discretion of the controllee when the control fields are executed" | Fixed | N/A |  |
|  | 11:8 | .ackBits | N | 0 | 0 | Acknowledge bits (zero in control packets) |  |  |  |
|  | 7:4 | .schReqType | B | 1 | 0:15 | Schedule request type: 0 => No schedule request. Message is for informational purposes only. The timeStamp indicates the time after which the scheduler can expect to reveive schedule requests using the configuration described in this packet. 1 =>  Requesting execute starting at timeStamp for dwell seconds; 2 => Requesting execute starting between timeStamp + earlyStartTime and timeStamp + lateStartTime for dwell seconds; 3 => Execute when possible starting now, until pre-empted by higher priority access, or duration is exceeded, or request is cancelled, return to execution at conclusion of pre-empting event (hoover mode); 4-14 => TBD, 15 => Cancel previous request (used in cancellation message). | Limited per VA Definition | N/A |  |
|  | 3 | .reqStatusChange | N | 0 | 0 |  |  |  |  |
|  | 2:0 | .reservedBits2to0 | N | 0 | 0 | Reserved |  |  |  |


### .messageId (Unique Message Identifier) (Mandatory)
| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Time Determined | Time When Payload Value Determined |
|---|---|---|---|---|---|---|---|---|
| 9 | 31:0 | .messageId | B | 0 | 1:2^32-1 | Used to distinguish Control packets with the same Stream ID and Class ID and to associate Control with Ack Packets. This is a continuously increasing message counter (starting at 1) for each unique control packet. | Dynamic | N/A |


### .controlleeId (Controllee Identifier, format is either 32-bit or 128-bit and is indicated by .cam.ie) (Forbidden)
| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Time Determined | Time When Payload Value Determined |
|---|---|---|---|---|---|---|---|---|
| N/A | 31:0 | .controlleeId | N | 0 | 0 | Unique 32-bit controlee Identifier | Per VA Definition | N/A |
| N/A | 127:96 | .controlleeUuidMsw | N | 0 | 0 | Controllee UUID Most Significant Bits | Per VA Definition | N/A |
| N/A | 95:64 | .controlleeUuidWord2 | N | 0 | 0 |  | Per VA Definition | N/A |
| N/A | 63:32 | .controlleeUuidWord1 | N | 0 | 0 |  | Per VA Definition | N/A |
| N/A | 31:0 | .controlleeUuidLsw | N | 0 | 0 | Controllee UUID Least Significant Bits | Per VA Definition | N/A |


### .controllerId (Controller Identifier, format is either 32-bit or 128-bit and is indicated by .cam.ir) (Forbidden)
| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Time Determined | Time When Payload Value Determined |
|---|---|---|---|---|---|---|---|---|
| N/A | 31:0 | .controllerId | N | 0 | 0 | Unique 32-bit controller Identifier | Per VA Definition | N/A |
| N/A | 127:96 | .controllerUuidMsw | N | 0 | 0 | Controller UUID Most Significant Bits | Per VA Definition | N/A |
| N/A | 95:64 | .controllerUuidWord2 | N | 0 | 0 |  | Per VA Definition | N/A |
| N/A | 63:32 | .controllerUuidWord1 | N | 0 | 0 |  | Per VA Definition | N/A |
| N/A | 31:0 | .controllerUuidLsw | N | 0 | 0 | Controller UUID Least Significant Bits | Per VA Definition | N/A |


### .cif0 (Control Indicator Field 0, Legacy Fields) (Mandatory)
**Notes:** See Cotrol-Context Payload Formats

| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Time Determined | Time When Payload Value Determined | Field Size (words) |
|---|---|---|---|---|---|---|---|---|---|
| 10 | 31 | .cfci | B | 1 | 1 | Context field change indicator: Control packets are only sent when something needs to be changed so this should be set to 1 | Fixed | N/A | 0 |
|  | 30 | .rpi | N | 0 | 0 | Reference point identifier |  |  |  |
|  | 29 | .bandwidth | B | 1 | 1 | 3 dB Bandwidth. | Fixed per VA Definition | Dynamic if allowed per VA Definition | 2 |
|  | 28 | .ifRefFreq | E | 0 | 0 | IF reference frequency.  This is the digital frequency  of the IF or baseband signal. ifRefFreq translates to/from rfRefFreq + rfRefFreqOffset during frequency conversion.  For real data samples it is in the range 0 - Fs/2, for complex data samples it is in the range -Fs/2 - +Fs/2 |  |  | 2 |
|  | 27 | .rfRefFreq | B | 1 | 1 | RF reference frequency.  This is usually the center of the analog superchannel and will always be a large positive number. If rfRefFreqOffset=0, rfRefFreq translatetes to ifRefFreq during frequency converslin. | Fixed per VA Definition | Dynamic within limits of VA definition | 2 |
|  | 26 | .rfRefFreqOffset | E | 0 | 0 | Offset from RF Ref Freq of RF frequency that translates to/from IF Ref Freq. rfRefFreq+rfRefFreqOffset translatetes to/from ifRefFreq during frequency conversion. If rfRefFreqOffset is not provided its value is set to 0. | Fixed per VA Definition | Dynamic within limits of VA definition | 2 |
|  | 25 | .ifBandOffset | E | 0 | 0 | IF band offset: IF band center = ifRefFreq + ifBandOffset. ifBandOffset may be positive or negative depending on IF band center relative to ifRefFreq. |  |  | 2 |
|  | 24 | .refLevel | E | 0 | 0 | Reference level relates digital signal amplitude to analog signal power. An analog Reference Point and location for the digital Described Signal must be defined in system documentation. The Reference Level is the AC power of a single analog sine wave at the reference point that produces a unit-scale digital sine wave as the Described Signal. Note that a unit scale sine wave for N-bit two's complement data is defined as one with amplitude ranging from -2^(N-1) to +2^(N-1), which exceeds the available range of an N-bit two's complement number at its peak value. | Fixed per VA Definition | Fixed or dyanmic, within limits, per VA definition | 1 |
|  | 23 | .gain | B | 1 | 1 | Gain bits 31:16 are not used in the base set and shall be set to 0. Bits 15:0 should be used in Base Set Rx Schedule Requests to control signal amplitude into the MFA ADC. Gain shall not be used on Tx, as it will conflict with EIRP (figure of merit, CIF 4/29). Gain bits 15:0 shall be set to 0 in Tx Schdule Requests. | Fixed per VA Definition | Dynamic within limits of VA definition | 1 |
|  | 22 | .overRangeCount | N | 0 | 0 | Over-range count |  |  |  |
|  | 21 | .sampleRate | B | 1 | 1 | Sample rate in Hz | Fixed per VA Definition | Fixed or dyanmic, within limits, per VA definition | 2 |
|  | 20 | .tsa | N | 0 | 0 | Time stamp adjustment |  |  |  |
|  | 19 | .tsCalTime | N | 0 | 0 | Time stamp calibration time |  |  |  |
|  | 18 | .temp | N | 0 | 0 | Temperature |  |  |  |
|  | 17 | .deviceId | N | 0 | 0 | Device identifier |  |  |  |
|  | 16 | .sei | N | 0 | 0 | State and Event Indicator. Eight predefined indicator bits with corresponding enable bits, plus 8 user-defined bits. Bits 28 and 16 indicate whether AGC is used. See [2] Table 9.10.8-1. Not currently in use. |  |  | 1 |
|  | 15 | .dataFormat | B | 1 | 1 | Signal data packet payload format.  We should always send the data packet format |  |  | 2 |
|  | 14 | .gps | N | 0 | 0 | Formatted GPS |  |  |  |
|  | 13 | .ins | N | 0 | 0 | Formatted INS |  |  |  |
|  | 12 | .ecef | N | 0 | 0 | ECEF ephemeris |  |  |  |
|  | 11 | .relativeEphemeris | N | 0 | 0 | Relative ephemeris |  |  |  |
|  | 10 | .ephemerisRefId | N | 0 | 0 | Ephermeris reference ID |  |  |  |
|  | 9 | .gpsAscii | N | 0 | 0 | GPS ASCII |  |  |  |
|  | 8 | .contextAssocLists | N | 0 | 0 | Context association lists |  |  |  |
|  | 7 | .cif7enable | N | 0 | 0 | Field attributes enable | Fixed: 0 | N/A |  |
|  | 6 | .reservedBit6 | N | 0 | 0 | Reserved |  |  |  |
|  | 5 | .reservedBit5 | N | 0 | 0 | Reserved |  |  |  |
|  | 4 | .cif4enable | B | 1 | 1 | Extension Fields | Fixed per VA Definition | N/A |  |
|  | 3 | .cif3enable | B | 1 | 1 | Temporal, environmental | Fixed per VA Definition | N/A |  |
|  | 2 | .cif2enable | B | 1 | 1 | Identifiers (tags) | Fixed per VA Definition | N/A |  |
|  | 1 | .cif1enable | B | 1 | 1 | Spatial, Signal spectral, I/O, Ctl | Fixed per VA Definition | N/A |  |
|  | 0 | .reservedBit0 | N | 0 | 0 | Reserved |  |  |  |


### .cif1 (Control Indicator Field 1, Spatial, Signal, Spectral, I/O, Ctl) (Mandatory)
**Notes:** See Cotrol-Context Payload Formats

| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Time Determined | Time When Payload Value Determined | Field Size (words) | Notes |
|---|---|---|---|---|---|---|---|---|---|---|
| 11 | 31 | .phaseOffset | E | 0 | 0 | Phase offset | Fixed per VA Definition | Dynamic if supported and within limits set by VA definition | 1 |  |
|  | 30 | .polarization | B | 1 | 1 | Polarization: Upper bits 31:16 indicate polarization ellipse tilt angle, lower bits 15:0 indicate ellipse eccentricity. Both are measured in radians. Tilt angle is the angle of the polarization-ellipse major axis  measured counter-clockwise from the array/antenna plane positive x-axis [2,4], with the x-axis defined in system documentation. Ellipticity describes the eccentricity of the polarization ellipse, including direction of rotation where appropriate. Linear polarization is described by ellipticity = π/4. For tilt = 0 linear polarization is horizontal, and for tilt = π/2 linear polarization is vertical, thus horzontal polarization is parallel to the antenna x-axis, and vertical polarization parallel to the antenna y-axis, both as defined in system documentation.  Ellipticity = 0 indicates right-handed circular polarization, and ellipticity = π/2 indicates left-handed circular polarization, where left- or right-handed rotation shall be defined  from the point of view of an observer at the receiver looking toward the transmitter [2,4].  Note that the direction in which tilt angle is measured, and the definitions of left- and right-handed rotation are inconsistent with IEEE standards [3]. | Fixed per VA Definition | Dynamic if supported and within limits set by VA definition | 1 |  |
|  | 29 | .pointing3d | B | 1 | 1 | 3-D pointing vector. Indicates pointing beam direction relative to the assigned aperture boresight. 3D pointing has two subfields: Bits 31:16 are elevation, bits 15:0 are azimuth, both in the antenna frame of reference. Define antenna boresight as the z-axis, the x axis is orthogonal to z and "horizontal" in the frame of reference defined by the antenna. The y axis points "up" in the antenna frame and is 90° clockwise from the x axis when looking out along boresight. Elevation is measured up (positive y direction) from the x-z plain to the pointing vector. Azimuth is measured in the x-z plain, between the projection of the pointing vector onto the x-z plain and boresight.  Positive azimuth angles are measured clockwise from boresight when looking down (from positive y toward the x-z plain). | Fixed per VA Definition | Dynamic if supported and within limits set by VA definition | 1 |  |
|  | 28 | .pointing3dStruct | E | 0 |  | 3-D pointing vector structure |  |  | -1 | Array-of-records structure. 3-D pointing vector and 3-D pointing vector structure are mutually exclusive. |
|  | 27 | .spatialScanType | E | 0 | 0 | Spatial scan type |  |  | 1 |  |
|  | 26 | .spatialRefType | E | 0 | 0 | Spatial reference type |  |  | 1 | Bits 3:2 can be used to define beam pointing frame of reference: 00 => NED, Az/El in NED frame, 01 =>  Not Allowed, 10 => Platform Centered (body frame), 11 => array centered, azimuth and elevation. This field affects the definition of polarization, pointing3D, and beamwidth. |
|  | 25 | .beamWidth | B | 1 | 1 | Beamwidth: Indicate the desired beamwidth. Schedule Request Interpretation may be specified in Service Contract or AIF. Possible interpretations are: 1) exact value within limits of accuracy 2) maximum value 3) minimum value. Default exact value  (TBR). Context shall be exact value. | Fixed per VA Definition | Dynamic within limits set by VA definition | 1 |  |
|  | 24 | .range | N | 0 | 0 | Range (distance) |  |  |  |  |
|  | 23:21 | .reservedBit23 | N | 0 | 0 | Reserved |  |  |  |  |
|  | 22 | .reservedBit22 | N | 0 | 0 | Reserved |  |  |  |  |
|  | 21 | .reservedBit21 | N | 0 | 0 | Reserved |  |  |  |  |
|  | 20 | .ber | N | 0 | 0 | Eb/No BER |  |  |  |  |
|  | 19 | .threshold | N | 0 | 0 | Threshold |  |  |  |  |
|  | 18 | .compressionPoint | N | 0 | 0 | Compression point |  |  |  |  |
|  | 17 | .ip2and3 | N | 0 | 0 | 2nd and 3rd order intercept points |  |  |  |  |
|  | 16 | .snr | N | 0 | 0 | SNR/Noise figure |  |  |  |  |
|  | 15 | .auxFreq | E | 0 | 0 | Auxiliary frequency |  |  | 2 |  |
|  | 14 | .auxGain | E | 0 | 0 | Auxiliary gain |  |  | 1 |  |
|  | 13 | .auxBandwidth | E | 0 | 0 | Auxiliary bandwidth |  |  | 2 |  |
|  | 12 | .reservedBit12 | N | 0 | 0 | Reserved |  |  |  |  |
|  | 11 | .cifArray | N | 0 | 0 | Array of CIFs |  |  |  |  |
|  | 10 | .spectrum | E | 0 | 0 | Spectrum |  |  | 13 |  |
|  | 9 | .scanStep | E | 0 | 0 | Sector/step-scan array of records. The array header consists of 3 mandatory words: total array size, header/record counts, and a bitmapped record subfield indicator. Records use the selected Sector/Step-Scan subfields. |  |  | -1 | Array-of-records structure. Header word 2 contains HeaderSize in bits 31:24, NumWords/Record in bits 23:12, and NumRecords in bits 11:0. Header word 3 uses bit 31 for sector number, bit 30 for F1 start frequency, bit 29 for F2 stop frequency, bit 28 for resolution bandwidth, bit 27 for tune step size, bit 26 for number of points, bit 25 for default gain, bit 24 for threshold, bit 23 for dwell time, bit 22 for start time, bit 21 for time 3, and bit 20 for time 4. Bits 19:0 are reserved. |
|  | 8 | .reservedBit8 | N | 0 | 0 | Reserved |  |  |  |  |
|  | 7 | .indexList | N | 0 | 0 | Index list |  |  |  |  |
|  | 6 | .discreteIo32 | N | 0 | 0 | Discrete I/O (32 bit) |  |  |  |  |
|  | 5 | .discreteIo64 | N | 0 | 0 | Discrete I/O (64 bit) |  |  |  |  |
|  | 4 | .healthStatus | N | 0 | 0 | Health status |  |  |  |  |
|  | 3 | .v49compliance | N | 0 | 0 | VITA 49 specification compliance - identifies version of VITA 49 spec implemented, 1 => V49.0, 2 => V49.1, 3=> V49A, 4 => V49.2 |  |  |  |  |
|  | 2 | .version | N | 0 | 0 | VITA 49 Version and build code - year, day, sub-version, user defined info. |  |  |  |  |
|  | 1 | .bufferSize | N | 0 | 0 | Buffer size |  |  |  |  |
|  | 0 | .reservedBit0 | N | 0 | 0 | Reserved |  |  |  |  |


### .cif2 (Control Indicator Field 2, Identifiers (tags)) (Mandatory)
**Notes:** See Cotrol-Context Payload Formats

| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Time Determined | Time When Payload Value Determined | Field Size (words) | Notes |
|---|---|---|---|---|---|---|---|---|---|---|
| 12 | 31 | .bind | N | 0 | 0 | Bind |  |  |  |  |
|  | 30 | .citedSid | E | 0 | 0 | The Cited Stream ID comes from the MEL or VAS in an AckR message and may be unknown to the Skill, or may change due to aircraft maneuvers or other reasons. When it is present, and this control packet is controlling one data stream (either because the activity only has one data stream per direction, or this control packet only refers to one stream in a set of mulitple streams) then the Cited Stream  ID is the SID for that data stream. When  the packet controls mulitple [parallel] data streams the contents of .citedSID may be a 32-bit index pointing at one entry in a list of SIDs for data-stream-sets.  When the .dataAddressStructure field (4/23) has a different list of SID's, .dataAddressStructure shall take precedence. | Fixed per VA Definition | Dynamic selection among a pre-negotiated set | 1 | This value may not be known to Skill at time of schedule request. It is required in the associated AckR packet |
|  | 29 | .sibSid | N | 0 | 0 | Sibling Stream Id |  |  |  |  |
|  | 28 | .parentSid | N | 0 | 0 |  |  |  |  |  |
|  | 27 | .childSid | N | 0 | 0 | Children Stream Id |  |  |  |  |
|  | 26 | .citedMsgId | E | 0 | 0:1 | Cited message ID |  |  | 1 |  |
|  | 25 | .controlleeId | N | 0 | 0 | Controllee ID |  |  |  |  |
|  | 24 | .controlleeUuid | N | 0 | 0 | Controlee UUID |  |  |  |  |
|  | 23 | .controllerId | N | 0 | 0 | Controller ID |  |  |  |  |
|  | 22 | .controllerUuid | N | 0 | 0 | Controller UUID |  |  |  |  |
|  | 21 | .infoSource | E | 0 | 0:1 | Information source is a pointer to the Configuration Object or Antenna Information File (AIF) that describes the VA (Virtual Antenna) being controlled by this packet. In order to ensure the Configuration Object index is unique, the most significant 16 bits  (bits 31:16) of this field indicate the MEL, and the least significant 16 bits (bits 15:0) indicate the Config Obj. or AIF index for that MEL. |  |  | 1 |  |
|  | 20 | .trackId | N | 0 | 0 | Track ID |  |  |  |  |
|  | 19 | .countryCode | N | 0 | 0 | Country code |  |  |  |  |
|  | 18 | .operator | N | 0 | 0 | Operator |  |  |  |  |
|  | 17 | .platformClass | N | 0 | 0 | Platform class |  |  |  |  |
|  | 16 | .platformInst | N | 0 | 0 | Platform instance |  |  |  |  |
|  | 15 | .platformDisp | N | 0 | 0 | Platform display |  |  |  |  |
|  | 14 | .emsDeviceClass | N | 0 | 0 | EMS device class |  |  |  |  |
|  | 13 | .emsDeviceType | N | 0 | 0 | EMS device type |  |  |  |  |
|  | 12 | .emsDeviceInst | N | 0 | 0 | EMS device instance |  |  |  |  |
|  | 11 | .modClass | N | 0 | 0 | Modulation class | Fixed per VA Definition | Dynamic within limits set by VA definition |  |  |
|  | 10 | .modType | N | 0 | 0 | Modulation type | Fixed per VA Definition | Dynamic within limits set by VA definition |  |  |
|  | 9 | .functionId | N | 0 | 0 | Function ID |  |  |  |  |
|  | 8 | .modeId | E | 0 | 0:1 | Mode ID: User defined, used by communication capability  to control local functions in MFA, e.g., EMCON | Fixed per VA Definition | Dynamic | 1 |  |
|  | 7 | .eventId | N | 0 | 0 | Event ID:  This field is used by the Multi-Function Capability  as in the VITA 49.2 specification and is a generic16 bit field. |  |  | 1 |  |
|  | 6 | .funcPriorityId | B | 1 | 1 | Function priority ID for resolving conflicting requests to use MFA resources. Unsigned 32-bit integer, larger values indicate  higher priority. | Fixed per VA Definition | Dynamic within limits of overall VA priority | 1 |  |
|  | 5 | .commPriorityId | N | 0 | 0 | Communication priority ID for determining access to internal data network when network becomes overloaded. Unsigned 32-bit integer. Bigger is higher priority. |  |  |  |  |
|  | 4 | .rfFootprint | N | 0 | 0 | RF footprint |  |  |  |  |
|  | 3 | .rfFootprintRange | N | 0 | 0 | RF footprint range |  |  |  |  |
|  | 2 | .reservedBit2 | N | 0 | 0 | Reserved |  |  |  |  |
|  | 1 | .reservedBit1 | N | 0 | 0 | Reserved |  |  |  |  |
|  | 0 | .reservedBit0 | N | 0 | 0 | Reserved |  |  |  |  |


### .cif3 (Control Indicator Field 3, Temporal, Environmental) (Mandatory)
**Notes:** See Cotrol-Context Payload Formats

| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Time Determined | Time When Payload Value Determined | Field Size (words) |
|---|---|---|---|---|---|---|---|---|---|
| 13 | 31 | .tsDetails | N | 0 | 0 | Time stamp details |  |  |  |
|  | 30 | .tsSkew | N | 0 | 0 | Time stamp skew |  |  |  |
|  | 29 | .reservedBit29 | N | 0 | 0 | Reserved |  |  |  |
|  | 28 | .reservedBit28 | N | 0 | 0 | Reserved |  |  |  |
|  | 27 | .riseTime | N | 0 | 0 | Rise time |  |  |  |
|  | 26 | .fallTime | N | 0 | 0 | Fall time |  |  |  |
|  | 25 | .offsetTime | N | 0 | 0 | Offset time |  |  |  |
|  | 24 | .pulseWidth | N | 0 | 0 | Pulse width |  |  |  |
|  | 23 | .period | N | 0 | 0 | Period |  |  |  |
|  | 22 | .duration | N | 0 | 0 | Duration |  |  |  |
|  | 21 | .dwell | B | 1 | 1 | Dwell duration in femtoseconds, where Dwell is the duration of a requested transmitter or receiver event, i.e. the transmitter or receiver has exclusive use of an assigned aperture. If Schedule Request type schReqType=1, a dwell shall begin at the Schedule Request TimeStamp and last for Dwell femtoseconds. For receive events all data samples received during a dwell will be sent to the requesting skill. For transmit events the aperture may transmit one or more data bursts during a dwell, with burst durations defined by the data packet time stamp and size, provided all bursts are entirely contained within the dwell. The dwell value ranges from 0 to ~9223 s ( ~154 minutes). | Fixed per VA Definition | Dynamic | 2 |
|  | 20 | .jitter | E | 0 | 0:1 | Jitter |  |  | 2 |
|  | 19 | .reservedBit19 | N | 0 | 0 | Reserved |  |  |  |
|  | 18 | .reservedBit18 | N | 0 | 0 | Reserved |  |  |  |
|  | 17 | .age | N | 0 | 0 | Age |  |  |  |
|  | 16 | .shelfLife | N | 0 | 0 | Shelf life |  |  |  |
|  | 15:8 | .reservedBits15to8 | N | 0 | 0 | Reserved |  |  |  |
|  | 7 | .airTemp | N | 0 | 0 | Air temperature |  |  |  |
|  | 6 | .seaGroundTemp | N | 0 | 0 | Sea/ground temperature |  |  |  |
|  | 5 | .humidity | N | 0 | 0 | Humidity |  |  |  |
|  | 4 | .barPressure | N | 0 | 0 | Barometric pressure |  |  |  |
|  | 3 | .seaState | N | 0 | 0 | Sea and swell state |  |  |  |
|  | 2 | .tropState | N | 0 | 0 | Tropospheric state |  |  |  |
|  | 1 | .netId | N | 0 | 0 | Network ID |  |  |  |
|  | 0 | .reservedBit0 | N | 0 | 0 | reserved |  |  |  |


### .cif4 (Control Indicator Field 4 - Extension Fields) (Mandatory)
**Notes:** See Cotrol-Context Payload Formats

| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Time Determined | Time When Payload Value Determined | Field Size (words) | Notes |
|---|---|---|---|---|---|---|---|---|---|---|
| 14 | 31 | .reservedBit31 | N | 0 | 0 |  | Fixed per VA Definition | Dynamic |  |  |
|  | 30 | .reservedBit30 | N | 0 | 0 | Reserved for future cif extensions |  |  |  |  |
| 14 | 29 | .rfFigureOfMerit | B | 1 | 1 | Requested input G/T (dB/K) range for Rx, requested EIRP (dBm) range on Tx.  The MFA shall, to the best of its ability, meet the EIRP or G/T request in the lower 16 bits of this word (bits 15:0) at any pointing angle inside the documented aperture Field of Regard. Calculation of EIRP or G/T values  supported by an MFA shall include the effects of scan angle, beamwidth, and frequency, as appropriate. The EIRP calculation shall include polarization mismatch only if transmit polarization realized is different from polarization requested, in which case it shall be assumed the receiver is configured for the requested polarization. G/T calculation shall not include the effects of external noise. The MFA shall provide an EIRP  as close to the requested value (bits 15:0) as possible without exceeding it. If the maximum available EIRP is below the lower limit in bits 31:16, the MFA shall reject the schedule request. Requests with pointing angle and/or other parameters that result in G/T below the value provided in the upper 16 bits (bits 31:16) shall be rejected by the scheduler. Configurations producing G/T greater than the requested value (bits 15:0) may be allowed.  Note that VITA 49.2 defines polariation angles and rotation from the point of view of an observer at the receiver, rather than at the transmitter as in [3]. |  |  | 1 |  |
|  | 28 | .earlyStartTime | E | 0 | 0 | Earliest allowed start time for flexible schedule request, offset from time stamp | Fixed per VA Definition | Dynamic within limits set by VA definition | 2 |  |
|  | 27 | .lateStartTime | E | 0 | 0 | Latest allowed start time for flexible schedule request, offset from time stamp | Fixed per VA Definition | Dynamic within limits set by VA definition | 2 |  |
|  | 26 | .rejectReason | E | 0 | 0 | 32 bit enumeration of reasons that this control was rejected by the scheduler. The first 8 bits indicated the requested parameter that caused a failure. Bits 31:29 are the failed parameter CIF # as a 3-bit unsigned integer, except that bits 31:29 = 111 indicates the failure is not tied to a specific parameter. Bits 28:24 indicate the failed parameter CIF Bit # as a 5-bit unsigned integer, and are all 0 if bits 31:29 = 111. Bits 23:0 form a 24-bit unsigned integer that describes the failure mode. See the Reject Reason Enumeration tab . |  |  | 1 | rejectReason describes the reason a Schedule Request was rejected by the VAS, i.e. was not scheduled. In this case .schX (Ack Packet CAM bit 10) = 0 indicationg schedule failure. Failures that occur after a request was schduled (schX=1) are reported by AckX packets. |
|  | 25 | .maxDataPacketDwell | E | 0 | 0 | fractional time format giving period of time represented by data packet - can be used to reduce data delay during long Rx dwells |  |  | 2 |  |
|  | 24 | .addressGroupIndex | B | 1 | 1 | Provides a look-up index that points to a collection of MFP address for data and Ack packets associated with this Schedule Request. That is one or more data packet stream(s) with SID matching the Control SID, or  SID's identified by the SID or index in the field (2/30) .citedSID. Plus AckR and AckX packet streams with SID matching the Control SID.  The corresponding MFA addresses are in the same field (CIF 4, Bit 24) of the corresponding AckR packet. |  |  | 1 | AGI points to a table with variable a number of entries. Minimum entry is a default address. Every packet sent to the Skill for which an address has not been specified will be sent to the default address. Other possible values include but are not limited to Data Address Index (DAI), Control Address Index (CAI), and Context Address Index) (XAI). |
|  | 23 | .dataAddressStructure | E | 0 | 0 | An array of records listing all the data streams associated with this control packet stream by data stream SID and MFP address. When present it may replace the information in (2/30) .citedSID and always replaces the information in (4/24) dataAddressIndex. The corresponding MFA addresses are contained in the same field location (4/23) of the AckR packet returned in response to this packet. This array provides greater flexibility than using .citedSId and .dataAddressIndex at the cost of variable-length control packets. The array header consists of the 3 mandatory header words (see reference or Change Notes) plus a variable number of records consisting of 1 or 2 words (SID, MFP Address Index) or  (MFP Address Index) as indicated in header word 3. |  |  | -1 | Data Address Index and Data Address Structure both indicate the MFP data addresses to which received data should be provided. These two fields are mutually exclusive, consequently at most one field may have a non-zero CIF bit |
|  | 22 | .dataAddressTime | E | 0 | 0 | If the data address(es) of 4/24 or 4/23 are not immediately applicable (e.g., If the MFA must switch antenna faces during a stream of VITA data packets transporting one long Rx/Tx packet, the MFA scheduler may send a new AckR packet with the new MFA port addresses with some lead time before the change), The .dataAddressTime parameter indicates when the data address information in 4/24 or 4/23 becomes applicable. Offset from time stamp. |  |  | 2 |  |
|  | 21 | .txDigitalInputPower | B | 1 | 1 | Bits 15:0 contain Nominal power level of digital TX signal input to MFA for transmission, dBfs, where "full scale power" shall be defined as the power of a complex digital sinusoid with amplitude equal to full scale. Set to 0 for Rx. Set bits 31:16 to 0. |  |  | 1 | This parameter combined with rfFigureOfMerit (EIRP) may be used by the MFA to calculate required MFA internal gain during  transmission.  The value supplied may be a long-term average (multiple Tx bursts), the measured power of the digital data burst being scheduled, or an  arbitrary value. For example, if short-term TX EIRP is variable, and the Skill implements rapid power control by varying the amplitude of the digital signal it inputs to the MFA,   then  txDigitalInputPower should be set to a value such that maximum expected MFA input digital power results in an RF transmit power value no greater than the specified maxium EIRP. |
|  | 20:0 | .reservedBits20to0 | N | 0 | 0 | Reserved for future cif extensions |  |  |  |  |


### .cif7 (Control Indicator Field 7,  Attributes) (Forbidden)
| Word | Bit | Designation | Bit Used | Default Value | Range | Function |
|---|---|---|---|---|---|---|
| N/A | 31 | .curVal | N | 0 | 0 | Current value |
|  | 30 | .avgVal | N | 0 | 0 | Average value |
|  | 29 | .medVal | N | 0 | 0 | Median value |
|  | 28 | .std | N | 0 | 0 | Standard deviation |
|  | 27 | .maxVal | N | 0 | 0 | Maximum value |
|  | 26 | .minVal | N | 0 | 0 | Minimum value |
|  | 25 | .precision | N | 0 | 0 | Precision |
|  | 24 | .accuracy | N | 0 | 0 | Accuracy |
|  | 23 | .velocity | N | 0 | 0 | 1st Derivative (velocity) |
|  | 22 | .acceleration | N | 0 | 0 | 2nd Derivative (acceleration) |
|  | 21 | .jerk | N | 0 | 0 | 3rd Derivative (jerk) |
|  | 20 | .probability | N | 0 | 0 | Probability |
|  | 19 | .belief | N | 0 | 0 | Belief |
|  | 18:0 | .reservedBits18to0 | N | 0 | 0 | Reserved |


### .payloadFields (Mandatory)
**Notes:** Payload fields are defined by enabled CIF indicators and the Control-Context Payload Formats table. Payload fields are included in CIF-bit order and may contain additional fields from Extension Set. This group is a metadata placeholder and has no fixed rows.



## ScheduleAck(AckR)Packet

Extension AckR Packet Template (Acknowledgement packet with Response Data - only exists as an extension)

### .header (Mandatory)
**Notes:** Header Bits mirror values in associated Extension Control Packet

| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Time Determined | Time When Payload Value Determined |
|---|---|---|---|---|---|---|---|---|
| 1 | 31:28 | .type | B | 0111 | 0111 | Packet Type: 0111 => Extension Command Packet | Fixed | N/A |
|  | 27 | .cBit | B | 1 | 1 | Indicates whether CLASS ID is included in the prologue | Fixed | N/A |
|  | 26 | .isAck | B | 1 | 1 | Indicates THAT THIS IS an acknowledge packet | Fixed | N/A |
|  | 25 | .reservedBit25 | N | 0 | 0 | Reserved | N/A | N/A |
|  | 24 | .isCancellation | N | 0 | 0 | Indicates whether this is cancellation packet | Fixed | N/A |
|  | 23:22 | .tsi | B | 11 | 11 | Indicates: 00 No int seconds time stamp, 01 => UTC, 10 => GPS, 11 => Other | Fixed per VA Definition | N/A |
|  | 21:20 | .tsf | B | 10 | 10 | Indicates: 00 => No frac seconds field, 01 => sample count, 10 => picoseconds, 11 => free running count | Fixed per VA Definition | N/A |
|  | 19:16 | .packetCount | B | 0 | 0:15 | 4-Bit Packet sequence number for this packet type in this streamID | Dynamic | N/A |
|  | 15:0 | .packetSize | B | 15 | 15 | Total number of 32-bit words in the packet including the prologue | Fixed per VA Definition | N/A |


### .streamId (Stream Identifier) (Mandatory)
| Word | Bit | Designation | Bit Used | Default Value | Function |
|---|---|---|---|---|---|
| 2 | 31:0 | .sid | B | 1 | See Control and Context SID definitions.  Data SIDs will match the applicable control SIDs |


### .classId (Class Identifier) (Mandatory)
| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Time Determined | Time When Payload Value Determined |
|---|---|---|---|---|---|---|---|---|
| 3 | 31:27 | .padBitCount | E | 0 | 0 | Number of pad bits (0…31) at the end of the packet to hit a 32-bit word boundary. Not used by Base Set | Fixed per VA Definition | N/A |
|  | 26:24 | .reservedBits26to24 | N | 0 | 0 | Reserved | N/A | N/A |
|  | 23:0 | .oui | B | 0xAAAAAA | 0xAAAAAA | Organizationally Unique Identifier (OUI) | Fixed per VA Definition | N/A |
| 4 | 31:24 | .infoClassCode_Type | B | 0x04 | 4:5 | 0 => Unknown; 1 => TxComm, 2 => RxComm, 3 => Sensor;  4 => TxComm base set; 5 => RxComm Base Set; 6 => TxComm extended set 1; 7 => RxComm extended set 1; 8 => TxComm  extended set 2; 9 => RxComm extended set 2; (10-255) => TBD | Fixed per VA Definition | N/A |
|  | 23:16 | .infoClassCode_Version | B | 0x00 | 0:2^8-1 | Version number of information class | Fixed per VA Definition | N/A |
|  | 15:8 | .packetClassCode_Type | B | 0x0B | 0x0B | Defined PacketTypes:  0x00 => undefined, 0x0A => Base Set Control ScheduleRequest, 0x0B => Base Set Schedule Acknowledge (AckR),  0x0C => Base Set Execution Acknowledge (AckX), 0x0D => Base Set Data, 0x0E => Base Set DataContext | Fixed per VA Definition | N/A |
|  | 7:0 | .packetClassCode_Version | B | 0x05 | 0:2^8-1 | Version of Packet Class definition for specified type | Fixed per VA Definition | N/A |


### .timeStamp (Time Stamp) (Mandatory)
**Notes:** When a self-scheduling MFA is responding to a Control Packet that requested a flexible schedule time (.flexShedReq=1 in CAM bit 7), and the MFA was able to schedule the requested access, the time stamp shall indicate the scheduled time for execution of the control.

| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Time Determined | Time When Payload Value Determined |
|---|---|---|---|---|---|---|---|---|
| 5 | 31:0 | .tsi | B | 0 | 0:2^32-1 | Integer time stamp, UTC seconds since midnight Jan 1, 2019 | Dynamic within per VA limits | N/A |
| 6 | 31:0 | .tsf | B | 0 | 0:2^64-1 | Fractional time stamp most-significant (Upper 32 bits); format indicated by Header.tsf, 10 = picoseceonds (UCI) since last second | Dynamic within per VA limits | N/A |
| 7 | 31:0 | .tsfLower | B | 0 |  | Fractional time stamp least-significant (Lower 32 bits); format indicated by Header.tsf, 10 = picoseceonds (UCI) since last second | Dynamic within per VA limits | N/A |


### .cam (Control/Ack Mode Field) (Mandatory)
| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Time Determined | Time When Payload Value Determined |
|---|---|---|---|---|---|---|---|---|
| 8 | 31 | .ce | N | 0 | 0 | Identifier fields are not used in the AMS Base Set; set to zero |  |  |
|  | 30 | .ie | N | 0 | 0 | Identifier fields are not used in the AMS Base Set; set to zero |  |  |
|  | 29 | .cr | N | 0 | 0 | Identifier fields are not used in the AMS Base Set; set to zero |  |  |
|  | 28 | .ir | N | 0 | 0 | Identifier fields are not used in the AMS Base Set; set to zero |  |  |
|  | 27 | .p | E | 0 | 0 | Matches values in Control Packet being Acknowledged |  |  |
|  | 26 | .w | E | 0 | 0 | Matches values in Control Packet being Acknowledged |  |  |
|  | 25 | .er | E | 0 | 0 | Matches values in Control Packet being Acknowledged |  |  |
|  | 24:23 | .actionBits | B | 10 | 10 | Matches values in Control Packet being Acknowledged |  |  |
|  | 22 | .nack | E | 0 | 0 | Matches values in Control Packet being Acknowledged  - setting bit 22 (Nack) to 1 does not change this Ack Message to a Nack message |  |  |
|  | 21 | .reservedBit21 | N | 0 | 0 | Reserved |  |  |
|  | 20 | .ackV | E | 0 | 0 | 1 = > Indicates this is a Validation Ack packet, 0 => otherwise. AckV, AckX, AckR, and AckS are mutually exclusive. | Fixed | N/A |
|  | 19 | .ackX | B | 0 | 0 | 1 = > Indicates this is an Execution Ack packet, 0 => otherwise. AckV, AckX, AckR, and AckS are mutually exclusive. | Fixed | N/A |
|  | 18 | .ackS | N | 0 | 0 | 1 => Indicates this is a Query-State Ack packet, 0 => otherwise. AckV, AckX, AckR, and AckS are mutually exclusive. | Fixed | N/A |
|  | 17 | .ackW | N | 0 | 0 | Warning reports are not supported in AckR by the AMS Base Set; set to zero | Dynamic | N/A |
|  | 16 | .ackEr | B | 0 | 0:1 | 1 => Indicates errors were generated. May be used in AckX or AckR, not applicable in AckV Packets. AckEr=1 in an AckR packet makes it a NackR packet. | Dynamic | N/A |
|  | 15 | .ackR | B | 1 | 1 | 1 => Indicates this is an Information-Response Ack (AckR) packet, 0 => otherwise. AckV, AckX, AckR, and AckS are mutually exclusive. | Fixed | N/A |
|  | 14:12 | .timingControl | E | 0 | 0 | Timing Control: refer to documentation for details, 000 => indicates "it is at the discretion of the controllee when the control fields are executed" | Fixed | N/A |
|  | 11 | .ackP | E | 0 | 0 | Indicates partial response tp selected CIFs | Dynamic | N/A |
|  | 10 | .schX | B | 1 | 0:1 | 0 => one or more fields can NOT be scheduled for execution at time in timestamp, 1 => all fields can be scheduled for execution at time in timestamp.  A self-scheduling MFA may send an AckR packet with .schX = 0 if the access requested by a Control Packet could not be scheduled. If a request was scheduled and an AckR packet was sent with .schX = 1, after which the packet was bumped from the schedule by a higher priority request, a second AckR or an AckV packet with .schX = 0 may be sent. An AckX packet with .schX = 0 shall be sent if the control packet was not executed due to schedule conflicts, e.g., the allowed (range of) execution time(s) has passed. | Dynamic | N/A |
|  | 9:8 | .reservedBits9to8 | N | 0 | 0 |  |  |  |
|  | 7:4 | .schReqType | B | 1 | 1 | Matches values in Control Packet being Acknowledged |  |  |
|  | 3 | .reqStatusChange | E | 0 | 0 | The status of the Control Packet Schedule Request being acknowledged has changed. For example if a schedule request was previously denied for some reason, e.g. a local pre-emption in the MFA, that reason has gone away. Some configurations for this example are: .reqStatusChange=1, .schX=1 => request status has changed and the request has now been scheduled at the .timeStamp of this message; .reqStatusChange=1, .schX=0, .rejectReason=7 (try request again) => request status has changed but the VAS was unable to schedule the request (possibly because the requested schedule time window is over), send a new request. Other uses of .reqStatusChange can be described in the packet class type description. | Dynamic | N/A |
|  | 2:0 | .reservedBits2to0 | N | 0 | 0 | Reserved |  |  |


### .messageId (Unique Message Identifier) (Mandatory)
| Word | Bit | Designation | Bit Used | Default Value | Range | Function |
|---|---|---|---|---|---|---|
| 9 | 31:0 | .messageId | B | 0 | 0:2^32-1 | Same as Message Id of Control Packet being acknowledged |


### .controlleeId (Controllee Identifier, format is either 32-bit or 128-bit and is indicated by .cam.ie) (Forbidden)
| Word | Bit | Designation | Bit Used | Default Value |
|---|---|---|---|---|
| 10 | 31:0 | .controlleeIdWord0 | N | 1 |
| 10 | 127:96 | .controlleeIdWord1 | N | 0 |
| 11 | 95:64 | .controlleeIdWord2 | N | 0 |
| 12 | 31:0 | .controlleeIdWord3 | N | 1 |
| 13 | 31:0 | .controlleeIdWord4 | N | 0 |


### .controllerId (Controller Identifier, format is either 32-bit or 128-bit and is indicated by .cam.ir) (Forbidden)
| Word | Bit | Designation | Bit Used | Default Value |
|---|---|---|---|---|
| 14 | 31:0 | .controllerIdWord0 | N | 0 |
| 14 | 127:96 | .controllerIdWord1 | N |  |
| 15 | 95:64 | .controllerIdWord2 | N | 0 |
| 16 | 31:0 | .controllerIdWord3 | N | 0 |
| 17 | 31:0 | .controllerIdWord4 | N | 0 |


### .cif0 (Control Indicator Field 0, Legacy Fields) (Mandatory)
**Notes:** CIF 0 structure, default and range may be the same as in the associated  Extension Control packet, or it may only contain fields needed by the AckR. Fields flagged in this page as present are information being sent from the MFA to the MFP, not error or warning flags as in AckV and AckX, and not sending back values previously set by the MFP in a Control Packet as in AckS.

| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Field Size (words) |
|---|---|---|---|---|---|---|---|
| 10 | 31 | .cfci | E | 0 | 0 | Context field change indicator: Control packets are only sent when something needs to be changed so this should be set to 1 | 0 |
|  | 30 | .rpi | N | 0 | 0 | Reference point identifier |  |
|  | 29 | .bandwidth | E | 0 | 0 | 3 dB Bandwidth | 2 |
|  | 28 | .ifRefFreq | E | 0 | 0 | IF reference frequency.  This is the digital frequency offset within the superchannel of the transmission burst.  It is referenced to the center of the superchannel.  Hence the ifRefFreq can be positive or negative. | 2 |
|  | 27 | .rfRefFreq | E | 0 | 0 | RF reference frequency.  This is the center of the superchannel and will always be a large positive number. | 2 |
|  | 26 | .rfRefFreqOffset | E | 0 | 0 | Offset from RF Ref Freq of RF frequency that translates to/from IF Ref Freq | 2 |
|  | 25 | .ifBandOffset | E | 0 | 0 | IF band offset | 2 |
|  | 24 | .refLevel | E | 0 | 0 | Reference level relates digital signal amplitude to analog signal power. An analog Reference Point and location for the digital Described Signal must be defined in system documentation. The Reference Level is the AC power of a single analog sine wave at the reference point that produces a unit-scale digital sine wave as the Described Signal. Note that a unit scale sine wave for N-bit two's complement data is defined as one with amplitude ranging from -2^(N-1) to +2^(N-1), which exceeds the available range of an N-bit two's complement number at its peak value. | 1 |
|  | 23 | .gain | E | 0 | 0 | Gain bits 31:16 are not used in the base set and shall be set to 0. Bits 15:0 should be used in Base Set Rx Schedule Requests to control signal amplitude into the MFA ADC. Gain shall not be used on Tx, as it will conflict with EIRP (figure of merit, CIF 4/29). Gain bits 15:0 shall be set to 0 in Tx Schdule Requests. | 1 |
|  | 22 | .overRangeCount | N | 0 | 0 | Over-range count |  |
|  | 21 | .sampleRate | E | 0 | 0 | Sample rate in Hz | 2 |
|  | 20 | .tsa | N | 0 | 0 | Time stamp adjustment |  |
|  | 19 | .tsCalTime | N | 0 | 0 | Time stamp calibration time |  |
|  | 18 | .temp | N | 0 | 0 | Temperature |  |
|  | 17 | .deviceId | N | 0 | 0 | Device identifier |  |
|  | 16 | .sei | N | 0 | 0 | State and Event Indicator. Eight predefined indicator bits with corresponding enable bits, plus 8 user-defined bits. Bits 28 and 16 indicate whether AGC is in use. See [2] Table 9.10.8-1. Not currently in use. | 1 |
|  | 15 | .dataFormat | E | 0 | 0 | Signal data packet payload format.  We should always send the data packet format | 2 |
|  | 14 | .gps | N | 0 | 0 | Formatted GPS |  |
|  | 13 | .ins | N | 0 | 0 | Formatted INS |  |
|  | 12 | .ecef | N | 0 | 0 | ECEF ephemeris |  |
|  | 11 | .relativeEphemeris | N | 0 | 0 | Relative ephemeris |  |
|  | 10 | .ephemerisRefId | N | 0 | 0 | Ephermeris reference ID |  |
|  | 9 | .gpsAscii | N | 0 | 0 | GPS ASCII |  |
|  | 8 | .contextAssocLists | N | 0 | 0 | Context association lists |  |
|  | 7 | .cif7enable | N | 0 | 0 | Field attributes enable |  |
|  | 6:5 | .reservedBits6to5 | N | 0 | 0 | Reserved |  |
|  | 4 | .cif4enable | B | 1 | 1 | Extension Fields |  |
|  | 3 | .cif3enable | E | 0 | 0 | Temporal, environmental |  |
|  | 2 | .cif2enable | B | 1 | 1 | Identifiers (tags) |  |
|  | 1 | .cif1enable | E | 0 | 0 | Spatial, Signal spectral, I/O, Ctl |  |
|  | 0 | .reservedBit0 | N | 0 | 0 | Reserved |  |


### .cif1 (Extension)
**Notes:** CIF 1 Is not used by the baseline AckR packet

| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Field Size (words) | Notes |
|---|---|---|---|---|---|---|---|---|
| N/A | 31 | .phaseOffset | E | 0 | 0 | Phase offset | 1 |  |
|  | 30 | .polarization | E | 0 | 0 | Polarization: Upper bits 31:16 indicate polarization ellipse tilt angle, lower bits 15:0 indicate ellipse eccentricity. Both are measured in radians. Tilt angle is the angle of the polarization-ellipse major axis  measured counter-clockwise from the array/antenna plane positive x-axis [2,4], with the x-axis defined in system documentation. Ellipticity describes the eccentricity of the polarization ellipse, including direction of rotation where appropriate. Linear polarization is described by ellipticity = π/4. For tilt = 0 linear polarization is horizontal, and for tilt = π/2 linear polarization is vertical, thus horzontal polarization is parallel to the antenna x-axis, and vertical polarization parallel to the antenna y-axis, both as defined in system documentation.  Ellipticity = 0 indicates right-handed circular polarization, and ellipticity = π/2 indicates left-handed circular polarization, where left- or right-handed rotation shall be defined  from the point of view of an observer at the receiver looking toward the transmitter [2,4].  Note that the direction in which tilt angle is measured, and the definitions of left- and right-handed rotation are inconsistent with IEEE standards [3]. | 1 |  |
|  | 29 | .pointing3d | E | 0 | 0 | 3-D pointing vector. Indicates pointing beam direction relative to the assigned aperture boresight. 3D pointing has two subfields: Bits 31:16 are elevation, bits 15:0 are azimuth, both in the antenna frame of reference. Define antenna boresight as the z-axis, the x axis is orthogonal to z and "horizontal" in the frame of reference defined by the antenna. The y axis points "up" in the antenna frame and is 90° clockwise from the x axis when looking out along boresight. Elevation is measured up (positive y direction) from the x-z plain to the pointing vector. Azimuth is measured in the x-z plain, between the projection of the pointing vector onto the x-z plain and boresight.  Positive azimuth angles are measured clockwise from boresight when looking down (from positive y toward the x-z plain). | 1 |  |
|  | 28 | .pointing3dStruct | E | 0 | 0 | 3-D pointing vector structure | -1 |  |
|  | 27 | .spatialScanType | N | 0 | 0 | Spatial scan type |  |  |
|  | 26 | .spatialRefType | E | 0 | 0 | Spatial reference type | 1 |  |
|  | 25 | .beamWidth | E | 0 | 0 | Beamwidth: Indicate the desired beamwidth. Schedule Request  Interpretation may be specified in Service Contract or AIF. Possible interpretations are: 1) exact value within limits of accuracy 2) maximum value 3) minimum value. Default exact value  (TBR). Context shall be exact value. | 1 |  |
|  | 24 | .range | N | 0 | 0 | Range (distance) |  |  |
|  | 23:21 | .reservedBits23to21 | N | 0 | 0 | Reserved |  |  |
|  | 20 | .ber | N | 0 | 0 | Eb/No BER |  |  |
|  | 19 | .threshold | N | 0 | 0 | Threshold |  |  |
|  | 18 | .compressionPoint | N | 0 | 0 | Compression point |  |  |
|  | 17 | .ip2and3 | N | 0 | 0 | 2nd and 3rd order intercept points |  |  |
|  | 16 | .snr | N | 0 | 0 | SNR/Noise figure |  |  |
|  | 15 | .auxFreq | E | 0 | 0 | Auxiliary frequency | 2 |  |
|  | 14 | .auxGain | E | 0 | 0 | Auxiliary gain | 1 |  |
|  | 13 | .auxBandwidth | E | 0 | 0 | Auxiliary bandwidth | 2 |  |
|  | 12 | .reservedBit12 | N | 0 | 0 | Reserved |  |  |
|  | 11 | .cifArray | N | 0 | 0 | Array of CIFs |  |  |
|  | 10 | .spectrum | E | 0 | 0 | Spectrum | 13 |  |
|  | 9 | .scanStep | E | 0 | 0 | Sector/step-scan array of records. The array header consists of 3 mandatory words: total array size, header/record counts, and a bitmapped record subfield indicator. Records use the selected Sector/Step-Scan subfields. | -1 | Array-of-records structure. Header word 2 contains HeaderSize in bits 31:24, NumWords/Record in bits 23:12, and NumRecords in bits 11:0. Header word 3 uses bit 31 for sector number, bit 30 for F1 start frequency, bit 29 for F2 stop frequency, bit 28 for resolution bandwidth, bit 27 for tune step size, bit 26 for number of points, bit 25 for default gain, bit 24 for threshold, bit 23 for dwell time, bit 22 for start time, bit 21 for time 3, and bit 20 for time 4. Bits 19:0 are reserved. |
|  | 8 | .reservedBit8 | N | 0 | 0 | Reserved |  |  |
|  | 7 | .indexList | N | 0 | 0 | Index list |  |  |
|  | 6 | .discreteIo32 | N | 0 | 0 | Discrete I/O (32 bit) |  |  |
|  | 5 | .discreteIo64 | N | 0 | 0 | Discrete I/O (64 bit) |  |  |
|  | 4 | .healthStatus | N | 0 | 0 | Health status |  |  |
|  | 3 | .v49compliance | N | 0 | 0 | VITA 49 specification compliance - identifies version of VITA 49 spec implemented, 1 => V49.0, 2 => V49.1, 3=> V49A, 4 => V49.2 |  |  |
|  | 2 | .version | N | 0 | 0 | VITA 49 Version and build code - year, day, sub-version, user defined info. |  |  |
|  | 1 | .bufferSize | N | 0 | 0 | Buffer size |  |  |
|  | 0 | .reservedBit0 | N | 0 | 0 | Reserved |  |  |


### .cif2 (Mandatory)
**Notes:** CIF 2 structure, default and range may be the same as in the associated  Extension Control packet, or it may only contain fields needed by the AckR.  Fields flagged in this page as present are information being sent from the MFA to the MFP, not error or warning flags as in AckV and AckX, and not sending back values previously set by the MFP in a Control Packet as in AckS.

| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Time Determined | Time When Payload Value Determined | Field Size (words) |
|---|---|---|---|---|---|---|---|---|---|
| 11 | 31 | .bind | N | 0 | 0 | Bind |  |  |  |
|  | 30 | .citedSid | B | 1 | 1 | Cited Stream Id - Used to provide data stream ID to Skill. This field may be present to provide a Data Stream SID or index a list of Data Stream SIDs that go with the MFA addresses provided in (4/24) .dataAddressIndex or (4/23) .dataAddressStructure. | Fixed per VA Definition | Dynamic selection among a pre-negotiated set | 1 |
|  | 29 | .sibSid | N | 0 | 0 | Sibling Stream Id |  |  |  |
|  | 28 | .parentSid | N | 0 | 0 |  |  |  |  |
|  | 27 | .childSid | N | 0 | 0 | Children Stream Id |  |  |  |
|  | 26 | .citedMsgId | E | 0 | 0 | Cited message ID |  |  | 1 |
|  | 25 | .controlleeId | N | 0 | 0 | Controllee ID |  |  |  |
|  | 24 | .controlleeUuid | N | 0 | 0 | Controlee UUID |  |  |  |
|  | 23 | .controllerId | N | 0 | 0 | Controller ID |  |  |  |
|  | 22 | .controllerUuid | N | 0 | 0 | Controller UUID |  |  |  |
|  | 21 | .infoSource | E | 0 | 0 | Information source is a pointer to the Configuration Object or Antenna Information File (AIF) that describes the VA (Virtual Antenna) being controlled by this packet. In order to ensure the Configuration Object index is unique, the most significant 16 bits  (bits 31:16) of this field indicate the MEL, and the least significant 16 bits (bits 15:0) indicate the Config Obj. or AIF index for that MEL. |  |  | 1 |
|  | 20 | .trackId | N | 0 | 0 | Track ID |  |  |  |
|  | 19 | .countryCode | N | 0 | 0 | Country code |  |  |  |
|  | 18 | .operator | N | 0 | 0 | Operator |  |  |  |
|  | 17 | .platformClass | N | 0 | 0 | Platform class |  |  |  |
|  | 16 | .platformInst | N | 0 | 0 | Platform instance |  |  |  |
|  | 15 | .platformDisp | N | 0 | 0 | Platform display |  |  |  |
|  | 14 | .emsDeviceClass | N | 0 | 0 | EMS device class |  |  |  |
|  | 13 | .emsDeviceType | N | 0 | 0 | EMS device type |  |  |  |
|  | 12 | .emsDeviceInst | N | 0 | 0 | EMS device instance |  |  |  |
|  | 11 | .modClass | N | 0 | 0 | Modulation class |  |  |  |
|  | 10 | .modType | N | 0 | 0 | Modulation type |  |  |  |
|  | 9 | .functionId | N | 0 | 0 | Function ID |  |  |  |
|  | 8 | .modeId | E | 0 | 0 | Mode ID: User defined, used by communication capability  to control local functions in MFA, e.g., EMCON |  |  | 1 |
|  | 7 | .eventId | N | 0 | 0 | Event ID:  This field is used by the Multi-Function Capability  as in the VITA 49.2 specification and is a generic16 bit field. |  |  | 1 |
|  | 6 | .funcPriorityId | E | 0 | 0 | Function priority ID for resolving conflicting requests to use MFA resources. Unsigned 32-bit integer, larger values indicate  higher priority. |  |  | 1 |
|  | 5 | .commPriorityId | E | 0 | 0 | Communication priority ID for determining access to internal data network when network becomes overloaded. Unsigned 32-bit integer. Bigger is higher priority. |  |  | 1 |
|  | 4 | .rfFootprint | N | 0 | 0 | RF footprint |  |  |  |
|  | 3 | .rfFootprintRange | N | 0 | 0 | RF footprint range |  |  |  |
|  | 2:0 | .reservedBits2to0 | N | 0 | 0 | Reserved |  |  |  |


### .cif3 (Extension)
**Notes:** CIF 3 is not used in the Baseline AckR packet

| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Field Size (words) |
|---|---|---|---|---|---|---|---|
| N/A | 31 | .tsDetails | N | 0 | 0 | Time stamp details |  |
|  | 30 | .tsSkew | N | 0 | 0 | Time stamp skew |  |
|  | 29:28 | .reservedBits29to28 | N | 0 | 0 | Reserved |  |
|  | 27 | .riseTime | N | 0 | 0 | Rise time |  |
|  | 26 | .fallTime | N | 0 | 0 | Fall time |  |
|  | 25 | .offsetTime | N | 0 | 0 | Offset time |  |
|  | 24 | .pulseWidth | N | 0 | 0 | Pulse width |  |
|  | 23 | .period | N | 0 | 0 | Period |  |
|  | 22 | .duration | N | 0 | 0 | Duration |  |
|  | 21 | .dwell | E | 0 | 0:1 | Dwell duration in femtoseconds, where Dwell is the duration of a requested transmitter or receiver event, i.e. the transmitter or receiver has exclusive use of an assigned aperture. If Schedule Request type schReqType=1, a dwell shall begin at the Schedule Request TimeStamp and last for Dwell femtoseconds. For receive events all data samples received during a dwell will be sent to the requesting skill. For transmit events the aperture may transmit one or more data bursts during a dwell, with burst durations defined by the data packet time stamp and size, provided all bursts are entirely contained within the dwell. The dwell value ranges from 0 to ~9223 s ( ~154 minutes). | 2 |
|  | 20 | .jitter | E | 0 | 0:1 | Jitter | 2 |
|  | 19:18 | .reservedBits19to18 | N | 0 | 0 | Reserved |  |
|  | 17 | .age | N | 0 | 0 | Age |  |
|  | 16 | .shelfLife | N | 0 | 0 | Shelf life |  |
|  | 15:8 | .reservedBits15to8 | N | 0 | 0 | Reserved |  |
|  | 7 | .airTemp | N | 0 | 0 | Air temperature |  |
|  | 6 | .seaGroundTemp | N | 0 | 0 | Sea/ground temperature |  |
|  | 5 | .humidity | N | 0 | 0 | Humidity |  |
|  | 4 | .barPressure | N | 0 | 0 | Barometric pressure |  |
|  | 3 | .seaState | N | 0 | 0 | Sea and swell state |  |
|  | 2 | .tropState | N | 0 | 0 | Tropospheric state |  |
|  | 1 | .netId | N | 0 | 0 | Network ID |  |
|  | 0 | .reservedBit0 | N | 0 | 0 | reserved |  |


### .cif4 (Mandatory)
**Notes:** CIF 4 structure, default and range may be the same as in the associated  Extension Control packet, or it may only contain fields needed by the AckR.  Fields flagged in this page as present are information being sent from the MFA to the MFP, not error or warning flags as in AckV and AckX, and not sending back values previously set by the MFP in a Control Packet as in AckS.

| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Time Determined | Time When Payload Value Determined | Field Size (words) | Notes |
|---|---|---|---|---|---|---|---|---|---|---|
| 12 | 31 | .shortTermKey | E | 0 | 0 | This is the TRANSEC short term key for the burst |  |  | 8 |  |
|  | 30 | .reservedBit30 | N | 0 | 0 | Reserved for future cif extensions |  |  |  |  |
|  | 29 | .rfFigureOfMerit | E | 0 | 0 | Bits 31:15 unused, set to 0. Bits 15:0 contain Input G/T (dB/K) to be used for Rx, or Target EIRP (dBm) to be used for Tx.  See Schedule Request for description of rfFigureOfMerit there. |  |  | 1 |  |
|  | 28 | .earlyStartTime | E | 0 | 0 | Earliest allowed start time for flexible schedule request, offset from time stamp |  |  | 2 |  |
|  | 27 | .lateStartTime | E | 0 | 0 | Latest allowed start time for flexible schedule request, offset from time stamp |  |  | 2 |  |
|  | 26 | .rejectReason | B | 1 | 1 | 32 bit enumeration of reasons that this control was rejected by the scheduler. The first 8 bits indicated the requested parameter that caused a failure. Bits 31:29 are the failed parameter CIF # as a 3-bit unsigned integer, except that bits 31:29 = 111 indicates the failure is not tied to a specific parameter. Bits 28:24 indicate the failed parameter CIF Bit # as a 5-bit unsigned integer, and are all 0 if bits 31:29 = 111.  Bits 23:0 form a 24-bit unsigned integer that describes the failure mode. See the Reject Reason Enumeration tab . | Dynamic | Dynamic selection among standard set | 1 | rejectReason describes the reason a Schedule Request was rejected by the VAS, i.e. was not scheduled. In this case .schX (Ack Packet CAM bit 10) = 0 indicates schedule failure and Reject Reason explains why. Failures that occur after a request was schduled (schX=1) are reported by AckX packets. <br>  <br>  Base-set v1.3.2 redifines enum in Bits 23:0 previously defined by AMS GRA:   <br> The reject reason definition includes a new set of enumurations, but includes the previous AMS GRA extend values:  <br> Reasource does not exist = redefined as => 3 <br> Resource is Not Working = redefined as => 4 <br> Requested param outside supported range => 16 <br> Insufficient lead time => 8 <br> Resourec Conflict => 6 |
|  | 25 | .maxDataPacketDwell | E | 0 | 0 | fractional time format given period of time represented by data packet | Fixed | N/A | 2 |  |
|  | 24 | .addressGroupIndex | B | 1 | 1 | Provides a look-up index that points to a collection of MFA address for packets associated with the Schedule Request being acknowledged. That is one or more data packet stream(s) with SID matching the Control SID, or  SID's identified by the SID or index in the field (2/30) .citedSID.  The corresponding MFP addresses for data and Acks are in the same field (CIF 4, Bit 24) of the corresponding Schedule Request packet. |  |  | 1 | AGI points to a table with variable number of entries. Minimum entry is a default address. Every packet sent to the MFA for which an address has not been specified will be sent to the default address. Other possible values are Data Address Index (DAI), and Control Address Index (CAI). |
|  | 23 | .dataAddressStructure | E | 0 | 0 | An array of records listing all the data streams associated with the Control Packet this AckR packet is responding to, by data stream SID and MFA address. When present it may replace the information in (2/30) .citedSID and always replaces the information in (4/24) dataAddressIndex. The corresponding MFP addresses are contained in the same field location (4/25) of the Control Packet this packet is responding to. This array provides greater flexibility than using .citedSId and .dataAddressIndex at the cost of variable-length Control and AckR packets. The array header consists of the 3 mandatory header words (see reference or Change Notes) plus a variable number of records consisting of 2 or 1 words (SID, MFP Address Index) or (MFP Address Index), as indicated in header word 3. |  |  | -1 | Data Address Index and Data Address Structure both indicate the MFP data addresses to which received data should be provided. These two fields are mutually exclusive, consequently at most one field may have a non-zero CIF bit |
|  | 22 | .dataAddressTime | E | 0 | 0 | If the data address(es) of 4/24 or 4/23 are not immediately applicable (e.g., If the MFA must switch antenna faces during a stream of VITA data packets transporting one long Rx/Tx packet, the MFA scheduler may send a new AckR packet with the new MFA port addresses with some lead time before the change), The .dataAddressTime parameter indicates when the data address information in 4/24 or 4/23 becomes applicable. Offset from time stamp. |  |  | 2 |  |
|  | 21 | .txDigitalInputPower | E | 0 | 0 | Bits 15:0 contain Nominal power level of digital TX signal input to MFA for transmission, dBfs, where "full scale power" shall be defined as the power of a complex digital sinusoid with amplitude equal to full scale. Set to 0 for Rx. Set bits 31:16 to 0. |  |  | 1 |  |
|  | 20:0 | .reservedBits20to0 | N | 0 | 0 | Reserved for future extensions |  |  |  |  |


### .cif7 (Forbidden)
| Word | Bit | Designation | Bit Used | Default Value | Range | Function |
|---|---|---|---|---|---|---|
| N/A | 31:0 | .cif7 | N | 0 | 0 | Control Indicator Field 7 is not used in the base set |


### .payloadFields (Mandatory)
**Notes:** Payload fields are defined by enabled CIF indicators and the Control-Context Payload Formats table. Payload fields are included in CIF-bit order and may contain additional fields from Extension Set. This group is a metadata placeholder and has no fixed rows.



## ExecutionAck(AckX)Packet

Extension AckX Packet Template (Execution Acknowledgement packet)

### .header (Mandatory)
**Notes:** These fields must mirror the corresponding control packet

| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Time Determined | Time When Payload Value Determined |
|---|---|---|---|---|---|---|---|---|
| 1 | 31:28 | .type | B | 0111 | 0111 | Packet Type: 0111 => Extension Command Packet | Fixed | N/A |
|  | 27 | .cBit | B | 1 | 1 | Indicates whether CLASS ID is included in the prologue | Fixed | N/A |
|  | 26 | .isAck | B | 1 | 1 | Indicates THAT THIS IS an acknowledge packet | Fixed | N/A |
|  | 25 | .reservedBit25 | N | 0 | 0 | Reserved | N/A | N/A |
|  | 24 | .isCancellation | B | 0 | 0 | Indicates whether this is cancellation packet | Fixed | N/A |
|  | 23:22 | .tsi | B | 11 | 11 | Indicates: 00 No int seconds time stamp, 01 => UTC, 10 => GPS, 11 => Other | Fixed per VA Definition | N/A |
|  | 21:20 | .tsf | B | 10 | 10 | Indicates: 00 => No frac seconds field, 01 => sample count, 10 => picoseconds, 11 => free running count | Fixed per VA Definition | N/A |
|  | 19:16 | .packetCount | B | 0 | 0:15 | 4-Bit Packet sequence number for this packet type in this streamID | Dynamic | N/A |
|  | 15:0 | .packetSize | B | N/A | 0:2^16-1 | Total number of 32-bit words in the packet including the prologue; computed from the final packet word count. | Fixed per VA Definition | N/A |


### .streamId (Stream Identifier) (Mandatory)
| Word | Bit | Designation | Bit Used | Default Value | Function | Time Determined | Time When Payload Value Determined | Notes |
|---|---|---|---|---|---|---|---|---|
| 2 | 31:0 | .sid | B | 0 | See Control and Context SID definitions.  Data SIDs will match the applicable control SIDs | VA Initialization | N/A | See Control Tab |


### .classId (Class Identifier) (Mandatory)
**Notes:** Packet Class Identifier bits should match Extension Control Packet Class ID bits

| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Time Determined | Time When Payload Value Determined |
|---|---|---|---|---|---|---|---|---|
| 3 | 31:27 | .padBitCount | E | 0 | 0 | Number of pad bits (0…31) at the end of the packet to hit a 32-bit word boundary. Not used by Base Set. | Fixed per VA Definition | N/A |
|  | 26:24 | .reservedBits26to24 | N | 0 | 0 | Reserved | N/A | N/A |
|  | 23:0 | .oui | B | 0xAAAAAA | 0xAAAAAA | Organization Number. Organizationally Unique Identifier (OUI),  "A 24-bit number that uniquely identifies a vendor, manufacturer, or other organization globally or worldwide" | Fixed per VA Definition | N/A |
| 4 | 31:24 | .infoClassCode_Type | B | 0x04 | 4:5 | 0 => Unknown; 1 => TxComm, 2 => RxComm, 3 => Sensor;  4 => TxComm base set; 5 => RxComm Base Set; 6 => TxComm extended set 1; 7 => RxComm extended set 1; 8 => TxComm  extended set 2; 9 => RxComm extended set 2; (10-255) => TBD | Fixed per VA Definition | N/A |
|  | 23:16 | .infoClassCode_Version | B | 0x00 | 0:2^8-1 | Version number of information class | Fixed per VA Definition | N/A |
|  | 15:8 | .packetClassCode_Type | B | 0x0C | 0x0C | Defined PacketTypes:  0x00 => undefined, 0x0A => Base Set Control ScheduleRequest, 0x0B => Base Set Schedule Acknowledge (AckR),  0x0C => Base Set Execution Acknowledge (AckX), 0x0D => Base Set Data, 0x0E => Base Set DataContext | Fixed per VA Definition | N/A |
|  | 7:0 | .packetClassCode_Version | B | 0x05 | 0:2^8-1 | Version of Packet Class definition for specified type | Fixed per VA Definition | N/A |


### .timeStamp (Time Stamp) (Mandatory)
| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Time Determined | Time When Payload Value Determined |
|---|---|---|---|---|---|---|---|---|
| 5 | 31:0 | .tsi | B | 0 | 0:2^32-1 | Integer time stamp, UTC seconds since midnight, Jan. 1, 2019 | Dynamic within per VA limits | N/A |
| 6 | 31:0 | .tsf | B | 0 | 0:2^64-1 | Fractional time stamp most-significant (Upper 32 bits); format indicated by Header.tsf, picoseconds since last seconds update | Dynamic within per VA limits | N/A |
| 7 | 31:0 | .tsfLower | B | 0 |  | Fractional time stamp least-significant (Lower 32 bits); format indicated by Header.tsf, picoseconds since last seconds update | Dynamic within per VA limits | N/A |


### .cam (Control/Ack Mode Field) (Mandatory)
| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Time Determined | Time When Payload Value Determined | Notes |
|---|---|---|---|---|---|---|---|---|---|
| 8 | 31 | .ce | N | 0 | 0 | Identifier fields are not used in the AMS Base Set; set to zero |  |  |  |
|  | 30 | .ie | N | 0 | 0 | Identifier fields are not used in the AMS Base Set; set to zero |  |  |  |
|  | 29 | .cr | N | 0 | 0 | Identifier fields are not used in the AMS Base Set; set to zero |  |  |  |
|  | 28 | .ir | N | 0 | 0 | Identifier fields are not used in the AMS Base Set; set to zero |  |  |  |
|  | 27 | .p | E | 0 | 0 | Matches value in Control Packet being Acknowledged |  |  |  |
|  | 26 | .w | E | 0 | 0 | Matches value in Control Packet being Acknowledged |  |  |  |
|  | 25 | .er | E | 0 | 0 | Matches value in Control Packet being Acknowledged |  |  |  |
|  | 24:23 | .actionBits | B | 10 | 10 | Matches value in Control Packet being Acknowledged |  |  |  |
|  | 22 | .nack | E | 0 | 0 | Matches value in Control Packet being Acknowledged |  |  |  |
|  | 21 | .reservedBit21 | N | 0 | 0 | Matches value in Control Packet being Acknowledged |  |  |  |
|  | 20 | .ackV | B | 0 | 0 | 1 = > Indicates this is a Validation Ack packet, 0 => otherwise. AckV, AckX, and AckS are mutually exclusive. | Fixed | N/A |  |
|  | 19 | .ackX | B | 1 | 1 | 1 = > Indicates this is an Execution Ack packet, 0 => otherwise. AckV, AckX, and AckS are mutually exclusive. | Fixed | N/A |  |
|  | 18 | .ackS | N | 0 | 0 | 1 => Indicates this is a Query-State Ack packet, 0 => otherwise. AckV, AckX, and AckS are mutually exclusive. | Fixed | N/A |  |
|  | 17 | .ackW | N | 0 | 0 | Warning reports are not supported in AckX by the AMS Base Set; set to zero | Dynamic | N/A |  |
|  | 16 | .ackEr | B | 0 | 0:1 | Indicates whether errors were generated | Dynamic | N/A | In AckX packets that support the Baseline Error Reporting format this bit indicates the presence of non-zero error indicator bits. When ackEr is 1, EIFs shall be present and shall identify the reported error payload fields. When ackEr is 0, no error fields are reported and EIFs shall not be present. |
|  | 15 | .ackR | B | 0 | 0 | 1 => Indicates this is an Information-Response Ack (AckR) packet, 0 => otherwise | Fixed | N/A |  |
|  | 14:12 | .timingControl | E | 0 | 0 | 000 => Executed the controls fields with no regard to timestamp constraints <br> 001 => Executed within the device timing precision window <br> 010 => Executed within device timing precision  window or within late window <br> 011 => Executed within device timing precision window or withing early timing  window <br> 100 => Executed within application timing execution window <br> 101 => reserved <br> 110 => reserved <br> 111 => Did not execute some controls | TBD | N/A |  |
|  | 11 | .ackP | E | 0 | 0 | Indicates validation Partially completed (This Class is not using this bit) | Dynamic | N/A |  |
|  | 10 | .schX | B | 1 | 0:1 | 0 => one or more fields can NOT be scheduled for execution at time in timestamp, 1 => all fields can be scheduled for execution at time in timestamp.  A self-scheduling MFA may send an AckR packet with .schX = 0 if the access requested by a Control Packet could not be scheduled. If a request was scheduled and an AckR packet was sent with .schX = 1, after which the packet was bumped from the schedule by a higher priority request, a second AckR or an AckV packet with .schX = 0 may be sent. An AckX packet with .schX = 0 shall be sent if the control packet was not executed due to schedule conflicts, e.g., the allowed (range of) execution time(s) has passed. | Dynamic | N/A | AMS GRA extention allows for AckR or AckV packet response to schedule conflicts |
|  | 9:8 | .reservedBits9to8 | N | 0 | 0 | Reserved |  |  |  |
|  | 7:4 | .schReqType | E | 0 | 0:15 | Matches values in Control Packet being Acknowledged |  |  |  |
|  | 3 | .reqStatusChange | E | 0 | 0 | This doesn't really make sense in an AckV or AckX packet. Set to 0. | Fixed | N/A |  |
|  | 2:0 | .reservedBits2to0 | N | 0 | 0 | Reserved |  |  |  |


### .messageId (Unique Message Identifier) (Mandatory)
| Word | Bit | Designation | Bit Used | Default Value | Range | Notes |
|---|---|---|---|---|---|---|
| 9 | 31:0 | .messageId | B | 0 | 0:2^32-1 | Same as Message Id of Control Packet being acknowledged |


### .controlleeId (Controllee Identifier, format is either 32-bit or 128-bit and is indicated by .cam.ie) (Forbidden)
| Word | Bit | Designation | Bit Used | Default Value |
|---|---|---|---|---|
| 10 | 31:0 | .controlleeIdWord0 | N | 0 |
| 10 | 127:96 | .controlleeIdWord1 | N | 0 |
| 11 | 95:64 | .controlleeIdWord2 | N | 0 |
| 12 | 63:32 | .controlleeIdWord3 | N | 0 |
| 13 | 31:0 | .controlleeIdWord4 | N | 0 |


### .controllerId (Controller Identifier, format is either 32-bit or 128-bit and is indicated by .cam.ir) (Forbidden)
| Word | Bit | Designation | Bit Used | Default Value |
|---|---|---|---|---|
| 14 | 31:0 | .controllerIdWord0 | N | 0 |
| 14 | 127:96 | .controllerIdWord1 | N | 0 |
| 15 | 95:64 | .controllerIdWord2 | N | 0 |
| 16 | 63:32 | .controllerIdWord3 | N | 0 |
| 17 | 31:0 | .controllerIdWord4 | N | 0 |


### .wif0 (Warning Indicator Field has same structure as Control Packet's Control Indicator Fields) (Forbidden)
**Notes:** The Base Set does not use WIFs.  WIFs shall be present if .ackW = 1 above, and .reqW = 1 in the corresponding Control Packet. WIF 0 structure is the same as in the associated Extension Control packet CIF 0 Structure. A WIF bit shall be set to 1 only when there is a warning for the associated field.  Associated field warnings all follow the 32-bit warning/error format below.

| Word | Bit | Designation | Bit Used | Default Value | Range | Function |
|---|---|---|---|---|---|---|
| N/A | 31:0 | .wif0 | E | 0 | 0:1 | Warning fields follow the 32-bit warning/error format below. |


### .wif1 (Warning Indicator Field has same structure as Control Packet's Control Indicator Fields) (Forbidden)
**Notes:** The Base Set does not use WIFs.  WIFs shall be present if .ackW = 1 above, and .reqW = 1 in the corresponding Control Packet. WIF 1 structure is the same as in the associated Extension Control packet CIF 1 Structure. A WIF bit shall be set to 1 only when there is a warning for the associated field.  Associated field warnings all follow the 32-bit warning/error format below.

| Word | Bit | Designation | Bit Used | Default Value | Range | Function |
|---|---|---|---|---|---|---|
| N/A | 31:0 | .wif1 | E | 0 | 0:1 | Warning fields follow the 32-bit warning/error format below. |


### .wif2 (Warning Indicator Field has same structure as Control Packet's Control Indicator Fields) (Forbidden)
**Notes:** The Base Set does not use WIFs.  WIFs shall be present if .ackW = 1 above, and .reqW = 1 in the corresponding Control Packet. WIF 2 structure is the same as in the associated Extension Control packet CIF 2 Structure. A WIF bit shall be set to 1 only when there is a warning for the associated field.  Associated field warnings all follow the 32-bit warning/error format below.

| Word | Bit | Designation | Bit Used | Default Value | Range | Function |
|---|---|---|---|---|---|---|
| N/A | 31:0 | .wif2 | E | 0 | 0:1 | Warning fields follow the 32-bit warning/error format below. |


### .wif3 (Warning Indicator Field has same structure as Control Packet's Control Indicator Fields) (Forbidden)
**Notes:** The Base Set does not use WIFs.  WIFs shall be present if .ackW = 1 above, and .reqW = 1 in the corresponding Control Packet. WIF 3 structure is the same as in the associated Extension Control packet CIF 3 Structure. A WIF bit shall be set to 1 only when there is a warning for the associated field.  Associated field warnings all follow the 32-bit warning/error format below.

| Word | Bit | Designation | Bit Used | Default Value | Range | Function |
|---|---|---|---|---|---|---|
| N/A | 31:0 | .wif3 | E | 0 | 0:1 | Warning fields follow the 32-bit warning/error format below. |


### .wif4 (Warning Indicator Field has same structure as Control Packet's Control Indicator Fields) (Forbidden)
**Notes:** The Base Set does not use WIFs.  WIFs shall be present if .ackW = 1 above, and .reqW = 1 in the corresponding Control Packet. WIF 4 structure is the same as in the associated Extension Control packet CIF 4 Structure. A WIF bit shall be set to 1 only when there is a warning for the associated field.  Associated field warnings all follow the 32-bit warning/error format below.

| Word | Bit | Designation | Bit Used | Default Value | Range | Function |
|---|---|---|---|---|---|---|
| N/A | 31:0 | .wif4 | E | 0 | 0:1 | Warning fields follow the 32-bit warning/error format below. |


### .wif7 (Warning Indicator Field has same structure as Control Packet's Control Indicator Fields) (Forbidden)
**Notes:** The Base Set does not use WIFs.  WIFs shall be present if .ackW = 1 above, and .reqW = 1 in the corresponding Control Packet. WIF 7 structure is the same as in the associated Extension Control packet CIF 7 Structure. A WIF bit shall be set to 1 only when there is a warning for the associated field.  Associated field warnings all follow the 32-bit warning/error format below.

| Word | Bit | Designation | Bit Used | Default Value | Range | Function |
|---|---|---|---|---|---|---|
| N/A | 31:0 | .wif7 | E | 0 | 0:1 | Warning fields follow the 32-bit warning/error format below. |


### .eif0 (Error indicator field has same structure as Control Packet's Control Indicator Fields) (Optional)
**Notes:** EIF0 shall  be present in AckX packets when an error condition occurs. The error may be associated with any field, or may not be assoicated with a specific field. When there is no error reported, EIF 0 it is not present. EIF 0 structure is the same as in the associated Extension Control (Schedule Request) packet CIF 0 Structure. An EIF bit shall be set to 1 when there is an error message for an associated field.  Associated-field error messages all follow the 32-bit warning/error format below.

| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Field Size (words) | Notes |
|---|---|---|---|---|---|---|---|---|
| 10 | 31 | .cfci | B | 0 | 0:1 | An Error has occurred with no correspoinding Payload Field |  | 1 => an Error Payload Field is included below describing an error  that does not correspond to any of the Command Payload Fields supported |
|  | 29 | .bandwidth | B | 0 | 0:1 | 3 dB Bandwidth | 2 |  |
|  | 27 | .rfRefFreq | B | 0 | 0:1 | RF reference frequency.  This is usually the center of the analog superchannel and will always be a large positive number. If rfRefFreqOffset=0, rfRefFreq translatetes to ifRefFreq during frequency converslin. | 2 |  |
|  | 23 | .gain | B | 0 | 0:1 | Gain bits 31:16 are not used in the base set and shall be set to 0. Bits 15:0 should be used in Base Set Rx Schedule Requests to control signal amplitude into the MFA ADC. Gain shall not be used on Tx, as it will conflict with EIRP (figure of merit, CIF 4/29). Gain bits 15:0 shall be set to 0 in Tx Schdule Requests. | 1 |  |
|  | 21 | .sampleRate | B | 0 | 0:1 | Sample rate in Hz | 2 |  |
|  | 15 | .dataFormat | B | 0 | 0:1 | Signal data packet payload format.  We should always send the data packet format | 2 |  |
|  | 4 | .eif4enable | B | 0 | 0:1 | Indicates presence of EIF 4 in packet |  | EIF 4 is  present when a CIF 4 Parameter causes an error |
|  | 3 | .eif3enable | B | 0 | 0:1 | Indicates presence of EIF 3 in packet |  | EIF 3 is  present when a CIF3 Parameter causes an error |
|  | 2 | .eif2enable | B | 0 | 0:1 | Indicates presence of EIF 2 in packet |  | EIF 2 is  present when a CIF 2 Parameter causes an error |
|  | 1 | .eif1enable | B | 0 | 0:1 | Indicates presence of EIF 1 in packet |  | EIF 1 is  present when a CIF 1 Parameter causes an error |


### .eif1 (Error indicator field has same structure as Control Packet's Control Indicator Fields) (Optional)
**Notes:** EIF1 shall  be present in AckX packets when a CIF 1 field causes an error. EIF 1 structure is the same as in the associated Extension Control (Schedule Request) packet CIF 1 Structure. An EIF bit shall be set to 1 when there is an error message for an associated field. Associated-field error messages all follow the 32-bit warning/error format below.

| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Field Size (words) |
|---|---|---|---|---|---|---|---|
| 11 | 30 | .polarization | B | 0 | 0:1 | Polarization: Upper bits 31:16 indicate polarization ellipse tilt angle, lower bits 15:0 indicate ellipse eccentricity. Both are measured in radians. Tilt angle is the angle of the polarization-ellipse major axis  measured counter-clockwise from the array/antenna plane positive x-axis [2,4], with the x-axis defined in system documentation. Ellipticity describes the eccentricity of the polarization ellipse, including direction of rotation where appropriate. Linear polarization is described by ellipticity = π/4. For tilt = 0 linear polarization is horizontal, and for tilt = π/2 linear polarization is vertical, thus horzontal polarization is parallel to the antenna x-axis, and vertical polarization parallel to the antenna y-axis, both as defined in system documentation.  Ellipticity = 0 indicates right-handed circular polarization, and ellipticity = π/2 indicates left-handed circular polarization, where left- or right-handed rotation shall be defined  from the point of view of an observer at the receiver looking toward the transmitter [2,4].  Note that the direction in which tilt angle is measured, and the definitions of left- and right-handed rotation are inconsistent with IEEE standards [3]. | 1 |
|  | 29 | .pointing3d | B | 0 | 0:1 | 3-D pointing vector. Indicates pointing beam direction relative to the assigned aperture boresight. 3D pointing has two subfields: Bits 31:16 are elevation, bits 15:0 are azimuth, both in the antenna frame of reference. Define antenna boresight as the z-axis, the x axis is orthogonal to z and "horizontal" in the frame of reference defined by the antenna. The y axis points "up" in the antenna frame and is 90° clockwise from the x axis when looking out along boresight. Elevation is measured up (positive y direction) from the x-z plain to the pointing vector. Azimuth is measured in the x-z plain, between the projection of the pointing vector onto the x-z plain and boresight.  Positive azimuth angles are measured clockwise from boresight when looking down (from positive y toward the x-z plain). | 1 |
|  | 25 | .beamWidth | B | 0 | 0:1 | Beamwidth: Indicate the desired beamwidth | 1 |


### .eif2 (Error indicator field has same structure as Control Packet's Control Indicator Fields) (Optional)
**Notes:** EIF2 shall  be present in AckX packets when a CIF 2 field causes an error. EIF 2 structure is the same as in the associated Extension Control (Schedule Request) packet CIF 2 Structure. An EIF bit shall be set to 1 when there is an error message for an associated field. Associated-field error messages all follow the 32-bit warning/error format below.

| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Field Size (words) |
|---|---|---|---|---|---|---|---|
| 12 | 6 | .funcPriorityId | B | 0 | 0:1 | Function priority ID for resolving conflicting requests to use MFA resources. Unsigned 32-bit integer, bigger is higher priority | 1 |


### .eif3 (Error indicator field has same structure as Control Packet's Control Indicator Fields) (Optional)
**Notes:** EIF3 shall  be present in AckX packets when a CIF 3 field causes an error. EIF 3 structure is the same as in the associated Extension Control (Schedule Request) packet CIF 3 Structure. An EIF bit shall be set to 1 when there is an error message for an associated field. Associated-field error messages all follow the 32-bit warning/error format below.

| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Field Size (words) |
|---|---|---|---|---|---|---|---|
| 13 | 21 | .dwell | B | 0 | 0:1 | Dwell duration in femtoseconds, where Dwell is the duration of a requested transmitter or receiver event, i.e. the transmitter or receiver has exclusive use of an assigned aperture. If Schedule Request type schReqType=1, a dwell shall begin at the Schedule Request TimeStamp and last for Dwell femtoseconds. For receive events all data samples received during a dwell will be sent to the requesting skill. For transmit events the aperture may transmit one or more data bursts during a dwell, with burst durations defined by the data packet time stamp and size, provided all bursts are entirely contained within the dwell. The dwell value ranges from 0 to ~9223 s ( ~154 minutes). | 2 |


### .eif4 (Error indicator field has same structure as Control Packet's Control Indicator Fields) (Optional)
**Notes:** EIF4 shall  be present in AckX packets when a CIF 4 field causes an error. EIF 4 structure is the same as in the associated Extension Control (Schedule Request) packet CIF 4 Structure. An EIF bit shall be set to 1 when there is an error message for an associated field. Associated-field error messages all follow the 32-bit warning/error format below.

| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Field Size (words) |
|---|---|---|---|---|---|---|---|
| 14 | 29 | .rfFigureOfMerit | B | 0 | 0:1 | Requested input G/T (dB/K) range for Rx, requested EIRP (dBm) range on Tx.  The MFA shall, to the best of its ability, meet the EIRP or G/T request in the lower 16 bits of this word (bits 15:0) at any pointing angle inside the documented aperture Field of Regard. Calculation of EIRP or G/T values  supported by an MFA shall include the effects of scan angle, beamwidth, and frequency, as appropriate. The EIRP calculation shall include polarization mismatch only if transmit polarization realized is different from polarization requested, in which case it shall be assumed the receiver is configured for the requested polarization. G/T calculation shall not include the effects of external noise. The MFA shall provide an EIRP  as close to the requested value (bits 15:0) as possible without exceeding it. If the maximum available EIRP is below the lower limit in bits 31:16, the MFA shall reject the schedule request. Requests with pointing angle and/or other parameters that result in G/T below the value provided in the upper 16 bits (bits 31:16) shall be rejected by the scheduler. Configurations producing G/T greater than the requested value (bits 15:0) may be allowed.  Note that VITA 49.2 defines polariation angles and rotation from the point of view of an observer at the receiver, rather than at the transmitter as in [3]. | 1 |
|  | 24 | .addressGroupIndex | B | 0 | 0:1 | Provides a look-up index that points to a collection of MFP address for data and Ack packets associated with this Schedule Request. That is one or more data packet stream(s) with SID matching the Control SID, or  SID's identified by the SID or index in the field (2/30) .citedSID. Plus AckR and AckX packet streams with SID matching the Control SID.  The corresponding MFA addresses are in the same field (CIF 4, Bit 24) of the corresponding AckR packet. | 1 |
|  | 21 | .txDigitalInputPower | B | 1 | 0:1 | Bits 15:0 contain Nominal power level of digital TX signal input to MFA for transmission, dBfs, where "full scale power" shall be defined as the power of a complex digital sinusoid with amplitude equal to full scale. Set to 0 for Rx. Set bits 31:16 to 0. | 1 |


### .eif7 (Error indicator field has same structure as Control Packet's Control Indicator Fields) (Forbidden)
**Notes:** EIF7 is not supported in the Baseline AckX packet

| Word | Bit | Designation | Bit Used | Default Value | Range | Function |
|---|---|---|---|---|---|---|
| N/A | 31:0 | .eif7 | N | 0 | 0:1 | Error fields follow the 32-bit warning/error format below. |


### .wifPayloadFields  (The following is an example of a warning field whose presence is indicated by the WIFs) (Forbidden)
**Notes:** Warning Payload Fields are not supported in the Base Set

| Word | Bit | Designation | Bit Used | Default Value | Range | Function |
|---|---|---|---|---|---|---|
| N/A | 31 | .notExecuted | N | 0 | 0:1 | Field not executed due to warning or error |
|  | 30 | .deviceFailure | N | 0 | 0:1 | Field not executed properly  due to a device failure |
|  | 29 | .erroneousField | N | 0 | 0:1 | Device does not support the field |
|  | 28 | .outOfRange | N | 0 | 0:1 | Supplied field value is beyond the capability or operational range of this device |
|  | 27 | .unsupportedPrecision | N | 0 | 0:1 | Supplied field value specifies a level of precision beyond the capability of this device |
|  | 26 | .invalidValue | N | 0 | 0:1 | Field had an invalid setting beyond those specified above |
|  | 25 | .badTimestamp | N | 0 | 0:1 | Controllee was unable to meet the timestamp requirement specified by the [T2,T1,T0] bits for the specified field. |
|  | 24 | .hazardousPowerLevels | N | 0 | 0:1 | Supplied field will cause transmission of hazardous power levels |
|  | 23 | .distortion | N | 0 | 0:1 | Supplied field will cause components to be over driven eading to distortion |
|  | 22 | .inBandPowerCompliance | N | 0 | 0:1 | Supplied field will place the in-band power levels out of compliance |
|  | 21 | .outOfBandPowerCompliance | N | 0 | 0:1 | supplied field will place the out-of-band power levels out of compliance |
|  | 20 | .coSiteInterference | N | 0 | 0:1 | Supplied field will cause co-site interference between transmitter and receiver at same location |
|  | 19 | .regionalInterference | N | 0 | 0:1 | Supplied field will cause interference between devices in same operational region |
|  | 18:13 | .reservedBits18to13 | N | 0 | 0 | Reserved |
|  | 12:1 | .userDefinedWarningErrors | N | 0 | 0:1 | User specified custom error types |
|  | 0 | .reservedBit0 | N | 0 | 0 | Reserved |


### .errorPayloadFields (Variable)
**Notes:** A variable number of error payload fields are included to describe errors flagged in EIF fields. Error payload fields are provided in the same order as the EIF bits. See Error Payload Fields.

| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Notes |
|---|---|---|---|---|---|---|---|
| 15 | 31 | .notExecuted | B | 0 | 0:1 | Field not executed due to warning or error |  |
|  | 30 | .deviceFailure | B | 0 | 0:1 | Field not executed properly  due to a device failure |  |
|  | 29 | .erroneousField | B | 0 | 0:1 | Device does not support the field |  |
|  | 28 | .outOfRange | B | 0 | 0:1 | Supplied field value is beyond the capability or operational range of this device |  |
|  | 27 | .unsupportedPrecision | B | 0 | 0:1 | Supplied field value specifies a level of precision beyond the capability of this device |  |
|  | 26 | .invalidValue | B | 0 | 0:1 | Field had an invalid setting beyond those specified above |  |
|  | 25 | .badTimestamp | B | 0 | 0:1 | Controllee was unable to meet the timestamp requirement specified by the [T2,T1,T0] bits for the specified field. |  |
|  | 24 | .hazardousPowerLevels | B | 0 | 0:1 | Supplied field will cause transmission of hazardous power levels |  |
|  | 23 | .distortion | B | 0 | 0:1 | Supplied field will cause components to be over driven eading to distortion |  |
|  | 22 | .inBandPowerCompliance | B | 0 | 0:1 | Supplied field will place the in-band power levels out of compliance |  |
|  | 21 | .outOfBandPowerCompliance | B | 0 | 0:1 | supplied field will place the out-of-band power levels out of compliance |  |
|  | 20 | .coSiteInterference | B | 0 | 0:1 | Supplied field will cause co-site interference between transmitter and receiver at same location |  |
|  | 19 | .regionalInterference | B | 0 | 0:1 | Supplied field will cause interference between devices in same operational region |  |
|  | 18 | .reservedBit18 | B | 0 | 0 |  |  |
|  | 17:15 | .eifNo | B | 0 | 0:4 | Number of EIF containing error flag this error payload field reports on | See Error Payload Field tab |
|  | 14:10 | .eifBitNo | B | 0 | 0:31 | Bit Number within EIF of error flag this error payload field reports on |  |
|  | 9:0 | .errorEnumIndex | B | 0 | 0:1023 | Index into Error Enumeration List: see Error Payload Field Tab |  |


## ExtensionDataContextPacket

Base Set Extension Data Context Packet Class

### .header (Mandatory)
| Word | Bit | Designation | Bit Used | Default Value | Range | Function |
|---|---|---|---|---|---|---|
| 1 | 31:28 | .type | B | 0101 | 0101 | Packet Type: 0101 => Extension Context Packet |
|  | 27 | .cBit | B | 1 | 1 | Indicates whether CLASS ID is included in the prologue |
|  | 26 | .reservedBit26 | N | 0 | 0 | Reserved |
|  | 25 | .notV49p0Packet | N | 0 | 0 | Not a V49.0 Packet |
|  | 24 | .timeStampMode | B | 0 | 0 | Timestamp Mode, 0=> fine, 1=>coarse (see 7.1.1, 7.1.3 and Permission 7.1.4-2) |
|  | 23:22 | .tsi | B | 11 | 11 | Indicates: 00 No int seconds time stamp, 01 => UTC, 10 => GPS, 11 => Other |
|  | 21:20 | .tsf | B | 10 | 10 | Indicates: 00 => No frac seconds field, 01 => sample count, 10 => picoseconds, 11 => free running count |
|  | 19:16 | .packetCount | B | 0 | 0:15 | 4-Bit Packet type sequence number |
|  | 15:0 | .packetSize | B | 29 | 29 | Total number of 32-bit words in the packet including the prologue |


### .streamId (Stream Identifier) (Mandatory)
| Word | Bit | Designation | Bit Used | Default Value | Function | Notes |
|---|---|---|---|---|---|---|
| 2 | 31:0 | .sid | B | 0 | Stream ID provided in associated AckR packets used to schedule access. May match associated data stream. Or not. | See Control Tab |


### .classId (Class Identifier) (Mandatory)
| Word | Bit | Designation | Bit Used | Default Value | Range | Function |
|---|---|---|---|---|---|---|
| 3 | 31:27 | .padBitCount | E | 0 | 0 | Number of pad bits (0…31) at the end of the packet to hit a 32-bit word boundary. Not used by Base Set. |
|  | 26:24 | .reservedBits26to24 | N | 0 | 0 | Reserved |
|  | 23:0 | .oui | B | 0xAAAAAA | 0xAAAAAA | Organization Number. Organizationally Unique Identifier (OUI),  "A 24-bit number that uniquely identifies a vendor, manufacturer, or other organization globally or worldwide" |
| 4 | 31:24 | .infoClassCode_Type | B | 0x05 | 4:5 | 0 => Unknown; 1 => TxComm, 2 => RxComm, 3 => Sensor;  4 => TxComm base set; 5 => RxComm Base Set; 6 => TxComm extended set 1; 7 => RxComm extended set 1; 8 => TxComm  extended set 2; 9 => RxComm extended set 2; (10-255) => TBD |
|  | 23:16 | .infoClassCode_Version | B | 0x00 | 0:2^8-1 | Version number of information class |
|  | 15:8 | .packetClassCode_Type | B | 0x0E | 0x0E | Defined PacketTypes:  0x00 => undefined, 0x0A => Base Set Control ScheduleRequest, 0x0B => Base Set Schedule Acknowledge (AckR),  0x0C => Base Set Execution Acknowledge (AckX), 0x0D => Base Set Data, 0x0E => Base Set DataContext |
|  | 7:0 | .packetClassCode_Version | B | 0x05 | 0:2^8-1 | Version of Packet Class definition for specified type |


### .timeStamp (Time Stamp) (Mandatory)
| Word | Bit | Designation | Bit Used | Default Value | Range | Function |
|---|---|---|---|---|---|---|
| 5 | 31:0 | .tsi | B | 0 | 0:2^32-1 | Integer time stamp, UTC seconds since midnight, Jan. 1, 2019 |
| 6 | 63:32 | .tsf | B | 0 | 0:2^64-1 | Fractional time stamp most-significant (Upper 32 bits); format indicated by header.tsf, picoseconds since last seconds update |
| 7 | 31:0 | .tsfLower | B | 0 |  | Fractional time stamp least-significant (Lower 32 bits); format indicated by header.tsf, picoseconds since last seconds update |


### .cif0 (Control Indicator Field 0, Legacy Fields) (Mandatory)
**Notes:** See Cotrol-Context Payload Formats

| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Field Size (words) |
|---|---|---|---|---|---|---|---|
| 8 | 31 | .cfci | B | 1 | 1 | Context field change indicator: Control packets are only sent when something needs to be changed so this should be set to 1 | 0 |
|  | 30 | .rpi | N | 0 | 0 | Reference point identifier |  |
|  | 29 | .bandwidth | B | 1 | 1 | 3 dB Bandwidth | 2 |
|  | 28 | .ifRefFreq | E | 0 | 0 | IF reference frequency.  This is the digital frequency offset within the superchannel of the transmission burst.  It is referenced to the center of the superchannel.  Hence the ifRefFreq can be positive or negative. | 2 |
|  | 27 | .rfRefFreq | B | 1 | 1 | RF reference frequency.  This is the center of the superchannel and will always be a large positive number. | 2 |
|  | 26 | .rfRefFreqOffset | E | 0 | 0 | Offset from RF Ref Freq of RF frequency that translates to/from IF Ref Freq | 2 |
|  | 25 | .ifBandOffset | E | 0 | 0 | IF band offset | 2 |
|  | 24 | .refLevel | E | 0 | 0 | CIF0/24 Reference level relates digital signal amplitude to analog signal power. An analog Reference Point and location for the digital Described Signal must be defined in system documentation. The Reference Level is the AC power of a single analog sine wave at the reference point that produces a unit-scale digital sine wave as the Described Signal. Note that a unit scale sine wave for N-bit two's complement data is defined as one with amplitude ranging from -2^(N-1) to +2^(N-1), which exceeds the available range of an N-bit two's complement number at its peak value. | 1 |
|  | 23 | .gain | B | 1 | 1 | Gain bits 31:16 are not used in the base set and shall be set to 0. Bits 15:0 should be used in Base Set Rx Schedule Requests to control signal amplitude into the MFA ADC. Context Packet Gain Bits 15:0 shall be used to indicate the MFA  Gain setting, which may differ from the requested value. Gain Bits shall not be used on Tx, as it will conflict with EIRP (figure of merit, CIF 4/29). Gain bits 15:0 shall be set to 0 in Tx Schdule Request and Context Packets. | 1 |
|  | 22 | .overRangeCount | N | 0 | 0 | Over-range count |  |
|  | 21 | .sampleRate | B | 1 | 1 | Sample rate in Hz | 2 |
|  | 20 | .tsa | N | 0 | 0 | Time stamp adjustment |  |
|  | 19 | .tsCalTime | N | 0 | 0 | Time stamp calibration time |  |
|  | 18 | .temp | N | 0 | 0 | Temperature |  |
|  | 17 | .deviceId | N | 0 | 0 | Device identifier |  |
|  | 16 | .sei | N | 0 | 0 | State and Event Indicator. Eight predefined indicator bits with corresponding enable bits, plus 8 user-defined bits. Bits 28 and 16 indicate whether AGC is in use. See [2] Table 9.10.8-1. Not currently in use. | 1 |
|  | 15 | .dataFormat | B | 1 | 1 | Signal data packet payload format.  We should always send the data packet format | 2 |
|  | 14 | .gps | N | 0 | 0 | Formatted GPS |  |
|  | 13 | .ins | N | 0 | 0 | Formatted INS |  |
|  | 12 | .ecef | N | 0 | 0 | ECEF ephemeris |  |
|  | 11 | .relativeEphemeris | N | 0 | 0 | Relative ephemeris |  |
|  | 10 | .ephemerisRefId | N | 0 | 0 | Ephermeris reference ID |  |
|  | 9 | .gpsAscii | N | 0 | 0 | GPS ASCII |  |
|  | 8 | .contextAssocLists | N | 0 | 0 | Context association lists |  |
|  | 7 | .cif7enable | N | 0 | 0 | Field attributes enable |  |
|  | 6:5 | .reservedBits6to5 | N | 0 | 0 | Reserved |  |
|  | 4 | .cif4enable | B | 1 | 1 | Extension Fields |  |
|  | 3 | .cif3enable | B | 1 | 1 | Temporal, environmental |  |
|  | 2 | .cif2enable | B | 1 | 1 | Identifiers (tags) |  |
|  | 1 | .cif1enable | B | 1 | 1 | Spatial, Signal spectral, I/O, Ctl |  |
|  | 0 | .reservedBit0 | N | 0 | 0 | Reserved |  |


### .cif1 (Control Indicator Field 1, Spatial, Signal, Spectral, I/O, Ctl) (Mandatory)
**Notes:** See Cotrol-Context Payload Formats

| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Field Size (words) | Notes |
|---|---|---|---|---|---|---|---|---|
| 9 | 31 | .phaseOffset | E | 0 | 0:1 | Phase offset | 1 |  |
|  | 30 | .polarization | B | 1 | 1 | Polarization: Upper bits 31:16 indicate polarization ellipse tilt angle, lower bits 15:0 indicate ellipse eccentricity. Both are measured in radians. Tilt angle is the angle of the polarization-ellipse major axis  measured counter-clockwise from the array/antenna plane positive x-axis [2,4], with the x-axis defined in system documentation. Ellipticity describes the eccentricity of the polarization ellipse, including direction of rotation where appropriate. Linear polarization is described by ellipticity = π/4. For tilt = 0 linear polarization is horizontal, and for tilt = π/2 linear polarization is vertical, thus horzontal polarization is parallel to the antenna x-axis, and vertical polarization parallel to the antenna y-axis, both as defined in system documentation.  Ellipticity = 0 indicates right-handed circular polarization, and ellipticity = π/2 indicates left-handed circular polarization, where left- or right-handed rotation shall be defined  from the point of view of an observer at the receiver looking toward the transmitter [2,4].  Note that the direction in which tilt angle is measured, and the definitions of left- and right-handed rotation are inconsistent with IEEE standards [3]. | 1 |  |
|  | 29 | .pointing3d | B | 1 | 1 | 3-D pointing vector. Indicates pointing beam direction relative to the assigned aperture boresight. 3D pointing has two subfields: Bits 31:16 are elevation, bits 15:0 are azimuth, both in the antenna frame of reference. Define antenna boresight as the z-axis, the x axis is orthogonal to z and "horizontal" in the frame of reference defined by the antenna. The y axis points "up" in the antenna frame and is 90° clockwise from the x axis when looking out along boresight. Elevation is measured up (positive y direction) from the x-z plain to the pointing vector. Azimuth is measured in the x-z plain, between the projection of the pointing vector onto the x-z plain and boresight.  Positive azimuth angles are measured clockwise from boresight when looking down (from positive y toward the x-z plain). | 1 |  |
|  | 28 | .pointing3dStruct | E | 0 | 0 | 3-D pointing vector structure | -1 |  |
|  | 27 | .spatialScanType | N | 0 | 0 | Spatial scan type |  |  |
|  | 26 | .spatialRefType | E | 0 | 0 | Spatial reference type | 1 | Bits 3:2 can be used to define beam pointing frame of reference: 00 => NED, Az/El in NED frame, 01 =>  Not Allowed, 10 => Platform Centered (body frame), 11 => array centered, azimuth and elevation. This field affects the definition of polarization, pointing3D, and beamwidth. |
|  | 25 | .beamWidth | B | 1 | 1 | Beamwidth: Indicate the desired beamwidth. Schedule Request Interpretation specified in Service Contract or AIF. Possible interpretations are: 1) exact value within limits of accuracy 2) maximum value 3) minimum value. Default exact value  (TBR). Context shall be exact value. | 1 |  |
|  | 24 | .range | N | 0 | 0 | Range (distance) |  |  |
|  | 23:21 | .reservedBits23to21 | N | 0 | 0 | Reserved |  |  |
|  | 20 | .ber | N | 0 | 0 | Eb/No BER |  |  |
|  | 19 | .threshold | N | 0 | 0 | Threshold |  |  |
|  | 18 | .compressionPoint | N | 0 | 0 | Compression point |  |  |
|  | 17 | .ip2and3 | N | 0 | 0 | 2nd and 3rd order intercept points |  |  |
|  | 16 | .snr | N | 0 | 0 | SNR/Noise figure |  |  |
|  | 15 | .auxFreq | N | 0 | 0 | Auxiliary frequency | 2 |  |
|  | 14 | .auxGain | N | 0 | 0 | Auxiliary gain | 1 |  |
|  | 13 | .auxBandwidth | N | 0 | 0 | Auxiliary bandwidth | 2 |  |
|  | 12 | .reservedBit12 | N | 0 | 0 | Reserved |  |  |
|  | 11 | .cifArray | N | 0 | 0 | Array of CIFs |  |  |
|  | 10 | .spectrum | E | 0 | 0 | Spectrum | 13 |  |
|  | 9 | .scanStep | E | 0 | 0 | Sector/step-scan array of records. The array header consists of 3 mandatory words: total array size, header/record counts, and a bitmapped record subfield indicator. Records use the selected Sector/Step-Scan subfields. | -1 | Array-of-records structure. Header word 2 contains HeaderSize in bits 31:24, NumWords/Record in bits 23:12, and NumRecords in bits 11:0. Header word 3 uses bit 31 for sector number, bit 30 for F1 start frequency, bit 29 for F2 stop frequency, bit 28 for resolution bandwidth, bit 27 for tune step size, bit 26 for number of points, bit 25 for default gain, bit 24 for threshold, bit 23 for dwell time, bit 22 for start time, bit 21 for time 3, and bit 20 for time 4. Bits 19:0 are reserved. |
|  | 8 | .reservedBit8 | N | 0 | 0 | Reserved |  |  |
|  | 7 | .indexList | N | 0 | 0 | Index list |  |  |
|  | 6 | .discreteIo32 | N | 0 | 0 | Discrete I/O (32 bit) |  |  |
|  | 5 | .discreteIo64 | N | 0 | 0 | Discrete I/O (64 bit) |  |  |
|  | 4 | .healthStatus | N | 0 | 0 | Health status |  |  |
|  | 3 | .v49compliance | N | 0 | 0 | VITA 49 specification compliance - identifies version of VITA 49 spec implemented, 1 => V49.0, 2 => V49.1, 3=> V49A, 4 => V49.2 |  |  |
|  | 2 | .version | N | 0 | 0 | VITA 49 Version and build code - year, day, sub-version, user defined info. |  |  |
|  | 1 | .bufferSize | N | 0 | 0 | Buffer size |  |  |
|  | 0 | .reservedBit0 | N | 0 | 0 | Reserved |  |  |


### .cif2 (Control Indicator Field 2, Identifiers (tags)) (Mandatory)
**Notes:** See Cotrol-Context Payload Formats

| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Field Size (words) |
|---|---|---|---|---|---|---|---|
| 10 | 31 | .bind | N | 0 | 0 | Bind |  |
|  | 30 | .citedSid | E | 0 | 0 | Cited Stream Id - When this control packet is controlling one data stream (either because the activity only has one data stream per direction, or this control packet only refers to one stream in a set of mulitple streams) then the Cited Stream  ID is the SID for that data stream. When  the packet controls mulitple [parallel] data streams the contents of .citedSID may be a 32-bit index pointing at one entry in a list of SIDs for data-stream-sets.  When the .dataAddressStructure field (4/23) has a different list of SID's, .dataAddressStructure shall take precedence. | 1 |
|  | 29 | .sibSid | N | 0 | 0 | Sibling Stream Id |  |
|  | 28 | .parentSid | N | 0 | 0 | Parent Stream Id |  |
|  | 27 | .childSid | N | 0 | 0 | Children Stream Id |  |
|  | 26 | .citedMsgId | E | 0 | 0 | Cited message ID: Include the message ID associated with the control message that requested the sample stream described by this Context Packet | 1 |
|  | 25 | .controlleeId | N | 0 | 0 | Controllee ID |  |
|  | 24 | .controlleeUuid | N | 0 | 0 | Controlee UUID |  |
|  | 23 | .controllerId | N | 0 | 0 | Controller ID |  |
|  | 22 | .controllerUuid | N | 0 | 0 | Controller UUID |  |
|  | 21 | .infoSource | E | 0 | 0 | Information source is a pointer to the Configuration Object or Antenna Information File (AIF) that describes the VA (Virtual Antenna) being controlled by this packet. In order to ensure the Configuration Object index is unique, the most significant 16 bits  (bits 31:16) of this field indicate the MEL, and the least significant 16 bits (bits 15:0) indicate the Config Obj. or AIF index for that MEL. | 1 |
|  | 20 | .trackId | N | 0 | 0 | Track ID |  |
|  | 19 | .countryCode | N | 0 | 0 | Country code |  |
|  | 18 | .operator | N | 0 | 0 | Operator |  |
|  | 17 | .platformClass | N | 0 | 0 | Platform class |  |
|  | 16 | .platformInst | N | 0 | 0 | Platform instance |  |
|  | 15 | .platformDisp | N | 0 | 0 | Platform display |  |
|  | 14 | .emsDeviceClass | N | 0 | 0 | EMS device class |  |
|  | 13 | .emsDeviceType | N | 0 | 0 | EMS device type |  |
|  | 12 | .emsDeviceInst | N | 0 | 0 | EMS device instance |  |
|  | 11 | .modClass | N | 0 | 0 | Modulation class |  |
|  | 10 | .modType | N | 0 | 0 | Modulation type |  |
|  | 9 | .functionId | N | 0 | 0 | Function ID |  |
|  | 8 | .modeId | E | 0 | 0 | Mode ID: User defined, used by communication capability  to control local functions in MFA, e.g., EMCON | 1 |
|  | 7 | .eventId | N | 0 | 0 | Event ID:  This field is used by the Multi-Function Capability  as in the VITA 49.2 specification and is a generic16 bit field. | 1 |
|  | 6 | .funcPriorityId | B | 1 | 1 | Function priority ID for resolving conflicting requests to use MFA resources. Unsigned 32-bit integer, larger values indicate  higher priority. | 1 |
|  | 5 | .commPriorityId | N | 0 | 0 | Communication priority ID for determining access to internal data network when network becomes overloaded. Unsigned 32-bit integer. Bigger is higher priority. |  |
|  | 4 | .rfFootprint | N | 0 | 0 | RF footprint |  |
|  | 3 | .rfFootprintRange | N | 0 | 0 | RF footprint range |  |
|  | 2:0 | .reservedBits2to0 | N | 0 | 0 | Reserved |  |


### .cif3 (Control Indicator Field 3, Temporal, Environmental) (Mandatory)
**Notes:** See Cotrol-Context Payload Formats

| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Field Size (words) |
|---|---|---|---|---|---|---|---|
| 11 | 31 | .tsDetails | N | 0 | 0 | Time stamp details |  |
|  | 30 | .tsSkew | N | 0 | 0 | Time stamp skew |  |
|  | 29:28 | .reservedBits29to28 | N | 0 | 0 | Reserved |  |
|  | 27 | .riseTime | N | 0 | 0 | Rise time |  |
|  | 26 | .fallTime | N | 0 | 0 | Fall time |  |
|  | 25 | .offsetTime | N | 0 | 0 | Offset time |  |
|  | 24 | .pulseWidth | N | 0 | 0 | Pulse width |  |
|  | 23 | .period | N | 0 | 0 | Period |  |
|  | 22 | .duration | N | 0 | 0 | Duration |  |
|  | 21 | .dwell | B | 1 | 1 | Dwell duration in femtoseconds, where Dwell is the duration of a requested transmitter or receiver event, i.e. the transmitter or receiver has exclusive use of an assigned aperture. If Schedule Request type schReqType=1, a dwell shall begin at the Schedule Request TimeStamp and last for Dwell femtoseconds. For receive events all data samples received during a dwell will be sent to the requesting skill. For transmit events the aperture may transmit one or more data bursts during a dwell, with burst durations defined by the data packet time stamp and size, provided all bursts are entirely contained within the dwell. The dwell value ranges from 0 to ~9223 s ( ~154 minutes). | 2 |
|  | 20 | .jitter | E | 0 | 0 | Jitter | 2 |
|  | 19:18 | .reservedBits19to18 | N | 0 | 0 | Reserved |  |
|  | 17 | .age | N | 0 | 0 | Age |  |
|  | 16 | .shelfLife | N | 0 | 0 | Shelf life |  |
|  | 15:8 | .reservedBits15to8 | N | 0 | 0 | Reserved |  |
|  | 7 | .airTemp | N | 0 | 0 | Air temperature |  |
|  | 6 | .seaGroundTemp | N | 0 | 0 | Sea/ground temperature |  |
|  | 5 | .humidity | N | 0 | 0 | Humidity |  |
|  | 4 | .barPressure | N | 0 | 0 | Barometric pressure |  |
|  | 3 | .seaState | N | 0 | 0 | Sea and swell state |  |
|  | 2 | .tropState | N | 0 | 0 | Tropospheric state |  |
|  | 1 | .netId | N | 0 | 0 | Network ID |  |
|  | 0 | .reservedBit0 | N | 0 | 0 | reserved |  |


### .cif4 (Control Indicator Field 4 - Extension Fields) (Mandatory)
**Notes:** See Cotrol-Context Payload Formats

| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Field Size (words) | Notes |
|---|---|---|---|---|---|---|---|---|
| 12 | 31 | .shortTermKey | N | 0 | 0 | This is the TRANSEC short term key for the burst | 8 |  |
|  | 30 | .reservedBit30 | N | 0 | 0 | Reserved for future cif extensions |  |  |
|  | 29 | .rfFigureOfMerit | B | 1 | 1 | Bits 31:15 unused, set to 0. Bits 15:0 contain Input G/T (dB/K) to be used for Rx, or Target EIRP (dBm) to be used for Tx.  See Schedule Request for description of rfFigureOfMerit there. | 1 |  |
|  | 28 | .earlyStartTime | E | 0 | 0:1 | Earliest allowed start time for flexible schedule request, offset from time stamp | 2 |  |
|  | 27 | .lateStartTime | E | 0 | 0:1 | Latest allowed start time for flexible schedule request, offset from time stamp | 2 |  |
|  | 26 | .rejectReason | E | 0 | 0:1 | 32 bit enumeration of reasons that this control was rejected by the scheduler. See Reject Reason Enumeration tab. | 1 |  |
|  | 25 | .maxDataPacketDwell | E | 0 | 0:1 | fractional time format giving maximum Rx time interval between data packets. Can be used to reduce data delay during long Rx dwells | 2 |  |
|  | 24 | .addressGroupIndex | B | 1 | 1 | Provides a look-up index that points to a collection of MFP address for data and Ack packets associated with this Schedule Request. That is one or more data packet stream(s) with SID matching the Control SID, or  SID's identified by the SID or index in the field (2/30) .citedSID. Plus AckR and AckX packet streams with SID matching the Control SID.  The corresponding MFA addresses are in the same field (CIF 4, Bit 24) of the corresponding AckR packet. | 1 | AGI points to a table with variable number of entries. Minimum entry is a default address. Every packet sent to the MFA for which an address has not been specified will be sent to the default address. Other possible values are Data Address Index (DAI), and Control Address Index (CAI). |
|  | 23 | .dataAddressStructure | E | 0 | 0 | An array of records listing all the data streams associated with this control packet stream by data stream SID and MFP address. When present it may replace the information in (2/30) .citedSID and always replaces the information in (4/24) dataAddressIndex. The corresponding MFA addresses are contained in the same field location (4/23) of the AckR packet returned in response to this packet. This array provides greater flexibility than using .citedSId and .dataAddressIndex at the cost of variable-length control packets. The array header consists of the 3 mandatory header words (see reference or Change Notes) plus a variable number of records consisting of 1 or 2 words (SID, MFP Address Index) or  (MFP Address Index) as indicated in header word 3. | -1 |  |
|  | 22 | .dataAddressTime | E | 0 | 0 | If the data address(es) of 4/24 or 4/23 are not immediately applicable (e.g., If the MFA must switch antenna faces during a stream of VITA data packets transporting one long Rx/Tx packet, the MFA scheduler may send a new AckR packet with the new MFA port addresses with some lead time before the change), The .dataAddressTime parameter indicates when the data address information in 4/24 or 4/23 becomes applicable. Offset from time stamp. | 2 |  |
|  | 21 | .txDigitalInputPower | E | 0 | 0:1 | Nominal power level of digital TX signal input to MFA for transmission, dBfs, where "full scale power" shall be defined as the power of a complex digital sinusoid with amplitude equal to full scale. | 1 |  |
|  | 20:0 | .reservedBits20to0 | N | 0 | 0 | Reserved for future cif extensions |  |  |


### .cif7 (Control Indicator Field 7,  Attributes) (Forbidden)
**Notes:** See Cotrol-Context Payload Formats

| Word | Bit | Designation | Bit Used | Default Value | Range | Function |
|---|---|---|---|---|---|---|
| N/A | 31 | .curVal | N | 0 |  | Current value |
|  | 30 | .avgVal | N | 0 |  | Average value |
|  | 29 | .medVal | N | 0 |  | Median value |
|  | 28 | .std | N | 0 |  | Standard deviation |
|  | 27 | .maxVal | N | 0 |  | Maximum value |
|  | 26 | .minVal | N | 0 |  | Minimum value |
|  | 25 | .precision | N | 0 |  | Precision |
|  | 24 | .accuracy | N | 0 |  | Accuracy |
|  | 23 | .velocity | N | 0 |  | 1st Derivative (velocity) |
|  | 22 | .acceleration | N | 0 |  | 2nd Derivative (acceleration) |
|  | 21 | .jerk | N | 0 |  | 3rd Derivative (jerk) |
|  | 20 | .probability | N | 0 |  | Probability |
|  | 19 | .belief | N | 0 |  | Belief |
|  | 18:0 | .reservedBits18to0 | N | 0 | 0 | Reserved |


### .payloadFields (Mandatory)
**Notes:** Payload fields are defined by enabled CIF indicators and the Control-Context Payload Formats table. Payload fields are included in CIF-bit order and shall match the size and format of corresponding payload fields in the Extension Control Packet. This group is a metadata placeholder and has no fixed rows.



## Data Packet

Control-Cancellation Packet Template

### .header (Mandatory)
| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Time Determined | Time When Payload Value Determined |
|---|---|---|---|---|---|---|---|---|
| 1 | 31:28 | .type | B | 0001 | 0001 | Packet Type: 0001 => Signal Data Packet with Stream Identifier; Type 0000 =>  Signal Data Packet without Stream Identifier | Fixed | N/A |
|  | 27 | .cBit | B | 1 | 1 | Indicates whether CLASS ID is included in the prologue | Fixed | N/A |
|  | 26 | .tBit | B | 1 | 1 | Indicates whether this packet has a trailer word | Fixed | N/A |
|  | 25 | .notV49p0Packet | E | 0 | 0 | Indicates whether this is a VITA 49.0 compliant packet | N/A | N/A |
|  | 24 | .sBit | E | 0 | 0 | Indicates whether packet contains spectrum data | Fixed | N/A |
|  | 23:22 | .tsi | B | 11 | 11 | Indicates: 00 No int seconds time stamp, 01 => UTC, 10 => GPS, 11 => Other | Fixed per VA Definition | N/A |
|  | 21:20 | .tsf | B | 10 | 10 | Indicates: 00 => No frac seconds field, 01 => sample count, 10 => picoseconds, 11 => free running count | Fixed per VA Definition | N/A |
|  | 19:16 | .packetCount | B | 0 | 0:15 | 4-Bit Packet sequence number for this packet type in this streamID | Dynamic | N/A |
|  | 15:0 | .packetSize | B | N/A | 0:2^16-1 | Total number of 32-bit words in the packet including the prologue; computed from the final packet word count. | Fixed per VA Definition | N/A |


### .streamId (Stream Identifier) (Mandatory)
| Word | Bit | Designation | Bit Used | Default Value | Function | Time Determined | Time When Payload Value Determined |
|---|---|---|---|---|---|---|---|
| 2 | 31:0 | .sid | B | 1 | Stream ID provided in associated AckR packets used to schedule access. | VA Initialization | N/A |


### .classId (Class Identifier) (Mandatory)
| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Time Determined | Time When Payload Value Determined |
|---|---|---|---|---|---|---|---|---|
| 3 | 31:27 | .padBitCount | N | 0 | 0:2^5-1 | Number of pad bits (0…31) at the end of the packet to hit a 32-bit word boundary. Not used by Base Set.  16-bit I/Q samples always fill each word. | Fixed per VA Definition | N/A |
|  | 26:24 | .reservedBits26to24 | N | 0 | 0 | Reserved | N/A | N/A |
|  | 23:0 | .oui | B | 0xAAAAAA | 0xAAAAAA | Organizationally Unique Identifier (OUI) | Fixed per VA Definition | N/A |
| 4 | 31:24 | .infoClassCode_Type | B | 0x04 | 4:5 | 0 => Unknown; 1 => TxComm, 2 => RxComm, 3 => Sensor;  4 => TxComm base set; 5 => RxComm Base Set; 6 => TxComm extended set 1; 7 => RxComm extended set 1; 8 => TxComm  extended set 2; 9 => RxComm extended set 2; (10-255) => TBD | Fixed per VA Definition | N/A |
|  | 23:16 | .infoClassCode_Version | B | 0x00 | 0:2^8-1 | Version number of information class | Fixed per VA Definition | N/A |
|  | 15:8 | .packetClassCode_Type | B | 0x0D | 0x0D | Defined PacketTypes:  0x00 => undefined, 0x0A => Base Set Control ScheduleRequest, 0x0B => Base Set Schedule Acknowledge (AckR),  0x0C => Base Set Execution Acknowledge (AckX), 0x0D => Base Set Data, 0x0E => Base Set DataContext |  |  |
|  | 7:0 | .packetClassCode_Version | B | 0x05 | 0:2^8-1 | Version of Packet Class definition for specified type |  |  |


### .timeStamp (Time Stamp) (Mandatory)
| Word | Bit | Designation | Bit Used | Default Value | Range | Function | Time Determined | Time When Payload Value Determined |
|---|---|---|---|---|---|---|---|---|
| 5 | 31:0 | .tsi | B | 1 | 0:2^32-1 | UTC seconds since midnight, Jan. 1, 2019 | Dynamic within per VA limits | N/A |
| 6 | 31:0 | .tsf | B | 0 | 0:2^32-1 | Most-significant (Upper 32 bits) pico seconds since last seconds update | Dynamic within per VA limits | N/A |
| 7 | 31:0 | .tsfLower | B | 0 | 0:2^32-1 | Least-significant (Lower 32 bits) pico seconds since last seconds update | Dynamic within per VA limits | N/A |


### .dataPayload (Signal Data Payload Words) (Mandatory)
**Notes:** Signal data payload is variable-length and encoded according to .dataFormat. See Payload Formats for specific data format information. This group is a metadata placeholder and has no fixed per-word rows.



### .trailer (Mandatory)
| Word | Bit | Designation | Bit Used | Default Value | Range | Function |
|---|---|---|---|---|---|---|
| N+1 | 31 | .calibratedTimeIndicatorEnable | N | 0 | 0 | Calibrated-time trailer reporting is not used in the AMS Base Set; set to zero |
|  | 30 | .validDataIndicatorEnable | N | 0 | 0 | Valid-data trailer reporting is not used in the AMS Base Set; set to zero |
|  | 29 | .referenceLockIndicatorEnable | E | 0 | 0 | Enables whether bit 17 indicates a valid status |
|  | 28 | .agcIndicatorEnable | E | 0 | 0 | Enables whether bit 16 indicates a valid status |
|  | 27 | .detectedSignalIndicatorEnable | E | 0 | 0 | Enables whether bit 15 indicates a valid status |
|  | 26 | .spectralInversionIndicatorEnable | N | 0 | 0 | Spectral inversion trailer reporting is not used in the AMS Base Set; set to zero |
|  | 25 | .overRangeIndicatorEnable | E | 0 | 0 | Enables whether bit 13 indicates a valid status |
|  | 24 | .sampleLossIndicatorEnable | E | 0 | 0 | Enables whether bit 12 indicates a valid status |
|  | 23:22 | .sampleFrameIndicatorEnables | B | 11 | 0:3 | Enables whether bits 11:10 below (.sampleFrameIndicators) contain valid status bits. Bits 23:22 => 11 then Bits 11:10 indicate the packet's role in transporting a large (multi-packet) data frame, where bits 23:22 => 11 and bits 11:10 => 00 indicates a single packet caries the entire data frame. Bits 23:22 => any value but 11 then 11:10 are not valid indicators. |
|  | 21:20 | .userDefinedIndicatorEnables | N | 0 | 0 | 1 => Enables user-defined bits 9:8 (.userDefinedIndicators) to indicate user defined  status. |
|  | 19 | .calibratedTimeIndicator | N | 0 | 0 | Calibrated-time trailer reporting is not used in the AMS Base Set; set to zero |
|  | 18 | .validDataIndicator | N | 0 | 0 | Valid-data trailer reporting is not used in the AMS Base Set; set to zero |
|  | 17 | .referenceLockIndicator | E | 0 | 0 | 1 =>(and bit 29 => 1), shall indicate that any phase-locked loops affecting the Data are locked and stable. 0 =>  (and bit 29 => 1) indicates at least one PLL is not locked and stable. |
|  | 16 | .agcIndicator | E | 0 | 0 | 1 => (and bit 28 => 1),  indicates that AGC is active.  0 => (and bit 28 => 1),  indicates MGC. |
|  | 15 | .detectedSignalIndicator | E | 0 | 0 | 1 => (and bit 27 => 1), indicates the data contained in the packet contains some detected signal. 0 =>  (and bit 27 => 1), indicates the packet does not contain some detected signal. |
|  | 14 | .spectralInversionIndicator | N | 0 | 0 | 1 => (and bit 26 => 1),indicates  the signal conveyed in the data payload has an inverted spectrum with respect to the spectrum of the signal at the system Reference Point. |
|  | 13 | .overRangeIndicator | E | 0 | 0 | 1 => (and bit 25 => 1),indicates that at least one Data Sample in the payload is invalid due to the signal exceeding the range of the Data Item. |
|  | 12 | .sampleLossIndicator | E | 0 | 0 | 1 => (and bit 24 => 1), indicates the packet contains at least one sample discontinuity due to processing errors and/or buffer overflow. |
|  | 11:10 | .sampleFrameIndicators | B | 00 | 00, 01, <br> 10, 11 | When bits 23:22 => '11' then bits 11:10 indicate this data packet's position in a data frame spanning mulitple data packets. Bits 11:10 = '00' => sample frame is one data packet, '01' => First data packet of a sample frame, '10' => Middle data packet of a sample frame, '11' => Final data packet of a sample frame. If a data stream is preempted in the middle of a data frame spanning mulitple data packets, bits 11:10 in the last data packet sent shall be '11'. |
|  | 9:8 | .userDefinedIndicators | N | 0 | 0 | 00, 01, 10, 11 are user defined status states of the data packet. |
|  | 7 | .eBit | N | 0 | 0 | Associated Context Packet Count Enable. When the E bit is set to 1, the Associated Context Packet Count shall provide a count of all of transmitted Context packets that are directly or indirectly associated with the Signal Data packet, OR a count of some special subset of these. When the E bit is cleared, the Associated Context Packet Count is undefined. |
|  | 6:0 | .assocContextPacketCount | N | 0 | 0 | When used, the 7-bit Associated Context Packet Count field shall contain an unsigned number in the range of zero to 127 inclusive. The lsb of the number shall be the right-most bit in the field. |


## Control-Context Payload Formats

### .payloadFormats
**Notes:** Supported data formats are exactly 16-bit and 8-bit I/Q signed integers; 1-bit real unsigned and floating-point formats are not supported by the Base Set.

| Field Bit | Designation | Bits Used | Range | Function | Field Size (words) | Notes | CIF/Bit |
|---|---|---|---|---|---|---|---|
| 63:0 | .bandwidth |  |  | CIF0/29 3 dB Bandwidth in Hz.  The Bandwidth field shall use the 64-bit, two’s-complement format.  This field has an integer and a fractional part with the radix point to the right of bit 20 in the second 32-bit word. | 2 |  | 0/29 |
| 63:0 | .ifRefFreq |  |  | CIF0/28 IF reference frequency in Hz.  The IF reference frequency field shall use the 64-bit, two’s-complement format.  This field has an integer and a fractional part with the radix point to the right of bit 20 in the second 32-bit word.  For real data samples it is in the range 0  to Fs/2, for complex data samples it is in the range -Fs/2 to +Fs/2. | 2 |  |  |
| 63:0 | .rfRefFreq |  |  | CIF0/27 RF reference frequency in Hz.  The RF reference frequency field shall use the 64-bit, two’s-complement format.  This field has an integer and a fractional part with the radix point to the right of bit 20 in the second 32-bit word. | 2 |  | 0/27 |
| 63:0 | .rfRefFreqOffset |  |  | CIF0/26 RF Frequency of Interest offset from RF Reference Frequency in Hz. The RF Reference Frequency Offset field shall use the 64-bit, two’s-complement format.  This field has an integer and a fractional part with the radix point to the right of bit 20 in the second 32-bit word. | 2 |  |  |
| 31:0 | .refLevel |  |  | CIF0/24  The upper 16 bits of this field are reserved and shall be set to zero. The Reference Level value shall be expressed in two’s-complement format in the lower 16 bits of this field. This field has an integer and a fractional part, with the radix point to the right of bit 7. | 1 |  |  |
| 63:0 | .ifBandOffset |  |  | CIF0/25 Offset of IF band center from ifRefFreq in Hz. The RF reference frequency field shall use the 64-bit, two’s-complement format.  This field has an integer and a fractional part with the radix point to the right of bit 20 in the second 32-bit word. Value may be postivie or negative. | 2 |  |  |
| 31:16 | .reservedBits31to16 |  |  | CIF0/24 |  |  |  |
| 15:0 | .refLevelStage1 |  |  | CIF0/24 Reference Level, units dBm. Reference Level value shall be expressed in two’s-complement format in the lower 16 bits of the Reference Level field. This subfield has an integer and a fractional part, with the radix point to the right of bit 7 of the subfield. Referemce Level relates digital signal amplitude to analog signal power. An analog Reference Point and location for the digital Described Signal must be defined in system documentation. The Reference Level is the AC power of a single analog sine wave at the reference point that produces a unit-scale digital sine wave as the Described Signal. Note that a unit scale sine wave for N-bit two's complement data is defined as one with amplitude ranging from -2^(N-1) to +2^(N-1), which exceeds the available range of an N-bit two's complement number at its peak value. | 1 |  |  |
| 31:16 | .gainStage2 |  |  | CIF0/23 This field is unused in our implementation | 1 |  |  |
| 15:0 | .gainStage1 |  |  | CIF0/23 The Stage 1 Gain subfield shall be expressed in units of decibels (dB). The Stage 1 Gain value shall be expressed in two’s-complement format in the lower 16 bits of the Gain field. This subfield has an integer and a fractional part, with the radix point to the right of bit 7 of the subfield. Gain is defined as the ratio of Described Signal power (output) to signal power at the Reference Point (input), in dB. The Reference Point and Described Signal location must be defined in system documentation. Typical values are the antenna feed and ADC/DAC analog input/output. Gain may apply to pairs of analog signals, or pairs of digital signals, but not to mixed-format signal pairs. If digital signals have different digital resolution at the Refence Point and Described Signal, signal power at each location is normalized by the power of a full-scale sinusoid before calculating Gain. | 1 |  |  |
| 31:16 | .gain |  |  | Bits 31:16 (Sometimes called Stage 2 gain, or back-end gain) is not used in the Base Set and shall be set to all zeros. When used in an extension set it is measured in dB. This subfield has an integer and a fractional part, with the radix point to the right of bit 7 of the subfield. It's value ranges from -256 to +255 in 1/128 dB steps. | 1 |  | 0/23 |
| 15:0 | .gainStage1_BaseSet |  |  | Bits 15:0 (Sometimes called Stage 1 gain, or front-end gain, or total gain) is present in  base Set Schedule Requests and Context Packets. It is measured in dB.  This subfield has an integer and a fractional part, with the radix point to the right of bit 7 of the subfield. It's value ranges from -256 to +255 in 1/128 dB steps. This sub-field is used in Schedule Requests and Context Packets for Rx data.When not used, e.g. Tx data or Rx data for systems that do not use gain to control signal amplitude into the ADC, it shall be set to all zeros. |  |  |  |
| 63:0 | .sampleRate |  |  | CIF0/21 The value of the Sample Rate field shall be expressed in units of Hertz. The Sample Rate field shall use the 64-bit, two’s-complement format shown in Figure 9.5.12-1. This field has an integer and a fractional part, with the radix point to the right of bit 20 in the second 32-bit word. | 2 |  | 0/21 |
| 31:20 | .seiEnables |  |  | CIF0/16 State and Event Indicator Field.  We will use the user-defined bits as follows: 7:2 => 000000; bit 1: 0 => this slot is not the first slot in the slot set, 1 => this slot is the first slot in the slot set; bit 0:  0 => this is not the last slot in the slot set, 1 => this is the last slot in the slot set.  Note, if there is only one slot in the slot set, then both bits will be set to 1.  The middle slots in the slot set will be indicated by 00 | 1 |  |  |
| 19:8 | .seiIndicators |  |  |  | 1 |  |  |
| 7:0 | .seiUserDefined |  |  |  | 1 |  |  |
| 31:0 | .sei |  |  | State and Event Indicator. Eight predefined indicator bits with corresponding enable bits, plus 8 user-defined bits. Bits 28 and 16 indicate whether AGC is in use. See [2] Table 9.10.8-1. Not currently in use. | 1 |  | 0/16 |
| 63:0 | .dataFormat |  |  | CIF0/15 Specifies the packing and content of the samples of the paired Data Packet Stream.  See below for more details for subfields of this field. | 2 |  | 0/15 |
| 31:16 | .polarizationTilt |  |  | CIF 1/30,  16-bit 2's complement number with radix to the right of bit 29,  in radians. Antenna polarization  tilt angle, θ, on the Poincaré sphere. For θ=0 the polariztion ellipse major axis is horizontal, for θ=π/2 the polarization ellipse major axis is vertical. Note that the direction in which θ is measured, defined in [2,4], is not consistent with IEEE standards [3]. | 1 |  | 1/30 |
| 15:0 | .polarizationEccentricity |  |  | CIF 1/30 16-bit 2's complement number with radix to the right of bit 13, in radians. Antenna polarization eccentricity angle, χ, on the Poincaré sphere.  When χ=0 the signal is right-hand circularly polarized, χ=π/4 indicates linear polarization, and χ=π/2 indicates left-hand curcular polarization, as defined in [2,4]. Note that ellipticity in [2,4] is defined  from the point of view of an observer at the receiver looking toward the transmitter, which makes the definitions of left- and right- handedness the reverse of IEEE standards [3]. | 1 |  |  |
| 31:16 | .pointing3dElevation |  |  | CIF1/29 Bits 31-16 - 16-bit 2's complement number with radix to the right of bit 23. It ranges from -90 to +90 degrees with minimum step size 1/128 degree. Define antenna boresight as the z-axis, the x axis is orthogonal to z and "horizontal" in the frame of reference defined by the antenna. The y axis points "up" in the antenna frame and is 90° clockwise from the x axis when looking out along boresight. Elevation is measured up (positive y direction) from the x-z plain to the pointing vector. | 1 |  | 1/29 |
| 15:0 | .pointing3dAzimuth |  |  | CIF1/29 bits 15:0 -  16-bit unsigned fixed-point number with radix to the right of bit 7, degrees. It ranges from 0 to 511.9921875 degrees with minimum step size 1/128 degree. Define antenna boresight as the z-axis, the x axis is orthogonal to z and "horizontal" in the frame of reference defined by the antenna. The y axis points "up" in the antenna frame and is 90° clockwise from the x axis when looking out along boresight.  Azimuth is measured in the x-z plain, between the projection of the pointing vector onto the x-z plain and boresight.  Positive azimuth angles are measured clockwise from boresight when looking down (from positive y toward the x-z plain). | 1 |  |  |
| 31:16 | .spatialRefType |  |  | User Defined Spatial Identifier - not used | 1 |  | 1/26 |
| 15:4 | .reserved15to4 |  |  | Reserved |  |  |  |
| 3:2 | .frameOfRef |  |  | Frame of Reference: 00 => NED, El above North-East plane, Az clockwise from true North when looking down, 01 => ECEF in VITA spec, not allowed by VITA 49.2, 10 => Platform Centered, elevation above Longitudinal-Transverse plane, azimuth clockwise from longitudeinal vector when looking down, 11 => Array Centered, elevation up from horizontal plane, azimuth in horizontal plain, clockwise from biresight when looking down. Not used in Base Set. |  |  |  |
| 1:0 | .beamType |  |  | Beam Type - not used |  |  |  |
| 31:16 | .beamWidthElevation |  |  | CIF1/25 16-bit 2's complement with radix to the right of bit 23, horizontal beam width in degrees.The frame of Reference is aligned with the beam pointing frame of reference, i.e. horizontal beamwidth is measured parallel to the plane in which beam-pointing azimuth is measured and orthogonal to the beam pointing direction. | 1 |  | 1/25 |
| 15:0 | .beamWidthAzimuth |  |  | CIF1/25 16-bit 2's complement with radix to the right of bit 7, vertical beam width in degrees. With vertical   beamwidth measured orthogonal to horizontal beamwidth and orthogonal to the beam pointing direction. | 1 |  |  |
| 31:0 | .citedSid |  |  | CIF2/26 32-bit number with the stream ID of the associated data packet. | 1 |  | 2/26 |
| 31:16 | .modeIdCurrent |  |  | CIF2/8 Reserved part of the Generic16 bit field. | 1 |  | 2/8 |
| 15:0 | .modeIdNext |  |  | CIF2/8 Mode ID: Used by Multi-Function Capability to indicate possible modes to the MFA. |  |  |  |
| 31:0 | .eventId |  |  | CIF2/7 The field is used by the Multi-Function Capability  as in the VITA 49.2 specification and is a generic16 bit field. | 1 |  |  |
| 31:0 | .funcPriorityId |  |  | CIF2/6 The Function Priority ID field shall convey the priority of an operation within a Command Packet via a Generic16 bit Identifier field. The field is interpreted as an unsigned integer with higher values indicating higher priority. | 1 |  | 2/6 |
| 63:0 | .dwell |  |  | CIF3/21 Dwell shall use the Fractional Time format of Rule 9.7-1 (not the same as a fractional timestamp) It shall be a 64-bit two’s complement integer occupying two consecutive 32-bit words in the packet as shown in Figure 9.7-3. The most significant 32 bits shall be in the first of these two words. The LSB of the Dwell Fractional Time field shall be 1 femtosecond. Dwell value may range from 0 to ~9223 s (~154 minutes). | 2 |  | 3/21 |
| 63:0 | .jitter |  |  | CIF 3/20 Maximum magnitude of time error for time fields (other than timestamp) in Control Packet, expressed in Fractional Time Format (64-bit two's complement integer expressing time interval in femtoseconds) | 2 |  |  |
| 255:0 | .shortTermKey |  | 8 | 8  32 bit integers with transec key. | 8 |  |  |
| 31:16 | .rfFigureOfMeritLowerBound |  |  | rfFigureOfMerit_LowerBound, subfield 31:16, two’s-complement format in the upper 16 bits of the field. This subfield has an integer and a fractional part with the radix point to the right of bit 23 of the subfield. It's value ranges from -256 to +255 in 1/128 dB or dB/K steps as appropriate. In Schedule Request shall indicate the minimum acceptable EIRP on transmit in dBm, or G/T on receive in dB/K.   If the MFA cannot support service with EIRP or G/T at or above the Lower Bound, a schedule request shall be rejected. In Context or AckR this sub-field is not used and shall be set to 0. | 1 |  | 4/29 |
| 15:0 | .rfFigureOfMeritRequested |  |  | rfFigureOfMerit_Requested subfield 15:0,  two’s-complement format in the lower 16 bits of the field. This subfield has an integer and a fractional part, with the radix point to the right of bit 7 of the subfield. It's value ranges from -256 to +255 in 1/128 dB or dB/K steps as appropriate. In Schedule Request shall indicate requested EIRP on transmit in dBm, or G/T on receive in dB/K. The MFA shall support the requested Tx service with EIRP as close as possible to the requested value without exceeding it. The MFA shall support the requested Rx service with G/T as close below the requested value as possible, and may support service with G/T above the requested value. In Context or AckR (if used) shall indicate value used (Context) or to be used (AckR). | 1 |  |  |
| 63:0 | .earlyStartTime |  |  | CIF 4/28 this field shall use the Fractional Time format of Rule 9.7-1 (not the same as a fractional timestamp) It shall be a 64-bit two’s complement integer occupying two consecutive 32-bit words in the packet as shown in Figure 9.7-3. The most significant 32 bits shall be in the first of these two words. The LSB of the Early Start Time Fractional Time field shall be 1 femtosecond. | 2 |  | 4/28 |
| 63:0 | .lateStartTime |  |  | CIF 4/27 this field shall use the Fractional Time format of Rule 9.7-1 (not the same as a fractional timestamp) It shall be a 64-bit two’s complement integer occupying two consecutive 32-bit words in the packet as shown in Figure 9.7-3. The most significant 32 bits shall be in the first of these two words. The LSB of the Late Start Time Fractional Time field shall be 1 femtosecond. | 2 |  | 4/27 |
| 31:0 | .rejectReason |  |  | 32 bit unsigned integer enumeration of reason that control was rejected. See Reject Reason Enumeration Tab. | 1 |  | 4/26 |
| 63:0 | .maxDataPacketDwell |  |  | CIF 4/25 MaxDataPacketDwell shall use the Fractional Time format of Rule 9.7-1 (not the same as a fractional timestamp) It shall be a 64-bit two’s complement integer occupying two consecutive 32-bit words in the packet as shown in Figure 9.7-3. The most significant 32 bits shall be in the first of these two words. The lsb of each word shall be on the right. The LSB of the Dwell Fractional Time field shall be 1 femtosecond. | 2 |  | 4/25 |
| 31:0 | .addressGroupIndex |  |  | CIF 4/24 32-bit unsigned integer, Provides a look-up index that points to a collection of MFP or MFA address for data, control, and Ack packets associated with this Schedule Request. That is one or more data packet stream(s) with SID matching the Control SID, or  SID's identified by the SID or index in the field (2/30) .citedSID. Plus Control, AckR and AckX packet streams with SID matching the Control SID.  The corresponding MFA addresses are in the same field (CIF 4, Bit 24) of the corresponding AckR packet. of one or more associated data streams. | 1 |  | 4/24 |
| 32*(3+N)-1 or 32*(3+2N)-1 depending    on structure implementation | .dataAddressStructure |  |  | Array of records with structure: | -1 | Word 1 is the total size of the array in 32-bit words, equal to 3+2N when SIDs are provided or 3+N when SIDs are not provided, where N is the number of data streams. Word 2 contains Header Size (8 bits) = 0, NumWords/Record (12 bits) = 1 or 2, and NumRecords (12 bits) = N. Word 3 indicates whether SIDs are provided or obtained using the index in .citedSID; bit 31 may be 0 or 1, bit 30 = 1 indicates mandatory MFP address indices are present, and bits 29:0 = 0. If word 3 bit 31 = 0, the remainder is N words containing 32-bit unsigned integers. If word 3 bit 31 = 1, the remainder is 2N words containing 32-bit unsigned integers. |  |
| 63:0 | .dataAddressTime |  |  | fractional time format (64-bit 2's complement number). | 2 |  | 4/22 |
| 31:16 | .reservedBits31to16_4_21 |  |  |  |  |  | 4/21 |
| 15:0 | .txDigitalInputPower |  |  | CIF 4/21 subfield 15:0 shall indicate nominal power level of digital TX signal input to the MFA for transmission, in units of dBfs, where "full scale power" shall be defined as the power of a complex digital sinusoid with amplitude equal to full scale. This parameter shall be expressed in two’s-complement format in the lower 16 bits of the field. This subfield has an integer and a fractional part, with the radix point to the right of bit 7 of the subfield. It's value ranges from -256 to +255 in 1/128 dBfs. | 1 |  | 4/21 |
| 31 | .dataFormat.packBit | 1 |  | 0 => Processing Efficient Packing, 1 => Link efficient packing | 1 |  | 1 |
| 30:29 | .dataFormat.realOrComplex | 01 |  | 00 => Real, 01 => Complex Cartesian, 10 => Complex Polar, 11 => Reserved | 1 |  |  |
| 28:24 | .dataFormat.dataItemFormat | 00000 |  | 00000 => Data are signed fixed-point integers using the two's complement format. | 1 |  |  |
| 23 | .dataFormat.repeat | 0 |  | 0 => Sample Component repeating is not in use, 1 => Sample Component repeating is in use. | 1 |  |  |
| 22:20 | .dataFormat.eventTagSize | 000 |  | Shall contain an unsigned number equal to the Event-Tag size used in the paired Data Packet Stream. | 1 |  |  |
| 19:16 | .dataFormat.channelTagSize | 0000 |  | Shall contain an unsigned number equal to the Channel-Tag size used in the paired Data Packet Stream. | 1 |  |  |
| 15:12 | .dataFormat.dataItemFracSize | 0000 |  | Shall apply specifically to the VRT Signed/Unsigned Fixed-Point Non-Normalized numeric formats. 0000 => for all other numeric formats, maintaining compatibility with V49.0. Shall express the number of bits in the fraction of a non-normalized number of the format “integer.fraction” where the total size of the number is given by the “Data Item Size” field.  The “Data Item Fraction Size” field shall not be greater than the Data Item Size – 1. | 1 |  |  |
| 11:6 | .dataFormat.itemPackingFieldSize | 001111 |  | Shall contain an unsigned number that is one less than the actual Item Packing Field size used in the paired Data Packet Stream. | 1 |  |  |
| 5:0 | .dataFormat.dataItemSize | 001111 |  | Shall contain an unsigned number that is one less than the actual Data Item size in the paired Data Packet Stream. | 1 |  |  |
| 31:16 | .dataFormat.repeatCount | 0x0000 |  | Not used | 1 |  | 2 |
| 15:0 | .dataFormat.vectorSize | 0x0000 |  | Not used | 1 |  |  |


## Reject Reason Enumeration

Reject Reason Enumeration

The Reject Reason Field  in AckR messages (CIF 4/26) indicates why a schedule request was rejected by the VAS. The first two sub-fields indicate the CIF (bits 31:29) and the CIF bit value (bits 28:24) of the parameter whose value resulted in rejection. If rejection is not tied to a specific parameter (e.g. Malformed Packet) the first 8 bits take on value '11100000'.

Bits 23:0 of the Reject Reason Field is an index into the Reject Reason Enumeration. The RREI index value is obtained by interpreting bit field as a  24-bit unsigned integer, with bit 23 as the MSB and bit 0 as the LSB. As indicated in the table below, higher index values are reserved for use by the MFA integrator to define MFA-specific values.

Baseline Reject Reason Field Contents Definition

| Bit Number | Error Bit Flag or Sub-Field | Notes |
|---|---|---|
| 31:29 | Schedule Request Payload Field Causing Rejection, CIF # | If rejection is not tied to a specific Schedule Request Payload Field, the CIF # (bits 31:29 and CIF Bit # (bits 28:24) fields shall be set to 0xE0 ('11100000') |
| 28:24 | Schedule Request Payload Field Causing Rejection, Bit # in CIF |  |
| 23:0 | Reject Reason Enumeration Index (RREI) |  |
| 0 | Not Applicable | Request was not rejected |
| 1 | Malformed Packet | Unable to parse packet, or important elements missing or invalid |
| 2 | Bad Stream ID | Stream ID exsits but is not a valid value |
| 3 | Resource does not exist |  |
| 4 | Resource not working |  |
| 5 | Resource too busy |  |
| 6 | Resource conflict | Schedule conflicts with higher priority user detected before scheduling, other than previous |
| 7 | Previously scheduled event pre-empted | Schedule conflict with higher priority user developed after scheduleing |
| 8 | Insufficient lead time, or too late | Execution time too close to command time |
| 9 | Excessive lead time | Exectution time too far from command time |
| 10 | Bad Time Stamp | Time stamp exists but is in error for reasons other than preceeding |
| 11 | Command buffer full |  |
| 12 | Data buffer full |  |
| 13 | Insufficient network capacity |  |
| 14 | Retry requested | Condition that prevents scheduling is expected to end soon |
| 15 | Parameter missing | Required parameter is not present in packet |
| 16 | Parameter out of range | Parameter is present in packet but is not an allowed value |
| 17 | Pointing vector outside FOR | Pointing Vector is correctly formatted and within allowed range for antenna, but antenna cannot point in requested direction, e.g. blockage |
| 18 | EIRP outside allowed range at pointing angle | Requested EIRP is inside overall range, but cannot be met at the requested pointing angle |
| 19 | Parameter combination not supported | Althoug all individual parameters are inside their allowed ranges, this combination of parameter values is not supported |
| 20:8388607 | TBD: Working Group | 20:(2^23-1) |
| 8388608:16777215 | TBD: MFA Integrator | (2^23):(2^24-1) |


## Error Payload Fields

Error Payload Fields

An error condition occurs when the MFA is not able to perform an activity scheduled by the VAS. The Error Payload Field  in AckX messages indicates the cause of an Error Condition, usually with reference to a particular Payload Field in the related Schedule Request.

Ref [2] defines a standard 32-bit Error Payload Field  used for every warning or error report. Bits 31:19 in the VITA 49.2 Error Payload Field are flags indicating particular errors [2]. In [2] bits 17:0 are Reserved or User Defined. In this Baseline packet packet standard bits 17:0 have been redefined as shown below.

Baseline Error Payload Field Contents Definition

| Bit Number | Error Bit Flag or Sub-Field | Notes |
|---|---|---|
| 31 | Field Not Executed |  |
| 30 | Device Failure |  |
| 29 | Erroneous Field (field not settable on this device) |  |
| 28 | Parameter Out-Of-Range |  |
| 27 | Parameter Precision Not Supported |  |
| 26 | Field Value Invalid (other than previous errors) |  |
| 25 | Timestamp Problem |  |
| 24 | Will Create Hazardous Power Levels |  |
| 23 | Will Cause Distortion |  |
| 22 | In-Band Power Out of Compliance |  |
| 21 | Out-of-Band Power Out of Compliance |  |
| 20 | Will Cause Co-Site Interference |  |
| 19 | Will Cause Regional Interference |  |
| 18 | Reserved |  |
| 17:15 | Schedule Request Erroneous Payload Field CIF # | If the reported error is not tied to a specific Schedule Request Payload Field, the CIF # (bits 17:15) and CIF Bit # (bits 14:10) fields shall be set to 0xE0 ('11100000') |
| 14:10 | Schedule Request Erroneoous Payload Field Bit # in CIF |  |
| 9:0 | Error Enumeration Index (EEI) |  |
|  |  | The EEI is a 10-bit unsigned integer with bit 9 as the MSB and bit 0 as the LSB. Each EEI value points to a different element of the Error Enumeration. As shown, higher index values are reserved for use by the MFA integrator to indicate MFA-specific errors. |
| EEI | Reason |  |
| 0 | No Error |  |
| 1 | Resource does not exist |  |
| 2 | Resource not working |  |
| 3 | Resource Conflict |  |
| 4 | Parameter out of range |  |
| 5 | Pointing vector outside FOR |  |
| 6 | Time Out | Expected information or data (e.g., Tx Data Packets) not received in time to execute |
| 7 | EIRP outside allowed range at pointing angle | Requested EIRP is inside overall range, but cannot be met at the requested pointing angle |
| 8:511 | To be defined by Working Group |  |
| 512:1022 | MFA Specific: To be defined by MFA integrator |  |


## Data Format

| Bit | Bits Used | Default | Range | Designation | Function | Reference |
|---|---|---|---|---|---|---|
| 31 | B | 0 |  | .dataFormat.packBit | Processing Efficient Packing for the AMS 16-bit signed I/Q format. |  |
| 30:29 | B | 01 |  | .dataFormat.realOrComplex | Complex Cartesian I/Q samples. |  |
| 28:24 | B | 00000 |  | .dataFormat.dataItemFormat | Signed fixed-point data items. |  |
| 23 | B | 0 |  | .dataFormat.repeat | Sample component repeating is not used. |  |
| 22:20 | B | 000 |  | .dataFormat.eventTagSize | Event tags are not used. |  |
| 19:16 | B | 0000 |  | .dataFormat.channelTagSize | Channel tags are not used. |  |
| 15:12 | B | 0000 |  | .dataFormat.dataItemFracSize | Fraction-size subfield is zero for the AMS signed fixed-point formats. |  |
| 11:6 | B | 001111 |  | .dataFormat.itemPackingFieldSize | Item packing field size is 16 bits. |  |
| 5:0 | B | 001111 |  | .dataFormat.dataItemSize | Data item size is 16 bits. |  |
| 31:16 | B | 0x0000 |  | .dataFormat.repeatCount | Repeat count is not used. |  |
| 15:0 | B | 0x0000 |  | .dataFormat.vectorSize | Vector size is not used. |  |
| 31 | B | 1 |  | .dataFormat.packBit | Link Efficient Packing for the AMS 8-bit signed I/Q format. |  |
| 30:29 | B | 01 |  | .dataFormat.realOrComplex | Complex Cartesian I/Q samples. |  |
| 28:24 | B | 00000 |  | .dataFormat.dataItemFormat | Signed fixed-point data items. |  |
| 23 | B | 0 |  | .dataFormat.repeat | Sample component repeating is not used. |  |
| 22:20 | B | 000 |  | .dataFormat.eventTagSize | Event tags are not used. |  |
| 19:16 | B | 0000 |  | .dataFormat.channelTagSize | Channel tags are not used. |  |
| 15:12 | B | 0000 |  | .dataFormat.dataItemFracSize | Fraction-size subfield is zero for the AMS signed fixed-point formats. |  |
| 11:6 | B | 000111 |  | .dataFormat.itemPackingFieldSize | Item packing field size is 8 bits. |  |
| 5:0 | B | 000111 |  | .dataFormat.dataItemSize | Data item size is 8 bits. |  |
| 31:16 | B | 0x0000 |  | .dataFormat.repeatCount | Repeat count is not used. |  |
| 15:0 | B | 0x0000 |  | .dataFormat.vectorSize | Vector size is not used. |  |
|  |  | Decimal | Binary |  | Data Item Type |  |
|  |  | 0 | 00000 |  | Signed Fixed-Point |  |
|  |  | 1 | 00001 |  | Signed VRT, 1-bit exponent |  |
|  |  | 2 | 00010 |  | Signed VRT, 2-bit exponent |  |
|  |  | 3 | 00011 |  | Signed VRT, 3-bit exponent |  |
|  |  | 4 | 00100 |  | Signed VRT, 4-bit exponent |  |
|  |  | 5 | 00101 |  | Signed VRT, 5-bit exponent |  |
|  |  | 6 | 00110 |  | Signed VRT, 6-bit exponent |  |
|  |  | 7 | 00111 |  | Signed Fixed-Point Non-Normalized |  |
|  |  | 8 | 01000 |  | Reserved |  |
|  |  | 9 | 01001 |  | Reserved |  |
|  |  | 10 | 01010 |  | Reserved |  |
|  |  | 11 | 01011 |  | Reserved |  |
|  |  | 12 | 01100 |  | Reserved |  |
|  |  | 13 | 01101 |  | IEEE-754 Half-Precision Floating-Point |  |
|  |  | 14 | 01110 |  | IEEE-754 Single-Precision Floating-Point |  |
|  |  | 15 | 01111 |  | IEEE-754 Double-Precision Floating-Point |  |
|  |  | 16 | 10000 |  | Unsigned Fixed-Point |  |
|  |  | 17 | 10001 |  | Unsigned VRT, 1-bit exponent |  |
|  |  | 18 | 10010 |  | Unsigned VRT, 2-bit exponent |  |
|  |  | 19 | 10011 |  | Unsigned VRT, 3-bit exponent |  |
|  |  | 20 | 10100 |  | Unsigned VRT, 4-bit exponent |  |
|  |  | 21 | 10101 |  | Unsigned VRT, 5-bit exponent |  |
|  |  | 22 | 10110 |  | Unsigned VRT, 6-bit exponent |  |
|  |  | 23 | 10111 |  | Unsigned Fixed-Point Non-Normalized |  |
|  |  | 24 | 11000 |  | Reserved |  |
|  |  | 25 | 11001 |  | Reserved |  |
|  |  | 26 | 11010 |  | Reserved |  |
|  |  | 27 | 11011 |  | Reserved |  |
|  |  | 28 | 11100 |  | Reserved |  |
|  |  | 29 | 11101 |  | Reserved |  |
|  |  | 30 | 11110 |  | Reserved |  |
|  |  | 31 | 11111 |  | Reserved |  |

