Proof of concept Qt client for NetMauMau
========================================

A proof of concept Qt client for NetMauMau, which serves for testing
the NetMauMau server and client API.

It is not a sophisticated client with a nice GUI, but it can serve as
base for potential client developers as well.

Building
--------

It depends on the NetMauMau sources, which can be found at
[https://github.com/velnias75/NetMauMau](https://github.com/velnias75/NetMauMau) 
and need to be checked out at the same directory level.

To build, invoke `qmake && make all`

Requirements
------------

It requires Qt in at least version 4.8.2


Binary releases
===============

Gentoo
------
NetMauMau is available on Gentoo via the overlay **games-overlay** which can be added by `layman`
The GitHub repository of **games-overlay** is here: [https://github.com/hasufell/games-overlay](https://github.com/hasufell/games-overlay)

Install NetMauMau with `emerge games-board/netmaumau`

Ubuntu
------
Binary packages are available for Precise, Trusty, Utopic and Vivid
in my Launchpad PPA at [https://launchpad.net/~velnias/+archive/ubuntu/velnias](https://launchpad.net/~velnias/+archive/ubuntu/velnias)

Add the repository to your system: 

`sudo add-apt-repository ppa:velnias/velnias`

Debian 7
--------
[http://download.opensuse.org/repositories/home:/velnias/Debian_7.0](http://download.opensuse.org/repositories/home:/velnias/Debian_7.0)


Windows
-------
[https://sourceforge.net/projects/netmaumau/](https://sourceforge.net/projects/netmaumau/)

