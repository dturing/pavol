pavol
=====

CLI utility to set volume/mute on PulseAudio sinks (output devices).

This is a simple C-based utility to iterate all PulseAudio sinks and set their Mute state and/or Volume.


Build Dependencies
------------------

	* libpulse (apt-get install libpulse-dev)
	* GNU Make
	* GCC
 
 
Building
--------

	$ git clone git://github.com/dturing/pavol.git
	$ cd pavol
	$ make
 
Usage
-----

	Mute all outputs:

    $ ./pavol -m
    

	Unmute all outputs:
	
	$ ./pavol
	
	
	Set Volume to 50% (-18dB):
	
	$ ./pavol -v 50


