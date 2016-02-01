Do you play music from your computer? Is your soundcard connected to a
traditional amplifier? Are you frustrated that you cannot control the volume of
the amplifier from your music player? This is the solution!

## How it works

You configure a
[dummy soundcard](http://www.alsa-project.org/main/index.php/Matrix:Module-dummy)
for the sole purpose of adjusting its volume. The tool monitors the mixer of
the soundcard and executes the commands you provided every time the volume
changes. The commands should do the necessary magic to tell your amplifier to
change the volume.

The simplest way to control the amplifier, is to emulate its infrared remote
(e.g., using the `irsend` utility).

Since the majority of the amplifiers can only be controlled using the infrared
remote, there is no way to know the current volume of the amplifier. Thus, for
simplicity, each volume change is treated as a single positive/negative step
and the volume of the soundcard is reset to 50% after each change.

## How to build it

```
$ gcc -o alsa-amp-mixer alsa-amp-mixer.cc -Wall -D__DEBUG__ \
  `pkg-config --cflags alsa` `pkg-config --libs alsa`
```

## How to use it

### Configure a dummy soundcard

```
$ sudo modprobe snd-dummy
```

### Figure out which is the dummy soundcard

```
$ aplay -l
card 0: sndrpihifiberry [snd_rpi_hifiberry_dac], device 0: HifiBerry DAC HiFi pcm5102a-hifi-0 []
  Subdevices: 0/1
  Subdevice #0: subdevice #0
card 1: Dummy [Dummy], device 0: Dummy PCM [Dummy PCM]
  Subdevices: 8/8
  Subdevice #0: subdevice #0
  Subdevice #1: subdevice #1
  Subdevice #2: subdevice #2
  ...
```

### Run the tool

The tool expects three command-line arguments: the name of the card (`hw:1`
given the example above), the command to execute when the volume is increased,
and the command to execute when the volume is decreased.

For example, you could use the command `irsend` to send the necessary infrared
commands to the amplifier to increase and decrease the volume.

```
$ ./alsa-amp-mixer hw:1 \
    "irsend SEND_ONCE CambridgeAudioAmp KEY_VOLUMEUP" \
    "irsend SEND_ONCE CambridgeAudioAmp KEY_VOLUMEDOWN"
```

### Test it

Increase the volume and make sure the first of the two commands is executed:
```
$ amixer -c 1 sset Master 90%
```

Decrease the volume and make sure the second of the two commands is executed:
```
$ amixer -c 1 sset Master 10%
```

Note that each volume change results in a single invocation of the
corresponding command and that the volume is automatically reset to 50% after
the command is terminated.

### Configure MPD to use the mixer of the dummy soundcard

[MPD](http://www.musicpd.org) allows to use a custom mixer device:

```
audio_output {
  type 		"alsa"
  name 		"HiFiBerry"
  device 	"hw:0,0"
  mixer_device  "hw:1"
  mixer_control	"Master"
}
```

Use your favourite MPD client to adjust the volume.

## My setup

My setup consists of a Raspberry PI, a
[HiFiBerry DAC](https://www.hifiberry.com/), and
[MPD](http://www.musicpd.org). The MPD client is
[MPDroid](https://github.com/abarisain/dmix).

The Raspberry PI is connected to an infrared receiver and to a
transmitter. Thus, I can control the volume of the amplifier from my MPD
client, and the playback using the amplifier's remote. The infrared transmitter
and receiver are placed next to the amplifier and the signal of the transmitter
is sufficiently strong to hit the wall on the other side of the room and bounce
back to the amplifier.

## Links

- [Raspberry Pi lirc_rpi](http://aron.ws/projects/lirc_rpi/)
