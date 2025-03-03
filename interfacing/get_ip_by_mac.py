import argparse
import subprocess
import re
import json
import getpass
import threading
import sys

def run_command_with_progress(robot_id, cmd, progress_callback):
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    while True:
        output = process.stdout.readline()
        if output == '' and process.poll() is not None:
            break
        if output:
            progress_callback(robot_id, output.strip())
    rc = process.poll()
    return rc

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
        sys.stdout.write(f"\r[{robot_id}] {progress}")
        sys.stdout.flush()

    def run_setup_command(robot_id, cmd):
        run_command_with_progress(robot_id, cmd, progress_callback)

    if args.command == 'ssh':
        cmds = [(robot_id, ['sshpass', '-p', password, 'ssh', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', f'pi@{ip}', args.c]) for robot_id, ip in id_to_ip.items()]
    elif args.command == 'scp':
        cmds = [(robot_id, ['sshpass', '-p', password, 'scp', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', args.f, f'pi@{ip}:{args.d}']) for robot_id, ip in id_to_ip.items()]
    elif args.command == 'setup':
        cmds = [(robot_id, ['sshpass', '-p', password, 'scp', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', '../setup/setup', f'pi@{ip}:/home/pi']) for robot_id, ip in id_to_ip.items()]
        cmds += [(robot_id, ['sshpass', '-p', password, 'ssh', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', f'pi@{ip}', 'sudo ./setup']) for robot_id, ip in id_to_ip.items()]
    elif args.command == 'setup_base_image':
        cmds = [(robot_id, ['sshpass', '-p', password, 'ssh', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', f'pi@{ip}', 'sudo /home/pi/git/GTernal/setup/setup_base_image']) for robot_id, ip in id_to_ip.items()]
    elif args.command == 'setup_from_base':
        cmds = [(robot_id, ['sshpass', '-p', password, 'ssh', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', f'pi@{ip}', 'sudo /home/pi/git/GTernal/setup/setup_from_base']) for robot_id, ip in id_to_ip.items()]
    elif args.command == 'setup_from_base_local':
        cmds = [(robot_id, ['sshpass', '-p', password, 'ssh', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', f'pi@{ip}', 'sudo /home/pi/git/GTernal/setup/setup_from_base_local']) for robot_id, ip in id_to_ip.items()]

    # pids = []
    # for cmd in cmds:
    #     pids.append(subprocess.Popen(cmd))
    #     pids[-1].wait() # Needed to wait for the setup script to be copied

    # for pid in pids:
    #     pid.communicate()

    # # For updating the mac_list.json and docker_run.sh files and restarting the Pi to apply all settings
    # if args.command == 'setup':
    #     cmds = []
    #     cmds += [(robot_id, ['sshpass', '-p', password, 'scp', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', '../config/mac_list.json', f'pi@{ip}:/home/pi/git/GTernal/config']) for robot_id, ip in id_to_ip.items()]
    #     cmds += [(robot_id, ['sshpass', '-p', password, 'scp', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', '../docker/docker_run.sh', f'pi@{ip}:/home/pi/git/GTernal/docker']) for robot_id, ip in id_to_ip.items()]
    #     cmds += [(robot_id, ['sshpass', '-p', password, 'ssh', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', f'pi@{ip}', 'sudo reboot']) for robot_id, ip in id_to_ip.items()]

    # elif args.command == 'setup_base_image':
    #     cmds = []
    #     cmds += [(robot_id, ['sshpass', '-p', password, 'scp', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', '../config/mac_list.json', f'pi@{ip}:/home/pi/git/GTernal/config']) for robot_id, ip in id_to_ip.items()]
    #     cmds += [(robot_id, ['sshpass', '-p', password, 'scp', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', '../docker/docker_run_local.sh', f'pi@{ip}:/home/pi/git/GTernal/docker']) for robot_id, ip in id_to_ip.items()]
    #     cmds += [(robot_id, ['sshpass', '-p', password, 'ssh', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', f'pi@{ip}', 'sudo reboot']) for robot_id, ip in id_to_ip.items()]

    threads = []
    for robot_id, cmd in cmds:
        thread = threading.Thread(target=run_setup_command, args=(robot_id, cmd))
        thread.start()
        threads.append(thread)

    for thread in threads:
        thread.join()

if __name__ == '__main__':
    main()
