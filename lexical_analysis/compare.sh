# Script:
#     
#    compares the output of lex to the output of the reference compiler (lexico)
#
#
# Usage:
#
#    ./compare.sh testinputs/<input-file>
#
#
# Note: 
#
#    if there are differences they will be shown as output. No output means
#    lex generated the same output as lexico.


./lex $1 output
./lexico $1 output.txt
diff output output.txt
