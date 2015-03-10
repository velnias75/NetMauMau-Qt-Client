Qt client for NetMauMau
=======================

A Qt client for the [NetMauMau server](https://github.com/velnias75/NetMauMau).

Requirements
------------

It requires Qt in at least version 4.4

(for Qt versions < 4.6 in `mainwindow.ui` you'll need to replace all occurences of 
`Qt::ToolButtonFollowStyle` to `Qt::ToolButtonTextUnderIcon`)

For the speech support you'll need [eSpeak](http://espeak.sourceforge.net) and Qt >= 4.8

Building
--------

It depends on the NetMauMau sources, which can be found at
[https://github.com/velnias75/NetMauMau](https://github.com/velnias75/NetMauMau) 

* to build and install with *previously installed NetMauMau*, invoke `qmake CONFIG+=release && make && sudo make install`
* to build and install with *previously installed NetMauMau* **and** *eSpeak*, invoke `qmake CONFIG+=release CONFIG+=espeak && make && sudo make install`

Help translating
----------------
<a href="https://hosted.weblate.org/engage/netmaumau/?utm_source=widget">
<img src="https://hosted.weblate.org/widgets/netmaumau/-/287x66-white.png" alt="Übersetzungsstatus" />
</a>

Binary releases
===============
[![Download NetMauMau](https://img.shields.io/sourceforge/dm/netmaumau.svg)](https://sourceforge.net/projects/netmaumau/files/latest/download)
Gentoo
------
NetMauMau is available on Gentoo via the overlay **games-overlay** which can be added by `layman`
The GitHub repository of **games-overlay** is here: [https://github.com/hasufell/games-overlay](https://github.com/hasufell/games-overlay)

**Adding the overlay**

With paludis: see [Paludis repository configuration](http://paludis.exherbo.org/configuration/repositories/index.html)

With layman:
```layman -f -o https://raw.github.com/hasufell/games-overlay/master/repository.xml -a games-overlay``` or ```layman -a games-overlay```

Install NetMauMau with `emerge games-board/netmaumau`

Ubuntu
------
Binary packages are available for Precise, Trusty, Utopic and Vivid
in my Launchpad PPA at [https://launchpad.net/~velnias/+archive/ubuntu/velnias](https://launchpad.net/~velnias/+archive/ubuntu/velnias)

Add the repository to your system: 

`sudo add-apt-repository ppa:velnias/velnias`

PlayDeb.net offers [binaries](http://www.playdeb.net/game/NetMauMau) too.

Debian 7
--------
* insert following line into `/etc/apt/sources.list`

    `deb http://download.opensuse.org/repositories/home:/velnias/Debian_7.0 /`

* *(optionally)* add the GPG-key

    `wget -O- http://sourceforge.net/projects/netmaumau/files/debian-apt.asc | apt-key add -`

* run `apt-get update` 

Windows
-------
[![Download NetMauMau](https://a.fsdn.com/con/app/sf-download-button)](https://sourceforge.net/projects/netmaumau/files/latest/download)

