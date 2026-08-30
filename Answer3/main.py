from abc import ABC, abstractmethod
import json
import platform
import subprocess

class HostInfo(ABC):
    def __init__(self):
        self.hostname = None
        self.memory = None
        self.cpu = None
        self.ip = None
        self.disk_size = None

    @abstractmethod
    def get_hardware_info(self):
        pass

    def display_hardware_info(self):
        print(json.dumps({
            "hostname": self.hostname,
            "memory": self.memory,
            "cpu": self.cpu,
            "ip": self.ip,
            "disk_size": self.disk_size
        }, indent=2))

class WindowsHost(HostInfo):
    def get_hardware_info(self):
        info = subprocess.check_output("systeminfo", shell=True).decode("utf-8", errors="ignore")
        for line in info.splitlines():
            line = line.strip()
            if line.startswith("Host Name:"):
                self.hostname = line.split(":", 1)[1].strip()
            elif line.startswith("Total Physical Memory:"):
                self.memory = line.split(":", 1)[1].strip()

        cpu = subprocess.check_output(
            'wmic cpu get Name /value', shell=True
        ).decode("utf-8", errors="ignore")
        for line in cpu.splitlines():
            if line.startswith("Name="):
                self.cpu = line.split("=", 1)[1].strip()
                break

        ip_out = subprocess.check_output(
            'powershell -Command "(Get-NetIPAddress -AddressFamily IPv4 | Where-Object { $_.IPAddress -ne \'127.0.0.1\' } | Select-Object -First 1).IPAddress"',
            shell=True
        ).decode("utf-8", errors="ignore").strip()
        self.ip = ip_out

        disk = subprocess.check_output(
            'wmic logicaldisk where "DeviceID=\'C:\'" get Size /value', shell=True
        ).decode("utf-8", errors="ignore")
        for line in disk.splitlines():
            if line.startswith("Size="):
                size_bytes = int(line.split("=", 1)[1].strip())
                self.disk_size = f"{size_bytes // (1024**3)} GB"
                break

class LinuxHost(HostInfo):
    def get_hardware_info(self):
        self.hostname = subprocess.check_output("hostname", shell=True).decode().strip()

        with open("/proc/meminfo") as f:
            for line in f:
                if line.startswith("MemTotal:"):
                    kb = int(line.split()[1])
                    self.memory = f"{kb // 1024} MB"
                    break

        with open("/proc/cpuinfo") as f:
            for line in f:
                if line.startswith("model name"):
                    self.cpu = line.split(":", 1)[1].strip()
                    break

        ip_out = subprocess.check_output(
            "hostname -I", shell=True
        ).decode().strip().split()[0]
        self.ip = ip_out

        disk = subprocess.check_output(
            "df -h / | tail -1 | awk '{print $2}'", shell=True
        ).decode().strip()
        self.disk_size = disk

if __name__ == "__main__":
    host = WindowsHost() if platform.system() == "Windows" else LinuxHost()
    host.get_hardware_info()
    host.display_hardware_info()
