	LB Real Audio Player for OS/2 ver. 0.02

This is a quick and dirty port of Amiga Real Audio decoder 1.1/2.0 with DART
playback. THIS SOFTWARE HAS NO WARRANTY, USE IT AT YOUR OWN RISK.


			Installation

First, to process .ram files or for .ra file streaming you need WGET. WGET is
a free batch http/ftp downloading tool, I think it can be found on
http://hobbes.nmsu.edu Socond, place wget.exe, RAPlay.exe and RAStream.cmd in
a directory in your PATH.


			Usage

To play a local .ra file type: RAPlay <.ra file>
To play a remote .ra file in streaming mode type:  RAStream <URL>
To play a local .ram file type: RAStream <.ram file>

To use this player as a Netscape helper application you must create two
application entries in Applicatons category of Netscape preferences.
The first application entry is for .ra files:

Description of type: Real Audio .ra
File extension:      .ra
MIME Type:           audio/x-realaudio
Application:         RAPlay.EXE

The second application entry is for .ram files:

Description of type: Real Audio .ram
File extension:      .ram
MIME Type:           audio/x-pn-realaudio
Application:         cmd.exe /c RAStream.cmd


			A Small FAQ

Q: I get "unsupported format" or "recognized format: 28.8kbaud, unrecognized
   file parameters" message when trying to play a file.
A: Unfortunately the original decoder does not support all the Real Audio
   formats, thus the player does not support them too. Let's hope it will
   later. Also there may be another problem: you're either trying to supply
   .ram file to RAPlay.exe or .ram file URL to RAStream.cmd, you can only
   supply local .ra file name to RAPLay.exe and either remote .ra file URL
   or local .ram file name to RAStream.cmd

Q: I get "access denied" or something like from WGET when trying to run
   RAStream.cmd
A: The problem is that the real Real Audio player uses a proprietary protocol
   for thansferring .ra files. All I know about it is that it has "pnm:"
   prefix in URL and has a lot of problems with firewalls. So I try accessing
   the .ra file by http protocol. On some servers this can be prohibited
   (really I can't imagine why do they do it) so you receive such a message.


			Contacts

My e-mail address is boga@fly.triniti.troitsk.ru
My OS/2 audio programs page URL is
http://www.freebyte.ml.org/~boga/OS2Programs.html
The original Amiga Real Time decoder page URL is
HTTP://csc.smsu.edu/~strauser/RA.html


			History

v.0.01	- Initial release
v.0.02	- Added support for playing from stdin and to stdout
	- Renamed ram.cmd to rastream.cmd and changed it to handle
	  .ra file streaming
	- Decreased DART buffer size to decrease initial CPU load peak.
	- Changed progress display and directed it to stdout in case decoded
	  data do not go to stdout. (Thus you can use >nul redirection to make
	  it quiet)
