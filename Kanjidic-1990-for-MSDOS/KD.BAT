if A == A%DATABASE% goto hiba
if not exist %NEWDATABASE% goto veba
call shiftfil
if exist %NEWDATABASE% goto ertzu
:veba
kanjidic %1 %2 %3 %4 %5 %6 %7 %8 %9
goto pois
:hiba
echo run batch SETK first !
goto pois
:ertzu
echo Error in batch SHIFTFIL.BAT
:pois
