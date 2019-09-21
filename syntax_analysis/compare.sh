# Script:
#     
#    compares the output of parser to the output of the reference
#    parser (sintatico)
#
#
# Usage:
#
#    ./compare.sh testinputs_sem/<input-file>
#
#
# Note: 
#
#    if there are differences they will be shown as output. No output means
#    lex generated the same output as lexico.


./sintatico $1 tmpoutput
./parser $1 tmpparoutput
rm paroutput
rm output

# the project allowed for differences in white space, this cuts it out
cat tmpparoutput | tr -d " \t\n\r" >> paroutput
cat tmpoutput | tr -d " \t\n\r" >> output

diff paroutput output
