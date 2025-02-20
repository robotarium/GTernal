import argparse
import subprocess
import re
import json
import getpass
import time
import os
from concurrent.futures import ThreadPoolExecutor, as_completed

def check_containers_status(remote_host, username, password):
    # Construct the SSH command to run `docker inspect` for all specified containers on the remote host
    containers = ["firmware", "mac_discovery", "watchtower"]
    inspect_commands = " && ".join([f"docker inspect --format=\"{{{{.State.Status}}}}\" {container}" for container in containers])
    ssh_command = f"sshpass -p {password} ssh -o StrictHostKeyChecking=no {username}@{remote_host} '{inspect_commands}'"

    # Execute the command
    result = subprocess.run(ssh_command, shell=True, capture_output=True, text=True)

    statuses = {}
    # Check the output
    if result.returncode == 0:
        output_lines = result.stdout.strip().split('\n')
        for container, status in zip(containers, output_lines):
            statuses[container] = status.strip()
    else:
        for container in containers:
            statuses[container] = "error"
    
    return statuses

def get_status_color(status):
    if status == "created":
        return "\033[43m"  # Yellow background
    elif status == "running":
        return "\033[42m"  # Green background
    elif status == "restarting":
        return "\033[43m"  # Yellow background
    elif status == "paused":
        return "\033[43m"  # Yellow background
    elif status == "loading":
        return "\033[90m"  # Gray background
    else:
        return "\033[41m"  # Magenta background for error

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('mac_list', help='Path to JSON file containing MAC to ID mapping')
    parser.add_argument('interface', help='Network interface on which to make query')
    parser.add_argument('command', help='Command for sshpass')
    parser.add_argument('-c', help='Optional command for address', default=None)
    parser.add_argument('-n', help='Optional number of robots needed to be found', default=None)
    parser.add_argument('-d', help='Optional directory for address', default='')
    parser.add_argument('-f', help='Optional file to copy to the address', default='')
    parser.add_argument('-id', help='Optional ID of the robot', default=None)

    args = parser.parse_args()
    interface = args.interface

    try:
        f = open(args.mac_list, 'r')
        mac_to_id = json.load(f)
    except Exception as e:
        print(repr(e))
        print('Could not open file ({})'.format(args.mac_list))

    checkNumberRobots = True
    start_time = time.time()
    just_started = True


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

        # Filter id_to_ip based on the -id argument
        if args.id is not None:
            robot_ids = args.id.split(',')
            id_to_ip = {robot_id: ip for robot_id, ip in id_to_ip.items() if robot_id in robot_ids}

        elapsed_time = time.time() - start_time
        output_lines = []  # Collect output lines
        if args.command == 'discover':
            header = "{:<10} | {:<15} | {:^10} | {:^15} | {:^10}".format("Robot ID", "IP Address", "firmware", "mac_discovery", "watchtower")
        else:
            header = "{:<10} | {:<15}".format("Robot ID", "IP Address")

        if id_to_ip:
            output_lines.append(f'Detected {len(id_to_ip)} robots:')
            output_lines.append(header)
            output_lines.append("-" * len(header))

            if just_started:
                # Initialize results with 'loading' status
                results = [(robot_id, ip, {'firmware': 'loading', 'mac_discovery': 'loading', 'watchtower': 'loading'}) for robot_id, ip in id_to_ip.items()]

                # Sort the results based on robot ID
                results.sort(key=lambda x: int(x[0]))

                just_started = False

            if args.command == 'discover':
                for robot_id, ip, container_statuses in results:
                    firmware_status = container_statuses["firmware"]
                    mac_discovery_status = container_statuses["mac_discovery"]
                    watchtower_status = container_statuses["watchtower"]

                    firmware_color = get_status_color(firmware_status)
                    mac_discovery_color = get_status_color(mac_discovery_status)
                    watchtower_color = get_status_color(watchtower_status)

                    output_lines.append("{:<10} | {:<15} | {}{:^10}\033[0m | {}{:^15}\033[0m | {}{:^10}\033[0m".format(
                        robot_id, ip,
                        firmware_color, firmware_status,
                        mac_discovery_color, mac_discovery_status,
                        watchtower_color, watchtower_status
                    ))
                output_lines.append(f'Scanning for {elapsed_time:.2f} seconds... Use ctrl + c to exit')
                
                # Print all collected output lines at once
                os.system('clear')  # Clear the terminal screen
                print("\n".join(output_lines))

                with ThreadPoolExecutor(max_workers=os.cpu_count()) as executor:
                    futures = {executor.submit(check_containers_status, ip, 'pi', 'raspberry'): (robot_id, ip) for robot_id, ip in id_to_ip.items()}
                    results = []
                    for future in as_completed(futures):
                        robot_id, ip = futures[future]
                    # for robot_id, ip in sorted(id_to_ip.items(), key=lambda item: int(item[0])):
                        try:
                            results.append((robot_id, ip, future.result()))
                        except Exception as e:
                            results.append((robot_id, ip, {'firmware': 'error', 'mac_discovery': 'error', 'watchtower': 'error'}))

                # Sort the results based on robot ID
                results.sort(key=lambda x: int(x[0]))

            else:
                for robot_id, ip in sorted(id_to_ip.items(), key=lambda item: int(item[0])):
                    # output_lines.append(f'{robot_id} | {ip}')
                    output_lines.append("{:<10} | {:<15}".format(robot_id, ip))
                output_lines.append(f'Number of robots found / Number of robots provided by user: {len(id_to_ip)} / {args.n}')
                output_lines.append(f'All robots not found, retrying for {elapsed_time:.2f} seconds...')
                
                # Print all collected output lines at once
                os.system('clear')  # Clear the terminal screen
                print("\n".join(output_lines))

        else:
            output_lines.append('No robots discovered')
        
        if((args.n is not None) and (len(id_to_ip) is int(args.n))):
           checkNumberRobots = False


    if(args.command != 'discover'):
        print('Enter secrets for robots')
        password = getpass.getpass()
        if args.command == 'ssh':
            cmds = [['sshpass', '-p', password, 'ssh', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', 'pi@'+x, args.c] for x in id_to_ip.values()]
        elif args.command == 'scp':
            cmds = [['sshpass', '-p', password, 'scp', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', args.f, 'pi@'+x+':'+args.d] for x in id_to_ip.values()]
        elif args.command == 'setup':
            cmds = [['sshpass', '-p', password, 'scp', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', '../setup/setup', 'pi@'+x+':/home/pi'] for x in id_to_ip.values()]
            _ = [cmds.append(['sshpass', '-p', password, 'ssh', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', 'pi@'+x, 'sudo ./setup']) for x in id_to_ip.values()]

        pids = []
        for cmd in cmds:
            pids.append(subprocess.Popen(cmd))
            if args.command == 'setup':
                pids[-1].wait() # Needed to wait for the setup script to be copied

        for pid in pids:
            pid.communicate()

        # For updating the mac_list.json and docker_run.sh files and restarting the Pi to apply all settings
        if args.command == 'setup':
            cmds = []
            _ = [cmds.append(['sshpass', '-p', password, 'scp', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', '../config/mac_list.json', 'pi@'+x+':/home/pi/git/GTernal/config']) for x in id_to_ip.values()]
            _ = [cmds.append(['sshpass', '-p', password, 'scp', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', '../docker/docker_run.sh', 'pi@'+x+':/home/pi/git/GTernal/docker']) for x in id_to_ip.values()]
            _ = [cmds.append(['sshpass', '-p', password, 'ssh', '-o', 'UserKnownHostsFile=/dev/null', '-o', 'StrictHostKeyChecking=no', 'pi@'+x, 'sudo reboot']) for x in id_to_ip.values()]

            pids = []
            for cmd in cmds:
                pids.append(subprocess.Popen(cmd))

            for pid in pids:
                pid.communicate()
    
    # time.sleep(1)  # Refresh interval in seconds

if __name__ == '__main__':
    main()
