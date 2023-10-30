import subprocess
import json
import argparse
import netifaces

def get_mac():
    """Gets the MAC address for the robot from the network config info.

    Returns:
        str: A MAC address for the robot.

    Example:
        >>> print(get_mac())
        AA:BB:CC:DD:EE:FF

    """

    interface = [x for x in netifaces.interfaces() if 'wlan' in x][0]
    return netifaces.ifaddresses(interface)[netifaces.AF_LINK][0]['addr']

def main():

    parser = argparse.ArgumentParser()
    parser.add_argument("mac_list", help="JSON file containing MAC to id mapping")

    # Retrieve the MAC address for the robot
    mac_address = get_mac()

    # Parser and set CLI arguments
    args = parser.parse_args()

    # Retrieve the MAC list file, containing a mapping from MAC address to robot ID
    try:
        f = open(args.mac_list, 'r')
        mac_list = json.load(f)
    except Exception as e:
        print(repr(e))
        print('Could not open file ({})'.format(args.node_descriptor))

    if(mac_address in mac_list):
        robot_id = mac_list[mac_address]
        cmds = ['sudo', 'raspi-config', 'nonint', 'do_hostname', 'GTernal'+robot_id]

        pids = []
        for cmd in cmds:
            pids.append(subprocess.Popen(cmd))

        for pid in pids:
            pid.communicate()
    else:
        print('MAC address {} not in supplied MAC list file'.format(mac_address))
        raise ValueError()

if __name__ == '__main__':
    main()
