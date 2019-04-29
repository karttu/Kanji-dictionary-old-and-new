del $TMPFUNDE.H
a:\aztec-4\proto -o $TMPFUNDE.H -a -r -s -ia:\ -ia:\incl -ib:\ kanjidic.c init.c
protheus -i -s < $TMPFUNDE.H > kdfundef.h
del $TMPFUNDE.H
