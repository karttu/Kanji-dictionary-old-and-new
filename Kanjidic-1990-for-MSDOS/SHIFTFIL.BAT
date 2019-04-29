if exist %DB_DIR%newest.kd   goto ok
echo Won't delete only database left !
goto poing
:ok
del %DB_DIR%2newest.kd
ren %DB_DIR%newest.kd  2newest.kd
:poing
