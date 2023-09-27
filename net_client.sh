#!/bin/bash
if [ $1 = "add" ]; then
    sysctl -w net.ipv4.ip_forward=1
    iptables -t nat -I POSTROUTING -o tun0 -j MASQUERADE
    iptables -t mangle -I PREROUTING -s 192.168.122.66 -j MARK --set-mark 1
    ip rule add not from all fwmark 1 table 120
    ip route add 192.168.122.66 dev virbr0 table 120
    ip route add 0.0.0.0/0 dev tun0 src 10.10.0.2 table 120
fi
if [ $1 = "del" ]; then
    sysctl -w net.ipv4.ip_forward=0
    iptables -t nat -D POSTROUTING -o tun0 -j MASQUERADE
    iptables -t mangle -D PREROUTING -s 192.168.122.66 -j MARK --set-mark 1
    ip rule del not from all fwmark 1 table 120
    ip route del 192.168.122.66 dev virbr0 table 120
    ip route del 0.0.0.0/0 dev tun0 src 10.10.0.2 table 120
fi
