<img width="100" height="100" alt="image" src="https://github.com/user-attachments/assets/53bdc388-d76a-4e8a-bd55-c003127f79a7" />

# Blower
GNOME ThinkPad Fan Speed Controller


<img width="482" height="704" alt="demo" src="https://github.com/user-attachments/assets/5058daae-89fe-44b7-a55d-302492c2f964" />

## NOTE!
For now you will need to enable `thinkpad_acpi` here's how to do it

1. Load it:
```bash
sudo modprobe thinkpad_acpi
```
2. Configure it:
```bash
echo "options thinkpad_acpi fan_control=1" | sudo tee -a /etc/modprobe.d/thinkfan.conf
```
3. Reload it:
```bash
sudo modprobe -r thinkpad_acpi && sudo modprobe thinkpad_acpi
```
4. Verify it:
```bash
cat /proc/acpi/ibm/fan
```

You should see somthing like:
```bash
status:		enabled
speed:		XXXXX
level:		XXXXX
commands:	level <level> (<level> is 0-7, auto, disengaged, full-speed)
commands:	enable, disable
commands:	watchdog <timeout> (<timeout> is 0 (off), 1-120 (seconds))
```

## Install

### Dependencies 
```
git build-essential libadwaita-1-dev libgtk-4-dev policykit-1 lm-sensors  
```

### Cloning
```
git clone https://github.com/GNUfault/blower.git && cd blower
```

### Compiling
```
make
```

### Running
Click on the Blower icon or run:
```
blower
```

## Remove
```
make remove
```
