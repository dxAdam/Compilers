./sintatico $1 tmpoutput
./parser $1 tmpparoutput
rm paroutput
rm output
cat tmpparoutput | tr -d " \t\n\r" >> paroutput
cat tmpoutput | tr -d " \t\n\r" >> output
diff paroutput output
