/* This is a REXX script for .ra streaming / processing .ram files.
 Requires WGET */

/* WGET name and parameters to pipe the output to stdout */
WGET='wget -O -'
/* RAPlay name */
RAPLAY='RAPLay'

Parse Arg In
if word(In,1)='' then do
   say 'Usage: RAStream {.ram file|.ra file URL}'
   exit
   end

/* This is a workaround for a weird thing: the command line parameter is cut
at '//' */
do while queued()<>0; parse pull; end
'@echo %1|rxqueue'
parse pull In
In=strip(In)

if pos('://',In)<>0 then do    /* It's an URL */
   URL=In
   call ProcURL
   exit
   end

do while chars(In)<>0
   URL=linein(In)
   call ProcURL
   end
exit

ProcURL:
   if translate(left(URL,4))='PNM:' then URL='http:'substr(URL,5)
   if (translate(left(URL,4))='FTP:')|(translate(left(URL,5))='HTTP:') then '@'WGET URL'|'RAPLAY '->nul'
   else if word(URL,1)<>'' then say 'Not a valid URL:' URL
   return
