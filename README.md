cambotino
=========

An Arduino project that acts as a wired remote shutter release for a Nikon DSLR.


## Pins

### Joypad

(15) PORTJ0 = CLK out (normal high, strobe low)
(14) PORTJ1 = LAT out (normal low, strobe high)
(16) PORTH1 = D0 in (actuated button = low)

### Relays and relay-like devices

(17) PORTH0 = cue shutter, active low
(18) PORTD3 = fire shutter, active low
(19) PORTD2 = open valve, active high

# LCD

(20) SDA
(21) SCL
