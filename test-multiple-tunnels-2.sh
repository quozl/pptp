#!/bin/sh
# $Id: test-multiple-tunnels-2.sh,v 1.1 2003/02/15 04:32:50 quozl Exp $
# Create multiple tunnels
# set -x

clear
echo -n "Enter total # of pptp tunnels to establish -> "
read numadds
echo -n "Enter the public ip address of the 3K server -> "
read serverpublic
echo -n "Enter the private ip address of the 3K server -> "
read serverprivate

if [ $numadds -le 0 ]
then
	echo "Must enter at least a positive integer"
  	exit 1
else
	counter1=1
	counter2=1
	counter3=1
        counter4=11
	hostnum=250
fi

while [ $counter1 -le $numadds ]
do
	if [ $counter2 -le $hostnum ]
	then
		./pptp $serverpublic --localbind=80.120.$counter4.$counter2 name pptpuser remotename PPTP ipparam $serverprivate	
		sleep 2
                echo "Tunnel $counter1 is up!!!"
                counter2=`expr $counter2 + 1`
	else
		counter2=1	
                counter4=`expr $counter4 + 1`	
		./pptp $serverpublic --localbind=80.120.$counter4.$counter2 name pptpuser remotename PPTP ipparam $serverprivate 
		sleep 2
		echo "Tunnel $counter1 is up!!"
		counter2=`expr $counter2 + 1`
	fi
	counter1=`expr $counter1 + 1`
done
