#!/bin/sh
# $Id: test-multiple-tunnels-1.sh,v 1.1 2003/02/15 04:32:50 quozl Exp $
# Create multiple alias network interfaces
# set -x

clear
echo -n "Enter total # of interfaces addresses to add on eth0 -> "
read numadds

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
		/sbin/ifconfig eth0:$counter1 80.120.$counter4.$counter2 netmask 255.252.0.0 broadcast 80.120.255.255
                counter2=`expr $counter2 + 1`
	else
		counter2=1	
		counter3=`expr $counter3 + 1`
                counter4=`expr $counter4 + 1`	
		/sbin/ifconfig eth0:$counter1 80.120.$counter4.$counter2 netmask 255.252.0.0 broadcast 80.120.255.255
		counter2=`expr $counter2 + 1`
	fi
	counter1=`expr $counter1 + 1`
done
