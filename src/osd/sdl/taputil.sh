#!/bin/sh

NAME=$2
UID=`id -u $NAME`
HOSTIP=$4
EMUIP=$3
TAP="tap-mess-$UID-0"

if [ `id -u` != "0" ]
then
echo "must be run as root"
exit
fi

if [ "$1" = "-d" ]
then
echo 0 > /proc/sys/net/ipv4/ip_forward
echo 0 > /proc/sys/net/ipv4/conf/all/proxy_arp
chmod 660 /dev/net/tun
ip tuntap del dev $TAP mode tap
exit
fi

if [ "$#" != "5" ]
then
echo "usage: mess-tap [-c] [-d] USER EMUADDR HOSTADDR MASK\n"
echo "-c\t\tcreate interface"
echo "-d\t\tdelete interface"
echo "USER\t\tuser to own interface, required to delete"
echo "EMUADDR\temulated machine ip address"
echo "HOSTADDR\thost ip address"
exit
fi

echo 1 > /proc/sys/net/ipv4/ip_forward
echo 1 > /proc/sys/net/ipv4/conf/all/proxy_arp
chmod 666 /dev/net/tun

ip tuntap add dev $TAP mode tap user $NAME pi
ip link set $TAP up arp on
ip addr replace dev $TAP $HOSTIP/32
ip route replace $EMUIP via $HOSTIP dev $TAP 
