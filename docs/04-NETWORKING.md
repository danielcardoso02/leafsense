# LeafSense Networking Guide

## Overview
This document describes the various networking options available for connecting the Raspberry Pi to a development machine or deploying LeafSense in a networked environment.

---

## Connection Types

### 1. Static Ethernet (Recommended for Development)
Direct USB-C to Ethernet connection with static IP addressing.

**Advantages:**
- Simple and reliable
- No DHCP server required
- Direct one-to-one connection
- Perfect for development and testing
- Minimal latency

**Disadvantages:**
- Requires USB-C Ethernet adapter on host
- Limited to single device connection
- Not suitable for multi-device setups

**Setup:**
```bash
# On Host PC (Linux)
# 1. Identify USB-C Ethernet adapter
ip link show | grep -E "enx|eth"

# 2. Configure static IP
sudo ip addr add 10.42.0.1/24 dev enx00e04c3601a6
sudo ip link set enx00e04c3601a6 up

# On Raspberry Pi (/etc/network/interfaces)
auto eth0
iface eth0 inet static
    address 10.42.0.196
    netmask 255.255.255.0
    gateway 10.42.0.1

# Connect via SSH
ssh root@10.42.0.196
```

---

### 2. DHCP Ethernet
Dynamic IP assignment using DHCP.

**Advantages:**
- Automatic IP assignment
- Scalable for multiple devices
- Standard network configuration
- Compatible with switches/routers

**Disadvantages:**
- Requires DHCP server
- IP addresses may change
- Slight additional latency
- More complex troubleshooting

**Setup:**
```bash
# On Raspberry Pi (/etc/network/interfaces)
auto eth0
iface eth0 inet dhcp

# Find IP address
arp-scan --localnet
# or
nmap -sn 192.168.1.0/24
```

---

### 3. USB Gadget Mode (Deprecated)
Hardware USB Ethernet emulation over USB power connection.

**Status:** ⚠️ **Not Recommended** - Complex setup, unreliable detection

**Why deprecated:**
- Requires GPIO configuration
- Kernel module loading issues
- Inconsistent device tree overlay support
- Higher failure rate than standard Ethernet

**For historical reference only - Use Static Ethernet instead.**

---

### 4. WiFi Connection (Future)
Wireless connectivity when WiFi module is added.

**Planned Features:**
- Automatic network scanning
- Password-protected connections
- Remote access capabilities

**Configuration (when available):**
```bash
# wpa_supplicant.conf
network={
    ssid="YourNetwork"
    psk="YourPassword"
    key_mgmt=WPA-PSK
}
```

---

## Network Configuration Files

### Boot Configuration (`/boot/config.txt`)
```ini
# Enable Ethernet (SPI-based Waveshare display uses SPI, not Ethernet pins)
# Standard Ethernet works over standard GPIO pins with proper driver

# Camera overlay
dtoverlay=ov5647

# Waveshare 3.5" LCD (C)
dtoverlay=waveshare35c
```

### Network Interfaces (`/etc/network/interfaces`)

**Static Configuration:**
```ini
auto lo
iface lo inet loopback

auto eth0
iface eth0 inet static
    address 10.42.0.196
    netmask 255.255.255.0
    gateway 10.42.0.1
    # Optional: DNS servers
    # dns-nameservers 8.8.8.8 8.8.4.4
```

**DHCP Configuration:**
```ini
auto lo
iface lo inet loopback

auto eth0
iface eth0 inet dhcp
```

---

## Network Diagnostics

### Test Connectivity
```bash
# From host machine
ping 10.42.0.196

# From Raspberry Pi
ping 10.42.0.1
```

### Check Interface Status
```bash
# View all interfaces
ip link show

# View IP configuration
ip addr show

# View routing table
ip route show
```

### SSH Connection
```bash
# Connect to Pi
ssh root@10.42.0.196

# With verbose output for debugging
ssh -vvv root@10.42.0.196

# Copy files to/from Pi
scp /path/to/file root@10.42.0.196:/destination/
scp root@10.42.0.196:/path/to/file /local/destination/
```

### Network Scanning
```bash
# Scan subnet for all devices
nmap -sn 10.42.0.0/24

# Find ARP entries
arp -a

# Monitor network traffic
tcpdump -i eth0 -n
```

---

## Troubleshooting

### No Connectivity
1. Check physical connection (cable/USB adapter)
2. Verify interface is UP: `ip link show`
3. Check IP configuration: `ip addr show`
4. Test with ping: `ping 10.42.0.196`
5. Check routing: `ip route show`

### SSH Connection Refused
1. Verify Pi is responding to ping
2. Check SSH service is running: `systemctl status dropbear`
3. Clear known_hosts: `ssh-keygen -R 10.42.0.196`
4. Try with verbose: `ssh -vvv root@10.42.0.196`

### DHCP IP Not Assigned
1. Verify DHCP server is running
2. Check DHCP lease: `ip addr show`
3. Restart DHCP client: `systemctl restart dhcpcd`
4. Fall back to static IP configuration

### Intermittent Disconnections
1. Check network cable for damage
2. Monitor system load: `top`, `htop`
3. Check kernel logs: `dmesg | grep -i "network\|eth"`
4. Test with different USB adapter
5. Consider static IP instead of DHCP

---

## Performance Tuning

### MTU Size
```bash
# View current MTU
ip link show eth0

# Increase for better throughput (if supported)
sudo ip link set eth0 mtu 9000
```

### Network Buffer Tuning
```bash
# Increase network buffer sizes
sudo sysctl -w net.core.rmem_max=134217728
sudo sysctl -w net.core.wmem_max=134217728
```

---

## Security Considerations

### SSH Key Management
```bash
# Generate new key pair
ssh-keygen -t ed25519 -f ~/.ssh/leafsense_rsa

# Add to Pi authorized_keys
cat ~/.ssh/leafsense_rsa.pub | ssh root@10.42.0.196 "cat >> ~/.ssh/authorized_keys"

# Use key for connections
ssh -i ~/.ssh/leafsense_rsa root@10.42.0.196
```

### Network Isolation
- Use static IP within private subnet (10.42.0.0/24)
- No direct internet exposure required
- Consider firewall rules for multi-device setups

---

## Quick Reference

| Method | Complexity | Reliability | Speed | Recommended |
|--------|-----------|-------------|-------|-------------|
| Static Ethernet | Low | Very High | High | ✅ Yes |
| DHCP Ethernet | Medium | High | High | Conditional |
| USB Gadget | High | Low | Medium | ❌ No |
| WiFi | Medium | Medium | Medium | Future |

---

## Related Documentation
- [Raspberry Pi Deployment](06-RASPBERRY-PI-DEPLOYMENT.md)
- [Troubleshooting Guide](10-TROUBLESHOOTING.md)
- [Terminology](19-TERMINOLOGY.md)
