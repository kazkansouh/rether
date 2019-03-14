# Rether

Rether is a tool for sending/receiving **R**aw **Ether**net ([IEEE
802.3](http://www.ieee802.org/3/)) frames on Linux. It was developed
as a tool for injecting frames into a network for testing network
security appliances.

## Using

Easiest way to get the program is by pulling from dockerhub.

```bash
docker pull karimkanso/rether
```

The docker image was designed to work in GNS3, and has a persistent
`/root` folder. However, it should work normally without GNS3 with the
network interface bridged to the physical network at layer 2.

### CLI

```bash
# rether --help
rether 0.1 - Copyright (C) 2019 Karim Kanso
Usage: rether [OPTION...]
  -s, --source-int=if                  Interface to send frame from. Defines source mac address. Default:
                                       eth0
  -e, --ethertype=0xZZZZ               Value for ethertype field of ethernet header. E.g. 0x0800 for IPv4.
                                       See IEEE 802.3. Default: 0xDEAD
  -r, --receive-only                   Listen for frames matching the specified ethertype and print them.
                                       Incompatible with below arguments.
  -d, --dest-mac=uu:vv:ww:xx:yy:zz     MAC address to send the frame to. Default: 00:01:02:03:04:05
  -p, --payload=data                   String to send as frame data, can not be used with --payload-file
                                       option. Binary data can be set with -b64 option. Default: "hello
                                       world"
  -f, --payload-file=file-name         File to read binary data from to send in frame, can not be used with
                                       --payload. Reads at most 10KiB, however, be careful of the MTU as no
                                       additional checks are performed. Can be given as '-' to read from
                                       stdin. Default: NONE
  -b, -b64                             Decode payload given on command line using base64 codec before
                                       sending frame.
```

For example:

* To listen for IPv6 packets on `eth1`, the following can be used:

    ```bash
    # rether -r -e 0x86DD -s eth1
    ```
* To inject a frame with anundefined
  [ethertype](https://www.iana.org/assignments/ieee-802-numbers) of
  0xFEED on `tap0` to `11:11:11:22:22:22`, the following can be used:

    ```bash
    # rether -e 0xFEED \
             -s tap0 \
             -b64 -p VGhlIHF1aWNrIGJyb3duIGZveCBqdW1wcyBvdmVyIHRoZSBsYXp5IGRvZw== \
             -d 11:11:11:22:22:22
    ```

  Here, the `-b64` and `-p` define a base64 encoded payload will be
  sent in the data portion of the frame. The application will:

    1. Pad the data portion to 46 bytes with zero bytes.
    2. Truncate the data to the MTU of the egress interface.
    3. Set the source address to the MAC of the egress interface.

## Building

Dependencies include `gnulib` and `libpopt`. These can be installed on
Ubuntu 18.04 with:

```bash
apt install libpopt-dev gnulib
```

In addition, standard build tools such as `git`, `autoconf`,
`automake`, `pkgconfig`, `gcc` are required.

Once a build environment is ready, run the following to install:

```bash
git clone https://github.com/kazkansouh/rether.git
cd rether
gnulib-tool --update
autoconf -ivf
configure
make
make install
```

## Other bits

Released under GPLv3.

(C) Karim Kanso 2019, all rights reserved.
