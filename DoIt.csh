#!/bin/csh
set index=0
set count=7802
while ($count)
echo $index
getslice $index 72 x72 P < jis24.fnt | ppmtogif -transparent white | tee -a mugi.out | wc -c >> mugi.len
set index=`echo $index 1 + p | dc`
set count=`echo $count 1 - p | dc`
end
