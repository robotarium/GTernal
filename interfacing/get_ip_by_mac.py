import argparse
import subprocess
import re
import json
import getpass
import threading
import sys

def run_command_with_progress(robot_id, cmds, progress_callback):
    for cmd in cmds:
        process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        while True:
            output = process.stdout.readline()
            if output == '' and process.poll() is not None:
                break
            if output:
                progress_callback(robot_id, output.strip())
        rc = process.poll()
        if rc != 0:
            return rc
    return 0

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
        with open(args.mac_list, 'r') as f:
            mac_to_id = json.load(f)
    except Exception as e:
        print(repr(e))
        print('Could not open file ({})'.format(args.mac_list))
        return

    checkNumberRobots = True   

    while checkNumberRobots:
        pid = subprocess.Popen(['arp-scan', '-I', interface, '-l', '-t', '100', '-r', '5'], stdout=subprocess.PIPE)
        out = pid.communicate()[0].decode()
        lines = out.split('\n')

        mac_to_ip = {}

        for line in lines:
            mac = re.search(r'([0-9A-F]{2}[:-]){5}([0-9A-F]{2})', line, re.I)
            if mac is None:
                continue
            mac = mac.group()

            ip = re.search(r'((2[0-5]|1[0-9]|[0-9])?[0-9]\.){3}((2[0-5]|1[0-9]|[0-9])?[0-9])', line, re.I)
            if ip is None:
                continue
            ip = ip.group()
            mac_to_ip[mac] = ip

        # Make ID to IP mapping
        if args.command == 'setup_base_image':
            id_to_ip = {mac_to_id[x]: y for x, y in mac_to_ip.items() if x in mac_to_id and mac_to_id[x] == "base_image"}
        else:
            id_to_ip = {mac_to_id[x]: y for x, y in mac_to_ip.items() if x in mac_to_id}

        print(id_to_ip)
        print('Number of robots found: ', len(id_to_ip))
        
        if args.n is None or len(id_to_ip) == int(args.n):
            checkNumberRobots = False
        else:
            print('Number of robots needed, provided by user: ', args.n)
            print('All robots not found, retrying...') 

    if args.command is None:
        return

    print('Enter secrets for robots')
    password = getpass.getpass()

    def progress_callback(robot_id, progress):
        status[robot_id] = progress
        sys.stdout.write("\033c")  # Clear the terminal
        for rid, stat in status.items():
            sys.stdout.write(f"[{rid}] {stat}\n")
        sys.stdout.flush()

    def run_setup_command(robot_id, cmd):
        run_command_with_progress(robot_id, cmd, progress_callback)

    status = {robot_id: "Starting..." for robot_id in id_to_ip.keys()}

    if args.command == 'ssh':
        cmds = [(robot_id, [['sshpass', '-p', password, 'ssh', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', f'pi@{ip}', args.c]]) for robot_id, ip in id_to_ip.items()]
    elif args.command == 'scp':
        cmds = [(robot_id, [['sshpass', '-p', password, 'scp', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', args.f, f'pi@{ip}:{args.d}']]) for robot_id, ip in id_to_ip.items()]
    elif args.command == 'setup':
        cmds = [(robot_id, [['sshpass', '-p', password, 'scp', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', '../setup/setup', f'pi@{ip}:/home/pi'],
                            ['sshpass', '-p', password, 'ssh', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', f'pi@{ip}', 'sudo ./setup']]) for robot_id, ip in id_to_ip.items()]
    elif args.command == 'setup_base_image':
        cmds = [(robot_id, [['sshpass', '-p', password, 'scp', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', '../setup/setup_base_image.sh', f'pi@{ip}:/home/pi'],
                            ['sshpass', '-p', password, 'scp', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', '../config/env_variables.sh', f'pi@{ip}:/home/pi'],
                            ['sshpass', '-p', password, 'ssh', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', f'pi@{ip}', 'sudo ./setup_base_image.sh']]) for robot_id, ip in id_to_ip.items()]
    elif args.command == 'setup_from_base':
        cmds = [(robot_id, [['sshpass', '-p', password, 'ssh', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', f'pi@{ip}', 'sudo rm -f /home/pi/git/GTernal/config/mac_list.json'],
                            ['sshpass', '-p', password, 'scp', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', '../config/mac_list.json', f'pi@{ip}:/home/pi/git/GTernal/config'],
                            ['sshpass', '-p', password, 'ssh', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', f'pi@{ip}', 'sudo /home/pi/git/GTernal/setup/setup_from_base.sh']]) for robot_id, ip in id_to_ip.items()]

    threads = []
    for robot_id, cmd in cmds:
        thread = threading.Thread(target=run_setup_command, args=(robot_id, cmd))
        thread.start()
        threads.append(thread)

    for thread in threads:
        thread.join()

if __name__ == '__main__':
    main()
