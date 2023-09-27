#!/bin/bash
if [ $1 = "add" ]; then
    sysctl -w net.ipv4.ip_forward=1
    iptables -t nat -I POSTROUTING -o enp1s0 -s 10.10.0.0/24 -j MASQUERADE
fi
if [ $1 = "del" ]; then
    sysctl -w net.ipv4.ip_forward=0
    iptables -t nat -D POSTROUTING -o enp1s0 -s 10.10.0.0/24 -j MASQUERADE
fi
