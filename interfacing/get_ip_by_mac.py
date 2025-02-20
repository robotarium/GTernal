import argparse
import subprocess
import re
import json
import getpass
import numpy as np


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('mac_list', help='Path to JSON file containing MAC to ID mapping')
    parser.add_argument('interface', help='Network interface on which to make query')
    parser.add_argument('command', help='Command for sshpass')
    parser.add_argument('-c', help='Optional command for address', default=None)
    parser.add_argument('-n', help='Optional number of robots needed to be found', default=None)
    parser.add_argument('-d', help='Optional directory for address', default='')
    parser.add_argument('-f', help='Optional file to copy to the address', default='')

    args = parser.parse_args()
    interface = args.interface

    try:
        f = open(args.mac_list, 'r')
        mac_to_id = json.load(f)
    except Exception as e:
        print(repr(e))
        print('Could not open file ({})'.format(args.mac_list))

    checkNumberRobots = True   

    while(checkNumberRobots):
        pid = subprocess.Popen(['arp-scan', '-I', interface, '-l', '-t', '100', '-r', '5'], stdout=subprocess.PIPE)
        out = pid.communicate()[0].decode()
        lines = out.split('\n')

        mac_to_ip = {}

        for line in lines:

            # Look for MAC address in the line
            mac = re.search(r'([0-9A-F]{2}[:-]){5}([0-9A-F]{2})', line, re.I)
            if(mac is None):
                continue
            # Else
            mac = mac.group()

            ip = re.search(r'((2[0-5]|1[0-9]|[0-9])?[0-9]\.){3}((2[0-5]|1[0-9]|[0-9])?[0-9])', line, re.I)
            if(ip is None):
                continue
            # Else
            ip = ip.group()
            mac_to_ip[mac] = ip

        # Make ID to IP mapping
        id_to_ip = dict({mac_to_id[x]: y for x, y in mac_to_ip.items() if x in mac_to_id})

        print(id_to_ip)
        print('Number of robots found: ', len(id_to_ip))
        
        if((args.n is None) or (len(id_to_ip) is int(args.n))):
           checkNumberRobots = False
        else:
            print('Number of robots needed, provided by user: ', args.n)
            print('All robots not found, retrying...') 

    if(args.command is None):
        return

    print('Enter secrets for robots')
    password = getpass.getpass()
    if args.command == 'ssh':
        cmds = [['sshpass', '-p', password, 'ssh', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', 'pi@'+x, args.c] for x in id_to_ip.values()]
    elif args.command == 'scp':
        cmds = [['sshpass', '-p', password, 'scp', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', args.f, 'pi@'+x+':'+args.d] for x in id_to_ip.values()]
    elif args.command == 'setup':
        cmds = [['sshpass', '-p', password, 'scp', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', '../setup/setup', 'pi@'+x+':/home/pi'] for x in id_to_ip.values()]
        _ = [cmds.append(['sshpass', '-p', password, 'ssh', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', 'pi@'+x, 'sudo ./setup']) for x in id_to_ip.values()]
    elif args.command == 'setup_base_image':
        cmds = [['sshpass', '-p', password, 'scp', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', '-r', '../../GTernal/', 'pi@'+x+':/home/pi/git'] for x in id_to_ip.values()]
        _ = [cmds.append(['sshpass', '-p', password, 'ssh', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', 'pi@'+x, 'sudo ../setup/setup_base_image']) for x in id_to_ip.values()]
    elif args.command == 'setup_from_base':
        cmds = [['sshpass', '-p', password, 'scp', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', '-r', '../../GTernal/', 'pi@'+x+':/home/pi/git'] for x in id_to_ip.values()]
        _ = [cmds.append(['sshpass', '-p', password, 'ssh', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', 'pi@'+x, 'sudo ../setup/setup_from_base']) for x in id_to_ip.values()]

    pids = []
    for cmd in cmds:
        pids.append(subprocess.Popen(cmd))
        pids[-1].wait() # Needed to wait for the setup script to be copied

    for pid in pids:
        pid.communicate()

    # For updating the mac_list.json and docker_run.sh files and restarting the Pi to apply all settings
    if args.command == 'setup':
        cmds = []
        _ = [cmds.append(['sshpass', '-p', password, 'scp', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', '../config/mac_list.json', 'pi@'+x+':/home/pi/git/GTernal/config']) for x in id_to_ip.values()]
        _ = [cmds.append(['sshpass', '-p', password, 'scp', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', '../docker/docker_run.sh', 'pi@'+x+':/home/pi/git/GTernal/docker']) for x in id_to_ip.values()]
        _ = [cmds.append(['sshpass', '-p', password, 'ssh', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', 'pi@'+x, 'sudo reboot']) for x in id_to_ip.values()]

    elif args.command == 'setup_base_image':
        cmds = []
        _ = [cmds.append(['sshpass', '-p', password, 'scp', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', '../config/mac_list.json', 'pi@'+x+':/home/pi/git/GTernal/config']) for x in id_to_ip.values()]
        _ = [cmds.append(['sshpass', '-p', password, 'scp', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', '../docker/docker_run_local.sh', 'pi@'+x+':/home/pi/git/GTernal/docker']) for x in id_to_ip.values()]
        _ = [cmds.append(['sshpass', '-p', password, 'ssh', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', 'pi@'+x, 'sudo reboot']) for x in id_to_ip.values()]

        pids = []
        for cmd in cmds:
            pids.append(subprocess.Popen(cmd))

        for pid in pids:
            pid.communicate()


if __name__ == '__main__':
    main()
